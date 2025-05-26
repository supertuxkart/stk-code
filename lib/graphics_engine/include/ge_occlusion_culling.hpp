#ifndef HEADER_GE_OCCLUSION_CULLING_HPP
#define HEADER_GE_OCCLUSION_CULLING_HPP

#include <aabbox3d.h>
#include <vector3d.h>

#include <LinearMath/btVector3.h>

#include <array>
#include <vector>

class btCollisionShape;
class btCollisionObject;
class btTriangleMesh;

namespace GE
{
class GEOcclusionCulling
{
private:
    btTriangleMesh* m_triangle_mesh;

    btCollisionShape* m_occluder_shape;

    btCollisionObject* m_occluder_object;

    // Point generators that yield one point at a time
    class PointGenerator
    {
    public:
        virtual ~PointGenerator() {}
        virtual bool getNextPoint(btVector3& point) = 0;
    };

    class SpherePointGenerator : public PointGenerator
    {
    private:
        btVector3 m_center;
        float m_radius;
        int m_num_points;
        int m_current_point;
    public:
        // --------------------------------------------------------------------
        SpherePointGenerator(const btVector3& center, float radius,
                             int num_points)
        {
            m_center = center;
            m_radius = radius;
            m_num_points = num_points;
            m_current_point = 0;
        }
        // --------------------------------------------------------------------
        bool getNextPoint(btVector3& point);
    };

public:
    // ------------------------------------------------------------------------
    GEOcclusionCulling();
    // ------------------------------------------------------------------------
    ~GEOcclusionCulling();
    // ------------------------------------------------------------------------
    void addOccluderMesh(const std::vector<std::array<btVector3, 3> >& tris);
    // ------------------------------------------------------------------------
    bool isOccluded(const irr::core::vector3df& cam_pos,
                    const irr::core::aabbox3df& aabbox)
    {
        // Use sphere test for now, testing the aabbox against the frustum
        // should be done first.
        float radius = aabbox.getExtent().getLength() / 2.0f;
        return isOccluded(cam_pos, aabbox.getCenter(), radius);
    }
    // ------------------------------------------------------------------------
    bool isOccluded(const irr::core::vector3df& cam_pos,
                    const irr::core::vector3df& irr_center, float radius);

};

}

#endif
