#include "ge_main.hpp"
#include "ge_vulkan_driver.hpp"

#include <chrono>

namespace GE
{
irr::video::IVideoDriver* g_driver = NULL;
GEConfig g_config = {};
std::string g_shader_folder = "";
std::chrono::steady_clock::time_point g_mono_start =
    std::chrono::steady_clock::now();

void setVideoDriver(irr::video::IVideoDriver* driver)
{
    g_driver = driver;
}

irr::video::IVideoDriver* getDriver()
{
    return g_driver;
}

GE::GEVulkanDriver* getVKDriver()
{
    return dynamic_cast<GE::GEVulkanDriver*>(g_driver);
}

GEConfig* getGEConfig()
{
    return &g_config;
}

void setShaderFolder(const std::string& path)
{
    g_shader_folder = path + "ge_shaders/";
}

const std::string& getShaderFolder()
{
    return g_shader_folder;
}

void deinit()
{
}

uint64_t getMonoTimeMs()
{
    auto duration = std::chrono::steady_clock::now() - g_mono_start;
    auto value =
        std::chrono::duration_cast<std::chrono::milliseconds>(duration);
    return value.count();
}

}
