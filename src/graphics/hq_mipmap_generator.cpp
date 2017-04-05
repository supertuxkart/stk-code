//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2017 SuperTuxKart-Team
//
//  This program is free software; you can redistribute it and/or
//  modify it under the terms of the GNU General Public License
//  as published by the Free Software Foundation; either version 3
//  of the License, or (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

#if !(defined(SERVER_ONLY) || defined(USE_GLES2))

#include "graphics/hq_mipmap_generator.hpp"
#include "graphics/stk_tex_manager.hpp"
#undef DUMP_MIPMAP
#ifdef DUMP_MIPMAP
#include "graphics/irr_driver.hpp"
#include "utils/string_utils.hpp"
#endif
#include <cassert>

extern "C"
{
    #include <mipmap/img.h>
    #include <mipmap/imgresize.h>
}

// ----------------------------------------------------------------------------
HQMipmapGenerator::HQMipmapGenerator(const io::path& name, uint8_t* data,
                                     const core::dimension2d<u32>& size,
                                     GLuint texture_name, TexConfig* tc)
                 : video::ITexture(name), m_orig_data(data), m_size(size),
                   m_texture_name(texture_name), m_texture_size(0),
                   m_mipmap_data(NULL), m_tex_config(tc)
{
    assert(m_tex_config != NULL);
    unsigned width = m_size.Width;
    unsigned height = m_size.Height;
    while (true)
    {
        width = width < 2 ? 1 : width >> 1;
        height = height < 2 ? 1 : height >> 1;
        m_mipmap_sizes.emplace_back(core::dimension2du(width, height),
            m_texture_size);
        m_texture_size += width * height * 4;
        if (width == 1 && height == 1)
            break;
    }
    m_texture_size = unsigned(m_mipmap_sizes.back().second) + 4;
    m_mipmap_data = malloc(sizeof(imMipmapCascade));
}   // HQMipmapGenerator

// ----------------------------------------------------------------------------
void HQMipmapGenerator::threadedReload(void* ptr, void* param) const
{
    imReduceOptions options;
    imReduceSetOptions(&options,
        m_tex_config->m_normal_map ?
        IM_REDUCE_FILTER_NORMALMAP: m_tex_config->m_srgb ?
        IM_REDUCE_FILTER_SRGB : IM_REDUCE_FILTER_LINEAR/*filter*/,
        2/*hopcount*/, 2.0f/*alpha*/, 1.0f/*amplifynormal*/,
        0.0f/*normalsustainfactor*/);
    imMipmapCascade* mm_cascade = (imMipmapCascade*)m_mipmap_data;
#ifdef DEBUG
    int ret = imBuildMipmapCascade(mm_cascade, m_orig_data, m_size.Width,
        m_size.Height, 1/*layercount*/, 4, m_size.Width * 4, &options, 0);
    assert(ret == 1);
#else
    imBuildMipmapCascade(mm_cascade, m_orig_data, m_size.Width,
        m_size.Height, 1/*layercount*/, 4, m_size.Width * 4, &options, 0);
#endif
    for (unsigned int i = 0; i < m_mipmap_sizes.size(); i++)
    {
        const unsigned size = m_mipmap_sizes[i].first.getArea() *  4;
        memcpy((uint8_t*)ptr + m_mipmap_sizes[i].second,
            mm_cascade->mipmap[i + 1], size);
#ifdef DUMP_MIPMAP
        video::IImage* image = irr_driver->getVideoDriver()
            ->createImageFromData(video::ECF_A8R8G8B8, m_mipmap_sizes[i].first,
            mm_cascade->mipmap[i + 1], false/*ownForeignMemory*/);
        irr_driver->getVideoDriver()->writeImageToFile(image, std::string
            (StringUtils::toString(i) + "_" +
            StringUtils::getBasename(NamedPath.getPtr())).c_str());
        image->drop();
#endif
    }
}   // threadedReload

// ----------------------------------------------------------------------------
void HQMipmapGenerator::threadedSubImage(void* ptr) const
{
#if !(defined(SERVER_ONLY) || defined(USE_GLES2))
    glBindTexture(GL_TEXTURE_2D, m_texture_name);
    for (unsigned int i = 0; i < m_mipmap_sizes.size(); i++)
    {
        glTexSubImage2D(GL_TEXTURE_2D, i + 1, 0, 0,
            m_mipmap_sizes[i].first.Width, m_mipmap_sizes[i].first.Height,
            GL_BGRA, GL_UNSIGNED_BYTE,
            (uint8_t*)ptr + m_mipmap_sizes[i].second);
    }
    delete this;
#endif
}   // threadedSubImage

// ----------------------------------------------------------------------------
void HQMipmapGenerator::cleanThreadedLoader()
{
    delete[] m_orig_data;
    imFreeMipmapCascade((imMipmapCascade*)m_mipmap_data);
    free(m_mipmap_data);
}   // cleanThreadedLoader

#endif
