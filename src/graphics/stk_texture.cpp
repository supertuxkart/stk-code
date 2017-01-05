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
#include "graphics/central_settings.hpp"
#include "graphics/irr_driver.hpp"
#include "graphics/material.hpp"
#include "graphics/material_manager.hpp"
#include "utils/log.hpp"
#include "utils/string_utils.hpp"

#include <fstream>
#include <functional>
#include <sstream>

// ----------------------------------------------------------------------------
STKTexture::STKTexture(const std::string& path, bool srgb, bool premul_alpha,
                       bool set_material, bool mesh_tex, bool no_upload)
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
    reload(no_upload);
}   // STKTexture

// ----------------------------------------------------------------------------
STKTexture::STKTexture(video::IImage* image, const std::string& name)
          : video::ITexture(name.c_str()), m_texture_handle(0), m_srgb(false),
            m_premul_alpha(false), m_mesh_texture(false), m_material(NULL),
            m_texture_name(0), m_texture_size(0), m_texture_image(NULL)
{
    reload(false/*no_upload*/, image/*pre_loaded_tex*/);
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
void STKTexture::reload(bool no_upload, video::IImage* pre_loaded_tex)
{
#ifndef SERVER_ONLY
    irr_driver->getDevice()->getLogger()->setLogLevel(ELL_NONE);

    std::string compressed_texture;
#if !defined(USE_GLES2)
    if (!no_upload && m_mesh_texture && CVS->isTextureCompressionEnabled())
    {
        std::string orig_file = NamedPath.getPtr();
        compressed_texture = getHashedName(orig_file);
        if (!file_manager->fileIsNewer(orig_file, compressed_texture))
        {
            if (loadCompressedTexture(compressed_texture))
            {
                Log::info("STKTexture", "Compressed %s for texture %s",
                    compressed_texture.c_str(), orig_file.c_str());
                return;
            }
        }
    }
#endif

    video::IImage* orig_img = NULL;
    if (pre_loaded_tex == NULL)
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
            return;
        }
    }
    else
        orig_img = pre_loaded_tex;

    video::IImage* new_texture = NULL;
    if (pre_loaded_tex == NULL)
    {
        new_texture = convertImage(&orig_img);
        applyMask(new_texture);
    }
    else
        new_texture = pre_loaded_tex;

    unsigned char* data = (unsigned char*)new_texture->lock();
    const unsigned int w = new_texture->getDimension().Width;
    const unsigned int h = new_texture->getDimension().Height;
    unsigned int format = GL_BGRA;
    unsigned int internal_format = GL_RGBA;

#if !defined(USE_GLES2)
    if (m_mesh_texture && CVS->isTextureCompressionEnabled())
    {
        internal_format = m_srgb ?
            GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT5_EXT :
            GL_COMPRESSED_RGBA_S3TC_DXT5_EXT;
    }
    else
    {
        internal_format = m_srgb ? GL_SRGB_ALPHA : GL_RGBA;
    }
#endif

#if defined(USE_GLES2)
    if (!CVS->isEXTTextureFormatBGRA8888Usable())
    {
        format = GL_RGBA;
        for (unsigned int i = 0; i < w * h; i++)
        {
            char tmp_val = data[i * 4];
            data[i * 4] = data[i * 4 + 2];
            data[i * 4 + 2] = tmp_val;
        }
    }
#endif
    if (m_premul_alpha)
    {
        for (unsigned int i = 0; i < w * h; i++)
        {
            float alpha = data[4 * i + 3];
            if (alpha > 0.0f)
                alpha = pow(alpha / 255.f, 1.f / 2.2f);
            data[i * 4] = (unsigned char)(data[i * 4] * alpha);
            data[i * 4 + 1] = (unsigned char)(data[i * 4 + 1] * alpha);
            data[i * 4 + 2] = (unsigned char)(data[i * 4 + 2] * alpha);
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
            glTexImage2D(GL_TEXTURE_2D, 0, internal_format,
                new_texture->getDimension().Width,
                new_texture->getDimension().Height, 0, format,
                GL_UNSIGNED_BYTE, data);
        }
        else
        {
            glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0,
                new_texture->getDimension().Width,
                new_texture->getDimension().Height, format, GL_UNSIGNED_BYTE,
                data);
        }
        new_texture->unlock();
        glGenerateMipmap(GL_TEXTURE_2D);
    }

    m_size = new_texture->getDimension();
    m_orig_size = orig_img->getDimension();
    orig_img->drop();
    m_texture_size = m_size.Width * m_size.Height * 4 /*BRGA*/;
    if (!no_upload && pre_loaded_tex == NULL)
        new_texture->drop();
    else if (pre_loaded_tex == NULL)
        m_texture_image = new_texture;

    if (!compressed_texture.empty())
        saveCompressedTexture(compressed_texture);
    if (!no_upload)
        glBindTexture(GL_TEXTURE_2D, 0);

    irr_driver->getDevice()->getLogger()->setLogLevel(ELL_WARNING);
