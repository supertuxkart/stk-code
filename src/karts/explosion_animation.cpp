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
#include "graphics/camera.hpp"
#include "items/attachment.hpp"
#include "karts/abstract_kart.hpp"
#include "karts/kart_properties.hpp"
#include "modes/world.hpp"
#include "network/network_config.hpp"
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
                                       bool direct_hit, bool from_state)
                  : AbstractKartAnimation(kart, "ExplosionAnimation")
{
    m_direct_hit = direct_hit;
    m_reset_ticks = -1;
    float timer = m_kart->getKartProperties()->getExplosionDuration();
    m_timer = stk_config->time2Ticks(timer);
    m_normal = m_kart->getNormal();

    // Non-direct hits will be only affected half as much.
    if (!m_direct_hit)
    {
        timer *= 0.5f;
        m_timer /= 2;
    }

    // Put the kart back to its own flag base like rescue if direct hit in CTF
    if (race_manager->getMajorMode() ==
        RaceManager::MAJOR_MODE_CAPTURE_THE_FLAG && m_direct_hit)
    {
        m_reset_ticks = stk_config->time2Ticks(timer * 0.2f);
    }

    if (m_reset_ticks != -1)
    {
        m_xyz = m_kart->getXYZ();
        m_orig_xyz = m_xyz;
        btTransform prev_trans = kart->getTrans();
        World::getWorld()->moveKartAfterRescue(kart);
        m_end_transform = kart->getTrans();
        m_reset_xyz = m_end_transform.getOrigin();
        m_reset_normal = m_end_transform.getBasis().getColumn(1);
        kart->getBody()->setCenterOfMassTransform(prev_trans);
        kart->setTrans(prev_trans);
    }
    else
    {
        m_end_transform = m_kart->getTrans();
        m_xyz = m_kart->getXYZ();
        m_orig_xyz = m_xyz;
    }
    m_kart->playCustomSFX(SFXManager::CUSTOM_EXPLODE);

    if (NetworkConfig::get()->isNetworking() &&
        NetworkConfig::get()->isServer())
    {
        m_end_ticks = m_timer + World::getWorld()->getTicksSinceStart() + 1;
    }
    // Half of the overall time is spent in raising, so only use
    // half of the explosion time here.
    // Velocity after t seconds is:
    // v(t) = m_velocity + t*gravity
    // Since v(explosion_time*0.5) = 0, the following forumla computes
    // the right initial velocity for a kart to land back after
    // the specified time.
    m_velocity = 0.5f * timer * Track::getCurrentTrack()->getGravity();

    m_curr_rotation.setHeading(m_kart->getHeading());
    m_curr_rotation.setPitch(m_kart->getPitch());
    m_curr_rotation.setRoll(m_kart->getRoll());

    const int max_rotation = m_direct_hit ? 2 : 1;
    // To get rotations in both directions for each axis we determine a random
    // number between -(max_rotation-1) and +(max_rotation-1)
    float f = 2.0f * M_PI / timer;
    m_add_rotation.setHeading( (rand()%(2*max_rotation+1)-max_rotation)*f );
    m_add_rotation.setPitch(   (rand()%(2*max_rotation+1)-max_rotation)*f );
    m_add_rotation.setRoll(    (rand()%(2*max_rotation+1)-max_rotation)*f );

    // Set invulnerable time, and graphical effects
    float t = m_kart->getKartProperties()->getExplosionInvulnerabilityTime();
    m_kart->setInvulnerableTicks(stk_config->time2Ticks(t));
    m_kart->showStarEffect(t);

    m_kart->getAttachment()->clear();
    // Clear powerups when direct hit in CTF
    if (!from_state)
        addNetworkAnimationChecker(m_reset_ticks != -1);
}   // ExplosionAnimation

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
 *  \param ticks Number of time steps - should be 1.
 */
void ExplosionAnimation::update(int ticks)
{
    float dt = stk_config->ticks2Time(ticks);
    m_velocity -= dt * Track::getCurrentTrack()->getGravity();
    m_xyz = m_xyz + dt * m_velocity * m_normal;

    // Make sure the kart does not end up under the track
    if ((m_xyz - m_orig_xyz).dot(m_normal)<0)
    {
        m_xyz = m_orig_xyz;
        // Don't end the animation if networking for predefined end transform
        if (!NetworkConfig::get()->isNetworking())
            m_timer = -1;
    }
    m_curr_rotation += dt * m_add_rotation;
    btQuaternion q(m_curr_rotation.getHeading(), m_curr_rotation.getPitch(),
                   m_curr_rotation.getRoll());

    if (m_reset_ticks != -1)
    {
        m_reset_xyz = m_reset_xyz + dt * m_velocity * m_reset_normal;
        if ((m_reset_xyz - m_end_transform.getOrigin()).dot(m_reset_normal) <
            0.0f)
            m_reset_xyz = m_end_transform.getOrigin();
    }
    if (m_reset_ticks != -1 && m_timer < m_reset_ticks)
    {
        m_kart->setXYZ(m_reset_xyz);
        m_kart->setRotation(m_end_transform.getRotation());
        m_kart->getBody()->setCenterOfMassTransform(m_kart->getTrans());
    }
    else
    {
        m_kart->setXYZ(m_xyz);
        m_kart->setRotation(q);
    }

    AbstractKartAnimation::update(ticks);
}   // update
