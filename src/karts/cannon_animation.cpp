//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2012 Joerg Henrichs
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

#include "karts/cannon_animation.hpp"

#include "animations/animation_base.hpp"
#include "karts/abstract_kart.hpp"
#include "modes/world.hpp"

#include "LinearMath/btTransform.h"

CannonAnimation::CannonAnimation(AbstractKart *kart, AnimationBase *ab)
             : AbstractKartAnimation(kart)
{
}   // CannonAnimation

// ----------------------------------------------------------------------------
CannonAnimation::~CannonAnimation()
{
}   // ~CannonAnimation

// ----------------------------------------------------------------------------
/** Updates the kart animation.
 *  \param dt Time step size.
 *  \return True if the explosion is still shown, false if it has finished.
 */
void CannonAnimation::update(float dt)
{
    AbstractKartAnimation::update(dt);
}   // update
