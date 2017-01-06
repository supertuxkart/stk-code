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

class Material;

class STKTexture : public video::ITexture, NoCopy
{
private:
    core::dimension2d<u32> m_size, m_orig_size;

    uint64_t m_texture_handle;

    bool m_srgb, m_premul_alpha, m_mesh_texture;

    Material* m_material;

    GLuint m_texture_name;

    unsigned int m_texture_size;

    video::IImage* m_texture_image;

    // ------------------------------------------------------------------------
    video::IImage* resizeImage(video::IImage* orig_img,
                               core::dimension2du* new_img_size = NULL,
                               core::dimension2du* new_tex_size = NULL);
    // ------------------------------------------------------------------------
    void applyMask(video::IImage* orig_img);
    // ------------------------------------------------------------------------
    bool loadCompressedTexture(const std::string& file_name);
    // ------------------------------------------------------------------------
    void saveCompressedTexture(const std::string& file_name);
    // ------------------------------------------------------------------------
    std::string getHashedName(const std::string& orig_file);

public:
    // ------------------------------------------------------------------------
    STKTexture(const std::string& path, bool srgb = false,
               bool premul_alpha = false, bool set_material = false,
               bool mesh_tex = false, bool no_upload = false);
    // ------------------------------------------------------------------------
    STKTexture(video::IImage* image, const std::string& name);
    // ------------------------------------------------------------------------
    virtual ~STKTexture();
    // ------------------------------------------------------------------------
    virtual void* lock(video::E_TEXTURE_LOCK_MODE mode =
                       video::ETLM_READ_WRITE, u32 mipmap_level = 0)
    {
        if (m_texture_image)
            return m_texture_image->lock();
        return NULL;
    }
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
    uint64_t getTextureHandle() const              { return m_texture_handle; }
    // ------------------------------------------------------------------------
    bool isSrgb() const                                      { return m_srgb; }
    // ------------------------------------------------------------------------
    bool isPremulAlpha() const                       { return m_premul_alpha; }
    // ------------------------------------------------------------------------
    bool isMeshTexture() const                       { return m_mesh_texture; }
    // ------------------------------------------------------------------------
    void setMeshTexture(bool val)                     { m_mesh_texture = val; }
    // ------------------------------------------------------------------------
    unsigned int getTextureSize() const              { return m_texture_size; }
    // ------------------------------------------------------------------------
    void reload(bool no_upload = false, video::IImage* pre_loaded_tex = NULL);
    // ------------------------------------------------------------------------
    video::IImage* getTextureImage()                { return m_texture_image; }

};   // STKTexture

#endif
