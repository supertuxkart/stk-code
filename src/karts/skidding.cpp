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

#include "karts/skidding.hpp"

#ifdef SKID_DEBUG
#  include "graphics/show_curve.hpp"
#endif
#include "achievements/achievement_info.hpp"
#include "config/player_manager.hpp"
#include "karts/kart.hpp"
#include "karts/kart_gfx.hpp"
#include "karts/kart_properties.hpp"
#include "karts/max_speed.hpp"
#include "karts/controller/controller.hpp"
#include "modes/world.hpp"
#include "physics/btKart.hpp"
#include "tracks/track.hpp"
#include "utils/log.hpp"

/** Constructor of the skidding object.
 */
Skidding::Skidding(Kart *kart, const SkiddingProperties *sp)
{
#ifdef SKID_DEBUG
    m_predicted_curve = new ShowCurve(0.05f, 0.05f,
                                      irr::video::SColor(128, 0, 128, 0));
    m_actual_curve    = new ShowCurve(0.05f, 0.05f,
                                      irr::video::SColor(128, 0, 0, 128));
    m_predicted_curve->setVisible(false);
    m_actual_curve->setVisible(false);
#endif
    m_kart = kart;
    copyFrom(sp);
    m_skid_reduce_turn_delta = m_skid_reduce_turn_max - m_skid_reduce_turn_min;
    reset();
}   // Skidding

// ----------------------------------------------------------------------------
Skidding::~Skidding()
{
#ifdef SKID_DEBUG
    delete m_predicted_curve;
    delete m_actual_curve;
#endif
}   // ~Skidding

// ----------------------------------------------------------------------------
/** Resets all skidding related values.
 */
void Skidding::reset()
{
    m_skid_time           = 0.0f;
    m_skid_state          = m_skid_visual_time<=0 ? SKID_OLD : SKID_NONE;
    m_skid_factor         = 1.0f;
    m_real_steering       = 0.0f;
    m_visual_rotation     = 0.0f;
    m_skid_bonus_ready    = false;
    m_gfx_jump_offset     = 0.0f;
    m_remaining_jump_time = 0.0f;
    m_jump_speed          = 0.0f;
    m_kart->getKartGFX()->setCreationRateAbsolute(KartGFX::KGFX_SKIDL, 0);
    m_kart->getKartGFX()->setCreationRateAbsolute(KartGFX::KGFX_SKIDR, 0);
    m_kart->getControls().m_skid = KartControl::SC_NONE;
}   // reset

// ----------------------------------------------------------------------------
/** Computes the actual steering fraction to be used in the physics, and
 *  stores it in m_real_skidding. This is later used by kart to set the
 *  physical steering. The real steering takes skidding into account: if the
 *  kart skids either left or right, the steering fraction is bound by
 *  reduce-turn-min and reduce-turn-max.
 */
void Skidding::updateSteering(float steer, float dt)
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
    case SKID_SHOW_GFX_LEFT:
    case SKID_SHOW_GFX_RIGHT:
    case SKID_NONE:
        m_real_steering = steer;
        if(m_skid_time<m_skid_visual_time && m_skid_time>0)
        {
            float f = m_visual_rotation - m_visual_rotation*dt/m_skid_time;
            // Floating point errors when m_skid_time is very close to 0
            // can result in visual rotation set to a large number
            if( (f<0 && m_visual_rotation>0 ) ||
                (f>0 && m_visual_rotation<0)     )
                m_visual_rotation = 0;
            else
                m_visual_rotation = f;
        }
        break;
    case SKID_BREAK:
        m_real_steering = steer;
        if (m_visual_rotation > 0.05f) m_visual_rotation -= 0.05f;
        else if (m_visual_rotation < -0.05f) m_visual_rotation += 0.05f;
        else
        {
            reset();
        }
        break;
    case SKID_ACCUMULATE_RIGHT:
        {
            float f = (1.0f+steer)*0.5f;   // map [-1,1] --> [0, 1]
            m_real_steering   = m_skid_reduce_turn_min+
                                m_skid_reduce_turn_delta*f;
            if(m_skid_time < m_skid_visual_time)
                m_visual_rotation = m_skid_visual*m_real_steering*m_skid_time
                                  / m_skid_visual_time;
            else
                m_visual_rotation = m_skid_visual * m_real_steering;
            break;
        }
    case SKID_ACCUMULATE_LEFT:
        {
            float f = (-1.0f+steer)*0.5f;   // map [-1,1] --> [-1, 0]
            m_real_steering   = -m_skid_reduce_turn_min+
                                 m_skid_reduce_turn_delta*f;
            if(m_skid_time < m_skid_visual_time)
                m_visual_rotation = m_skid_visual*m_real_steering*m_skid_time
                                  / m_skid_visual_time;
            else
                m_visual_rotation = m_skid_visual * m_real_steering;
            break;
        }
        

    }   // switch m_skid_state

}   // updateSteering

