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

#include "physics/physics.hpp"

#include "animations/three_d_animation.hpp"
#include "config/user_config.hpp"
#include "network/race_state.hpp"
#include "graphics/stars.hpp"
#include "physics/btKart.hpp"
#include "physics/btUprightConstraint.hpp"
#include "physics/irr_debug_drawer.hpp"
#include "physics/physical_object.hpp"
#include "physics/stk_dynamics_world.hpp"
#include "physics/triangle_mesh.hpp"
#include "tracks/track.hpp"

// ----------------------------------------------------------------------------
/** Initialise physics.
 *  Create the bullet dynamics world.
 */
Physics::Physics() : btSequentialImpulseConstraintSolver()
{
    m_collision_conf = new btDefaultCollisionConfiguration();
    m_dispatcher     = new btCollisionDispatcher(m_collision_conf);
}   // Physics

//-----------------------------------------------------------------------------
/** The actual initialisation of the physics, which is called after the track
 *  model is loaded. This allows the physics to use the actual track dimension
 *  for the axis sweep.
 */
void Physics::init(const Vec3 &world_min, const Vec3 &world_max)
{
    m_axis_sweep     = new btAxisSweep3(world_min, world_max);
    m_dynamics_world = new STKDynamicsWorld(m_dispatcher,
                                            m_axis_sweep,
                                            this,
                                            m_collision_conf);
    m_dynamics_world->setGravity(
        btVector3(0.0f,
                  -World::getWorld()->getTrack()->getGravity(),
                  0.0f));
    m_debug_drawer = new IrrDebugDrawer();
    m_dynamics_world->setDebugDrawer(m_debug_drawer);
}   // init

//-----------------------------------------------------------------------------
Physics::~Physics()
{
    delete m_debug_drawer;
    delete m_dynamics_world;
    delete m_axis_sweep;
    delete m_dispatcher;
    delete m_collision_conf;
}   // ~Physics

// ----------------------------------------------------------------------------
/** Adds a kart to the physics engine.
 *  This adds the rigid body, the vehicle, and the upright constraint.
 *  \param kart The kart to add.
 *  \param vehicle The raycast vehicle object.
 */
void Physics::addKart(const Kart *kart)
{
    m_dynamics_world->addRigidBody(kart->getBody());
    m_dynamics_world->addVehicle(kart->getVehicle());
    m_dynamics_world->addConstraint(kart->getUprightConstraint());
}   // addKart

//-----------------------------------------------------------------------------
/** Removes a kart from the physics engine. This is used when rescuing a kart
 *  (and during cleanup).
 *  \param kart The kart to remove.
 */
void Physics::removeKart(const Kart *kart)
{
    m_dynamics_world->removeRigidBody(kart->getBody());
    m_dynamics_world->removeVehicle(kart->getVehicle());
    m_dynamics_world->removeConstraint(kart->getUprightConstraint());
}   // removeKart

//-----------------------------------------------------------------------------
/** Updates the physics simulation and handles all collisions.
 *  \param dt Time step.
 */
