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
    len += 1 + num_karts*(KartControl::getLength() 
                          + getVec3Length()+getQuaternionLength()
                          + getFloatLength()) ;

    // 2. Add information about eaten herrings
    // ---------------------------------------
    len += 1 + m_herring_info.size()* HerringInfo::getLength();

    // 3. Add rocket positions
    // -----------------------
    len += 2 + m_flyable_info.size()*FlyableInfo::getLength();

    // 4. Add collisions
    // =================
    len += 1 + m_collision_info.size()*getCharLength();

    // Now add the data
    // ================
    allocate(len);

    // 1. Kart positions
    // -----------------
    addChar(num_karts);
    for(unsigned int i=0; i<num_karts; i++)
    {
        const Kart* kart=world->getKart(i);
        m_kart_controls[i].serialise(this);
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

    // 3. Projectiles
    // --------------
    addShort(m_flyable_info.size());
    for(unsigned int i=0; i<m_flyable_info.size(); i++)
    {
        m_flyable_info[i].serialise(this);
    }

    // 4. Collisions
    // -------------
    addChar(m_collision_info.size());
    for(unsigned int i=0; i<m_collision_info.size(); i++)
    {
        addChar(m_collision_info[i]);
    }
    m_collision_info.clear();

}   // serialise

// ----------------------------------------------------------------------------
void RaceState::clear()
{
    m_herring_info.clear();
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
        KartControl kc(this);
        // Currently not used!
        Vec3 xyz       = getVec3();
        btQuaternion q = getQuaternion();
        Kart *kart     = world->getKart(i);
        // Firing needs to be done from here to guarantee that any potential
        // new rockets are created before the update for the rockets is handled
        if(kc.fire)
            kart->getCollectable()->use();
        kart->setXYZ(xyz);
        kart->setRotation(q);
        kart->setSpeed(getFloat());
    }   // for i

    // 2. Eaten herrings
    // -----------------
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

    // 3. Projectiles
    // --------------
    unsigned short num_flyables = getShort();
    m_flyable_info.clear();
    m_flyable_info.resize(num_flyables);
    for(unsigned short i=0; i<num_flyables; i++)
    {
        FlyableInfo f(this);
        m_flyable_info[i] = f;
    }

    // 4. Collisions
    // -------------
    unsigned int num_collisions = getChar();
    // Collisions are stored as pairs, so handle a pair at a time
    for(unsigned int i=0; i<num_collisions; i+=2)
    {
        signed char kart_id1 = getChar();
        signed char kart_id2 = getChar();
        if(kart_id2==-1)
        {   // kart - track collision
            world->getKart(kart_id1)->crashed(NULL);
        }
        else
        {
            world->getPhysics()->KartKartCollision(world->getKart(kart_id1),
                                                   world->getKart(kart_id2));
        }
    }   // for(i=0; i<num_collisions; i+=2)
    clear();  // free message buffer

}   // receive
// ----------------------------------------------------------------------------
