#ifndef HEADER_VULKAN_WRAPPER_HPP
#define HEADER_VULKAN_WRAPPER_HPP

#if !defined(__APPLE__) || defined(DLOPEN_MOLTENVK)
#include <glad/vulkan.h>

#ifdef DLOPEN_MOLTENVK
#define VK_NO_PROTOTYPES 1
#include <vk_mvk_moltenvk.h>
extern PFN_vkGetMoltenVKConfigurationMVK vkGetMoltenVKConfigurationMVK;
extern PFN_vkSetMoltenVKConfigurationMVK vkSetMoltenVKConfigurationMVK;
extern PFN_vkGetPhysicalDeviceMetalFeaturesMVK vkGetPhysicalDeviceMetalFeaturesMVK;
#endif

#else
#include <vulkan/vulkan.h>

#if defined(__APPLE__)
#include <MoltenVK/vk_mvk_moltenvk.h>
#endif

#endif

#endif