#endif   // !SERVER_ONLY
}   // reload

// ----------------------------------------------------------------------------
video::IImage* STKTexture::convertImage(video::IImage** orig_img)
{
#ifndef SERVER_ONLY
    video::IImage* image = *orig_img;
    core::dimension2du size = image->getDimension();

    bool scale_image = false;
    const float ratio = float(size.Width) / float(size.Height);
    const unsigned int drv_max_size =
        irr_driver->getVideoDriver()->getMaxTextureSize().Width;

    if ((size.Width > drv_max_size) && (ratio >= 1.0f))
    {
        size.Width = drv_max_size;
        size.Height = (unsigned)(drv_max_size / ratio);
        scale_image = true;
    }
    else if (size.Height > drv_max_size)
    {
        size.Height = drv_max_size;
        size.Width = (unsigned)(drv_max_size * ratio);
        scale_image = true;
    }
    if (scale_image)
    {
        video::IImage* new_img = irr_driver->getVideoDriver()
            ->createImage(video::ECF_A8R8G8B8, size);
        image->copyToScaling(new_img);
        image->drop();
        image = new_img;
        *orig_img = new_img;
    }

    bool scale_texture = false;
    size = size.getOptimalSize
        (!irr_driver->getVideoDriver()->queryFeature(video::EVDF_TEXTURE_NPOT));
    const core::dimension2du& max_size = irr_driver->getVideoDriver()
        ->getDriverAttributes().getAttributeAsDimension2d("MAX_TEXTURE_SIZE");

    if (max_size.Width > 0 && size.Width > max_size.Width)
    {
        size.Width = max_size.Width;
        scale_texture = true;
    }
    if (max_size.Height > 0 && size.Height > max_size.Height)
    {
        size.Height = max_size.Height;
        scale_texture = true;
    }

    video::IImage* new_texture =
        irr_driver->getVideoDriver()->createImage(video::ECF_A8R8G8B8, size);
    if (scale_texture)
        image->copyToScaling(new_texture);
    else
        image->copyTo(new_texture);

    return new_texture;
#endif   // !SERVER_ONLY
}   // convertImage

// ----------------------------------------------------------------------------
void STKTexture::applyMask(video::IImage* orig_img)
{
#ifndef SERVER_ONLY
    if (m_material && !m_material->getAlphaMask().empty())
    {
        video::IImage* tmp_mask = irr_driver->getVideoDriver()
            ->createImageFromFile(m_material->getAlphaMask().c_str());
        if (tmp_mask == NULL)
        {
            Log::warn("STKTexture", "Applying mask failed for '%s'!",
                m_material->getAlphaMask().c_str());
            return;
        }
        video::IImage* converted_mask = convertImage(&tmp_mask);
        tmp_mask->drop();
        if (converted_mask->lock())
        {
            core::dimension2d<u32> dim = orig_img->getDimension();
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
    return false;
#endif
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
std::string STKTexture::getHashedName(const std::string& orig_file)
{
    std::string result = file_manager->getCachedTexturesDir();
    std::stringstream hash;
    size_t hash_1 = std::hash<std::string>{}(StringUtils::getPath(orig_file));
    size_t hash_2 =
        std::hash<std::string>{}(StringUtils::getBasename(orig_file));
    const core::dimension2du& max_size = irr_driver->getVideoDriver()
        ->getDriverAttributes().getAttributeAsDimension2d("MAX_TEXTURE_SIZE");
    hash << std::hex << hash_1 << hash_2 << max_size.Height;
    return result + hash.str() + ".stktz";
}   // getHashedName
