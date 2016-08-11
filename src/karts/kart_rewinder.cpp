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
#include "karts/abstract_kart.hpp"
#include "items/attachment.hpp"
#include "items/powerup.hpp"
#include "modes/world.hpp"
#include "network/rewind_manager.hpp"
#include "network/network_string.hpp"
#include "utils/vec3.hpp"

#include <string.h>

KartRewinder::KartRewinder(AbstractKart *kart) : Rewinder(/*can_be_destroyed*/ false)
{
    m_kart = kart;
}   // KartRewinder

// ----------------------------------------------------------------------------
/** Resets status in case of a resetart.
 */
void KartRewinder::reset()
{
    m_previous_control    = m_kart->getControls();
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
    const int MEMSIZE = 13*sizeof(float) + 9+2;

    BareNetworkString *buffer = new BareNetworkString(MEMSIZE);
    const btRigidBody *body = m_kart->getBody();

    // 1) Physics values: transform and velocities
    // -------------------------------------------
    const btTransform &t = body->getWorldTransform();
    buffer->add(t.getOrigin());
    btQuaternion q = t.getRotation();
    buffer->add(q);
    buffer->add(body->getLinearVelocity());
    buffer->add(body->getAngularVelocity());

    // 2) Steering and other player controls
    // -------------------------------------
    m_kart->getControls().copyToBuffer(buffer);

    // 3) Attachment
    // -------------
    m_kart->getAttachment()->saveState(buffer);

    // 4) Powerup
    // ----------
    m_kart->getPowerup()->saveState(buffer);
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
    btRigidBody *body = m_kart->getBody();
    body->proceedToTransform(t);
    body->setLinearVelocity(buffer->getVec3());
    body->setAngularVelocity(buffer->getVec3());

    // 2) Steering and other controls
    // ------------------------------
    m_kart->getControls().setFromBuffer(buffer);

    // 3) Attachment
    // -------------
    m_kart->getAttachment()->rewindTo(buffer);

    // 4) Powerup
    // ----------
    m_kart->getPowerup()->rewindTo(buffer);
    return;
}   // rewindToState

// ----------------------------------------------------------------------------
/** Called once a frame. It will add a new kart control event to the rewind
 *  manager if any control values have changed.
 */
void KartRewinder::update()
{
    // Don't store events from a rewind
    if(RewindManager::get()->isRewinding()) return;

    // Check if an event has happened that needs to be recorded
    bool control_event = !(m_kart->getControls()   == m_previous_control);
    uint8_t type = (control_event ? EVENT_CONTROL : 0);
    if(type == 0)
        return;   // no event

    // Likely it is only a single event, so limit initial memory allocation
    BareNetworkString *buffer = 
                      new BareNetworkString(m_previous_control.getLength());
    buffer->addUInt8(type);
    if(control_event)
    {
        m_previous_control = m_kart->getControls();
        m_previous_control.copyToBuffer(buffer);
    }
    // The rewind manager will free the memory once it's not needed anymore
    //XXXXXXXXXXXXXXX RewindManager::get()->addEvent(this, buffer);
}   // update

// ----------------------------------------------------------------------------
void KartRewinder::rewindToEvent(BareNetworkString *buffer)
{
    buffer->reset();
    uint8_t type = buffer->getUInt8();
    if(type & EVENT_CONTROL)
        m_kart->getControls().setFromBuffer(buffer);
};   // rewindToEvent


