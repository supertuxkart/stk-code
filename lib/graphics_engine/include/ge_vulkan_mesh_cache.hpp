#ifndef HEADER_GE_VULKAN_MESH_CACHE_HPP
#define HEADER_GE_VULKAN_MESH_CACHE_HPP

#include "vulkan_wrapper.h"

namespace GE
{
class GEVulkanDriver;
namespace GEVulkanMeshCache
{
// ----------------------------------------------------------------------------
void init(GEVulkanDriver*);
// ----------------------------------------------------------------------------
void irrlichtMeshChanged();
// ----------------------------------------------------------------------------
void updateCache();
// ----------------------------------------------------------------------------
void destroy();
// ----------------------------------------------------------------------------
VkBuffer getVBO();
// ----------------------------------------------------------------------------
VkBuffer getIBO();
};   // GEVulkanMeshCache

}

#endif
