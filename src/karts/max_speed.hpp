//  $Id$
//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2010  Joerg Henrichs
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

/** \defgroup karts */

class Kart;

class MaxSpeed
{
public:
    /** The categories to use for increasing the speed of a kart:
     *  Increase due to zipper, slipstream, nitro, rubber band usage. */
    enum  {MS_INCREASE_MIN,
           MS_INCREASE_ZIPPER = MS_INCREASE_MIN, 
           MS_INCREASE_SLIPSTREAM, 
           MS_INCREASE_NITRO,
           MS_INCREASE_RUBBER,
           MS_INCREASE_MAX};

    /** The categories to use for decreasing the speed of a kart:
     *  Decrease due to terrain, different AI levels and end controller. */
    enum {MS_DECREASE_MIN,
          MS_DECREASE_TERRAIN = MS_DECREASE_MIN, 
          MS_DECREASE_AI, 
          MS_DECREASE_SQUASH,
          MS_DECREASE_MAX};

private:
    /** A pointer to the kart to which this speed handling object belongs. */
    Kart *m_kart;

    /** The current maximum speed. */
    float m_current_max_speed;

    // ------------------------------------------------------------------------
    /** An internal class to store and handle speed increase related data. */
    class SpeedIncrease
    {
    public:
        /** The maximum additional speed allowed. */
        float m_max_add_speed;
        /** How long this speed will apply. This is used as a timer internally,
         *  to the duration will be decreased. When the duration is <0, the
         *  fade out time starts, and duration will go down to 
         *  -m_fade_out_time before this speed increase stops. */
        float m_duration;
        /** The fadeout time. */
        float m_fade_out_time;
        /** The current max speed increase value. */
        float m_current_speedup;

        /** The constructor initialised the values with a no-increase 
         *  entry, i.e. an entry that does affect top speed at all. */
        SpeedIncrease()
        {
            m_max_add_speed   = 0;
            m_duration        = -9999999;
            m_fade_out_time   = 0;
            m_current_speedup = 0;
        }   // SpeedIncrease
        void update(float dt);
        /** Returns the current speedup for this category. */
        float getSpeedIncrease() const {return m_current_speedup;}
        /** Returns the remaining time till the fade out time starts.
         *  Note that this function will return a negative value if
         *  the fade_out time has started or this speed increase has
         *  expired. */
        float getTimeLeft() const      {return m_duration;       }
    };   // SpeedIncrease

    // ------------------------------------------------------------------------
    /** An internal class to store and handle speed decrease related data. */
    class SpeedDecrease
    {
    public:
        /** The maximum slowdown to apply. */
        float m_max_speed_fraction;
        /** How long it should take for the full slowdown to take effect. */
        float m_fade_in_time;
        /** The current slowdown fraction, taking the fade-in time 
         *  into account. */
        float m_current_fraction;

        /** The constructor initialises the data with data that won't
         *  affect top speed at all. */
        SpeedDecrease()
        {
            m_max_speed_fraction = 1.0f;
            m_fade_in_time       = 0.0f;
            m_current_fraction   = 1.0f;
        }   // SpeedDecrease
        void update(float dt);
        /** Returns the current slowdown fracftion, taking a 'fade in' 
         *  into account. */
        float getSlowdownFraction() const {return m_current_fraction;}
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
                           float duration, float fade_out_time);
    void  setSlowdown(unsigned int category, float max_speed_fraction, 
                      float fade_in_time);
    float getSpeedIncreaseTimeLeft(unsigned int category);
    void  update(float dt);
    void  reset();
    /** Returns the current maximum speed for this kart. */
    float getCurrentMaxSpeed() const { return m_current_max_speed; }

};   // MaxSpeed
#endif
