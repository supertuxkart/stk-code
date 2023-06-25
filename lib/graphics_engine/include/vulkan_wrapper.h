#ifndef HEADER_VULKAN_WRAPPER_HPP
#define HEADER_VULKAN_WRAPPER_HPP

#if !defined(__APPLE__) || defined(DLOPEN_MOLTENVK)
#include <glad/vulkan.h>

#define VK_API_VERSION_MAJOR(version) (((uint32_t)(version) >> 22) & 0x7FU)
#define VK_API_VERSION_MINOR(version) (((uint32_t)(version) >> 12) & 0x3FFU)
#define VK_API_VERSION_PATCH(version) ((uint32_t)(version) & 0xFFFU)

#ifdef DLOPEN_MOLTENVK
#define VK_NO_PROTOTYPES 1
// We copy mvk_config.h and mvk_private_api.h with #include <vulkan/vulkan.h>
// removed
#include <mvk_config.h>
#include <mvk_private_api.h>
extern PFN_vkGetMoltenVKConfigurationMVK vkGetMoltenVKConfigurationMVK;
extern PFN_vkSetMoltenVKConfigurationMVK vkSetMoltenVKConfigurationMVK;
extern PFN_vkGetPhysicalDeviceMetalFeaturesMVK vkGetPhysicalDeviceMetalFeaturesMVK;
#endif

#else
#include <vulkan/vulkan.h>

#if defined(__APPLE__)
#include <MoltenVK/mvk_config.h>
#include <MoltenVK/mvk_private_api.h>
#endif

#endif

#endif
