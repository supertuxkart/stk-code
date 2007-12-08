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
//  but WITHOUT ANY WARRANTY; without even the implied warranty ofati
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.


#ifdef BULLET

#include "bullet/Demos/OpenGL/GL_ShapeDrawer.h"

#include "physics.hpp"
#include "ssg_help.hpp"
#include "world.hpp"
#include "flyable.hpp"

#include "moving_physics.hpp"
#include "user_config.hpp"
#include "sound_manager.hpp"
#include "material_manager.hpp"

/** Initialise physics. */

float const Physics::NOHIT=-99999.9f;

Physics::Physics(float gravity) : btSequentialImpulseConstraintSolver()
{
    m_collision_conf    = new btDefaultCollisionConfiguration();
    m_dispatcher        = new btCollisionDispatcher(m_collision_conf);
    
    btVector3 worldMin(-1000, -1000, -1000);
    btVector3 worldMax( 1000,  1000,  1000);
    m_axis_sweep        = new btAxisSweep3(worldMin, worldMax);
    m_dynamics_world    = new btDiscreteDynamicsWorld(m_dispatcher, 
                                                      m_axis_sweep, 
                                                      this,
                                                      m_collision_conf);
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
    if(user_config->m_bullet_debug) delete m_debug_drawer;
    delete m_dynamics_world;
    delete m_axis_sweep;
    delete m_dispatcher;
    delete m_collision_conf;
    
}   // ~Physics

// -----------------------------------------------------------------------------
//* Convert the ssg track tree into its physics equivalents.
void Physics::setTrack(ssgEntity* track)
{
    if(!track) return;
    sgMat4 mat;
    sgMakeIdentMat4(mat);
    btTriangleMesh *track_mesh = new btTriangleMesh();

    // Collect all triangles in the track_mesh
    convertTrack(track, mat, track_mesh);
    
    // Now convert the triangle mesh into a static rigid body
    btCollisionShape *mesh_shape = new btBvhTriangleMeshShape(track_mesh, true);
    btTransform startTransform;
    startTransform.setIdentity();
    btDefaultMotionState *myMotionState = new btDefaultMotionState(startTransform);
    btRigidBody *body=new btRigidBody(0.0f, myMotionState, mesh_shape);
    // FIXME: can the mesh_shape and or track_mesh be deleted now?
    m_dynamics_world->addRigidBody(body);
    body->setUserPointer(0);
}   // setTrack

