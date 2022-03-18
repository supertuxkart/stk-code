#include "ge_main.hpp"
#include "ge_vulkan_driver.hpp"

namespace GE
{
irr::video::IVideoDriver* g_driver = NULL;
GEConfig g_config = {};
std::string g_shader_folder = "";

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

}
