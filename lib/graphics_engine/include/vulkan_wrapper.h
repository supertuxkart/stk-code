#ifndef HEADER_VULKAN_WRAPPER_HPP
#define HEADER_VULKAN_WRAPPER_HPP

#if !defined(__APPLE__) || defined(DLOPEN_MOLTENVK)
#include <glad/vulkan.h>

#ifdef DLOPEN_MOLTENVK
#define VK_NO_PROTOTYPES 1
// We copy mvk_private_api.h with #include <vulkan/vulkan.h>
// removed
#include <mvk_private_api.h>
extern PFN_vkGetPhysicalDeviceMetalFeaturesMVK vkGetPhysicalDeviceMetalFeaturesMVK;
#endif

#else
#include <vulkan/vulkan.h>

#if defined(__APPLE__)
#include <MoltenVK/mvk_private_api.h>
#endif

#endif

#endif

#ifndef TILED_GPU
#if defined(__arm__) || defined(__aarch64__) || defined(_M_ARM) || defined (_M_ARM64)
#define TILED_GPU 1
#endif
#endif
