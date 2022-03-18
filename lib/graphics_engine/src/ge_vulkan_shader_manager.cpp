#include "ge_vulkan_shader_manager.hpp"

#include "ge_main.hpp"
#include "ge_vulkan_driver.hpp"

#include <algorithm>
#include <sstream>
#include <string>
#include <stdexcept>

#include "IFileSystem.h"

namespace GE
{
// ============================================================================
GEVulkanDriver* g_vk = NULL;
irr::io::IFileSystem* g_file_system = NULL;

std::string g_predefines = "";
uint32_t g_sampler_size = 0;

VkShaderModule g_2d_render_vert = VK_NULL_HANDLE;
VkShaderModule g_2d_render_frag = VK_NULL_HANDLE;
// ============================================================================
void GEVulkanShaderManager::init(GEVulkanDriver* vk)
{
    g_vk = vk;
    g_file_system = vk->getFileSystem();

    VkPhysicalDeviceLimits limit = g_vk->getPhysicalDeviceProperties().limits;
    // According to https://vulkan.gpuinfo.org/displaydevicelimit.php?name=maxDescriptorSetSampledImages&platform=all
    // almost all users have at least 96
    if (limit.maxDescriptorSetSampledImages < 96)
        throw std::runtime_error("maxDescriptorSetSampledImages is too low");
    g_sampler_size = 96;

    std::ostringstream oss;
    oss << "#version 450\n";
    oss << "#define SAMPLER_SIZE " << g_sampler_size << "\n";
    g_predefines = oss.str();

    // 2D rendering shader
    g_2d_render_vert = loadShader(shaderc_vertex_shader, "2d_render.vert");
    g_2d_render_frag = loadShader(shaderc_fragment_shader, "2d_render.frag");
}   // init

// ----------------------------------------------------------------------------
void GEVulkanShaderManager::destroy()
{
    vkDestroyShaderModule(g_vk->getDevice(), g_2d_render_vert, NULL);
    vkDestroyShaderModule(g_vk->getDevice(), g_2d_render_frag, NULL);
}   // destroy

// ----------------------------------------------------------------------------
VkShaderModule GEVulkanShaderManager::loadShader(shaderc_shader_kind kind,
                                                 const std::string& name)
{
    std::string shader_fullpath = getShaderFolder() + name;
    irr::io::IReadFile* r =
        g_file_system->createAndOpenFile(shader_fullpath.c_str());
    if (!r)
    {
        throw std::runtime_error(std::string("File ") + shader_fullpath +
            " is missing");
    }

    std::string shader_data;
    shader_data.resize(r->getSize());
    int nb_read = 0;
    if ((nb_read = r->read(&shader_data[0], r->getSize())) != r->getSize())
    {
        r->drop();
        throw std::runtime_error(
            std::string("File ") + name + " failed to be read");
    }
    r->drop();
    shader_data = g_predefines + shader_data;

    shaderc::Compiler compiler;
    shaderc::SpvCompilationResult module = compiler.CompileGlslToSpv(
        shader_data, kind, shader_fullpath.c_str());
    if (module.GetCompilationStatus() != shaderc_compilation_status_success)
        throw std::runtime_error(module.GetErrorMessage());

    std::vector<uint32_t> shader_bytecode(module.cbegin(), module.cend());
    VkShaderModuleCreateInfo create_info = {};
    create_info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    create_info.pNext = NULL;
    create_info.codeSize = shader_bytecode.size() * sizeof(uint32_t);
    create_info.pCode = shader_bytecode.data();

    VkShaderModule shader_module;
    if (vkCreateShaderModule(g_vk->getDevice(), &create_info, NULL,
        &shader_module) != VK_SUCCESS)
    {
        throw std::runtime_error(
            std::string("vkCreateShaderModule failed for ") + name);
    }
    return shader_module;
}   // loadShader

}