// ----------------------------------------------------------------------------
/** Returns the steering value necessary to set in KartControls.m_steer in
 *  order to actually to steer the specified amount in 'steering'.
 *  If the kart is not skidding, the return value is just
 *  'steering'. Otherwise the return value will be (depending on current
 *  skidding direction) 'steering1', so that when the kart
 *  steers 'steering1', it will de facto steer by the original 'steering'
 *  amount. This function might return a result outside of [-1,1] if the
 *  specified steering can not be reached (e.g. due to skidding)
 */
float Skidding::getSteeringWhenSkidding(float steering) const
{
    switch(m_skid_state)
    {
    case SKID_OLD:            assert(false); break;
    case SKID_SHOW_GFX_LEFT:
    case SKID_SHOW_GFX_RIGHT:
    case SKID_BREAK:
    case SKID_NONE:           return steering;
        break;
    case SKID_ACCUMULATE_RIGHT:
        {
            float f = (steering - m_skid_reduce_turn_min)
                   /  m_skid_reduce_turn_delta;
            return f *2.0f-1.0f;
        }
    case SKID_ACCUMULATE_LEFT:
        {
            float f = (steering + m_skid_reduce_turn_min)
                    / m_skid_reduce_turn_delta;
            return 2.0f * f +1.0f;
        }
    }   // switch m_skid_state
    return 0;   // keep compiler quiet
}   // getSteeringWhenSkidding

 // ----------------------------------------------------------------------------
/** Updates skidding status.
 *  \param dt Time step size.
 *  \param is_on_ground True if the kart is on ground.
 *  \param steering Raw steering of the kart [-1,1], i.e. not adjusted by
 *               the kart's max steering angle.
 *  \param skidding  True if the skid button is pressed.
 */
