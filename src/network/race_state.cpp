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

#include "network/race_state.hpp"

#include "items/item_manager.hpp"
#include "items/powerup.hpp"
#include "items/projectile_manager.hpp"
#include "karts/rescue_animation.hpp"
#include "modes/world.hpp"
#include "network/network_manager.hpp"
#include "physics/physics.hpp"

RaceState *race_state=NULL;

// ----------------------------------------------------------------------------
void RaceState::serialise()
{
    // First compute the overall size needed
    // =====================================
    int len = 0;

    // 1. Add all kart information
    // ---------------------------
    unsigned int num_karts = World::getWorld()->getCurrentNumKarts();
    KartControl c;
    // Send the number of karts and for each kart the compressed 
    // control structure, xyz,hpr, and speed (which is necessary to
    // display the speed, and e.g. to determine when a parachute is detached)
    len += 1 + num_karts*(KartControl::getLength() 
                          + getVec3Length()+getQuaternionLength()
                          + getFloatLength()) ;

    // 2. Add information about collected items
    // ---------------------------------------
    len += 1 + m_item_info.size()* ItemInfo::getLength();

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
    World *world = World::getWorld();
    for(unsigned int i=0; i<num_karts; i++)
    {
        const AbstractKart* kart = world->getKart(i);
        m_kart_controls[i].serialise(this);
        addVec3(kart->getXYZ());
        addQuaternion(kart->getRotation());
        addFloat(kart->getSpeed());
    }   // for i

    // 2. Collected items
    // -----------------
    addChar(m_item_info.size());
    for(unsigned int i=0; i<m_item_info.size(); i++)
    {
        m_item_info[i].serialise(this);
    }

    // 3. Projectiles
    // --------------
    addShort(m_flyable_info.size());
    for(unsigned int i=0; i<(unsigned int)m_flyable_info.size(); i++)
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
    m_item_info.clear();
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
    World *world = World::getWorld();
    for(unsigned int i=0; i<num_karts; i++)
    {
        KartControl kc(this);
        // Currently not used!
        Vec3 xyz       = getVec3();
        btQuaternion q = getQuaternion();
        AbstractKart *kart     = world->getKart(i);
        // Firing needs to be done from here to guarantee that any potential
        // new rockets are created before the update for the rockets is handled
        if(kc.m_fire)
            kart->getPowerup()->use();
        kart->setXYZ(xyz);
        kart->setRotation(q);
        kart->setSpeed(getFloat());
    }   // for i

    // 2. Collected Items
    // -----------------
    unsigned short num_items=getChar();
    for(unsigned int i=0; i<num_items; i++)
    {
        ItemInfo hi(this);
        if(hi.m_item_id==-1)     // Rescue triggered
            new RescueAnimation(world->getKart(hi.m_kart_id));
        else
            ItemManager::get()->collectedItem(hi.m_item_id,
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
            // FIXME: KartKartCollision now takes information about the 
            // collision points. This either needs to be added as the third
            // parameter, or perhaps the outcome of the collision (the 
            // impulse) could be added.
            world->getPhysics()->KartKartCollision(
                world->getKart(kart_id1), Vec3(0,0,0),
                world->getKart(kart_id2), Vec3(0,0,0));
        }
    }   // for(i=0; i<num_collisions; i+=2)
    clear();  // free message buffer

}   // receive
// ----------------------------------------------------------------------------
