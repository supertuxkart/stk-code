//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2015  Supertuxkart-Team
//
//  This program is free software; you can redistribute it and/or
//  modify it under the terms of the GNU General Public License
//  as published by the Free Software Foundation; either version 3
//  of the License, or (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

#include "network/protocols/game_protocol.hpp"

#include "items/item_manager.hpp"
#include "items/network_item_manager.hpp"
#include "karts/abstract_kart.hpp"
#include "karts/controller/player_controller.hpp"
#include "modes/world.hpp"
#include "network/event.hpp"
#include "network/network_config.hpp"
#include "network/game_setup.hpp"
#include "network/network_config.hpp"
#include "network/network_string.hpp"
#include "network/protocol_manager.hpp"
#include "network/rewind_manager.hpp"
#include "network/stk_host.hpp"
#include "network/stk_peer.hpp"
#include "utils/log.hpp"
#include "utils/time.hpp"
#include "main_loop.hpp"

// ============================================================================
std::weak_ptr<GameProtocol> GameProtocol::m_game_protocol;
// ============================================================================
std::shared_ptr<GameProtocol> GameProtocol::createInstance()
{
    if (!emptyInstance())
    {
        Log::fatal("GameProtocol", "Create only 1 instance of GameProtocol!");
        return NULL;
    }
    auto gm = std::make_shared<GameProtocol>();
    m_game_protocol = gm;
    return gm;
}   // createInstance

//-----------------------------------------------------------------------------
/** Constructor. Allocates the buffer for events to send to the server. */
GameProtocol::GameProtocol()
            : Protocol( PROTOCOL_CONTROLLER_EVENTS)
{
    m_data_to_send = getNetworkString();

}   // GameProtocol

//-----------------------------------------------------------------------------
GameProtocol::~GameProtocol()
{
    delete m_data_to_send;
}   // ~GameProtocol

//-----------------------------------------------------------------------------
/** Will send all commands collected during the last
 *  frame (and could optional only send messages every N frames).
 */
void GameProtocol::sendAllActions()
{
    if (m_all_actions.size() == 0) return;   // nothing to do

    // Clear left-over data from previous frame. This way the network
    // string will increase till it reaches maximum size necessary
    m_data_to_send->clear();
    m_data_to_send->addUInt8(GP_CONTROLLER_ACTION)
                   .addUInt8(uint8_t(m_all_actions.size()));

    // Add all actions
    for (auto a : m_all_actions)
    {
        m_data_to_send->addUInt32(a.m_ticks);
        m_data_to_send->addUInt8(a.m_kart_id);
        m_data_to_send->addUInt8((uint8_t)(a.m_action)).addUInt32(a.m_value)
                       .addUInt32(a.m_value_l).addUInt32(a.m_value_r);
    }   // for a in m_all_actions

    // FIXME: for now send reliable
    sendToServer(m_data_to_send, /*reliable*/ true);
    m_all_actions.clear();
}   // sendAllActions

//-----------------------------------------------------------------------------
/** Called when a message from a remote GameProtocol is received.
 */
bool GameProtocol::notifyEventAsynchronous(Event* event)
{
    if(!checkDataSize(event, 1)) return true;

    NetworkString &data = event->data();
    uint8_t message_type = data.getUInt8();
    switch (message_type)
    {
    case GP_CONTROLLER_ACTION: handleControllerAction(event); break;
    case GP_STATE:             handleState(event);            break;
    case GP_ADJUST_TIME:       handleAdjustTime(event);       break;
    //case GP_ITEM_UPDATE:       handleItemUpdate(event);       break;
    case GP_ITEM_CONFIRMATION: handleItemEventConfirmation(event); break;
    default: Log::error("GameProtocol",
                        "Received unknown message type %d - ignored.",
                        message_type);                        break;
    }   // switch message_type
    return true;
}   // notifyEventAsynchronous

//-----------------------------------------------------------------------------
/** Called from the local kart controller when an action (like steering,
 *  acceleration, ...) was triggered. It sends a message with the new info
 *  to the server and informs the rewind manager to store the event.
 *  \param Kart id that triggered the action.
 *  \param action Which action was triggered.
 *  \param value New value for the given action.
 */
void GameProtocol::controllerAction(int kart_id, PlayerAction action,
                                    int value, int val_l, int val_r)
{
    // Store the action in the list of actions that will be sent to the
    // server next.
    assert(NetworkConfig::get()->isClient());
    Action a;
    a.m_kart_id = kart_id;
    a.m_action  = action;
    a.m_value   = value;
    a.m_value_l = val_l;
    a.m_value_r = val_r;
    a.m_ticks   = World::getWorld()->getTimeTicks();

    m_all_actions.push_back(a);

    // Store the event in the rewind manager, which is responsible
    // for freeing the allocated memory
    BareNetworkString *s = new BareNetworkString(4);
    s->addUInt8(kart_id).addUInt8(action).addUInt32(value)
                        .addUInt32(val_l).addUInt32(val_r);

    RewindManager::get()->addEvent(this, s, /*confirmed*/true,
                                   World::getWorld()->getTimeTicks() );

    Log::info("GameProtocol", "Action at %d: %d value %d",
              World::getWorld()->getTimeTicks(), action, 
              action==PlayerAction::PA_STEER_RIGHT ? -value : value);
}   // controllerAction

