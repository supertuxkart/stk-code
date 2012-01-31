//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2010 Joerg Henrichs
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

#include "karts/emergency_animation.hpp"

#include "graphics/camera.hpp"
#include "graphics/referee.hpp"
#include "graphics/stars.hpp"
#include "items/attachment.hpp"
#include "karts/kart.hpp"
#include "modes/world.hpp"
#include "modes/three_strikes_battle.hpp"
#include "physics/btKart.hpp"
#include "tracks/track.hpp"
#include "utils/constants.hpp"

/** The constructor stores a pointer to the kart this object is animating,
 *  and initialised the timer.
 *  \param kart Pointer to the kart which is animated.
 */
EmergencyAnimation::EmergencyAnimation(Kart *kart)
{
    m_stars_effect = NULL;
    m_referee      = NULL;
    m_kart         = kart;
    // Setting kart mode here is important! If the mode should be rescue when 
    // reset() is called, it is assumed that this was triggered by a restart, 
    // and that the vehicle must be added back to the physics world. Since 
    // reset() is also called at the very start, it must be guaranteed that 
    // rescue is not set.
    m_kart_mode    = EA_NONE;
    m_eliminated   = false;
};   // EmergencyAnimation

//-----------------------------------------------------------------------------
EmergencyAnimation::~EmergencyAnimation()
{
    if(m_stars_effect)
        delete m_stars_effect;
}   // ~EmergencyAnimation

//-----------------------------------------------------------------------------
/** Resets all data at the beginning of a race.
 */
void EmergencyAnimation::reset()
{

    // Create the stars effect in the first reset
    if(!m_stars_effect)
        m_stars_effect =
        new Stars(m_kart->getNode(),
                  core::vector3df(0.0f, 
                                  m_kart->getKartModel()->getModel()
                                        ->getBoundingBox().MaxEdge.Y,
                                  0.0f)                               );

    // Reset star effect in case that it is currently being shown.
    m_stars_effect->reset();

    // If the kart was eliminated or rescued, the body was removed from the
    // physics world. Add it again.
    if(m_eliminated || playingEmergencyAnimation())
    {
        World::getWorld()->getPhysics()->addKart(m_kart);
    }
    m_timer      = 0;
    m_kart_mode  = EA_NONE;
    m_eliminated = false;
    if(m_referee)
    {
        delete m_referee;
        m_referee = NULL;
    }
}   // reset

//-----------------------------------------------------------------------------
/** Eliminates a kart from the race. It removes the kart from the physics
 *  world, and makes the scene node invisible.
 */
void EmergencyAnimation::eliminate(bool remove)
{
    if (!playingEmergencyAnimation() && remove)
    {
        World::getWorld()->getPhysics()->removeKart(m_kart);
    }
    
    if (m_stars_effect)
    {
        m_stars_effect->reset();
        m_stars_effect->update(1);
    }
    
    m_eliminated = true;
    m_kart_mode = EA_NONE;
    
    if (remove)
    {
         m_kart->getNode()->setVisible(false);
    }
}   // eliminate

//-----------------------------------------------------------------------------
/** Sets the mode of the kart to being rescued, attaches the rescue model
 *  and saves the current pitch and roll (for the rescue animation). It
 *  also removes the kart from the physics world.
 */
void EmergencyAnimation::forceRescue(bool is_auto_rescue)
{
    if(playingEmergencyAnimation()) return;

    assert(!m_referee);
    m_referee     = new Referee(*m_kart);
    m_kart->getNode()->addChild(m_referee->getSceneNode());
    m_kart_mode   = EA_RESCUE;
    m_timer       = m_kart->getKartProperties()->getRescueTime();
    m_up_velocity = m_kart->getKartProperties()->getRescueHeight() / m_timer;
    m_xyz         = m_kart->getXYZ();

    m_kart->getAttachment()->clear();

    m_curr_rotation.setPitch(m_kart->getPitch());
    m_curr_rotation.setRoll(m_kart->getRoll()  );
    m_curr_rotation.setHeading(0);
    m_add_rotation = -m_curr_rotation/m_timer;
    m_curr_rotation.setHeading(m_kart->getHeading());

    World::getWorld()->getPhysics()->removeKart(m_kart);

    // Add a hit unless it was auto-rescue
    if(race_manager->getMinorMode()==RaceManager::MINOR_MODE_3_STRIKES &&
        !is_auto_rescue)
    {
        ThreeStrikesBattle *world=(ThreeStrikesBattle*)World::getWorld();
        world->kartHit(m_kart->getWorldKartId());
    }
}   // forceRescue

