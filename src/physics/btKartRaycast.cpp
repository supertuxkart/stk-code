/*
 * Copyright (C) 2005-2015 Erwin Coumans http://continuousphysics.com/Bullet/
 *
 * Permission to use, copy, modify, distribute and sell this software
 * and its documentation for any purpose is hereby granted without fee,
 * provided that the above copyright notice appear in all copies.
 * Erwin Coumans makes no representations about the suitability
 * of this software for any purpose.
 * It is provided "as is" without express or implied warranty.
*/

#include "LinearMath/btVector3.h"
#include "btKartRaycast.hpp"

#include "BulletCollision/CollisionDispatch/btCollisionWorld.h"
#include "BulletDynamics/Dynamics/btDynamicsWorld.h"

#include "modes/world.hpp"
#include "physics/triangle_mesh.hpp"
#include "tracks/track.hpp"

void* btKartRaycaster::castRay(const btVector3& from, const btVector3& to,
                               btVehicleRaycasterResult& result)
{
    // ========================================================================
    class ClosestWithNormal : public btCollisionWorld::ClosestRayResultCallback
    {
    private:
        int m_triangle_index;
    public:
        /** Constructor, initialises the triangle index. */
        ClosestWithNormal(const btVector3 &from,
                          const btVector3 &to)
                          : btCollisionWorld::ClosestRayResultCallback(from,to)
        {
            m_triangle_index = -1;
        }   // CloestWithNormal
        // --------------------------------------------------------------------
        /** Stores the index of the triangle hit. */
        virtual    btScalar addSingleResult(btCollisionWorld::LocalRayResult& rayResult,
                                         bool normalInWorldSpace)
        {
            // We don't always get a triangle index, sometimes (e.g. ray hits
            // other kart) we get shapePart=-1, or no localShapeInfo at all
            if(rayResult.m_localShapeInfo &&
                rayResult.m_localShapeInfo->m_shapePart>-1)
                m_triangle_index = rayResult.m_localShapeInfo->m_triangleIndex;
            return
                btCollisionWorld::ClosestRayResultCallback::addSingleResult(rayResult,
                normalInWorldSpace);
        }
        // --------------------------------------------------------------------
        /** Returns the index of the triangle which was hit, or -1 if
         *  no triangle was hit. */
        int getTriangleIndex() const { return m_triangle_index; }

    };   // CloestWithNormal
    // ========================================================================

    ClosestWithNormal rayCallback(from,to);

    m_dynamicsWorld->rayTest(from, to, rayCallback);

    if (rayCallback.hasHit())
    {
        btRigidBody* body = btRigidBody::upcast(rayCallback.m_collisionObject);
        if (body && body->hasContactResponse())
        {
            result.m_hitPointInWorld = rayCallback.m_hitPointWorld;
            result.m_hitNormalInWorld = rayCallback.m_hitNormalWorld;
            result.m_hitNormalInWorld.normalize();
            result.m_distFraction = rayCallback.m_closestHitFraction;
            result.m_triangle_index = -1;
            const TriangleMesh &tm =
                Track::getCurrentTrack()->getTriangleMesh();
            // FIXME: this code assumes atm that the object the kart is
            // driving on is the main track (and not e.g. a physical object).
            // If this should not be the case (i.e. the object hit by the
            // raycast is not the object of the main track mesh, don't smooth
            // the normals (since the index of the triangle is meant for a
            // different triangle mesh). TODO: Add a mapping from bullet
            // objects back to triangle meshes, so that it's easy to pick up
            // the right triangle mesh for smoothing
            TriangleMesh::RigidBodyTriangleMesh *rbtm =
                dynamic_cast<TriangleMesh::RigidBodyTriangleMesh*>(body);
            if(m_smooth_normals &&
                rayCallback.getTriangleIndex()>-1 &&
                rbtm != NULL                         )
            {
#undef DEBUG_NORMALS
#ifdef DEBUG_NORMALS
                btVector3 n=result.m_hitNormalInWorld;
#endif
                result.m_triangle_index = rayCallback.getTriangleIndex();
                result.m_hitNormalInWorld =
                    rbtm->m_triangle_mesh->getInterpolatedNormal(rayCallback.getTriangleIndex(),
                                             result.m_hitPointInWorld);
#ifdef DEBUG_NORMALS
                printf("old %f %f %f new %f %f %f\n",
                    n.getX(), n.getY(), n.getZ(),
                    result.m_hitNormalInWorld.getX(),
                    result.m_hitNormalInWorld.getY(),
                    result.m_hitNormalInWorld.getZ());
#endif
            }
            return body;
        }
    }
    return 0;
}

