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
#include "graphics/hq_mipmap_generator.hpp"
#include "graphics/irr_driver.hpp"
#include "graphics/material.hpp"
#include "graphics/material_manager.hpp"
#include "graphics/materials.hpp"
#include "graphics/stk_tex_manager.hpp"
#include "modes/profile_world.hpp"
#include "utils/log.hpp"
#include "utils/string_utils.hpp"

#include <fstream>
#include <functional>

#if !defined(USE_GLES2)
static const uint8_t CACHE_VERSION = 2;
#endif
// ----------------------------------------------------------------------------
STKTexture::STKTexture(const std::string& path, TexConfig* tc, bool no_upload)
          : video::ITexture(path.c_str()), m_texture_handle(0),
            m_single_channel(false), m_tex_config(NULL), m_material(NULL),
            m_texture_name(0), m_texture_size(0), m_texture_image(NULL),
            m_file(NULL), m_img_loader(NULL)
{
    if (tc != NULL)
    {
        m_tex_config = (TexConfig*)malloc(sizeof(TexConfig));
        memcpy(m_tex_config, tc, sizeof(TexConfig));
        m_single_channel = m_tex_config->m_colorization_mask;
        if (m_tex_config->m_set_material)
            m_material = material_manager->getMaterialFor(this);
    }
#ifndef SERVER_ONLY
    if (m_tex_config)
    {
        if ((!CVS->isARBSRGBFramebufferUsable() && !CVS->isDefferedEnabled()) ||
            !CVS->isGLSL())
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
                       bool single_channel, bool delete_ttl)
          : video::ITexture(name.c_str()), m_texture_handle(0),
            m_single_channel(single_channel), m_tex_config(NULL),
            m_material(NULL), m_texture_name(0), m_texture_size(0),
            m_texture_image(NULL), m_file(NULL), m_img_loader(NULL)
{
    m_size.Width = size;
    m_size.Height = size;
    m_orig_size = m_size;
    if (!delete_ttl)
        reload(false/*no_upload*/, data);
}   // STKTexture

// ----------------------------------------------------------------------------
STKTexture::STKTexture(video::IImage* img, const std::string& name)
          : video::ITexture(name.c_str()), m_texture_handle(0),
            m_single_channel(false), m_tex_config(NULL), m_material(NULL),
            m_texture_name(0), m_texture_size(0), m_texture_image(NULL),
            m_file(NULL), m_img_loader(NULL)
{
    reload(false/*no_upload*/, NULL/*preload_data*/, img);
}   // STKTexture

// ----------------------------------------------------------------------------
STKTexture::~STKTexture()
{
#ifndef SERVER_ONLY
    unloadHandle();
    if (m_texture_name != 0)
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
        m_texture_name = 1;
        if (preload_data)
            delete[] preload_data;
        if (preload_img)
            preload_img->drop();
        return;
    }
#ifndef SERVER_ONLY

    std::string compressed_texture;
#if !defined(USE_GLES2)
    if (!no_upload && isMeshTexture() && CVS->isTextureCompressionEnabled())
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

            if ((!file_manager->fileExists(compressed_texture) ||
                file_manager->fileIsNewer(compressed_texture, orig_file)) &&
                loadCompressedTexture(compressed_texture))
            {
                Log::debug("STKTexture", "Compressed %s for texture %s",
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
    video::IImage* orig_img = NULL;
    uint8_t* data = preload_data;
    if (data == NULL)
    {
        if (preload_img)
            orig_img = preload_img;
        else
        {
            m_file = irr_driver->getDevice()->getFileSystem()
                ->createAndOpenFile(NamedPath);
            if (m_file == NULL)
                return;
            irr_driver->getVideoDriver()->createImageFromFile(m_file,
                &m_img_loader);
            if (m_img_loader == NULL)
                return;
            m_file->seek(0);
            m_orig_size = m_img_loader->getImageSize(m_file);
            if ((!m_material || m_material->getAlphaMask().empty()) &&
                useThreadedLoading() && !no_upload)
            {
                if (m_orig_size.Width == 0 || m_orig_size.Height == 0)
                {
                    m_file->drop();
                    m_file = NULL;
                    m_img_loader = NULL;
                    return;
                }
            }
            else
            {
                orig_img = m_img_loader->loadImage(m_file);
                m_file->drop();
                m_file = NULL;
                if (orig_img == NULL || orig_img->getDimension().Width == 0 ||
                    orig_img->getDimension().Height == 0)
                {
                    if (orig_img)
                        orig_img->drop();
                    return;
                }
                m_img_loader = NULL;
            }
        }
        orig_img = resizeImage(orig_img, &m_orig_size, &m_size);
        applyMask(orig_img);
        data = orig_img ? (uint8_t*)orig_img->lock() : NULL;
        if (m_single_channel && !useThreadedLoading())
        {
            data = singleChannelConversion(data);
            orig_img->unlock();
            orig_img->drop();
            orig_img = NULL;
        }
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

#if !defined(USE_GLES2)
    if (isMeshTexture() && CVS->isTextureCompressionEnabled())
    {
        internal_format = m_single_channel ? GL_COMPRESSED_RED_RGTC1 :
            isSrgb() ? GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT5_EXT :
            GL_COMPRESSED_RGBA_S3TC_DXT5_EXT;
    }
#endif

    if (!useThreadedLoading())
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
            if (useThreadedLoading())
            {
                int levels = 1;
                int width = w;
                int height = h;
                while (true)
                {
                    width = width < 2 ? 1 : width >> 1;
                    height = height < 2 ? 1 : height >> 1;
                    levels++;
                    if (width == 1 && height == 1)
                        break;
                }
                glTexStorage2D(GL_TEXTURE_2D, levels, internal_format, w, h);
            }
            else
            {
                glTexImage2D(GL_TEXTURE_2D, 0, internal_format, w, h, 0, format,
                    GL_UNSIGNED_BYTE, data);
            }
        }
        else if (!useThreadedLoading())
        {
            glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, w, h, format,
                GL_UNSIGNED_BYTE, data);
        }
        if (orig_img)
            orig_img->unlock();
        if (!useThreadedLoading() && hasMipMaps())
            glGenerateMipmap(GL_TEXTURE_2D);
    }

    m_texture_size = w * h * (m_single_channel ? 1 : 4);
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

                if (CVS->isARBSRGBFramebufferUsable() || 
                    CVS->isDefferedEnabled())
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
    core::dimension2du img_size = image ? image->getDimension() : *orig_size;
    core::dimension2du tex_size = img_size.getOptimalSize
        (!irr_driver->getVideoDriver()->queryFeature(video::EVDF_TEXTURE_NPOT));
    const core::dimension2du& max_size = irr_driver->getVideoDriver()
        ->getDriverAttributes().getAttributeAsDimension2d("MAX_TEXTURE_SIZE");

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
        video::IImage* new_texture = irr_driver
            ->getVideoDriver()->createImage(video::ECF_A8R8G8B8, tex_size);
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
        Log::debug("STKTexture", "%s version %d is not supported!",
            file_name.c_str(), cache_verison);
        ifs.close();
        std::remove(file_name.c_str());
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

    std::vector<char> compressed;
    compressed.resize(m_texture_size);
    ifs.read(compressed.data(), m_texture_size);
    if (!ifs.fail())
    {
        // No on the fly reload is supported for compressed texture
        assert(m_texture_name == 0);
        glGenTextures(1, &m_texture_name);
        glBindTexture(GL_TEXTURE_2D, m_texture_name);
        if (internal_format == GL_COMPRESSED_RED_RGTC1)
        {
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_R, GL_ONE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_G, GL_ONE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_B, GL_ONE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_A, GL_RED);
        }
        glCompressedTexImage2D(GL_TEXTURE_2D, 0, internal_format,
            m_size.Width, m_size.Height, 0, m_texture_size, compressed.data());
        unsigned width = m_size.Width;
        unsigned height = m_size.Height;
        std::vector<std::pair<unsigned, unsigned> > mipmap_sizes;
        while (true)
        {
            width = width < 2 ? 1 : width >> 1;
            height = height < 2 ? 1 : height >> 1;
            mipmap_sizes.emplace_back(width, height);
            if (width == 1 && height == 1)
                break;
        }
        for (unsigned i = 0; i < mipmap_sizes.size(); i++)
        {
            unsigned cur_mipmap_size = 0;
            ifs.read((char*)&cur_mipmap_size, sizeof(unsigned int));
            ifs.read(compressed.data(), cur_mipmap_size);
            if (cur_mipmap_size == 0 || ifs.fail())
            {
                ifs.close();
                std::remove(file_name.c_str());
                return false;
            }
            glCompressedTexImage2D(GL_TEXTURE_2D, i + 1, internal_format,
                mipmap_sizes[i].first, mipmap_sizes[i].second, 0,
                cur_mipmap_size, compressed.data());
        }
        glBindTexture(GL_TEXTURE_2D, 0);
        return true;
    }
    ifs.close();
    std::remove(file_name.c_str());
