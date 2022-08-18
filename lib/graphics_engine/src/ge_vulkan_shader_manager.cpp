#include "ge_vulkan_shader_manager.hpp"

#include "ge_main.hpp"
#include "ge_vulkan_driver.hpp"
#include "ge_vulkan_features.hpp"

#include <algorithm>
#include <map>
#include <sstream>
#include <string>
#include <stdexcept>

#include "IFileSystem.h"

namespace GE
{
namespace GEVulkanShaderManager
{
// ============================================================================
GEVulkanDriver* g_vk = NULL;
irr::io::IFileSystem* g_file_system = NULL;

std::string g_predefines = "";

// More when PBR is used later
uint32_t g_mesh_texture_layer = 2;

uint32_t g_sampler_size = 512;

std::map<std::string, VkShaderModule> g_shaders;
}   // GEVulkanShaderManager

// ============================================================================
shaderc_include_result* showError(const char* message)
{
    shaderc_include_result* err = new shaderc_include_result;
    err->source_name = "";
    err->source_name_length = 0;
    err->content = message;
    err->content_length = strlen(message);
    err->user_data = NULL;
    return err;
}   // showError

// ============================================================================
class FileIncluder : public shaderc::CompileOptions::IncluderInterface
{
private:
    std::vector<std::string> m_shader_fullpath;

    std::vector<std::string> m_shader_data;
public:
    // ------------------------------------------------------------------------
    shaderc_include_result* GetInclude(const char* requested_source,
                                       shaderc_include_type type,
                                       const char* requesting_source,
                                       size_t include_depth)
    {
        if (type != shaderc_include_type_relative)
            return showError("Only relateive included supported");

        std::string path = requesting_source;
        size_t pos = path.find_last_of('/');
        if (pos == std::string::npos)
            pos = path.find_last_of('\\');
        if (pos == std::string::npos)
            throw std::runtime_error(std::string("Invalid path: ") + path);

        m_shader_fullpath.push_back(std::string());
        std::string& shader_fullpath = m_shader_fullpath.back();
        shader_fullpath = path.substr(0, pos) + '/' + requested_source;
        irr::io::IReadFile* r = GEVulkanShaderManager::
            g_file_system->createAndOpenFile(shader_fullpath.c_str());
        if (!r)
        {
            throw std::runtime_error(std::string("File ") + shader_fullpath +
                " is missing");
        }

        m_shader_data.push_back(std::string());
        std::string& shader_data = m_shader_data.back();
        shader_data.resize(r->getSize());
        int nb_read = 0;
        if ((nb_read = r->read(&shader_data[0], r->getSize())) != r->getSize())
        {
            r->drop();
            throw std::runtime_error(std::string("File ") +
                requested_source + " failed to be read");
        }
        r->drop();

        shaderc_include_result* result = new shaderc_include_result;
        result->source_name = shader_fullpath.c_str();
        result->source_name_length = shader_fullpath.size();
        result->content = shader_data.c_str();
        result->content_length = shader_data.size();
        result->user_data = NULL;
        return result;
    }
    // ------------------------------------------------------------------------
    void ReleaseInclude(shaderc_include_result* include_result)
    {
        delete include_result;
    }
};   // FileIncluder

// ============================================================================
void GEVulkanShaderManager::init(GEVulkanDriver* vk)
{
    g_vk = vk;
    g_file_system = vk->getFileSystem();

    std::ostringstream oss;
    oss << "#version 450\n";
    oss << "#define SAMPLER_SIZE " << g_sampler_size << "\n";
    oss << "#define TOTAL_MESH_TEXTURE_LAYER " << g_mesh_texture_layer << "\n";
    if (GEVulkanFeatures::supportsBindTexturesAtOnce())
        oss << "#define BIND_TEXTURES_AT_ONCE\n";
    if (GEVulkanFeatures::supportsBindMeshTexturesAtOnce())
        oss << "#define BIND_MESH_TEXTURES_AT_ONCE\n";

    if (GEVulkanFeatures::supportsDifferentTexturePerDraw())
    {
        oss << "#extension GL_EXT_nonuniform_qualifier : enable\n";
        oss << "#define GE_SAMPLE_TEX_INDEX nonuniformEXT\n";
    }
    else
        oss << "#define GE_SAMPLE_TEX_INDEX int\n";
    g_predefines = oss.str();

    loadAllShaders();
}   // init

// ----------------------------------------------------------------------------
void GEVulkanShaderManager::destroy()
{
    if (!g_vk)
        return;
    for (auto& p : g_shaders)
        vkDestroyShaderModule(g_vk->getDevice(), p.second, NULL);
    g_shaders.clear();
}   // destroy

// ----------------------------------------------------------------------------
void GEVulkanShaderManager::loadAllShaders()
{
    irr::io::IFileList* files = g_file_system->createFileList(
        getShaderFolder().c_str());
    for (unsigned i = 0; i < files->getFileCount(); i++)
    {
        if (files->isDirectory(i))
            continue;
        std::string filename = files->getFileName(i).c_str();
        std::string ext = filename.substr(filename.find_last_of(".") + 1);
        shaderc_shader_kind kind;
        if (ext == "vert")
            kind = shaderc_vertex_shader;
        else if (ext == "frag")
            kind = shaderc_fragment_shader;
        else if (ext == "comp")
            kind = shaderc_compute_shader;
        else if (ext == "tesc")
            kind = shaderc_tess_control_shader;
        else if (ext == "tese")
            kind = shaderc_tess_evaluation_shader;
        else
            continue;
        g_shaders[filename] = loadShader(kind, filename);
    }
    files->drop();
}   // loadAllShaders

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
    FileIncluder* fi = new FileIncluder;
    shaderc::CompileOptions options;
    options.SetIncluder(std::unique_ptr<
        shaderc::CompileOptions::IncluderInterface>(fi));

    shaderc::SpvCompilationResult module = compiler.CompileGlslToSpv(
        shader_data, kind, shader_fullpath.c_str(), options);
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

// ----------------------------------------------------------------------------
unsigned GEVulkanShaderManager::getSamplerSize()
{
    return g_sampler_size;
}   // getSamplerSize

// ----------------------------------------------------------------------------
unsigned GEVulkanShaderManager::getMeshTextureLayer()
{
    return g_mesh_texture_layer;
}   // getMeshTextureLayer

// ----------------------------------------------------------------------------
VkShaderModule GEVulkanShaderManager::getShader(const std::string& filename)
{
    return g_shaders.at(filename);
}   // getShader

}
