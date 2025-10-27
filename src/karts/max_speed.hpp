//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2010-2015  Joerg Henrichs
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

#ifndef HEADER_MAX_SPEED_HPP
#define HEADER_MAX_SPEED_HPP

#include "utils/types.hpp"
#include <limits>

/** \defgroup karts */

class Kart;
class BareNetworkString;

class MaxSpeed
{
friend class KartRewinder;
public:
    /** The categories to use for increasing the speed of a kart:
     *  Increase due to zipper, slipstream, nitro, rubber band,
     *  skidding usage, or electro-shield. */
    enum  {MS_INCREASE_MIN,
           MS_INCREASE_ZIPPER = MS_INCREASE_MIN,
           MS_INCREASE_GROUND_ZIPPER,
           MS_INCREASE_SLIPSTREAM,
           MS_INCREASE_NITRO,
           MS_INCREASE_RUBBER,
           MS_INCREASE_SKIDDING,
           MS_INCREASE_RED_SKIDDING,
           MS_INCREASE_PURPLE_SKIDDING,
           MS_INCREASE_ELECTRO,
           MS_INCREASE_MAX};

    /** The categories to use for decreasing the speed of a kart:
     *  Decrease due to terrain, different AI levels and end controller. */
    enum {MS_DECREASE_MIN,
          MS_DECREASE_TERRAIN = MS_DECREASE_MIN,
          MS_DECREASE_AI,
          MS_DECREASE_BUBBLE,
          MS_DECREASE_SQUASH,
          MS_DECREASE_MAX};

private:
    /** A pointer to the kart to which this speed handling object belongs. */
    Kart *m_kart;

    /** The current maximum speed. */
    float m_current_max_speed;

    /** Additional engine force, summed from all SpeedIncrease engine forces. */
    float m_add_engine_force;

    /** If >0 then the minimum speed a kart should have (used for zippers). */
    float m_min_speed;

    /** Used for display of skid particles after triggering a skid bonus.
    *  0 is no bonus, 1 is first-stage bonus, etc. */
    uint8_t m_last_triggered_skid_level;

    // ------------------------------------------------------------------------
    /** An internal class to store and handle speed increase related data. */
    class SpeedIncrease
    {
    public:
        /** The maximum additional speed allowed, 3 digits precision. */
        uint16_t m_max_add_speed;
        /** How long this speed will apply. This is used as a timer internally,
         *  to the duration will be decreased. When the duration is <0, the
         *  fade out time starts, and duration will go down to
         *  -m_fade_out_time before this speed increase stops. */
        int16_t m_duration;
        /** The fadeout time. */
        int16_t m_fade_out_time;
        /** The current max speed increase value, updated with duration. */
        float m_current_speedup;
        /** Additional engine force, 1 digit precision. */
        uint16_t m_engine_force;
        /** The constructor initialised the values with a no-increase
         *  entry, i.e. an entry that does affect top speed at all. */
        SpeedIncrease()
        {
            reset();
        }   // SpeedIncrease
        // --------------------------------------------------------------------
        /** Resets this increase category to be not active. */
        void reset()
        {
            m_max_add_speed   = 0;
            m_duration        = std::numeric_limits<int16_t>::min();
            m_fade_out_time   = 0;
            m_current_speedup = 0;
            m_engine_force    = 0;
        }   // reset
        // --------------------------------------------------------------------
        void update(int ticks);
        void saveState(BareNetworkString *buffer) const;
        void rewindTo(BareNetworkString *buffer, bool is_active);
        // --------------------------------------------------------------------
        /** Returns the current speedup for this category. */
        float getSpeedIncrease() const {return m_current_speedup;}
        // --------------------------------------------------------------------
        /** Returns the remaining time till the fade out time starts.
         *  Note that this function will return a negative value if
         *  the fade_out time has started or this speed increase has
         *  expired. */
        int getTimeLeft() const      {return m_duration;       }
        // --------------------------------------------------------------------
        /** Returns the additional engine force for this speed increase. */
        float getEngineForce() const
        {
            return m_duration > 0 ? (float)m_engine_force / 10.0f : 0;
        }   // getEngineForce
        // --------------------------------------------------------------------
        /** This allows to gracefully end a speed increase, keeping
         * the fade-out time active */
        void endSpeedIncrease() { m_duration = 0; }
        // --------------------------------------------------------------------
        /** Returns if this speed increase is active atm. */
        bool isActive() const { return m_duration > -m_fade_out_time; }
    };   // SpeedIncrease

