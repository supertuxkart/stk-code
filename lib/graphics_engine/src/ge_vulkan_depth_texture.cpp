#include <stdexcept>

#include "ge_vulkan_depth_texture.hpp"

#include "ge_main.hpp"
#include "ge_vulkan_driver.hpp"

#include <stdexcept>

namespace GE
{
GEVulkanDepthTexture::GEVulkanDepthTexture(GEVulkanDriver* vk,
                                           const core::dimension2d<u32>& size)
                    : GEVulkanTexture()
{
    m_vk = vk;
    m_vulkan_device = m_vk->getDevice();
    m_image = VK_NULL_HANDLE;
    m_vma_allocation = VK_NULL_HANDLE;
    m_has_mipmaps = false;
    m_locked_data = NULL;
    m_size = m_orig_size = m_max_size = size;

    std::vector<VkFormat> preferred =
    {
        VK_FORMAT_D32_SFLOAT,
        VK_FORMAT_D32_SFLOAT_S8_UINT,
        VK_FORMAT_D24_UNORM_S8_UINT
    };
    m_internal_format = m_vk->findSupportedFormat(preferred,
        VK_IMAGE_TILING_OPTIMAL,
        VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT);

    if (!createImage(VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT))
        throw std::runtime_error("createImage failed for depth texture");

    if (!createImageView(VK_IMAGE_ASPECT_DEPTH_BIT))
        throw std::runtime_error("createImageView failed for depth texture");
}   // GEVulkanDepthTexture

}