// ----------------------------------------------------------------------------
/** Called when a controller event is received - either on the server from
 *  a client, or on a client from the server. It sorts the event into the
 *  RewindManager's network event queue. The server will also send this 
 *  event immediately to all clients (except to the original sender).
 */
void GameProtocol::handleControllerAction(Event *event)
{
    NetworkString &data = event->data();
    uint8_t count = data.getUInt8();
    bool will_trigger_rewind = false;
    int rewind_delta = 0;
    int cur_ticks = 0;
    const int not_rewound = RewindManager::get()->getNotRewoundWorldTicks();
    for (unsigned int i = 0; i < count; i++)
    {
        cur_ticks = data.getUInt32();
        // Since this is running in a thread, it might be called during
        // a rewind, i.e. with an incorrect world time. So the event
        // time needs to be compared with the World time independent
        // of any rewinding.
        if (cur_ticks < not_rewound && !will_trigger_rewind)
        {
            will_trigger_rewind = true;
            rewind_delta = not_rewound - cur_ticks;
        }
        uint8_t kart_id = data.getUInt8();
        assert(kart_id < World::getWorld()->getNumKarts());

        PlayerAction action = (PlayerAction)(data.getUInt8());
        int value   = data.getUInt32();
        int value_l = data.getUInt32();
        int value_r = data.getUInt32();
        Log::info("GameProtocol", "Action at %d: %d %d %d %d %d",
                  cur_ticks, kart_id, action, value, value_l, value_r);
        BareNetworkString *s = new BareNetworkString(3);
        s->addUInt8(kart_id).addUInt8(action).addUInt32(value)
                            .addUInt32(value_l).addUInt32(value_r);
        RewindManager::get()->addNetworkEvent(this, s, cur_ticks);
    }

    if (data.size() > 0)
    {
        Log::warn("GameProtocol",
                  "Received invalid controller data - remains %d",data.size());
    }
    if (NetworkConfig::get()->isServer())
    {
        // Send update to all clients except the original sender if the event
        // is after the server time
        if (!will_trigger_rewind)
            STKHost::get()->sendPacketExcept(event->getPeer(), &data, false);

        if (not_rewound == 0 ||
            m_initial_ticks.find(event->getPeer()) == m_initial_ticks.end())
            return;
        int cur_diff = cur_ticks - not_rewound;
        const int max_adjustment = 12;
        const int ticks_difference = m_initial_ticks.at(event->getPeer());
        if (will_trigger_rewind)
        {
            rewind_delta += max_adjustment;
            Log::info("GameProtocol", "At %d %f %d requesting time adjust"
                " (speed up) of %d for host %d",
                World::getWorld()->getTimeTicks(), StkTime::getRealTime(),
                not_rewound, rewind_delta, event->getPeer()->getHostId());
            // This message from a client triggered a rewind in the server.
            // To avoid this, signal to the client that it should speed up.
            adjustTimeForClient(event->getPeer(), rewind_delta);
            return;
        }

        if (cur_diff > 0 &&
            cur_diff - ticks_difference > max_adjustment)
        {
            const int adjustment = ticks_difference - cur_diff;
            Log::info("GameProtocol", "At %d %f %d requesting time adjust"
                " (slow down) of %d for host %d",
                World::getWorld()->getTimeTicks(), StkTime::getRealTime(),
                not_rewound, adjustment, event->getPeer()->getHostId());
            adjustTimeForClient(event->getPeer(), adjustment);
        }
    }   // if server

}   // handleControllerAction

// ----------------------------------------------------------------------------
/** The server might request that a client adjusts its world clock (in order to
 *  reduce rewinds). This function sends a a (unreliable) message to the 
 *  client.
 *  \param peer The peer that triggered the rewind.
 *  \param t Time that the peer needs to slowdown (<0) or speed up(>0).
 */
void GameProtocol::adjustTimeForClient(STKPeer *peer, int ticks)
{
    assert(NetworkConfig::get()->isServer());

    if (m_last_adjustments.find(peer) != m_last_adjustments.end() &&
        StkTime::getRealTime() - m_last_adjustments.at(peer) < 3.0)
        return;

    NetworkString *ns = getNetworkString(5);
    ns->addUInt8(GP_ADJUST_TIME).addUInt32(ticks);
    // This message can be send unreliable, it's not critical if it doesn't
    // get delivered, the server will request again later anyway.
    peer->sendPacket(ns, /*reliable*/false);
    m_last_adjustments[peer] = StkTime::getRealTime();
    delete ns;
}   // adjustTimeForClient

// ----------------------------------------------------------------------------
/** Called on a client when the server requests an adjustment of this client's
 *  world clock time (in order to reduce rewinds).
 */
