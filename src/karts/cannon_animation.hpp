//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2012-2015 Joerg Henrichs
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

#ifndef HEADER_CANNON_ANIMATION_HPP
#define HEADER_CANNON_ANIMATION_HPP

#include "karts/abstract_kart_animation.hpp"
#include "utils/vec3.hpp"

#include "LinearMath/btQuaternion.h"

class AbstractKart;
class AnimationBase;
class Ipo;


/** This animation shoots the kart to a specified point on the track.
 *
 * \ingroup karts
 */

class CannonAnimation: public AbstractKartAnimation
{
protected:
    /** This is the difference between the position of the kart when the
     *  cannon line is crossed and the curve interpolation at t=0. This
     *  is added to each interpolated curve value to give the final
     *  kart position (so the kart moves relative to the curve). */
    Vec3           m_delta;

    /** The amount of rotation to be applied to m_delta so that it keeps
     *  being on the 'right' side of the curve. */
    btQuaternion m_delta_rotation;

    /** Stores the curve interpolation for the cannon. */
    AnimationBase *m_curve;

    /** This stores the original (unmodified) interpolated curve value. THis
     *  is used to determine the orientation of the kart. */
    Vec3           m_previous_orig_xyz;

public:
             CannonAnimation(AbstractKart *kart, Ipo *ipo,
                             const Vec3 &start_left, const Vec3 &start_right,
                             const Vec3 &end_left, const Vec3 &end_right);
    virtual ~CannonAnimation();
    virtual void  update(float dt);

};   // CannonAnimation
#endif
