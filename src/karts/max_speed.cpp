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

#include "karts/max_speed.hpp"

#include "config/stk_config.hpp"
#include "karts/abstract_kart.hpp"
#include "karts/kart_properties.hpp"
#include "network/network_string.hpp"
#include "physics/btKart.hpp"

#include <algorithm>
#include <assert.h>
#include <cstdlib>

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
MaxSpeed::MaxSpeed(AbstractKart *kart)
{
    m_kart = kart;
    // Initialise m_add_engine_force since it might be queried before
    // update() is called.
    m_add_engine_force  = 0;
    // This can be used if command line option -N is used
    m_current_max_speed = 0;
}   // MaxSpeed

// ----------------------------------------------------------------------------
/** Reset to prepare for a restart. It just overwrites each entry with a
 *  newly constructed values, i.e. values that don't cause any slowdown
 *  or speedup.
 */
void MaxSpeed::reset()
{
    m_current_max_speed = m_kart->getKartProperties()->getEngineMaxSpeed();
    m_min_speed         = -1.0f;

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
                                float engine_force, int duration,
                                int fade_out_time)
{
    // Allow fade_out_time 0 if add_speed is set to 0.
    assert(add_speed==0.0f || fade_out_time>0.01f);
    assert(category>=MS_INCREASE_MIN && category <MS_INCREASE_MAX);

    if (add_speed < 0.0f || engine_force < 0.0f)
    {
        Log::warn("MaxSpeed::increaseMaxSpeed",
            "Negative add_speed %f or engine_force %f, ignored.",
            add_speed, engine_force);
        return;
    }

    int add_speed_i = (int)(add_speed * 1000.0f);
    if (add_speed_i > 65535)
    {
        Log::warn("MaxSpeed::increaseMaxSpeed",
            "%f add_speed too large.", add_speed);
        add_speed_i = 65535;
    }

    int engine_force_i = (int)(engine_force * 10.0f);
    if (engine_force_i > 65535)
    {
        Log::warn("MaxSpeed::increaseMaxSpeed",
            "%f engine_force too large.", engine_force);
        engine_force_i = 65535;
    }

    int16_t fade = 0;
    if (fade_out_time > 32767)
    {
        Log::warn("MaxSpeed::increaseMaxSpeed",
            "%d fade_out_time too large.", fade);
        fade = (int16_t)32767;
    }
    else
        fade = (int16_t)fade_out_time;

    int16_t dur = 0;
    if (duration > 32767)
    {
        Log::warn("MaxSpeed::increaseMaxSpeed",
            "%d duration too large.", dur);
        dur = (int16_t)32767;
    }
    else
        dur = (int16_t)duration;

    m_speed_increase[category].m_max_add_speed = (uint16_t)add_speed_i;
    m_speed_increase[category].m_duration        = dur;
    m_speed_increase[category].m_fade_out_time   = fade;
    m_speed_increase[category].m_current_speedup = add_speed;
    m_speed_increase[category].m_engine_force = (uint16_t)engine_force_i;
}   // increaseMaxSpeed

// ----------------------------------------------------------------------------
/** This adjusts the top speed using increaseMaxSpeed, but additionally
 *  causes an instant speed boost, which can be smaller than add-max-speed.
 *  (e.g. a zipper can give an instant boost of 5 m/s, but over time would
 *  allow the speed to go up by 10 m/s). Note that bullet does not restrict
 *  speed (e.g. by simulating air resistance), so without capping the speed
 *  (which is done my this object) the speed would go arbitrary high over time
 *  \param category The category for which the speed is increased.
 *  \param add_max_speed Increase of the maximum allowed speed.
 *  \param speed_boost An instant speed increase for this kart.
 *  \param engine_force Additional engine force.
 *  \param duration Duration of the increased speed.
 *  \param fade_out_time How long the maximum speed will fade out linearly.
 */
