//  $Id: triangle_mesh.cpp 839 2006-10-24 00:01:56Z hiker $
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
//  but WITHOUT ANY WARRANTY; without even the implied warranty ofati
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

#include "physics/triangle_mesh.hpp"

#include "modes/world.hpp"

// -----------------------------------------------------------------------------
/** Constructor: Initialises all data structures with zero.
 */
TriangleMesh::TriangleMesh() : m_mesh()
{
    m_body            = NULL;
    m_motion_state    = NULL;
    m_collision_shape = NULL;
}   // TriangleMesh

// -----------------------------------------------------------------------------
/** Destructor: delete all allocated data structures.
 */
TriangleMesh::~TriangleMesh()
{
    if(m_body)
    {
        World::getWorld()->getPhysics()->removeBody(m_body);
        delete m_body;
        delete m_motion_state;
        delete m_collision_shape;
    }
}   // ~TriangleMesh

// -----------------------------------------------------------------------------
void TriangleMesh::addTriangle(const btVector3 &t1, const btVector3 &t2, 
                               const btVector3 &t3, const Material* m)
{
    m_triangleIndex2Material.push_back(m);
    m_mesh.addTriangle(t1, t2, t3);
}   // addTriangle

// -----------------------------------------------------------------------------
/** Creates the physics body for this triangle mesh. If the body already
 *  exists (because it was created by a previous call to createBody)
 *  it is first removed from the world. This is used by loading the track
 *  where a physics body is used to determine the height of terrain. To have
 *  an optimised rigid body including all static objects, the track is then
 *  removed and all objects together with the track is converted again into
 *  a single rigid body. This avoids using irrlicht (or the graphics engine)
 *  for height of terrain detection).
 */
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
    m_collision_shape = new btBvhTriangleMeshShape(&m_mesh, true);
    btTransform startTransform;
    startTransform.setIdentity();
    m_motion_state = new btDefaultMotionState(startTransform);
    btRigidBody::btRigidBodyConstructionInfo info(0.0f, m_motion_state, m_collision_shape);
    m_body=new btRigidBody(info);

    World::getWorld()->getPhysics()->addBody(m_body);
    m_user_pointer.set(this);
    m_body->setUserPointer(&m_user_pointer);
    m_body->setCollisionFlags(m_body->getCollisionFlags()  | 
                              flags                        |
                              btCollisionObject::CF_CUSTOM_MATERIAL_CALLBACK);
}   // createBody

// -----------------------------------------------------------------------------
/** Removes the created body from the physics world. This is used when creating
 *  a temporary rigid body of the main track to get bullet raycasts. Then the
 *  main track is removed, and the track (main track including all additional
 *  objects which were loaded later) is converted again.
 */
void TriangleMesh::removeBody()
{
    World::getWorld()->getPhysics()->removeBody(m_body);
    delete m_body;
    m_body = 0;
}   // removeBody

// -----------------------------------------------------------------------------
