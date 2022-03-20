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
// https://chunkstories.xyz/blog/a-note-on-descriptor-indexing
bool g_supports_descriptor_indexing = false;
bool g_supports_non_uniform_indexing = false;
bool g_supports_partially_bound = false;
}   // GEVulkanFeatures

// ============================================================================
void GEVulkanFeatures::init(GEVulkanDriver* vk)
{
    g_supports_bind_textures_at_once = true;
    VkPhysicalDeviceLimits limit = vk->getPhysicalDeviceProperties().limits;
    // https://vulkan.gpuinfo.org/displaydevicelimit.php?name=maxDescriptorSetSamplers&platform=all
    // https://vulkan.gpuinfo.org/displaydevicelimit.php?name=maxDescriptorSetSampledImages&platform=all
    // https://vulkan.gpuinfo.org/displaydevicelimit.php?name=maxPerStageDescriptorSamplers&platform=all
    // https://vulkan.gpuinfo.org/displaydevicelimit.php?name=maxPerStageDescriptorSampledImages&platform=all
    // We decide 256 (GEVulkanShaderManager::getSamplerSize()) based on those infos
    if (limit.maxDescriptorSetSamplers < GEVulkanShaderManager::getSamplerSize())
        g_supports_bind_textures_at_once = false;
    if (limit.maxDescriptorSetSampledImages < GEVulkanShaderManager::getSamplerSize())
        g_supports_bind_textures_at_once = false;
    if (limit.maxPerStageDescriptorSamplers < GEVulkanShaderManager::getSamplerSize())
        g_supports_bind_textures_at_once = false;
    if (limit.maxPerStageDescriptorSampledImages < GEVulkanShaderManager::getSamplerSize())
        g_supports_bind_textures_at_once = false;
    if (vk->getPhysicalDeviceFeatures().shaderSampledImageArrayDynamicIndexing == VK_FALSE)
        g_supports_bind_textures_at_once = false;

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

    if (!vkGetPhysicalDeviceFeatures2)
        return;
    vkGetPhysicalDeviceFeatures2(vk->getPhysicalDevice(), &supported_features);
    if (supported_features.sType !=
        VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2)
        return;

    g_supports_non_uniform_indexing = (descriptor_indexing_features
        .shaderSampledImageArrayNonUniformIndexing == VK_TRUE);
    g_supports_partially_bound = (descriptor_indexing_features
        .descriptorBindingPartiallyBound == VK_TRUE);
}   // init

// ----------------------------------------------------------------------------
void GEVulkanFeatures::printStats()
{
    os::Printer::log(
        "Vulkan can bind textures at once in shader",
        g_supports_bind_textures_at_once ? "true" : "false");
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
