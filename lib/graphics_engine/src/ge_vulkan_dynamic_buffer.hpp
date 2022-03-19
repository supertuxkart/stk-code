#ifndef HEADER_GE_VULKAN_DYNAMIC_BUFFER_HPP
#define HEADER_GE_VULKAN_DYNAMIC_BUFFER_HPP

#include "vulkan_wrapper.h"

namespace GE
{

enum GEVulkanDynamicBufferType : unsigned
{
    GVDBT_GPU_RAM,
    GVDBT_SYSTEM_RAM
};

class GEVulkanDynamicBuffer
{
private:
    VkBuffer* m_buffer;

    VkDeviceMemory* m_memory;

    VkBuffer* m_staging_buffer;

    VkDeviceMemory* m_staging_memory;

    void** m_mapped_addr;

    VkBufferUsageFlags m_usage;

    GEVulkanDynamicBufferType m_type;

    static VkMemoryPropertyFlags m_host_flag;

    size_t m_size;

    // ------------------------------------------------------------------------
    void initPerFrame(unsigned frame);
    // ------------------------------------------------------------------------
    void destroyPerFrame(unsigned frame);
    // ------------------------------------------------------------------------
    void destroy();
public:
    // ------------------------------------------------------------------------
    GEVulkanDynamicBuffer(GEVulkanDynamicBufferType t,
                          VkBufferUsageFlags usage, size_t initial_size);
    // ------------------------------------------------------------------------
    ~GEVulkanDynamicBuffer();
    // ------------------------------------------------------------------------
    void setCurrentData(void* data, size_t size);
    // ------------------------------------------------------------------------
    VkBuffer getCurrentBuffer() const;
};   // GEVulkanDynamicBuffer

}

#endif
