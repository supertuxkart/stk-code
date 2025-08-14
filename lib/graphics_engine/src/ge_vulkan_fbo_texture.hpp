#ifndef HEADER_GE_VULKAN_FBO_TEXTURE_HPP
#define HEADER_GE_VULKAN_FBO_TEXTURE_HPP

#include "ge_vulkan_texture.hpp"

namespace GE
{
class GEVulkanAttachmentTexture;
class GEVulkanFBOTexture : public GEVulkanTexture
{
protected:
    GEVulkanAttachmentTexture* m_depth_texture;

    std::vector<VkRenderPass> m_rtt_render_pass;

    std::vector<VkFramebuffer> m_rtt_frame_buffer;
public:
    // ------------------------------------------------------------------------
    GEVulkanFBOTexture(GEVulkanDriver* vk, const core::dimension2d<u32>& size,
                       bool lazy_depth = true);
    // ------------------------------------------------------------------------
    virtual ~GEVulkanFBOTexture();
    // ------------------------------------------------------------------------
    virtual void* lock(video::E_TEXTURE_LOCK_MODE mode =
                       video::ETLM_READ_WRITE, u32 mipmap_level = 0)
                                                               { return NULL; }
    // ------------------------------------------------------------------------
    virtual void unlock()                                                    {}
    // ------------------------------------------------------------------------
    virtual const core::dimension2d<u32>& getOriginalSize() const
                                                        { return m_orig_size; }
    // ------------------------------------------------------------------------
    virtual const core::dimension2d<u32>& getSize() const    { return m_size; }
    // ------------------------------------------------------------------------
    virtual bool hasMipMaps() const                           { return false; }
    // ------------------------------------------------------------------------
    virtual void regenerateMipMapLevels(void* mipmap_data = NULL)            {}
    // ------------------------------------------------------------------------
    virtual u64 getTextureHandler() const
                                  { return (u64)(m_image_view.get()->load()); }
    // ------------------------------------------------------------------------
    virtual void reload()                                                    {}
    // ------------------------------------------------------------------------
    virtual void updateTexture(void* data, irr::video::ECOLOR_FORMAT format,
                               u32 w, u32 h, u32 x, u32 y)                   {}
    // ------------------------------------------------------------------------
    virtual std::shared_ptr<std::atomic<VkImageView> > getImageView(
                                                       bool srgb = false) const
                                                       { return m_image_view; }
    // ------------------------------------------------------------------------
    virtual void createRTT();
    // ------------------------------------------------------------------------
    virtual void createOutputImage(VkImageUsageFlags usage =
        VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT);
    // ------------------------------------------------------------------------
    VkRenderPass getRTTRenderPass(unsigned id = 0) const
                                           { return m_rtt_render_pass.at(id); }
    // ------------------------------------------------------------------------
    unsigned getRTTRenderPassCount() const
                                           { return m_rtt_render_pass.size(); }
    // ------------------------------------------------------------------------
    VkFramebuffer getRTTFramebuffer(unsigned id = 0) const
                                          { return m_rtt_frame_buffer.at(id); }
    // ------------------------------------------------------------------------
    GEVulkanAttachmentTexture* getDepthTexture() const
                                                    { return m_depth_texture; }
    // ------------------------------------------------------------------------
    virtual bool isDeferredFBO() const                        { return false; }
    // ------------------------------------------------------------------------
    virtual bool useSwapChainOutput() const                   { return false; }
    // ------------------------------------------------------------------------
    virtual unsigned getZeroClearCountForPass(unsigned pass) const
                                                                  { return 0; }
    // ------------------------------------------------------------------------
    virtual VkDescriptorSetLayout getDescriptorSetLayout(unsigned id) const
                                                     { return VK_NULL_HANDLE; }
    // ------------------------------------------------------------------------
    virtual const VkDescriptorSet* getDescriptorSet(unsigned id) const
                                                               { return NULL; }

};   // GEVulkanFBOTexture

}

#endif
