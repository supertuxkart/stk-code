//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2012-2013  Joerg Henrichs
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

#ifndef HEADER_SKIDDING_PROPERTIES_HPP
#define HEADER_SKIDDING_PROPERTIES_HPP

#include "utils/leak_check.hpp"
#include "utils/no_copy.hpp"

class Kart;
class XMLNode;

#include <string>
#include <vector>

/** A simple class that stores all skidding related properties. It acts as
 *  interface between kart_properties and Skidding (to avoid either passing
 *  very many individual variables, or making KartProperties a dependency
 *  of Skidding).
 * \ingroup karts
 */

class SkiddingProperties
{
public:
    //LEAK_CHECK();
protected:
    /** Skidding is multiplied by this when skidding
    *  to increase to m_skid_increase. */
    float m_skid_increase;

    /** Skidding is multiplied by this when not skidding to decrease to 1.0. */
    float m_skid_decrease;

    /** How long it takes for visual skid to reach maximum. */
    float m_skid_visual_time;

    /** How long it takes for the physical and graphical bodies to be
     *  in sync again after a skid. */
    float m_skid_revert_visual_time;

    /** Time till maximum skidding is reached. */
    float m_time_till_max_skid;

    /** Maximal increase of steering when skidding. */
    float m_skid_max;

    /** Additional rotation of 3d model when skidding. */
    float m_skid_visual;

    /** Time for a small physical when skidding starts. */
    float m_physical_jump_time;

    /** Time for a small graphics-only jump when skidding starts. */
    float m_graphical_jump_time;

    /** This factor is used to determine how much the chassis of a kart
    *  should rotate to match the graphical view. A factor of 1 is
    *  identical, a smaller factor will rotate the kart less (which might
    *  feel better). */
    float m_post_skid_rotate_factor;

    /*** Minimum speed a kart must have before it can skid. */
    float m_min_skid_speed;

    /** Time of skidding before you get a bonus boost. It's possible to
    *  define more than one time, i.e. longer skidding gives more bonus. */
    std::vector<float> m_skid_time_till_bonus;

    /** How much additional speed a kart gets when skidding. It's possible to
    *  define more than one speed, i.e. longer skidding gives more bonus. */
    std::vector<float> m_skid_bonus_speed;

    /** How long the bonus will last. It's possible to define more than one
    *   time, i.e. longer skidding gives more bonus. */
    std::vector<float> m_skid_bonus_time;

    /** Additional force accelerating the kart (in addition to the immediate
     *  speed bonus). Without this force turning to correct the direction
     *  after skidding will use up nearly all of the additional speed (turning
     *  reduces the forward engine impulse) */
    std::vector<float> m_skid_bonus_force;

    /** A factor is used to reduce the amount of steering while skidding. This
     *  is the minimum factor used (i.e. resulting in the largest turn
     *  radius). */
    float m_skid_reduce_turn_min;

    /** A factor is used to reduce the amount of steering while skidding. This
     *  is the maximum factor used (i.e. resulting in the smallest turn
     *  radius). */
    float m_skid_reduce_turn_max;


    /** Kart leaves skid marks. */
    bool  m_has_skidmarks;

    /** Used to check that all values are defined in the xml file. */
    static float UNDEFINED;

public:

         SkiddingProperties();
    void load(const XMLNode *skid_node);
    void copyFrom(const SkiddingProperties *destination);
    void checkAllSet(const std::string &filename) const;
    // ------------------------------------------------------------------------
    /** Returns if the kart leaves skidmarks or not. */
    bool hasSkidmarks() const { return m_has_skidmarks; }

    // ------------------------------------------------------------------------
    /** Returns the maximum factor by which the steering angle
     *  can be increased. */
    float getMaxSkid() const {return m_skid_max; }

    // ------------------------------------------------------------------------
    /** Returns additional rotation of 3d model when skidding. */
    float getSkidVisual             () const {return m_skid_visual;           }

    // ------------------------------------------------------------------------
    /** Returns the time for the visual skid to reach maximum. */
    float getSkidVisualTime         () const {return m_skid_visual_time;      }

    // ------------------------------------------------------------------------
    /** Returns a factor to be used to determine how much the chassis of a
     *  kart should rotate to match the graphical view. A factor of 1 is
     *  identical, a smaller factor will rotate the kart less (which might
     *  feel better). */
    float getPostSkidRotateFactor  () const {return m_post_skid_rotate_factor;}

    // ------------------------------------------------------------------------
    /** Returns the factor by which to recude the amount of steering while
        skidding. */
    float getSkidReduceTurnMin     () const { return m_skid_reduce_turn_min;  }
    // ------------------------------------------------------------------------
    float getSkidReduceTurnMax     () const { return m_skid_reduce_turn_max;  }
    // ------------------------------------------------------------------------
    /** Returns how many boni are defined for this kart. */
    int getNumberOfBonusTimes() const      { return (int) m_skid_bonus_time.size(); }
    // ------------------------------------------------------------------------
    /** Returns how long a kart must skid in order to reach the specified
     *  bonus level.
     *  param n Bonus level (0<=n<m_skid_bonus_time.size())
     */
    float getTimeTillBonus(unsigned int n) const
                                          { return m_skid_time_till_bonus[n]; }
    // ------------------------------------------------------------------------


};   // SkiddingProperties


#endif

/* EOF */

