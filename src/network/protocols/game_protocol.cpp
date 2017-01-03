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

#include "modes/world.hpp"
#include "karts/abstract_kart.hpp"
#include "karts/controller/player_controller.hpp"
#include "network/event.hpp"
#include "network/network_config.hpp"
#include "network/network_player_profile.hpp"
#include "network/game_setup.hpp"
#include "network/network_config.hpp"
#include "network/network_string.hpp"
#include "network/protocol_manager.hpp"
#include "network/rewind_manager.hpp"
#include "network/stk_host.hpp"
#include "network/stk_peer.hpp"
#include "utils/log.hpp"

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
/** Synchronous update - will send all commands collected during the last
 *  frame (and could optional only send messages every N frames).
 */
void GameProtocol::update(float dt)
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
        m_data_to_send->addFloat(a.m_time);
        m_data_to_send->addUInt8(a.m_kart_id);
        m_data_to_send->addUInt8((uint8_t)(a.m_action)).addUInt32(a.m_value);
    }   // for a in m_all_actions

        // FIXME: for now send reliable
    sendToServer(m_data_to_send, /*reliable*/ true);
    m_all_actions.clear();
}   // update

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
                                    int value)
{
    // Store the action in the list of actions that will be sent to the
    // server next.
    assert(NetworkConfig::get()->isClient());
    Action a;
    a.m_kart_id = kart_id;
    a.m_action  = action;
    a.m_value   = value;
    a.m_time    = World::getWorld()->getTime();

    m_all_actions.push_back(a);

    // Store the event in the rewind manager, which is responsible
    // for freeing the allocated memory
    BareNetworkString *s = new BareNetworkString(4);
    s->addUInt8(kart_id).addUInt8(action).addUInt16(uint16_t(value));
    RewindManager::get()->addEvent(this, s, /*confirmed*/true,
                                   World::getWorld()->getTime() );

    Log::info("GameProtocol", "Action at %f: %d value %d",
              World::getWorld()->getTime(), action, value);
}   // controllerAction

// ----------------------------------------------------------------------------
/** Called when a controller event is receiver - either on the server from
 *  a client, or on a client from the server. It sorts the event into the
 *  RewindManager's network event queue. The server will also send this 
 *  event immediately to all clients (except to the original sender).
 */
void GameProtocol::handleControllerAction(Event *event)
{
    NetworkString &data = event->data();
    uint8_t count = data.getUInt8();
    for (unsigned int i = 0; i < count; i++)
    {
        float   time    = data.getFloat();
        uint8_t kart_id = data.getUInt8();
        assert(kart_id < World::getWorld()->getNumKarts());

        PlayerAction action = (PlayerAction)(data.getUInt8());
        int          value  = data.getUInt32();
        Log::info("GameProtocol", "Action at %f: %d %d %d",
                  time, kart_id, action, value);
        BareNetworkString *s = new BareNetworkString(3);
        s->addUInt8(kart_id).addUInt8(action).addUInt16(value);
        RewindManager::get()->addNetworkEvent(this, s, time);
    }

    if (data.size() > 0)
    {
        Log::warn("GameProtocol",
                  "Received invalid controller data - remains %d",data.size());
    }
    if (NetworkConfig::get()->isServer())
    {
        // Send update to all clients except the original sender.
        STKHost::get()->sendPacketExcept(event->getPeer(),
                                         &data, false);
    }   // if server

}   // handleControllerAction

// ----------------------------------------------------------------------------
/** Called by the server before assembling a new message containing the full
 *  state of the race to be sent to a client.
 */
void GameProtocol::startNewState()
{
    assert(NetworkConfig::get()->isServer());
    m_data_to_send->clear();
    m_data_to_send->addUInt8(GP_STATE).addFloat(World::getWorld()->getTime());
    Log::info("GameProtocol", "Sending new state at %f.",
              World::getWorld()->getTime());
}   // startNewState

// ----------------------------------------------------------------------------
/** Called by a server to add data to the current state.
 */
void GameProtocol::addState(BareNetworkString *buffer)
{
    assert(NetworkConfig::get()->isServer());
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
    sendMessageToPeersChangingToken(m_data_to_send, /*reliable*/true);
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
    NetworkString &data = event->data();
    float time          = data.getFloat();
    Log::info("GameProtocol", "Received at %f state from %f",
              World::getWorld()->getTime(), time);
    int index           = 0;
    while (data.size() > 0)
    {
        uint16_t count = data.getUInt16();
        BareNetworkString *state = new BareNetworkString(data.getCurrentData(),
                                                         count);
        data.skip(count);
        RewindManager::get()->addNetworkState(index, state, time);
        index++;
    }   // while data.size()>0
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
    int value = buffer->getUInt16();
    Controller *c = World::getWorld()->getKart(kart_id)->getController();
    c->action(action, value);
}   // rewind
