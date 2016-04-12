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
#include "modes/world.hpp"
#include "network/rewind_manager.hpp"
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
int KartRewinder::getState(char **buffer) const
{
    const int MEMSIZE = 13*sizeof(float) + 9;

    *buffer = new char[MEMSIZE];
    float* p = (float*)*buffer;
    if(!buffer)
    {
        Log::error("KartRewinder", "Can not allocate %d bytes.", MEMSIZE);
        return -1;
    }

    const btRigidBody *body = m_kart->getBody();

    const btTransform &t = body->getWorldTransform();
    btQuaternion q = t.getRotation();
    memcpy(p+ 0, t.getOrigin(), 3*sizeof(float));
    memcpy(p+ 3, &q, 4*sizeof(float));
    memcpy(p+ 7, body->getLinearVelocity(), 3*sizeof(float));
    memcpy(p+10, body->getAngularVelocity(), 3*sizeof(float));
    m_kart->getControls().copyToMemory((char*)(p+13));
    return MEMSIZE;
}   // getState

// ----------------------------------------------------------------------------
/** Actually rewind to the specified state. */
void KartRewinder::rewindToState(char *buffer)
{
    btTransform t;
    float *p = (float*)buffer;
    t.setOrigin(*(btVector3*)p);
    t.setRotation(*(btQuaternion*)(p+3));
    btRigidBody *body = m_kart->getBody();
    body->proceedToTransform(t);
    body->setLinearVelocity(*(btVector3*)(p+7));
    body->setAngularVelocity(*(btVector3*)(p+10));

    m_kart->getControls().setFromMemory((char*)(p+13));

    return;
}   // rewindToState

// ----------------------------------------------------------------------------
/** Called once a frame. It will add a new kart control event to the rewind
 *  manager if any control values have changed.
 */
void KartRewinder::update()
{
    if(m_kart->getControls() == m_previous_control)
        return;
    m_previous_control = m_kart->getControls();

    char *buffer = new char[m_previous_control.getLength()];
    m_previous_control.copyToMemory(buffer);

    // The rewind manager will free the memory once it's not needed anymore
    RewindManager::get()->addEvent(this, buffer);
}   // update

// ----------------------------------------------------------------------------
void KartRewinder::rewindToEvent(char *p)
{
    m_kart->getControls().setFromMemory(p);
};   // rewindToEvent


