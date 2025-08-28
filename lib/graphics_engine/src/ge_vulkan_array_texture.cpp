#include "ge_vulkan_array_texture.hpp"

#include "ge_main.hpp"
#include "ge_mipmap_generator.hpp"
#include "ge_compressor_astc_4x4.hpp"
#include "ge_compressor_bptc_bc7.hpp"
#include "ge_compressor_s3tc_bc3.hpp"
#include "ge_texture.hpp"
#include "ge_vulkan_command_loader.hpp"
#include "ge_vulkan_driver.hpp"
#include "ge_vulkan_features.hpp"

#include <IImageLoader.h>
#include <cassert>
#include <stdexcept>

namespace GE
{
// ============================================================================
GEVulkanArrayTexture::ThreadLoader::ThreadLoader(GEVulkanArrayTexture* texture,
                                             const std::vector<io::path>& list,
                      std::function<void(video::IImage*, unsigned)> image_mani,
                                                        video::SColor unicolor,
                                                    VkImageLayout first_layout)
                    : m_texture(texture), m_list(list),
                      m_image_mani(image_mani), m_unicolor(unicolor),
                      m_first_layout(first_layout)
{
    m_images.resize(m_texture->m_layer_count);
    m_mipmaps.resize(m_texture->m_layer_count);
    m_texture->m_image_view_lock.lock();
    m_texture->m_thread_loading_lock.lock();
}   // GEVulkanArrayTexture::ThreadLoader

// ----------------------------------------------------------------------------
GEVulkanArrayTexture::ThreadLoader::~ThreadLoader()
{
    bool storage_image = m_list[0].empty();
    VkDeviceSize image_size = 0;
    switch (m_texture->m_internal_format)
    {
    case VK_FORMAT_ASTC_4x4_UNORM_BLOCK:
    case VK_FORMAT_BC7_UNORM_BLOCK:
    case VK_FORMAT_BC3_UNORM_BLOCK:
        image_size = get4x4CompressedTextureSize(m_texture->m_size.Width,
            m_texture->m_size.Height);
        break;
    default:
        image_size = m_texture->m_size.Width * m_texture->m_size.Height * 4;
        break;
    }
    VkDeviceSize mipmap_data_size = m_mipmaps[0]->getMipmapSizes();
    VkDeviceSize image_total_size = image_size + mipmap_data_size;
    image_total_size *= m_mipmaps.size();

    VkBuffer staging_buffer = VK_NULL_HANDLE;
    VmaAllocation staging_buffer_allocation = NULL;
    VmaAllocationCreateInfo staging_buffer_create_info = {};
    staging_buffer_create_info.usage = VMA_MEMORY_USAGE_AUTO_PREFER_HOST;
    staging_buffer_create_info.flags =
        VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT;
    staging_buffer_create_info.preferredFlags =
        VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
    uint8_t* mapped;
    unsigned offset = 0;
    VkCommandBuffer command_buffer = VK_NULL_HANDLE;
    GEVulkanDriver* vk = m_texture->m_vk;
    VkImageUsageFlags usage_flags = VK_IMAGE_USAGE_TRANSFER_SRC_BIT |
        VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
    if (storage_image)
        usage_flags |= VK_IMAGE_USAGE_STORAGE_BIT;

    if (!vk->createBuffer(image_total_size,
        VK_BUFFER_USAGE_TRANSFER_SRC_BIT, staging_buffer_create_info,
        staging_buffer, staging_buffer_allocation))
    {
        goto destroy;
    }
    if (vmaMapMemory(vk->getVmaAllocator(),
        staging_buffer_allocation, (void**)&mapped) != VK_SUCCESS)
    {
        goto destroy;
    }

    for (unsigned i = 0; i < m_mipmaps.size(); i++)
    {
        for (GEImageLevel& level : m_mipmaps[i]->getAllLevels())
        {
            memcpy(mapped, level.m_data, level.m_size);
            mapped += level.m_size;
        }
    }
    vmaUnmapMemory(vk->getVmaAllocator(), staging_buffer_allocation);
    vmaFlushAllocation(vk->getVmaAllocator(), staging_buffer_allocation, 0,
        image_total_size);

    if (!m_texture->createImage(usage_flags))
        goto destroy;

    command_buffer = GEVulkanCommandLoader::beginSingleTimeCommands();

    m_texture->transitionImageLayout(command_buffer, VK_IMAGE_LAYOUT_UNDEFINED,
        VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

    for (unsigned i = 0; i < m_mipmaps.size(); i++)
    {
        std::vector<GEImageLevel>& levels = m_mipmaps[i]->getAllLevels();
        for (unsigned j = 0; j < levels.size(); j++)
        {
            GEImageLevel& level = levels[j];
            m_texture->copyBufferToImage(command_buffer, staging_buffer,
                level.m_dim.Width, level.m_dim.Height, 0, 0, offset, j, i);
            offset += level.m_size;
        }
    }
    if (m_first_layout == VK_IMAGE_LAYOUT_UNDEFINED)
    {
        if (storage_image)
            m_first_layout = VK_IMAGE_LAYOUT_GENERAL;
        else
            m_first_layout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    }
    m_texture->transitionImageLayout(command_buffer,
        VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, m_first_layout);

    GEVulkanCommandLoader::endSingleTimeCommands(command_buffer);

    m_texture->createImageView(VK_IMAGE_ASPECT_COLOR_BIT, !storage_image);

destroy:
    m_texture->m_image_view_lock.unlock();
    for (video::IImage* image : m_images)
        image->drop();
    for (GEMipmapGenerator* mipmap_generator : m_mipmaps)
        delete mipmap_generator;
    if (staging_buffer != VK_NULL_HANDLE)
    {
        vmaDestroyBuffer(vk->getVmaAllocator(), staging_buffer,
            staging_buffer_allocation);
    }
    m_texture->m_thread_loading_lock.unlock();
}   // ~GEVulkanArrayTexture::ThreadLoader

// ----------------------------------------------------------------------------
void GEVulkanArrayTexture::ThreadLoader::load(unsigned layer)
{
    const io::path& fullpath = m_list[layer];
    core::dimension2du size = m_texture->m_size;
    video::IImage* texture_image;
    if (fullpath.empty())
    {
        std::vector<video::SColor> data(size.Width * size.Height, m_unicolor);
        texture_image = m_texture->m_vk->createImageFromData(
            video::ECF_A8R8G8B8, size, data.data(), false/*ownForeignMemory*/);
    }
    else
       texture_image = getResizedImageFullPath(fullpath, size, NULL, &size);
    if (texture_image == NULL)
    {
        throw std::runtime_error(
            "Missing texture_image in "
            "GEVulkanArrayTexture::ThreadLoader::load");
    }
    if (m_image_mani)
        m_image_mani(texture_image, layer);
    uint8_t* texture_data = (uint8_t*)texture_image->lock();
    m_texture->bgraConversion(texture_data);
    GEMipmapGenerator* mipmap_generator = NULL;
    const bool normal_map = (std::string(fullpath.c_str()).find(
        "_Normal.") != std::string::npos);
    bool texture_compression = getGEConfig()->m_texture_compression &&
        !fullpath.empty();
    if (texture_compression && GEVulkanFeatures::supportsASTC4x4())
    {
        if (layer == 0)
            m_texture->m_internal_format = VK_FORMAT_ASTC_4x4_UNORM_BLOCK;
        mipmap_generator = new GECompressorASTC4x4(texture_data, 4, size,
            normal_map);
    }
    else if (texture_compression && GEVulkanFeatures::supportsBPTCBC7())
    {
        if (layer == 0)
            m_texture->m_internal_format = VK_FORMAT_BC7_UNORM_BLOCK;
        mipmap_generator = new GECompressorBPTCBC7(texture_data, 4, size,
            normal_map);
    }
    else if (texture_compression && GEVulkanFeatures::supportsS3TCBC3())
    {
        if (layer == 0)
            m_texture->m_internal_format = VK_FORMAT_BC3_UNORM_BLOCK;
        mipmap_generator = new GECompressorS3TCBC3(texture_data, 4, size,
            normal_map);
    }
    else
    {
        if (layer == 0)
            m_texture->m_internal_format = VK_FORMAT_R8G8B8A8_UNORM;
        mipmap_generator = new GEMipmapGenerator(texture_data, 4, size,
            normal_map);
    }
    m_mipmaps[layer] = mipmap_generator;
    m_images[layer] = texture_image;
}   // GEVulkanArrayTexture::load

// ============================================================================
std::vector<io::path> getPathList(const std::vector<GEVulkanTexture*>& tlist)
{
    std::vector<io::path> list;
    for (GEVulkanTexture* tex : tlist)
        list.push_back(tex->getFullPath());
    return list;
}   // getPathList

// ============================================================================
GEVulkanArrayTexture::GEVulkanArrayTexture(const std::vector<io::path>& list,
                                           VkImageViewType type,
                                           std::function<void(video::IImage*,
                                           unsigned)> image_mani)
                    : GEVulkanTexture()
{
    if (list.empty())
        throw std::runtime_error("empty texture list for array texture");

    m_layer_count = list.size();
    m_image_view_type = type;
    m_vk = getVKDriver();
    m_vulkan_device = m_vk->getDevice();
    m_image = VK_NULL_HANDLE;
    m_vma_allocation = VK_NULL_HANDLE;
    m_has_mipmaps = true;
    m_locked_data = NULL;
    m_internal_format = VK_FORMAT_R8G8B8A8_UNORM;

    unsigned width = 0;
    unsigned height = 0;
    for (unsigned i = 0; i < list.size(); i++)
    {
        const io::path& fullpath = list[i];
        video::IImageLoader* loader = NULL;
        io::IReadFile* file = io::createReadFile(fullpath);
        if (!file)
        {
            printf("Missing file %s in GEVulkanArrayTexture, layer %d",
                fullpath.c_str(), i);
            return;
        }
        m_vk->createImageFromFile(file, &loader);
        core::dimension2du dim;
        if (!loader || !loader->getImageSize(file, &dim))
        {
            file->drop();
            printf("Missing image loader for %s in "
                "GEVulkanArrayTexture, layer %d", fullpath.c_str(), i);
            return;
        }
        file->drop();
        if (m_image_view_type == VK_IMAGE_VIEW_TYPE_CUBE ||
            m_image_view_type == VK_IMAGE_VIEW_TYPE_CUBE_ARRAY)
        {
            width = std::max({ width, dim.Width, dim.Height });
            height = width;
        }
        else
        {
            width = std::max(width, dim.Width);
            height = std::max(height, dim.Height);
        }
    }
    const core::dimension2du& max_size = m_vk->getDriverAttributes()
        .getAttributeAsDimension2d("MAX_TEXTURE_SIZE");
    width = std::min(width, max_size.Width);
    height = std::min(height, max_size.Height);
    m_size = core::dimension2du(width, height);
    m_orig_size = m_size;

    std::shared_ptr<ThreadLoader> tl = std::make_shared<ThreadLoader>(this,
        list, image_mani);
    for (unsigned i = 0; i < list.size(); i++)
    {
        GEVulkanCommandLoader::addMultiThreadingCommand(
            [tl, i](){ tl->load(i); });
    }
}   // GEVulkanArrayTexture

// ----------------------------------------------------------------------------
GEVulkanArrayTexture::GEVulkanArrayTexture(
                                 const std::vector<GEVulkanTexture*>& textures,
                                 VkImageViewType type,
                                 std::function<void(video::IImage*, unsigned)>
                                 image_mani)
                    : GEVulkanArrayTexture(getPathList(textures), type,
                      image_mani)
{
}   // GEVulkanArrayTexture

// ----------------------------------------------------------------------------
GEVulkanArrayTexture::GEVulkanArrayTexture(VkFormat internal_format,
                                           VkImageViewType type,
                                           const core::dimension2du& size,
                                           unsigned layer_count,
                                           video::SColor unicolor,
                                           VkImageLayout first_layout)
{
    m_layer_count = layer_count;
    m_image_view_type = type;
    m_vk = getVKDriver();
    m_vulkan_device = m_vk->getDevice();
    m_image = VK_NULL_HANDLE;
    m_vma_allocation = VK_NULL_HANDLE;
    m_has_mipmaps = true;
    m_locked_data = NULL;
    m_internal_format = internal_format;
    if (size.Width < 4 || size.Height < 4)
        throw std::runtime_error("Minimum width and height of 4 is required.");
    m_orig_size = m_size = size;

    if (m_internal_format == VK_FORMAT_R8G8B8A8_UNORM)
    {
        std::vector<io::path> list;
        for (unsigned i = 0; i < m_layer_count; i++)
            list.push_back("");
        std::shared_ptr<ThreadLoader> tl = std::make_shared<ThreadLoader>(this,
            list, nullptr, unicolor, first_layout);
        for (unsigned i = 0; i < list.size(); i++)
        {
            GEVulkanCommandLoader::addMultiThreadingCommand(
                [tl, i](){ tl->load(i); });
        }
    }
    else
    {
        if (!createImage(VK_IMAGE_USAGE_STORAGE_BIT |
            VK_IMAGE_USAGE_SAMPLED_BIT))
        {
            throw std::runtime_error(
                "createImage failed in storage array texture creation");
        }
        VkCommandBuffer command_buffer =
            GEVulkanCommandLoader::beginSingleTimeCommands();
        if (first_layout != VK_IMAGE_LAYOUT_UNDEFINED)
        {
            transitionImageLayout(command_buffer, VK_IMAGE_LAYOUT_UNDEFINED,
                first_layout);
        }
        else
        {
            transitionImageLayout(command_buffer, VK_IMAGE_LAYOUT_UNDEFINED,
                VK_IMAGE_LAYOUT_GENERAL);
        }
        GEVulkanCommandLoader::endSingleTimeCommands(command_buffer);
        createImageView(VK_IMAGE_ASPECT_COLOR_BIT);
    }
}   // GEVulkanArrayTexture

}
