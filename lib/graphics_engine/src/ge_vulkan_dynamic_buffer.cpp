#include "ge_vulkan_dynamic_buffer.hpp"

#include "ge_vulkan_driver.hpp"
#include "ge_main.hpp"

#include <array>
#include <vector>
#include <functional>

namespace GE
{
VkMemoryPropertyFlags GEVulkanDynamicBuffer::m_host_flag = (VkMemoryPropertyFlags)-1;
// ----------------------------------------------------------------------------
GEVulkanDynamicBuffer::GEVulkanDynamicBuffer(GEVulkanDynamicBufferType t,
                                             VkBufferUsageFlags usage,
                                             size_t initial_size)
{
    m_type = t;
    m_usage = usage;
    m_size = m_real_size = initial_size;

    m_buffer = new VkBuffer[GEVulkanDriver::getMaxFrameInFlight()];
    m_memory = new VkDeviceMemory[GEVulkanDriver::getMaxFrameInFlight()];
    m_mapped_addr = new void*[GEVulkanDriver::getMaxFrameInFlight()];
    if (t == GVDBT_GPU_RAM)
    {
        m_staging_buffer = new VkBuffer[GEVulkanDriver::getMaxFrameInFlight()];
        m_staging_memory = new VkDeviceMemory[GEVulkanDriver::getMaxFrameInFlight()];
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
    m_buffer[frame] = VK_NULL_HANDLE;
    m_memory[frame] = VK_NULL_HANDLE;
    m_mapped_addr[frame] = NULL;
    if (m_type == GVDBT_GPU_RAM)
    {
        m_staging_buffer[frame] = VK_NULL_HANDLE;
        m_staging_memory[frame] = VK_NULL_HANDLE;
    }

    VkBuffer host_buffer = VK_NULL_HANDLE;
    VkDeviceMemory host_memory = VK_NULL_HANDLE;
    VkMemoryPropertyFlags host_flag = 0;
    if (m_host_flag == (VkFlags)-1)
    {
        // https://zeux.io/2020/02/27/writing-an-efficient-vulkan-renderer/
        m_host_flag = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT |
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
            VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
        bool better_flag_exist = false;
        if (getVKDriver()->createBuffer(m_size,
            m_usage | VK_BUFFER_USAGE_TRANSFER_SRC_BIT, m_host_flag,
            host_buffer, host_memory))
        {
            better_flag_exist = true;
            if (m_type == GVDBT_SYSTEM_RAM)
                goto succeed;
        }

        if (host_buffer != VK_NULL_HANDLE)
            vkDestroyBuffer(getVKDriver()->getDevice(), host_buffer, NULL);
        host_buffer = VK_NULL_HANDLE;
        if (host_memory != VK_NULL_HANDLE)
            vkFreeMemory(getVKDriver()->getDevice(), host_memory, NULL);
        host_memory = VK_NULL_HANDLE;

        if (!better_flag_exist)
        {
            m_host_flag = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
        }
    }

    host_flag = m_host_flag;
    // From the above website:
    // This flag should be used to store staging buffers that are used to
    // populate static resources allocated with
    // VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT with data.
    if (m_type == GVDBT_GPU_RAM)
    {
        host_flag = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
            VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
    }

    if (!getVKDriver()->createBuffer(m_size,
        m_usage | VK_BUFFER_USAGE_TRANSFER_SRC_BIT, host_flag,
        host_buffer, host_memory))
    {
        if (host_buffer != VK_NULL_HANDLE)
            vkDestroyBuffer(getVKDriver()->getDevice(), host_buffer, NULL);
        if (host_memory != VK_NULL_HANDLE)
            vkFreeMemory(getVKDriver()->getDevice(), host_memory, NULL);
        return;
    }

succeed:
    if (m_type == GVDBT_SYSTEM_RAM)
    {
        m_buffer[frame] = host_buffer;
        m_memory[frame] = host_memory;

        if (vkMapMemory(getVKDriver()->getDevice(), m_memory[frame], 0, m_size,
            0, &m_mapped_addr[frame]) != VK_SUCCESS)
        {
            destroyPerFrame(frame);
        }
        return;
    }

    VkBuffer local_buffer = VK_NULL_HANDLE;
    VkDeviceMemory local_memory = VK_NULL_HANDLE;
    if (!getVKDriver()->createBuffer(m_size,
        m_usage | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, local_buffer, local_memory))
    {
        if (host_buffer != VK_NULL_HANDLE)
            vkDestroyBuffer(getVKDriver()->getDevice(), host_buffer, NULL);
        if (host_memory != VK_NULL_HANDLE)
            vkFreeMemory(getVKDriver()->getDevice(), host_memory, NULL);
        if (local_buffer != VK_NULL_HANDLE)
            vkDestroyBuffer(getVKDriver()->getDevice(), local_buffer, NULL);
        if (local_memory != VK_NULL_HANDLE)
            vkFreeMemory(getVKDriver()->getDevice(), local_memory, NULL);
        return;
    }
    m_buffer[frame] = local_buffer;
    m_memory[frame] = local_memory;
    m_staging_buffer[frame] = host_buffer;
    m_staging_memory[frame] = host_memory;

    if (vkMapMemory(getVKDriver()->getDevice(), m_staging_memory[frame], 0,
        m_size, 0, &m_mapped_addr[frame]) != VK_SUCCESS)
    {
        destroyPerFrame(frame);
    }
}   // initPerFrame

// ----------------------------------------------------------------------------
void GEVulkanDynamicBuffer::destroyPerFrame(unsigned frame)
{
    if ((m_staging_memory && m_staging_memory[frame] != VK_NULL_HANDLE) ||
        m_memory[frame] != VK_NULL_HANDLE)
    {
        vkUnmapMemory(getVKDriver()->getDevice(), m_type == GVDBT_GPU_RAM ?
            m_staging_memory[frame] : m_memory[frame]);
    }

    if (m_memory[frame] != VK_NULL_HANDLE)
        vkFreeMemory(getVKDriver()->getDevice(), m_memory[frame], NULL);
    if (m_buffer[frame] != VK_NULL_HANDLE)
        vkDestroyBuffer(getVKDriver()->getDevice(), m_buffer[frame], NULL);

    m_buffer[frame] = VK_NULL_HANDLE;
    m_memory[frame] = VK_NULL_HANDLE;
    m_mapped_addr[frame] = NULL;

    if (m_type == GVDBT_GPU_RAM)
    {
        if (m_staging_buffer[frame] != VK_NULL_HANDLE)
            vkDestroyBuffer(getVKDriver()->getDevice(), m_staging_buffer[frame], NULL);
        if (m_staging_memory[frame] != VK_NULL_HANDLE)
            vkFreeMemory(getVKDriver()->getDevice(), m_staging_memory[frame], NULL);
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
void GEVulkanDynamicBuffer::setCurrentData(void* data, size_t size)
{
    const unsigned cur_frame = getVKDriver()->getCurrentFrame();
    if (size > m_size)
    {
        destroy();
        m_size = size + 100;
        for (unsigned i = 0; i < GEVulkanDriver::getMaxFrameInFlight(); i++)
            initPerFrame(i);
    }
    m_real_size = size;
    if (m_mapped_addr[cur_frame] == NULL)
        return;
    memcpy(m_mapped_addr[cur_frame], data, size);
    if (m_type == GVDBT_GPU_RAM)
    {
        VkBufferCopy copy_region = {};
        copy_region.size = size;
        vkCmdCopyBuffer(getVKDriver()->getCurrentCommandBuffer(),
            m_staging_buffer[cur_frame], m_buffer[cur_frame], 1, &copy_region);
    }
}   // setCurrentData

// ----------------------------------------------------------------------------
VkBuffer GEVulkanDynamicBuffer::getCurrentBuffer() const
{
    return m_buffer[getVKDriver()->getCurrentFrame()];
}   // getCurrentBuffer

}
