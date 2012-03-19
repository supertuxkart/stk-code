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

#include "network/race_result_message.hpp"

#include "karts/abstract_kart.hpp"
#include "modes/world.hpp"
#include "race/race_manager.hpp"

/** Creates a message containing the finishing time and rank of each kart. 
 *  This message is serialised so that it can be sent.
 */
RaceResultMessage::RaceResultMessage() : Message(MT_RACE_RESULT)
{
    World *world = World::getWorld();
    const unsigned int num_karts = world->getNumKarts();
    allocate(num_karts * (getFloatLength()+getCharLength()));
    for(unsigned int i=0; i<num_karts; i++)
    {
        const AbstractKart *kart = world->getKart(i);
        addFloat(kart->getFinishTime());
        addChar(kart->getPosition());
    }   // for i in karts
}   // RaceResultMessage

// ----------------------------------------------------------------------------
/** De-serialises a race result message and sets the appropriate results in 
 *  the kart and the race manager.
 *  \param pkt The enet message paket.
 */
RaceResultMessage::RaceResultMessage(ENetPacket* pkt) 
                 : Message(pkt, MT_RACE_RESULT)
{
    World *world = World::getWorld();
    const unsigned int num_karts = world->getNumKarts();
    for(unsigned int i=0; i<num_karts; i++)
    {
        AbstractKart *kart = world->getKart(i);
        float time  = getFloat();
        char position = getChar();
        kart->setPosition(position);
        kart->finishedRace(time);
    }
}   // RaceResultMessage

