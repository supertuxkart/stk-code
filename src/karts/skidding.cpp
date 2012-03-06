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
    m_kart             = kart;
    copyFrom(sp);
    reset();
}   // Skidding

// ----------------------------------------------------------------------------    
/** Resets all skidding related values.
 */
void Skidding::reset()
{
    m_skid_time   = 0.0f;
    m_skid_state  = m_skid_visual_time<=0 ? SKID_OLD : SKID_NONE;
    m_skid_factor = 1.0f;
}   // reset

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

    // FIXME hiker: remove once the new skidding code is finished.
    if(m_skid_state == SKID_OLD)
        return;

    // This is only reached if the new skidding is enabled
    // ---------------------------------------------------

    // There are four distinct states related to skidding, controlled
    // by m_skid_state:
    // SKID_NONE: no skidding is happening. From here SKID_ACCUMULATE
    //    is reached when the skid key is pressed.
    // SKID_ACCUMULATE:
    //    The kart is still skidding. The skidding time will be
    //    accumulated in m_skid_time, and once the minimum time for a 
    //    bonus is reached, the "bonus gfx now available" gfx is shown.
    //    If the skid button is not pressed anymore, this will trigger
    //    a potential bonus. Also the rotation of the physical body to 
    //    be in synch with the graphical kart is started (which is 
    //    independently handled in the kart physics). 
    // SKID_SHOW_GFX
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
        if(!skidding || steering==0) break;
        m_skid_state = steering > 0 ? SKID_ACCUMULATE_LEFT
                                    : SKID_ACCUMULATE_RIGHT;
        m_skid_time  = 0;   // fallthrough
    case SKID_ACCUMULATE_LEFT:
    case SKID_ACCUMULATE_RIGHT:
        {
            m_skid_time += dt;
            float bonus_time, bonus_force;
            unsigned int level = getSkidBonus(&bonus_time, 
                                              &bonus_force);
            // If at least level 1 bonus is reached, show appropriate gfx
            if(level>0) m_kart->getKartGFX()->setSkidLevel(level);
            // If player stops skidding, trigger bonus, and change state to
            // SKID_SHOW_GFX
            if(!skidding) 
            {
                m_skid_state = SKID_SHOW_GFX;
                float t = (m_skid_time <= m_skid_visual_time)
                        ? m_skid_time
                        : m_skid_visual_time;
                float vso = getVisualSkidOffset();
                btVector3 rot(0, vso*m_post_skid_rotate_factor, 0);
                m_kart->getVehicle()->setTimedRotation(t, rot);
                // skid_time is used to count backwards for the GFX
                m_skid_time = t;
                if(bonus_time>0)
                {
                    m_kart->MaxSpeed::increaseMaxSpeed(
                        MaxSpeed::MS_INCREASE_SKIDDING, 10, bonus_time, 1);
                    m_kart->getKartGFX()
                          ->setCreationRateRelative(KartGFX::KGFX_SKID, 1.0f);
                    // FIXME hiker: for now just misuse the zipper code
                    m_kart->handleZipper(0);
                }
                else
                    m_kart->getKartGFX()
                          ->setCreationRateAbsolute(KartGFX::KGFX_SKID, 0);
            }
            break;
        }   // case
    case SKID_SHOW_GFX:
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

// ----------------------------------------------------------------------------
/** Determines how much the graphics model of the kart should be rotated
 *  additionally (for skidding), depending on how long the kart has been
 *  skidding etc.
 *  \return Returns the angle of the additional rotation of the kart.
 */
float Skidding::getVisualSkidOffset() const
{
    float speed = m_kart->getSpeed();
    float steer_percent = m_kart->getSteerPercent();
    float current_max_speed = m_kart->getCurrentMaxSpeed();
    if(m_skid_visual_time==0)
    {
        float speed_ratio = speed / current_max_speed;
        float r = m_skid_factor / m_skid_max;
        return steer_percent * speed_ratio * r;
    }

    // New skidding code
    float f = m_skid_visual * steer_percent;
    //if(m_kart->getSpeed() < m_kart->getKartProperties()->getMaxSpeed())
    //    f *= m_kart->getSpeed()/m_kart->getKartProperties()->getMaxSpeed();

    float st = fabsf(m_skid_time);
    if(st<m_skid_visual_time)
        f *= st/m_skid_visual_time;

    return f;

}   // getVisualSkidOffset

/* EOF */
