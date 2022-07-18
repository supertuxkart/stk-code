#include "ge_vulkan_draw_call.hpp"

#include "ge_culling_tool.hpp"
#include "ge_spm_buffer.hpp"
#include "ge_vulkan_animated_mesh_scene_node.hpp"
#include "ge_vulkan_camera_scene_node.hpp"
#include "ge_vulkan_mesh_scene_node.hpp"

namespace GE
{
// ----------------------------------------------------------------------------
GEVulkanDrawCall::GEVulkanDrawCall()
{
    m_culling_tool = new GECullingTool;
}   // GEVulkanDrawCall

// ----------------------------------------------------------------------------
GEVulkanDrawCall::~GEVulkanDrawCall()
{
    delete m_culling_tool;
}   // ~GEVulkanDrawCall

// ----------------------------------------------------------------------------
void GEVulkanDrawCall::addNode(irr::scene::ISceneNode* node)
{
    irr::scene::IMesh* mesh;
    if (node->getType() == irr::scene::ESNT_ANIMATED_MESH)
    {
        mesh = static_cast<
            GEVulkanAnimatedMeshSceneNode*>(node)->getMesh();
    }
    else if (node->getType() == irr::scene::ESNT_MESH)
    {
        mesh = static_cast<irr::scene::IMeshSceneNode*>(node)->getMesh();
        for (unsigned i = 0; i < mesh->getMeshBufferCount(); i++)
        {
            irr::scene::IMeshBuffer* b = mesh->getMeshBuffer(i);
            if (b->getVertexType() != irr::video::EVT_SKINNED_MESH)
                return;
        }
    }
    else
        return;

    for (unsigned i = 0; i < mesh->getMeshBufferCount(); i++)
    {
        GESPMBuffer* buffer = static_cast<GESPMBuffer*>(
            mesh->getMeshBuffer(i));
        if (m_culling_tool->isCulled(buffer, node))
            continue;
        m_visible_nodes[buffer].push_back(node);
    }
}   // addNode

// ----------------------------------------------------------------------------
void GEVulkanDrawCall::generate()
{
    unsigned accumulated_instance = 0;
    for (auto& p : m_visible_nodes)
    {
        unsigned visible_count = p.second.size();
        if (visible_count != 0)
        {
            for (auto* node : p.second)
                m_visible_trans.push_back(node->getAbsoluteTransformation());
            VkDrawIndexedIndirectCommand cmd;
            cmd.indexCount = p.first->getIndexCount();
            cmd.instanceCount = visible_count;
            cmd.firstIndex = p.first->getIBOOffset();
            cmd.vertexOffset = p.first->getVBOOffset();
            cmd.firstInstance = accumulated_instance;
            accumulated_instance += visible_count;
            m_cmds.push_back(cmd);
        }
    }
}   // generate

// ----------------------------------------------------------------------------
void GEVulkanDrawCall::prepare(GEVulkanCameraSceneNode* cam)
{
    m_visible_nodes.clear();
    m_cmds.clear();
    m_visible_trans.clear();
    m_culling_tool->init(cam);
}   // prepare

}
