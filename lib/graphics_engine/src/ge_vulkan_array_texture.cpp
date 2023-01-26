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
    m_max_size = m_vk->getDriverAttributes()
        .getAttributeAsDimension2d("MAX_TEXTURE_SIZE");
    m_internal_format = VK_FORMAT_R8G8B8A8_UNORM;

    m_size_lock.lock();
    m_image_view_lock.lock();
    m_thread_loading_lock.lock();
    GEVulkanCommandLoader::addMultiThreadingCommand(
        [list, image_mani, this]()
        {
            reloadInternal(list, image_mani);
        });
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
void GEVulkanArrayTexture::reloadInternal(const std::vector<io::path>& list,
                                          std::function<void(video::IImage*,
                                          unsigned)> image_mani)
{
    VkDeviceSize image_size = 0;
    VkDeviceSize mipmap_data_size = 0;
    VkDeviceSize image_total_size = 0;
    std::vector<video::IImage*> images;
    std::vector<GEMipmapGenerator*> mipmaps;

    VkBuffer staging_buffer = VK_NULL_HANDLE;
    VmaAllocation staging_buffer_allocation = NULL;
    VmaAllocationCreateInfo staging_buffer_create_info = {};
    staging_buffer_create_info.usage = VMA_MEMORY_USAGE_AUTO_PREFER_HOST;
    staging_buffer_create_info.flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT;
    staging_buffer_create_info.preferredFlags = VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;

    assert(m_layer_count == list.size());
    uint8_t* mapped;
    VkCommandBuffer command_buffer = VK_NULL_HANDLE;
    unsigned offset = 0;
    unsigned width = 0;
    unsigned height = 0;

    for (unsigned i = 0; i < list.size(); i++)
    {
        const io::path& fullpath = list[i];
        video::IImageLoader* loader = NULL;
        io::IReadFile* file = io::createReadFile(fullpath);
        if (!file)
        {
            printf("Missing file %s in GEVulkanArrayTexture::reloadInternal",
                fullpath.c_str());
            goto destroy;
        }
        m_vk->createImageFromFile(file, &loader);
        core::dimension2du dim;
        if (!loader || !loader->getImageSize(file, &dim))
        {
            file->drop();
            printf("Missing image loader for %s in "
                "GEVulkanArrayTexture::reloadInternal", fullpath.c_str());
            goto destroy;
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

    m_size = core::dimension2du(width, height);
    image_size = m_size.Width * m_size.Height * 4;
    m_orig_size = m_size;
    m_size_lock.unlock();

    for (unsigned i = 0; i < list.size(); i++)
    {
        const io::path& fullpath = list[i];
        video::IImage* texture_image = getResizedImageFullPath(fullpath,
            m_max_size, NULL, &m_size);
        if (texture_image == NULL)
            goto destroy;
        if (image_mani)
            image_mani(texture_image, i);
        uint8_t* texture_data = (uint8_t*)texture_image->lock();
        bgraConversion(texture_data);
        GEMipmapGenerator* mipmap_generator = NULL;
        const bool normal_map = (std::string(fullpath.c_str()).find(
            "_Normal.") != std::string::npos);
        bool texture_compression = getGEConfig()->m_texture_compression;
        if (texture_compression && GEVulkanFeatures::supportsASTC4x4())
        {
            if (i == 0)
            {
                image_size = get4x4CompressedTextureSize(m_size.Width,
                    m_size.Height);
                m_internal_format = VK_FORMAT_ASTC_4x4_UNORM_BLOCK;
            }
            mipmap_generator = new GECompressorASTC4x4(texture_data, 4, m_size,
                normal_map);
        }
        else if (texture_compression && GEVulkanFeatures::supportsBPTCBC7())
        {
            if (i == 0)
            {
                image_size = get4x4CompressedTextureSize(m_size.Width,
                    m_size.Height);
                m_internal_format = VK_FORMAT_BC7_UNORM_BLOCK;
            }
            mipmap_generator = new GECompressorBPTCBC7(texture_data, 4, m_size,
                normal_map);
        }
        else if (texture_compression && GEVulkanFeatures::supportsS3TCBC3())
        {
            if (i == 0)
            {
                image_size = get4x4CompressedTextureSize(m_size.Width,
                    m_size.Height);
                m_internal_format = VK_FORMAT_BC3_UNORM_BLOCK;
            }
            mipmap_generator = new GECompressorS3TCBC3(texture_data, 4, m_size,
                normal_map);
        }
        else
        {
            if (i == 0)
                m_internal_format = VK_FORMAT_R8G8B8A8_UNORM;
            mipmap_generator = new GEMipmapGenerator(texture_data, 4, m_size,
                normal_map);
        }
        mipmap_data_size = mipmap_generator->getMipmapSizes();
        if (mipmaps.empty())
        {
            image_total_size = image_size + mipmap_data_size;
            image_total_size *= m_layer_count;
            if (!m_vk->createBuffer(image_total_size,
                VK_BUFFER_USAGE_TRANSFER_SRC_BIT, staging_buffer_create_info,
                staging_buffer, staging_buffer_allocation))
            {
                goto destroy;
            }
            if (vmaMapMemory(m_vk->getVmaAllocator(),
                staging_buffer_allocation, (void**)&mapped) != VK_SUCCESS)
            {
                goto destroy;
            }
        }
        mipmaps.push_back(mipmap_generator);
        for (GEImageLevel& level : mipmap_generator->getAllLevels())
        {
            memcpy(mapped, level.m_data, level.m_size);
            mapped += level.m_size;
        }
        images.push_back(texture_image);
    }
    vmaUnmapMemory(m_vk->getVmaAllocator(), staging_buffer_allocation);
    vmaFlushAllocation(m_vk->getVmaAllocator(),
        staging_buffer_allocation, 0, image_total_size);

    if (!createImage(VK_IMAGE_USAGE_TRANSFER_SRC_BIT |
        VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT))
        goto destroy;

    command_buffer = GEVulkanCommandLoader::beginSingleTimeCommands();

    transitionImageLayout(command_buffer, VK_IMAGE_LAYOUT_UNDEFINED,
        VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

    for (unsigned i = 0; i < list.size(); i++)
    {
        std::vector<GEImageLevel>& levels =  mipmaps[i]->getAllLevels();
        for (unsigned j = 0; j < levels.size(); j++)
        {
            GEImageLevel& level = levels[j];
            copyBufferToImage(command_buffer, staging_buffer,
                level.m_dim.Width, level.m_dim.Height, 0, 0, offset, j, i);
            offset += level.m_size;
        }
    }
    transitionImageLayout(command_buffer, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
        VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

    GEVulkanCommandLoader::endSingleTimeCommands(command_buffer);

    createImageView(VK_IMAGE_ASPECT_COLOR_BIT);

destroy:
    m_image_view_lock.unlock();
    for (video::IImage* image : images)
        image->drop();
    for (GEMipmapGenerator* mipmap_generator : mipmaps)
        delete mipmap_generator;
    if (staging_buffer != VK_NULL_HANDLE)
    {
        vmaDestroyBuffer(m_vk->getVmaAllocator(), staging_buffer,
            staging_buffer_allocation);
    }
    m_thread_loading_lock.unlock();

}   // reloadInternal

}
