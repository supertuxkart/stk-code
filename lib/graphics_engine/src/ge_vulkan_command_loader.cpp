#include "ge_vulkan_command_loader.hpp"

#include "ge_vulkan_driver.hpp"

#include <atomic>
#include <condition_variable>
#include <cstdio>
#include <deque>
#include <memory>
#include <mutex>
#include <stdexcept>
#include <thread>
#include <stdexcept>

#include "../source/Irrlicht/os.h"

#ifndef thread_local
# if __STDC_VERSION__ >= 201112 && !defined __STDC_NO_THREADS__
#  define thread_local _Thread_local
# elif defined _WIN32 && ( \
       defined _MSC_VER || \
       defined __ICL || \
       defined __DMC__ || \
       defined __BORLANDC__ )
#  define thread_local __declspec(thread)
/* note that ICC (linux) and Clang are covered by __GNUC__ */
# elif defined __GNUC__ || \
       defined __SUNPRO_C || \
       defined __xlC__
#  define thread_local __thread
# else
#  error "Cannot define thread_local"
# endif
#endif

namespace GE
{
namespace GEVulkanCommandLoader
{
// ============================================================================
GEVulkanDriver* g_vk = NULL;

std::mutex g_loaders_mutex;
std::condition_variable g_loaders_cv;
std::vector<std::thread> g_loaders;
std::deque<std::function<void()> > g_threaded_commands;
thread_local int g_loader_id = 0;
std::atomic_uint g_loader_count(0);

std::vector<VkCommandPool> g_command_pools;
std::vector<VkFence> g_command_fences;
std::vector<std::unique_ptr<std::atomic<bool> > > g_thread_idle;
}   // GEVulkanCommandLoader

// ============================================================================
void GEVulkanCommandLoader::init(GEVulkanDriver* vk)
{
    g_vk = vk;
    unsigned thread_count = std::thread::hardware_concurrency();
    if (thread_count == 0)
        thread_count = 3;
    else
        thread_count += 3;

    g_command_pools.resize(thread_count);
    g_command_fences.resize(thread_count);
    for (unsigned i = 0; i < thread_count; i++)
    {
        VkCommandPoolCreateInfo pool_info = {};
        pool_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
        pool_info.queueFamilyIndex = g_vk->getGraphicsFamily();
        pool_info.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
        VkResult result = vkCreateCommandPool(g_vk->getDevice(), &pool_info,
            NULL, &g_command_pools[i]);
        if (result != VK_SUCCESS)
        {
            throw std::runtime_error(
                "GEVulkanCommandLoader: vkCreateCommandPool failed");
        }
        VkFenceCreateInfo fence_info = {};
        fence_info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
        result = vkCreateFence(g_vk->getDevice(), &fence_info, NULL,
            &g_command_fences[i]);
        if (result != VK_SUCCESS)
        {
            throw std::runtime_error(
                "GEVulkanCommandLoader: vkCreateFence failed");
        }
    }

    for (unsigned i = 0; i < thread_count - 1; i++)
    {
        std::unique_ptr<std::atomic<bool> > idle;
        idle.reset(new std::atomic<bool>(true));
        g_thread_idle.push_back(std::move(idle));
    }
    g_loader_count.store(thread_count);
    for (unsigned i = 0; i < thread_count - 1; i++)
    {
        g_loaders.emplace_back(
            [i]()->void
            {
                g_loader_id = i + 1;
                while (true)
                {
                    g_thread_idle[i]->store(true);
                    std::unique_lock<std::mutex> ul(g_loaders_mutex);
                    g_loaders_cv.wait(ul, []
                        {
                            return !g_threaded_commands.empty();
                        });
                    if (g_loader_count.load() == 0)
                        return;

                    g_thread_idle[i]->store(false);
                    std::function<void()> copied = g_threaded_commands.front();
                    g_threaded_commands.pop_front();
                    ul.unlock();
                    copied();
                }
            });
    }

    char thread_count_str[40] = {};
    snprintf(thread_count_str, 40, "%d threads used, %d graphics queue(s)",
        thread_count - 1, vk->getGraphicsQueueCount());
    os::Printer::log("Vulkan command loader", thread_count_str);
}   // init

// ----------------------------------------------------------------------------
void GEVulkanCommandLoader::destroy()
{
    g_loader_count.store(0);
    if (!g_loaders.empty())
    {
        std::unique_lock<std::mutex> ul(g_loaders_mutex);
        g_threaded_commands.push_back([](){});
        g_loaders_cv.notify_all();
        ul.unlock();
        for (std::thread& t : g_loaders)
            t.join();
        g_loaders.clear();
    }
    for (auto& f : g_threaded_commands)
        f();
    g_threaded_commands.clear();
    g_thread_idle.clear();

    for (VkCommandPool& pool : g_command_pools)
        vkDestroyCommandPool(g_vk->getDevice(), pool, NULL);
    g_command_pools.clear();
    for (VkFence& fence : g_command_fences)
        vkDestroyFence(g_vk->getDevice(), fence, NULL);
    g_command_fences.clear();
}   // destroy

// ----------------------------------------------------------------------------
bool GEVulkanCommandLoader::multiThreadingEnabled()
{
    return g_loader_count.load() != 0;
}   // enabled

// ----------------------------------------------------------------------------
bool GEVulkanCommandLoader::isUsingMultiThreadingNow()
{
    return g_loader_id != 0;
}   // isUsingMultiThreadingNow

// ----------------------------------------------------------------------------
unsigned GEVulkanCommandLoader::getLoaderCount()
{
    return g_loader_count.load();
}   // getLoaderCount

// ----------------------------------------------------------------------------
int GEVulkanCommandLoader::getLoaderId()
{
    return g_loader_id;
}   // getLoaderId

// ----------------------------------------------------------------------------
VkCommandPool GEVulkanCommandLoader::getCurrentCommandPool()
{
    return g_command_pools[g_loader_id];
}   // getCurrentCommandPool

// ----------------------------------------------------------------------------
VkFence GEVulkanCommandLoader::getCurrentFence()
{
    return g_command_fences[g_loader_id];
}   // getCurrentFence

// ----------------------------------------------------------------------------
void GEVulkanCommandLoader::addMultiThreadingCommand(std::function<void()> cmd)
{
    if (g_loaders.empty())
        return;
    std::lock_guard<std::mutex> lock(g_loaders_mutex);
    g_threaded_commands.push_back(cmd);
    g_loaders_cv.notify_one();
}   // addMultiThreadingCommand

// ----------------------------------------------------------------------------
VkCommandBuffer GEVulkanCommandLoader::beginSingleTimeCommands()
{
    VkCommandBufferAllocateInfo alloc_info = {};
    alloc_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    alloc_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    alloc_info.commandPool = g_command_pools[g_loader_id];
    alloc_info.commandBufferCount = 1;

    VkCommandBuffer command_buffer;
    vkAllocateCommandBuffers(g_vk->getDevice(), &alloc_info, &command_buffer);

    VkCommandBufferBeginInfo begin_info = {};
    begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    begin_info.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

    vkBeginCommandBuffer(command_buffer, &begin_info);
    return command_buffer;
}   // beginSingleTimeCommands

// ----------------------------------------------------------------------------
void GEVulkanCommandLoader::endSingleTimeCommands(VkCommandBuffer command_buffer,
                                                  VkQueueFlagBits bit)
{
    vkEndCommandBuffer(command_buffer);

    VkSubmitInfo submit_info = {};
    submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submit_info.commandBufferCount = 1;
    submit_info.pCommandBuffers = &command_buffer;

    const int loader_id = g_loader_id;
    VkQueue queue = VK_NULL_HANDLE;
    std::unique_lock<std::mutex> lock = g_vk->getGraphicsQueue(&queue);
    vkQueueSubmit(queue, 1, &submit_info, g_command_fences[loader_id]);
    lock.unlock();

    vkWaitForFences(g_vk->getDevice(), 1, &g_command_fences[loader_id],
        VK_TRUE, std::numeric_limits<uint64_t>::max());
    vkResetFences(g_vk->getDevice(), 1, &g_command_fences[loader_id]);
    vkFreeCommandBuffers(g_vk->getDevice(), g_command_pools[loader_id], 1,
        &command_buffer);
}   // endSingleTimeCommands

// ----------------------------------------------------------------------------
void GEVulkanCommandLoader::waitIdle()
{
    while (true)
    {
        std::lock_guard<std::mutex> lock(g_loaders_mutex);
        if (g_threaded_commands.empty())
            break;
    }

    unsigned i = 0;
    while (i < g_thread_idle.size())
    {
        if (g_thread_idle[i]->load() == false)
            continue;
        i++;
    }
}   // waitIdle

}
