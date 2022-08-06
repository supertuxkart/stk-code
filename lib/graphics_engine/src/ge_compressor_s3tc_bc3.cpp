#include "ge_compressor_s3tc_bc3.hpp"
#include "ge_main.hpp"

#include <algorithm>
#include <cassert>

#include <squish.h>
static_assert(squish::kColourClusterFit == (1 << 5), "Wrong header");
static_assert(squish::kColourRangeFit == (1 << 6), "Wrong header");
static_assert(squish::kColourIterativeClusterFit == (1 << 8), "Wrong header");

// ============================================================================
extern "C" void squishCompressImage(uint8_t* rgba, int width, int height,
                                    int pitch, void* blocks, unsigned flags)
{
    // This function is copied from CompressImage in libsquish to avoid omp
    // if enabled by shared libsquish, because we are already using
    // multiple thread
    for (int y = 0; y < height; y += 4)
    {
        // initialise the block output
        uint8_t* target_block = reinterpret_cast<uint8_t*>(blocks);
        target_block += ((y >> 2) * ((width + 3) >> 2)) * 16;
        for (int x = 0; x < width; x += 4)
        {
            // build the 4x4 block of pixels
            uint8_t source_rgba[16 * 4];
            uint8_t* target_pixel = source_rgba;
            int mask = 0;
            for (int py = 0; py < 4; py++)
            {
                for (int px = 0; px < 4; px++)
                {
                    // get the source pixel in the image
                    int sx = x + px;
                    int sy = y + py;
                    // enable if we're in the image
                    if (sx < width && sy < height)
                    {
                        // copy the rgba value
                        uint8_t* source_pixel = rgba + pitch * sy + 4 * sx;
                        memcpy(target_pixel, source_pixel, 4);
                        // enable this pixel
                        mask |= (1 << (4 * py + px));
                    }
                    // advance to the next pixel
                    target_pixel += 4;
                }
            }
            // compress it into the output
            squish::CompressMasked(source_rgba, mask, target_block, flags);
            // advance
            target_block += 16;
        }
    }
}   // squishCompressImage

namespace GE
{
// ----------------------------------------------------------------------------
GECompressorS3TCBC3::GECompressorS3TCBC3(uint8_t* texture, unsigned channels,
                                         const irr::core::dimension2d<irr::u32>& size,
                                         bool normal_map)
                   : GEMipmapGenerator(texture, channels, size, normal_map)
{
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
    const unsigned tc_flag = squish::kDxt5 | squish::kColourRangeFit;

    for (GEImageLevel& level : m_levels)
    {
        squishCompressImage((uint8_t*)level.m_data, level.m_dim.Width,
            level.m_dim.Height, level.m_dim.Width * channels,
            cur_offset, tc_flag);
        unsigned cur_size = get4x4CompressedTextureSize(level.m_dim.Width,
            level.m_dim.Height);
        compressed_levels.push_back({ level.m_dim, cur_size, cur_offset });
        cur_offset += cur_size;
    }
    freeMipmapCascade();
    std::swap(compressed_levels, m_levels);
}   // GECompressorS3TCBC3

}
