#include <stdexcept>

#include "ge_vulkan_attachment_texture.hpp"

#include "ge_main.hpp"
#include "ge_vulkan_driver.hpp"

#include <stdexcept>

namespace GE
{
GEVulkanAttachmentTexture::GEVulkanAttachmentTexture(GEVulkanDriver* vk,
                                           const core::dimension2d<u32>& size,
                                           VkFormat format,
                                           VkImageUsageFlags iu,
                                           VkImageAspectFlags ia)
                         : GEVulkanTexture()
{
    m_vk = vk;
    m_vulkan_device = m_vk->getDevice();
    m_image = VK_NULL_HANDLE;
    m_vma_allocation = VK_NULL_HANDLE;
    m_has_mipmaps = false;
    m_locked_data = NULL;
    m_size = m_orig_size = size;
    m_internal_format = format;

    if (!createImage(iu))
        throw std::runtime_error("createImage failed for attachment texture");

    if (!createImageView(ia))
        throw std::runtime_error("createImageView failed for attachment texture");
}   // GEVulkanAttachmentTexture

// ----------------------------------------------------------------------------
GEVulkanAttachmentTexture* GEVulkanAttachmentTexture::createDepthTexture(
                        GEVulkanDriver* vk, const core::dimension2d<u32>& size,
                        bool lazy_allocation)
{
    std::vector<VkFormat> preferred =
    {
        VK_FORMAT_D32_SFLOAT,
        VK_FORMAT_D24_UNORM_S8_UINT,
        VK_FORMAT_D16_UNORM
    };
    VkFormat format = vk->findSupportedFormat(preferred,
        VK_IMAGE_TILING_OPTIMAL,
        VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT);
    VkImageUsageFlags iu = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT |
        VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT;
    if (lazy_allocation)
        iu |= VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT;
    else
        iu |= VK_IMAGE_USAGE_SAMPLED_BIT;
    return new GEVulkanAttachmentTexture(vk, size, format, iu,
        VK_IMAGE_ASPECT_DEPTH_BIT);
}   // createDepthTexture

}
