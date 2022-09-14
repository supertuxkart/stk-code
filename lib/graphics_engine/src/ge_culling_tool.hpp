#ifndef HEADER_GE_CULLING_TOOL_HPP
#define HEADER_GE_CULLING_TOOL_HPP

#include "aabbox3d.h"
#include "quaternion.h"
#include "matrix4.h"

namespace irr
{
    namespace scene { class ISceneNode; }
}

namespace GE
{
class GESPMBuffer;
class GEVulkanCameraSceneNode;

class GECullingTool
{
private:
    irr::core::quaternion m_frustum[6];

    irr::core::aabbox3df m_cam_bbox;
public:
    // ------------------------------------------------------------------------
    void init(GEVulkanCameraSceneNode* cam);
    // ------------------------------------------------------------------------
    bool isCulled(irr::core::aabbox3df& bb);
    // ------------------------------------------------------------------------
    bool isCulled(GESPMBuffer* buffer, irr::scene::ISceneNode* node);
};   // GECullingTool

}

#endif
