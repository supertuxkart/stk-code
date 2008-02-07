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
// ----------------------------------------------------------------------------

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
        if(p->a->is(UserPointer::UP_KART)) {          // kart-kart collision
            KartKartCollision(p->a->getPointerKart(), p->b->getPointerKart());
        }  // if kart-kart collision
        else  // now the first object must be a projectile
        {
            if(p->b->is(UserPointer::UP_TRACK))       // must be projectile hit track
            {
                p->a->getPointerFlyable()->hitTrack();
            }
            else if(p->b->is(UserPointer::UP_MOVING_PHYSICS))
            {
                p->a->getPointerFlyable()->explode(NULL, p->b->getPointerMovingPhysics());

            }
            else if(p->b->is(UserPointer::UP_KART))   // projectile hit kart
            {
                p->a->getPointerFlyable()->explode((Kart*)(p->b));
            }
            else                                     // projectile hits projectile
            {
                p->a->getPointerFlyable()->explode(NULL);
                p->b->getPointerFlyable()->explode(NULL);
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
    kartA->crashed();   // will play crash sound for player karts
    kartB->crashed(); 
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

        UserPointer *upA        = (UserPointer*)(objA->getUserPointer());
        UserPointer *upB        = (UserPointer*)(objB->getUserPointer());

	// FIXME: Must be a moving physics object
	// FIXME: A rocket should explode here!

	if(!upA || !upB) continue;
        // 1) object A is a track
        // =======================
        if(upA->is(UserPointer::UP_TRACK)) 
        { 
            if(upB->is(UserPointer::UP_FLYABLE))   // 1.1 projectile hits track
                m_all_collisions.push_back(upB, upA);
            else if(upB->is(UserPointer::UP_KART))
                // FIXME: sound disabled for now, since the chassis of the karts hits
                //        the track when accelerating, causing a constant crash sfx
                //        to be played. Might be fixed with better physics parameters
                //upB->getPointerKart()->crashed();
                0;  // avoid VS compiler warning while the above statement is commented out
        }
        // 2) object a is a kart
        // =====================
        else if(upA->is(UserPointer::UP_KART))
        {
            if(upB->is(UserPointer::UP_TRACK))
                // FIXME: sound disabled for now, since the chassis of the karts hits
                //        the track when accelerating, causing a constant crash sfx
                //        to be played. Might be fixed with better physics parameters
                // upA->getPointerKart()->crashed(); // Kart hit track
                ;
            else if(upB->is(UserPointer::UP_FLYABLE))
                m_all_collisions.push_back(upB, upA);   // 2.1 projectile hits kart
            else if(upB->is(UserPointer::UP_KART))
                m_all_collisions.push_back(upA, upB);   // 2.2 kart hits kart
        }
        // 3) object is a projectile
        // ========================
        else if(upA->is(UserPointer::UP_FLYABLE))
        {
            if(upB->is(UserPointer::UP_TRACK         ) ||   // 3.1) projectile hits track
               upB->is(UserPointer::UP_FLYABLE       ) ||   // 3.2) projectile hits projectile
               upB->is(UserPointer::UP_MOVING_PHYSICS) ||   // 3.3) projectile hits projectile
               upB->is(UserPointer::UP_KART          )   )  // 3.4) projectile hits kart
            {
                m_all_collisions.push_back(upA, upB);
            }
        } 
        else if(upA->is(UserPointer::UP_MOVING_PHYSICS))
        {
            if(upB->is(UserPointer::UP_FLYABLE)) 
            {
                m_all_collisions.push_back(upB, upA);
            }
        }
        else assert("Unknown user pointer");            // 4) Should never happen
    }   // for i<numManifolds

    return returnValue;
}   // solveGroup

// -----------------------------------------------------------------------------
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

/* EOF */

