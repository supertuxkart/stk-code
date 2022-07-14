#ifndef HEADER_GE_VULKAN_DYNAMIC_BUFFER_HPP
#define HEADER_GE_VULKAN_DYNAMIC_BUFFER_HPP

#include "vulkan_wrapper.h"
#include "ge_vma.hpp"

#include <vector>
#include <utility>

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

    VmaAllocation* m_memory;

    VkBuffer* m_staging_buffer;

    VmaAllocation* m_staging_memory;

    void** m_mapped_addr;

    VkBufferUsageFlags m_usage;

    GEVulkanDynamicBufferType m_type;

    size_t m_size, m_real_size;

    static int m_supports_host_transfer;
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
    void setCurrentData(const std::vector<std::pair<void*, size_t> >& data);
    // ------------------------------------------------------------------------
    void setCurrentData(void* data, size_t size)
                                          { setCurrentData({{ data, size }}); }
    // ------------------------------------------------------------------------
    VkBuffer getCurrentBuffer() const;
    // ------------------------------------------------------------------------
    size_t getRealSize() const                          { return m_real_size; }
};   // GEVulkanDynamicBuffer

}

#endif
