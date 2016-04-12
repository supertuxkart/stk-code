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

#ifndef HEADER_RESCUE_ANIMATION_HPP
#define HEADER_RESCUE_ANIMATION_HPP

#include "karts/abstract_kart_animation.hpp"
#include "utils/vec3.hpp"

class AbstractKart;
class Referee;

/** This triggers a rescue of the specified kart.
 * \ingroup karts
 */
class RescueAnimation: public AbstractKartAnimation
{
protected:
    /** The coordinates where the kart was hit originally. */
    Vec3 m_xyz;

    /** The kart's current rotation. */
    Vec3 m_curr_rotation;

    /** The artificial rotation to toss the kart around. It's in units
     *  of rotation per second. */
    Vec3 m_add_rotation;

    /** The velocity with which the kart is moved. */
    float m_velocity;

    /** Duration for the animation. This can potentially be set
     *  with different values for different karts, or depending
     *  on difficulty (so that on easy you can drive again earlier. */
    float m_duration;

    /** The referee during a rescue operation. */
    Referee      *m_referee;

public:
                 RescueAnimation(AbstractKart *kart, bool is_auto_rescue=false);
    virtual     ~RescueAnimation();
    virtual void update(float dt);
};   // RescueAnimation
#endif