void MaxSpeed::instantSpeedIncrease(unsigned int category,
                                   float add_max_speed, float speed_boost,
                                   float engine_force, int duration,
                                   int fade_out_time)
{
    increaseMaxSpeed(category, add_max_speed, engine_force, duration,
                     fade_out_time);
    // This will result in all max speed settings updated, but no
    // changes to any slow downs since dt=0
    update(0);
    float speed = std::min(m_kart->getSpeed()+ speed_boost,
                           getCurrentMaxSpeed() );

    // If there is a min_speed defined, make sure that the kart is still
    // fast enough (otherwise e.g. on easy difficulty even with zipper
    // the speed might be too low for certain jumps).
    if(speed < m_min_speed) speed = m_min_speed;

    m_kart->getVehicle()->setMinSpeed(speed);

}   // instantSpeedIncrease

// ----------------------------------------------------------------------------
/** Handles the update of speed increase objects. The m_duration variable
 *  contains the remaining time - as long as this variable is positive
 *  the maximum speed increase applies, while when it is between
 *  -m_fade_out_time and 0, the maximum speed will linearly decrease.
 *  \param dt Time step size.
 */
void MaxSpeed::SpeedIncrease::update(int ticks)
{
    if (m_duration == std::numeric_limits<int16_t>::min())
    {
        m_current_speedup = 0;
        m_max_add_speed = 0;
        return;
    }
    m_duration -= ticks;
    // End of increased max speed reached.
    if(m_duration < -m_fade_out_time)
    {
        m_duration = std::numeric_limits<int16_t>::min();
        m_current_speedup = 0;
        m_max_add_speed = 0;
        return;
    }
    // If we are still in main max speed increase time, do nothing
    m_current_speedup = (float)m_max_add_speed / 1000.0f;
    if (m_duration > 0)
        return;

    // Now we are in the fade out period: decrease time linearly
    m_current_speedup -= std::abs(m_duration - ticks) *
        ((float)m_max_add_speed / 1000.0f) / m_fade_out_time;
}   // SpeedIncrease::update

// ----------------------------------------------------------------------------
void MaxSpeed::SpeedIncrease::saveState(BareNetworkString *buffer) const
{
    buffer->addUInt16(m_max_add_speed);
    buffer->addUInt16(m_duration);
    buffer->addUInt16(m_fade_out_time);
    buffer->addUInt16(m_engine_force);
}   // saveState

// ----------------------------------------------------------------------------
void MaxSpeed::SpeedIncrease::rewindTo(BareNetworkString *buffer,
                                       bool is_active)
{
    if(is_active)
    {
        m_max_add_speed   = buffer->getUInt16();
        m_duration        = buffer->getUInt16();
        m_fade_out_time   = buffer->getUInt16();
        m_engine_force    = buffer->getUInt16();
    }
    else   // make sure to disable this category
    {
        reset();
    }
}   // rewindTo

// ----------------------------------------------------------------------------
/** Defines a slowdown, which is in fraction of top speed.
 *  \param category The category for which the speed is increased.
 *  \param max_speed_fraction Fraction of top speed to allow only.
 *  \param fade_in_time How long till maximum speed is capped.
 *  \param duration How long the effect will lasts. The value of -1 (default)
 *         indicates that this effect stays active forever (i.e. till its
 *         value is changed to something else).
 */
void MaxSpeed::setSlowdown(unsigned int category, float max_speed_fraction,
                           int fade_in_ticks, int duration)
{
    assert(category>=MS_DECREASE_MIN && category <MS_DECREASE_MAX);
    if (max_speed_fraction < 0.0f)
    {
        Log::warn("MaxSpeed::increaseMaxSpeed",
            "Negative max_speed_fraction %f, ignored.",
            max_speed_fraction);
        return;
    }

    int max_speed_fraction_i = (int)(max_speed_fraction * 1000.0f);
    if (max_speed_fraction_i > 65535)
    {
        Log::warn("MaxSpeed::increaseMaxSpeed",
            "%f max_speed_fraction too large.", max_speed_fraction);
        max_speed_fraction_i = 65535;
    }

    int16_t fade = 0;
    if (fade_in_ticks > 32767)
    {
        Log::warn("MaxSpeed::setSlowdown",
            "%d fade_in_ticks too large.", fade);
        fade = (int16_t)32767;
    }
    else
        fade = (int16_t)fade_in_ticks;

    int16_t dur = 0;
    if (duration > 32767)
    {
        Log::warn("MaxSpeed::setSlowdown",
            "%d duration too large.", dur);
        dur = (int16_t)32767;
    }
    else
        dur = (int16_t)duration;

    m_speed_decrease[category].m_fade_in_ticks = fade;
    m_speed_decrease[category].m_duration = dur;
    m_speed_decrease[category].m_max_speed_fraction =
        (uint16_t)max_speed_fraction_i;
}   // setSlowdown