void Physics::update(float dt)
{
    // Bullet can report the same collision more than once (up to 4
    // contact points per collision). Additionally, more than one internal
    // substep might be taken, resulting in potentially even more
    // duplicates. To handle this, all collisions (i.e. pair of objects)
    // are stored in a vector, but only one entry per collision pair
    // of objects.
    m_all_collisions.clear();

    // Maximum of three substeps. This will work for framerate down to
    // 20 FPS (bullet default frequency is 60 HZ).
    m_dynamics_world->stepSimulation(dt, 3);

    // Now handle the actual collision. Note: flyables can not be removed
    // inside of this loop, since the same flyables might hit more than one
    // other object. So only a flag is set in the flyables, the actual
    // clean up is then done later in the projectile manager.
    std::vector<CollisionPair>::iterator p;
    for(p=m_all_collisions.begin(); p!=m_all_collisions.end(); ++p)
    {
        // Kart-kart collision
        // --------------------
        if(p->getUserPointer(0)->is(UserPointer::UP_KART))
        {
            Kart *a=p->getUserPointer(0)->getPointerKart();
            Kart *b=p->getUserPointer(1)->getPointerKart();
            race_state->addCollision(a->getWorldKartId(),
                                     b->getWorldKartId());
            KartKartCollision(p->getUserPointer(0)->getPointerKart(), 
                              p->getContactPointCS(0),
                              p->getUserPointer(1)->getPointerKart(), 
                              p->getContactPointCS(1)                );
            continue;
        }  // if kart-kart collision

        if(p->getUserPointer(0)->is(UserPointer::UP_PHYSICAL_OBJECT))
        {
            // Kart hits physical object
            // -------------------------
            PhysicalObject *obj = p->getUserPointer(0)
                                   ->getPointerPhysicalObject();
            if(obj->isCrashReset())
            {
                Kart *kart = p->getUserPointer(1)->getPointerKart();
                kart->forceRescue();
            }
            continue;
        }

        if(p->getUserPointer(0)->is(UserPointer::UP_ANIMATION))
        {
            // Kart hits animation
            ThreeDAnimation *anim=p->getUserPointer(0)->getPointerAnimation();
            if(anim->isCrashReset())
            {
                Kart *kart = p->getUserPointer(1)->getPointerKart();
                kart->forceRescue();
            }
            continue;

        }
        // now the first object must be a projectile
        // =========================================
        if(p->getUserPointer(1)->is(UserPointer::UP_TRACK))
        {
            // Projectile hits track
            // ---------------------
            p->getUserPointer(0)->getPointerFlyable()->hitTrack();
        }
        else if(p->getUserPointer(1)->is(UserPointer::UP_PHYSICAL_OBJECT))
        {
            // Projectile hits physical object
            // -------------------------------
            p->getUserPointer(0)->getPointerFlyable()
                ->hit(NULL, p->getUserPointer(1)->getPointerPhysicalObject());

        }
        else if(p->getUserPointer(1)->is(UserPointer::UP_KART))
        {
            // Projectile hits kart
            // --------------------
            // Only explode a bowling ball if the target is
            // not invulnerable
            if(p->getUserPointer(0)->getPointerFlyable()->getType()
                !=PowerupManager::POWERUP_BOWLING                         ||
                !p->getUserPointer(1)->getPointerKart()->isInvulnerable()   )
                    p->getUserPointer(0)->getPointerFlyable()
                     ->hit(p->getUserPointer(1)->getPointerKart());
        }
        else
        {
            // Projectile hits projectile
            // --------------------------
            p->getUserPointer(0)->getPointerFlyable()->hit(NULL);
            p->getUserPointer(1)->getPointerFlyable()->hit(NULL);
        }
    }  // for all p in m_all_collisions
}   // update

//-----------------------------------------------------------------------------
/** Project all karts downwards onto the surface below.
 *  Used in setting the starting positions of all the karts.
 */

bool Physics::projectKartDownwards(const Kart *k)
{
    btVector3 hell(0, -10000, 0);
    return k->getVehicle()->projectVehicleToSurface(hell, 
                                                    /*allow translation*/true);
} //projectKartsDownwards

//-----------------------------------------------------------------------------
/** Determines the side (left, front, ...) of a rigid body with a box 
 *  collision shape that has a given contact point.
 *  \param body The rigid body (box shape).
 *  \param contact_point The contact point (in local coordinates) of the 
 *         contact point.
 */
Physics::CollisionSide Physics::getCollisionSide(const btRigidBody *body,
                                                 const Vec3 &contact_point)
{
    btVector3 aabb_min, aabb_max;
    static btTransform zero_trans(btQuaternion(0, 0, 0));
    body->getCollisionShape()->getAabb(zero_trans, aabb_min, aabb_max);
    btVector3 extend = 0.5f*(aabb_max - aabb_min);

    CollisionSide result = COL_LEFT;
    if(contact_point.getX()>0) // --> right side
    {
        if(contact_point.getZ()>0) // --> front or right side
        {
            result = fabsf(extend.getX() - contact_point.getX()) <
                     fabsf(extend.getZ() - contact_point.getZ()) ? COL_RIGHT 
                                                                 : COL_FRONT; 
        }
        else   // getZ()<0  --> back or right side
        {
            result = fabsf( extend.getX() - contact_point.getX()) <
                     fabsf( extend.getZ() + contact_point.getZ()) ? COL_RIGHT 
                                                                  : COL_BACK; 
        }
    }
    else   // getX() < 0 --> left side
    {
        if(contact_point.getZ()>0) // --> front or left side
        {
            result = fabsf(extend.getX() + contact_point.getX()) <
                     fabsf(extend.getZ() - contact_point.getZ()) ? COL_LEFT 
                                                                 : COL_FRONT; 
        }
        else   // --> back or left side
        {
            result = fabsf(extend.getX() + contact_point.getX()) <
                     fabsf(extend.getZ() + contact_point.getZ()) ? COL_LEFT 
                                                                 : COL_BACK; 
        }
    }

    return result;
}   // getCollisionSide

