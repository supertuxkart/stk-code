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
#include "network/network_string.hpp"
#include "physics/btKart.hpp"
#include "tracks/track.hpp"
#include "utils/log.hpp"

/** Constructor of the skidding object.
 */
Skidding::Skidding(Kart *kart)
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
    m_skid_reduce_turn_delta = m_kart->getKartProperties()->getSkidReduceTurnMax()
                             - m_kart->getKartProperties()->getSkidReduceTurnMin();
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
    m_skid_state          = SKID_NONE;
    m_skid_factor         = 1.0f;
    m_real_steering       = 0.0f;
    m_visual_rotation     = 0.0f;
    m_skid_bonus_ready    = false;
    m_gfx_jump_offset     = 0.0f;
    m_remaining_jump_time = 0.0f;
    m_jump_speed          = 0.0f;
    m_kart->getKartGFX()->setCreationRateAbsolute(KartGFX::KGFX_SKIDL, 0);
    m_kart->getKartGFX()->setCreationRateAbsolute(KartGFX::KGFX_SKIDR, 0);
    m_kart->getKartGFX()->updateSkidLight(0);
    m_kart->getControls().setSkidControl(KartControl::SC_NONE);
    
    btVector3 rot(0, 0, 0);
    // Only access the vehicle if the kart is not a ghost
    if (!m_kart->isGhostKart())
        m_kart->getVehicle()->setTimedRotation(0, rot);
}   // reset

// ----------------------------------------------------------------------------
/** Save the skidding state of a kart. It only saves the important physics
 *  values, not visual only values like m_visual_rotation, m_gfx_jump_offset, 
 *  m_remaining_jump_time and m_jump_speed. Similarly m_real_steering is output
 *  of updateRewind() and will be recomputed every frame when update() is called,
 *  and similart m_skid_bonus_ready
 *  \param buffer Buffer for the state information. 
 */
void Skidding::saveState(BareNetworkString *buffer)
{
    buffer->addUInt8(m_skid_state);
    if(m_skid_state == SKID_NONE)
        return;
    buffer->addFloat(m_skid_time);
    buffer->addFloat(m_skid_factor);
}   // saveState

// ----------------------------------------------------------------------------
/** Restores the skidding state of a kart.
 *  \param buffer Buffer with state information. 
 */
void Skidding::rewindTo(BareNetworkString *buffer)
{
    m_skid_state = (SkidState)buffer->getUInt8();
    if(m_skid_state == SKID_NONE)
        return;
    m_skid_time =   buffer->getFloat();
    m_skid_factor = buffer->getFloat();
}   // rewindTo

// ----------------------------------------------------------------------------
/** Computes the actual steering fraction to be used in the physics, and
 *  stores it in m_real_skidding. This is later used by kart to set the
 *  physical steering. The real steering takes skidding into account: if the
 *  kart skids either left or right, the steering fraction is bound by
 *  reduce-turn-min and reduce-turn-max.
 */
