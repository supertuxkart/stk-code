#include "ge_vulkan_mesh_cache.hpp"

#include "ge_main.hpp"

#include "ge_spm_buffer.hpp"
#include "ge_vulkan_driver.hpp"
#include "ge_vulkan_features.hpp"

#include "IAnimatedMesh.h"

#include <algorithm>
#include <cassert>
#include <stdexcept>
#include <vector>
#include <stdexcept>

namespace GE
{
// ----------------------------------------------------------------------------
GEVulkanMeshCache::GEVulkanMeshCache()
                 : irr::scene::CMeshCache()
{
    m_vk = getVKDriver();
    m_irrlicht_cache_time = getMonoTimeMs();
    m_ge_cache_time = 0;
    m_buffer = VK_NULL_HANDLE;
    m_memory = VK_NULL_HANDLE;
    m_ibo_offset = m_skinning_vbo_offset = 0;
}   // init

// ----------------------------------------------------------------------------
void GEVulkanMeshCache::meshCacheChanged()
{
    m_irrlicht_cache_time = getMonoTimeMs();
}   // meshCacheChanged

// ----------------------------------------------------------------------------
void GEVulkanMeshCache::updateCache()
{
    if (!GEVulkanFeatures::supportsBaseVertexRendering())
        return;

    if (m_irrlicht_cache_time <= m_ge_cache_time)
        return;
    m_ge_cache_time = m_irrlicht_cache_time;

    destroy();
    size_t total_pitch = getVertexPitchFromType(video::EVT_SKINNED_MESH);
    size_t bone_pitch = sizeof(int16_t) * 8;
    size_t static_pitch = total_pitch - bone_pitch;

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
            GESPMBuffer* mb = static_cast<GESPMBuffer*>(mesh->getMeshBuffer(j));
            size_t pitch = mb->hasSkinning() ? total_pitch : static_pitch;
            vbo_size += mb->getVertexCount() * pitch;
            ibo_size += mb->getIndexCount();
            buffers.push_back(mb);
        }
    }
    ibo_size *= sizeof(uint16_t);
    // Some devices (Apple for now) require vertex offset of alignment 4 bytes
    ibo_size += getPadding(ibo_size, 4);
    std::stable_partition(buffers.begin(), buffers.end(),
        [](const GESPMBuffer* mb)
        {
            return !mb->hasSkinning();
        });

    VkBuffer staging_buffer = VK_NULL_HANDLE;
    VmaAllocation staging_memory = VK_NULL_HANDLE;
    VmaAllocationCreateInfo staging_buffer_create_info = {};
    staging_buffer_create_info.usage = VMA_MEMORY_USAGE_AUTO_PREFER_HOST;
    staging_buffer_create_info.flags =
        VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT |
        VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT;
    staging_buffer_create_info.preferredFlags = VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;

    if (!m_vk->createBuffer(vbo_size + ibo_size,
        VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT |
        VK_BUFFER_USAGE_TRANSFER_SRC_BIT, staging_buffer_create_info,
        staging_buffer, staging_memory))
        throw std::runtime_error("updateCache create staging buffer failed");

    uint8_t* mapped;
    if (vmaMapMemory(m_vk->getVmaAllocator(), staging_memory,
        (void**)&mapped) != VK_SUCCESS)
        throw std::runtime_error("updateCache vmaMapMemory failed");
    size_t offset = 0;
    for (GESPMBuffer* spm_buffer : buffers)
    {
        size_t real_size = spm_buffer->getVertexCount() * total_pitch;
        size_t copy_size = spm_buffer->getVertexCount() * static_pitch;
        uint8_t* loc = mapped + offset;
        for (unsigned i = 0; i < real_size; i += total_pitch)
        {
            uint8_t* vertices = ((uint8_t*)spm_buffer->getVertices()) + i;
            memcpy(loc, vertices, static_pitch);
            loc += static_pitch;
        }
        spm_buffer->setVBOOffset(offset / static_pitch);
        offset += copy_size;
    }
    m_ibo_offset = offset;

    offset = 0;
    for (GESPMBuffer* spm_buffer : buffers)
    {
        size_t copy_size = spm_buffer->getIndexCount() * sizeof(uint16_t);
        uint8_t* loc = mapped + offset + m_ibo_offset;
        memcpy(loc, spm_buffer->getIndices(), copy_size);
        spm_buffer->setIBOOffset(offset / sizeof(uint16_t));
        offset += copy_size;
    }
    m_skinning_vbo_offset = m_ibo_offset + offset;
    m_skinning_vbo_offset += getPadding(m_skinning_vbo_offset, 4);

    offset = 0;
    size_t static_vertex_offset = 0;
    for (GESPMBuffer* spm_buffer : buffers)
    {
        if (!spm_buffer->hasSkinning())
        {
            static_vertex_offset += spm_buffer->getVertexCount() * bone_pitch;
            continue;
        }
        uint8_t* loc = mapped + offset + m_skinning_vbo_offset;
        size_t real_size = spm_buffer->getVertexCount() * total_pitch;
        for (unsigned i = 0; i < real_size; i += total_pitch)
        {
            uint8_t* vertices = ((uint8_t*)spm_buffer->getVertices()) + i +
                static_pitch;
            memcpy(loc, vertices, bone_pitch);
            loc += bone_pitch;
        }
        offset += spm_buffer->getVertexCount() * bone_pitch;
    }
    assert(m_skinning_vbo_offset + offset == vbo_size + ibo_size);
    assert(static_vertex_offset < m_skinning_vbo_offset);
    m_skinning_vbo_offset -= static_vertex_offset;

    vmaUnmapMemory(m_vk->getVmaAllocator(), staging_memory);
    vmaFlushAllocation(m_vk->getVmaAllocator(), staging_memory, 0, offset);

    VmaAllocationCreateInfo local_create_info = {};
    local_create_info.usage = VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE;
    local_create_info.flags = VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT;
    if (!m_vk->createBuffer(vbo_size + ibo_size,
        VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT |
        VK_BUFFER_USAGE_TRANSFER_DST_BIT, local_create_info, m_buffer,
        m_memory))
        throw std::runtime_error("updateCache create buffer failed");

    m_vk->copyBuffer(staging_buffer, m_buffer, vbo_size + ibo_size);
    vmaDestroyBuffer(m_vk->getVmaAllocator(), staging_buffer, staging_memory);
}   // updateCache

// ----------------------------------------------------------------------------
void GEVulkanMeshCache::destroy()
{
    m_vk->waitIdle();
    m_vk->setDisableWaitIdle(true);
    if (!GEVulkanFeatures::supportsBaseVertexRendering())
    {
        for (unsigned i = 0; i < Meshes.size(); i++)
        {
            scene::IAnimatedMesh* mesh = Meshes[i].Mesh;
            if (mesh->getMeshType() != scene::EAMT_SPM)
                continue;
            for (unsigned j = 0; j < mesh->getMeshBufferCount(); j++)
            {
                GESPMBuffer* mb = static_cast<GESPMBuffer*>(
                    mesh->getMeshBuffer(j));
                mb->destroyVertexIndexBuffer();
            }
        }
    }
    else
    {
        vmaDestroyBuffer(m_vk->getVmaAllocator(), m_buffer, m_memory);
        m_buffer = VK_NULL_HANDLE;
        m_memory = VK_NULL_HANDLE;
    }
    m_vk->setDisableWaitIdle(false);

    m_ibo_offset = m_skinning_vbo_offset = 0;
}   // destroy

}
