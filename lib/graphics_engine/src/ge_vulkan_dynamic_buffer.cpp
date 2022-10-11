#include "ge_vulkan_dynamic_buffer.hpp"

#include "ge_vulkan_driver.hpp"
#include "ge_main.hpp"

#include <array>
#include <vector>
#include <functional>

namespace GE
{
int GEVulkanDynamicBuffer::m_supports_host_transfer = -1;
// ----------------------------------------------------------------------------
GEVulkanDynamicBuffer::GEVulkanDynamicBuffer(VkBufferUsageFlags usage,
                                             size_t initial_size,
                                             unsigned host_buffer_size,
                                             unsigned local_buffer_size,
                                             bool enable_host_transfer)
                     : m_usage(usage),
                       m_enable_host_transfer(enable_host_transfer)
{
    m_size = m_real_size = initial_size;

    m_host_buffer.resize(host_buffer_size, VK_NULL_HANDLE);
    m_host_memory.resize(host_buffer_size, VK_NULL_HANDLE);
    m_mapped_addr.resize(host_buffer_size, VK_NULL_HANDLE);

    m_local_buffer.resize(local_buffer_size, VK_NULL_HANDLE);
    m_local_memory.resize(local_buffer_size, VK_NULL_HANDLE);

    for (unsigned i = 0; i < m_host_buffer.size(); i++)
        initHostBuffer(i, m_local_buffer.size() == 0);
    for (unsigned i = 0; i < m_local_buffer.size(); i++)
        initLocalBuffer(i);
}   // GEVulkanDynamicBuffer

// ----------------------------------------------------------------------------
GEVulkanDynamicBuffer::~GEVulkanDynamicBuffer()
{
    destroy();
}   // ~GEVulkanDynamicBuffer

// ----------------------------------------------------------------------------
void GEVulkanDynamicBuffer::initHostBuffer(unsigned frame, bool with_transfer)
{
    GEVulkanDriver* vk = getVKDriver();
start:
    VkMemoryPropertyFlags prop = {};
    VkBuffer host_buffer = VK_NULL_HANDLE;
    VmaAllocation host_memory = VK_NULL_HANDLE;
    VmaAllocationCreateInfo host_info = {};
    host_info.usage = VMA_MEMORY_USAGE_AUTO_PREFER_HOST;
    host_info.flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT |
        VMA_ALLOCATION_CREATE_MAPPED_BIT;
    host_info.preferredFlags = VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
    if (((with_transfer && m_supports_host_transfer == 1) ||
        (with_transfer && m_supports_host_transfer == -1)) &&
        m_enable_host_transfer)
    {
        host_info.flags |=
            VMA_ALLOCATION_CREATE_HOST_ACCESS_ALLOW_TRANSFER_INSTEAD_BIT;
        host_info.usage = VMA_MEMORY_USAGE_AUTO;
    }

    if (!vk->createBuffer(m_size, m_usage | VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
        host_info, host_buffer, host_memory))
    {
        vmaDestroyBuffer(vk->getVmaAllocator(), host_buffer, host_memory);
        return;
    }

    if (with_transfer && m_enable_host_transfer &&
        m_supports_host_transfer == -1)
    {
        vmaGetAllocationMemoryProperties(vk->getVmaAllocator(), host_memory,
            &prop);
        if ((prop & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT) == 0)
        {
            m_supports_host_transfer = 0;
            vmaDestroyBuffer(vk->getVmaAllocator(), host_buffer, host_memory);
            goto start;
        }
        else
            m_supports_host_transfer = 1;
    }

    VmaAllocationInfo info = {};
    vmaGetAllocationInfo(vk->getVmaAllocator(), host_memory, &info);

    m_host_buffer[frame] = host_buffer;
    m_host_memory[frame] = host_memory;
    m_mapped_addr[frame] = info.pMappedData;
}   // initHostBuffer

// ----------------------------------------------------------------------------
void GEVulkanDynamicBuffer::initLocalBuffer(unsigned frame)
{
    GEVulkanDriver* vk = getVKDriver();
    VkBuffer local_buffer = VK_NULL_HANDLE;
    VmaAllocation local_memory = VK_NULL_HANDLE;
    VmaAllocationCreateInfo local_info = {};
    local_info.usage = VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE;

    VkBufferUsageFlags flags = m_usage;
    if (!m_host_buffer.empty())
        flags |= VK_BUFFER_USAGE_TRANSFER_DST_BIT;
    if (!vk->createBuffer(m_size, flags, local_info, local_buffer,
        local_memory))
    {
        vmaDestroyBuffer(vk->getVmaAllocator(), local_buffer, local_memory);
        return;
    }
    m_local_buffer[frame] = local_buffer;
    m_local_memory[frame] = local_memory;
}   // initLocalBuffer

// ----------------------------------------------------------------------------
void GEVulkanDynamicBuffer::destroy()
{
    GEVulkanDriver* vk = getVKDriver();
    vk->waitIdle();
    for (unsigned i = 0; i < m_host_buffer.size(); i++)
    {
        vmaDestroyBuffer(vk->getVmaAllocator(), m_host_buffer[i],
            m_host_memory[i]);
        m_host_buffer[i] = VK_NULL_HANDLE;
        m_host_memory[i] = VK_NULL_HANDLE;
        m_mapped_addr[i] = NULL;
    }
    for (unsigned i = 0; i < m_local_buffer.size(); i++)
    {
        vmaDestroyBuffer(vk->getVmaAllocator(), m_local_buffer[i],
            m_local_memory[i]);
        m_local_buffer[i] = VK_NULL_HANDLE;
        m_local_memory[i] = VK_NULL_HANDLE;
    }
}   // destroy

// ----------------------------------------------------------------------------
bool GEVulkanDynamicBuffer::setCurrentData(const std::vector<
                                           std::pair<void*, size_t> >& data,
                                           VkCommandBuffer custom_cmd,
                                           unsigned cur_frame)
{
    GEVulkanDriver* vk = getVKDriver();

    size_t size = 0;
    for (auto& p : data)
        size += p.second;
    bool ret = resizeIfNeeded(size);

    m_real_size = size;
    bool forced_frame = true;
    if (cur_frame == (unsigned)-1)
    {
        cur_frame = vk->getCurrentFrame();
        forced_frame = false;
    }
    if (cur_frame >= m_mapped_addr.size())
        cur_frame = 0;

    if (size == 0 || m_mapped_addr.empty() || m_mapped_addr[cur_frame] == NULL)
        return ret;

    uint8_t* addr = (uint8_t*)m_mapped_addr[cur_frame];
    for (auto& p : data)
    {
        memcpy(addr, p.first, p.second);
        addr += p.second;
    }
    vmaFlushAllocation(vk->getVmaAllocator(), m_host_memory[cur_frame], 0,
        size);

    if (!m_local_buffer.empty())
    {
        unsigned cur_local_frame = vk->getCurrentFrame();
        if (forced_frame)
            cur_local_frame = cur_frame;
        if (cur_local_frame >= m_local_buffer.size())
            cur_local_frame = 0;
        VkBufferCopy copy_region = {};
        copy_region.size = size;
        vkCmdCopyBuffer(custom_cmd ? custom_cmd : vk->getCurrentCommandBuffer(),
            m_host_buffer[cur_frame], m_local_buffer[cur_local_frame], 1,
            &copy_region);
    }
    return ret;
}   // setCurrentData

// ----------------------------------------------------------------------------
bool GEVulkanDynamicBuffer::resizeIfNeeded(size_t new_size)
{
    if (new_size > m_size)
    {
        destroy();
        m_size = new_size + 100;
        for (unsigned i = 0; i < m_host_buffer.size(); i++)
            initHostBuffer(i, m_local_buffer.size() == 0);
        for (unsigned i = 0; i < m_local_buffer.size(); i++)
            initLocalBuffer(i);
        return true;
    }
    return false;
}   // resizeIfNeeded

// ----------------------------------------------------------------------------
VkBuffer GEVulkanDynamicBuffer::getCurrentBuffer() const
{
    unsigned cur_frame = getVKDriver()->getCurrentFrame();
    if (m_local_buffer.empty())
    {
        if (cur_frame >= m_host_buffer.size())
            cur_frame = 0;
        return m_host_buffer[cur_frame];
    }
    else
    {
        if (cur_frame >= m_local_buffer.size())
            cur_frame = 0;
        return m_local_buffer[cur_frame];
    }
}   // getCurrentBuffer

}
