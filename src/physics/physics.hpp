//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2006-2015 Joerg Henrichs
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

#ifndef HEADER_PHYSICS_HPP
#define HEADER_PHYSICS_HPP

/**
  * \defgroup physics
  * Contains various physics utilities.
  */

#include <set>
#include <vector>

#include "btBulletDynamicsCommon.h"

#include "physics/irr_debug_drawer.hpp"
#include "physics/stk_dynamics_world.hpp"
#include "physics/user_pointer.hpp"

class AbstractKart;
class STKDynamicsWorld;
class Vec3;

/**
  * \ingroup physics
  */
class Physics : public btSequentialImpulseConstraintSolver
{
private:
    /** Bullet can report the same collision more than once (up to 4
     *  contact points per collision. Additionally, more than one internal
     *  substep might be taken, resulting in potentially even more
     *  duplicates. To handle this, all collisions (i.e. pair of objects)
     *  are stored in a vector, but only one entry per collision pair
     *  of objects.
     *  While this is a natural application of std::set, the set has some
     *  overhead (since it will likely use a tree to sort the entries).
     *  Considering that the number of collisions is usually rather small
     *  a simple list and linear search is faster is is being used here. */
    class CollisionPair
    {
    private:
        /** The user pointer of the objects involved in this collision. */
        const UserPointer *m_up[2];

        /** The contact point for each object (in local coordincates). */
        Vec3               m_contact_point[2];
    public:
        /** The entries in Collision Pairs are sorted: if a projectile
         * is included, it's always 'a'. If only two karts are reported
         * the first kart pointer is the smaller one. */
        CollisionPair(const UserPointer *a, const btVector3 &contact_point_a,
                      const UserPointer *b, const btVector3 &contact_point_b)
        {
            if(a->is(UserPointer::UP_KART) &&
               b->is(UserPointer::UP_KART) && a>b) {
                m_up[0]=b; m_contact_point[0] = contact_point_b;
                m_up[1]=a; m_contact_point[1] = contact_point_a;
            } else {
                m_up[0]=a; m_contact_point[0] = contact_point_a;
                m_up[1]=b; m_contact_point[1] = contact_point_b;
            }
        };  //    CollisionPair
        // --------------------------------------------------------------------
        /** Tests if two collision pairs involve the same objects. This test
         *  is simplified (i.e. no test if p.b==a and p.a==b) since the
         *  elements are sorted. */
        bool operator==(const CollisionPair &p) const
        {
            return (p.m_up[0]==m_up[0] && p.m_up[1]==m_up[1]);
        }   // operator==
        // --------------------------------------------------------------------
        const UserPointer *getUserPointer(unsigned int n) const
        {
            assert(n<=1);
            return m_up[n];
        }   // getUserPointer
        // --------------------------------------------------------------------
        /** Returns the contact point of the collision in
         *  car (local) coordinates. */
        const Vec3 &getContactPointCS(unsigned int n) const
        {
            assert(n>=0 && n<=1);
            return m_contact_point[n];
        }   // getContactPointCS
    };  // CollisionPair

    // ========================================================================
    // This class is the list of collision objects, where each collision
    // pair is stored as most once.
    class CollisionList : public std::vector<CollisionPair>
    {
    private:
        void push_back(CollisionPair p) {
            // only add a pair if it's not already in there
            for(iterator i=begin(); i!=end(); i++) {
                if((*i)==p) return;
            }
            std::vector<CollisionPair>::push_back(p);
        };  // push_back
    public:
        /** Adds information about a collision to this vector. */
        void push_back(const UserPointer *a, const btVector3 &contact_point_a,
                       const UserPointer *b, const btVector3 &contact_point_b)
        {
            push_back(CollisionPair(a, contact_point_a, b, contact_point_b));
        }
    };  // CollisionList
    // ========================================================================

    /** This flag is set while bullets time step processing is taking
    *  place. It is used to avoid altering data structures that might
    *  be used (e.g. removing a kart while a loop over all karts is
    *  taking place, as can happen in collision handling). */
    bool               m_physics_loop_active;

    /** If kart need to be removed from the physics world while physics
    *  processing is taking place, store the pointers to the karts to
    *  be removed here, and remove them once the physics processing
    *  is finished. */
    std::vector<const AbstractKart*> m_karts_to_delete;

    /** Pointer to the physics dynamics world. */
    STKDynamicsWorld                *m_dynamics_world;

    /** Used in physics debugging to draw the physics world. */
    IrrDebugDrawer                  *m_debug_drawer;

    btCollisionDispatcher           *m_dispatcher;
    btBroadphaseInterface           *m_axis_sweep;
    btDefaultCollisionConfiguration *m_collision_conf;
    CollisionList                    m_all_collisions;

             Physics();
    virtual ~Physics();

public:
    // ----------------------------------------------------------------------------------------
    static Physics* get();
    // ----------------------------------------------------------------------------------------
    static void create();
    // ----------------------------------------------------------------------------------------
    static void destroy();
    // ----------------------------------------------------------------------------------------
    void  init             (const Vec3 &min_world, const Vec3 &max_world);
    void  addKart          (const AbstractKart *k);
    void  addBody          (btRigidBody* b) {m_dynamics_world->addRigidBody(b);}
    void  removeKart       (const AbstractKart *k);
    void  removeBody       (btRigidBody* b) {m_dynamics_world->removeRigidBody(b);}
    void  KartKartCollision(AbstractKart *ka, const Vec3 &contact_point_a,
                            AbstractKart *kb, const Vec3 &contact_point_b);
    void  update           (int ticks);
    void  draw             ();
    STKDynamicsWorld*
          getPhysicsWorld  () const {return m_dynamics_world;}
    /** Activates the next debug mode (or switches it off again).
     */
    void  nextDebugMode    () {m_debug_drawer->nextDebugMode(); }
    void  setDebugMode(IrrDebugDrawer::DebugModeType mode) { m_debug_drawer->setDebugMode(mode); }
    /** Returns true if the debug drawer is enabled. */
    bool  isDebug() const     {return m_debug_drawer->debugEnabled(); }
    IrrDebugDrawer* getDebugDrawer() { return m_debug_drawer; }
    virtual btScalar solveGroup(btCollisionObject** bodies, int numBodies,
                                btPersistentManifold** manifold,int numManifolds,
                                btTypedConstraint** constraints,int numConstraints,
                                const btContactSolverInfo& info,
                                btIDebugDraw* debugDrawer, btStackAlloc* stackAlloc,
                                btDispatcher* dispatcher);
};

#endif // HEADER_PHYSICS_HPP
