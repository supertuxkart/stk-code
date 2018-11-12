//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2012-2015  Joerg Henrichs
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

#include "karts/controller/kart_control.hpp"
#include "utils/leak_check.hpp"
#include "utils/no_copy.hpp"
#include "utils/types.hpp"

class BareNetworkString;
class Kart;
class ShowCurve;

#include <vector>

/**
 * \ingroup karts
 */

#undef SKID_DEBUG

class Skidding
{
friend class KartRewinder;
public:
    LEAK_CHECK();
private:
    /** This is m_skid_reduce_turn_max - m_skid_reduce_turn_min. */
    float m_skid_reduce_turn_delta;

    /** This is the actual steering (in fraction of max-steering-angle)
     *  to be used by this kart. */
    float m_real_steering;

    /** An additional rotation (heading) of the kart while skidding. This
     *  is only a graphical effect, the actual physics body is not rotated. */
    float m_visual_rotation;

    /** Accumulated skidding factor. */
    float m_skid_factor;

    /** Keeps track on how long a kart has been skidding, in order to
     *  trigger the skidding bonus. */
    uint16_t m_skid_time;

    /** True if the kart has skidded long enough to get a skid bonus if it
     *  stopped skidding now. */
    bool m_skid_bonus_ready;

    /** Set to >0 when a graphical jump is to be done. */
    float m_remaining_jump_time;

    float m_graphical_remaining_jump_time;

    float m_prev_visual_rotation;

    float m_smoothing_time;

    float m_smoothing_dt;

    int m_skid_bonus_end_ticks;

public:
    /** SKID_NONE: Kart is currently not skidding.
     *  SKID_ACCUMULATE_LEFT: Kart is skidding to the left and accumulating
     *             for bonus.
     *  SKID_ACCUMULATE_RIGHT: Similar for turning right
     *  SKID_SHOW_GFX_LEFT: Shows the gfx, while the bonus is active,
     *             and the kart was turning left.
     *  SKID_SHOW_GFX_RIGHT: Similar for turning right.
     *  SKID_BREAK: Skidding was stopped without bonus */
    enum SkidState { SKID_NONE, SKID_ACCUMULATE_LEFT, SKID_ACCUMULATE_RIGHT,
                     SKID_SHOW_GFX_LEFT, SKID_SHOW_GFX_RIGHT, SKID_BREAK} ;

private:
    /** The current skidding state. */
    SkidState m_skid_state;

    /** A read-only pointer to the kart's properties. */
    Kart *m_kart;

#ifdef SKID_DEBUG
    ShowCurve *m_predicted_curve;
    ShowCurve *m_actual_curve;
#endif

    unsigned int getSkidBonus(float *bonus_time, float *bonus_speed,
                              float *bonus_force) const;
    float updateSteering(float steer, int ticks);
public:
         Skidding(Kart *kart);
        ~Skidding();
    void reset();
    float updateGraphics(float dt);
    void update(int dt, bool is_on_ground, float steer,
                KartControl::SkidControl skidding);
    void saveState(BareNetworkString *buffer);
    void rewindTo(BareNetworkString *buffer);
    // ------------------------------------------------------------------------
    /** Determines how much the graphics model of the kart should be rotated
     *  additionally (for skidding), depending on how long the kart has been
     *  skidding etc. */
    float getVisualSkidRotation() const
    {
        if (m_smoothing_dt >= 0.0f)
        {
            return m_smoothing_dt * m_visual_rotation +
                (1.0f - m_smoothing_dt) * m_prev_visual_rotation;
        }
        return m_visual_rotation;
    }
    // ------------------------------------------------------------------------
    /** Returns the current skid factor in [1, skid_max_for_this_kart]. */
    float getSkidFactor() const { return m_skid_factor; }
    // ------------------------------------------------------------------------
    /** Returns true if the kart is skidding. */
    bool isSkidding() const { return m_skid_factor>1.0f; }
    // ------------------------------------------------------------------------
    /** Returns the steering fraction to be used by the physics. This is
     *  a fraction of the maximum steering angle ( so in [-1, 1]). */
    float getSteeringFraction() { return m_real_steering; }
    // ------------------------------------------------------------------------
    /** Returns the skidding state. */
    SkidState getSkidState() const { return m_skid_state; }
    // ------------------------------------------------------------------------
    float getSteeringWhenSkidding(float steering) const;
    // ------------------------------------------------------------------------
    /** Returns if the kart has skidded long enough to get a skid bonus if it
     *  stopped skidding now. This function returns false if the kart is
     *  actually using the skid bonus. */
    bool getSkidBonusReady() const { return m_skid_bonus_ready; }
    // ------------------------------------------------------------------------
    bool isJumping() const { return m_graphical_remaining_jump_time > 0;  }
    // ------------------------------------------------------------------------
    void prepareSmoothing();
    // ------------------------------------------------------------------------
    void checkSmoothing();

};   // Skidding


#endif

/* EOF */

