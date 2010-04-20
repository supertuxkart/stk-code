//  $Id$
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

#include "karts/explosion_animation.hpp"

#include "karts/kart.hpp"
#include "modes/world.hpp"
#include "tracks/track.hpp"
#include "utils/constants.hpp"

Vec3 m_add_rotation;
Vec3 m_curr_rotation;

/** The constructor stores a pointer to the kart this object is animating,
 *  and initialised the timer.
 *  \param kart Pointer to the kart which is animated.
 */
ExplosionAnimation::ExplosionAnimation(Kart *kart)
{
    m_timer = -1;
    m_kart  = kart;
};   // ExplosionAnimation

// ----------------------------------------------------------------------------
/** Starts an explosion animation.
 *  \param pos The coordinates of the explosion.
 *  \param direct_hig True if the kart was hit directly --> maximal impact.
 */
void ExplosionAnimation::handleExplosion(const Vec3 &pos, bool direct_hit)
{
    // Avoid doing another explosion while a kart is thrown around in the air.
    if(m_timer>=0) return;

    m_timer = 0;
    m_xyz   = m_kart->getXYZ();

    float t = m_kart->getKartProperties()->getExplosionTime();

    // Half of the overall time is spent in raising, so only use
    // half of the explosion time here.
    // Velocity after t seconds is:
    // v(t) = m_up_velocity + t*gravity
    // Since v(explosion_time*0.5) = 0, the following forumla computes 
    // the right initial velocity for a kart to land back after
    // the specified time.
    m_up_velocity = 0.5f * t * World::getWorld()->getTrack()->getGravity();
    World::getWorld()->getPhysics()->removeKart(m_kart);
    
    m_curr_rotation.setHPR(m_kart->getRotation());
    const int max_rotation = direct_hit ? 2 : 1;
    // To get rotations in botb directions for each axis we determine a random
    // number between -(max_rotation-1) and +(max_rotation-1)
    float f=2.0f*M_PI/t;
    m_add_rotation.setHeading( (rand()%(2*max_rotation+1)-max_rotation)*f );
    m_add_rotation.setPitch(   (rand()%(2*max_rotation+1)-max_rotation)*f );
    m_add_rotation.setRoll(    (rand()%(2*max_rotation+1)-max_rotation)*f );

}   // handleExplosion

// ----------------------------------------------------------------------------
/** Updates the explosion animation.
 *  \param dt Time step size.
 *  \return True if the explosion is still shown, false if it has finished.
 */
bool ExplosionAnimation::update(float dt)
{
    assert(m_timer>=0);
    m_timer += dt;
    if(m_timer>m_kart->getKartProperties()->getExplosionTime())
    {
        m_timer=-1;
        World::getWorld()->getPhysics()->addKart(m_kart);
        m_kart->getBody()->setLinearVelocity(btVector3(0,0,0));
        m_kart->getBody()->setAngularVelocity(btVector3(0,0,0));
        return false;
    }
    m_up_velocity -= dt*World::getWorld()->getTrack()->getGravity();
    m_xyz.setY(m_xyz.getY()+m_up_velocity*dt);
    m_kart->setXYZ(m_xyz);
    m_curr_rotation += dt*m_add_rotation;
    btQuaternion q(m_curr_rotation.getHeading(), m_curr_rotation.getPitch(),
        m_curr_rotation.getRoll());
    m_kart->setRotation(q);
    return true;
}   // update