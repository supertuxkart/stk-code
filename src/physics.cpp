//  $Id: physics.cpp 839 2006-10-24 00:01:56Z hiker $
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

#include "physics.hpp"
#include "ssg_help.hpp"
#include "world.hpp"

#ifdef BULLET
#include "../bullet/Demos/OpenGL/GL_ShapeDrawer.h"

/** Initialise physics. */
Physics::Physics(float gravity)
{
    m_dynamics_world = new btDiscreteDynamicsWorld();
    m_dynamics_world->setGravity(btVector3(0.0f, 0.0f, -gravity));
    m_debug_drawer=new GLDebugDrawer();
    m_debug_drawer->setDebugMode(btIDebugDraw::DBG_DrawWireframe);
    m_dynamics_world->setDebugDrawer(m_debug_drawer);
}   // Physics

//-----------------------------------------------------------------------------
Physics::~Physics()
{
    delete m_dynamics_world;
}   // ~Physics

// -----------------------------------------------------------------------------
//* Convert the ssg track tree into its physics equivalents.
void Physics::setTrack(ssgEntity* track)
{
    //return;               // debug only FIXME
    if(!track) return;
    sgMat4 mat;
    sgMakeIdentMat4(mat);
    convertTrack(track, mat);
}   // setTrack

// -----------------------------------------------------------------------------
//* Convert the ssg track tree into its physics equivalents.
void Physics::convertTrack(ssgEntity *track, sgMat4 m)
{
    if(!track) return;
    if(track->isAKindOf(ssgTypeLeaf()))
    {
        ssgLeaf             *leaf       = (ssgLeaf*)(track);
        // printf("triangles %d\n",leaf->getNumTriangles());
        btTriangleMesh      *mesh       = new btTriangleMesh();
        for(int i=0; i<leaf->getNumTriangles(); i++) 
        {
            short v1,v2,v3;
            sgVec3 vv1, vv2, vv3;
            
            leaf->getTriangle(i, &v1, &v2, &v3);
            sgXformPnt3 ( vv1, leaf->getVertex(v1), m );
            sgXformPnt3 ( vv2, leaf->getVertex(v2), m );
            sgXformPnt3 ( vv3, leaf->getVertex(v3), m );
            btVector3 vb1(vv1[0],vv1[1],vv1[2]);
            btVector3 vb2(vv2[0],vv2[1],vv2[2]);
            btVector3 vb3(vv3[0],vv3[1],vv3[2]);
            mesh->addTriangle(vb1, vb2, vb3);
        }

        btCollisionShape *mesh_shape = new btBvhTriangleMeshShape(mesh);
        btTransform startTransform;
        startTransform.setIdentity();
        btDefaultMotionState *myMotionState = new btDefaultMotionState(startTransform);
        btRigidBody *body=new btRigidBody(0.0f, myMotionState, mesh_shape);
        m_dynamics_world->addRigidBody(body);
    }   // if(track isAKindOf leaf)
    else if(track->isAKindOf(ssgTypeTransform()))
    {
        ssgBaseTransform *t = (ssgBaseTransform*)(track);
        sgMat4 tmpT, tmpM;
        t->getTransform(tmpT);
        sgCopyMat4(tmpM, m);
        sgPreMultMat4(tmpM,tmpT);
        for(ssgEntity *e = t->getKid(0); e!=NULL; e=t->getNextKid())
        {
            convertTrack(e, tmpM);
        }   // for i
    }
    else if (track->isAKindOf(ssgTypeBranch())) 
    {
        ssgBranch *b =(ssgBranch*)track;
        for(ssgEntity* e=b->getKid(0); e!=NULL; e=b->getNextKid()) {
            convertTrack(e, m);
        }   // for i<getNumKids
    }
    else
    {
        assert(!"Unkown ssg type in convertTrack");
    }
}   // convertTrack

// -----------------------------------------------------------------------------
//* Adds a kart to the physics engine
void Physics::addKart(const Kart *kart, btRaycastVehicle *vehicle)
{
    m_dynamics_world->addRigidBody(kart->getKartBody());
    m_dynamics_world->addVehicle(vehicle);

}   // addKart

//-----------------------------------------------------------------------------
void Physics::update(float dt)
{
    m_dynamics_world->stepSimulation(dt);
    // ???  m_dynamicsWorld->updateAabbs();

}   // update

// -----------------------------------------------------------------------------
//* 
void Physics::draw()
{
    GL_ShapeDrawer::drawCoordSystem();
                               
    int num_objects = m_dynamics_world->getNumCollisionObjects();
    for(int i=0; i<num_objects; i++)
    {
        btCollisionObject *obj = m_dynamics_world->getCollisionObjectArray()[i];
        btRigidBody* body = btRigidBody::upcast(obj);
        if(!body) continue;
        //const btVector3 &pos=body->getCenterOfMassPosition();
        //printf("body %d: %f %f %f dt %f\n",i, pos.x(), pos.y(), pos.z(),dt);
        float m[16];
        btVector3 wireColor(1,0,0);
        btDefaultMotionState *myMotion = (btDefaultMotionState*)body->getMotionState();
        myMotion->m_graphicsWorldTrans.getOpenGLMatrix(m);
        debugDraw(m, obj->getCollisionShape(), wireColor);

    }
}   // draw

// -----------------------------------------------------------------------------
void Physics::debugDraw(float m[16], btCollisionShape *s, const btVector3 color)
    
{
    GL_ShapeDrawer::drawOpenGL(m, s, color, 0);
                               //                               btIDebugDraw::DBG_DrawWireframe);
    //                               btIDebugDraw::DBG_DrawAabb);

}   // debugDraw
// -----------------------------------------------------------------------------

#endif
/* EOF */

