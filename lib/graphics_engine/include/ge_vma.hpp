#ifndef HEADER_GE_VMA_HPP
#define HEADER_GE_VMA_HPP
#include "vulkan_wrapper.h"

// Remove clang warnings
#define VMA_NULLABLE
#define VMA_NOT_NULL

#if !defined(__APPLE__) || defined(DLOPEN_MOLTENVK)
#define VMA_STATIC_VULKAN_FUNCTIONS 0
#define VMA_DYNAMIC_VULKAN_FUNCTIONS 1

#elif defined(IOS_STK)
// MoltenVK doesn't provide full 1.3 support, which will lead to linking errors
#define VMA_VULKAN_VERSION 1002000

#endif
#include "vk_mem_alloc.h"
#endif
