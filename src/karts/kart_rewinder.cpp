//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2013 Joerg Henrichs
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

#include "karts/kart_rewinder.hpp"

#include "items/attachment.hpp"
#include "items/powerup.hpp"
#include "karts/abstract_kart.hpp"
#include "karts/max_speed.hpp"
#include "karts/skidding.hpp"
#include "modes/world.hpp"
#include "network/rewind_manager.hpp"
#include "network/network_string.hpp"
#include "physics/btKart.hpp"
#include "utils/vec3.hpp"

#include <string.h>

KartRewinder::KartRewinder(const std::string& ident,unsigned int world_kart_id,
                           int position, const btTransform& init_transform,
                           PerPlayerDifficulty difficulty,
                           KartRenderType krt)
            : Rewinder(/*can_be_destroyed*/ false)
            , Kart(ident, world_kart_id, position, init_transform, difficulty,
                   krt)
{
}   // KartRewinder

// ----------------------------------------------------------------------------
/** Resets status in case of a resetart.
 */
void KartRewinder::reset()
{
    Kart::reset();
    Rewinder::reset();
}   // reset

// ----------------------------------------------------------------------------
/** Saves all state information for a kart in a memory buffer. The memory
 *  is allocated here and the address returned. It will then be managed
 *  by the RewindManager. The size is used to keep track of memory usage
 *  for rewinding.
 *  \param[out] buffer  Address of the memory buffer.
 *  \returns    Size of allocated memory, or -1 in case of an error.
 */
BareNetworkString* KartRewinder::saveState() const
{
    const int MEMSIZE = 13*sizeof(float) + 9+3;

    BareNetworkString *buffer = new BareNetworkString(MEMSIZE);
    const btRigidBody *body = getBody();

    // 1) Physics values: transform and velocities
    // -------------------------------------------
    const btTransform &t = body->getWorldTransform();
    buffer->add(t.getOrigin());
    btQuaternion q = t.getRotation();
    buffer->add(q);
    buffer->add(body->getLinearVelocity());
    buffer->add(body->getAngularVelocity());
    buffer->addUInt8(m_has_started);   // necessary for startup speed boost
    buffer->addFloat(m_vehicle->getInstantSpeedIncrease());

    // 2) Steering and other player controls
    // -------------------------------------
    getControls().copyToBuffer(buffer);

    // 3) Attachment
    // -------------
    getAttachment()->saveState(buffer);

    // 4) Powerup
    // ----------
    getPowerup()->saveState(buffer);

    // 5) Max speed info
    // ------------------
    m_max_speed->saveState(buffer);

    // 6) Skidding
    // -----------
    m_skidding->saveState(buffer);

    return buffer;
}   // saveState

// ----------------------------------------------------------------------------
/** Actually rewind to the specified state. */
void KartRewinder::rewindToState(BareNetworkString *buffer)
{
    buffer->reset();   // make sure the buffer is read from the beginning

    // 1) Physics values: transform and velocities
    // -------------------------------------------
    btTransform t;
    t.setOrigin(buffer->getVec3());
    t.setRotation(buffer->getQuat());
    btRigidBody *body = getBody();
    body->setLinearVelocity(buffer->getVec3());
    body->setAngularVelocity(buffer->getVec3());
    // This function also reads the velocity, so it must be called
    // after the velocities are set
    body->proceedToTransform(t);
    // Update kart transform in case that there are access to its value
    // before Moveable::update() is called (which updates the transform)
    setTrans(t);
    m_has_started = buffer->getUInt8()!=0;   // necessary for startup speed boost
    m_vehicle->instantSpeedIncreaseTo(buffer->getFloat());

    // 2) Steering and other controls
    // ------------------------------
    getControls().setFromBuffer(buffer);

    // 3) Attachment
    // -------------
    getAttachment()->rewindTo(buffer);

    // 4) Powerup
    // ----------
    getPowerup()->rewindTo(buffer);

    // 5) Max speed info
    // ------------------
    m_max_speed->rewindTo(buffer);
    m_max_speed->update(0);

    // 6) Skidding
    // -----------
    m_skidding->rewindTo(buffer);
    return;
}   // rewindToState

// ----------------------------------------------------------------------------
/** Called once a frame. It will add a new kart control event to the rewind
 *  manager if any control values have changed.
 */
void KartRewinder::update(float dt)
{
    Kart::update(dt);
}   // update

// ----------------------------------------------------------------------------
void KartRewinder::rewindToEvent(BareNetworkString *buffer)
{
};   // rewindToEvent


