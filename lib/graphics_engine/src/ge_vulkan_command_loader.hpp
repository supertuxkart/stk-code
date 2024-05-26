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
VkCommandPool getCurrentCommandPool(uint32_t index);
// ----------------------------------------------------------------------------
VkFence getCurrentFence(uint32_t index);
// ----------------------------------------------------------------------------
void addMultiThreadingCommand(std::function<void()> cmd);
// ----------------------------------------------------------------------------
VkCommandBuffer beginSingleTimeCommands(uint32_t index);
// ----------------------------------------------------------------------------
void endSingleTimeCommands(VkCommandBuffer command_buffer, uint32_t index);
// ----------------------------------------------------------------------------
void waitIdle();
};   // GEVulkanCommandLoader

}

#endif
