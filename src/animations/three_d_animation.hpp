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

#ifndef HEADER_THREE_D_ANIMATION_HPP
#define HEADER_THREE_D_ANIMATION_HPP

#include <string>

#include <vector3d.h>
using namespace irr;

#include "btBulletDynamicsCommon.h"

#include "animations/animation_base.hpp"
#include "physics/user_pointer.hpp"

class BezierCurve;
class XMLNode;

/** \brief A virtual base class for all animations.
  * \ingroup animations
  */
class ThreeDAnimation : public AnimationBase
{
private:
    /** The bullet collision shape for the physics. */
    btCollisionShape     *m_collision_shape;

    /** The bullet rigid body. */
    btRigidBody          *m_body;

    /** Motion state of the physical object. */
    btMotionState        *m_motion_state;

    /** A user pointer to connect a bullet body with this object. */
    UserPointer           m_user_pointer;

    /** Non-null only if the shape is exact */
    TriangleMesh         *m_triangle_mesh;

    /** True if a collision with this object should trigger 
     *  rescuing a kart. */
    bool                  m_crash_reset;
    
    /** We have to store the rotation value as computed in blender, since
     *  irrlicht uses a different order, so for rotation animations we
     *  can not use the value returned by getRotation from a scene node. */
    Vec3                  m_hpr;

    void createPhysicsBody(const std::string &shape);

public:
                 ThreeDAnimation(const XMLNode &node);
    virtual     ~ThreeDAnimation();
    virtual void update(float dt);
    // ------------------------------------------------------------------------
    /** Returns true if a collision with this object should 
     * trigger a rescue. */
    bool isCrashReset() const { return m_crash_reset; }

};   // ThreeDAnimation
#endif

