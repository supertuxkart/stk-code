#ifndef HEADER_GE_VULKAN_TEXTURE_HPP
#define HEADER_GE_VULKAN_TEXTURE_HPP

#include "vulkan_wrapper.h"

#include <functional>
#include <string>
#include <ITexture.h>

using namespace irr;

namespace GE
{
class GEVulkanTexture : public video::ITexture
{
private:
    core::dimension2d<u32> m_size, m_orig_size;

    std::function<void(video::IImage*)> m_image_mani;

    uint8_t* m_locked_data;

    VkDevice m_vulkan_device;

    VkImage m_image;

    VkDeviceMemory m_image_memory;

    VkImageView m_image_view;

    unsigned int m_texture_size;

    const bool m_disable_reload;

    bool m_single_channel;

    // ------------------------------------------------------------------------
    bool createTextureImage(uint8_t* texture_data);
    // ------------------------------------------------------------------------
    bool createImage(VkImageUsageFlags usage);
    // ------------------------------------------------------------------------
    bool createImageView(VkImageAspectFlags aspect_flags);
    // ------------------------------------------------------------------------
    void transitionImageLayout(VkImageLayout old_layout,
                               VkImageLayout new_layout);
    // ------------------------------------------------------------------------
    void copyBufferToImage(VkBuffer buffer, u32 w, u32 h, s32 x, s32 y);
    // ------------------------------------------------------------------------
    void upload(uint8_t* data);
    // ------------------------------------------------------------------------
    void clearVulkanData();
    // ------------------------------------------------------------------------
public:
    // ------------------------------------------------------------------------
    GEVulkanTexture(const std::string& path,
                    std::function<void(video::IImage*)> image_mani = nullptr);
    // ------------------------------------------------------------------------
    GEVulkanTexture(video::IImage* img, const std::string& name);
    // ------------------------------------------------------------------------
    GEVulkanTexture(const std::string& name, unsigned int size,
                    bool single_channel);
    // ------------------------------------------------------------------------
    virtual ~GEVulkanTexture();
    // ------------------------------------------------------------------------
    virtual void* lock(video::E_TEXTURE_LOCK_MODE mode =
                       video::ETLM_READ_WRITE, u32 mipmap_level = 0);
    // ------------------------------------------------------------------------
    virtual void unlock()
    {
        if (m_locked_data)
        {
            delete [] m_locked_data;
            m_locked_data = NULL;
        }
    }
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
    virtual u64 getTextureHandler() const         { return (u64)m_image_view; }
    // ------------------------------------------------------------------------
    virtual unsigned int getTextureSize() const      { return m_texture_size; }
    // ------------------------------------------------------------------------
    virtual void reload();
    // ------------------------------------------------------------------------
    virtual void updateTexture(void* data, irr::video::ECOLOR_FORMAT format,
                               u32 w, u32 h, u32 x, u32 y);
};   // GEVulkanTexture

}

#endif
