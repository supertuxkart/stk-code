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

#ifndef HEADER_RACE_STATE_HPP
#define HEADER_RACE_STATE_HPP

#include <vector>

#include "items/flyable.hpp"
#include "items/item.hpp"
#include "karts/abstract_kart.hpp"
#include "karts/controller/kart_control.hpp"
#include "modes/world.hpp"
#include "network/flyable_info.hpp"
#include "network/item_info.hpp"
#include "network/message.hpp"
#include "utils/aligned_array.hpp"

/** This class stores the state information of a (single) race, e.g. the 
    position and orientation of karts, collisions that have happened etc.
    It is used for the network version to update the clients with the
    'official' state information from the server.
    */
class RaceState : public Message
{
private:

    /** Updates about collected items. */
    std::vector<ItemInfo> m_item_info;
    /** Updates about existing flyables. */
    AlignedArray<FlyableInfo> m_flyable_info;
    /** Stores the controls of each kart at the beginning of its update(). */
    std::vector<KartControl> m_kart_controls;
    /** Collision information. This vector stores information about which
     *  kart collided with which kart or track (kartid=-1)                 */
    std::vector<signed char> m_collision_info;
        
    public:
        /** Initialise the global race state. */
        RaceState() : Message(MT_RACE_STATE) 
        {
            m_kart_controls.resize(World::getWorld()->getNumKarts());
        }   // RaceState()
        // --------------------------------------------------------------------
        void itemCollected(int kartid, int item_id, char add_info=-1)
        {
            m_item_info.push_back(ItemInfo(kartid, item_id, add_info));
        }   // itemCollected
        // --------------------------------------------------------------------
        /** Collects information about collision in which at least one kart was
         *  involved. Other collision (e.g. projectiles, moving physics) are
         *  not needed on the client, so it's not stored at all. If a kart
         *  track collision happens, the second kart id is -1 (necessary to 
         *  play back sound effects). A simple int vector is used to store the 
         *  pair of collision, so the first collision is using the index 0 and
         *  1; the second one 2 and 3 etc.
         *  \param kartId1 World id of the kart involved in the collision.
         *  \param kartId2 World id of the 2nd kart involved in the collision,
         *                 or -1 if it's the track (which is the default).
         */
        void addCollision(signed char kartId1, signed char kartId2=-1)
        {
            m_collision_info.push_back(kartId1);
            m_collision_info.push_back(kartId2);
        }   // addCollision
        // --------------------------------------------------------------------
        void setNumFlyables(int n) { m_flyable_info.resize(n); }
        // --------------------------------------------------------------------
        void setFlyableInfo(int n, const FlyableInfo& fi)
        {
            m_flyable_info[n] = fi;
        }
        // --------------------------------------------------------------------
        /** Stores the current kart control (at the time kart->update() is 
         *  called. This allows modifications of kart->m_control during the
         *  update (e.g. see in kart::update() how firing is handled).
         */
        void storeKartControls(const AbstractKart& kart) 
        {
            m_kart_controls[kart.getWorldKartId()] = kart.getControls();
        }   // storeKartControls
        // --------------------------------------------------------------------
        void serialise();
        void receive(ENetPacket *pkt);
        void clear();      // Removes all currently stored information
        unsigned int getNumFlyables() const {return m_flyable_info.size(); }
        const FlyableInfo 
                    &getFlyable(unsigned int i) const {return m_flyable_info[i];}
    };   // RaceState

extern RaceState *race_state;

#endif

