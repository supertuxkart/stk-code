#ifndef HEADER_GE_VULKAN_COMMAND_LOADER_HPP
#define HEADER_GE_VULKAN_COMMAND_LOADER_HPP

#include "vulkan_wrapper.h"

#include <functional>

namespace GE
{
class GEVulkanDriver;
namespace GEVulkanCommandLoader
{
// ----------------------------------------------------------------------------
void init(GEVulkanDriver*);
// ----------------------------------------------------------------------------
void destroy();
// ----------------------------------------------------------------------------
bool multiThreadingEnabled();
// ----------------------------------------------------------------------------
bool isUsingMultiThreadingNow();
// ----------------------------------------------------------------------------
unsigned getLoaderCount();
// ----------------------------------------------------------------------------
int getLoaderId();
// ----------------------------------------------------------------------------
VkCommandPool getCurrentCommandPool();
// ----------------------------------------------------------------------------
VkFence getCurrentFence();
// ----------------------------------------------------------------------------
void addMultiThreadingCommand(std::function<void()> cmd);
// ----------------------------------------------------------------------------
VkCommandBuffer beginSingleTimeCommands();
// ----------------------------------------------------------------------------
void endSingleTimeCommands(VkCommandBuffer command_buffer,
                           VkQueueFlagBits bit = VK_QUEUE_GRAPHICS_BIT);
// ----------------------------------------------------------------------------
void waitIdle();
};   // GEVulkanCommandLoader

}

#endif
