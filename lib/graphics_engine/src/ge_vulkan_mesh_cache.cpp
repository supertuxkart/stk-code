#include "ge_vulkan_mesh_cache.hpp"

#include "ge_main.hpp"
#include "ge_spm_buffer.hpp"
#include "ge_vulkan_driver.hpp"

#include "IAnimatedMesh.h"

#include <vector>

namespace GE
{
// ----------------------------------------------------------------------------
GEVulkanMeshCache::GEVulkanMeshCache()
                 : irr::scene::CMeshCache()
{
    m_vk = getVKDriver();
    m_irrlicht_cache_time = getMonoTimeMs();
    m_ge_cache_time = 0;
    m_vbo_buffer = VK_NULL_HANDLE;
    m_vbo_memory = VK_NULL_HANDLE;
    m_ibo_buffer = VK_NULL_HANDLE;
    m_ibo_memory = VK_NULL_HANDLE;
}   // init

// ----------------------------------------------------------------------------
void GEVulkanMeshCache::meshCacheChanged()
{
    m_irrlicht_cache_time = getMonoTimeMs();
}   // meshCacheChanged

// ----------------------------------------------------------------------------
void GEVulkanMeshCache::updateCache()
{
    if (m_irrlicht_cache_time <= m_ge_cache_time)
        return;
    m_ge_cache_time = m_irrlicht_cache_time;

    destroy();
    size_t vbo_size, ibo_size;
    vbo_size = 0;
    ibo_size = 0;
    std::vector<GESPMBuffer*> buffers;
    for (unsigned i = 0; i < Meshes.size(); i++)
    {
        scene::IAnimatedMesh* mesh = Meshes[i].Mesh;
        if (mesh->getMeshType() != scene::EAMT_SPM)
            continue;
        for (unsigned j = 0; j < mesh->getMeshBufferCount(); j++)
        {
            scene::IMeshBuffer* mb = mesh->getMeshBuffer(j);
            vbo_size += mesh->getMeshBuffer(j)->getVertexCount();
            ibo_size += mesh->getMeshBuffer(j)->getIndexCount();
            buffers.push_back(static_cast<GESPMBuffer*>(mb));
        }
    }
    vbo_size *= getVertexPitchFromType(video::EVT_SKINNED_MESH);
    ibo_size *= sizeof(uint16_t);

    VkBuffer staging_buffer = VK_NULL_HANDLE;
    VmaAllocation staging_memory = VK_NULL_HANDLE;
    VmaAllocationCreateInfo staging_buffer_create_info = {};
    staging_buffer_create_info.usage = VMA_MEMORY_USAGE_AUTO;
    staging_buffer_create_info.flags =
        VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT |
        VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT;
    staging_buffer_create_info.preferredFlags = VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;

    if (!m_vk->createBuffer(vbo_size,
        VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
        staging_buffer_create_info, staging_buffer, staging_memory))
        throw std::runtime_error("updateCache create staging vbo failed");

    uint8_t* mapped;
    if (vmaMapMemory(m_vk->getVmaAllocator(), staging_memory,
        (void**)&mapped) != VK_SUCCESS)
        throw std::runtime_error("updateCache vmaMapMemory failed");
    size_t offset = 0;
    for (GESPMBuffer* spm_buffer : buffers)
    {
        size_t copy_size = spm_buffer->getVertexCount() *
            getVertexPitchFromType(video::EVT_SKINNED_MESH);
        uint8_t* loc = mapped + offset;
        memcpy(loc, spm_buffer->getVertices(), copy_size);
        spm_buffer->setVBOOffset(
            offset / getVertexPitchFromType(video::EVT_SKINNED_MESH));
        offset += copy_size;
    }
    vmaUnmapMemory(m_vk->getVmaAllocator(), staging_memory);
    vmaFlushAllocation(m_vk->getVmaAllocator(), staging_memory, 0, offset);

    VmaAllocationCreateInfo local_create_info = {};
    local_create_info.usage = VMA_MEMORY_USAGE_AUTO;
    local_create_info.flags = VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT;
    if (!m_vk->createBuffer(vbo_size,
        VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
        local_create_info, m_vbo_buffer, m_vbo_memory))
        throw std::runtime_error("updateCache create vbo failed");

    m_vk->copyBuffer(staging_buffer, m_vbo_buffer, vbo_size);
    vmaDestroyBuffer(m_vk->getVmaAllocator(), staging_buffer, staging_memory);

    if (!m_vk->createBuffer(ibo_size,
        VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
        staging_buffer_create_info, staging_buffer, staging_memory))
        throw std::runtime_error("updateCache create staging ibo failed");

    if (vmaMapMemory(m_vk->getVmaAllocator(), staging_memory,
        (void**)&mapped) != VK_SUCCESS)
        throw std::runtime_error("updateCache vmaMapMemory failed");
    offset = 0;
    for (GESPMBuffer* spm_buffer : buffers)
    {
        size_t copy_size = spm_buffer->getIndexCount() * sizeof(uint16_t);
        uint8_t* loc = mapped + offset;
        memcpy(loc, spm_buffer->getIndices(), copy_size);
        spm_buffer->setIBOOffset(offset / sizeof(uint16_t));
        offset += copy_size;
    }
    vmaUnmapMemory(m_vk->getVmaAllocator(), staging_memory);
    vmaFlushAllocation(m_vk->getVmaAllocator(), staging_memory, 0, offset);

    if (!m_vk->createBuffer(ibo_size,
        VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
        local_create_info, m_ibo_buffer, m_ibo_memory))
        throw std::runtime_error("updateCache create ibo failed");

    m_vk->copyBuffer(staging_buffer, m_ibo_buffer, ibo_size);
    vmaDestroyBuffer(m_vk->getVmaAllocator(), staging_buffer, staging_memory);
}   // updateCache

// ----------------------------------------------------------------------------
void GEVulkanMeshCache::destroy()
{
    m_vk->waitIdle();

    vmaDestroyBuffer(m_vk->getVmaAllocator(), m_vbo_buffer, m_vbo_memory);
    vmaDestroyBuffer(m_vk->getVmaAllocator(), m_ibo_buffer, m_ibo_memory);

    m_vbo_memory = VK_NULL_HANDLE;
    m_vbo_buffer = VK_NULL_HANDLE;
    m_ibo_memory = VK_NULL_HANDLE;
    m_ibo_buffer = VK_NULL_HANDLE;
}   // destroy

}
