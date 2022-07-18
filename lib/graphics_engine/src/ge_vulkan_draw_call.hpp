#ifndef HEADER_GE_VULKAN_DRAW_CALL_HPP
#define HEADER_GE_VULKAN_DRAW_CALL_HPP

#include <unordered_map>
#include <vector>

#include "vulkan_wrapper.h"

#include "matrix4.h"

namespace irr
{
    namespace scene { class ISceneNode; }
}

namespace GE
{
class GECullingTool;
class GESPMBuffer;
class GEVulkanCameraSceneNode;

class GEVulkanDrawCall
{
private:
    std::unordered_map<GESPMBuffer*, std::vector<irr::scene::ISceneNode*> > m_visible_nodes;

    GECullingTool* m_culling_tool;

    std::vector<VkDrawIndexedIndirectCommand> m_cmds;

    std::vector<irr::core::matrix4> m_visible_trans;
public:
    // ------------------------------------------------------------------------
    GEVulkanDrawCall();
    // ------------------------------------------------------------------------
    ~GEVulkanDrawCall();
    // ------------------------------------------------------------------------
    void addNode(irr::scene::ISceneNode* node);
    // ------------------------------------------------------------------------
    void prepare(GEVulkanCameraSceneNode* cam);
    // ------------------------------------------------------------------------
    void generate();
};   // GEVulkanDrawCall

}

#endif
