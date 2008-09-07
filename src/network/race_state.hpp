//  $Id: race_state.hpp 2128 2008-06-13 00:53:52Z cosmosninja $
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

#ifndef HEADER_RACE_STATE_H
#define HEADER_RACE_STATE_H

#include <vector>

#include "network/message.hpp"
#include "network/herring_info.hpp"
#include "network/flyable_info.hpp"
#include "kart_control.hpp"
#include "herring.hpp"
#include "flyable.hpp"
#include "race_manager.hpp"
#include "kart.hpp"

/** This class stores the state information of a (single) race, e.g. the 
    position and orientation of karts, collisions that have happened etc.
    It is used for the network version to update the clients with the
    'official' state information from the server.
    */
class RaceState : public Message
{
private:

    /** Updates about collected herrings. */
    std::vector<HerringInfo> m_herring_info;
    /** Updates about existing flyables. */
    std::vector<FlyableInfo> m_flyable_info;
    /** Stores the controls of each kart at the beginning of its update(). */
    std::vector<KartControl> m_kart_controls;

    public:
        /** Initialise the global race state. */
        RaceState() : Message(MT_RACE_STATE) 
        {
            m_kart_controls.resize(race_manager->getNumKarts());
        }   // RaceState()
        // --------------------------------------------------------------------
        void herringCollected(int kartid, int herring_id, char add_info=-1)
        {
            m_herring_info.push_back(HerringInfo(kartid, herring_id, add_info));
        }
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
        void storeKartControls(const Kart& kart) 
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

