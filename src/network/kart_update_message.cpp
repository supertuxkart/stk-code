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
#include "world.hpp"
#include "kart.hpp"

KartUpdateMessage::KartUpdateMessage()
                 : Message(Message::MT_KART_INFO)
{
    unsigned int num_karts = world->getCurrentNumKarts();
    KartControl c;
    // Send the number of karts and for each kart the compressed 
    // control structure (3 ints) and xyz,hpr (4 floats: quaternion:
    allocate(getLength(num_karts)+
             num_karts*(KartControl::getCompressedSize() + 7*getLength(1.0f)) );
    add(num_karts);
    for(unsigned int i=0; i<num_karts; i++)
    {
        const Kart* kart=world->getKart(i);
        const KartControl& kc=kart->getControls();
        assert(KartControl::getCompressedSize()<=9);
        char compressed[9];         // avoid the new/delete overhead
        kc.compress(compressed);
        add(compressed, KartControl::getCompressedSize());
        add(kart->getXYZ());
        add(kart->getRotation());
    }   // for i
}   // KartUpdateMessage
// ----------------------------------------------------------------------------
KartUpdateMessage::KartUpdateMessage(ENetPacket* pkt)
                  : Message(pkt, MT_KART_INFO)
{
    unsigned int num_karts = getInt();
    for(unsigned int i=0; i<num_karts; i++)
    {
        assert(KartControl::getCompressedSize()<=9);
        char compressed[9];   // avoid new/delete overhead
        getChar(compressed, KartControl::getCompressedSize());
        KartControl kc;
        kc.uncompress(compressed);
        Vec3 xyz = getVec3();
        btQuaternion q = getQuaternion();
        Kart *kart = world->getKart(i);
        kart->setXYZ(xyz);
        kart->setRotation(q);
    }   // for i
};   // KartUpdateMessage

