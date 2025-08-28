#ifndef HEADER_GE_VULKAN_ATTACHMENT_TEXTURE_HPP
#define HEADER_GE_VULKAN_ATTACHMENT_TEXTURE_HPP

#include "ge_vulkan_texture.hpp"

namespace GE
{
class GEVulkanDriver;
class GEVulkanAttachmentTexture : public GEVulkanTexture
{
public:
    // ------------------------------------------------------------------------
    GEVulkanAttachmentTexture(GEVulkanDriver* vk,
                              const core::dimension2d<u32>& size,
                              VkFormat format, VkImageUsageFlags iu,
                              VkImageAspectFlags ia);
    // ------------------------------------------------------------------------
    virtual ~GEVulkanAttachmentTexture()                                     {}
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
    virtual video::E_DRIVER_TYPE getDriverType() const
                                                  { return video::EDT_VULKAN; }
    // ------------------------------------------------------------------------
    virtual video::ECOLOR_FORMAT getColorFormat() const
                                                { return video::ECF_A8R8G8B8; }
    // ------------------------------------------------------------------------
    virtual u32 getPitch() const                                  { return 0; }
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
    static GEVulkanAttachmentTexture* createDepthTexture(
                       GEVulkanDriver* vk, const core::dimension2d<u32>& size,
                       bool lazy_allocation = true);

};   // GEVulkanAttachmentTexture

}

#endif
