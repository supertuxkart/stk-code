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

#ifndef HEADER_HQ_MIPMAP_GENERATOR_HPP
#define HEADER_HQ_MIPMAP_GENERATOR_HPP

#if !(defined(SERVER_ONLY) || defined(USE_GLES2))

#include "graphics/gl_headers.hpp"
#include "utils/no_copy.hpp"
#include "utils/types.hpp"

#include <vector>
#include <ITexture.h>

using namespace irr;
struct TexConfig;

class HQMipmapGenerator : public video::ITexture, NoCopy
{
private:
    uint8_t* m_orig_data;

    core::dimension2d<u32> m_size;

    GLuint m_texture_name;

    unsigned int m_texture_size;

    void* m_mipmap_data;

    TexConfig* m_tex_config;

    std::vector<std::pair<core::dimension2d<u32>, size_t> > m_mipmap_sizes;

public:
    // ------------------------------------------------------------------------
    HQMipmapGenerator(const io::path& name, uint8_t* data,
                      const core::dimension2d<u32>& size, GLuint texture_name,
                      TexConfig* tc);
    // ------------------------------------------------------------------------
    virtual ~HQMipmapGenerator() {}
    // ------------------------------------------------------------------------
    virtual void* lock(video::E_TEXTURE_LOCK_MODE mode =
                       video::ETLM_READ_WRITE, u32 mipmap_level = 0)
                                                               { return NULL; }
    // ------------------------------------------------------------------------
    virtual void unlock()                                                    {}
    // ------------------------------------------------------------------------
    virtual const core::dimension2d<u32>& getOriginalSize() const
                                                             { return m_size; }
    // ------------------------------------------------------------------------
    virtual const core::dimension2d<u32>& getSize() const    { return m_size; }
    // ------------------------------------------------------------------------
    virtual video::E_DRIVER_TYPE getDriverType() const
    {
#if defined(USE_GLES2)
        return video::EDT_OGLES2;
#else
        return video::EDT_OPENGL;
#endif
    }
    // ------------------------------------------------------------------------
    virtual video::ECOLOR_FORMAT getColorFormat() const
                                                { return video::ECF_A8R8G8B8; }
    // ------------------------------------------------------------------------
    virtual u32 getPitch() const                                  { return 0; }
    // ------------------------------------------------------------------------
    virtual bool hasMipMaps() const                           { return false; }
    // ------------------------------------------------------------------------
    virtual void regenerateMipMapLevels(void* mipmap_data = NULL)            {}
    // ------------------------------------------------------------------------
    virtual u32 getOpenGLTextureName() const         { return m_texture_name; }
    // ------------------------------------------------------------------------
    virtual u64 getHandle()                                       { return 0; }
    // ------------------------------------------------------------------------
    virtual unsigned int getTextureSize() const      { return m_texture_size; }
    // ------------------------------------------------------------------------
    virtual void threadedReload(void* ptr, void* param) const;
    // ------------------------------------------------------------------------
    virtual void threadedSubImage(void* ptr) const;
    // ------------------------------------------------------------------------
    virtual void cleanThreadedLoader();

};   // HQMipmapGenerator

#endif

#endif
