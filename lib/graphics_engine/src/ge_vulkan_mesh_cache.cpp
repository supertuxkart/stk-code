#include "ge_vulkan_mesh_cache.hpp"

#include "ge_main.hpp"
#include "ge_spm_buffer.hpp"
#include "ge_vulkan_driver.hpp"

#include "IAnimatedMesh.h"
#include "IMeshCache.h"
#include "ISceneManager.h"
#include "IrrlichtDevice.h"

#include <vector>

namespace GE
{
namespace GEVulkanMeshCache
{
// ============================================================================
GEVulkanDriver* g_vk;
uint64_t g_irrlicht_cache_time;
uint64_t g_ge_cache_time;
VkBuffer g_vbo_buffer;
VkDeviceMemory g_vbo_memory;
VkBuffer g_ibo_buffer;
VkDeviceMemory g_ibo_memory;
// ----------------------------------------------------------------------------
void init(GEVulkanDriver* vk)
{
    g_vk = vk;
    g_irrlicht_cache_time = getMonoTimeMs();
    g_ge_cache_time = 0;
    g_vbo_buffer = VK_NULL_HANDLE;
    g_vbo_memory = VK_NULL_HANDLE;
    g_ibo_buffer = VK_NULL_HANDLE;
    g_ibo_memory = VK_NULL_HANDLE;
}   // init

// ----------------------------------------------------------------------------
void irrlichtMeshChanged()
{
    g_irrlicht_cache_time = getMonoTimeMs();
}   // irrlichtMeshChanged

// ----------------------------------------------------------------------------
void updateCache()
{
    if (g_irrlicht_cache_time <= g_ge_cache_time)
        return;
    g_ge_cache_time = g_irrlicht_cache_time;

    destroy();
    size_t vbo_size, ibo_size;
    vbo_size = 0;
    ibo_size = 0;
    scene::IMeshCache* cache = g_vk->getIrrlichtDevice()->getSceneManager()
        ->getMeshCache();
    std::vector<GESPMBuffer*> buffers;
    for (unsigned i = 0; i < cache->getMeshCount(); i++)
    {
        scene::IAnimatedMesh* mesh = cache->getMeshByIndex(i);
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

    if (!g_vk->createBuffer(vbo_size,
        VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
        VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
        staging_buffer, staging_memory))
        throw std::runtime_error("updateCache create staging vbo failed");

    uint8_t* mapped;
    if (vkMapMemory(g_vk->getDevice(), staging_memory, 0,
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

    if (!g_vk->createBuffer(vbo_size,
        VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
        g_vbo_buffer, g_vbo_memory))
        throw std::runtime_error("updateCache create vbo failed");

    g_vk->copyBuffer(staging_buffer, g_vbo_buffer, vbo_size);
    vkUnmapMemory(g_vk->getDevice(), staging_memory);
    vkFreeMemory(g_vk->getDevice(), staging_memory, NULL);
    vkDestroyBuffer(g_vk->getDevice(), staging_buffer, NULL);

    if (!g_vk->createBuffer(ibo_size,
        VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
        VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
        staging_buffer, staging_memory))
        throw std::runtime_error("updateCache create staging ibo failed");

    if (vkMapMemory(g_vk->getDevice(), staging_memory, 0,
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

    if (!g_vk->createBuffer(ibo_size,
        VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
        g_ibo_buffer, g_ibo_memory))
        throw std::runtime_error("updateCache create ibo failed");

    g_vk->copyBuffer(staging_buffer, g_ibo_buffer, ibo_size);
    vkUnmapMemory(g_vk->getDevice(), staging_memory);
    vkFreeMemory(g_vk->getDevice(), staging_memory, NULL);
    vkDestroyBuffer(g_vk->getDevice(), staging_buffer, NULL);
}   // updateCache

// ----------------------------------------------------------------------------
void destroy()
{
    g_vk->waitIdle();

    if (g_vbo_memory != VK_NULL_HANDLE)
        vkFreeMemory(g_vk->getDevice(), g_vbo_memory, NULL);
    g_vbo_memory = VK_NULL_HANDLE;
    if (g_vbo_buffer != VK_NULL_HANDLE)
        vkDestroyBuffer(g_vk->getDevice(), g_vbo_buffer, NULL);
    g_vbo_buffer = VK_NULL_HANDLE;

    if (g_ibo_memory != VK_NULL_HANDLE)
        vkFreeMemory(g_vk->getDevice(), g_ibo_memory, NULL);
    g_ibo_memory = VK_NULL_HANDLE;
    if (g_ibo_buffer != VK_NULL_HANDLE)
        vkDestroyBuffer(g_vk->getDevice(), g_ibo_buffer, NULL);
    g_ibo_buffer = VK_NULL_HANDLE;
}   // destroy

// ----------------------------------------------------------------------------
VkBuffer getVBO()
{
    return g_vbo_buffer;
}   // getVBO

// ----------------------------------------------------------------------------
VkBuffer getIBO()
{
    return g_ibo_buffer;
}   // getIBO

}

}
