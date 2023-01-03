#ifndef HEADER_GE_VULKAN_SHADER_MANAGER_HPP
#define HEADER_GE_VULKAN_SHADER_MANAGER_HPP

#include "vulkan_wrapper.h"
#include <string>
#ifdef DISABLE_SHADERC
  #define shaderc_shader_kind int
#else
  #include <shaderc/shaderc.h>
#endif

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
void loadAllShaders();
// ----------------------------------------------------------------------------
VkShaderModule getShader(const std::string& filename);
// ----------------------------------------------------------------------------
VkShaderModule loadShader(shaderc_shader_kind, const std::string&);
// ----------------------------------------------------------------------------
unsigned getSamplerSize();
// ----------------------------------------------------------------------------
unsigned getMeshTextureLayer();
};   // GEVulkanShaderManager

}

#endif
