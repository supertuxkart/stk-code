#include "ge_culling_tool.hpp"

#include "ge_main.hpp"
#include "ge_spm_buffer.hpp"
#include "ge_vulkan_camera_scene_node.hpp"

#include "ISceneNode.h"

namespace GE
{
// ----------------------------------------------------------------------------
void GECullingTool::init(GEVulkanCameraSceneNode* cam)
{
    mathPlaneFrustumf(&m_frustum[0].X, cam->getPVM());
    m_cam_bbox = cam->getViewFrustum()->getBoundingBox();
}   // init

// ----------------------------------------------------------------------------
bool GECullingTool::isCulled(irr::core::aabbox3df& bb)
{
    if (!m_cam_bbox.intersectsWithBox(bb))
        return true;

    using namespace irr;
    using namespace core;
    quaternion edges[8] =
    {
        quaternion(bb.MinEdge.X, bb.MinEdge.Y, bb.MinEdge.Z, 1.0f),
        quaternion(bb.MaxEdge.X, bb.MinEdge.Y, bb.MinEdge.Z, 1.0f),
        quaternion(bb.MinEdge.X, bb.MaxEdge.Y, bb.MinEdge.Z, 1.0f),
        quaternion(bb.MaxEdge.X, bb.MaxEdge.Y, bb.MinEdge.Z, 1.0f),
        quaternion(bb.MinEdge.X, bb.MinEdge.Y, bb.MaxEdge.Z, 1.0f),
        quaternion(bb.MaxEdge.X, bb.MinEdge.Y, bb.MaxEdge.Z, 1.0f),
        quaternion(bb.MinEdge.X, bb.MaxEdge.Y, bb.MaxEdge.Z, 1.0f),
        quaternion(bb.MaxEdge.X, bb.MaxEdge.Y, bb.MaxEdge.Z, 1.0f)
    };

    for (int i = 0; i < 6; i++)
    {
        bool culled = true;
        for (int j = 0; j < 8; j++)
        {
            if (m_frustum[i].dotProduct(edges[j]) >= 0.0)
            {
                culled = false;
                break;
            }
        }
        if (culled)
            return true;
    }
    return false;
}   // isCulled

// ----------------------------------------------------------------------------
bool GECullingTool::isCulled(GESPMBuffer* buffer, irr::scene::ISceneNode* node)
{
    irr::core::aabbox3df bb = buffer->getBoundingBox();
    node->getAbsoluteTransformation().transformBoxEx(bb);
    return isCulled(bb);
}   // isCulled

}
