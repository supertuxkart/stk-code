#ifndef HEADER_GE_VULKAN_MESH_CACHE_HPP
#define HEADER_GE_VULKAN_MESH_CACHE_HPP

#include "vulkan_wrapper.h"
#include "ge_vma.hpp"
#include "../source/Irrlicht/CMeshCache.h"

#include <cstdint>

namespace GE
{
class GEVulkanDriver;
class GEVulkanMeshCache : public irr::scene::CMeshCache
{
private:
    GEVulkanDriver* m_vk;

    uint64_t m_irrlicht_cache_time, m_ge_cache_time;

    VkBuffer m_vbo_buffer, m_ibo_buffer;

    VmaAllocation m_vbo_memory, m_ibo_memory;
public:
    // ------------------------------------------------------------------------
    GEVulkanMeshCache();
    // ------------------------------------------------------------------------
    virtual void meshCacheChanged();
    // ------------------------------------------------------------------------
    void updateCache();
    // ------------------------------------------------------------------------
    void destroy();
    // ------------------------------------------------------------------------
    VkBuffer getVBO() const                            { return m_vbo_buffer; }
    // ------------------------------------------------------------------------
    VkBuffer getIBO() const                            { return m_ibo_buffer; }
};   // GEVulkanMeshCache

}

#endif
