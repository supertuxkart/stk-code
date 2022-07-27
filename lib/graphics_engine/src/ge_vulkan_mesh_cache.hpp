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

    VkBuffer m_buffer;

    VmaAllocation m_memory;

    size_t m_ibo_offset, m_skinning_vbo_offset;
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
    VkBuffer getBuffer() const                             { return m_buffer; }
    // ------------------------------------------------------------------------
    size_t getIBOOffset() const                        { return m_ibo_offset; }
    // ------------------------------------------------------------------------
    size_t getSkinningVBOOffset() const       { return m_skinning_vbo_offset; }
};   // GEVulkanMeshCache

}

#endif
