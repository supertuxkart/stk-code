//  $Id: billboard_animation.cpp 1681 2008-04-09 13:52:48Z hikerstk $
//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2009  Joerg Henrichs
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

#include "animations/billboard_animation.hpp"

class XMLNode;

/** A 2d billboard animation. */
BillboardAnimation::BillboardAnimation(const Track &track_name, 
                                       const XMLNode &node)
                  : AnimationBase(node)
{
}   // BillboardAnimation

// ----------------------------------------------------------------------------
/** Update the animation, called one per time step.
 *  \param dt Time since last call. */
void BillboardAnimation::update(float dt)
{
    // FIXME: not implemented yet.
    core::vector3df xyz(0, 0, 0);
    core::vector3df hpr(0, 0, 0);
    core::vector3df scale(1,1,1);
    AnimationBase::update(dt, &xyz, &hpr, &scale);

}   // update
