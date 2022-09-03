#include "ge_main.hpp"
#include "ge_vulkan_driver.hpp"

#include <chrono>

namespace GE
{
irr::video::IVideoDriver* g_driver = NULL;
GEConfig g_config =
{
    false,
    false,
    false,
    true,
    false,
    {},
    1.0f
};
std::string g_shader_folder = "";
std::chrono::steady_clock::time_point g_mono_start =
    std::chrono::steady_clock::now();

void setVideoDriver(irr::video::IVideoDriver* driver)
{
    if (driver != g_driver)
    {
        // Reset everytime driver is recreated
        g_config.m_ondemand_load_texture_paths.clear();
        g_driver = driver;
    }
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

void mathPlaneNormf(float *p)
{
    float f = 1.0f / sqrtf(p[0] * p[0] + p[1] * p[1] + p[2] * p[2]);
    p[0] *= f;
    p[1] *= f;
    p[2] *= f;
    p[3] *= f;
}

void mathPlaneFrustumf(float* out, const irr::core::matrix4& pvm)
{
    // return 6 planes, 24 floats
    const float* m = pvm.pointer();

    // near
    out[0] = m[3] + m[2];
    out[1] = m[7] + m[6];
    out[2] = m[11] + m[10];
    out[3] = m[15] + m[14];
    mathPlaneNormf(&out[0]);

    // right
    out[4] = m[3] - m[0];
    out[4 + 1] = m[7] - m[4];
    out[4 + 2] = m[11] - m[8];
    out[4 + 3] = m[15] - m[12];
    mathPlaneNormf(&out[4]);

    // left
    out[2 * 4] = m[3] + m[0];
    out[2 * 4 + 1] = m[7] + m[4];
    out[2 * 4 + 2] = m[11] + m[8];
    out[2 * 4 + 3] = m[15] + m[12];
    mathPlaneNormf(&out[2 * 4]);

    // bottom
    out[3 * 4] = m[3] + m[1];
    out[3 * 4 + 1] = m[7] + m[5];
    out[3 * 4 + 2] = m[11] + m[9];
    out[3 * 4 + 3] = m[15] + m[13];
    mathPlaneNormf(&out[3 * 4]);

    // top
    out[4 * 4] = m[3] - m[1];
    out[4 * 4 + 1] = m[7] - m[5];
    out[4 * 4 + 2] = m[11] - m[9];
    out[4 * 4 + 3] = m[15] - m[13];
    mathPlaneNormf(&out[4 * 4]);

    // far
    out[5 * 4] = m[3] - m[2];
    out[5 * 4 + 1] = m[7] - m[6];
    out[5 * 4 + 2] = m[11] - m[10];
    out[5 * 4 + 3] = m[15] - m[14];
    mathPlaneNormf(&out[5 * 4]);
}

}
