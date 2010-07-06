//  $Id: triangle_mesh.hpp 839 2006-10-24 00:01:56Z hiker $
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

#ifndef HEADER_TRIANGLE_MESH_HPP
#define HEADER_TRIANGLE_MESH_HPP

#include <vector>
#include "btBulletDynamicsCommon.h"

#include "physics/user_pointer.hpp"

class Material;

/** 
 * \brief A special class to store a triangle mesh with a separate material per triangle.
 * \ingroup physics
 */
class TriangleMesh
{
private:
    UserPointer                  m_user_pointer;
    std::vector<const Material*> m_triangleIndex2Material;
    btRigidBody                 *m_body;
    btTriangleMesh               m_mesh;
    btDefaultMotionState        *m_motion_state;
    btCollisionShape            *m_collision_shape;
public:
         TriangleMesh();
        ~TriangleMesh();
    void addTriangle(const btVector3 &t1, const btVector3 &t2, 
                     const btVector3 &t3, const Material* m);
    void createBody(btCollisionObject::CollisionFlags flags=
                         (btCollisionObject::CollisionFlags)0);
    void removeBody();
    const Material* getMaterial(int n) const {return m_triangleIndex2Material[n];}
};
#endif
/* EOF */

