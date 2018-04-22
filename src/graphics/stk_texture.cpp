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

#include "graphics/stk_texture.hpp"
#include "config/user_config.hpp"
#include "graphics/central_settings.hpp"
#include "graphics/irr_driver.hpp"
#include "graphics/graphics_restrictions.hpp"
#include "graphics/stk_tex_manager.hpp"
#include "graphics/material.hpp"
#include "graphics/material_manager.hpp"
#include "modes/profile_world.hpp"
#include "utils/log.hpp"
#include "utils/string_utils.hpp"

// ----------------------------------------------------------------------------
STKTexture::STKTexture(const std::string& path, TexConfig* tc, bool no_upload)
          : video::ITexture(path.c_str()), m_single_channel(false),
            m_tex_config(NULL), m_texture_name(0), m_texture_size(0),
            m_texture_image(NULL)
{
    if (tc != NULL)
    {
        m_tex_config = (TexConfig*)malloc(sizeof(TexConfig));
        memcpy(m_tex_config, tc, sizeof(TexConfig));
    }
#ifndef SERVER_ONLY
    if (m_tex_config)
    {
        if (ProfileWorld::isNoGraphics() ||
            (!CVS->isDeferredEnabled()) || !CVS->isGLSL())
        {
            m_tex_config->m_srgb = false;
        }
    }
    if (!CVS->isARBTextureSwizzleUsable())
        m_single_channel = false;
#endif
    reload(no_upload);
}   // STKTexture

// ----------------------------------------------------------------------------
STKTexture::STKTexture(uint8_t* data, const std::string& name, unsigned int size,
                       bool single_channel)
          : video::ITexture(name.c_str()), m_single_channel(single_channel),
            m_tex_config(NULL), m_texture_name(0), m_texture_size(0),
            m_texture_image(NULL)
{
    m_size.Width = size;
    m_size.Height = size;
    m_orig_size = m_size;
    reload(false/*no_upload*/, data);
}   // STKTexture

// ----------------------------------------------------------------------------
STKTexture::STKTexture(video::IImage* img, const std::string& name)
          : video::ITexture(name.c_str()), m_single_channel(false),
            m_tex_config(NULL), m_texture_name(0), m_texture_size(0),
            m_texture_image(NULL)
{
    reload(false/*no_upload*/, NULL/*preload_data*/, img);
}   // STKTexture

// ----------------------------------------------------------------------------
STKTexture::~STKTexture()
{
#ifndef SERVER_ONLY
    if (m_texture_name != 0 && !ProfileWorld::isNoGraphics())
    {
        glDeleteTextures(1, &m_texture_name);
    }
#endif   // !SERVER_ONLY
    if (m_texture_image != NULL)
        m_texture_image->drop();
    free(m_tex_config);
}   // ~STKTexture

