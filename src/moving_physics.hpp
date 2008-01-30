//  $Id: moving_physics.hpp 839 2006-10-24 00:01:56Z hiker $
//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2006 Joerg Henrichs
//
//  This program is free software; you can redistribute it and/or
//  modify it under the terms of the GNU General Public License
//  as published by the Free Software Foundation; either version 2
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

#ifndef HEADER_MOVING_PHYSICS_H
#define HEADER_MOVING_PHYSICS_H
#include <plib/ssg.h>
#include "btBulletDynamicsCommon.h"
#include "callback.hpp"
#include <string>

class MovingPhysics : public ssgTransform, public Callback
{
public:
    enum bodyTypes {BODY_NONE, BODY_CONE, BODY_BOX};

protected:
    bodyTypes             m_body_type;
    btCollisionShape     *m_shape;
    btRigidBody          *m_body;
    btDefaultMotionState *m_motion_state;
    float                 m_half_height;
    float                 m_mass;
public:
    MovingPhysics           (const std::string data);
    ~MovingPhysics          (); 
    void update             (float dt);
    void         init       ();
    const char  *getTypeName()          {return "moving physics";}
};  // MovingPhysics

#endif
/* EOF */

