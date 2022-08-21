#include "ge_vulkan_camera_scene_node.hpp"

#include "ge_main.hpp"
#include "ge_vulkan_driver.hpp"
#include "ge_vulkan_scene_manager.hpp"

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
    static_cast<GEVulkanSceneManager*>(SceneManager)->addDrawCall(this);
}   // GEVulkanCameraSceneNode

// ----------------------------------------------------------------------------
GEVulkanCameraSceneNode::~GEVulkanCameraSceneNode()
{
    static_cast<GEVulkanSceneManager*>(SceneManager)->removeDrawCall(this);
}   // ~GEVulkanCameraSceneNode

// ----------------------------------------------------------------------------
void GEVulkanCameraSceneNode::render()
{
    irr::scene::CCameraSceneNode::render();

    m_ubo_data.m_view_matrix = ViewArea.getTransform(irr::video::ETS_VIEW);
    m_ubo_data.m_projection_matrix = ViewArea.getTransform(irr::video::ETS_PROJECTION);
    // Irrlicht matrix4 was originally designed for OpenGL,
    // where the Y coordinate of the clip coordinates is inverted.
    // The easiest way to compensate for that is to flip the sign on the
    // scaling factor of the Y axis in the projection matrix.
    m_ubo_data.m_projection_matrix(1, 1) *= -1.0f;
    GEVulkanDriver* vk = getVKDriver();
    if (!vk->getRTTTexture())
    {
        m_ubo_data.m_projection_matrix = vk->getPreRotationMatrix() *
            m_ubo_data.m_projection_matrix;
    }

    irr::core::matrix4 mat;
    ViewArea.getTransform(irr::video::ETS_VIEW).getInverse(mat);
    m_ubo_data.m_inverse_view_matrix = mat;

    m_ubo_data.m_projection_matrix.getInverse(mat);
    m_ubo_data.m_inverse_projection_matrix = mat;

    mat = m_ubo_data.m_projection_matrix * m_ubo_data.m_view_matrix;

    m_ubo_data.m_projection_view_matrix = mat;

    m_ubo_data.m_projection_view_matrix.getInverse(
        m_ubo_data.m_inverse_projection_view_matrix);
}   // render

// ----------------------------------------------------------------------------
irr::core::matrix4 GEVulkanCameraSceneNode::getPVM() const
{
    // Use the original unedited matrix for culling
    return ViewArea.getTransform(irr::video::ETS_PROJECTION) *
        ViewArea.getTransform(irr::video::ETS_VIEW);
}   // getPVM

}
