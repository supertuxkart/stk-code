#ifndef HEADER_GE_MAIN_HPP
#define HEADER_GE_MAIN_HPP

#include <IVideoDriver.h>
#include <matrix4.h>
#include <cstdint>
#include <string>

namespace GE
{
class GEVulkanDriver;
struct GEConfig
{
bool m_disable_npot_texture;
};

void setVideoDriver(irr::video::IVideoDriver* driver);
void setShaderFolder(const std::string& path);
irr::video::IVideoDriver* getDriver();
GE::GEVulkanDriver* getVKDriver();
const std::string& getShaderFolder();
GEConfig* getGEConfig();
void deinit();
uint64_t getMonoTimeMs();
void mathPlaneFrustumf(float* out, const irr::core::matrix4& pvm);
inline size_t getPadding(size_t in, size_t alignment)
{
    if (in == 0 || alignment == 0)
        return 0;
    size_t mod = in % alignment;
    if (mod == 0)
        return 0;
    else
        return alignment - mod;
}
}

#endif