    // ------------------------------------------------------------------------
    /** An internal class to store and handle speed decrease related data. */
    class SpeedDecrease
    {
    public:
        /** The maximum slowdown to apply, 3 digits precision. */
        uint16_t m_max_speed_fraction;
        /** The current slowdown fraction, taking the fade-in time
         *  into account. */
        float m_current_fraction;

        /** How long it should take for the full slowdown to take effect. */
        int16_t m_fade_in_ticks;

        /** How long the effect should last. A -1 as value indicates
         *  that this effect stays active till it is changed back. */
        int16_t m_duration;

        /** The constructor initialises the data with data that won't
         *  affect top speed at all. */
        SpeedDecrease()
        {
            reset();
        }   // SpeedDecrease
        // --------------------------------------------------------------------
        /** Resets the state to be inactive. */
        void reset()
        {
            m_max_speed_fraction = 1000;
            m_current_fraction   = 1.0f;
            m_fade_in_ticks      = 0;
            m_duration           = 0;
        }   //reset
        // --------------------------------------------------------------------
        void update(int ticks);
        void saveState(BareNetworkString *buffer) const;
        void rewindTo(BareNetworkString *buffer, bool is_active);
        // --------------------------------------------------------------------
        /** Returns the current slowdown fracftion, taking a 'fade in'
         *  into account. */
        float getSlowdownFraction() const        { return m_current_fraction; }
        // --------------------------------------------------------------------
        int getTimeLeft() const                          { return m_duration; }
        // --------------------------------------------------------------------
        /** Returns if this speed decrease is active atm. A duration of
         *  -1 indicates an ongoing effect. */
        bool isActive() const    { return m_duration > 0 || m_duration <= -1; }
    };   // SpeedDecrease

    // ------------------------------------------------------------------------
    /** Stores all speed decrease related information
     *  for each possible category. */
    SpeedDecrease  m_speed_decrease[MS_DECREASE_MAX];

    /** Stores all speed increase related information
     *  for each possible category. */
    SpeedIncrease  m_speed_increase[MS_INCREASE_MAX];


public:
          MaxSpeed(Kart *kart);

    void  increaseMaxSpeed(unsigned int category, float add_speed,
                           float engine_force, int duration,
                           int fade_out_time);
    void  instantSpeedIncrease(unsigned int category,
                               float add_speed, float speed_boost,
                               float engine_force, int duration,
                               int fade_out_time);
    void  setSlowdown(unsigned int category, float max_speed_fraction,
                      int fade_in_time, int duration=-1);
    int   getSpeedIncreaseTicksLeft(unsigned int category);
    int   isSpeedIncreaseActive(unsigned int category);
    int   isSpeedDecreaseActive(unsigned int category);
    void  endSpeedIncrease(unsigned int category);
    void  update(int ticks);
    void  reset(bool leave_squash = false);
    void  saveState(BareNetworkString *buffer) const;
    void  rewindTo(BareNetworkString *buffer);
    // ------------------------------------------------------------------------
    /** Sets the minimum speed a kart should have. This is used to guarantee
     *  that e.g. zippers on ramps will always fast enough for the karts to
     *  reach the other end. If set to a negative number, it will have
     *  no effect. */
    void setMinSpeed(float s) { m_min_speed = s; }
    // ------------------------------------------------------------------------
    /** Returns the current maximum speed for this kart. */
    float getCurrentMaxSpeed() const { return m_current_max_speed; }
    // --------------------------------------------------------------------
    unsigned int getLatestSkidLevel() const { return m_last_triggered_skid_level; }
    // ------------------------------------------------------------------------
    /** Returns the additional engine force. */
    float getCurrentAdditionalEngineForce() const { return m_add_engine_force;}
};   // MaxSpeed
#endif
