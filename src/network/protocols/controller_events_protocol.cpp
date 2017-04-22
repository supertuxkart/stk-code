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
    if(!checkDataSize(event, 13)) return true;

    NetworkString &data = event->data();
    float time = data.getFloat();

    uint8_t client_index = -1;
    while (data.size() >= 9)
    {
        uint8_t kart_id = data.getUInt8();
        if (kart_id >=World::getWorld()->getNumKarts())
        {
            Log::warn("ControllerEventProtocol", "No valid kart id (%s).",
                      kart_id);
            continue;
        }
        uint8_t serialized_1   = data.getUInt8();
        uint8_t serialized_2   = data.getUInt8();
        uint8_t serialized_3   = data.getUInt8();
        PlayerAction action    = (PlayerAction)(data.getUInt8());
        int action_value       = data.getUInt32();
        Log::info("ControllerEventsProtocol", "KartID %d action %d value %d",
                  kart_id, action, action_value);
        Controller *controller = World::getWorld()->getKart(kart_id)
                                                  ->getController();
        KartControl *controls  = controller->getControls();
        controls->setBrake(   (serialized_1 & 0x40)!=0);
        controls->setNitro(   (serialized_1 & 0x20)!=0);
        controls->setRescue(  (serialized_1 & 0x10)!=0);
        controls->setFire(    (serialized_1 & 0x08)!=0);
        controls->setLookBack((serialized_1 & 0x04)!=0);
        controls->setSkidControl(KartControl::SkidControl(serialized_1 & 0x03));

        controller->action(action, action_value);
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
    serialized_1 |= (controls->getBrake()==true);
    serialized_1 <<= 1;
    serialized_1 |= (controls->getNitro()==true);
    serialized_1 <<= 1;
    serialized_1 |= (controls->getRescue()==true);
    serialized_1 <<= 1;
    serialized_1 |= (controls->getFire()==true);
    serialized_1 <<= 1;
    serialized_1 |= (controls->getLookBack()==true);
    serialized_1 <<= 2;
    serialized_1 += controls->getSkidControl();
    uint8_t serialized_2 = (uint8_t)(controls->getAccel()*255.0);
    uint8_t serialized_3 = (uint8_t)(controls->getSteer()*127.0);

    NetworkString *ns = getNetworkString(13);
    ns->addFloat(World::getWorld()->getTime());
    ns->addUInt8(controller->getKart()->getWorldKartId());
    ns->addUInt8(serialized_1).addUInt8(serialized_2).addUInt8(serialized_3);
    ns->addUInt8((uint8_t)(action)).addUInt32(value);
    sendToServer(ns, false); // send message to server
    delete ns;

    Log::info("ControllerEventsProtocol", "Action %d value %d", action, value);
}   // controllerAction
