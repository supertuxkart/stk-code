//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2012  Joerg Henrichs
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

#ifndef HEADER_SKIDDING_HPP
#define HEADER_SKIDDING_HPP

#include "skidding_properties.hpp"
#include "utils/leak_check.hpp"
#include "utils/no_copy.hpp"

class Kart;

#include <vector>

/**
 * \ingroup karts
 */

class Skidding : public SkiddingProperties
{
public:
    LEAK_CHECK();
private:
    /** Accumulated skidding factor. */
    float m_skid_factor;

    /** Keeps track on how long a kart has been skidding, in order to
     *  trigger the skidding bonus. */
    float m_skid_time;

    /** SKID_OLD: old skidding, will be removed. */
    /** SKID_NONE: Kart is currently not skidding.
     *  SKID_ACCUMULATE_LEFT: Kart is skidding to the left and accumulating
     *             for bonus.
     *  SKID_ACCUMULATE_RIGHT: Similar for turning right
     *  SKID_SHOW_GFX: Shows the gfx, while the bonus is actibe. */
    enum  {SKID_OLD, SKID_NONE, SKID_ACCUMULATE_LEFT, SKID_ACCUMULATE_RIGHT,
           SKID_SHOW_GFX} 
          m_skid_state;

    /** A read-only pointer to the kart's properties. */
    Kart *m_kart;

    unsigned int Skidding::getSkidBonus(float *bonus_time, 
                                        float *bonus_speed) const;

public:
           Skidding(Kart *kart, const SkiddingProperties *sp);
      void reset();
      void update(float dt, bool is_on_ground, float steer,
                  bool skidding);
      float getVisualSkidOffset() const;

      // ----------------------------------------------------------------------
      /** Returns the current skid factor in [1, skid_max_for_this_kart]. */
      float getSkidFactor() const { return m_skid_factor; }
      // ----------------------------------------------------------------------
      /** Returns true if the kart is skidding. */
      bool isSkidding() const { return m_skid_factor>1.0f; }

};   // Skidding


#endif

/* EOF */