//-----------------------------------------------------------------------------
/** Starts an explosion animation.
 *  \param pos The coordinates of the explosion.
 *  \param direct_hig True if the kart was hit directly --> maximal impact.
 */
void EmergencyAnimation::handleExplosion(const Vec3 &pos, bool direct_hit)
{
    // Avoid doing another explosion while a kart is thrown around in the air.
    if(playingEmergencyAnimation())
        return;
    
    if(m_kart->isInvulnerable())
        return;
    
    m_xyz   = m_kart->getXYZ();
    // Ignore explosion that are too far away.
    float r = m_kart->getKartProperties()->getExplosionRadius();
    if(!direct_hit && pos.distance2(m_xyz)>r*r) return;

    m_kart->playCustomSFX(SFXManager::CUSTOM_EXPLODE);
    m_kart_mode = EA_EXPLOSION;
    m_timer     = m_kart->getKartProperties()->getExplosionTime();;

    // Non-direct hits will be only affected half as much.
    if(!direct_hit) m_timer*=0.5f;

    // Half of the overall time is spent in raising, so only use
    // half of the explosion time here.
    // Velocity after t seconds is:
    // v(t) = m_up_velocity + t*gravity
    // Since v(explosion_time*0.5) = 0, the following forumla computes 
    // the right initial velocity for a kart to land back after
    // the specified time.
    m_up_velocity = 0.5f * m_timer * World::getWorld()->getTrack()->getGravity();
    World::getWorld()->getPhysics()->removeKart(m_kart);
    
    m_curr_rotation.setHeading(m_kart->getHeading());
    m_curr_rotation.setPitch(m_kart->getPitch());
    m_curr_rotation.setRoll(m_kart->getRoll());

    const int max_rotation = direct_hit ? 2 : 1;
    // To get rotations in both directions for each axis we determine a random
    // number between -(max_rotation-1) and +(max_rotation-1)
    float f=2.0f*M_PI/m_timer;
    m_add_rotation.setHeading( (rand()%(2*max_rotation+1)-max_rotation)*f );
    m_add_rotation.setPitch(   (rand()%(2*max_rotation+1)-max_rotation)*f );
    m_add_rotation.setRoll(    (rand()%(2*max_rotation+1)-max_rotation)*f );
    
    // Set invulnerable time, and graphical effects
    float t = m_kart->getKartProperties()->getExplosionInvulnerabilityTime();
    m_kart->setInvulnerableTime(t);
    if ( UserConfigParams::m_graphical_effects )
    {
        m_stars_effect->showFor(t);
    }

    m_kart->getAttachment()->clear();
}   // handleExplosion

// ----------------------------------------------------------------------------
/** Updates the explosion animation.
 *  \param dt Time step size.
 *  \return True if the explosion is still shown, false if it has finished.
 */
void EmergencyAnimation::update(float dt)
{
    if ( UserConfigParams::m_graphical_effects )
    {
        // update star effect (call will do nothing if stars are not activated)
        m_stars_effect->update(dt);
    }

    if(!playingEmergencyAnimation()) return;

    // See if the timer expires, if so return the kart to normal game play
    m_timer -= dt;
    if(m_timer<0)
    {
        if(m_kart_mode==EA_RESCUE)
        {
            World::getWorld()->moveKartAfterRescue(m_kart);
            m_kart->getNode()->removeChild(m_referee->getSceneNode());
            delete m_referee;
            m_referee = NULL;
        }
        World::getWorld()->getPhysics()->addKart(m_kart);
        m_kart->getBody()->setLinearVelocity(btVector3(0,0,0));
        m_kart->getBody()->setAngularVelocity(btVector3(0,0,0));
        m_kart_mode  = EA_NONE;
        // We have to make sure that m_kart_mode and m_eliminated are in 
        // synch, otherwise it can happen that a kart is entered in world
        // here, and again in reset (e.g. when restarting the race) if
        // m_eliminated is still true.
        m_eliminated = false;
        if(m_kart->getCamera() && m_kart->getCamera()->getMode() != Camera::CM_FINAL)
            m_kart->getCamera()->setMode(Camera::CM_NORMAL);
        return;
    }

    // Explosions change the upwards velocity:
    if ( m_kart_mode==EA_EXPLOSION)
    {
        m_up_velocity -= dt*World::getWorld()->getTrack()->getGravity();
    }

    m_xyz.setY(m_xyz.getY()+dt*m_up_velocity);
    m_kart->setXYZ(m_xyz);
    m_curr_rotation += dt*m_add_rotation;
    btQuaternion q(m_curr_rotation.getHeading(), m_curr_rotation.getPitch(),
                   m_curr_rotation.getRoll());
    m_kart->setRotation(q);
}   // update
