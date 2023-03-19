#ifndef HEADER_GE_MAIN_HPP
#define HEADER_GE_MAIN_HPP

#include <IVideoDriver.h>
#include <matrix4.h>

#include <cstdint>
#include <string>
#include <unordered_set>

namespace irr
{
    namespace scene
    {
        class IMesh; class IAnimatedMesh;
    }
}

namespace GE
{
class GEVulkanDriver;
struct GEConfig
{
bool m_disable_npot_texture;
bool m_convert_irrlicht_mesh;
bool m_texture_compression;
bool m_fullscreen_desktop;
bool m_enable_draw_call_cache;
bool m_pbr;
std::unordered_set<std::string> m_ondemand_load_texture_paths;
float m_render_scale;
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
inline int get4x4CompressedTextureSize(int width, int height)
{
    int blockcount = ((width + 3) / 4) * ((height + 3) / 4);
    int blocksize = 4 * 4;
    return blockcount * blocksize;
}
irr::scene::IAnimatedMesh* convertIrrlichtMeshToSPM(irr::scene::IMesh* mesh);

}
#endif
