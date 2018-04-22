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
#include "karts/abstract_kart.hpp"
#include "karts/kart_model.hpp"
#include "karts/skidding.hpp"
#include "physics/physics.hpp"

/** Constructor. Note that kart can be NULL in case that the animation is
 *  used for a basket ball in a cannon animation.
 *  \param kart Pointer to the kart that is animated, or NULL if the
 *         the animation is meant for a basket ball etc.
 */
AbstractKartAnimation::AbstractKartAnimation(AbstractKart *kart,
                                             const std::string &name)
{
    m_timer = 0;
    m_kart  = kart;
    m_name  = name;

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
        kart->setKartAnimation(this);
        Physics::getInstance()->removeKart(m_kart);
        kart->getSkidding()->reset();
        kart->getSlipstream()->reset();
        if (kart->isSquashed())
        {
            // A time of 0 reset the squashing
            kart->setSquash(0.0f, 0.0f);
        }

        // Reset the wheels (and any other animation played for that kart)
        // This avoid the effect that some wheels might be way below the kart
        // which is very obvious in the rescue animation.
        m_kart->getKartModel()->resetVisualWheelPosition();
    }
}   // AbstractKartAnimation

// ----------------------------------------------------------------------------
AbstractKartAnimation::~AbstractKartAnimation()
{
    // If m_timer >=0, this object is deleted because the kart
    // is deleted (at the end of a race), which means that
    // world is in the process of being deleted. In this case
    // we can't call getPhysics() anymore.
    if(m_timer < 0 && m_kart)
    {
        m_kart->getBody()->setAngularVelocity(btVector3(0,0,0));
        Physics::getInstance()->addKart(m_kart);
    }
}   // ~AbstractKartAnimation

// ----------------------------------------------------------------------------
/** Updates the timer, and if it expires (<0), the kart animation will be
 *  removed from the kart and this object will be deleted.
 *  NOTE: calling this function must be the last thing done in any kart
 *  animation class, since this object might be deleted, so accessing any
 *  members might be invalid.
 *  \param dt Time step size.
 */
void AbstractKartAnimation::update(float dt)
{
    // See if the timer expires, if so return the kart to normal game play
    m_timer -= dt;
    if(m_timer<0)
    {
        if(m_kart) m_kart->setKartAnimation(NULL);
        delete this;
    }
}   // update
