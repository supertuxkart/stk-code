//  $Id$
//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2006 Joerg Henrichs
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

#ifndef HEADER_PHYSICAL_OBJECT_HPP
#define HEADER_PHYSICAL_OBJECT_HPP

#include <string>

#include "btBulletDynamicsCommon.h"

#include "physics/user_pointer.hpp"
#include "tracks/track_object.hpp"
#include "utils/vec3.hpp"

class XMLNode;

/**
  * \ingroup physics
  */
class PhysicalObject : public TrackObject
{
public:
    /** The supported collision shapes. */
    enum bodyTypes {MP_NONE, 
                    MP_CONE_Y, MP_CONE_X, MP_CONE_Z,
                    MP_CYLINDER_Y, MP_CYLINDER_X, MP_CYLINDER_Z,
                    MP_BOX, MP_SPHERE};

private:
    /** The shape of this object. */
    bodyTypes             m_body_type;

    /** The bullet collision shape. */
    btCollisionShape     *m_shape;

    /** The corresponding bullet rigid body. */
    btRigidBody          *m_body;

    /** Bullet's motion state for this object. */
    btDefaultMotionState *m_motion_state;

    /** The mass of this object. */
    float                 m_mass;

    /** The pointer that is stored in the bullet rigid body back to 
     *  this object. */
    UserPointer           m_user_pointer;

    /** This is the initial position of the object for the physics. */
    btTransform           m_init_pos;

    /** Radius of the object - this obviously depends on the actual shape.
     *  As a default the radius is being determined from the shape of the
     *  mesh, but in somce cases that could lead to incorrect results
     *  (if the mesh does not closely resemble a sphere, see init() for 
     *  details, but is supposed to be a sphere). In this case the radius
     *  can be set in the scene file. */
    float                 m_radius;

    /** True if a kart colliding with this object should be rescued. */
    bool                  m_crash_reset;

public:
                 PhysicalObject (const XMLNode &node);
    
                 PhysicalObject(const std::string& model,
                                bodyTypes shape, float mass, float radius,
                                const core::vector3df& hpr,
                                const core::vector3df& pos,
                                const core::vector3df& scale);
    
    virtual     ~PhysicalObject (); 
    virtual void reset          ();
    virtual void handleExplosion(const Vec3& pos, bool directHit);
    void         update         (float dt);
    void         init           ();
    // ------------------------------------------------------------------------
    /** Returns the rigid body of this physical object. */
    btRigidBody *getBody        ()          { return m_body; }
    // ------------------------------------------------------------------------
    /** Returns true if this object should trigger a rescue in a kart that
     *  hits it. */
    bool isCrashReset() const { return m_crash_reset; }
};  // PhysicalObject

#endif
/* EOF */

