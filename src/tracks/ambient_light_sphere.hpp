//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2009-2015  Joerg Henrichs
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

#ifndef HEADER_AMBIENT_LIGHT_SPHERE_HPP
#define HEADER_AMBIENT_LIGHT_SPHERE_HPP

#include <SColor.h>
using namespace irr;

#include "tracks/check_sphere.hpp"

class XMLNode;
class CheckManager;

/**
 * \brief This class implements a check sphere that is used to change the ambient
 *  light if a kart is inside this sphere.
 *
 *  Besides a normal radius this sphere also has a 2nd 'inner' radius: player karts
 *  inside the inner radius will have the full new ambient light, karts outside the
 *  default light, and karts in between will mix the light dependent on distance.
 *
 * \ingroup tracks
 */
class AmbientLightSphere : public CheckSphere
{
private:
    /** The inner radius defines the area during which the ambient light
     *  is extrapolated. The square of the value specified in the scene
     *  file is stored. */
    float         m_inner_radius2;

    /** THe full ambient color to use once the kart is inside the
     *  inner radius. */
    video::SColor m_ambient_color;
public:
                  AmbientLightSphere(const XMLNode &node, unsigned int index);
    virtual      ~AmbientLightSphere() {};
    virtual void  update(float dt);
    virtual bool  isTriggered(const Vec3 &old_pos, const Vec3 &new_pos,
                              unsigned int indx);
};   // AmbientLightSphere

#endif