// ----------------------------------------------------------------------------
/** Handles the speed increase for a certain category.
 *  \param dt Time step size.
 */
void MaxSpeed::SpeedDecrease::update(int ticks)
{
    if(m_duration>-1)
    {
        // It's a timed slowdown
        m_duration -= ticks;
        if(m_duration<0)
        {
            m_duration           = 0;
            m_current_fraction   = 1.0f;
            m_max_speed_fraction = 1000;
            return;
        }
    }

    float diff = m_current_fraction - m_max_speed_fraction / 1000.0f;

    if (diff > 0)
    {
        if (diff * m_fade_in_ticks > ticks)
            m_current_fraction -= float(ticks) / float(m_fade_in_ticks);
        else
            m_current_fraction = m_max_speed_fraction / 1000.0f;
    }
    else
        m_current_fraction = m_max_speed_fraction / 1000.0f;
}   // SpeedDecrease::update

// ----------------------------------------------------------------------------
/** Saves the state of an (active) speed decrease category. It is not called
 *  if the speed decrease is not active.
 *  \param buffer Buffer which will store the state information.
 */
void MaxSpeed::SpeedDecrease::saveState(BareNetworkString *buffer) const
{
    buffer->addUInt16(m_max_speed_fraction);
    buffer->addFloat(m_current_fraction);
    buffer->addUInt16(m_fade_in_ticks);
    buffer->addUInt16(m_duration);
}   // saveState

// ----------------------------------------------------------------------------
/** Restores a previously saved state for an active speed decrease category.
 */
void MaxSpeed::SpeedDecrease::rewindTo(BareNetworkString *buffer,
                                       bool is_active)
{
    if(is_active)
    {
        m_max_speed_fraction = buffer->getUInt16();
        m_current_fraction   = buffer->getFloat();
        m_fade_in_ticks      = buffer->getUInt16();
        m_duration           = buffer->getUInt16();
    }
    else   // make sure it is not active
    {
        reset();
    }
}   // rewindTo

// ----------------------------------------------------------------------------
/** Returns how much increased speed time is left over in the given category.
 *  \param category Which category to report on.
 */
int MaxSpeed::getSpeedIncreaseTicksLeft(unsigned int category)
{
    return m_speed_increase[category].getTimeLeft();
}   // getSpeedIncreaseTimeLeft

// ----------------------------------------------------------------------------
/** Returns if increased speed is active in the given category.
 *  \param category Which category to report on.
 */
int MaxSpeed::isSpeedIncreaseActive(unsigned int category)
{
    return m_speed_increase[category].isActive();
}   // isSpeedIncreaseActive

// ----------------------------------------------------------------------------
/** Returns if decreased speed is active in the given category.
 *  \param category Which category to report on.
 */
int MaxSpeed::isSpeedDecreaseActive(unsigned int category)
{
    return m_speed_decrease[category].isActive();
}   // isSpeedDecreaseActive

// ----------------------------------------------------------------------------
/** Updates all speed increase and decrease objects, and determines the
 *  current maximum speed. Note that the function can be called with
 *  dt=0, in which case the maximum speed will be updated, but no
 *  change to any of the speed increase/decrease objects will be done.
 *  \param dt Time step size (dt=0 only updates the current maximum speed).
 */