#endif
    return false;
}   // loadCompressedTexture

//-----------------------------------------------------------------------------
/** Try to save the last texture sent to glTexImage2D in a file of the given
 *   file name. This function should only be used for textures sent to
 *   glTexImage2D with a compressed internal format as argument.<br>
 *   \note The following format is used to save the compressed texture:<br>
 *         <version><internal-format><w><h><orig_w><orig_h><size><data> <br>
 *         The first element is the version of cache, next six elements are
 *         integers and the last one is stored on \c size bytes.
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

    std::vector<char> compressed;
    compressed.resize(m_texture_size);
    glGetCompressedTexImage(GL_TEXTURE_2D, 0, compressed.data());
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
        ofs.write(compressed.data(), m_texture_size);
        unsigned width = m_size.Width;
        unsigned height = m_size.Height;
        std::vector<std::pair<unsigned, unsigned> > mipmap_sizes;
        while (true)
        {
            width = width < 2 ? 1 : width >> 1;
            height = height < 2 ? 1 : height >> 1;
            mipmap_sizes.emplace_back(width, height);
            if (width == 1 && height == 1)
                break;
        }
        for (unsigned i = 0; i < mipmap_sizes.size(); i++)
        {
            GLint cur_mipmap_size = 0;
            glGetTexLevelParameteriv(GL_TEXTURE_2D, i + 1,
                GL_TEXTURE_COMPRESSED_IMAGE_SIZE, &cur_mipmap_size);
            if (cur_mipmap_size == 0)
            {
                ofs.close();
                std::remove(compressed_tex.c_str());
            }
            glGetCompressedTexImage(GL_TEXTURE_2D, i + 1, compressed.data());
            ofs.write((char*)&cur_mipmap_size, sizeof(unsigned int));
            ofs.write(compressed.data(), cur_mipmap_size);
        }
    }
#endif
}   // saveCompressedTexture

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
u64 STKTexture::getHandle()
{
#if !(defined(SERVER_ONLY) || defined(USE_GLES2))
    assert(CVS->isAZDOEnabled());
    if (m_texture_handle != 0) return m_texture_handle;

    m_texture_handle =
        glGetTextureSamplerHandleARB( m_texture_name,
        ObjectPass1Shader::getInstance()->m_sampler_ids[0]);

    if (!glIsTextureHandleResidentARB(m_texture_handle))
        glMakeTextureHandleResidentARB(m_texture_handle);
#endif
    return m_texture_handle;
}   // getHandle

//-----------------------------------------------------------------------------
void STKTexture::unloadHandle()
{
#if !(defined(SERVER_ONLY) || defined(USE_GLES2))
    if (CVS->isAZDOEnabled())
    {
        if (m_texture_handle == 0) return;
        glMakeTextureHandleNonResidentARB(m_texture_handle);
        m_texture_handle = 0;
    }
#endif
}   // unloadHandle

//-----------------------------------------------------------------------------
bool STKTexture::useThreadedLoading() const
{
#ifdef SERVER_ONLY
    return false;
#else
    return CVS->supportsThreadedTextureLoading() &&
        !CVS->isTextureCompressionEnabled() && isMeshTexture() &&
        m_img_loader && m_img_loader->supportThreadedLoading();
#endif
}   // useThreadedLoading

//-----------------------------------------------------------------------------
void STKTexture::threadedReload(void* ptr, void* param) const
{
#if !(defined(SERVER_ONLY) || defined(USE_GLES2))
    video::IImage* orig_img =
        m_img_loader->loadImage(m_file, true/*skip_checking*/);
    orig_img = resizeImage(orig_img);
    uint8_t* data = (uint8_t*)orig_img->lock();
    if (m_single_channel)
    {
        data = singleChannelConversion(data);
        orig_img->unlock();
        orig_img->drop();
        orig_img = NULL;
    }
    formatConversion(data, NULL, m_size.Width, m_size.Height);
    memcpy(ptr, data, m_texture_size);

    if (orig_img)
    {
        orig_img->unlock();
        orig_img->setDeleteMemory(false);
        orig_img->drop();
    }
    if (useHQMipmap())
    {
        HQMipmapGenerator* hqmg = new HQMipmapGenerator(NamedPath, data,
            m_size, m_texture_name, m_tex_config);
        ((STKTexManager*)(param))->addThreadedLoadTexture(hqmg);
    }
    else
        delete[] data;
#endif
}   // threadedReload

//-----------------------------------------------------------------------------
void STKTexture::threadedSubImage(void* ptr) const
{
#if !(defined(SERVER_ONLY) || defined(USE_GLES2))
    glBindTexture(GL_TEXTURE_2D, m_texture_name);
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, m_size.Width, m_size.Height,
        m_single_channel ? GL_RED : GL_BGRA, GL_UNSIGNED_BYTE, ptr);
    if (useHQMipmap())
        return;
    if (hasMipMaps())
        glGenerateMipmap(GL_TEXTURE_2D);

#endif
}   // threadedSubImage

//-----------------------------------------------------------------------------
void STKTexture::cleanThreadedLoader()
{
#if !(defined(SERVER_ONLY) || defined(USE_GLES2))
    assert(m_file);
    m_file->drop();
    m_file = NULL;
    m_img_loader = NULL;
#endif
}   // cleanThreadedLoader

//-----------------------------------------------------------------------------
bool STKTexture::useHQMipmap() const
{
    return !m_single_channel && UserConfigParams::m_hq_mipmap &&
        m_size.Width > 1 && m_size.Height > 1;
}   // useHQMipmap

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
