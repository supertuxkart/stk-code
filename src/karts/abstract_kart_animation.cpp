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

#include "karts/abstract_kart.hpp"
#include "karts/abstract_kart_animation.hpp"

AbstractKartAnimation::AbstractKartAnimation(AbstractKart *kart)
{
    m_timer = 0;
    m_kart  = kart;
    // Register this animation with the kart (which will free it
    // later).
    kart->setKartAnimation(this);
}   // AbstractKartAnimation

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
        m_kart->setKartAnimation(NULL);
        delete this;
    }
}   // update
