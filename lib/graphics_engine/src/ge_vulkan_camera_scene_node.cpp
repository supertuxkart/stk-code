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
    // https://matthewwellings.com/blog/the-new-vulkan-coordinate-system/
    // Vulkan clip space has inverted Y and half Z
    irr::core::matrix4 clip;
    clip[5] = -1.0f;
    clip[10] = 0.5f;
    clip[14] = 0.5f;
    m_ubo_data.m_projection_matrix = clip * m_ubo_data.m_projection_matrix;
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

    VkViewport vp = {};
    float scale = getGEConfig()->m_render_scale;
    if (vk->getSeparateRTTTexture())
        scale = 1.0f;
    vp.x = m_viewport.UpperLeftCorner.X * scale;
    vp.y = m_viewport.UpperLeftCorner.Y * scale;
    vp.width = m_viewport.getWidth() * scale;
    vp.height = m_viewport.getHeight() * scale;
    vk->getRotatedViewport(&vp, true/*handle_rtt*/);

    m_ubo_data.m_viewport.UpperLeftCorner.X = vp.x;
    m_ubo_data.m_viewport.UpperLeftCorner.Y = vp.y;
    m_ubo_data.m_viewport.LowerRightCorner.X = vp.width;
    m_ubo_data.m_viewport.LowerRightCorner.Y = vp.height;
}   // render

// ----------------------------------------------------------------------------
irr::core::matrix4 GEVulkanCameraSceneNode::getPVM() const
{
    // Use the original unedited matrix for culling
    return ViewArea.getTransform(irr::video::ETS_PROJECTION) *
        ViewArea.getTransform(irr::video::ETS_VIEW);
}   // getPVM

}
