#include "ge_vulkan_texture.hpp"

#include "ge_vulkan_driver.hpp"
#include "ge_gl_utils.hpp"
#include "ge_main.hpp"
#include "ge_texture.hpp"

#include <limits>
#include <vector>

namespace GE
{
GEVulkanTexture::GEVulkanTexture(const std::string& path,
                         std::function<void(video::IImage*)> image_mani)
               : video::ITexture(path.c_str()), m_image_mani(image_mani),
                 m_locked_data(NULL),
                 m_vulkan_device(getVKDriver()->getDevice()),
                 m_image(VK_NULL_HANDLE), m_image_memory(VK_NULL_HANDLE),
                 m_image_view(VK_NULL_HANDLE), m_texture_size(0),
                 m_disable_reload(false), m_single_channel(false)
{
    reload();
}   // GEVulkanTexture

// ----------------------------------------------------------------------------
GEVulkanTexture::GEVulkanTexture(video::IImage* img, const std::string& name)
               : video::ITexture(name.c_str()), m_image_mani(nullptr),
                 m_locked_data(NULL),
                 m_vulkan_device(getVKDriver()->getDevice()),
                 m_image(VK_NULL_HANDLE), m_image_memory(VK_NULL_HANDLE),
                 m_image_view(VK_NULL_HANDLE), m_texture_size(0),
                 m_disable_reload(true), m_single_channel(false)
{
    if (!img)
        return;
    m_size = m_orig_size = img->getDimension();
    uint8_t* data = (uint8_t*)img->lock();
    upload(data);
    img->unlock();
    img->drop();
}   // GEVulkanTexture

// ----------------------------------------------------------------------------
GEVulkanTexture::GEVulkanTexture(const std::string& name, unsigned int size,
                                 bool single_channel)
           : video::ITexture(name.c_str()), m_image_mani(nullptr),
             m_locked_data(NULL), m_vulkan_device(getVKDriver()->getDevice()),
             m_image(VK_NULL_HANDLE), m_image_memory(VK_NULL_HANDLE),
             m_image_view(VK_NULL_HANDLE), m_texture_size(0),
             m_disable_reload(true), m_single_channel(single_channel)
{
    m_orig_size.Width = size;
    m_orig_size.Height = size;
    m_size = m_orig_size;

    std::vector<uint8_t> data;
    data.resize(size * size * (m_single_channel ? 1 : 4), 0);
    upload(data.data());
}   // GEVulkanTexture

// ----------------------------------------------------------------------------
GEVulkanTexture::~GEVulkanTexture()
{
    clearVulkanData();
}   // ~GEVulkanTexture

// ----------------------------------------------------------------------------
bool GEVulkanTexture::createTextureImage(uint8_t* texture_data)
{
    unsigned channels = (m_single_channel ? 1 : 4);
    VkDeviceSize image_size = m_size.Width * m_size.Height * channels;
    VkBuffer staging_buffer;
    VkDeviceMemory staging_buffer_memory;

    bool success = getVKDriver()->createBuffer(image_size,
        VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
        VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, staging_buffer,
        staging_buffer_memory);

    if (!success)
        return false;

    void* data;
    vkMapMemory(m_vulkan_device, staging_buffer_memory, 0, image_size, 0, &data);
    memcpy(data, texture_data, (size_t)(image_size));
    vkUnmapMemory(m_vulkan_device, staging_buffer_memory);

    success = createImage(VK_IMAGE_USAGE_TRANSFER_DST_BIT |
        VK_IMAGE_USAGE_SAMPLED_BIT);
    if (!success)
        return false;

    transitionImageLayout(VK_IMAGE_LAYOUT_UNDEFINED,
        VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

    copyBufferToImage(staging_buffer, m_size.Width, m_size.Height, 0, 0);

    transitionImageLayout(VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
        VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

    vkDestroyBuffer(m_vulkan_device, staging_buffer, NULL);
    vkFreeMemory(m_vulkan_device, staging_buffer_memory, NULL);

    return true;
}   // createTextureImage

// ----------------------------------------------------------------------------
bool GEVulkanTexture::createImage(VkImageUsageFlags usage)
{
    VkImageCreateInfo image_info = {};
    image_info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    image_info.imageType = VK_IMAGE_TYPE_2D;
    image_info.extent.width = m_size.Width;
    image_info.extent.height = m_size.Height;
    image_info.extent.depth = 1;
    image_info.mipLevels = 1;
    image_info.arrayLayers = 1;
    image_info.format =
        (m_single_channel ? VK_FORMAT_R8_UNORM : VK_FORMAT_R8G8B8A8_UNORM);
    image_info.tiling = VK_IMAGE_TILING_OPTIMAL;
    image_info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    image_info.usage = usage;
    image_info.samples = VK_SAMPLE_COUNT_1_BIT;
    image_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    VkResult result = vkCreateImage(m_vulkan_device, &image_info, NULL,
        &m_image);

    if (result != VK_SUCCESS)
        return false;

    VkMemoryRequirements mem_requirements;
    vkGetImageMemoryRequirements(m_vulkan_device, m_image, &mem_requirements);

    VkPhysicalDeviceMemoryProperties mem_properties;
    vkGetPhysicalDeviceMemoryProperties(getVKDriver()->getPhysicalDevice(),
        &mem_properties);

    uint32_t memory_type_index = std::numeric_limits<uint32_t>::max();
    uint32_t type_filter = mem_requirements.memoryTypeBits;
    VkMemoryPropertyFlags properties = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;

    for (uint32_t i = 0; i < mem_properties.memoryTypeCount; i++)
    {
        if ((type_filter & (1 << i)) &&
            (mem_properties.memoryTypes[i].propertyFlags & properties) == properties)
        {
            memory_type_index = i;
            break;
        }
    }

    if (memory_type_index == std::numeric_limits<uint32_t>::max())
        return false;

    VkMemoryAllocateInfo alloc_info = {};
    alloc_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    alloc_info.allocationSize = mem_requirements.size;
    alloc_info.memoryTypeIndex = memory_type_index;

    result = vkAllocateMemory(m_vulkan_device, &alloc_info, NULL,
        &m_image_memory);

    if (result != VK_SUCCESS)
        return false;

    vkBindImageMemory(m_vulkan_device, m_image, m_image_memory, 0);
    return true;
}   // createImage

// ----------------------------------------------------------------------------
void GEVulkanTexture::transitionImageLayout(VkImageLayout old_layout,
                                            VkImageLayout new_layout)
{
    VkImageMemoryBarrier barrier = {};
    barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    barrier.oldLayout = old_layout;
    barrier.newLayout = new_layout;
    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.image = m_image;
    barrier.subresourceRange.baseMipLevel = 0;
    barrier.subresourceRange.levelCount = 1;
    barrier.subresourceRange.baseArrayLayer = 0;
    barrier.subresourceRange.layerCount = 1;

    if (new_layout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL)
    {
        barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
    }
    else
    {
        barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    }

    VkPipelineStageFlags source_stage;
    VkPipelineStageFlags destination_stage;

    if (old_layout == VK_IMAGE_LAYOUT_UNDEFINED &&
        new_layout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL)
    {
        barrier.srcAccessMask = 0;
        barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

        source_stage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
        destination_stage = VK_PIPELINE_STAGE_TRANSFER_BIT;
    }
    else if (old_layout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL &&
        new_layout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
    {
        barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

        source_stage = VK_PIPELINE_STAGE_TRANSFER_BIT;
        destination_stage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
    }
    else if (old_layout == VK_IMAGE_LAYOUT_UNDEFINED &&
        new_layout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL)
    {
        barrier.srcAccessMask = 0;
        barrier.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT |
            VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

        source_stage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
        destination_stage = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
    }
    else
    {
        return;
    }

    VkCommandBuffer command_buffer = getVKDriver()->beginSingleTimeCommands();
    vkCmdPipelineBarrier(command_buffer, source_stage, destination_stage, 0, 0,
        NULL, 0, NULL, 1, &barrier);
    getVKDriver()->endSingleTimeCommands(command_buffer);
}   // transitionImageLayout

// ----------------------------------------------------------------------------
void GEVulkanTexture::copyBufferToImage(VkBuffer buffer, u32 w, u32 h, s32 x,
                                        s32 y)
{
    VkCommandBuffer command_buffer = getVKDriver()->beginSingleTimeCommands();

    VkBufferImageCopy region = {};
    region.bufferOffset = 0;
    region.bufferRowLength = 0;
    region.bufferImageHeight = 0;
    region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    region.imageSubresource.mipLevel = 0;
    region.imageSubresource.baseArrayLayer = 0;
    region.imageSubresource.layerCount = 1;
    region.imageOffset = {x, y, 0};
    region.imageExtent = {w, h, 1};

    vkCmdCopyBufferToImage(command_buffer, buffer, m_image,
        VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);

    getVKDriver()->endSingleTimeCommands(command_buffer);
}   // copyBufferToImage

// ----------------------------------------------------------------------------
bool GEVulkanTexture::createImageView(VkImageAspectFlags aspect_flags)
{
    VkImageViewCreateInfo view_info = {};
    view_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    view_info.image = m_image;
    view_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
    view_info.format =
        (m_single_channel ? VK_FORMAT_R8_UNORM : VK_FORMAT_R8G8B8A8_UNORM);
    view_info.subresourceRange.aspectMask = aspect_flags;
    view_info.subresourceRange.baseMipLevel = 0;
    view_info.subresourceRange.levelCount = 1;
    view_info.subresourceRange.baseArrayLayer = 0;
    view_info.subresourceRange.layerCount = 1;
    if (m_single_channel)
    {
        view_info.components.r = VK_COMPONENT_SWIZZLE_ONE;
        view_info.components.g = VK_COMPONENT_SWIZZLE_ONE;
        view_info.components.b = VK_COMPONENT_SWIZZLE_ONE;
        view_info.components.a = VK_COMPONENT_SWIZZLE_R;
    }

    VkResult result = vkCreateImageView(m_vulkan_device, &view_info, NULL,
        &m_image_view);
    return (result == VK_SUCCESS);
}   // createImageView

// ----------------------------------------------------------------------------
void GEVulkanTexture::clearVulkanData()
{
    if (m_image_view != VK_NULL_HANDLE)
        vkDestroyImageView(m_vulkan_device, m_image_view, NULL);
    if (m_image != VK_NULL_HANDLE)
        vkDestroyImage(m_vulkan_device, m_image, NULL);
    if (m_image_memory != VK_NULL_HANDLE)
        vkFreeMemory(m_vulkan_device, m_image_memory, NULL);
}   // clearVulkanData

// ----------------------------------------------------------------------------
void GEVulkanTexture::reload()
{
    if (m_disable_reload)
        return;

    clearVulkanData();
    video::IImage* texture_image = getResizedImage(NamedPath.getPtr(),
        &m_orig_size);
    if (texture_image == NULL)
        return;
    m_size = texture_image->getDimension();
    if (m_image_mani)
        m_image_mani(texture_image);

    uint8_t* data = (uint8_t*)texture_image->lock();
    upload(data);
    texture_image->unlock();
    texture_image->drop();
}   // reload

// ----------------------------------------------------------------------------
void GEVulkanTexture::upload(uint8_t* data)
{
    if (!m_single_channel)
    {
        for (unsigned int i = 0; i < m_size.Width * m_size.Height; i++)
        {
            uint8_t tmp_val = data[i * 4];
            data[i * 4] = data[i * 4 + 2];
            data[i * 4 + 2] = tmp_val;
        }
    }
    if (!createTextureImage(data))
        return;
    if (!createImageView(VK_IMAGE_ASPECT_COLOR_BIT))
        return;
    m_texture_size = m_size.Width * m_size.Height * (m_single_channel ? 1 : 4);
}   // upload

// ----------------------------------------------------------------------------
void* GEVulkanTexture::lock(video::E_TEXTURE_LOCK_MODE mode, u32 mipmap_level)
{
    return NULL;
}   // lock

//-----------------------------------------------------------------------------
void GEVulkanTexture::updateTexture(void* data, video::ECOLOR_FORMAT format,
                                    u32 w, u32 h, u32 x, u32 y)
{
    VkBuffer staging_buffer;
    VkDeviceMemory staging_buffer_memory;
    if (m_single_channel)
    {
        if (format == video::ECF_R8)
        {
            unsigned image_size = w * h;
            if (!getVKDriver()->createBuffer(image_size,
                VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, staging_buffer,
                staging_buffer_memory))
                return;

            void* mapped_data;
            vkMapMemory(m_vulkan_device, staging_buffer_memory, 0, image_size,
                0, &mapped_data);
            memcpy(mapped_data, data, (size_t)(image_size));
            vkUnmapMemory(m_vulkan_device, staging_buffer_memory);
        }
    }
    else
    {
        if (format == video::ECF_R8)
        {
            unsigned image_size = w * h * 4;
            if (!getVKDriver()->createBuffer(w * h * 4,
                VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, staging_buffer,
                staging_buffer_memory))
                return;

            const unsigned int size = w * h;
            std::vector<uint8_t> image_data(size * 4, 255);
            uint8_t* orig_data = (uint8_t*)data;
            for (unsigned int i = 0; i < size; i++)
                image_data[4 * i + 3] = orig_data[i];
            void* mapped_data;
            vkMapMemory(m_vulkan_device, staging_buffer_memory, 0, image_size,
                0, &mapped_data);
            memcpy(mapped_data, image_data.data(), (size_t)(image_size));
            vkUnmapMemory(m_vulkan_device, staging_buffer_memory);
        }
        else if (format == video::ECF_A8R8G8B8)
        {
            unsigned image_size = w * h * 4;
            if (!getVKDriver()->createBuffer(w * h * 4,
                VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, staging_buffer,
                staging_buffer_memory))
                return;

            uint8_t* u8_data = (uint8_t*)data;
            for (unsigned int i = 0; i < w * h; i++)
            {
                uint8_t tmp_val = u8_data[i * 4];
                u8_data[i * 4] = u8_data[i * 4 + 2];
                u8_data[i * 4 + 2] = tmp_val;
            }
            void* mapped_data;
            vkMapMemory(m_vulkan_device, staging_buffer_memory, 0, image_size,
                0, &mapped_data);
            memcpy(mapped_data, u8_data, (size_t)(image_size));
            vkUnmapMemory(m_vulkan_device, staging_buffer_memory);
        }
    }
    copyBufferToImage(staging_buffer, w, h, x, y);
    vkDestroyBuffer(m_vulkan_device, staging_buffer, NULL);
    vkFreeMemory(m_vulkan_device, staging_buffer_memory, NULL);
}   // updateTexture

}
