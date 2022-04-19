#include "ge_vulkan_features.hpp"

#include "ge_vulkan_driver.hpp"
#include "ge_vulkan_shader_manager.hpp"

#include <set>
#include <string>
#include <vector>

#include "../source/Irrlicht/os.h"

namespace GE
{
namespace GEVulkanFeatures
{
// ============================================================================
bool g_supports_bind_textures_at_once = false;
bool g_supports_rgba8_blit = false;
bool g_supports_r8_blit = false;
// https://chunkstories.xyz/blog/a-note-on-descriptor-indexing
bool g_supports_descriptor_indexing = false;
bool g_supports_non_uniform_indexing = false;
bool g_supports_partially_bound = false;
}   // GEVulkanFeatures

// ============================================================================
void GEVulkanFeatures::init(GEVulkanDriver* vk)
{
    g_supports_bind_textures_at_once = true;
    bool dynamic_indexing = true;
    VkPhysicalDeviceLimits limit = vk->getPhysicalDeviceProperties().limits;
    // https://vulkan.gpuinfo.org/displaydevicelimit.php?name=maxDescriptorSetSamplers&platform=all
    // https://vulkan.gpuinfo.org/displaydevicelimit.php?name=maxDescriptorSetSampledImages&platform=all
    // https://vulkan.gpuinfo.org/displaydevicelimit.php?name=maxPerStageDescriptorSamplers&platform=all
    // https://vulkan.gpuinfo.org/displaydevicelimit.php?name=maxPerStageDescriptorSampledImages&platform=all
    // We decide 256 (GEVulkanShaderManager::getSamplerSize()) based on those infos
    const unsigned max_sampler_size = GEVulkanShaderManager::getSamplerSize();
    if (limit.maxDescriptorSetSamplers < max_sampler_size)
        g_supports_bind_textures_at_once = false;
    if (limit.maxDescriptorSetSampledImages < max_sampler_size)
        g_supports_bind_textures_at_once = false;
    if (limit.maxPerStageDescriptorSamplers < max_sampler_size)
        g_supports_bind_textures_at_once = false;
    if (limit.maxPerStageDescriptorSampledImages < max_sampler_size)
        g_supports_bind_textures_at_once = false;
    if (vk->getPhysicalDeviceFeatures().shaderSampledImageArrayDynamicIndexing == VK_FALSE)
    {
        dynamic_indexing = false;
        g_supports_bind_textures_at_once = false;
    }

    VkFormatProperties format_properties = {};
    vkGetPhysicalDeviceFormatProperties(vk->getPhysicalDevice(),
        VK_FORMAT_R8G8B8A8_UNORM, &format_properties);
    g_supports_rgba8_blit = format_properties.optimalTilingFeatures & VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT;
    format_properties = {};
    vkGetPhysicalDeviceFormatProperties(vk->getPhysicalDevice(),
        VK_FORMAT_R8_UNORM, &format_properties);
    g_supports_r8_blit = format_properties.optimalTilingFeatures & VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT;

    uint32_t extension_count;
    vkEnumerateDeviceExtensionProperties(vk->getPhysicalDevice(), NULL,
        &extension_count, NULL);
    std::vector<VkExtensionProperties> extensions(extension_count);
    vkEnumerateDeviceExtensionProperties(vk->getPhysicalDevice(), NULL,
        &extension_count, &extensions[0]);

    for (VkExtensionProperties& prop : extensions)
    {
        if (std::string(prop.extensionName) == "VK_EXT_descriptor_indexing")
            g_supports_descriptor_indexing = true;
    }

    VkPhysicalDeviceFeatures2 supported_features = {};
    supported_features.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2;
    VkPhysicalDeviceDescriptorIndexingFeatures descriptor_indexing_features = {};
    descriptor_indexing_features.sType =
        VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DESCRIPTOR_INDEXING_FEATURES;
    supported_features.pNext = &descriptor_indexing_features;

    if (vk->getPhysicalDeviceProperties().apiVersion < VK_API_VERSION_1_1 ||
        !vkGetPhysicalDeviceFeatures2)
        return;
    vkGetPhysicalDeviceFeatures2(vk->getPhysicalDevice(), &supported_features);
    if (supported_features.sType !=
        VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2)
        return;

    g_supports_non_uniform_indexing = (descriptor_indexing_features
        .shaderSampledImageArrayNonUniformIndexing == VK_TRUE);
    g_supports_partially_bound = (descriptor_indexing_features
        .descriptorBindingPartiallyBound == VK_TRUE);

    bool missing_vkGetPhysicalDeviceProperties2 =
        !vkGetPhysicalDeviceProperties2;
    if (!missing_vkGetPhysicalDeviceProperties2 &&
        !g_supports_bind_textures_at_once && dynamic_indexing)
    {
        // Required for moltenvk argument buffers
        VkPhysicalDeviceProperties2 props1 = {};
        props1.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2;
        VkPhysicalDeviceDescriptorIndexingProperties props2 = {};
        props2.sType =
            VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DESCRIPTOR_INDEXING_PROPERTIES;
        props1.pNext = &props2;
        vkGetPhysicalDeviceProperties2(vk->getPhysicalDevice(), &props1);
        if (props2.sType ==
            VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DESCRIPTOR_INDEXING_PROPERTIES)
        {
            g_supports_bind_textures_at_once =
                props2.maxPerStageDescriptorUpdateAfterBindSamplers > max_sampler_size &&
                props2.maxPerStageDescriptorUpdateAfterBindSampledImages > max_sampler_size &&
                props2.maxDescriptorSetUpdateAfterBindSamplers > max_sampler_size &&
                props2.maxDescriptorSetUpdateAfterBindSampledImages > max_sampler_size;
        }
    }
}   // init

// ----------------------------------------------------------------------------
void GEVulkanFeatures::printStats()
{
    os::Printer::log(
        "Vulkan can bind textures at once in shader",
        g_supports_bind_textures_at_once ? "true" : "false");
    os::Printer::log(
        "Vulkan supports linear blitting for rgba8",
        g_supports_rgba8_blit ? "true" : "false");
    os::Printer::log(
        "Vulkan supports linear blitting for r8",
        g_supports_r8_blit ? "true" : "false");
    os::Printer::log(
        "Vulkan supports VK_EXT_descriptor_indexing",
        g_supports_descriptor_indexing ? "true" : "false");
    os::Printer::log(
        "Vulkan descriptor indexes can be dynamically non-uniform",
        g_supports_non_uniform_indexing ? "true" : "false");
    os::Printer::log(
        "Vulkan descriptor can be partially bound",
        g_supports_partially_bound ? "true" : "false");
}   // printStats

// ----------------------------------------------------------------------------
bool GEVulkanFeatures::supportsBindTexturesAtOnce()
{
    return g_supports_bind_textures_at_once;
}   // supportsBindTexturesAtOnce

// ----------------------------------------------------------------------------
bool GEVulkanFeatures::supportsRGBA8Blit()
{
    return g_supports_rgba8_blit;
}   // supportsRGBA8Blit

// ----------------------------------------------------------------------------
bool GEVulkanFeatures::supportsR8Blit()
{
    return g_supports_r8_blit;
}   // supportsR8Blit

// ----------------------------------------------------------------------------
bool GEVulkanFeatures::supportsDescriptorIndexing()
{
    return g_supports_descriptor_indexing;
}   // supportsDescriptorIndexing

// ----------------------------------------------------------------------------
bool GEVulkanFeatures::supportsNonUniformIndexing()
{
    return g_supports_non_uniform_indexing;
}   // supportsNonUniformIndexing

// ----------------------------------------------------------------------------
bool GEVulkanFeatures::supportsDifferentTexturePerDraw()
{
    return g_supports_bind_textures_at_once &&
        g_supports_descriptor_indexing && g_supports_non_uniform_indexing;
}   // supportsDifferentTexturePerDraw

// ----------------------------------------------------------------------------
bool GEVulkanFeatures::supportsPartiallyBound()
{
    return g_supports_partially_bound;
}   // supportsPartiallyBound

}
