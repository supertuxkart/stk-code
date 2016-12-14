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
bool GameProtocol::notifyEventAsynchronous(Event* event)
{
    if(!checkDataSize(event, 1)) return true;

    NetworkString &data = event->data();
    uint8_t count = data.getUInt8();
    for (unsigned int i = 0; i < count; i++)
    {
        float        time    = data.getFloat();
        uint8_t      kart_id = data.getUInt8();
        assert(kart_id < World::getWorld()->getNumKarts());
        PlayerAction action  = (PlayerAction)(data.getUInt8());
        int          value   = data.getUInt32();
        Log::info("GameProtocol", "Action at %f: %d %d %d",
                  time, kart_id, action, value);
        BareNetworkString *s = new BareNetworkString(3);
        s->addUInt8(kart_id).addUInt8(action).addUInt16(value);
        RewindManager::get()->addEvent(this, s, /*confirmed*/ true, time);
    }

    if (data.size() > 0 )
    {
        Log::warn("ControllerEventProtocol",
                  "The data seems corrupted. Remains %d", data.size());
    }
    if (NetworkConfig::get()->isServer())
    {
        // Send update to all clients except the original sender.
        STKHost::get()->sendPacketExcept(event->getPeer(), 
                                         &data, false);
    }   // if server
    return true;
}   // notifyEventAsynchronous

//-----------------------------------------------------------------------------
/** Synchronous update - will send all commands collected during the last
 *  frame (and could optional only send messages every N frames). */
void GameProtocol::update(float dt)
{
    if (m_all_actions.size() == 0) return;   // nothing to do

    // Clear left-over data from previous frame. This way the network
    // string will increase till it reaches maximum size necessary
    m_data_to_send->clear();
    m_data_to_send->addUInt8( uint8_t( m_all_actions.size() ) );

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