void Skidding::update(float dt, bool is_on_ground,
                      float steering, KartControl::SkidControl skidding)
{
    // If a kart animation is shown, stop all skidding bonuses.
    if(m_kart->getKartAnimation())
    {
        reset();
        return;
    }

    // No skidding backwards or while stopped
    if(m_kart->getSpeed() < 0.001f &&
       m_skid_state != SKID_NONE && m_skid_state != SKID_BREAK)
    {
        m_skid_state = SKID_BREAK;
        m_kart->getKartGFX()->setCreationRateAbsolute(KartGFX::KGFX_SKIDL, 0);
        m_kart->getKartGFX()->setCreationRateAbsolute(KartGFX::KGFX_SKIDR, 0);
    }

    m_skid_bonus_ready = false;
    if (is_on_ground)
    {
        if((fabs(steering) > 0.001f) &&
            m_kart->getSpeed()>m_min_skid_speed &&
            (skidding==KartControl::SC_LEFT||skidding==KartControl::SC_RIGHT))
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
    {
        updateSteering(steering, dt);
        return;
    }

    // If skidding was started and a graphical jump should still
    // be displayed, update the data
    if(m_remaining_jump_time>0)
    {
        m_jump_speed -= World::getWorld()->getTrack()->getGravity()*dt;
        m_gfx_jump_offset += m_jump_speed * dt;
        m_remaining_jump_time -= dt;
        if(m_remaining_jump_time<0)
        {
            m_remaining_jump_time = 0.0f;
            m_gfx_jump_offset     = 0.0f;
        }
    }

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
        {
            if(skidding!=KartControl::SC_LEFT &&
                skidding!=KartControl::SC_RIGHT)
                break;
            // Don't allow skidding while the kart is (apparently)
            // still in the air, or when the kart is too slow
            if(m_remaining_jump_time>0 ||
                m_kart->getSpeed() <m_min_skid_speed) break;

            m_skid_state = skidding==KartControl::SC_RIGHT
                         ? SKID_ACCUMULATE_RIGHT
                         : SKID_ACCUMULATE_LEFT;
            // Add a little jump to the kart. Determine the vertical speed
            // necessary for the kart to go 0.5*jump_time up (then it needs
            // the same time to come down again), based on v = gravity * t.
            // Then use this speed to determine the impulse necessary to
            // reach this speed.
            float v = World::getWorld()->getTrack()->getGravity()
                    * 0.5f*m_physical_jump_time;
            btVector3 imp(0, v / m_kart->getBody()->getInvMass(),0);
            m_kart->getVehicle()->getRigidBody()->applyCentralImpulse(imp);

            // Some karts might use a graphical-only jump. Set it up:
            m_jump_speed = World::getWorld()->getTrack()->getGravity()
                         * 0.5f*m_graphical_jump_time;
            m_remaining_jump_time = m_graphical_jump_time;

#ifdef SKID_DEBUG
#define SPEED 20.0f
            updateSteering(steering, dt);
            m_actual_curve->clear();
            m_actual_curve->setVisible(true);
            m_predicted_curve->clear();
            m_predicted_curve->setVisible(true);
            m_predicted_curve->setPosition(m_kart->getXYZ());
            m_predicted_curve->setHeading(m_kart->getHeading());
            float angle = m_kart->getKartProperties()
                                ->getMaxSteerAngle(m_kart->getSpeed())
                        * fabsf(getSteeringFraction());
            angle = m_kart->getKartProperties()
                                ->getMaxSteerAngle(SPEED)
                        * fabsf(getSteeringFraction());
            float r = m_kart->getKartProperties()->getWheelBase()
                   / asin(angle)*1.0f;

            const int num_steps = 50;

            float dx = 2*r / num_steps;

            for(float x = 0; x <=2*r; x+=dx)
            {
                float real_x = m_skid_state==SKID_ACCUMULATE_LEFT ? -x : x;
                Vec3 xyz(real_x, 0.2f, sqrt(r*r-(r-x)*(r-x))*(1.0f+SPEED/150.0f)
                          *(1+(angle/m_kart->getKartProperties()->getMaxSteerAngle(SPEED)-0.6f)*0.1f));
                Vec3 xyz1=m_kart->getTrans()(xyz);
                Log::debug("Skidding", "predict %f %f %f speed %f angle %f",
                    xyz1.getX(), xyz1.getY(), xyz1.getZ(),
                    m_kart->getSpeed(), angle);
                m_predicted_curve->addPoint(xyz);
            }

#endif
            m_skid_time  = 0;   // fallthrough
        }
    case SKID_BREAK:
        {
            updateSteering(steering, dt);
            break;
        }
    case SKID_ACCUMULATE_LEFT:
    case SKID_ACCUMULATE_RIGHT:
        {
#ifdef SKID_DEBUG
            Vec3 v=m_kart->getVelocity();
            if(v.length()>5)
            {
                float r = SPEED/sqrt(v.getX()*v.getX() + v.getZ()*v.getZ());
                v.setX(v.getX()*r);
                v.setZ(v.getZ()*r);
                m_kart->getBody()->setLinearVelocity(v);

            }

            m_actual_curve->addPoint(m_kart->getXYZ());
            Log::debug("Skidding", "actual %f %f %f turn %f speed %f angle %f",
                m_kart->getXYZ().getX(),m_kart->getXYZ().getY(),m_kart->getXYZ().getZ(),
                m_real_steering, m_kart->getSpeed(),
                m_kart->getKartProperties()->getMaxSteerAngle(m_kart->getSpeed()));
#endif
            m_skid_time += dt;
            float bonus_time, bonus_speed, bonus_force;
            unsigned int level = getSkidBonus(&bonus_time, &bonus_speed,
                                              &bonus_force);
            // If at least level 1 bonus is reached, show appropriate gfx
            if(level>0)
            {
                m_skid_bonus_ready = true;
                m_kart->getKartGFX()->setSkidLevel(level);
            }
            // If player stops skidding, trigger bonus, and change state to
            // SKID_SHOW_GFX_*
            if(skidding == KartControl::SC_NONE)
            {
                m_skid_state = m_skid_state == SKID_ACCUMULATE_LEFT
                             ? SKID_SHOW_GFX_LEFT
                             : SKID_SHOW_GFX_RIGHT;
                float t = std::min(m_skid_time, m_skid_visual_time);
                t       = std::min(t,           m_skid_revert_visual_time);

                float vso = getVisualSkidRotation();
                btVector3 rot(0, vso*m_post_skid_rotate_factor, 0);
                m_kart->getVehicle()->setTimedRotation(t, rot);
                // skid_time is used to count backwards for the GFX
                m_skid_time = t;
                if(bonus_time>0)
                {
                    m_kart->getKartGFX()
                          ->setCreationRateRelative(KartGFX::KGFX_SKIDL, 1.0f);
                    m_kart->getKartGFX()
                          ->setCreationRateRelative(KartGFX::KGFX_SKIDR, 1.0f);
                    m_kart->m_max_speed->
                        instantSpeedIncrease(MaxSpeed::MS_INCREASE_SKIDDING,
                                             bonus_speed, bonus_speed,
                                             bonus_force, bonus_time,
                                             /*fade-out-time*/ 1.0f);
                    
                    StateManager::ActivePlayer *c = m_kart->getController()->getPlayer();
                    if (c && c->getConstProfile() == PlayerManager::getCurrentPlayer())
                    {
                        PlayerManager::increaseAchievement(AchievementInfo::ACHIEVE_SKIDDING, "skidding");
                    }
                }
                else {
                    m_kart->getKartGFX()
                          ->setCreationRateAbsolute(KartGFX::KGFX_SKIDL, 0);
                    m_kart->getKartGFX()
                          ->setCreationRateAbsolute(KartGFX::KGFX_SKIDR, 0);
            }
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
                  ->setCreationRateAbsolute(KartGFX::KGFX_SKIDL, 0);
            m_kart->getKartGFX()
                  ->setCreationRateAbsolute(KartGFX::KGFX_SKIDR, 0);
            m_skid_state = SKID_NONE;
        }
    }   // switch
    updateSteering(steering, dt);
}   // update

// ----------------------------------------------------------------------------
/** Determines the bonus time and speed given the currently accumulated
 *  m_skid_time.
 *  \param bonus_time On return contains how long the bonus should be active.
 *  \param bonus_speed How much additional speed the kart should get.
 *  \param bonus_force Additional engine force.
 *  \return The bonus level: 0 = no bonus, 1 = first entry in bonus array etc.
 */
unsigned int Skidding::getSkidBonus(float *bonus_time,
                                    float *bonus_speed,
                                    float *bonus_force) const
{
    *bonus_time  = 0;
    *bonus_speed = 0;
    *bonus_force = 0;
    for(unsigned int i=0; i<m_skid_bonus_speed.size(); i++)
    {
        if(m_skid_time<=m_skid_time_till_bonus[i]) return i;
        *bonus_speed = m_skid_bonus_speed[i];
        *bonus_time  = m_skid_bonus_time[i];
        *bonus_force = m_skid_bonus_force[i];
    }
    return (unsigned int) m_skid_bonus_speed.size();
}   // getSkidBonusForce

