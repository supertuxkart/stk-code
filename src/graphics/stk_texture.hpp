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

#ifndef HEADER_STK_TEXTURE_HPP
#define HEADER_STK_TEXTURE_HPP

#include "graphics/gl_headers.hpp"
#include "utils/no_copy.hpp"

#include <string>
#include <ITexture.h>

using namespace irr;

struct TexConfig;

class STKTexture : public video::ITexture, NoCopy
{
private:
    core::dimension2d<u32> m_size, m_orig_size;

    bool m_single_channel;

    TexConfig* m_tex_config;

    GLuint m_texture_name;

    unsigned int m_texture_size;

    video::IImage* m_texture_image;

    // ------------------------------------------------------------------------
    video::IImage* resizeImage(video::IImage* orig_img,
                               core::dimension2du* orig_size = NULL,
                               core::dimension2du* final_size = NULL) const;
    // ------------------------------------------------------------------------
    void formatConversion(uint8_t* data, unsigned int* format, unsigned int w,
                          unsigned int h) const;
    // ------------------------------------------------------------------------
    bool isSrgb() const;
    // ------------------------------------------------------------------------
    bool isPremulAlpha() const;
    // ------------------------------------------------------------------------
    void applyMask(video::IImage* orig_img);
    // ------------------------------------------------------------------------

public:
    // ------------------------------------------------------------------------
    STKTexture(const std::string& path, TexConfig* tc, bool no_upload = false);
    // ------------------------------------------------------------------------
    STKTexture(uint8_t* data, const std::string& name, unsigned int size,
               bool single_channel = false);
    // ------------------------------------------------------------------------
    STKTexture(video::IImage* img, const std::string& name);
    // ------------------------------------------------------------------------
    virtual ~STKTexture();
    // ------------------------------------------------------------------------
    virtual void* lock(video::E_TEXTURE_LOCK_MODE mode =
                       video::ETLM_READ_WRITE, u32 mipmap_level = 0);
    // ------------------------------------------------------------------------
    virtual void unlock()
    {
        if (m_texture_image)
            m_texture_image->unlock();
    }
    // ------------------------------------------------------------------------
    virtual const core::dimension2d<u32>& getOriginalSize() const
                                                        { return m_orig_size; }
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
    virtual bool hasMipMaps() const;
    // ------------------------------------------------------------------------
    virtual void regenerateMipMapLevels(void* mipmap_data = NULL)            {}
    // ------------------------------------------------------------------------
    virtual u32 getOpenGLTextureName() const         { return m_texture_name; }
    // ------------------------------------------------------------------------
    virtual unsigned int getTextureSize() const      { return m_texture_size; }
    // ------------------------------------------------------------------------
    void reload(bool no_upload = false, uint8_t* preload_data = NULL,
                video::IImage* preload_img = NULL);
    // ------------------------------------------------------------------------
    video::IImage* getTextureImage()                { return m_texture_image; }
    // ------------------------------------------------------------------------
    bool isMeshTexture() const;

};   // STKTexture

#endif
