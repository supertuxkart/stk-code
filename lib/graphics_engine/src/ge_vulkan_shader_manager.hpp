#ifndef HEADER_GE_VULKAN_SHADER_MANAGER_HPP
#define HEADER_GE_VULKAN_SHADER_MANAGER_HPP

#include "vulkan_wrapper.h"
#include <string>
#include <shaderc/shaderc.hpp>

namespace GE
{
class GEVulkanDriver;
namespace GEVulkanShaderManager
{
// ----------------------------------------------------------------------------
void init(GEVulkanDriver*);
// ----------------------------------------------------------------------------
void destroy();
// ----------------------------------------------------------------------------
VkShaderModule loadShader(shaderc_shader_kind, const std::string&);
};   // GEVulkanShaderManager

}

#endif
