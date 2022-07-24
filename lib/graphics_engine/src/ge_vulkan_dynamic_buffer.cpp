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
GEVulkanDynamicBuffer::GEVulkanDynamicBuffer(GEVulkanDynamicBufferType t,
                                             VkBufferUsageFlags usage,
                                             size_t initial_size)
{
    m_type = t;
    m_usage = usage;
    m_size = m_real_size = initial_size;

    m_buffer = new VkBuffer[GEVulkanDriver::getMaxFrameInFlight()];
    m_memory = new VmaAllocation[GEVulkanDriver::getMaxFrameInFlight()];
    m_mapped_addr = new void*[GEVulkanDriver::getMaxFrameInFlight()];
    if (t == GVDBT_GPU_RAM)
    {
        m_staging_buffer = new VkBuffer[GEVulkanDriver::getMaxFrameInFlight()];
        m_staging_memory = new VmaAllocation[GEVulkanDriver::getMaxFrameInFlight()];
    }
    else
    {
        m_staging_buffer = NULL;
        m_staging_memory = NULL;
    }

    for (unsigned i = 0; i < GEVulkanDriver::getMaxFrameInFlight(); i++)
        initPerFrame(i);
}   // GEVulkanDynamicBuffer

// ----------------------------------------------------------------------------
GEVulkanDynamicBuffer::~GEVulkanDynamicBuffer()
{
    destroy();
    delete [] m_buffer;
    delete [] m_memory;
    delete [] m_staging_buffer;
    delete [] m_staging_memory;
    delete [] m_mapped_addr;
}   // ~GEVulkanDynamicBuffer

// ----------------------------------------------------------------------------
void GEVulkanDynamicBuffer::initPerFrame(unsigned frame)
{
    GEVulkanDriver* vk = getVKDriver();
    m_buffer[frame] = VK_NULL_HANDLE;
    m_memory[frame] = VK_NULL_HANDLE;
    m_mapped_addr[frame] = NULL;
    if (m_type == GVDBT_GPU_RAM)
    {
        m_staging_buffer[frame] = VK_NULL_HANDLE;
        m_staging_memory[frame] = VK_NULL_HANDLE;
    }

start:
    VkMemoryPropertyFlags prop = {};
    VkBuffer host_buffer = VK_NULL_HANDLE;
    VmaAllocation host_memory = VK_NULL_HANDLE;
    VmaAllocationCreateInfo host_info = {};
    host_info.usage = VMA_MEMORY_USAGE_AUTO;
    host_info.flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT |
        VMA_ALLOCATION_CREATE_MAPPED_BIT;
    host_info.preferredFlags = VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
    if ((m_type == GVDBT_SYSTEM_RAM && m_supports_host_transfer == 1) ||
        (m_type == GVDBT_SYSTEM_RAM && m_supports_host_transfer == -1))
    {
        host_info.flags |=
            VMA_ALLOCATION_CREATE_HOST_ACCESS_ALLOW_TRANSFER_INSTEAD_BIT;
    }

    if (!vk->createBuffer(m_size, m_usage | VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
        host_info, host_buffer, host_memory))
    {
        vmaDestroyBuffer(vk->getVmaAllocator(), host_buffer, host_memory);
        return;
    }

    if (m_type == GVDBT_SYSTEM_RAM && m_supports_host_transfer == -1)
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
    if (m_type == GVDBT_SYSTEM_RAM)
    {
        m_buffer[frame] = host_buffer;
        m_memory[frame] = host_memory;
        m_mapped_addr[frame] = info.pMappedData;
        return;
    }

    VkBuffer local_buffer = VK_NULL_HANDLE;
    VmaAllocation local_memory = VK_NULL_HANDLE;
    VmaAllocationCreateInfo local_info = {};
    local_info.usage = VMA_MEMORY_USAGE_AUTO;

    if (!vk->createBuffer(m_size, m_usage | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
        local_info, local_buffer, local_memory))
    {
        vmaDestroyBuffer(vk->getVmaAllocator(), host_buffer, host_memory);
        vmaDestroyBuffer(vk->getVmaAllocator(), local_buffer, local_memory);
        return;
    }
    m_buffer[frame] = local_buffer;
    m_memory[frame] = local_memory;
    m_staging_buffer[frame] = host_buffer;
    m_staging_memory[frame] = host_memory;
    m_mapped_addr[frame] = info.pMappedData;
}   // initPerFrame

// ----------------------------------------------------------------------------
void GEVulkanDynamicBuffer::destroyPerFrame(unsigned frame)
{
    GEVulkanDriver* vk = getVKDriver();
    vmaDestroyBuffer(vk->getVmaAllocator(), m_buffer[frame], m_memory[frame]);
    m_buffer[frame] = VK_NULL_HANDLE;
    m_memory[frame] = VK_NULL_HANDLE;
    m_mapped_addr[frame] = NULL;

    if (m_type == GVDBT_GPU_RAM)
    {
        vmaDestroyBuffer(vk->getVmaAllocator(), m_staging_buffer[frame],
            m_staging_memory[frame]);
        m_staging_buffer[frame] = VK_NULL_HANDLE;
        m_staging_memory[frame] = VK_NULL_HANDLE;
    }
}   // destroyPerFrame

// ----------------------------------------------------------------------------
void GEVulkanDynamicBuffer::destroy()
{
    getVKDriver()->waitIdle();
    for (unsigned i = 0; i < GEVulkanDriver::getMaxFrameInFlight(); i++)
        destroyPerFrame(i);
}   // destroy

// ----------------------------------------------------------------------------
void GEVulkanDynamicBuffer::setCurrentData(const std::vector<
                                           std::pair<void*, size_t> >& data,
                                           VkCommandBuffer custom_cmd)
{
    GEVulkanDriver* vk = getVKDriver();
    const unsigned cur_frame = vk->getCurrentFrame();

    size_t size = 0;
    for (auto& p : data)
        size += p.second;
    if (size > m_size)
    {
        destroy();
        m_size = size + 100;
        for (unsigned i = 0; i < GEVulkanDriver::getMaxFrameInFlight(); i++)
            initPerFrame(i);
    }
    m_real_size = size;
    if (size == 0 || m_mapped_addr[cur_frame] == NULL)
        return;

    uint8_t* addr = (uint8_t*)m_mapped_addr[cur_frame];
    for (auto& p : data)
    {
        memcpy(addr, p.first, p.second);
        addr += p.second;
    }
    vmaFlushAllocation(vk->getVmaAllocator(), m_staging_memory != NULL ?
        m_staging_memory[cur_frame] : m_memory[cur_frame], 0, size);

    if (m_type == GVDBT_GPU_RAM)
    {
        VkBufferCopy copy_region = {};
        copy_region.size = size;
        vkCmdCopyBuffer(custom_cmd ? custom_cmd : vk->getCurrentCommandBuffer(),
            m_staging_buffer[cur_frame], m_buffer[cur_frame], 1, &copy_region);
    }
}   // setCurrentData

// ----------------------------------------------------------------------------
VkBuffer GEVulkanDynamicBuffer::getCurrentBuffer() const
{
    return m_buffer[getVKDriver()->getCurrentFrame()];
}   // getCurrentBuffer

}
