//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2010-2015 Joerg Henrichs
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

#include "karts/abstract_kart_animation.hpp"

#include "graphics/slip_stream.hpp"
#include "items/powerup.hpp"
#include "karts/abstract_kart.hpp"
#include "karts/kart_model.hpp"
#include "karts/skidding.hpp"
#include "modes/world.hpp"
#include "network/network_string.hpp"
#include "physics/btKart.hpp"
#include "physics/physics.hpp"
#include "physics/triangle_mesh.hpp"
#include "tracks/track.hpp"
#include "mini_glm.hpp"

#include <limits>

/** Constructor. Note that kart can be NULL in case that the animation is
 *  used for a basket ball in a cannon animation.
 *  \param kart Pointer to the kart that is animated, or NULL if the
 *         the animation is meant for a basket ball etc.
 */
AbstractKartAnimation::AbstractKartAnimation(AbstractKart* kart,
                                             const std::string &name)
{
    m_kart = kart;
    m_name = name;
    m_created_ticks = World::getWorld()->getTicksSinceStart();
    m_created_transform = btTransform(btQuaternion(0.0f, 0.0f, 0.0f, 1.0f));
    // Remove previous animation if there is one
#ifndef DEBUG
    // Use this code in non-debug mode to avoid a memory leak (and messed
    // up animations) if this should happen. In debug mode this condition
    // is caught by setKartAnimation(), and useful error messages are
    // printed
    if (kart && kart->getKartAnimation())
    {
        AbstractKartAnimation* ka = kart->getKartAnimation();
        kart->setKartAnimation(NULL);
        delete ka;
    }
#endif
    // Register this animation with the kart (which will free it
    // later).
    if (kart)
    {
        m_created_transform = kart->getTrans();
        kart->setKartAnimation(this);
        Physics::get()->removeKart(m_kart);
        kart->getSkidding()->reset();
        kart->getSlipstream()->reset();
        if (kart->isSquashed())
        {
            // A time of 0 reset the squashing
            kart->setSquash(0.0f, 0.0f);
        }
    }
    MiniGLM::compressbtTransform(m_created_transform,
        m_created_transform_compressed);
}   // AbstractKartAnimation

// ----------------------------------------------------------------------------
AbstractKartAnimation::~AbstractKartAnimation()
{
    // If m_end_ticks != int max, this object is deleted because the kart
    // is deleted (at the end of a race), which means that
    // world is in the process of being deleted. In this case
    // we can't call getPhysics() anymore.
    if (m_end_ticks != std::numeric_limits<int>::max() && m_kart)
    {
        Vec3 linear_velocity = m_kart->getBody()->getLinearVelocity();
        Vec3 angular_velocity = m_kart->getBody()->getAngularVelocity();
        // Use getTrans so the setXYZ and setRotation from subclass result
        // can be used here
        btTransform transform = m_kart->getTrans();
        m_kart->getBody()->setLinearVelocity(linear_velocity);
        m_kart->getBody()->setAngularVelocity(angular_velocity);
        m_kart->getBody()->proceedToTransform(transform);
        m_kart->setTrans(transform);
        // Reset all btKart members (bounce back ticks / rotation ticks..)
        m_kart->getVehicle()->reset();
        Physics::get()->addKart(m_kart);
    }
}   // ~AbstractKartAnimation

// ----------------------------------------------------------------------------
/** In CTF mode call this to reset kart powerup when get hit.
 */
void AbstractKartAnimation::resetPowerUp()
{
    if (m_kart)
        m_kart->getPowerup()->reset();
}   // resetPowerUp

// ----------------------------------------------------------------------------
/** Updates the timer, and if it expires, the kart animation will be
 *  removed from the kart and this object will be deleted.
 *  NOTE: calling this function must be the last thing done in any kart
 *  animation class, since this object might be deleted, so accessing any
 *  members might be invalid.
 *  \param ticks Number of time steps - should be 1.
 */
void AbstractKartAnimation::update(int ticks)
{
    // See if the timer expires, if so return the kart to normal game play
    World* w = World::getWorld();
    if (!w)
        return;

    if (m_end_ticks - w->getTicksSinceStart() == 0)
    {
        if (m_kart)
            m_kart->setKartAnimation(NULL);
        delete this;
    }
}   // update

// ----------------------------------------------------------------------------
void AbstractKartAnimation::updateGraphics(float dt)
{
    // Reset the wheels (and any other animation played for that kart)
    // This avoid the effect that some wheels might be way below the kart
    // which is very obvious in the rescue animation.
    if (m_kart)
        m_kart->getKartModel()->resetVisualWheelPosition();
}   // updateGraphics

// ----------------------------------------------------------------------------
/** Returns the current animation timer.
  */
float AbstractKartAnimation::getAnimationTimer() const
{
    World* w = World::getWorld();
    if (!w)
        return 0.0f;
    return stk_config->ticks2Time(m_end_ticks - w->getTicksSinceStart());
}   // getAnimationTimer

// ----------------------------------------------------------------------------
/** Determine maximum rescue height with up-raycast
 */
float AbstractKartAnimation::getMaximumHeight(const Vec3& up_vector,
                                              float height_remove)
{
    float hit_dest = 9999999.9f;
    Vec3 hit;
    const Material* m = NULL;
    Vec3 to = up_vector * 10000.0f;
    const TriangleMesh &tm = Track::getCurrentTrack()->getTriangleMesh();
    if (tm.castRay(m_created_transform.getOrigin(), to, &hit, &m,
        NULL/*normal*/, /*interpolate*/true))
    {
        hit_dest = (hit - m_created_transform.getOrigin()).length();
        hit_dest -= height_remove;
        if (hit_dest < 1.0f)
        {
            hit_dest = 1.0f;
        }
    }
    return hit_dest;
}   // getMaximumHeight

// ----------------------------------------------------------------------------
void AbstractKartAnimation::saveState(BareNetworkString* buffer)
{
    buffer->addUInt32(m_created_ticks);
    buffer->addInt24(m_created_transform_compressed[0])
        .addInt24(m_created_transform_compressed[1])
        .addInt24(m_created_transform_compressed[2])
        .addUInt32(m_created_transform_compressed[3]);
}   // saveState

// ----------------------------------------------------------------------------
/** Used in constructor of sub-class as no virtual function can be used there.
 */
void AbstractKartAnimation::restoreBasicState(BareNetworkString* buffer)
{
    m_created_ticks = buffer->getUInt32();
    m_created_transform_compressed[0] = buffer->getInt24();
    m_created_transform_compressed[1] = buffer->getInt24();
    m_created_transform_compressed[2] = buffer->getInt24();
    m_created_transform_compressed[3] = buffer->getUInt32();
    m_created_transform =
        MiniGLM::decompressbtTransform(m_created_transform_compressed);
}   // restoreBasicState
