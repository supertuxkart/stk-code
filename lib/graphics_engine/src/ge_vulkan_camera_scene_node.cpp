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
    memcpy(m_ubo_data.m_view_matrix.data(),
        ViewArea.getTransform(irr::video::ETS_VIEW).pointer(),
        16 * sizeof(float));
    memcpy(m_ubo_data.m_projection_matrix.data(),
        ViewArea.getTransform(irr::video::ETS_PROJECTION).pointer(),
        16 * sizeof(float));
    irr::core::matrix4 mat;
    ViewArea.getTransform(irr::video::ETS_VIEW).getInverse(mat);
    memcpy(m_ubo_data.m_inverse_view_matrix.data(), mat.pointer(),
        16 * sizeof(float));
    ViewArea.getTransform(irr::video::ETS_PROJECTION).getInverse(mat);
    memcpy(m_ubo_data.m_inverse_projection_matrix.data(), mat.pointer(),
        16 * sizeof(float));
    mat = ViewArea.getTransform(irr::video::ETS_PROJECTION) *
        ViewArea.getTransform(irr::video::ETS_VIEW);
    memcpy(m_ubo_data.m_projection_view_matrix.data(), mat.pointer(),
        16 * sizeof(float));
}   // render

}
