#ifndef HEADER_GE_MIPMAP_GENERATOR_HPP
#define HEADER_GE_MIPMAP_GENERATOR_HPP

extern "C"
{
    #include <mipmap/img.h>
    #include <mipmap/imgresize.h>
}
#include <cstdint>
#include <memory>
#include <vector>

#include "dimension2d.h"

namespace GE
{
struct GEImageLevel
{
    irr::core::dimension2du m_dim;
    unsigned m_size;
    void* m_data;
};

class GEMipmapGenerator
{
private:
    imMipmapCascade* m_cascade;

protected:
    unsigned m_mipmap_sizes;

    std::vector<GEImageLevel> m_levels;

    // ------------------------------------------------------------------------
    void freeMipmapCascade()
    {
        if (m_cascade)
        {
            imFreeMipmapCascade(m_cascade);
            delete m_cascade;
            m_cascade = NULL;
        }
    }
public:
    // ------------------------------------------------------------------------
    GEMipmapGenerator(uint8_t* texture, unsigned channels,
                      const irr::core::dimension2d<irr::u32>& size,
                      bool normal_map)
    {
        m_cascade = new imMipmapCascade();
        unsigned width = size.Width;
        unsigned height = size.Height;
        m_levels.push_back({ size, width * height * channels, texture });

        m_mipmap_sizes = 0;
        while (true)
        {
            width = width < 2 ? 1 : width >> 1;
            height = height < 2 ? 1 : height >> 1;
            const unsigned cur_mipmap_size = width * height * channels;
            m_levels.push_back({ irr::core::dimension2du(width, height),
                cur_mipmap_size, NULL });
            m_mipmap_sizes += cur_mipmap_size;
            if (width == 1 && height == 1)
            {
                break;
            }
        }

        imReduceOptions options;
        imReduceSetOptions(&options, normal_map ?
            IM_REDUCE_FILTER_NORMALMAP: IM_REDUCE_FILTER_LINEAR/*filter*/,
            2/*hopcount*/, 2.0f/*alpha*/, 1.0f/*amplifynormal*/,
            0.0f/*normalsustainfactor*/);

#ifdef DEBUG
        int ret = imBuildMipmapCascade(m_cascade, texture,
            m_levels[0].m_dim.Width, m_levels[0].m_dim.Height, 1/*layercount*/,
            channels, m_levels[0].m_dim.Width * channels, &options, 0);
        if (ret != 1)
            throw std::runtime_error("imBuildMipmapCascade failed");
#else
        imBuildMipmapCascade(m_cascade, texture, m_levels[0].m_dim.Width,
            m_levels[0].m_dim.Height, 1/*layercount*/, channels,
            m_levels[0].m_dim.Width * channels, &options, 0);
#endif
        for (unsigned int i = 1; i < m_levels.size(); i++)
            m_levels[i].m_data = m_cascade->mipmap[i];
    }
    // ------------------------------------------------------------------------
    virtual ~GEMipmapGenerator()                       { freeMipmapCascade(); }
    // ------------------------------------------------------------------------
    unsigned getMipmapSizes() const                  { return m_mipmap_sizes; }
    // ------------------------------------------------------------------------
    std::vector<GEImageLevel>& getAllLevels()              { return m_levels; }
};   // GEMipmapGenerator

}

#endif
