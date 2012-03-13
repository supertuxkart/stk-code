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

#include "karts/skidding.hpp"

#include "karts/kart.hpp"
#include "karts/kart_gfx.hpp"
#include "karts/kart_properties.hpp"
#include "physics/btKart.hpp"

/** Constructor of the skidding object.
 */
Skidding::Skidding(Kart *kart, const SkiddingProperties *sp)
{
    m_kart = kart;
    copyFrom(sp);
    m_skid_reduce_turn_delta = m_skid_reduce_turn_max - m_skid_reduce_turn_min;
    reset();
}   // Skidding

// ----------------------------------------------------------------------------
/** Resets all skidding related values.
 */
void Skidding::reset()
{
    m_skid_time     = 0.0f;
    m_skid_state    = m_skid_visual_time<=0 ? SKID_OLD : SKID_NONE;
    m_skid_factor   = 1.0f;
    m_real_steering = 0.0f;
}   // reset

// ----------------------------------------------------------------------------
/** Computes the actual steering fraction to be used in the physics, and 
 *  stores it in m_real_skidding. This is later used by kart to set the
 *  physical steering. The real steering takes skidding into account: if the
 *  kart skids either left or right, the steering fraction is bound by 
 *  reduce-turn-min and reduce-turn-max.
 */
void Skidding::updateSteering(float steer)
{
    if(m_skid_state==SKID_OLD)
    {
        float speed             = m_kart->getSpeed();
        float current_max_speed = m_kart->getCurrentMaxSpeed();
        float speed_ratio       = speed / current_max_speed;
        m_real_steering         = steer * m_skid_factor;
        m_visual_rotation       = m_real_steering /m_skid_max * speed_ratio;
        return;
    }
    // Now only new skidding is happening
    switch(m_skid_state)
    {
    case SKID_OLD: assert(false);
        break;
    case SKID_NONE:
        m_real_steering = steer;
        break;
    case SKID_SHOW_GFX_RIGHT:
    case SKID_ACCUMULATE_RIGHT:
        {
            float f = (1.0f+steer)*0.5f;   // map [-1,1] --> [0, 1]
            m_real_steering = m_skid_reduce_turn_min+ 
                              m_skid_reduce_turn_delta*f;
            break;
        }
    case SKID_SHOW_GFX_LEFT:
    case SKID_ACCUMULATE_LEFT:
        {
            float f = (-1.0f+steer)*0.5f;   // map [-1,1] --> [-1, 0]
            m_real_steering = -m_skid_reduce_turn_min+
                               m_skid_reduce_turn_delta*f;
            break;
        }
    }   // switch m_skid_state
    m_visual_rotation = m_skid_visual * m_real_steering;

    float st = fabsf(m_skid_time);
    if(st<m_skid_visual_time)
        m_visual_rotation *= st/m_skid_visual_time;
}   // updateSteering

// ----------------------------------------------------------------------------
/** Updates skidding status.
 *  \param dt Time step size.
 *  \param is_on_ground True if the kart is on ground.
 *  \param steering Raw steering of the kart [-1,1], i.e. not adjusted by
 *               the kart's max steering angle.
 *  \param skidding  True if the skid button is pressed.
 */
