#include "ge_vulkan_dynamic_spm_buffer.hpp"

#include "ge_main.hpp"
#include "ge_vulkan_driver.hpp"
#include "ge_vulkan_dynamic_buffer.hpp"

#include <cmath>

namespace GE
{

// ----------------------------------------------------------------------------
GEVulkanDynamicSPMBuffer::GEVulkanDynamicSPMBuffer()
{
    unsigned frame_count = GEVulkanDriver::getMaxFrameInFlight() + 1;
    m_vertex_buffer = new GEVulkanDynamicBuffer(
        VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, 100, frame_count, 0);
    m_index_buffer = new GEVulkanDynamicBuffer(
        VK_BUFFER_USAGE_INDEX_BUFFER_BIT, 100, frame_count, 0);
    m_vk = getVKDriver();
    m_vk->addDynamicSPMBuffer(this);
    m_vertex_update_offsets = new uint32_t[frame_count]();
    m_index_update_offsets = new uint32_t[frame_count]();
}   // GEVulkanDynamicSPMBuffer

// ----------------------------------------------------------------------------
GEVulkanDynamicSPMBuffer::~GEVulkanDynamicSPMBuffer()
{
    m_vk->removeDynamicSPMBuffer(this);
    delete m_vertex_buffer;
    delete m_index_buffer;
    delete [] m_vertex_update_offsets;
    delete [] m_index_update_offsets;
}   // ~GEVulkanDynamicSPMBuffer

// ----------------------------------------------------------------------------
void GEVulkanDynamicSPMBuffer::updateVertexIndexBuffer(int buffer_index)
{
    if (m_vertex_update_offsets[buffer_index] == m_vertices.size() &&
        m_index_update_offsets[buffer_index] == m_indices.size())
        return;

    const size_t stride = sizeof(irr::video::S3DVertexSkinnedMesh) - 16;
    double vertex_size = (double)(m_vertices.size() * stride);
    double base = std::log2(vertex_size);
    unsigned frame_count = GEVulkanDriver::getMaxFrameInFlight() + 1;
    if (m_vertex_buffer->resizeIfNeeded(2 << (unsigned)base))
        std::fill_n(m_vertex_update_offsets, frame_count, 0);

    uint8_t* mapped_addr = (uint8_t*)m_vertex_buffer->getMappedAddr()
        [buffer_index];
    unsigned voffset = m_vertex_update_offsets[buffer_index] * stride;
    mapped_addr += voffset;
    for (unsigned i = m_vertex_update_offsets[buffer_index];
        i < m_vertices.size(); i++)
    {
        memcpy(mapped_addr, &m_vertices[i], stride);
        mapped_addr += stride;
    }
    m_vertex_update_offsets[buffer_index] = m_vertices.size();

    double index_size = (double)(m_indices.size() * sizeof(uint16_t));
    base = std::log2(index_size);
    if (m_index_buffer->resizeIfNeeded(2 << (unsigned)base))
        std::fill_n(m_index_update_offsets, frame_count, 0);

    mapped_addr = (uint8_t*)m_index_buffer->getMappedAddr()
        [buffer_index];
    unsigned ioffset = m_index_update_offsets[buffer_index] * sizeof(uint16_t);
    mapped_addr += ioffset;
    for (unsigned i = m_index_update_offsets[buffer_index];
        i < m_indices.size(); i++)
    {
        memcpy(mapped_addr, &m_indices[i], sizeof(uint16_t));
        mapped_addr += sizeof(uint16_t);
    }
    m_index_update_offsets[buffer_index] = m_indices.size();
}   // updateVertexIndexBuffer

// ----------------------------------------------------------------------------
void GEVulkanDynamicSPMBuffer::drawDynamicVertexIndexBuffer(VkCommandBuffer cmd,
                                                            int buffer_index)
{
    std::array<VkBuffer, 2> vertex_buffer =
    {{
        m_vertex_buffer->getHostBuffer()[buffer_index],
        m_vertex_buffer->getHostBuffer()[buffer_index]
    }};
    std::array<VkDeviceSize, 2> offsets =
    {{
        0,
        0
    }};
    vkCmdBindVertexBuffers(cmd, 0, vertex_buffer.size(), vertex_buffer.data(),
        offsets.data());
    VkBuffer index_buffer = m_index_buffer->getHostBuffer()[buffer_index];
    vkCmdBindIndexBuffer(cmd, index_buffer, 0, VK_INDEX_TYPE_UINT16);
    vkCmdDrawIndexed(cmd, getIndexCount(), 1, 0, 0, 0);
}   // drawDynamicVertexIndexBuffer

// ----------------------------------------------------------------------------
void GEVulkanDynamicSPMBuffer::setDirtyOffset(irr::u32 offset,
                                              irr::scene::E_BUFFER_TYPE buffer)
{
    int vertex_update_offset = -1;
    int index_update_offset = -1;
    if (buffer == irr::scene::EBT_VERTEX_AND_INDEX)
        vertex_update_offset = index_update_offset = offset;
    else if (buffer == irr::scene::EBT_VERTEX)
        vertex_update_offset = offset;
    else if (buffer == irr::scene::EBT_INDEX)
        index_update_offset = offset;
    unsigned frame_count = GEVulkanDriver::getMaxFrameInFlight() + 1;
    if (vertex_update_offset != -1)
    {
        for (unsigned i = 0; i < frame_count; i++)
        {
            if (m_vertex_update_offsets[i] > vertex_update_offset)
                m_vertex_update_offsets[i] = vertex_update_offset;
        }
    }
    if (index_update_offset != -1)
    {
        for (unsigned i = 0; i < frame_count; i++)
        {
            if (m_index_update_offsets[i] > index_update_offset)
                m_index_update_offsets[i] = index_update_offset;
        }
    }
}   // setDirtyOffset

} // end namespace GE