// ----------------------------------------------------------------------------
void STKTexture::reload(bool no_upload, uint8_t* preload_data,
                        video::IImage* preload_img)
{
    if (ProfileWorld::isNoGraphics())
    {
        m_orig_size.Width = 2;
        m_orig_size.Height = 2;
        m_size = m_orig_size;
        m_texture_name = 1;
        if (preload_data)
            delete[] preload_data;
        if (preload_img)
            preload_img->drop();
        return;
    }
#ifndef SERVER_ONLY

    video::IImage* orig_img = NULL;
    uint8_t* data = preload_data;
    if (data == NULL)
    {
        orig_img = preload_img ? preload_img :
            irr_driver->getVideoDriver()->createImageFromFile(NamedPath);
        if (orig_img == NULL)
        {
            return;
        }

        if (orig_img->getDimension().Width  == 0 ||
            orig_img->getDimension().Height == 0)
        {
            orig_img->drop();
            return;
        }
        orig_img = resizeImage(orig_img, &m_orig_size, &m_size);
        applyMask(orig_img);
        data = orig_img ? (uint8_t*)orig_img->lock() : NULL;
    }

    const unsigned int w = m_size.Width;
    const unsigned int h = m_size.Height;
    unsigned int format = m_single_channel ? GL_RED : GL_BGRA;
    unsigned int internal_format = m_single_channel ? GL_R8 : isSrgb() ?
        GL_SRGB8_ALPHA8 : GL_RGBA8;

    // GLES 2.0 specs doesn't allow GL_RGBA8 internal format
#if defined(USE_GLES2)
    if (!CVS->isGLSL())
    {
        internal_format = GL_RGBA;
    }
#endif

    formatConversion(data, &format, w, h);

    if (!no_upload)
    {
        const bool reload = m_texture_name != 0;
        if (!reload)
            glGenTextures(1, &m_texture_name);

        glBindTexture(GL_TEXTURE_2D, m_texture_name);
        if (!reload)
        {
            if (m_single_channel)
            {
                glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_R, GL_ONE);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_G, GL_ONE);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_B, GL_ONE);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_A, GL_RED);
            }
            glTexImage2D(GL_TEXTURE_2D, 0, internal_format, w, h, 0, format,
                GL_UNSIGNED_BYTE, data);
        }
        else
        {
            glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, w, h, format,
                GL_UNSIGNED_BYTE, data);
        }
        if (orig_img)
            orig_img->unlock();
        if (hasMipMaps())
            glGenerateMipmap(GL_TEXTURE_2D);
    }

    m_texture_size = w * h * (m_single_channel ? 1 : 4);
    if (no_upload)
        m_texture_image = orig_img;
    else if (orig_img)
        orig_img->drop();
    else
        delete[] data;

    if (!no_upload)
        glBindTexture(GL_TEXTURE_2D, 0);

#endif   // !SERVER_ONLY
}   // reload

//-----------------------------------------------------------------------------
void STKTexture::formatConversion(uint8_t* data, unsigned int* format,
                                  unsigned int w, unsigned int h) const
{
#ifndef SERVER_ONLY
#if defined(USE_GLES2)
    if (!m_single_channel)
    {
        if (format)
            *format = GL_RGBA;
        for (unsigned int i = 0; i < w * h; i++)
        {
            uint8_t tmp_val = data[i * 4];
            data[i * 4] = data[i * 4 + 2];
            data[i * 4 + 2] = tmp_val;
        }
    }
#endif
    if (isPremulAlpha() && !m_single_channel)
    {
        for (unsigned int i = 0; i < w * h; i++)
        {
            float alpha = data[4 * i + 3];
            if (alpha > 0.0f)
            {
                alpha /= 255.0f;

                if (CVS->isDeferredEnabled())
                {
                    alpha = pow(alpha, 1.0f / 2.2f);
                }
            }
            data[i * 4] = (uint8_t)(data[i * 4] * alpha);
            data[i * 4 + 1] = (uint8_t)(data[i * 4 + 1] * alpha);
            data[i * 4 + 2] = (uint8_t)(data[i * 4 + 2] * alpha);
        }
    }
#endif   // !SERVER_ONLY
}   // formatConversion

// ----------------------------------------------------------------------------
video::IImage* STKTexture::resizeImage(video::IImage* orig_img,
                                       core::dimension2du* orig_size,
                                       core::dimension2du* final_size) const
{
    video::IImage* image = orig_img;
#ifndef SERVER_ONLY
    if (image == NULL)
        assert(orig_size && orig_size->Width > 0 && orig_size->Height > 0);
        
    video::IVideoDriver* driver = irr_driver->getVideoDriver();
        
    core::dimension2du img_size = image ? image->getDimension() : *orig_size;
    
    bool has_npot = !GraphicsRestrictions::isDisabled(
                                GraphicsRestrictions::GR_NPOT_TEXTURES) && 
                                driver->queryFeature(video::EVDF_TEXTURE_NPOT);
                                
    core::dimension2du tex_size = img_size.getOptimalSize(!has_npot);
    const core::dimension2du& max_size = driver->getDriverAttributes().
                                  getAttributeAsDimension2d("MAX_TEXTURE_SIZE");

    if (tex_size.Width > max_size.Width)
        tex_size.Width = max_size.Width;
    if (tex_size.Height > max_size.Height)
        tex_size.Height = max_size.Height;

    if (orig_size && final_size)
    {
        *orig_size = img_size;
        *final_size = tex_size;
    }
    if (image == NULL)
        return NULL;

    if (image->getColorFormat() != video::ECF_A8R8G8B8 ||
        tex_size != img_size)
    {
        video::IImage* new_texture = driver->createImage(video::ECF_A8R8G8B8, 
                                                         tex_size);
        if (tex_size != img_size)
            image->copyToScaling(new_texture);
        else
            image->copyTo(new_texture);
        image->drop();
        image = new_texture;
    }

#endif   // !SERVER_ONLY
    return image;
}   // resizeImage

