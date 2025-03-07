#ifndef HEADER_GE_CLUSTER_DATA_GENERATOR_HPP
#define HEADER_GE_CLUSTER_DATA_GENERATOR_HPP

#include "aabbox3d.h"
#include "dimension2d.h"
#include "quaternion.h"
#include "matrix4.h"
#include "vector3d.h"

#include <atomic>
#include <future>
#include <vector>

#include <math.h>
#include <stdint.h>

namespace GE
{
class GEVulkanCameraSceneNode;

class GEClusterDataGenerator
{
private:

    unsigned m_set_size;

    irr::core::vector3df m_frustum_fld; // Far Left Down
    irr::core::vector3df m_frustum_fru; // Far Right Up
    irr::core::vector3df m_origin;

    float m_near;
    float m_far;

    irr::core::dimension2du m_screen;

    bool m_initialized;
    bool m_generated;

    irr::core::matrix4 m_view_matrix;

    std::vector<uint32_t> m_cluster_data_xz;
    std::vector<uint32_t> m_cluster_data_yz;

    std::vector<irr::core::vector3df> m_x_planes;
    std::vector<irr::core::vector3df> m_y_planes;

    std::vector<irr::core::vector3df> m_object_pos;
    std::vector<float>                m_object_rad;
    std::vector<unsigned>             m_object_ids;

    std::vector<std::future<void> > m_tasks;

public:
    // ------------------------------------------------------------------------
    static const unsigned getTileSize() { return 16u; }
    // ------------------------------------------------------------------------
    static const unsigned getDepthDivisions() { return 64u; }
    // ------------------------------------------------------------------------
    static const unsigned getChunkSize() { return 32u; }
    // ------------------------------------------------------------------------
    static const float depthKernel(float d) { return sqrtf(d); }
    // ------------------------------------------------------------------------
    GEClusterDataGenerator();
    // ------------------------------------------------------------------------
    ~GEClusterDataGenerator();
    // ------------------------------------------------------------------------
    void init(GEVulkanCameraSceneNode *cam);
    // ------------------------------------------------------------------------
    void addObject(irr::core::vector3df view_pos, float radius, unsigned id);
    // ------------------------------------------------------------------------
    void generate();
    // ------------------------------------------------------------------------
    const std::vector<uint32_t>& getClusterDataXZ();
    // ------------------------------------------------------------------------
    const std::vector<uint32_t>& getClusterDataYZ();
    // ------------------------------------------------------------------------
    unsigned getClusterSetSize() const { return m_set_size; }
    // ------------------------------------------------------------------------
    float getClusterFarDistance() const { return m_far; }
    // ------------------------------------------------------------------------
    void clear();
};   // GECullingTool

}

#endif
