#ifndef HEADER_GE_VULKAN_TEXTURE_HPP
#define HEADER_GE_VULKAN_TEXTURE_HPP

#include "vulkan_wrapper.h"

#include "ge_vma.hpp"
#include "ge_spin_lock.hpp"

#include <algorithm>
#include <cmath>
#include <functional>
#include <string>
#include <vector>
#include <ITexture.h>

using namespace irr;

namespace GE
{
class GEVulkanDriver;
class GEVulkanTexture : public video::ITexture
{
protected:
    core::dimension2d<u32> m_size, m_orig_size, m_max_size;

    std::function<void(video::IImage*)> m_image_mani;

    uint8_t* m_locked_data;

    VkDevice m_vulkan_device;

    VkImage m_image;

    VmaAllocation m_vma_allocation;

    VkImageView m_image_view;

    unsigned int m_texture_size;

    const bool m_disable_reload;

    bool m_has_mipmaps;

    GESpinLock m_size_lock;

    GESpinLock m_image_view_lock;

    io::path m_full_path;

    VkFormat m_internal_format;

    GEVulkanDriver* m_vk;

    // ------------------------------------------------------------------------
    bool createTextureImage(uint8_t* texture_data, bool generate_hq_mipmap);
    // ------------------------------------------------------------------------
    bool createImage(VkImageUsageFlags usage);
    // ------------------------------------------------------------------------
    bool createImageView(VkImageAspectFlags aspect_flags);
    // ------------------------------------------------------------------------
    void transitionImageLayout(VkCommandBuffer command_buffer,
                               VkImageLayout old_layout,
                               VkImageLayout new_layout);
    // ------------------------------------------------------------------------
    void copyBufferToImage(VkCommandBuffer command_buffer, VkBuffer buffer,
                           u32 w, u32 h, s32 x, s32 y, u32 offset,
                           u32 mipmap_level);
    // ------------------------------------------------------------------------
    void upload(uint8_t* data, bool generate_hq_mipmap = false);
    // ------------------------------------------------------------------------
    void clearVulkanData();
    // ------------------------------------------------------------------------
    void reloadInternal();
    // ------------------------------------------------------------------------
    void bgraConversion(uint8_t* img_data);
    // ------------------------------------------------------------------------
    uint8_t* getTextureData();
    // ------------------------------------------------------------------------
    std::vector<std::pair<core::dimension2du, unsigned> > getMipmapSizes()
    {
        std::vector<std::pair<core::dimension2du, unsigned> > mipmap_sizes;
        unsigned width = m_size.Width;
        unsigned height = m_size.Height;
        mipmap_sizes.emplace_back(core::dimension2du(width, height),
            0);
        while (true)
        {
            width = width < 2 ? 1 : width >> 1;
            height = height < 2 ? 1 : height >> 1;
            mipmap_sizes.emplace_back(core::dimension2du(width, height), 0);
            if (width == 1 && height == 1)
            {
                break;
            }
        }
        return mipmap_sizes;
    }
    // ------------------------------------------------------------------------
    void generateHQMipmap(void* in,
                          std::vector<std::pair<core::dimension2du,
                          unsigned> >& mms, uint8_t* out);
    // ------------------------------------------------------------------------
    unsigned getMipmapLevels() const
    {
        if (!m_has_mipmaps)
            return 1;
        return std::floor(std::log2(std::max(m_size.Width, m_size.Height))) + 1;
    }
    // ------------------------------------------------------------------------
    bool isSingleChannel() const
                            { return m_internal_format == VK_FORMAT_R8_UNORM; }
    // ------------------------------------------------------------------------
    GEVulkanTexture() : video::ITexture(""), m_disable_reload(true)          {}
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
    {
        m_size_lock.lock();
        m_size_lock.unlock();
        return m_orig_size;
    }
    // ------------------------------------------------------------------------
    virtual const core::dimension2d<u32>& getSize() const
    {
        m_size_lock.lock();
        m_size_lock.unlock();
        return m_size;
    }
    // ------------------------------------------------------------------------
    virtual video::E_DRIVER_TYPE getDriverType() const
                                                  { return video::EDT_VULKAN; }
    // ------------------------------------------------------------------------
    virtual video::ECOLOR_FORMAT getColorFormat() const
                                                { return video::ECF_A8R8G8B8; }
    // ------------------------------------------------------------------------
    virtual u32 getPitch() const                                  { return 0; }
    // ------------------------------------------------------------------------
    virtual bool hasMipMaps() const                   { return m_has_mipmaps; }
    // ------------------------------------------------------------------------
    virtual void regenerateMipMapLevels(void* mipmap_data = NULL)            {}
    // ------------------------------------------------------------------------
    virtual u64 getTextureHandler() const
    {
        m_image_view_lock.lock();
        m_image_view_lock.unlock();
        return (u64)m_image_view;
    }
    // ------------------------------------------------------------------------
    virtual unsigned int getTextureSize() const
    {
        m_image_view_lock.lock();
        m_image_view_lock.unlock();
        return m_texture_size;
    }
    // ------------------------------------------------------------------------
    virtual void reload();
    // ------------------------------------------------------------------------
    virtual void updateTexture(void* data, irr::video::ECOLOR_FORMAT format,
                               u32 w, u32 h, u32 x, u32 y);
    // ------------------------------------------------------------------------
    VkFormat getInternalFormat() const            { return m_internal_format; }
};   // GEVulkanTexture

}

#endif