float Skidding::updateSteering(float steer, float dt)
{
    float steer_result;

    const KartProperties *kp = m_kart->getKartProperties();

    switch(m_skid_state)
    {
    case SKID_SHOW_GFX_LEFT:
    case SKID_SHOW_GFX_RIGHT:
    case SKID_NONE:
        steer_result = steer;
        if (m_skid_time < kp->getSkidVisualTime() && m_skid_time > 0)
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
        steer_result = steer;
        if (m_visual_rotation > 0.1f) m_visual_rotation -= 0.1f;
        else if (m_visual_rotation < -0.1f) m_visual_rotation += 0.1f;
        else
        {
            reset();
        }
        break;
    case SKID_ACCUMULATE_RIGHT:
        {
            float f = (1.0f+steer)*0.5f;   // map [-1,1] --> [0, 1]
            steer_result  = kp->getSkidReduceTurnMin()
                             + m_skid_reduce_turn_delta * f;
            if(m_skid_time < kp->getSkidVisualTime())
                m_visual_rotation = kp->getSkidVisual()
                                  * steer_result * m_skid_time
                                  / kp->getSkidVisualTime();
            else
                m_visual_rotation = kp->getSkidVisual() * steer_result;
            break;
        }   // SKID_ACCUMULATE_RIGHT
    case SKID_ACCUMULATE_LEFT:
        {
            float f = (-1.0f+steer)*0.5f;   // map [-1,1] --> [-1, 0]
            steer_result   = -kp->getSkidReduceTurnMin()
                               + m_skid_reduce_turn_delta * f;
            if(m_skid_time < kp->getSkidVisualTime())
                m_visual_rotation = kp->getSkidVisual()
                                  * steer_result * m_skid_time
                                  / kp->getSkidVisualTime();
            else
                m_visual_rotation = kp->getSkidVisual() * steer_result;
            break;
        }   // case SKID_ACCUMULATE_LEFT

    }   // switch m_skid_state

    return steer_result;
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
    case SKID_SHOW_GFX_LEFT:
    case SKID_SHOW_GFX_RIGHT:
    case SKID_BREAK:
    case SKID_NONE:           return steering;
        break;
    case SKID_ACCUMULATE_RIGHT:
        {
            float f = (steering - m_kart->getKartProperties()->getSkidReduceTurnMin())
                   /  m_skid_reduce_turn_delta;
            return f *2.0f-1.0f;
        }
    case SKID_ACCUMULATE_LEFT:
        {
            float f = (steering + m_kart->getKartProperties()->getSkidReduceTurnMin())
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
    const KartProperties *kp = m_kart->getKartProperties();

    // If a kart animation is shown, stop all skidding bonuses.
    if(m_kart->getKartAnimation())
    {
        reset();
        return;
    }
#ifdef SKIDDING_PARTICLE_DEBUG
    // This code will cause skidmarks to be displayed all the time, even
    // when the kart is not moving.
    m_kart->getKartGFX()->setCreationRateRelative(KartGFX::KGFX_SKIDL, 0.5f);
    m_kart->getKartGFX()->setCreationRateRelative(KartGFX::KGFX_SKIDR, 0.5f);
#endif

    // No skidding backwards or while stopped
    if(m_kart->getSpeed() < kp->getSkidMinSpeed() &&
       m_skid_state != SKID_NONE && m_skid_state != SKID_BREAK)
    {
        m_skid_state = SKID_BREAK;
        m_kart->getKartGFX()->setCreationRateAbsolute(KartGFX::KGFX_SKIDL, 0);
        m_kart->getKartGFX()->setCreationRateAbsolute(KartGFX::KGFX_SKIDR, 0);
    }

    m_skid_bonus_ready = false;
    if (is_on_ground)
    {
        if ((fabs(steering) > 0.001f) &&
            m_kart->getSpeed() > kp->getSkidMinSpeed() &&
            (skidding == KartControl::SC_LEFT || skidding == KartControl::SC_RIGHT))
        {
            m_skid_factor += kp->getSkidIncrease()
                           * dt / kp->getSkidTimeTillMax();
        }
        else if (m_skid_factor > 1.0f)
        {
            m_skid_factor *= kp->getSkidDecrease();
        }
    }
    else
    {
        m_skid_factor = 1.0f; // Lose any skid factor as soon as we fly
    }

    if (m_skid_factor > kp->getSkidMax())
        m_skid_factor = kp->getSkidMax();
    else
        if (m_skid_factor < 1.0f) m_skid_factor = 1.0f;

    // If skidding was started and a graphical jump should still
    // be displayed, update the data
    if(m_remaining_jump_time>0)
    {
        m_jump_speed -= Track::getCurrentTrack()->getGravity()*dt;
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
    // SKID_SHOW_GFX_{LEFT,RIGHT}
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
            if (m_remaining_jump_time > 0 ||
                m_kart->getSpeed() < kp->getSkidMinSpeed())
                break;

            m_skid_state = skidding==KartControl::SC_RIGHT
                         ? SKID_ACCUMULATE_RIGHT
                         : SKID_ACCUMULATE_LEFT;
            // Add a little jump to the kart. Determine the vertical speed
            // necessary for the kart to go 0.5*jump_time up (then it needs
            // the same time to come down again), based on v = gravity * t.
            // Then use this speed to determine the impulse necessary to
            // reach this speed.
            float v = Track::getCurrentTrack()->getGravity()
                    * 0.5f * kp->getSkidPhysicalJumpTime();
            btVector3 imp(0, v / m_kart->getBody()->getInvMass(),0);
            m_kart->getVehicle()->getRigidBody()->applyCentralImpulse(imp);

            // Some karts might use a graphical-only jump. Set it up:
            m_jump_speed = Track::getCurrentTrack()->getGravity()
                         * 0.5f * kp->getSkidGraphicalJumpTime();
            m_remaining_jump_time = kp->getSkidGraphicalJumpTime();

#ifdef SKID_DEBUG
#define SPEED 20.0f
            m_real_steering = updateSteering(steering, dt);
            m_actual_curve->clear();
            m_actual_curve->setVisible(true);
            m_predicted_curve->clear();
            m_predicted_curve->setVisible(true);
            m_predicted_curve->setPosition(m_kart->getXYZ());
            m_predicted_curve->setHeading(m_kart->getHeading());
            float angle = kp->getMaxSteerAngle(SPEED)
                        * fabsf(getSteeringFraction());
            float r = kp->getWheelBase()
                    / asin(angle)*1.0f;

            const int num_steps = 50;

            float dx = 2*r / num_steps;

            for(float x = 0; x <=2*r; x+=dx)
            {
                float real_x = m_skid_state==SKID_ACCUMULATE_LEFT ? -x : x;
                Vec3 xyz(real_x, 0.2f, sqrt(r*r-(r-x)*(r-x))*(1.0f+SPEED/150.0f)
                          *(1+(angle/kp->getMaxSteerAngle(SPEED)-0.6f)*0.1f));
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
                kp->getMaxSteerAngle(m_kart->getSpeed()));
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
                m_kart->getKartGFX()->updateSkidLight(level);
            }
            // If player stops skidding, trigger bonus, and change state to
            // SKID_SHOW_GFX_*
            if(skidding == KartControl::SC_NONE)
            {
                m_skid_state = m_skid_state == SKID_ACCUMULATE_LEFT
                             ? SKID_SHOW_GFX_LEFT
                             : SKID_SHOW_GFX_RIGHT;
                float t = std::min(m_skid_time, kp->getSkidVisualTime());
                t       = std::min(t,           kp->getSkidRevertVisualTime());

                float vso = getVisualSkidRotation();
                btVector3 rot(0, vso * kp->getSkidPostSkidRotateFactor(), 0);
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
                    
                    if (m_kart->getController()->canGetAchievements())
                    {
                        PlayerManager::increaseAchievement(
                                AchievementInfo::ACHIEVE_SKIDDING, "skidding");
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
            m_kart->getKartGFX()->updateSkidLight(0);
            m_skid_state = SKID_NONE;
        }
    }   // switch

    m_real_steering = updateSteering(steering, dt);
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
    const KartProperties *kp = m_kart->getKartProperties();

    *bonus_time  = 0;
    *bonus_speed = 0;
    *bonus_force = 0;
    for (unsigned int i = 0; i < kp->getSkidBonusSpeed().size(); i++)
    {
        if (m_skid_time <= kp->getSkidTimeTillBonus()[i])
            return i;
        *bonus_speed = kp->getSkidBonusSpeed()[i];
        *bonus_time  = kp->getSkidBonusTime()[i];
        *bonus_force = kp->getSkidBonusForce()[i];
    }
    return (unsigned int) kp->getSkidBonusSpeed().size();
}   // getSkidBonusForce

