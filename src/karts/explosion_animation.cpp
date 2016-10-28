//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2012-2015 Joerg Henrichs
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

#include "karts/explosion_animation.hpp"

#include "audio/sfx_manager.hpp"
#include "graphics/callbacks.hpp"
#include "graphics/camera.hpp"
#include "items/attachment.hpp"
#include "karts/abstract_kart.hpp"
#include "karts/kart_properties.hpp"
#include "modes/world.hpp"
#include "tracks/track.hpp"

/** A static create function that does only create an explosion if
 *  the explosion happens to be close enough to affect the kart.
 *  Otherwise, NULL is returned.
 *  \param kart The kart that is exploded.
 *  \param pos The position where the explosion happened.
 *  \param direct_hit If the kart was hit directly.
 */
ExplosionAnimation *ExplosionAnimation::create(AbstractKart *kart,
                                               const Vec3 &pos,
                                               bool direct_hit)
{
    if(kart->isInvulnerable()) return NULL;

    float r = kart->getKartProperties()->getExplosionRadius();

    // Ignore explosion that are too far away.
    if(!direct_hit && pos.distance2(kart->getXYZ())>r*r) return NULL;

    if(kart->isShielded())
    {
        kart->decreaseShieldTime();
        return NULL;
    }

    return new ExplosionAnimation(kart, pos, direct_hit);
}   // create

// ----------------------------------------------------------------------------
/** A static create function that does only create an explosion if
 *  the explosion happens to be close enough to affect the kart.
 *  Otherwise, NULL is returned. */
ExplosionAnimation *ExplosionAnimation::create(AbstractKart *kart)
{
    if(kart->isInvulnerable()) return NULL;
    else if(kart->isShielded())
    {
        kart->decreaseShieldTime();
        return NULL;
    }
    return new ExplosionAnimation(kart, kart->getXYZ(), /*direct hit*/true);
}   // create

// ----------------------------------------------------------------------------
ExplosionAnimation::ExplosionAnimation(AbstractKart *kart,
                                       const Vec3 &explosion_position,
                                       bool direct_hit)
                  : AbstractKartAnimation(kart, "ExplosionAnimation")
 {
    m_xyz = m_kart->getXYZ();
    m_orig_xyz = m_xyz;
    m_normal = m_kart->getNormal();
    m_kart->playCustomSFX(SFXManager::CUSTOM_EXPLODE);
    m_timer = m_kart->getKartProperties()->getExplosionDuration();

    // Non-direct hits will be only affected half as much.
    if(!direct_hit) m_timer*=0.5f;

    // Half of the overall time is spent in raising, so only use
    // half of the explosion time here.
    // Velocity after t seconds is:
    // v(t) = m_velocity + t*gravity
    // Since v(explosion_time*0.5) = 0, the following forumla computes
    // the right initial velocity for a kart to land back after
    // the specified time.
    m_velocity = 0.5f * m_timer * World::getWorld()->getTrack()->getGravity();

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
    m_kart->showStarEffect(t);
    
    m_kart->getAttachment()->clear();

 };   // ExplosionAnimation

//-----------------------------------------------------------------------------
ExplosionAnimation::~ExplosionAnimation()
{
    // Only play with physics and camera if the object is getting destroyed
    // because its time is up. If there is still time left when this gets
    // called, it means that the world is getting destroyed so we don't touch
    // these settings.
    if (m_timer < 0)
    {
        m_kart->getBody()->setLinearVelocity(btVector3(0,0,0));
        m_kart->getBody()->setAngularVelocity(btVector3(0,0,0));
        for(unsigned int i=0; i<Camera::getNumCameras(); i++)
        {
            Camera *camera = Camera::getCamera(i);
            if(camera->getType() != Camera::CM_TYPE_END)
                camera->setMode(Camera::CM_NORMAL);
        }
    }
}   // ~KartAnimation

// ----------------------------------------------------------------------------
/** Updates the kart animation.
 *  \param dt Time step size.
 *  \return True if the explosion is still shown, false if it has finished.
 */
void ExplosionAnimation::update(float dt)
{
    m_velocity -= dt*World::getWorld()->getTrack()->getGravity();
    m_xyz = m_xyz + dt*m_velocity*m_normal;

    // Make sure the kart does not end up under the track
    if ((m_xyz - m_orig_xyz).dot(m_normal)<0)
    {
        m_xyz = m_orig_xyz;
        // This will trigger the end of the animations
        m_timer = -1;
    }
    m_kart->setXYZ(m_xyz);
    m_curr_rotation += dt*m_add_rotation;
    btQuaternion q(m_curr_rotation.getHeading(), m_curr_rotation.getPitch(),
                   m_curr_rotation.getRoll());
    m_kart->setRotation(q);

    AbstractKartAnimation::update(dt);
}   // update
