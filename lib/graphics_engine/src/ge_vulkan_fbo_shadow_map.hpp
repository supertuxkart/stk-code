#ifndef HEADER_GE_VULKAN_FBO_SHADOW_MAP_HPP
#define HEADER_GE_VULKAN_FBO_SHADOW_MAP_HPP

#include "ge_vulkan_texture.hpp"

namespace GE
{
class GEVulkanFBOShadowMap : public GEVulkanTexture
{
private:
    VkRenderPass m_rtt_render_pass;

    VkFramebuffer m_rtt_frame_buffer;

    std::vector<VkImageView> m_frame_buffer_image_views;
public:
    // ------------------------------------------------------------------------
    GEVulkanFBOShadowMap(GEVulkanDriver* vk);
    // ------------------------------------------------------------------------
    virtual ~GEVulkanFBOShadowMap();
    // ------------------------------------------------------------------------
    void createRTT();
    // ------------------------------------------------------------------------
    VkRenderPass getRTTRenderPass() const         { return m_rtt_render_pass; }
    // ------------------------------------------------------------------------
    VkFramebuffer getRTTFramebuffer() const      { return m_rtt_frame_buffer; }

};   // GEVulkanFBOTexture

}

#endif
