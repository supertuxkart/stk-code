//  $Id$
//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2006 Joerg Henrichs
//
//  This program is free software; you can redistribute it and/or
//  modify it under the terms of the GNU General Public License
//  as published by the Free Software Foundation; either version 2
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

#ifndef HEADER_STKCONFIG_H
#define HEADER_STKCONFIG_H

#include "kart_properties.hpp"
#include "lisp/lisp.hpp"

class STKConfig : public KartProperties
{
public:
    float m_anvil_weight;            // Additional kart weight if anvil is attached
    float m_anvil_speed_factor;      // To decrease speed once when attached
    float m_parachute_friction;      // Increased air friction when parachute
    float m_magnet_range_sq;         // Squared range for magnets
    float m_magnet_min_range_sq;     // Squared minimum range for magnets
    float m_magnet_time;             // time a magnet is active
    float m_jump_impulse;            // percentage of gravity when jumping
    float m_air_res_reduce[3];       // air resistance reduction for the three levels
    float m_parachute_time;          // time a parachute is active
    float m_parachute_time_other;    // time a parachute attached to other karts is active
    float m_bomb_time;               // time before a bomb explodes
    float m_bomb_time_increase;      // time added to bomb timer when it's passed on
    float m_anvil_time;              // time an anvil is active
    float m_max_road_distance;       // max distance from road to be still ON road
    float m_shortcut_segments;       // skipping more than this number of segments is
                                     // considered to be a shortcut
    float m_explosion_impulse;       // impulse affecting each non-hit kart
    int   m_max_karts;               // maximum number of karts
    int   m_grid_order;              // whether grand prix grid is in point order or reverse point order

    STKConfig() : KartProperties() {};
    void init_defaults    ();
    void getAllData       (const lisp::Lisp* lisp);
    void load             (const std::string filename);
}
;   // STKConfig

extern STKConfig* stk_config;
#endif
