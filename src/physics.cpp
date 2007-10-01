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


#ifdef BULLET

#include "physics.hpp"
#include "ssg_help.hpp"
#include "world.hpp"

#include "../bullet/Demos/OpenGL/GL_ShapeDrawer.h"
#include "moving_physics.hpp"
#include "user_config.hpp"

/** Initialise physics. */
Physics::Physics(float gravity)
{

    // The bullet interface has changed recently, define NEWBULLET
    // for the new interface, default is the old interface. This version
    // works with SVN revision 813 of bullet.

#ifdef NEWBULLET
    btDefaultCollisionConfiguration* collisionConf = new btDefaultCollisionConfiguration();
    btCollisionDispatcher *dispatcher = new btCollisionDispatcher(collisionConf);
#else
    btCollisionDispatcher *dispatcher = new btCollisionDispatcher();
#endif
    btVector3 worldMin(-1000, -1000, -1000);
    btVector3 worldMax( 1000,  1000,  1000);

#ifdef NEWBULLET
    btBroadphaseInterface *axis_sweep    = new btAxisSweep3(worldMin, worldMax);
    btConstraintSolver *constraintSolver = new btSequentialImpulseConstraintSolver();
    m_dynamics_world = new btDiscreteDynamicsWorld(dispatcher, axis_sweep, 
                                                   constraintSolver);
#else
    btOverlappingPairCache *pairCache    = new btAxisSweep3(worldMin, worldMax);
    btConstraintSolver *constraintSolver = new btSequentialImpulseConstraintSolver();
    m_dynamics_world = new btDiscreteDynamicsWorld(dispatcher, pairCache, 
                                                   constraintSolver);
#endif
    m_dynamics_world->setGravity(btVector3(0.0f, 0.0f, -gravity));
    if(user_config->m_bullet_debug)
    {
        m_debug_drawer=new GLDebugDrawer();
        m_debug_drawer->setDebugMode(btIDebugDraw::DBG_DrawWireframe);
        m_dynamics_world->setDebugDrawer(m_debug_drawer);
    }
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
    MovingPhysics *mp = dynamic_cast<MovingPhysics*>(track);
    if(mp)
    {
        // If the track contains obect of type MovingPhysics,
        // these objects will be real rigid body and are already
        // part of the world. So these objects must not be converted
        // to triangle meshes.
    } 
    else if(track->isAKindOf(ssgTypeLeaf()))
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

        btCollisionShape *mesh_shape = new btBvhTriangleMeshShape(mesh, true);
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
    m_dynamics_world->addRigidBody(kart->getBody());
    m_dynamics_world->addVehicle(vehicle);

}   // addKart

//-----------------------------------------------------------------------------
/** Removes a kart from the physics engine.
 *  Removes a kart from the physics engine. This is used when rescuing a kart
 */
void Physics::removeKart(const Kart *kart, btRaycastVehicle *vehicle)
{
    m_dynamics_world->removeRigidBody(kart->getBody());
    m_dynamics_world->removeVehicle(vehicle);
}   // removeKart

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
    if(user_config->m_bullet_debug)
    {
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
            if(myMotion) 
            {
                myMotion->m_graphicsWorldTrans.getOpenGLMatrix(m);
                debugDraw(m, obj->getCollisionShape(), wireColor);
            }
        }  // for i
    }   // if m_bullet_debug
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

