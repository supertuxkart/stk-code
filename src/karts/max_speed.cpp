//  $Id: max_speed.cpp 6306 2010-10-18 06:04:05Z hikerstk $
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

#include "karts/max_speed.hpp"

#include <algorithm>
#include <assert.h>

#include "karts/kart.hpp"

/** This class handles maximum speed for karts. Several factors can influence
 *  the maximum speed a kart can drive, some will decrease the maximum speed,
 *  some will increase the maximum speed. 
 *  Slowdowns are specified in fraction of the (kart specific) maximum speed 
 *  of that kart. The following categories are defined:
 *  - terrain-specific slow downs
 *  - AI related slow down (low level AIs might drive slower than player)
 *  - end controller related AI (end controller drives slower)
 *  The largest slowdown of all those factors is applied to the maximum
 *  speed of the kart.
 *  Increase of maximum speed is given absolute, i.e. in m/s. The following
 *  circumstances can increase the maximum speed:
 *  - Use of a zipper
 *  - Use of sliptstream
 *  - Use of nitro
 *  The speed increases for all those are added after applying the maximum
 *  slowdown fraction.
 *  At the end the maximum is capped by a value specified in stk_config
 *  (to avoid issues with physics etc).
*/
MaxSpeed::MaxSpeed(Kart *kart)
{
    m_kart = kart;
}   // MaxSpeed

// ----------------------------------------------------------------------------
/** Reset to prepare for a restart. It just overwrites each entry with a 
 *  newly constructed values, i.e. values that don't cause any slowdown
 *  or speedup.
 */
void MaxSpeed::reset()
{
    m_current_max_speed = m_kart->getKartProperties()->getMaxSpeed();
    for(unsigned int i=MS_DECREASE_MIN; i<MS_DECREASE_MAX; i++)
    {
        SpeedDecrease sd;
        m_speed_decrease[i] = sd;
    }

    // Then add the speed increase from each category
    // ----------------------------------------------
    for(unsigned int i=MS_INCREASE_MIN; i<MS_INCREASE_MAX; i++)
    {
        SpeedIncrease si;
        m_speed_increase[i] = si;
    }
}   // reset

// ----------------------------------------------------------------------------
/** Sets an increased maximum speed for a category.
 *  \param category The category for which to set the higher maximum speed.
 *  \param add_speed How much speed (in m/s) is added to the maximum speed.
 *  \param duration How long the speed increase will last.
 *  \param fade_out_time How long the maximum speed will fade out linearly.
 */
void MaxSpeed::increaseMaxSpeed(unsigned int category, float add_speed,
                                float duration, float fade_out_time)
{
    assert(category>=MS_INCREASE_MIN && category <MS_INCREASE_MAX);
    m_speed_increase[category].m_duration        = duration;
    m_speed_increase[category].m_fade_out_time   = fade_out_time;
    m_speed_increase[category].m_max_add_speed   = add_speed;
    m_speed_increase[category].m_current_speedup = add_speed;
}   // increaseMaxSpeed

// ----------------------------------------------------------------------------
/** Handles the update of speed increase objects. The m_duration variable
 *  contains the remaining time - as long as this variable is positibe,
 *  the maximum speed increase applies, while when it is between
 *  -m_fade_out_time and 0, the maximum speed will linearly decrease.
 *  \param dt Time step size.
 */
void MaxSpeed::SpeedIncrease::update(float dt)
{
    m_duration -= dt;
    // ENd of increased max speed reached.
    if(m_duration < -m_fade_out_time)
    {
        m_current_speedup = 0;
        return;
    }
    // If we are still in main max speed increase time, do nothing
    if(m_duration >0) return;

    // Now we are in the fade out period: decrease time linearly
    m_current_speedup -= dt*m_max_add_speed/m_fade_out_time;
}   // SpeedIncrease::update

// ----------------------------------------------------------------------------
void MaxSpeed::setSlowdown(unsigned int category, float max_speed_fraction, 
                           float fade_in_time)
{
    assert(category>=MS_DECREASE_MIN && category <MS_DECREASE_MAX);
    m_speed_decrease[category].m_max_speed_fraction = max_speed_fraction;
    m_speed_decrease[category].m_fade_in_time       = fade_in_time;
}   // setSlowdown

// ----------------------------------------------------------------------------
void MaxSpeed::SpeedDecrease::update(float dt)
{
    float diff = m_current_fraction - m_max_speed_fraction;
    if(diff > 0)
    {
        if (diff * m_fade_in_time > dt)
            m_current_fraction -= dt/m_fade_in_time;
        else
            m_current_fraction = m_max_speed_fraction;
    }
    else
        m_current_fraction = m_max_speed_fraction;
}   // SpeedDecrease::update

// ----------------------------------------------------------------------------
float MaxSpeed::getSpeedIncreaseTimeLeft(unsigned int category)
{
    return m_speed_increase[category].getTimeLeft();
}   // getSpeedIncreaseTimeLeft

// ----------------------------------------------------------------------------
void MaxSpeed::update(float dt)
{

    // First comput the minimum max-speed fraction, which 
    // determines the overall decrease of maximum speed.
    // ---------------------------------------------------
    float f = 1.0f;
    for(unsigned int i=MS_DECREASE_MIN; i<MS_DECREASE_MAX; i++)
    {
        SpeedDecrease &slowdown = m_speed_decrease[i];
        slowdown.update(dt);
        f = std::min(f, slowdown.getSlowdownFraction());
    }
    m_current_max_speed = m_kart->getKartProperties()->getMaxSpeed() * f;

    // Then add the speed increase from each category
    // ----------------------------------------------
    for(unsigned int i=MS_INCREASE_MIN; i<MS_INCREASE_MAX; i++)
    {
        SpeedIncrease &speedup = m_speed_increase[i];
        speedup.update(dt);
        m_current_max_speed += speedup.getSpeedIncrease();
    }

    // Then cap the current speed of the kart
    // --------------------------------------
    m_kart->capSpeed(m_current_max_speed);

}   // update

// ----------------------------------------------------------------------------
