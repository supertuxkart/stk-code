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

#include "config/user_config.hpp"
#include "graphics/stk_texture.hpp"
#include "graphics/central_settings.hpp"
#include "graphics/irr_driver.hpp"
#include "graphics/material.hpp"
#include "graphics/material_manager.hpp"
#include "modes/profile_world.hpp"
#include "utils/log.hpp"
#include "utils/string_utils.hpp"

#include <fstream>
#include <functional>

#if !defined(USE_GLES2)
static const uint8_t CACHE_VERSION = 1;
#endif
// ----------------------------------------------------------------------------
STKTexture::STKTexture(const std::string& path, bool srgb, bool premul_alpha,
                       bool set_material, bool mesh_tex, bool no_upload,
                       bool single_channel)
          : video::ITexture(path.c_str()), m_texture_handle(0), m_srgb(srgb),
            m_premul_alpha(premul_alpha), m_mesh_texture(mesh_tex),
            m_material(NULL), m_texture_name(0), m_texture_size(0),
            m_texture_image(NULL)
{
    if (set_material)
    {
        m_material = material_manager->getMaterialFor(this);
        m_mesh_texture = true;
    }
#ifndef SERVER_ONLY
    bool sc = single_channel;
    if (!CVS->isGLSL())
        m_srgb = false;
    if (!CVS->isARBTextureSwizzleUsable())
        sc = false;
#endif
    reload(no_upload, NULL/*preload_data*/, sc);
}   // STKTexture

// ----------------------------------------------------------------------------
STKTexture::STKTexture(uint8_t* data, const std::string& name, size_t size,
                       bool single_channel)
          : video::ITexture(name.c_str()), m_texture_handle(0), m_srgb(false),
            m_premul_alpha(false), m_mesh_texture(false), m_material(NULL),
            m_texture_name(0), m_texture_size(0), m_texture_image(NULL)
{
    m_size.Width = size;
    m_size.Height = size;
    m_orig_size = m_size;
    reload(false/*no_upload*/, data, single_channel);
}   // STKTexture

// ----------------------------------------------------------------------------
STKTexture::~STKTexture()
{
#ifndef SERVER_ONLY
    if (m_texture_name != 0)
    {
        glDeleteTextures(1, &m_texture_name);
    }
#endif   // !SERVER_ONLY
    if (m_texture_image != NULL)
        m_texture_image->drop();
}   // ~STKTexture

