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
#include "karts/controller/controller.hpp"
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
                           std::shared_ptr<RenderInfo> ri)
            : Rewinder(/*can_be_destroyed*/ false)
            , Kart(ident, world_kart_id, position, init_transform, difficulty,
                   ri)
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
/** This function is called immediately before a rewind is done and saves
 *  the current transform for the kart. The difference between this saved
 *  transform and the new transform after rewind is the error that needs
 *  (smoothly) be applied to the graphical position of the kart. 
 */
void KartRewinder::saveTransform()
{
    if (!getKartAnimation())
        Moveable::prepareSmoothing();
}   // saveTransform

// ----------------------------------------------------------------------------
void KartRewinder::computeError()
{
    if (!getKartAnimation())
        Moveable::checkSmoothing();
}   // computeError

// ----------------------------------------------------------------------------
/** Saves all state information for a kart in a memory buffer. The memory
 *  is allocated here and the address returned. It will then be managed
 *  by the RewindManager. The size is used to keep track of memory usage
 *  for rewinding.
 *  \param[out] buffer  Address of the memory buffer.
 *  \returns    Size of allocated memory, or -1 in case of an error.
 */
BareNetworkString* KartRewinder::saveState()
{
    const int MEMSIZE = 17*sizeof(float) + 9+3;

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
    buffer->addFloat(m_vehicle->getMinSpeed());
    buffer->addFloat(m_vehicle->getTimedRotationTime());
    buffer->add(m_vehicle->getTimedRotation());

    // 2) Steering and other player controls
    // -------------------------------------
    getControls().saveState(buffer);
    getController()->saveState(buffer);
    buffer->addTime(m_brake_ticks);

    // 3) Attachment, powerup, nitro
    // -----------------------------
    getAttachment()->saveState(buffer);
    getPowerup()->saveState(buffer);
    buffer->addFloat(getEnergy());

    // 4) Max speed info
    // ------------------
    m_max_speed->saveState(buffer);

    // 5) Skidding
    // -----------
    m_skidding->saveState(buffer);

    return buffer;
}   // saveState

// ----------------------------------------------------------------------------
/** Actually rewind to the specified state. 
 *  \param buffer The buffer with the state info.
 *  \param count Number of bytes that must be used up in this function (not
 *         used).
 */
void KartRewinder::restoreState(BareNetworkString *buffer, int count)
{
    // 1) Physics values: transform and velocities
    // -------------------------------------------
    btTransform t;
    t.setOrigin(buffer->getVec3());
    t.setRotation(buffer->getQuat());
    btRigidBody *body = getBody();
    Vec3 lv = buffer->getVec3();
    Vec3 av = buffer->getVec3();

    // Don't restore to phyics position if showing kart animation
    if (!getKartAnimation())
    {
        body->setLinearVelocity(lv);
        body->setAngularVelocity(av);
        // This function also reads the velocity, so it must be called
        // after the velocities are set
        body->proceedToTransform(t);
        // Update kart transform in case that there are access to its value
        // before Moveable::update() is called (which updates the transform)
        setTrans(t);
    }

    m_has_started = buffer->getUInt8()!=0;   // necessary for startup speed boost
    m_vehicle->setMinSpeed(buffer->getFloat());
    float time_rot = buffer->getFloat();
    // Set timed rotation divides by time_rot
    m_vehicle->setTimedRotation(time_rot, time_rot*buffer->getVec3());
    // For the raycast to determine the current material under the kart
    // the m_hardPointWS of the wheels is used. So after a rewind we
    // must restore the m_hardPointWS to the new values, otherwise they
    // would still point at the kart position at the previous rewind
    // (i.e. different terrain --> different slowdown).
    m_vehicle->updateAllWheelTransformsWS();

    // 2) Steering and other controls
    // ------------------------------
    getControls().rewindTo(buffer);
    getController()->rewindTo(buffer);
    m_brake_ticks = buffer->getTime();

    // 3) Attachment, powerup, nitro
    // ------------------------------
    getAttachment()->rewindTo(buffer);
    getPowerup()->rewindTo(buffer);
    float nitro = buffer->getFloat();
    setEnergy(nitro);

    // 5) Max speed info
    // ------------------
    m_max_speed->rewindTo(buffer);

    // 6) Skidding
    // -----------
    m_skidding->rewindTo(buffer);
    return;
}   // restoreState

// ----------------------------------------------------------------------------
/** Called once a frame. It will add a new kart control event to the rewind
 *  manager if any control values have changed.
 */
void KartRewinder::update(int ticks)
{
    Kart::update(ticks);
}   // update

// ----------------------------------------------------------------------------
void KartRewinder::rewindToEvent(BareNetworkString *buffer)
{
};   // rewindToEvent


