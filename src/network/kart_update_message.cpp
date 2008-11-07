//  $Id:kart_update_message.cpp 2128 2008-06-13 00:53:52Z cosmosninja $
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
#include "kart_update_message.hpp"
#include "modes/world.hpp"
#include "karts/kart.hpp"

KartUpdateMessage::KartUpdateMessage()
                 : Message(Message::MT_KART_INFO)
{
    unsigned int num_karts = RaceManager::getWorld()->getCurrentNumKarts();

    // Send the number of karts and for each kart the compressed 
    // control structure (3 ints) and xyz,hpr (4 floats: quaternion:
    allocate(getCharLength()+
             num_karts*(KartControl::getLength() + getVec3Length()
                         +getQuaternionLength()) );
    addChar(num_karts);
    for(unsigned int i=0; i<num_karts; i++)
    {
        const Kart* kart = RaceManager::getKart(i);
        const KartControl& kc=kart->getControls();
        kc.serialise(this);
        addVec3(kart->getXYZ());
        addQuaternion(kart->getRotation());
    }   // for i
}   // KartUpdateMessage
// ----------------------------------------------------------------------------
KartUpdateMessage::KartUpdateMessage(ENetPacket* pkt)
                  : Message(pkt, MT_KART_INFO)
{
    unsigned int num_karts = getInt();
    for(unsigned int i=0; i<num_karts; i++)
    {
        // Currently not used
        KartControl kc(this);
        Vec3 xyz = getVec3();
        btQuaternion q = getQuaternion();
        Kart *kart = RaceManager::getKart(i);
        kart->setXYZ(xyz);
        kart->setRotation(q);
    }   // for i
};   // KartUpdateMessage

