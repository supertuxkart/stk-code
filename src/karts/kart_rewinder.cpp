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
/** Saves all state information for a kart in a memory buffer. The memory
 *  is allocated here and the address returned. It will then be managed
 *  by the RewindManager. The size is used to keep track of memory usage
 *  for rewinding.
 *  \param[out] buffer  Address of the memory buffer.
 *  \returns    Size of allocated memory, or -1 in case of an error.
 */
BareNetworkString* KartRewinder::getState() const
{
    const int MEMSIZE = 13*sizeof(float) + 9;

    BareNetworkString *buffer = new BareNetworkString(MEMSIZE);
    const btRigidBody *body = m_kart->getBody();

    const btTransform &t = body->getWorldTransform();
    buffer->add(t.getOrigin());
    btQuaternion q = t.getRotation();
    buffer->add(q);
    buffer->add(body->getLinearVelocity());
    buffer->add(body->getAngularVelocity());

    // Attachment
    Attachment::AttachmentType atype = m_kart->getAttachment()->getType();
    //buffer->addUInt8(uint8_t(atype));
    if(atype!=Attachment::ATTACH_NOTHING)
    {
        //buffer->addFloat(m_kart->getAttachment()->getTimeLeft());
    }

    // Steering information
    m_kart->getControls().copyToBuffer(buffer);

    return buffer;
}   // getState

// ----------------------------------------------------------------------------
/** Actually rewind to the specified state. */
void KartRewinder::rewindToState(BareNetworkString *buffer)
{
    buffer->reset();   // make sure the buffer is read from the beginning
    btTransform t;
    t.setOrigin(buffer->getVec3());
    t.setRotation(buffer->getQuat());
    btRigidBody *body = m_kart->getBody();
    body->proceedToTransform(t);

    body->setLinearVelocity(buffer->getVec3());
    body->setAngularVelocity(buffer->getVec3());

    m_kart->getControls().setFromBuffer(buffer);

    return;
}   // rewindToState

// ----------------------------------------------------------------------------
/** Called once a frame. It will add a new kart control event to the rewind
 *  manager if any control values have changed.
 */
void KartRewinder::update()
{
    if(m_kart->getControls() == m_previous_control ||
        RewindManager::get()->isRewinding())
        return;
    m_previous_control = m_kart->getControls();

    BareNetworkString *buffer = new BareNetworkString(m_previous_control.getLength());
    m_previous_control.copyToBuffer(buffer);

    // The rewind manager will free the memory once it's not needed anymore
    RewindManager::get()->addEvent(this, buffer);
}   // update

// ----------------------------------------------------------------------------
void KartRewinder::rewindToEvent(BareNetworkString *buffer)
{
    m_kart->getControls().setFromBuffer(buffer);
};   // rewindToEvent