// -----------------------------------------------------------------------------
//* Convert the ssg track tree into its physics equivalents.
void Physics::convertTrack(ssgEntity *track, sgMat4 m, btTriangleMesh* track_mesh)
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
        ssgLeaf  *leaf  = (ssgLeaf*)(track);
        Material *mat   = material_manager->getMaterial(leaf);
        // Don't convert triangles with material that is ignored (e.g. fuzzy_sand)
        if(!mat || mat->isIgnore()) return;

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
            track_mesh->addTriangle(vb1, vb2, vb3);
        }
        
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
            convertTrack(e, tmpM, track_mesh);
        }   // for i
    }
    else if (track->isAKindOf(ssgTypeBranch())) 
    {
        ssgBranch *b =(ssgBranch*)track;
        for(ssgEntity* e=b->getKid(0); e!=NULL; e=b->getNextKid()) {
            convertTrack(e, m, track_mesh);
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
/** Removes a kart from the physics engine. This is used when rescuing a kart
  * (and during cleanup).
 */
void Physics::removeKart(const Kart *kart)
{
    m_dynamics_world->removeRigidBody(kart->getBody());
    m_dynamics_world->removeVehicle(kart->getVehicle());
}   // removeKart

//-----------------------------------------------------------------------------
void Physics::update(float dt)
{
    // Bullet can report the same collision more than once (up to 4 
    // contact points per collision. Additionally, more than one internal
    // substep might be taken, resulting in potentially even more 
    // duplicates. To handle this, all collisions (i.e. pair of objects)
    // are stored in a vector, but only one entry per collision pair
    // of objects.
    m_all_collisions.clear();
    m_dynamics_world->stepSimulation(dt);

    // Now handle the actual collision. Note: rockets can not be removed
    // inside of this loop, since the same rocket might hit more than one
    // other object. So, only a flag is set in the rockets, the actual
    // clean up is then done later in the projectile manager.
    std::vector<CollisionPair>::iterator p;
    for(p=m_all_collisions.begin(); p!=m_all_collisions.end(); ++p)
    {
        if(p->type_a==Moveable::MOV_KART) {          // kart-kart collision
            Kart *kartA = (Kart*)(p->a);
            Kart *kartB = (Kart*)(p->b);
            KartKartCollision(kartA, kartB);
        }  // if kart-kart collision
        else  // now the first object must be a projectile
        {
            if(p->type_b==Moveable::MOV_TRACK)       // must be projectile hit track
            {
                ((Flyable*)(p->a))->hitTrack();
            }
            else if(p->type_b==Moveable::MOV_KART)   // projectile hit kart
            {
                Flyable *f=(Flyable*)(p->a);
                f->explode((Kart*)(p->b));
            }
            else                                     // projectile hits projectile
            {
                ((Flyable*)(p->a))->explode(NULL);
                ((Flyable*)(p->b))->explode(NULL);
            }
        }
    }  // for all p in m_all_collisions
}   // update

//-----------------------------------------------------------------------------
/** Handles the special case of two karts colliding with each other
 *  If both karts have a bomb, they'll explode immediately
 */
void Physics::KartKartCollision(Kart *kartA, Kart *kartB)
{
    Attachment *attachmentA=kartA->getAttachment();
    Attachment *attachmentB=kartB->getAttachment();

    if(attachmentA->getType()==ATTACH_BOMB)
    {
        // If both karts have a bomb, explode them immediately:
        if(attachmentB->getType()==ATTACH_BOMB)
        {
            attachmentA->setTimeLeft(0.0f);
            attachmentB->setTimeLeft(0.0f);
        } 
        else  // only A has a bomb, move it to B (unless it was from B)
        {
            if(attachmentA->getPreviousOwner()!=kartB) 
            {
                attachmentA->moveBombFromTo(kartA, kartB);
            }
        }
    }
    else if(attachmentB->getType()==ATTACH_BOMB &&
        attachmentB->getPreviousOwner()!=kartA) 
    {
        attachmentB->moveBombFromTo(kartB, kartA);
    }
    if(kartA->isPlayerKart()) sound_manager->playSfx(SOUND_CRASH);
    if(kartB->isPlayerKart()) sound_manager->playSfx(SOUND_CRASH);

}   // KartKartCollision

//-----------------------------------------------------------------------------
/** This function is called at each internal bullet timestep. It is used
  * here to do the collision handling: using the contact manifolds after a
  * physics time step might miss some collisions (when more than one internal
  * time step was done, and the collision is added and removed).
  **/
btScalar Physics::solveGroup(btCollisionObject** bodies, int numBodies,
                             btPersistentManifold** manifold,int numManifolds,
                             btTypedConstraint** constraints,int numConstraints,
                             const btContactSolverInfo& info, 
                             btIDebugDraw* debugDrawer, btStackAlloc* stackAlloc,
                             btDispatcher* dispatcher) {
    btScalar returnValue=
        btSequentialImpulseConstraintSolver::solveGroup(bodies, numBodies, manifold, 
                                                        numManifolds, constraints, 
                                                        numConstraints, info, 
                                                        debugDrawer, stackAlloc,
                                                        dispatcher);
    int currentNumManifolds = m_dispatcher->getNumManifolds();
    // We can't explode a rocket in a loop, since a rocket might collide with 
    // more than one object, and/or more than once with each object (if there 
    // is more than one collision point). So keep a list of rockets that will
    // be exploded after the collisions
    std::vector<Moveable*> rocketsToExplode;
    for(int i=0; i<currentNumManifolds; i++)
    {               
        btPersistentManifold* contactManifold = m_dynamics_world->getDispatcher()->getManifoldByIndexInternal(i);

        btCollisionObject* objA = static_cast<btCollisionObject*>(contactManifold->getBody0());
        btCollisionObject* objB = static_cast<btCollisionObject*>(contactManifold->getBody1());
        
        int numContacts = contactManifold->getNumContacts();
        if(!numContacts) continue;   // no real collision

        Moveable *movA          = static_cast<Moveable*>(objA->getUserPointer());
        Moveable *movB          = static_cast<Moveable*>(objB->getUserPointer());

        if(!numContacts) continue;   // no real collision

        // 1) object A is a track
        // =======================
        if(!movA) 
        { 
            if(movB && movB->getMoveableType()==Moveable::MOV_PROJECTILE)
            {   // 1.1 projectile hits track
                // -------------------------
                m_all_collisions.push_back(CollisionPair(movB, Moveable::MOV_PROJECTILE,
							 NULL, Moveable::MOV_TRACK  ));  
            }
        } 
        // 2) object a is a kart
        // =====================
        else if(movA->getMoveableType()==Moveable::MOV_KART)
        {
            if(movB && movB->getMoveableType()==Moveable::MOV_PROJECTILE)
            {   // 2.1 projectile hits kart
                // -------------------------
                m_all_collisions.push_back(CollisionPair(movB, Moveable::MOV_PROJECTILE,
                            							 movA, Moveable::MOV_KART      ));  
            }
            else if(movB && movB->getMoveableType()==Moveable::MOV_KART)
            {   // 2.2 kart hits kart
                // ------------------
                m_all_collisions.push_back(CollisionPair(movA, Moveable::MOV_KART,
							                             movB, Moveable::MOV_KART      ));  
                
            }
        }
        // 3) object is a projectile
        // ========================
        else if(movA->getMoveableType()==Moveable::MOV_PROJECTILE)
        {
            if(!movB)
            {   // 3.1) projectile hits track
                // --------------------------
                m_all_collisions.push_back(CollisionPair(movA, Moveable::MOV_PROJECTILE,
							 NULL, Moveable::MOV_TRACK     ));
            }
            else if(movB->getMoveableType()==Moveable::MOV_PROJECTILE)
            {   // 3.2 projectile hits projectile
                // ------------------------------
                m_all_collisions.push_back(CollisionPair(movA, Moveable::MOV_PROJECTILE,
							 movB, Moveable::MOV_PROJECTILE));  
            }
            else if(movB->getMoveableType()==Moveable::MOV_KART)
            {   // 3.3 projectile hits kart
                // ------------------------
                m_all_collisions.push_back(CollisionPair(movA, Moveable::MOV_PROJECTILE,
							 movB, Moveable::MOV_KART      ));  
            }
        }
        // 4) Nothing else should happen
        // =============================
        else
        {
            assert("Unknown user pointer");
        }
    }   // for i<numManifolds

    return returnValue;
}   // solveGroup

// -----------------------------------------------------------------------------
float Physics::getHAT(btVector3 pos)
{
    btVector3 to_pos(pos);
    to_pos.setZ(-100000.f);
    btCollisionWorld::ClosestRayResultCallback 
        rayCallback(pos, to_pos);

    m_dynamics_world->rayTest(pos, to_pos, rayCallback);
    if(!rayCallback.HasHit()) return NOHIT;

    return pos.getZ()-rayCallback.m_hitPointWorld.getZ();
}   // getHAT
// -----------------------------------------------------------------------------
bool Physics::getTerrainNormal(btVector3 pos, btVector3* normal)
{
    btVector3 to_pos(pos);
    to_pos.setZ(-100000.f);
    btCollisionWorld::ClosestRayResultCallback 
        rayCallback(pos, to_pos);

    m_dynamics_world->rayTest(pos, to_pos, rayCallback);
    if(!rayCallback.HasHit()) return false;
    *normal = rayCallback.m_hitNormalWorld;
    normal->normalize();
    return true;
}   // getTerrainNormal

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