//-----------------------------------------------------------------------------
/** Handles the special case of two karts colliding with each other, which 
 *  means that bombs must be passed on. If both karts have a bomb, they'll 
 *  explode immediately. This function is called from physics::update() on the 
 *  server and if no networking is used, and from race_state on the client to 
 *  replay what happened on the server.
 *  \param kart_a First kart involved in the collision.
 *  \param kart_b Second kart involved in the collision.
 */
void Physics::KartKartCollision(Kart *kart_a, const Vec3 &contact_point_a,
                                Kart *kart_b, const Vec3 &contact_point_b)
{
    kart_a->crashed(kart_b);   // will play crash sound for player karts
    kart_b->crashed(kart_a);
    Attachment *attachmentA=kart_a->getAttachment();
    Attachment *attachmentB=kart_b->getAttachment();

    if(attachmentA->getType()==Attachment::ATTACH_BOMB)
    {
        // If both karts have a bomb, explode them immediately:
        if(attachmentB->getType()==Attachment::ATTACH_BOMB)
        {
            attachmentA->setTimeLeft(0.0f);
            attachmentB->setTimeLeft(0.0f);
        }
        else  // only A has a bomb, move it to B (unless it was from B)
        {
            if(attachmentA->getPreviousOwner()!=kart_b)
            {
                attachmentA->moveBombFromTo(kart_a, kart_b);
                // Play appropriate SFX
                kart_b->playCustomSFX(SFXManager::CUSTOM_ATTACH);
            }
        }
    }
    else if(attachmentB->getType()==Attachment::ATTACH_BOMB &&
            attachmentB->getPreviousOwner()!=kart_a)
    {
        attachmentB->moveBombFromTo(kart_b, kart_a);
        kart_a->playCustomSFX(SFXManager::CUSTOM_ATTACH);
    }
    else
    {
        kart_a->playCustomSFX(SFXManager::CUSTOM_CRASH);
        kart_b->playCustomSFX(SFXManager::CUSTOM_CRASH);
    }

    // If bouncing crashes is enabled, add an additional force to the
    // slower kart
    Kart *faster_kart, *slower_kart;
    Vec3 faster_cp, slower_cp;
    if(kart_a->getSpeed()>=kart_b->getSpeed())
    {
        faster_kart = kart_a;
        faster_cp   = contact_point_a;
        slower_kart = kart_b;
        slower_cp   = contact_point_b;
    }
    else
    {
        faster_kart = kart_b;
        faster_cp   = contact_point_b;
        slower_kart = kart_a;
        slower_cp   = contact_point_a;
    }

    CollisionSide faster_side = getCollisionSide(faster_kart->getBody(), 
                                                 faster_cp);
    CollisionSide slower_side = getCollisionSide(slower_kart->getBody(), 
                                                 slower_cp);

    // This probably needs adjusting once we have different kart properties.
    // E.g. besides speed we might also want to take mass into account(?)
    if(faster_side==COL_FRONT)
    {
        // Special case: the faster kart hits a kart front on. In this case
        // the slower kart will be pushed out of the faster kart's way
        Vec3 dir = faster_kart->getVelocity();

        // The direction in which the impulse will be applied depends on 
        // which side of the faster kart was hitting it: if the hit is
        // on the right side of the faster kart, it will push the slower
        // kart to the right and vice versa. This is based on the 
        // assumption that a hit to the right indicates that it's
        // shorter to push the slower kart to the right.
        Vec3 impulse;
        if(faster_cp.getX()>0)
            impulse = Vec3( dir.getZ(), 0, -dir.getX());
        else
            impulse = Vec3(-dir.getZ(), 0,  dir.getX());
        impulse.normalize();
        impulse *= faster_kart->getKartProperties()->getCollisionImpulse();
        float t = 
            faster_kart->getKartProperties()->getCollisionImpulseTime();
        if(t>0)
            slower_kart->getVehicle()->setTimedCentralImpulse(t, impulse);
        else
            slower_kart->getBody()->applyCentralImpulse(impulse);
        slower_kart->getBody()->setAngularVelocity(btVector3(0,0,0));
        // Apply some impulse to the slower kart as well?
    }
    else
    {
        // Non-frontal collision, push the two karts away from each other
        // First the faster kart
        Vec3 dir = faster_kart->getVelocity();
        Vec3 impulse;
        if(faster_cp.getX()>0)
            impulse = Vec3(-dir.getZ(), 0,  dir.getX());
        else
            impulse = Vec3( dir.getZ(), 0, -dir.getX());
        impulse.normalize();
        impulse *= slower_kart->getKartProperties()->getCollisionImpulse();
        float t = 
            faster_kart->getKartProperties()->getCollisionImpulseTime();
        if(t>0)
            faster_kart->getVehicle()->setTimedCentralImpulse(t, impulse);
        else
            faster_kart->getBody()->applyCentralImpulse(impulse);
        faster_kart->getBody()->setAngularVelocity(btVector3(0,0,0));

        // Then the slower kart
        dir = slower_kart->getVelocity();
        if(slower_cp.getX()>0)
            impulse = Vec3(-dir.getZ(), 0,  dir.getX());
        else
            impulse = Vec3( dir.getZ(), 0, -dir.getX());

        impulse.normalize();
        impulse *= faster_kart->getKartProperties()->getCollisionImpulse();
        t = faster_kart->getKartProperties()->getCollisionImpulseTime();
        if(t>0)
            slower_kart->getVehicle()->setTimedCentralImpulse(t, impulse);
        else
            slower_kart->getBody()->applyCentralImpulse(impulse);
        slower_kart->getBody()->setAngularVelocity(btVector3(0,0,0));
    }

}   // KartKartCollision

