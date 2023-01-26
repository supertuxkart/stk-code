#include "ge_spm_buffer.hpp"

#include "ge_main.hpp"
#include "ge_vulkan_driver.hpp"
#include "ge_vulkan_features.hpp"

#include <algorithm>
#include <stdexcept>

#include "mini_glm.hpp"

namespace GE
{
// ----------------------------------------------------------------------------
void GESPMBuffer::createVertexIndexBuffer()
{
    if (GEVulkanFeatures::supportsBaseVertexRendering())
        return;

    GEVulkanDriver* vk = getVKDriver();
    size_t total_pitch = getVertexPitchFromType(video::EVT_SKINNED_MESH);
    size_t bone_pitch = sizeof(int16_t) * 8;
    size_t static_pitch = total_pitch - bone_pitch;
    size_t vbo_size = getVertexCount() * static_pitch;
    m_ibo_offset = vbo_size;
    size_t ibo_size = getIndexCount() * sizeof(uint16_t);
    size_t total_size = vbo_size + ibo_size;
    if (m_has_skinning)
    {
        total_size += getPadding(total_size, 4);
        m_skinning_vbo_offset = total_size;
        total_size += getVertexCount() * bone_pitch;
    }

    VkBuffer staging_buffer = VK_NULL_HANDLE;
    VmaAllocation staging_memory = VK_NULL_HANDLE;
    VmaAllocationCreateInfo staging_buffer_create_info = {};
    staging_buffer_create_info.usage = VMA_MEMORY_USAGE_AUTO_PREFER_HOST;
    staging_buffer_create_info.flags =
        VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT;
    staging_buffer_create_info.preferredFlags = VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;

    if (!vk->createBuffer(total_size,
        VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT |
        VK_BUFFER_USAGE_TRANSFER_SRC_BIT, staging_buffer_create_info,
        staging_buffer, staging_memory))
    {
        throw std::runtime_error("createVertexIndexBuffer create staging "
            "buffer failed");
    }

    uint8_t* mapped;
    if (vmaMapMemory(vk->getVmaAllocator(), staging_memory,
        (void**)&mapped) != VK_SUCCESS)
        throw std::runtime_error("createVertexIndexBuffer vmaMapMemory failed");

    size_t real_size = getVertexCount() * total_pitch;
    uint8_t* loc = mapped;
    for (unsigned i = 0; i < real_size; i += total_pitch)
    {
        uint8_t* vertices = ((uint8_t*)getVertices()) + i;
        memcpy(loc, vertices, static_pitch);
        loc += static_pitch;
    }
    memcpy(loc, m_indices.data(), m_indices.size() * sizeof(uint16_t));

    if (m_has_skinning)
    {
        loc = mapped + m_skinning_vbo_offset;
        for (unsigned i = 0; i < real_size; i += total_pitch)
        {
            uint8_t* vertices = ((uint8_t*)getVertices()) + i +
                static_pitch;
            memcpy(loc, vertices, bone_pitch);
            loc += bone_pitch;
        }
    }

    vmaUnmapMemory(vk->getVmaAllocator(), staging_memory);
    vmaFlushAllocation(vk->getVmaAllocator(), staging_memory, 0, total_size);

    VmaAllocationCreateInfo local_create_info = {};
    local_create_info.usage = VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE;
    if (!vk->createBuffer(total_size,
        VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT |
        VK_BUFFER_USAGE_TRANSFER_DST_BIT, local_create_info, m_buffer,
        m_memory))
        throw std::runtime_error("updateCache create buffer failed");

    vk->copyBuffer(staging_buffer, m_buffer, total_size);
    vmaDestroyBuffer(vk->getVmaAllocator(), staging_buffer, staging_memory);
}   // createVertexIndexBuffer

// ----------------------------------------------------------------------------
void GESPMBuffer::destroyVertexIndexBuffer()
{
    if (m_buffer == VK_NULL_HANDLE || m_memory == VK_NULL_HANDLE)
        return;

    getVKDriver()->waitIdle();
    vmaDestroyBuffer(getVKDriver()->getVmaAllocator(), m_buffer, m_memory);
    m_buffer = VK_NULL_HANDLE;
    m_memory = VK_NULL_HANDLE;
}   // destroyVertexIndexBuffer

// ----------------------------------------------------------------------------
void GESPMBuffer::setNormal(u32 i, const core::vector3df& normal)
{
    m_vertices[i].m_normal = MiniGLM::compressVector3(normal);
}   // setNormal

// ----------------------------------------------------------------------------
void GESPMBuffer::setTCoords(u32 i, const core::vector2df& tcoords)
{
    m_vertices[i].m_all_uvs[0] = MiniGLM::toFloat16(tcoords.X);
    m_vertices[i].m_all_uvs[1] = MiniGLM::toFloat16(tcoords.Y);
}   // setTCoords

}