// ----------------------------------------------------------------------------
void STKTexture::reload(bool no_upload, uint8_t* preload_data,
                        bool single_channel)
{
    if (ProfileWorld::isNoGraphics())
    {
        m_texture_name = 1;
        if (preload_data)
            delete[] preload_data;
        return;
    }
#ifndef SERVER_ONLY
    irr_driver->getDevice()->getLogger()->setLogLevel(ELL_NONE);

    std::string compressed_texture;
#if !defined(USE_GLES2)
    if (!no_upload && !single_channel && m_mesh_texture &&
        CVS->isTextureCompressionEnabled())
    {
        std::string orig_file = NamedPath.getPtr();

        std::string basename = StringUtils::getBasename(orig_file);
        std::string container_id;
        if (file_manager->searchTextureContainerId(container_id, basename))
        {
            std::string cache_subdir = "hd/";
            if ((UserConfigParams::m_high_definition_textures & 0x01) == 0x01)
            {
                cache_subdir = "hd/";
            }
            else
            {
                cache_subdir = StringUtils::insertValues("resized_%i/",
                    (int)UserConfigParams::m_max_texture_size);
            }

            std::string cache_dir = file_manager->getCachedTexturesDir() +
                cache_subdir + container_id;
            compressed_texture = cache_dir + "/" + basename + ".stktz";

            if (loadCompressedTexture(compressed_texture))
            {
                Log::verbose("STKTexture", "Compressed %s for texture %s",
                    compressed_texture.c_str(), orig_file.c_str());
                return;
            }

            file_manager->checkAndCreateDirectoryP(cache_dir);
        }
        else
        {
            Log::warn("STKTexture", "Cannot find container_id for texture %s",
                orig_file.c_str());
        }
    }
#endif
    unsigned int format = single_channel ? GL_RED : GL_BGRA;
    unsigned int internal_format = single_channel ? GL_R8 : GL_RGBA;
    video::IImage* orig_img = NULL;
    uint8_t* data = preload_data;
    if (data == NULL)
    {
        orig_img =
            irr_driver->getVideoDriver()->createImageFromFile(NamedPath);
        if (orig_img == NULL)
        {
            Log::warn("STKTexture", "No image %s.", NamedPath.getPtr());
            return;
        }

        if (orig_img->getDimension().Width  == 0 ||
            orig_img->getDimension().Height == 0)
        {
            Log::warn("STKTexture", "image %s has 0 size.",
                NamedPath.getPtr());
            orig_img->drop();
            return;
        }
        orig_img = resizeImage(orig_img, &m_orig_size, &m_size);
        applyMask(orig_img);
        data = (uint8_t*)orig_img->lock();
        if (single_channel)
        {
            uint8_t* sc = new uint8_t[m_size.Width * m_size.Height];
            for (unsigned int i = 0; i < m_size.Width * m_size.Height; i++)
                sc[i] = data[4 * i + 3];
            orig_img->unlock();
            orig_img->drop();
            orig_img = NULL;
            data = sc;
        }
    }

    const unsigned int w = m_size.Width;
    const unsigned int h = m_size.Height;

#if !defined(USE_GLES2)
    if (m_mesh_texture && CVS->isTextureCompressionEnabled() & !single_channel)
    {
        internal_format = m_srgb ?
            GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT5_EXT :
            GL_COMPRESSED_RGBA_S3TC_DXT5_EXT;
    }
    else if (!single_channel)
    {
        internal_format = m_srgb ? GL_SRGB_ALPHA : GL_RGBA;
    }
#endif

#if defined(USE_GLES2)
    if (!CVS->isEXTTextureFormatBGRA8888Usable() && !single_channel)
    {
        format = GL_RGBA;
        for (unsigned int i = 0; i < w * h; i++)
        {
            uint8_t tmp_val = data[i * 4];
            data[i * 4] = data[i * 4 + 2];
            data[i * 4 + 2] = tmp_val;
        }
    }
#endif
    if (m_premul_alpha && !single_channel)
    {
        for (unsigned int i = 0; i < w * h; i++)
        {
            float alpha = data[4 * i + 3];
            if (alpha > 0.0f)
                alpha = pow(alpha / 255.f, 1.f / 2.2f);
            data[i * 4] = (uint8_t)(data[i * 4] * alpha);
            data[i * 4 + 1] = (uint8_t)(data[i * 4 + 1] * alpha);
            data[i * 4 + 2] = (uint8_t)(data[i * 4 + 2] * alpha);
        }
    }

    if (!no_upload)
    {
        const bool reload = m_texture_name != 0;
        if (!reload)
            glGenTextures(1, &m_texture_name);

        glBindTexture(GL_TEXTURE_2D, m_texture_name);
        if (!reload)
        {
            if (single_channel)
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

    m_texture_size = w * h * (single_channel ? 1 : 4);
    if (no_upload)
        m_texture_image = orig_img;
    else if (orig_img)
        orig_img->drop();
    else
        delete[] data;

    if (!compressed_texture.empty())
        saveCompressedTexture(compressed_texture);
    if (!no_upload)
        glBindTexture(GL_TEXTURE_2D, 0);

    irr_driver->getDevice()->getLogger()->setLogLevel(ELL_WARNING);
#endif   // !SERVER_ONLY
}   // reload

// ----------------------------------------------------------------------------
video::IImage* STKTexture::resizeImage(video::IImage* orig_img,
                                       core::dimension2du* new_img_size,
                                       core::dimension2du* new_tex_size)
{
    video::IImage* image = orig_img;
#ifndef SERVER_ONLY
    const core::dimension2du& old_size = image->getDimension();
    core::dimension2du img_size = old_size;

    const float ratio = float(img_size.Width) / float(img_size.Height);
    const unsigned int drv_max_size =
        irr_driver->getVideoDriver()->getMaxTextureSize().Width;

    if ((img_size.Width > drv_max_size) && (ratio >= 1.0f))
    {
        img_size.Width = drv_max_size;
        img_size.Height = (unsigned)(drv_max_size / ratio);
    }
    else if (img_size.Height > drv_max_size)
    {
        img_size.Height = drv_max_size;
        img_size.Width = (unsigned)(drv_max_size * ratio);
    }

    if (img_size != old_size)
    {
        video::IImage* new_img = irr_driver->getVideoDriver()
            ->createImage(video::ECF_A8R8G8B8, img_size);
        image->copyToScaling(new_img);
        image->drop();
        image = new_img;
    }

    core::dimension2du tex_size = img_size.getOptimalSize
        (!irr_driver->getVideoDriver()->queryFeature(video::EVDF_TEXTURE_NPOT));
    const core::dimension2du& max_size = irr_driver->getVideoDriver()
        ->getDriverAttributes().getAttributeAsDimension2d("MAX_TEXTURE_SIZE");

    if (max_size.Width > 0 && tex_size.Width > max_size.Width)
        tex_size.Width = max_size.Width;
    if (max_size.Height > 0 && tex_size.Height > max_size.Height)
        tex_size.Height = max_size.Height;

    if (image->getColorFormat() != video::ECF_A8R8G8B8 ||
        tex_size != img_size)
    {
        video::IImage* new_texture = irr_driver
            ->getVideoDriver()->createImage(video::ECF_A8R8G8B8, tex_size);
        if (tex_size != img_size)
            image->copyToScaling(new_texture);
        else
            image->copyTo(new_texture);
        image->drop();
        image = new_texture;
    }

    if (new_img_size && new_tex_size)
    {
        *new_img_size = img_size;
        *new_tex_size = tex_size;
    }
#endif   // !SERVER_ONLY
    return image;
}   // resizeImage

// ----------------------------------------------------------------------------
void STKTexture::applyMask(video::IImage* orig_img)
{
#ifndef SERVER_ONLY
    if (m_material && !m_material->getAlphaMask().empty())
    {
        video::IImage* converted_mask = irr_driver->getVideoDriver()
            ->createImageFromFile(m_material->getAlphaMask().c_str());
        if (converted_mask == NULL)
        {
            Log::warn("STKTexture", "Applying mask failed for '%s'!",
                m_material->getAlphaMask().c_str());
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
/** Try to load a compressed texture from the given file name.
 *  Data in the specified file need to have a specific format. See the
 *  saveCompressedTexture() function for a description of the format.
 *  \return true if the loading succeeded, false otherwise.
 *  \see saveCompressedTexture
 */
bool STKTexture::loadCompressedTexture(const std::string& file_name)
{
#if !(defined(SERVER_ONLY) || defined(USE_GLES2))
    std::ifstream ifs(file_name.c_str(), std::ios::in | std::ios::binary);
    if (!ifs.is_open())
        return false;

    int internal_format;
    uint8_t cache_verison;
    ifs.read((char*)&cache_verison, sizeof(uint8_t));
    if (cache_verison != CACHE_VERSION)
    {
        Log::warn("STKTexture", "%s version %d is not supported!",
            file_name.c_str(), cache_verison);
        ifs.close();
        // Remove the file later if we have more version
        return false;
    }
    ifs.read((char*)&internal_format, sizeof(int));
    ifs.read((char*)&m_size.Width, sizeof(unsigned int));
    ifs.read((char*)&m_size.Height, sizeof(unsigned int));
    ifs.read((char*)&m_orig_size.Width, sizeof(unsigned int));
    ifs.read((char*)&m_orig_size.Height, sizeof(unsigned int));
    ifs.read((char*)&m_texture_size, sizeof(unsigned int));

    if (ifs.fail() || m_texture_size == 0)
        return false;

    char *data = new char[m_texture_size];
    ifs.read(data, m_texture_size);
    if (!ifs.fail())
    {
        // No on the fly reload is supported for compressed texture
        assert(m_texture_name == 0);
        glGenTextures(1, &m_texture_name);
        glBindTexture(GL_TEXTURE_2D, m_texture_name);
        glCompressedTexImage2D(GL_TEXTURE_2D, 0, internal_format,
            m_size.Width, m_size.Height, 0, m_texture_size, (GLvoid*)data);
        glGenerateMipmap(GL_TEXTURE_2D);
        glBindTexture(GL_TEXTURE_2D, 0);
        delete[] data;
        ifs.close();
        return true;
    }
    delete[] data;
#endif
    return false;
}   // loadCompressedTexture

//-----------------------------------------------------------------------------
/** Try to save the last texture sent to glTexImage2D in a file of the given
 *   file name. This function should only be used for textures sent to
 *   glTexImage2D with a compressed internal format as argument.<br>
 *   \note The following format is used to save the compressed texture:<br>
 *         <internal-format><w><h><orig_w><orig_h><size><data> <br>
 *         The first six elements are integers and the last one is stored
 *         on \c size bytes.
 *   \see loadCompressedTexture
 */
void STKTexture::saveCompressedTexture(const std::string& compressed_tex)
{
#if !(defined(SERVER_ONLY) || defined(USE_GLES2))
    int internal_format;
    int compression_successful = 0;
    glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_INTERNAL_FORMAT,
        (GLint *)&internal_format);
    glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_WIDTH,
        (GLint *)&m_size.Width);
    glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_HEIGHT,
        (GLint *)&m_size.Height);
    glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_COMPRESSED,
        (GLint *)&compression_successful);
    if (compression_successful == 0) return;
    glGetTexLevelParameteriv(GL_TEXTURE_2D, 0,
        GL_TEXTURE_COMPRESSED_IMAGE_SIZE, (GLint *)&m_texture_size);
    if (m_texture_size == 0) return;

    char *data = new char[m_texture_size];
    glGetCompressedTexImage(GL_TEXTURE_2D, 0, (GLvoid*)data);
    std::ofstream ofs(compressed_tex.c_str(),
        std::ios::out | std::ios::binary);
    if (ofs.is_open())
    {
        ofs.write((char*)&CACHE_VERSION, sizeof(uint8_t));
        ofs.write((char*)&internal_format, sizeof(int));
        ofs.write((char*)&m_size.Width, sizeof(unsigned int));
        ofs.write((char*)&m_size.Height, sizeof(unsigned int));
        ofs.write((char*)&m_orig_size.Width, sizeof(unsigned int));
        ofs.write((char*)&m_orig_size.Height, sizeof(unsigned int));
        ofs.write((char*)&m_texture_size, sizeof(unsigned int));
        ofs.write(data, m_texture_size);
        ofs.close();
    }
    delete[] data;
#endif
}   // saveCompressedTexture

//-----------------------------------------------------------------------------
bool STKTexture::hasMipMaps() const
{
#ifndef SERVER_ONLY
    return CVS->getGLSLVersion() >= 130;
#endif   // !SERVER_ONLY
    return false;
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
