#ifndef HEADER_GE_VULKAN_TEXTURE_HPP
#define HEADER_GE_VULKAN_TEXTURE_HPP

#include "vulkan_wrapper.h"

#include "ge_vma.hpp"
#include "ge_spin_lock.hpp"

#include <algorithm>
#include <atomic>
#include <cmath>
#include <functional>
#include <memory>
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

    VmaAllocationInfo m_vma_info;

    std::shared_ptr<std::atomic<VkImageView> > m_image_view;

    std::shared_ptr<std::atomic<VkImageView> > m_image_view_srgb;

    std::shared_ptr<std::atomic<VkImageView> > m_placeholder_view;

    unsigned m_layer_count;

    VkImageViewType m_image_view_type;

    const bool m_disable_reload;

    bool m_has_mipmaps;

    bool m_ondemand_load;

    mutable std::atomic<bool> m_ondemand_loading;

    GESpinLock m_size_lock;

    mutable GESpinLock m_image_view_lock;

    GESpinLock m_thread_loading_lock;

    io::path m_full_path;

    VkFormat m_internal_format;

    GEVulkanDriver* m_vk;

    // ------------------------------------------------------------------------
    VkFormat getSRGBformat(VkFormat format)
    {
        if (format == VK_FORMAT_R8G8B8A8_UNORM)
            return VK_FORMAT_R8G8B8A8_SRGB;
        else if (format == VK_FORMAT_ASTC_4x4_UNORM_BLOCK)
            return VK_FORMAT_ASTC_4x4_SRGB_BLOCK;
        else if (format == VK_FORMAT_BC7_UNORM_BLOCK)
            return VK_FORMAT_BC7_SRGB_BLOCK;
        else if (format == VK_FORMAT_BC3_UNORM_BLOCK)
            return VK_FORMAT_BC3_SRGB_BLOCK;
        return format;
    }
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
                           u32 mipmap_level, u32 layer_level);
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
    void setPlaceHolderView();
    // ------------------------------------------------------------------------
    std::shared_ptr<std::atomic<VkImageView> > getImageViewLive(
                                                      bool srgb = false) const;
    // ------------------------------------------------------------------------
    bool waitImageView() const
    {
        if (!m_ondemand_load)
        {
            m_image_view_lock.lock();
            m_image_view_lock.unlock();
        }
        else
        {
            while (m_ondemand_loading.load());
            if (m_image == VK_NULL_HANDLE)
                return false;
        }
        return true;
    }
    // ------------------------------------------------------------------------
    GEVulkanTexture() : video::ITexture(""), m_vma_info(), m_layer_count(1),
                        m_image_view_type(VK_IMAGE_VIEW_TYPE_2D),
                        m_disable_reload(true), m_ondemand_load(false),
                        m_ondemand_loading(false)                            {}
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
        if (!m_ondemand_load)
        {
            m_size_lock.lock();
            m_size_lock.unlock();
        }
        return m_orig_size;
    }
    // ------------------------------------------------------------------------
    virtual const core::dimension2d<u32>& getSize() const
    {
        if (!m_ondemand_load)
        {
            m_size_lock.lock();
            m_size_lock.unlock();
        }
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
        if (!m_ondemand_load)
        {
            m_image_view_lock.lock();
            m_image_view_lock.unlock();
            return m_image_view ? (u64)(m_image_view.get()->load()) : 0;
        }
        else
        {
            auto image_view = getImageViewLive();
            return (u64)(image_view.get()->load());
        }
    }
    // ------------------------------------------------------------------------
    virtual unsigned int getTextureSize() const
    {
        waitImageView();
        return (unsigned int)m_vma_info.size;
    }
    // ------------------------------------------------------------------------
    virtual void reload();
    // ------------------------------------------------------------------------
    virtual void updateTexture(void* data, irr::video::ECOLOR_FORMAT format,
                               u32 w, u32 h, u32 x, u32 y);
    // ------------------------------------------------------------------------
    virtual std::shared_ptr<std::atomic<VkImageView> > getImageView(
                                                       bool srgb = false) const
    {
        if (!m_ondemand_load)
        {
            m_image_view_lock.lock();
            m_image_view_lock.unlock();
            if (srgb && m_image_view_srgb)
                return m_image_view_srgb;
            else
                return m_image_view;
        }
        else
            return getImageViewLive(srgb);
    }
    // ------------------------------------------------------------------------
    virtual bool useOnDemandLoad() const            { return m_ondemand_load; }
    // ------------------------------------------------------------------------
    virtual const io::path& getFullPath() const         { return m_full_path; }
    // ------------------------------------------------------------------------
    VkFormat getInternalFormat() const            { return m_internal_format; }
};   // GEVulkanTexture

}

#endif
