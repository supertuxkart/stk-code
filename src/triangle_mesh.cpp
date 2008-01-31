//  $Id: triangle_mesh.cpp 839 2006-10-24 00:01:56Z hiker $
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
//  but WITHOUT ANY WARRANTY; without even the implied warranty ofati
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

#include "triangle_mesh.hpp"
#include "world.hpp"

// -----------------------------------------------------------------------------
TriangleMesh::~TriangleMesh()
{
    if(m_body)
    {
        world->getPhysics()->removeBody(m_body);
        delete m_body;
        delete m_motion_state;
        delete m_collision_shape;
    }
}   // ~TriangleMesh

// -----------------------------------------------------------------------------
void TriangleMesh::addTriangle(btVector3 t1, btVector3 t2, btVector3 t3,
                               const Material* m)
{
    m_triangleIndex2Material.push_back(m);
    m_mesh.addTriangle(t1, t2, t3);
}   // addTriangle

// -----------------------------------------------------------------------------
void TriangleMesh::createBody(btCollisionObject::CollisionFlags flags)
{
    if(m_triangleIndex2Material.size()==0)
    {
        m_collision_shape = NULL;
        m_motion_state    = NULL;
        m_body            = NULL;
        return;
    }
    // Now convert the triangle mesh into a static rigid body
    m_collision_shape = new btBvhTriangleMeshShape(&m_mesh, false);
    btTransform startTransform;
    startTransform.setIdentity();
    m_motion_state = new btDefaultMotionState(startTransform);
    btRigidBody::btRigidBodyConstructionInfo info(0.0f, m_motion_state, m_collision_shape);
    m_body=new btRigidBody(info);

    world->getPhysics()->addBody(m_body);
    m_user_pointer.set(UserPointer::UP_TRACK, this);
    m_body->setUserPointer(&m_user_pointer);
    m_body->setCollisionFlags(m_body->getCollisionFlags()  | 
                              flags                        |
                              btCollisionObject::CF_CUSTOM_MATERIAL_CALLBACK);
}   // createBody

// -----------------------------------------------------------------------------
