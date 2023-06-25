#include "ge_compressor_astc_4x4.hpp"

#include "ge_main.hpp"
#include "ge_vulkan_command_loader.hpp"
#include "ge_vulkan_features.hpp"

#include <algorithm>
#include <cassert>
#include <cstdio>
#include <vector>

#ifdef ENABLE_LIBASTCENC
#include <astcenc.h>
#include <SDL_cpuinfo.h>
#endif

namespace GE
{
// ============================================================================
#ifdef ENABLE_LIBASTCENC
namespace GEVulkanFeatures
{
extern bool g_supports_astc_4x4;
}

std::vector<astcenc_context*> g_astc_contexts;
#endif
// ============================================================================
void GECompressorASTC4x4::init()
{
#ifdef ENABLE_LIBASTCENC
    if (!GEVulkanFeatures::g_supports_astc_4x4)
        return;

    // Check for neon existence because libastcenc doesn't do that
#if defined(__arm__) || defined(__aarch64__) || defined(_M_ARM) || defined (_M_ARM64)
    if (SDL_HasNEON() == SDL_FALSE)
        return;
#else
    if (SDL_HasSSE41() == SDL_FALSE)
        return;
#endif

    astcenc_config cfg = {};
    float quality = ASTCENC_PRE_FASTEST;
    if (astcenc_config_init(ASTCENC_PRF_LDR, 4, 4, 1, quality, 0, &cfg) !=
        ASTCENC_SUCCESS)
        return;

    for (unsigned i = 0; i < GEVulkanCommandLoader::getLoaderCount(); i++)
    {
        astcenc_context* context = NULL;
        if (astcenc_context_alloc(&cfg, 1, &context) != ASTCENC_SUCCESS)
        {
            destroy();
            return;
        }
        g_astc_contexts.push_back(context);
    }
#endif
}   // init

// ============================================================================
void GECompressorASTC4x4::destroy()
{
#ifdef ENABLE_LIBASTCENC
    for (astcenc_context* context : g_astc_contexts)
        astcenc_context_free(context);
    g_astc_contexts.clear();
#endif
}   // destroy

// ============================================================================
bool GECompressorASTC4x4::loaded()
{
#ifdef ENABLE_LIBASTCENC
    return !g_astc_contexts.empty();
#else
    return false;
#endif
}   // loaded

// ----------------------------------------------------------------------------
GECompressorASTC4x4::GECompressorASTC4x4(uint8_t* texture, unsigned channels,
                                         const irr::core::dimension2d<irr::u32>& size,
                                         bool normal_map)
                   : GEMipmapGenerator(texture, channels, size, normal_map)
{
    m_compressed_data = NULL;
#ifdef ENABLE_LIBASTCENC
    assert(channels == 4);
    size_t total_size = 0;
    m_mipmap_sizes = 0;
    for (unsigned i = 0; i < m_levels.size(); i++)
    {
        GEImageLevel& level = m_levels[i];
        unsigned cur_size = get4x4CompressedTextureSize(level.m_dim.Width,
            level.m_dim.Height);
        total_size += cur_size;
        if (i > 0)
            m_mipmap_sizes += cur_size;
    }

    std::vector<GEImageLevel> compressed_levels;
    m_compressed_data = new uint8_t[total_size];
    uint8_t* cur_offset = m_compressed_data;

    for (GEImageLevel& level : m_levels)
    {
        astcenc_image img;
        img.dim_x = level.m_dim.Width;
        img.dim_y = level.m_dim.Height;
        img.dim_z = 1;
        img.data_type = ASTCENC_TYPE_U8;
        img.data = &level.m_data;

        astcenc_swizzle swizzle;
        swizzle.r = ASTCENC_SWZ_R;
        swizzle.g = ASTCENC_SWZ_G;
        swizzle.b = ASTCENC_SWZ_B;
        swizzle.a = ASTCENC_SWZ_A;

        unsigned cur_size = get4x4CompressedTextureSize(level.m_dim.Width,
            level.m_dim.Height);
        if (astcenc_compress_image(
            g_astc_contexts[GEVulkanCommandLoader::getLoaderId()], &img,
            &swizzle, cur_offset, cur_size, 0) != ASTCENC_SUCCESS)
            printf("astcenc_compress_image failed!\n");
        compressed_levels.push_back({ level.m_dim, cur_size, cur_offset });
        cur_offset += cur_size;
    }
    freeMipmapCascade();
    std::swap(compressed_levels, m_levels);
#endif
}   // GECompressorASTC4x4

}
