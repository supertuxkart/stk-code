#include "ge_vulkan_dynamic_spm_buffer.hpp"

#include "ge_vulkan_driver.hpp"
#include "ge_vulkan_dynamic_buffer.hpp"

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
}   // GEVulkanDynamicSPMBuffer

// ----------------------------------------------------------------------------
GEVulkanDynamicSPMBuffer::~GEVulkanDynamicSPMBuffer()
{
    delete m_vertex_buffer;
    delete m_index_buffer;
}   // ~GEVulkanDynamicSPMBuffer

} // end namespace GE