//-----------------------------------------------------------------------------
/** This function is called at each internal bullet timestep. It is used
 *  here to do the collision handling: using the contact manifolds after a
 *  physics time step might miss some collisions (when more than one internal
 *  time step was done, and the collision is added and removed). So this
 *  function stores all collisions in a list, which is then handled after the
 *  actual physics timestep. This list only stores a collision if it's not
 *  already in the list, so a collisions which is reported more than once is
 *  nevertheless only handled once.
 *  The list of collision 
 *  Parameters: see bullet documentation for details.
 */
btScalar Physics::solveGroup(btCollisionObject** bodies, int numBodies,
                             btPersistentManifold** manifold,int numManifolds,
                             btTypedConstraint** constraints,
                             int numConstraints,
                             const btContactSolverInfo& info,
                             btIDebugDraw* debugDrawer, 
                             btStackAlloc* stackAlloc, 
                             btDispatcher* dispatcher)
{
    btScalar returnValue=
        btSequentialImpulseConstraintSolver::solveGroup(bodies, numBodies, 
                                                        manifold, numManifolds,
                                                        constraints,
                                                        numConstraints, info,
                                                        debugDrawer, 
                                                        stackAlloc,
                                                        dispatcher);
    int currentNumManifolds = m_dispatcher->getNumManifolds();
    // We can't explode a rocket in a loop, since a rocket might collide with
    // more than one object, and/or more than once with each object (if there
    // is more than one collision point). So keep a list of rockets that will
    // be exploded after the collisions
    std::vector<Moveable*> rocketsToExplode;
    for(int i=0; i<currentNumManifolds; i++)
    {
        btPersistentManifold* contact_manifold = 
            m_dynamics_world->getDispatcher()->getManifoldByIndexInternal(i);

        btCollisionObject* objA = 
            static_cast<btCollisionObject*>(contact_manifold->getBody0());
        btCollisionObject* objB = 
            static_cast<btCollisionObject*>(contact_manifold->getBody1());

        unsigned int num_contacts = contact_manifold->getNumContacts();
        if(!num_contacts) continue;   // no real collision

        UserPointer *upA        = (UserPointer*)(objA->getUserPointer());
        UserPointer *upB        = (UserPointer*)(objB->getUserPointer());

        if(!upA || !upB) continue;

        // 1) object A is a track
        // =======================
        if(upA->is(UserPointer::UP_TRACK))
        {
            if(upB->is(UserPointer::UP_FLYABLE))   // 1.1 projectile hits track
                m_all_collisions.push_back(
                    upB, contact_manifold->getContactPoint(0).m_localPointB,
                    upA, contact_manifold->getContactPoint(0).m_localPointA);
            else if(upB->is(UserPointer::UP_KART))
            {
                Kart *kart=upB->getPointerKart();
                race_state->addCollision(kart->getWorldKartId());
                int n = contact_manifold->getContactPoint(0).m_index0;
                const Material *m 
                    = n>=0 ? upA->getPointerTriangleMesh()->getMaterial(n)
                           : NULL;
                kart->crashed(NULL, m);
            }
        }
        // 2) object a is a kart
        // =====================
        else if(upA->is(UserPointer::UP_KART))
        {
            if(upB->is(UserPointer::UP_TRACK))
            {
                Kart *kart = upA->getPointerKart();
                race_state->addCollision(kart->getWorldKartId());
                int n = contact_manifold->getContactPoint(0).m_index1;
                const Material *m 
                    = n>=0 ? upB->getPointerTriangleMesh()->getMaterial(n)
                           : NULL;
                kart->crashed(NULL, m);   // Kart hit track
            }
            else if(upB->is(UserPointer::UP_FLYABLE))
                // 2.1 projectile hits kart
                m_all_collisions.push_back(
                    upB, contact_manifold->getContactPoint(0).m_localPointB,
                    upA, contact_manifold->getContactPoint(0).m_localPointA);
            else if(upB->is(UserPointer::UP_KART))
                // 2.2 kart hits kart
                m_all_collisions.push_back(
                    upA, contact_manifold->getContactPoint(0).m_localPointA,
                    upB, contact_manifold->getContactPoint(0).m_localPointB);
            else if(upB->is(UserPointer::UP_PHYSICAL_OBJECT))
                // 2.3 kart hits physical object
                m_all_collisions.push_back(
                    upB, contact_manifold->getContactPoint(0).m_localPointB,
                    upA, contact_manifold->getContactPoint(0).m_localPointA);
            else if(upB->is(UserPointer::UP_ANIMATION))
                m_all_collisions.push_back(
                    upB, contact_manifold->getContactPoint(0).m_localPointB,
                    upA, contact_manifold->getContactPoint(0).m_localPointA);
        }
        // 3) object is a projectile
        // =========================
        else if(upA->is(UserPointer::UP_FLYABLE))
        {
            // 3.1) projectile hits track
            // 3.2) projectile hits projectile
            // 3.3) projectile hits physical object
            // 3.4) projectile hits kart
            if(upB->is(UserPointer::UP_TRACK          ) ||
               upB->is(UserPointer::UP_FLYABLE        ) ||
               upB->is(UserPointer::UP_PHYSICAL_OBJECT) ||
               upB->is(UserPointer::UP_KART           )   )
            {
                m_all_collisions.push_back(
                    upA, contact_manifold->getContactPoint(0).m_localPointA,
                    upB, contact_manifold->getContactPoint(0).m_localPointB);
            }
        }
        // Object is a physical object
        // ===========================
        else if(upA->is(UserPointer::UP_PHYSICAL_OBJECT))
        {
            if(upB->is(UserPointer::UP_FLYABLE))
                m_all_collisions.push_back(
                    upB, contact_manifold->getContactPoint(0).m_localPointB,
                    upA, contact_manifold->getContactPoint(0).m_localPointA);
            else if(upB->is(UserPointer::UP_KART))
                m_all_collisions.push_back(
                    upA, contact_manifold->getContactPoint(0).m_localPointA,
                    upB, contact_manifold->getContactPoint(0).m_localPointB);
        }
        else if (upA->is(UserPointer::UP_ANIMATION))
        {
            if(upB->is(UserPointer::UP_KART))
                m_all_collisions.push_back(
                    upA, contact_manifold->getContactPoint(0).m_localPointA,
                    upB, contact_manifold->getContactPoint(0).m_localPointB);
        }
        else 
            assert("Unknown user pointer");           // 4) Should never happen
    }   // for i<numManifolds

    return returnValue;
}   // solveGroup

// ----------------------------------------------------------------------------
/** A debug draw function to show the track and all karts.
 */
void Physics::draw()
{
    if(!m_debug_drawer->debugEnabled() ||
        !World::getWorld()->isRacePhase()) return;

    video::SColor color(77,179,0,0);
    video::SMaterial material;
    material.Thickness = 2;
    material.AmbientColor = color;
    material.DiffuseColor = color;
    material.EmissiveColor= color;
    material.BackfaceCulling = false;
    material.setFlag(video::EMF_LIGHTING, false);
    irr_driver->getVideoDriver()->setMaterial(material);
    irr_driver->getVideoDriver()->setTransform(video::ETS_WORLD, 
                                               core::IdentityMatrix);
    m_dynamics_world->debugDrawWorld();
    return;
}   // draw

// ----------------------------------------------------------------------------

/* EOF */

