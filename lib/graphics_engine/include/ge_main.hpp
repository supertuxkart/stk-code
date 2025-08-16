#ifndef HEADER_GE_MAIN_HPP
#define HEADER_GE_MAIN_HPP

#include <IVideoDriver.h>
#include <matrix4.h>
#include <SColor.h>

#include <array>
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
class GEOcclusionCulling;
class GESPMBuffer;
class GEVulkanDriver;
enum GEAutoDeferredType : unsigned
{
    GADT_DISABLED = 0,
    GADT_SINGLE_PASS,
    GADT_DISPLACE
};
enum GEScreenSpaceReflectionType : unsigned
{
    GSSRT_DISABLED = 0,
    GSSRT_FAST,
    GSSRT_HIZ,
    GSSRT_HIZ100 = GSSRT_HIZ,
    GSSRT_HIZ200,
    GSSRT_HIZ400,
    GSSRT_COUNT,
};

struct GEConfig
{
bool m_disable_npot_texture;
bool m_convert_irrlicht_mesh;
bool m_texture_compression;
bool m_fullscreen_desktop;
bool m_enable_draw_call_cache;
bool m_pbr;
bool m_ibl;
GEAutoDeferredType m_auto_deferred_type;
GEScreenSpaceReflectionType m_screen_space_reflection_type;
bool m_force_deferred;
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
inline uint8_t srgb255ToLinear(unsigned color_srgb_255)
{
    static unsigned srgb_linear_map[256] =
    {
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        1, 1, 1, 1, 1, 1, 1, 1, 1, 2, 2, 2,
        2, 2, 2, 2, 3, 3, 3, 3, 3, 4, 4, 4,
        4, 4, 5, 5, 5, 5, 6, 6, 6, 6, 7, 7,
        7, 8, 8, 8, 9, 9, 9, 10, 10, 10, 11, 11,
        11, 12, 12, 13, 13, 13, 14, 14, 15, 15, 16, 16,
        16, 17, 17, 18, 18, 19, 19, 20, 20, 21, 22, 22,
        23, 23, 24, 24, 25, 26, 26, 27, 27, 28, 29, 29,
        30, 31, 31, 32, 33, 33, 34, 35, 36, 36, 37, 38,
        38, 39, 40, 41, 42, 42, 43, 44, 45, 46, 47, 47,
        48, 49, 50, 51, 52, 53, 54, 55, 55, 56, 57, 58,
        59, 60, 61, 62, 63, 64, 65, 66, 67, 68, 70, 71,
        72, 73, 74, 75, 76, 77, 78, 80, 81, 82, 83, 84,
        85, 87, 88, 89, 90, 92, 93, 94, 95, 97, 98, 99,
        101, 102, 103, 105, 106, 107, 109, 110, 112, 113, 114, 116,
        117, 119, 120, 122, 123, 125, 126, 128, 129, 131, 132, 134,
        135, 137, 139, 140, 142, 144, 145, 147, 148, 150, 152, 153,
        155, 157, 159, 160, 162, 164, 166, 167, 169, 171, 173, 175,
        176, 178, 180, 182, 184, 186, 188, 190, 192, 193, 195, 197,
        199, 201, 203, 205, 207, 209, 211, 213, 215, 218, 220, 222,
        224, 226, 228, 230, 232, 235, 237, 239, 241, 243, 245, 248,
        250, 252, 255
    };
    return uint8_t(srgb_linear_map[color_srgb_255]);
}
inline irr::video::SColor srgb255ToLinearFromSColor(irr::video::SColor scolor_srgb)
{
    irr::video::SColor out = scolor_srgb;
    out.setRed(srgb255ToLinear(scolor_srgb.getRed()));
    out.setGreen(srgb255ToLinear(scolor_srgb.getGreen()));
    out.setBlue(srgb255ToLinear(scolor_srgb.getBlue()));
    return out;
}
void copyToMappedBuffer(uint32_t* mapped, GESPMBuffer* spmb, size_t offset = 0);
GEOcclusionCulling* getOcclusionCulling();
void resetOcclusionCulling();
bool hasOcclusionCulling();
bool needsDeferredRendering(bool auto_deferred = true);
std::array<float, 4>& getDisplaceDirection();

}
#endif
