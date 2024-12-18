#ifndef HEADER_GE_CULLING_TOOL_HPP
#define HEADER_GE_CULLING_TOOL_HPP

#include "aabbox3d.h"
#include "quaternion.h"
#include "matrix4.h"
#include "IVideoDriver.h"

#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace irr
{
    namespace scene { class ISceneNode; }
}

class CullingThreadpool;
class MaskedOcclusionCulling;

namespace GE
{
enum GEVulkanShadowCameraCascade : unsigned;

class GESPMBuffer;
class GEVulkanCameraSceneNode;
class GEVulkanShadowCameraSceneNode;
class GECullingTool

{
private:
    irr::core::quaternion m_frustum[6];
    bool m_ignore_near_plane; // For shadows

    irr::core::aabbox3df m_cam_bbox;
    irr::core::matrix4 m_pvm_matrix;
    irr::core::vector3df m_cam_position;

    MaskedOcclusionCulling *m_occlusion;
    CullingThreadpool *m_occlusion_manager;

    std::unordered_map<irr::scene::IMeshBuffer*, std::vector<float> > m_occluder_vertices;
    std::unordered_map<irr::scene::IMeshBuffer*, std::vector<unsigned> > m_occluder_indices;

    std::unordered_set<irr::scene::ISceneNode*> m_all_occluder_nodes;
public:
    // ------------------------------------------------------------------------
    GECullingTool();
    // ------------------------------------------------------------------------
    ~GECullingTool();
    // ------------------------------------------------------------------------
    void init(GEVulkanCameraSceneNode* cam, bool occlusion = false);
    // ------------------------------------------------------------------------
    void init(GEVulkanShadowCameraSceneNode* cam, GEVulkanShadowCameraCascade cascade);
    // ------------------------------------------------------------------------
    void addOccluder(GESPMBuffer* buffer, irr::scene::ISceneNode* node);
    // ------------------------------------------------------------------------
    void clearOccluders();
    // ------------------------------------------------------------------------
    void processOccluders();
    // ------------------------------------------------------------------------
    bool isViewCulled(irr::core::aabbox3df& bb);
    // ------------------------------------------------------------------------
    bool isOcclusionCulled(irr::core::aabbox3df& bb);
    // ------------------------------------------------------------------------
    bool isCulled(irr::core::aabbox3df& bb);
    // ------------------------------------------------------------------------
    bool isCulled(GESPMBuffer* buffer, irr::scene::ISceneNode* node);
};   // GECullingTool

}

#endif
