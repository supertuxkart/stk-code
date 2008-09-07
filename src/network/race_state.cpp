//  $Id: race_state.cpp 2128 2008-06-13 00:53:52Z cosmosninja $
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

#include "world.hpp"
#include "network/network_manager.hpp"
#include "network/race_state.hpp"
#include "projectile_manager.hpp"

RaceState *race_state=NULL;

// ----------------------------------------------------------------------------
void RaceState::serialise()
{
    // First compute the overall size needed
    // =====================================
    int len = 0;

    // 1. Add all kart information
    // ---------------------------
    unsigned int num_karts = world->getCurrentNumKarts();
    KartControl c;
    // Send the number of karts and for each kart the compressed 
    // control structure, xyz,hpr, and speed (which is necessary to
    // display the speed, and e.g. to determine when a parachute is detached)
    len += 1 + num_karts*(KartControl::getCompressedSize() 
                          + getVec3Length()+getQuaternionLength()
                          + getFloatLength()) ;

    // 2. Add information about eaten herrings
    // ---------------------------------------
    
    // We can't use sizeof() here, since the data structures might be padded
    len += 1 + m_herring_info.size()* HerringInfo::getLength();

    // 3. Add the data about new flyables
    // ----------------------------------
    len += 1 + m_new_flyable.size() * 2;

    if(projectile_manager->getNumProjectiles()>0)
        printf("rocket\n");
    // 4. Add rocket positions
    // -----------------------
    len += 2+projectile_manager->getNumProjectiles()*FlyableInfo::getLength();

    // Now add the data
    // ================
    allocate(len);

    // 1. Kart positions
    // -----------------
    addChar(num_karts);
    assert(KartControl::getCompressedSize()<=9);
    for(unsigned int i=0; i<num_karts; i++)
    {
        const Kart* kart=world->getKart(i);
        const KartControl& kc=kart->getControls();
        char compressed[9];         // avoid the new/delete overhead
        kc.compress(compressed);
        addCharArray(compressed, KartControl::getCompressedSize());
        addVec3(kart->getXYZ());
        addQuaternion(kart->getRotation());
        addFloat(kart->getSpeed());
    }   // for i

    // 2. Eaten herrings
    // -----------------
    addChar(m_herring_info.size());
    for(unsigned int i=0; i<m_herring_info.size(); i++)
    {
        m_herring_info[i].serialise(this);
    }

    // 3. New projectiles
    // ------------------
    addChar(m_new_flyable.size());
    for(unsigned int i=0; i<m_new_flyable.size(); i++)
    {
        addChar(m_new_flyable[i].m_type);
        addChar(m_new_flyable[i].m_kart_id);
    }

    // 4. Projectiles
    // --------------
    addShort(m_flyable_info.size());
    // The exploded flag could be compressed by combining 8 bits into one byte
    for(unsigned int i=0; i<m_flyable_info.size(); i++)
    {
        m_flyable_info[i].serialise(this);
    }

}   // serialise

// ----------------------------------------------------------------------------
void RaceState::clear()
{
    m_herring_info.clear();
    m_flyable_info.clear();
}   // clear

// ----------------------------------------------------------------------------
/** Unserialises a race state message.
 *  This function unserialises the message, and updates the state of the race
 *  simulation appropriately.
 */
void RaceState::receive(ENetPacket *pkt)
{
    Message::receive(pkt, MT_RACE_STATE);

    // 1. Kart information
    // -------------------
    unsigned int num_karts = getChar();
    for(unsigned int i=0; i<num_karts; i++)
    {
        assert(KartControl::getCompressedSize()<=9);
        char compressed[9];   // avoid new/delete overhead
        getCharArray(compressed, KartControl::getCompressedSize());
        KartControl kc;
        kc.uncompress(compressed);
        // Currently not used!
        Vec3 xyz       = getVec3();
        btQuaternion q = getQuaternion();
        Kart *kart     = world->getKart(i);
        kart->setXYZ(xyz);
        kart->setRotation(q);
        kart->setSpeed(getFloat());
    }   // for i

    // Eaten herrings
    // --------------
    unsigned short num_herrings=getChar();
    for(unsigned int i=0; i<num_herrings; i++)
    {
        HerringInfo hi(this);
        if(hi.m_herring_id==-1)     // Rescue triggered
            world->getKart(hi.m_kart_id)->forceRescue();
        else
            herring_manager->eatenHerring(hi.m_herring_id,
                                          world->getKart(hi.m_kart_id),
                                          hi.m_add_info);
    }
    // 3. New flyables
    // ---------------
    unsigned int new_fl = getChar();
    for(unsigned int i=0; i<new_fl; i++)
    {
        char type          = getChar();
        char world_kart_id = getChar();
        projectile_manager->newProjectile(world->getKart(world_kart_id),
                                          (CollectableType)type);
    }

    // 4. Projectiles
    // --------------
    unsigned short num_flyables = getShort();
    m_flyable_info.clear();
    m_flyable_info.resize(num_flyables);
    for(unsigned short i=0; i<projectile_manager->getNumProjectiles(); i++)
    {
        FlyableInfo f(this);
        m_flyable_info[i] = f;
    }
    clear();  // free message bugger

}   // receive
// ----------------------------------------------------------------------------
