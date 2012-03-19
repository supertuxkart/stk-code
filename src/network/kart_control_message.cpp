//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2008 Joerg Henrichs
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

#include "network/kart_control_message.hpp"

#include "karts/controller/controller.hpp"
#include "modes/world.hpp"
#include "network/network_kart.hpp"

KartControlMessage::KartControlMessage()
                  : Message(Message::MT_KART_CONTROL)
{
    World *world=World::getWorld();
    unsigned int num_local_players = race_manager->getNumLocalPlayers();
    unsigned int control_size      = KartControl::getLength();
    allocate(control_size*num_local_players);
    for(unsigned int i=0; i<num_local_players; i++)
    {
        const AbstractKart *kart    = world->getLocalPlayerKart(i);
        const KartControl& controls = kart->getControls();
        controls.serialise(this);
    }
}   // KartControlMessage
// ----------------------------------------------------------------------------
/** Receives a kart control message. 
 *  \param kart_id_offset is the global id of the first kart on the host from 
 *         which this packet was received.
 */
KartControlMessage::KartControlMessage(ENetPacket* pkt, int kart_id_offset,
                                       int num_local_players)
                  : Message(pkt, MT_KART_CONTROL)
{
    // FIXME: This probably does not work anymore - it assume that
    // num_local_Players is the number of all local karts, while it might
    // only be the number of all network karts.
    for(int i=kart_id_offset; i<kart_id_offset+num_local_players; i++)
    {
        KartControl kc(this);
        AbstractKart *kart = World::getWorld()->getKart(i);
        if(kart->getController()->isNetworkController())
        {
            ((NetworkKart*)kart)->setControl(kc);
        }
    }
};   // KartControlMessage