void GameProtocol::handleAdjustTime(Event *event)
{
    int ticks = event->data().getUInt32();
    main_loop->setTicksAdjustment(ticks);
    Log::verbose("GameProtocol", "%d ticks adjustment", ticks);
}   // handleAdjustTime

// ----------------------------------------------------------------------------
/** Sends a confirmation to the server that all item events up to 'ticks'
 *  have been received.
 *  \param ticks Up to which time in ticks the item events have been received.
 */
void GameProtocol::sendItemEventConfirmation(int ticks)
{
    assert(NetworkConfig::get()->isClient());
    NetworkString *ns = getNetworkString(5);
    ns->addUInt8(GP_ITEM_CONFIRMATION).addUInt32(ticks);
    // This message can be sent unreliable, it's not critical if it doesn't
    // get delivered, a future update will come through
    sendToServer(ns, /*reliable*/false);
    delete ns;
}   // sendItemEventConfirmation

// ----------------------------------------------------------------------------
/** Handles an item even confirmation from a client. Once it has been confirmed
 *  that all clients have received certain events, those can be deleted and
 *  do not need to be sent again.
 *  \param event The data from the client.
 */
void GameProtocol::handleItemEventConfirmation(Event *event)
{
    assert(NetworkConfig::get()->isServer());
    int ticks = event->data().getTime();
    NetworkItemManager::get()->setItemConfirmationTime(event->getPeerSP(),
        ticks);
}   // handleItemEventConfirmation

// ----------------------------------------------------------------------------
/** Called by the server before assembling a new message containing the full
 *  state of the race to be sent to a client.
 * \param local_save If set it allows a state to be saved on a client.
 *        This only happens at the very first time step to ensure each client
 *        has a state in case it receives an event before a server state.
 */
void GameProtocol::startNewState(bool local_save)
{
    assert(local_save || NetworkConfig::get()->isServer());

    m_data_to_send->clear();
    // Local saves don't neet this info, they pass time directly to the
    // RewindInfo in RewindManager::saveLocalState.
    if (!local_save)
    {
        m_data_to_send->addUInt8(GP_STATE)
                       .addUInt32(World::getWorld()->getTimeTicks());
    }
}   // startNewState

// ----------------------------------------------------------------------------
/** Called by a server to add data to the current state. The data in buffer
 *  is copied, so the data can be freed after this call/.
 *  \param buffer Adds the data in the buffer to the current state.
 */
void GameProtocol::addState(BareNetworkString *buffer)
{
    m_data_to_send->addUInt16(buffer->size());
    (*m_data_to_send) += *buffer;
}   // addState

// ----------------------------------------------------------------------------
/** Called when the last state information has been added and the message
 *  can be sent to the clients.
 */
void GameProtocol::sendState()
{
    assert(NetworkConfig::get()->isServer());
    sendMessageToPeers(m_data_to_send, /*reliable*/true);
}   // sendState

// ----------------------------------------------------------------------------
/** Called when a new full state is received form the server.
 */
void GameProtocol::handleState(Event *event)
{
    // Ignore events arriving when client has already exited
    if (!World::getWorld())
        return;

    assert(NetworkConfig::get()->isClient());
    const NetworkString &data = event->data();
    int ticks          = data.getUInt32();
    // Now copy the state data (without ticks etc) to a new
    // string, so it can be reset to the beginning easily
    // when restoring the state:
    BareNetworkString *bns = new BareNetworkString(data.getCurrentData(),
                                                   data.size());

    // The memory for bns will be handled in the RewindInfoState object
    RewindManager::get()->addNetworkState(bns, ticks);
}   // handleState

// ----------------------------------------------------------------------------
/** Called from the RewindManager when rolling back.
 *  \param buffer Pointer to the saved state information.
 */
void GameProtocol::undo(BareNetworkString *buffer)
{

}   // undo

// ----------------------------------------------------------------------------
/** Called from the RewindManager after a rollback to replay the stored
 *  events.
 *  \param buffer Pointer to the saved state information.
 */
void GameProtocol::rewind(BareNetworkString *buffer)
{
    int kart_id = buffer->getUInt8();
    PlayerAction action = PlayerAction(buffer->getUInt8());
    int value   = buffer->getUInt32();
    int value_l = buffer->getUInt32();
    int value_r = buffer->getUInt32();
    Controller *c = World::getWorld()->getKart(kart_id)->getController();
    PlayerController *pc = dynamic_cast<PlayerController*>(c);
    // This can be endcontroller when finishing the race
    if (pc)
        pc->actionFromNetwork(action, value, value_l, value_r);
}   // rewind

// ----------------------------------------------------------------------------
void GameProtocol::addInitialTicks(STKPeer* p, int ticks)
{
    Log::verbose("GameProtocol", "Host %d with ticks difference %d",
        p->getHostId(), ticks);
    m_initial_ticks[p] = ticks;
}   // addInitialTicks