// ----------------------------------------------------------------------------
void STKTexture::applyMask(video::IImage* orig_img)
{
#ifndef SERVER_ONLY
    Material* material = NULL;
    if (material_manager)
    {
        material = material_manager->getMaterialFor(this);
    }
    if (material && !material->getAlphaMask().empty())
    {
        video::IImage* converted_mask = irr_driver->getVideoDriver()
            ->createImageFromFile(material->getAlphaMask().c_str());
        if (converted_mask == NULL)
        {
            Log::warn("STKTexture", "Applying mask failed for '%s'!",
                material->getAlphaMask().c_str());
            return;
        }
        converted_mask = resizeImage(converted_mask);
        if (converted_mask->lock())
        {
            const core::dimension2du& dim = orig_img->getDimension();
            for (unsigned int x = 0; x < dim.Width; x++)
            {
                for (unsigned int y = 0; y < dim.Height; y++)
                {
                    video::SColor col = orig_img->getPixel(x, y);
                    video::SColor alpha = converted_mask->getPixel(x, y);
                    col.setAlpha(alpha.getRed());
                    orig_img->setPixel(x, y, col, false);
                }   // for y
            }   // for x
        }
        converted_mask->unlock();
        converted_mask->drop();
    }
#endif   // !SERVER_ONLY
}   // applyMask

//-----------------------------------------------------------------------------
bool STKTexture::hasMipMaps() const
{
#if defined(USE_GLES2)
    return true;
#elif defined(SERVER_ONLY)
    return false;
#else
    return CVS->getGLSLVersion() >= 130;
#endif   // !SERVER_ONLY
}   // hasMipMaps

//-----------------------------------------------------------------------------
void* STKTexture::lock(video::E_TEXTURE_LOCK_MODE mode, u32 mipmap_level)
{
    if (m_texture_image)
        return m_texture_image->lock();

#if !(defined(SERVER_ONLY) || defined(USE_GLES2))
    uint8_t* pixels = new uint8_t[m_size.Width * m_size.Height * 4]();
    GLint tmp_texture;
    glGetIntegerv(GL_TEXTURE_BINDING_2D, &tmp_texture);
    glBindTexture(GL_TEXTURE_2D, m_texture_name);
    glGetTexImage(GL_TEXTURE_2D, 0, GL_BGRA, GL_UNSIGNED_BYTE, pixels);
    glBindTexture(GL_TEXTURE_2D, tmp_texture);
    return pixels;
#endif   // !SERVER_ONLY
    return NULL;
}   // lock

//-----------------------------------------------------------------------------
bool STKTexture::isSrgb() const
{
    return m_tex_config && m_tex_config->m_srgb;
}   // isSrgb

//-----------------------------------------------------------------------------
bool STKTexture::isPremulAlpha() const
{
    return m_tex_config && m_tex_config->m_premul_alpha;
}   // isPremulAlpha

//-----------------------------------------------------------------------------
bool STKTexture::isMeshTexture() const
{
    return m_tex_config && m_tex_config->m_mesh_tex;
}   // isMeshTexture
