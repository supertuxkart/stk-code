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
    VkDeviceMemory staging_memory = VK_NULL_HANDLE;

    if (!m_vk->createBuffer(vbo_size,
        VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
        VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
        staging_buffer, staging_memory))
        throw std::runtime_error("updateCache create staging vbo failed");

    uint8_t* mapped;
    if (vkMapMemory(m_vk->getDevice(), staging_memory, 0,
        vbo_size, 0, (void**)&mapped) != VK_SUCCESS)
        throw std::runtime_error("updateCache vkMapMemory failed");
    size_t offset = 0;
    for (GESPMBuffer* spm_buffer : buffers)
    {
        size_t copy_size = spm_buffer->getVertexCount() *
            getVertexPitchFromType(video::EVT_SKINNED_MESH);
        memcpy(&mapped[offset], spm_buffer->getVertices(), copy_size);
        spm_buffer->setVBOOffset(
            offset / getVertexPitchFromType(video::EVT_SKINNED_MESH));
        offset += copy_size;
    }

    if (!m_vk->createBuffer(vbo_size,
        VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
        m_vbo_buffer, m_vbo_memory))
        throw std::runtime_error("updateCache create vbo failed");

    m_vk->copyBuffer(staging_buffer, m_vbo_buffer, vbo_size);
    vkUnmapMemory(m_vk->getDevice(), staging_memory);
    vkFreeMemory(m_vk->getDevice(), staging_memory, NULL);
    vkDestroyBuffer(m_vk->getDevice(), staging_buffer, NULL);

    if (!m_vk->createBuffer(ibo_size,
        VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
        VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
        staging_buffer, staging_memory))
        throw std::runtime_error("updateCache create staging ibo failed");

    if (vkMapMemory(m_vk->getDevice(), staging_memory, 0,
        ibo_size, 0, (void**)&mapped) != VK_SUCCESS)
        throw std::runtime_error("updateCache vkMapMemory failed");
    offset = 0;
    for (GESPMBuffer* spm_buffer : buffers)
    {
        size_t copy_size = spm_buffer->getIndexCount() * sizeof(uint16_t);
        memcpy(&mapped[offset], spm_buffer->getIndices(), copy_size);
        spm_buffer->setIBOOffset(offset / sizeof(uint16_t));
        offset += copy_size;
    }

    if (!m_vk->createBuffer(ibo_size,
        VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
        m_ibo_buffer, m_ibo_memory))
        throw std::runtime_error("updateCache create ibo failed");

    m_vk->copyBuffer(staging_buffer, m_ibo_buffer, ibo_size);
    vkUnmapMemory(m_vk->getDevice(), staging_memory);
    vkFreeMemory(m_vk->getDevice(), staging_memory, NULL);
    vkDestroyBuffer(m_vk->getDevice(), staging_buffer, NULL);
}   // updateCache

// ----------------------------------------------------------------------------
void GEVulkanMeshCache::destroy()
{
    m_vk->waitIdle();

    if (m_vbo_memory != VK_NULL_HANDLE)
        vkFreeMemory(m_vk->getDevice(), m_vbo_memory, NULL);
    m_vbo_memory = VK_NULL_HANDLE;
    if (m_vbo_buffer != VK_NULL_HANDLE)
        vkDestroyBuffer(m_vk->getDevice(), m_vbo_buffer, NULL);
    m_vbo_buffer = VK_NULL_HANDLE;

    if (m_ibo_memory != VK_NULL_HANDLE)
        vkFreeMemory(m_vk->getDevice(), m_ibo_memory, NULL);
    m_ibo_memory = VK_NULL_HANDLE;
    if (m_ibo_buffer != VK_NULL_HANDLE)
        vkDestroyBuffer(m_vk->getDevice(), m_ibo_buffer, NULL);
    m_ibo_buffer = VK_NULL_HANDLE;
}   // destroy

}