void MaxSpeed::update(int ticks)
{

    // First compute the minimum max-speed fraction, which
    // determines the overall decrease of maximum speed.
    // ---------------------------------------------------
    float slowdown_factor = 1.0f;
    for(unsigned int i=MS_DECREASE_MIN; i<MS_DECREASE_MAX; i++)
    {
        SpeedDecrease &slowdown = m_speed_decrease[i];
        slowdown.update(ticks);
        slowdown_factor = std::min(slowdown_factor,
                                   slowdown.getSlowdownFraction());
    }

    m_add_engine_force  = 0;
    m_current_max_speed = m_kart->getKartProperties()->getEngineMaxSpeed();

    // Then add the speed increase from each category
    // ----------------------------------------------
    for(unsigned int i=MS_INCREASE_MIN; i<MS_INCREASE_MAX; i++)
    {
        SpeedIncrease &speedup = m_speed_increase[i];
        speedup.update(ticks);
        m_current_max_speed += speedup.getSpeedIncrease();
        m_add_engine_force  += speedup.getEngineForce();
    }
    if (getSpeedIncreaseTicksLeft(MS_INCREASE_SKIDDING) > 0 &&
        getSpeedIncreaseTicksLeft(MS_INCREASE_RED_SKIDDING) > 0)
    {
        SpeedIncrease &speedup = m_speed_increase[MS_INCREASE_SKIDDING];
        m_current_max_speed -= speedup.getSpeedIncrease();
        m_add_engine_force  -= speedup.getEngineForce();
    }

    m_current_max_speed *= slowdown_factor;

    // Then cap the current speed of the kart
    // --------------------------------------
    if(m_min_speed > 0 && m_kart->getSpeed() < m_min_speed)
    {
        m_kart->getVehicle()->setMinSpeed(m_min_speed);
    }
    else 
        m_kart->getVehicle()->setMinSpeed(0);   // no additional acceleration

    if (m_kart->isOnGround())
        m_kart->getVehicle()->setMaxSpeed(m_current_max_speed);
    else
        m_kart->getVehicle()->setMaxSpeed(9999.9f);

}   // update

// ----------------------------------------------------------------------------
/** Saves the speed data in a network string for rewind.
 *  \param buffer Pointer to the network string to store the data.
 */
void MaxSpeed::saveState(BareNetworkString *buffer) const
{
    // Save the slowdown states
    // ------------------------
    // Get the bit pattern of all active slowdowns
    uint8_t active_slowdown = 0;
    for(unsigned int i=MS_DECREASE_MIN, b=1; i<MS_DECREASE_MAX; i++, b <<=1)
    {
        // Don't bother saving terrain, this will get updated automatically
        // each frame.
        if(i==MS_DECREASE_TERRAIN) continue;
        if (m_speed_decrease[i].isActive()) 
            active_slowdown |= b;
    }
    buffer->addUInt8(active_slowdown);

    for(unsigned int i=MS_DECREASE_MIN, b=1; i<MS_DECREASE_MAX; i++, b <<= 1)
    {
        if (i == MS_DECREASE_TERRAIN)
        {
            // Handle in local state
            continue;
        }
        else if (active_slowdown & b)
            m_speed_decrease[i].saveState(buffer);
    }

    // Now save the speedup state
    // --------------------------
    // Get the bit pattern of all active speedups
    uint8_t active_speedups = 0;
    for(unsigned int i=MS_INCREASE_MIN, b=1; i<MS_INCREASE_MAX; i++, b <<= 1)
    {
        if(m_speed_increase[i].isActive())
            active_speedups |= b;
    }
    buffer->addUInt8(active_speedups);
    for(unsigned int i=MS_INCREASE_MIN, b=1; i<MS_INCREASE_MAX; i++, b <<= 1)
    {
        if(active_speedups & b)
            m_speed_increase[i].saveState(buffer);
    }

}   // saveState

// ----------------------------------------------------------------------------
/** Restore a saved state.
 *  \param buffer Saved state.
 */
void MaxSpeed::rewindTo(BareNetworkString *buffer)
{
    // Restore the slowdown states
    // ---------------------------
    // Get the bit pattern of all active slowdowns
    uint8_t active_slowdown = buffer->getUInt8();

    for(unsigned int i=MS_DECREASE_MIN, b=1; i<MS_DECREASE_MAX; i++, b <<= 1)
    {
        if (i == MS_DECREASE_TERRAIN)
        {
            // Handle in local state
            continue;
        }
        else
            m_speed_decrease[i].rewindTo(buffer, (active_slowdown & b) == b);
    }

    // Restore the speedup state
    // --------------------------
    // Get the bit pattern of all active speedups
    uint8_t active_speedups = buffer->getUInt8();
    for(unsigned int i=MS_INCREASE_MIN, b=1; i<MS_INCREASE_MAX; i++, b <<= 1)
    {
        m_speed_increase[i].rewindTo(buffer, (active_speedups & b) == b);
    }
    // Make sure to update the physics
    update(0);
}   // rewindoTo

