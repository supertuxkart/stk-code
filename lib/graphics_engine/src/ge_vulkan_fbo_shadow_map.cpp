#include "ge_vulkan_fbo_shadow_map.hpp"

#include "ge_main.hpp"
#include "ge_vulkan_driver.hpp"
#include "ge_vulkan_shadow_camera_scene_node.hpp"

#include <array>
#include <exception>
#include <stdexcept>

namespace GE
{
GEVulkanFBOShadowMap::GEVulkanFBOShadowMap(GEVulkanDriver* vk)
                  : GEVulkanTexture()
{
    m_vk = vk;
    m_vulkan_device = m_vk->getDevice();
    m_image = VK_NULL_HANDLE;
    m_vma_allocation = VK_NULL_HANDLE;
    m_has_mipmaps = false;
    m_locked_data = NULL;
    m_size = m_orig_size = m_max_size = 
        irr::core::dimension2du(GEVulkanShadowCameraSceneNode::getShadowMapSize(),
                                GEVulkanShadowCameraSceneNode::getShadowMapSize());
    m_layer_count = GVSCC_COUNT;
    m_image_view_type = VK_IMAGE_VIEW_TYPE_2D_ARRAY;

    std::vector<VkFormat> preferred =
    {
        VK_FORMAT_D16_UNORM,
        VK_FORMAT_D32_SFLOAT,
        VK_FORMAT_D24_UNORM_S8_UINT,
        VK_FORMAT_D32_SFLOAT_S8_UINT
    };
    m_internal_format = m_vk->findSupportedFormat(preferred,
        VK_IMAGE_TILING_OPTIMAL,
        VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT);

    if (!createImage(VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT |
        VK_IMAGE_USAGE_SAMPLED_BIT))
        throw std::runtime_error("createImage failed for depth texture");

    if (!createImageView(VK_IMAGE_ASPECT_DEPTH_BIT))
        throw std::runtime_error("createImageView failed for depth texture");
    
    m_rtt_render_pass = VK_NULL_HANDLE;
    m_rtt_frame_buffer = VK_NULL_HANDLE;
}   // GEVulkanFBOTexture

// ----------------------------------------------------------------------------
GEVulkanFBOShadowMap::~GEVulkanFBOShadowMap()
{
    if (m_rtt_frame_buffer != VK_NULL_HANDLE)
    {
        clearVulkanData();
        vkDestroyFramebuffer(m_vk->getDevice(), m_rtt_frame_buffer, NULL);
        m_vk->handleDeletedTextures();

        m_image_view.reset();
        m_image = VK_NULL_HANDLE;
        m_vma_allocation = VK_NULL_HANDLE;
    }
    if (m_rtt_render_pass != VK_NULL_HANDLE)
        vkDestroyRenderPass(m_vk->getDevice(), m_rtt_render_pass, NULL);
    
    for (int i = 0; i < m_frame_buffer_image_views.size(); i++)
    {
        vkDestroyImageView(m_vulkan_device, m_frame_buffer_image_views[i], NULL);
    }
}   // ~GEVulkanFBOTexture

// ----------------------------------------------------------------------------
void GEVulkanFBOShadowMap::createRTT()
{
    std::array<VkAttachmentDescription, GVSCC_COUNT> shadow_attachments = {};
    for (int i = 0; i < shadow_attachments.size(); i++)
    {
        shadow_attachments[i].format = m_internal_format;
        shadow_attachments[i].samples = VK_SAMPLE_COUNT_1_BIT;
        shadow_attachments[i].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        shadow_attachments[i].storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        shadow_attachments[i].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        shadow_attachments[i].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        shadow_attachments[i].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        shadow_attachments[i].finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
    }

    std::array<VkAttachmentReference, GVSCC_COUNT> shadow_attachment_refs = {};
    for (int i = 0; i < shadow_attachment_refs.size(); i++)
    {
        shadow_attachment_refs[i].attachment = i;
        shadow_attachment_refs[i].layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
    }

    std::array<VkSubpassDescription, GVSCC_COUNT> subpasses = {};
    for (int i = 0; i < subpasses.size(); i++)
    {
        subpasses[i].pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
        subpasses[i].colorAttachmentCount = 0;
        subpasses[i].pColorAttachments = NULL;
        subpasses[i].pDepthStencilAttachment = &shadow_attachment_refs[i];
    }

    std::array<VkSubpassDependency, GVSCC_COUNT * 2> dependencies = {};
    for (int i = 0; i < subpasses.size(); i++)
    {
        dependencies[2 * i].srcSubpass = VK_SUBPASS_EXTERNAL;
        dependencies[2 * i].dstSubpass = i;
        dependencies[2 * i].srcStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
        dependencies[2 * i].dstStageMask = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
        dependencies[2 * i].srcAccessMask = VK_ACCESS_SHADER_READ_BIT;
        dependencies[2 * i].dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
        dependencies[2 * i].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

        dependencies[2 * i + 1].srcSubpass = i;
        dependencies[2 * i + 1].dstSubpass = VK_SUBPASS_EXTERNAL;
        dependencies[2 * i + 1].srcStageMask = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
        dependencies[2 * i + 1].dstStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
        dependencies[2 * i + 1].srcAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
        dependencies[2 * i + 1].dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
        dependencies[2 * i + 1].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;
    }

    VkRenderPassCreateInfo render_pass_info = {};
    render_pass_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    render_pass_info.attachmentCount = shadow_attachments.size();
    render_pass_info.pAttachments = shadow_attachments.data();
    render_pass_info.subpassCount = subpasses.size();
    render_pass_info.pSubpasses = subpasses.data();
    render_pass_info.dependencyCount = dependencies.size();
    render_pass_info.pDependencies = dependencies.data();

    VkResult result = vkCreateRenderPass(m_vk->getDevice(), &render_pass_info, NULL,
        &m_rtt_render_pass);

    if (result != VK_SUCCESS)
        throw std::runtime_error("vkCreateRenderPass failed");

    // Separate layers for framebuffer
    for (int i = 0; i < GVSCC_COUNT; i++)
    {
        VkImageViewCreateInfo view_info = {};
        view_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        view_info.image = m_image;
        view_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
        view_info.format = m_internal_format;
        view_info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
        view_info.subresourceRange.baseMipLevel = 0;
        view_info.subresourceRange.levelCount = getMipmapLevels();
        view_info.subresourceRange.baseArrayLayer = i;
        view_info.subresourceRange.layerCount = 1;

        VkImageView view_ptr = VK_NULL_HANDLE;
        VkResult result = vkCreateImageView(m_vulkan_device, &view_info, NULL,
            &view_ptr);
        
        m_frame_buffer_image_views.push_back(view_ptr);
    }

    VkFramebufferCreateInfo framebuffer_info = {};
    framebuffer_info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
    framebuffer_info.renderPass = m_rtt_render_pass;
    framebuffer_info.attachmentCount = m_frame_buffer_image_views.size();
    framebuffer_info.pAttachments = m_frame_buffer_image_views.data();
    framebuffer_info.width = m_size.Width;
    framebuffer_info.height = m_size.Height;
    framebuffer_info.layers = 1;

    result = vkCreateFramebuffer(m_vk->getDevice(), &framebuffer_info,
        NULL, &m_rtt_frame_buffer);
    if (result != VK_SUCCESS)
        throw std::runtime_error("vkCreateFramebuffer failed");
}   // createRTT

}
