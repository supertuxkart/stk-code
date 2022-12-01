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
    m_vertex_buffer = new GEVulkanDynamicBuffer(
        VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, 100,
        GEVulkanDriver::getMaxFrameInFlight() + 1, 0);
    m_index_buffer = new GEVulkanDynamicBuffer(
        VK_BUFFER_USAGE_INDEX_BUFFER_BIT, 100,
        GEVulkanDriver::getMaxFrameInFlight() + 1, 0);
    getVKDriver()->addDynamicSPMBuffer(this);
}   // GEVulkanDynamicSPMBuffer

// ----------------------------------------------------------------------------
GEVulkanDynamicSPMBuffer::~GEVulkanDynamicSPMBuffer()
{
    getVKDriver()->removeDynamicSPMBuffer(this);
    delete m_vertex_buffer;
    delete m_index_buffer;
}   // ~GEVulkanDynamicSPMBuffer

// ----------------------------------------------------------------------------
void GEVulkanDynamicSPMBuffer::updateVertexIndexBuffer(int buffer_index)
{
    const size_t stride = sizeof(irr::video::S3DVertexSkinnedMesh) - 16;
    double vertex_size = (double)(m_vertices.size() * stride);
    double base = std::log2(vertex_size);
    m_vertex_buffer->resizeIfNeeded(2 << (unsigned)base);
    uint8_t* mapped_addr = (uint8_t*)m_vertex_buffer->getMappedAddr()
        [buffer_index];
    for (unsigned i = 0; i < m_vertices.size(); i++)
    {
        memcpy(mapped_addr, &m_vertices[i], stride);
        mapped_addr += stride;
    }
    m_index_buffer->setCurrentData(m_indices.data(), m_indices.size() *
        sizeof(uint16_t), NULL, buffer_index);
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

} // end namespace GE
