#include "ge_vulkan_camera_scene_node.hpp"

#include "ge_vulkan_dynamic_buffer.hpp"

namespace GE
{
// ----------------------------------------------------------------------------
GEVulkanCameraSceneNode::GEVulkanCameraSceneNode(irr::scene::ISceneNode* parent,
                                                 irr::scene::ISceneManager* mgr,
                                                 irr::s32 id,
                                           const irr::core::vector3df& position,
                                             const irr::core::vector3df& lookat)
                       : CCameraSceneNode(parent, mgr, id, position, lookat)
{
    m_buffer = new GEVulkanDynamicBuffer(GVDBT_GPU_RAM,
        VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, sizeof(GEVulkanCameraUBO));
}   // GEVulkanCameraSceneNode

// ----------------------------------------------------------------------------
GEVulkanCameraSceneNode::~GEVulkanCameraSceneNode()
{
    delete m_buffer;
}   // ~GEVulkanCameraSceneNode

// ----------------------------------------------------------------------------
void GEVulkanCameraSceneNode::render()
{
    irr::scene::CCameraSceneNode::render();

    m_ubo_data.m_view_matrix = ViewArea.getTransform(irr::video::ETS_VIEW);
    m_ubo_data.m_projection_matrix = ViewArea.getTransform(irr::video::ETS_PROJECTION);

    irr::core::matrix4 mat;
    ViewArea.getTransform(irr::video::ETS_VIEW).getInverse(mat);
    m_ubo_data.m_inverse_view_matrix = mat;

    ViewArea.getTransform(irr::video::ETS_PROJECTION).getInverse(mat);
    m_ubo_data.m_inverse_projection_matrix = mat;

    mat = ViewArea.getTransform(irr::video::ETS_PROJECTION) *
        ViewArea.getTransform(irr::video::ETS_VIEW);

    m_ubo_data.m_projection_view_matrix = mat;
}   // render

}
