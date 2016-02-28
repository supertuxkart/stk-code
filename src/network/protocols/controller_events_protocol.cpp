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

#include "network/protocols/controller_events_protocol.hpp"

#include "modes/world.hpp"
#include "karts/abstract_kart.hpp"
#include "karts/controller/controller.hpp"
#include "network/event.hpp"
#include "network/network_config.hpp"
#include "network/network_player_profile.hpp"
#include "network/game_setup.hpp"
#include "network/network_config.hpp"
#include "network/protocol_manager.hpp"
#include "network/stk_host.hpp"
#include "network/stk_peer.hpp"
#include "utils/log.hpp"

//-----------------------------------------------------------------------------

ControllerEventsProtocol::ControllerEventsProtocol()
                        : Protocol( PROTOCOL_CONTROLLER_EVENTS)
{
}   // ControllerEventsProtocol

//-----------------------------------------------------------------------------

ControllerEventsProtocol::~ControllerEventsProtocol()
{
}   // ~ControllerEventsProtocol

//-----------------------------------------------------------------------------

bool ControllerEventsProtocol::notifyEventAsynchronous(Event* event)
{
    if (event->getType() != EVENT_TYPE_MESSAGE)
        return true;

    const NetworkString &data = event->data();
    if (data.size() < 17)
    {
        Log::error("ControllerEventsProtocol",
                   "The data supplied was not complete. Size was %d.",
                    data.size());
        return true;
    }
    uint32_t token = data.getUInt32();
    NetworkString pure_message = data;
    pure_message.removeFront(4);
    if (token != event->getPeer()->getClientServerToken())
    {
        Log::error("ControllerEventsProtocol", "Bad token from peer.");
        return true;
    }
    NetworkString ns = pure_message;

    ns.removeFront(4);
    uint8_t client_index = -1;
    while (ns.size() >= 9)
    {
        //uint8_t controller_index = ns.gui8();
        uint8_t kart_id = ns.getUInt8();
        if (kart_id >=World::getWorld()->getNumKarts())
        {
            Log::warn("ControllerEventProtocol", "No valid kart id (%s).",
                      kart_id);
            return true;
        }
        uint8_t serialized_1   = ns.getUInt8(1);
        PlayerAction action    = (PlayerAction)(ns.getUInt8(4));
        int action_value       = ns.getUInt32(5);
        Controller *controller = World::getWorld()->getKart(kart_id)
                                                  ->getController();
        KartControl *controls  = controller->getControls();
        controls->m_brake      = (serialized_1 & 0x40)!=0;
        controls->m_nitro      = (serialized_1 & 0x20)!=0;
        controls->m_rescue     = (serialized_1 & 0x10)!=0;
        controls->m_fire       = (serialized_1 & 0x08)!=0;
        controls->m_look_back  = (serialized_1 & 0x04)!=0;
        controls->m_skid       = KartControl::SkidControl(serialized_1 & 0x03);

        controller->action(action, action_value);
        ns.removeFront(9);
        //Log::info("ControllerEventProtocol", "Registered one action.");
    }
    if (ns.size() > 0 && ns.size() != 9)
    {
        Log::warn("ControllerEventProtocol",
                  "The data seems corrupted. Remains %d", ns.size());
        return true;
    }
    if (NetworkConfig::get()->isServer())
    {
        const std::vector<STKPeer*> &peers = STKHost::get()->getPeers();
        for(unsigned int i=0; i<peers.size(); i++)
        {
            STKPeer *peer = peers[i];
            // Don't send message to the host from which the message 
            // was sent from originally
            if(peer != event->getPeer())
            {
                pure_message.setToken(peer->getClientServerToken());
                peer->sendPacket(&pure_message, false);
            }   // if peer != event->getPeer()
        }   // for i in peers
    }   // if server
    return true;
}   // notifyEventAsynchronous

//-----------------------------------------------------------------------------

void ControllerEventsProtocol::update()
{
}   // update

//-----------------------------------------------------------------------------
/** Called from the local kart controller when an action (like steering,
 *  acceleration, ...) was triggered. It compresses the current kart control
 *  state and sends a message with the new info to the server.
 *  \param controller The controller that triggered the action.
 *  \param action Which action was triggered.
 *  \param value New value for the given action.
 */
void ControllerEventsProtocol::controllerAction(Controller* controller,
                                                PlayerAction action, int value)
{
    assert(!NetworkConfig::get()->isServer());

    KartControl* controls = controller->getControls();
    uint8_t serialized_1 = 0;
    serialized_1 |= (controls->m_brake==true);
    serialized_1 <<= 1;
    serialized_1 |= (controls->m_nitro==true);
    serialized_1 <<= 1;
    serialized_1 |= (controls->m_rescue==true);
    serialized_1 <<= 1;
    serialized_1 |= (controls->m_fire==true);
    serialized_1 <<= 1;
    serialized_1 |= (controls->m_look_back==true);
    serialized_1 <<= 2;
    serialized_1 += controls->m_skid;
    uint8_t serialized_2 = (uint8_t)(controls->m_accel*255.0);
    uint8_t serialized_3 = (uint8_t)(controls->m_steer*127.0);

    NetworkString *ns = getNetworkString(17);
    ns->setToken(STKHost::get()->getPeers()[0]->getClientServerToken());
    ns->addFloat(World::getWorld()->getTime());
    ns->addUInt8(controller->getKart()->getWorldKartId());
    ns->addUInt8(serialized_1).addUInt8(serialized_2).addUInt8(serialized_3);
    ns->addUInt8((uint8_t)(action)).addUInt32(value);

    Log::info("ControllerEventsProtocol", "Action %d value %d", action, value);
    sendToServer(ns, false); // send message to server
}   // controllerAction
