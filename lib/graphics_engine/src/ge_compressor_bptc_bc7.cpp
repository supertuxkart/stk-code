#include "ge_compressor_bptc_bc7.hpp"
#include "ge_main.hpp"
#include "ge_vulkan_features.hpp"

#ifdef BC7_ISPC
#include <bc7e_ispc.h>
#endif
#include <algorithm>
#include <cassert>

namespace GE
{
// ============================================================================
void GECompressorBPTCBC7::init()
{
#ifdef BC7_ISPC
    if (!GEVulkanFeatures::supportsBPTCBC7())
        return;
    ispc::bc7e_compress_block_init();
#endif
}   // init

// ----------------------------------------------------------------------------
GECompressorBPTCBC7::GECompressorBPTCBC7(uint8_t* texture, unsigned channels,
                                         const irr::core::dimension2d<irr::u32>& size,
                                         bool normal_map)
                   : GEMipmapGenerator(texture, channels, size, normal_map)
{
    m_compressed_data = NULL;
#ifdef BC7_ISPC
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

    ispc::bc7e_compress_block_params p = {};
    ispc::bc7e_compress_block_params_init_ultrafast(&p, true/*perceptual*/);
    std::vector<GEImageLevel> compressed_levels;
    m_compressed_data = new uint8_t[total_size];
    uint8_t* cur_offset = m_compressed_data;

    for (GEImageLevel& level : m_levels)
    {
        uint8_t* out = cur_offset;
        for (unsigned y = 0; y < level.m_dim.Height; y += 4)
        {
            for (unsigned x = 0; x < level.m_dim.Width; x += 4)
            {
                // build the 4x4 block of pixels
                uint32_t source_rgba[16] = {};
                uint8_t* target_pixel = (uint8_t*)source_rgba;
                for (unsigned py = 0; py < 4; py++)
                {
                    for (unsigned px = 0; px < 4; px++)
                    {
                        // get the source pixel in the image
                        unsigned sx = x + px;
                        unsigned sy = y + py;
                        // enable if we're in the image
                        if (sx < level.m_dim.Width && sy < level.m_dim.Height)
                        {
                            uint8_t* rgba = (uint8_t*)level.m_data;
                            const unsigned pitch = level.m_dim.Width * 4;
                            uint8_t* source_pixel = rgba + pitch * sy + 4 * sx;
                            memcpy(target_pixel, source_pixel, 4);
                        }
                        // advance to the next pixel
                        target_pixel += 4;
                    }
                }
                ispc::bc7e_compress_blocks(1, (uint64_t*)out, source_rgba, &p);
                out += 16;
            }
        }
        unsigned cur_size = get4x4CompressedTextureSize(level.m_dim.Width,
            level.m_dim.Height);
        compressed_levels.push_back({ level.m_dim, cur_size, cur_offset });
        cur_offset += cur_size;
    }
    freeMipmapCascade();
    std::swap(compressed_levels, m_levels);
#endif
}   // GECompressorBPTCBC7

}
