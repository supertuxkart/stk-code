#include "ge_occlusion_culling.hpp"

#include <BulletCollision/NarrowPhaseCollision/btRaycastCallback.h>
#include <btBulletDynamicsCommon.h>
#include <cmath>
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

namespace GE
{
// ============================================================================
bool GEOcclusionCulling::SpherePointGenerator::getNextPoint(btVector3& point)
{
    if (m_current_point >= m_num_points)
        return false;

    if (m_current_point == 0)
    {
        point = m_center;
        m_current_point++;
        return true;
    }

    float golden_ratio = (1.0f + sqrtf(5.0f)) / 2.0f;
    float angle_increment = 2.0f * M_PI * golden_ratio;

    float t = float(m_current_point) / float(m_num_points);
    float inclination = acos(1.0f - 2.0f * t);
    float azimuth = angle_increment * float(m_current_point);

    float x = sin(inclination) * cos(azimuth);
    float y = sin(inclination) * sin(azimuth);
    float z = cos(inclination);

    // Test points at 80% of the sphere's surface
    float offset = 0.8f;
    point = m_center + btVector3(x, y, z) * m_radius * offset;
    m_current_point++;
    return true;
}   // GEOcclusionCulling::SpherePointGenerator::getNextPoint

// ----------------------------------------------------------------------------
GEOcclusionCulling::GEOcclusionCulling()
{
    m_triangle_mesh = NULL;
    m_occluder_shape = NULL;
    m_occluder_object = NULL;
}   // GEOcclusionCulling

// ----------------------------------------------------------------------------
GEOcclusionCulling::~GEOcclusionCulling()
{
    delete m_occluder_object;
    delete m_occluder_shape;
    delete m_triangle_mesh;
}   // ~GEOcclusionCulling

// ----------------------------------------------------------------------------
void GEOcclusionCulling::addOccluderMesh(
                            const std::vector<std::array<btVector3, 3> >& tris)
{
    assert(m_triangle_mesh == NULL);
    m_triangle_mesh = new btTriangleMesh();
    for (auto& t : tris)
        m_triangle_mesh->addTriangle(t[0], t[1], t[2]);
    m_occluder_shape = new btBvhTriangleMeshShape(m_triangle_mesh,
        false/*useQuantizedAabbCompression*/);
    m_occluder_object = new btCollisionObject();
    m_occluder_object->setCollisionShape(m_occluder_shape);
}   // addOccluderMesh

// ----------------------------------------------------------------------------
bool GEOcclusionCulling::isOccluded(const irr::core::vector3df& cam_pos,
                                    const irr::core::vector3df& irr_center,
                                    float radius)
{
    if (!m_triangle_mesh)
        return false;
    float radius_2 = radius * radius;
    float distance_2 = (cam_pos - irr_center).getLengthSQ();
    // Inside the sphere
    if (distance_2 <= radius_2)
        return false;

    // Calculate points using logarithmic scaling
    float distance = sqrtf(distance_2);
    // Formula: MAX_POINTS - log2(distance/MIN_DISTANCE) * SCALE_FACTOR
    // Clamp between MIN_POINTS and MAX_POINTS
    const float MIN_DISTANCE = 80.0f;   // Distance at which maximum points are used
    const int MAX_POINTS = 12;
    const int MIN_POINTS = 3;
    const float SCALE_FACTOR = 4.0f;    // Controls how quickly the point count decreases

    int num_points = MAX_POINTS -
        int(std::log2(std::max(distance / MIN_DISTANCE, 1.0f)) * SCALE_FACTOR);
    num_points = std::min(MAX_POINTS, std::max(MIN_POINTS, num_points));

    btVector3 center = btVector3(irr_center.X, irr_center.Y, irr_center.Z);
    PointGenerator* generator = new SpherePointGenerator(center, radius, num_points);

    btVector3 test_point;
    btVector3 cam_p(cam_pos.X, cam_pos.Y, cam_pos.Z);
    bool culled = true;
    while (generator->getNextPoint(test_point))
    {
        btCollisionWorld::ClosestRayResultCallback cb(cam_p, test_point);
        cb.m_flags |= btTriangleRaycastCallback::kF_FilterBackfaces;
        btTransform from_trans, to_trans;
        from_trans.setIdentity();
        from_trans.setOrigin(cam_p);
        to_trans.setIdentity();
        to_trans.setOrigin(test_point);
        btCollisionWorld::rayTestSingle(from_trans, to_trans,
            m_occluder_object, m_occluder_shape,
            m_occluder_object->getWorldTransform(), cb);
        if (!cb.hasHit() ||
            (cb.m_hitPointWorld - test_point).length2() <= radius_2 || // Hit point inside the sphere
            (cb.m_hitPointWorld - cam_p).length2() > (test_point - cam_p).length2()) // Hit point behind the sphere
        {
            culled = false;
            break;
        }
    }
    delete generator;
    return culled;
}   // isOccluded

}