void Skidding::update(float dt, bool is_on_ground, 
                      float steering, bool skidding)
{
    if (is_on_ground)
    {
        if((fabs(steering) > 0.001f) && skidding)
        {
            m_skid_factor +=  m_skid_increase *dt/m_time_till_max_skid;
        }
        else if(m_skid_factor>1.0f)
        {
            m_skid_factor *= m_skid_decrease;
        }
    }
    else
    {
        m_skid_factor = 1.0f; // Lose any skid factor as soon as we fly
    }

    if(m_skid_factor>m_skid_max)
        m_skid_factor = m_skid_max;
    else 
        if(m_skid_factor<1.0f) m_skid_factor = 1.0f;

    updateSteering(steering);
    // FIXME hiker: remove once the new skidding code is finished.
    if(m_skid_state == SKID_OLD)
        return;

    // This is only reached if the new skidding is enabled
    // ---------------------------------------------------

    // There are four distinct states related to skidding, controlled
    // by m_skid_state:
    // SKID_NONE: no skidding is happening. From here SKID_ACCUMULATE
    //    is reached when the skid key is pressed.
    // SKID_ACCUMULATE_{LEFT,RIGHT}:
    //    The kart is still skidding. The skidding time will be
    //    accumulated in m_skid_time, and once the minimum time for a 
    //    bonus is reached, the "bonus gfx now available" gfx is shown.
    //    If the skid button is not pressed anymore, this will trigger
    //    a potential bonus. Also the rotation of the physical body to 
    //    be in synch with the graphical kart is started (which is 
    //    independently handled in the kart physics). 
    // SKID_SHOW_GFX_{LEFT<RIGHT}
    //    Shows the skidding gfx while the bonus is available.
    // FIXME: what should we do if skid key is pressed while still in 
    //   SKID_SHOW_GFX??? Adjusting the body rotation is difficult.
    //   For now skidding will only start again once SKID_SHOW_GFX
    //   is changed to SKID_NONE.
    switch(m_skid_state)
    {
    case SKID_NONE: 
        // If skidding is pressed while the kart is going straight,
        // do nothing (till the kart starts to steer in one direction).
        // Just testing for the sign of steering can result in unexpected
        // beahviour, e.g. if a player is still turning left, but already
        // presses right (it will take a few frames for this steering to
        // actuallu take place, see player_controller) - the kart would skid 
        // to the left. So we test for a 'clear enough' steering direction.
        if(!skidding || fabsf(steering)<0.3f) break;
        m_skid_state = steering > 0 ? SKID_ACCUMULATE_RIGHT
                                    : SKID_ACCUMULATE_LEFT;
        m_skid_time  = 0;   // fallthrough
    case SKID_ACCUMULATE_LEFT:
    case SKID_ACCUMULATE_RIGHT:
        {
            m_skid_time += dt;
            float bonus_time, bonus_speed;
            unsigned int level = getSkidBonus(&bonus_time, 
                                              &bonus_speed);
            // If at least level 1 bonus is reached, show appropriate gfx
            if(level>0) m_kart->getKartGFX()->setSkidLevel(level);
            // If player stops skidding, trigger bonus, and change state to
            // SKID_SHOW_GFX_*
            if(!skidding) 
            {
                m_skid_state = m_skid_state == SKID_ACCUMULATE_LEFT 
                             ? SKID_SHOW_GFX_LEFT
                             : SKID_SHOW_GFX_RIGHT;
                float t = (m_skid_time <= m_skid_visual_time)
                        ? m_skid_time
                        : m_skid_visual_time;
                float vso = getVisualSkidRotation();
                btVector3 rot(0, vso*m_post_skid_rotate_factor, 0);
                m_kart->getVehicle()->setTimedRotation(t, rot);
                // skid_time is used to count backwards for the GFX
                m_skid_time = t;
                if(bonus_time>0)
                {
                    m_kart->getKartGFX()
                          ->setCreationRateRelative(KartGFX::KGFX_SKID, 1.0f);
                    m_kart->MaxSpeed::
                        instantSpeedIncrease(MaxSpeed::MS_INCREASE_SKIDDING,
                                             bonus_speed, bonus_speed, 
                                             bonus_time, 
                                             /*fade-out-time*/ 1.0f);
                }
                else
                    m_kart->getKartGFX()
                          ->setCreationRateAbsolute(KartGFX::KGFX_SKID, 0);
            }
            break;
        }   // case
    case SKID_SHOW_GFX_LEFT:
    case SKID_SHOW_GFX_RIGHT:
        m_skid_time -= dt;
        if(m_skid_time<=0) 
        {
            m_skid_time = 0;
            m_kart->getKartGFX()
                  ->setCreationRateAbsolute(KartGFX::KGFX_SKID, 0);
            m_skid_state = SKID_NONE;
        }
    }   // switch
}   // update

// ----------------------------------------------------------------------------
/** Determines the bonus time and speed given the currently accumulated
 *  m_skid_time.
 *  \param bonus_time On return contains how long the bonus should be active.
 *  \param bonus_speed How much additional speed the kart should get.
 *  \return The bonus level: 0 = no bonus, 1 = first entry in bonus array etc.
 */
unsigned int Skidding::getSkidBonus(float *bonus_time, 
                                    float *bonus_speed) const
{
    *bonus_time  = 0;
    *bonus_speed = 0;
    for(unsigned int i=0; i<m_skid_bonus_speed.size(); i++)
    {
        if(m_skid_time<=m_skid_time_till_bonus[i]) return i;
        *bonus_speed = m_skid_bonus_speed[i];
        *bonus_time= m_skid_bonus_time[i];
    }
    return m_skid_bonus_speed.size();
}   // getSkidBonusForce

