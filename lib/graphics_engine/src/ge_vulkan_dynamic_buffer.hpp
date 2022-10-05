#ifndef HEADER_GE_VULKAN_DYNAMIC_BUFFER_HPP
#define HEADER_GE_VULKAN_DYNAMIC_BUFFER_HPP

#include "vulkan_wrapper.h"
#include "ge_vma.hpp"

#include <vector>
#include <utility>

namespace GE
{

class GEVulkanDynamicBuffer
{
private:
    std::vector<VkBuffer> m_host_buffer, m_local_buffer;

    std::vector<VmaAllocation> m_host_memory, m_local_memory;

    std::vector<void*> m_mapped_addr;

    VkBufferUsageFlags m_usage;

    size_t m_size, m_real_size;

    static int m_supports_host_transfer;
    // ------------------------------------------------------------------------
    void initHostBuffer(unsigned frame, bool with_transfer);
    // ------------------------------------------------------------------------
    void initLocalBuffer(unsigned frame);
    // ------------------------------------------------------------------------
    void destroy();
public:
    // ------------------------------------------------------------------------
    GEVulkanDynamicBuffer(VkBufferUsageFlags usage, size_t initial_size,
                          unsigned host_buffer_size,
                          unsigned local_buffer_size);
    // ------------------------------------------------------------------------
    ~GEVulkanDynamicBuffer();
    // ------------------------------------------------------------------------
    void setCurrentData(const std::vector<std::pair<void*, size_t> >& data,
                        VkCommandBuffer custom_cmd = VK_NULL_HANDLE);
    // ------------------------------------------------------------------------
    void setCurrentData(void* data, size_t size,
                        VkCommandBuffer custom_cmd = VK_NULL_HANDLE)
                              { setCurrentData({{ data, size }}, custom_cmd); }
    // ------------------------------------------------------------------------
    VkBuffer getCurrentBuffer() const;
    // ------------------------------------------------------------------------
    void resizeIfNeeded(size_t new_size);
    // ------------------------------------------------------------------------
    size_t getRealSize() const                          { return m_real_size; }
    // ------------------------------------------------------------------------
    std::vector<VkBuffer>& getHostBuffer()            { return m_host_buffer; }
    // ------------------------------------------------------------------------
    std::vector<VkBuffer>& getLocalBuffer()          { return m_local_buffer; }
    // ------------------------------------------------------------------------
    std::vector<VmaAllocation>& getHostMemory()       { return m_host_memory; }
    // ------------------------------------------------------------------------
    std::vector<VmaAllocation>& getLocalMemory()     { return m_local_memory; }
    // ------------------------------------------------------------------------
    std::vector<void*>& getMappedAddr()               { return m_mapped_addr; }

};   // GEVulkanDynamicBuffer

}

#endif
