//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2018 SuperTuxKart-Team
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

#include "graphics/sp/sp_texture.hpp"
#include "config/stk_config.hpp"
#include "config/user_config.hpp"
#include "io/file_manager.hpp"
#include "graphics/sp/sp_base.hpp"
#include "graphics/sp/sp_shader.hpp"
#include "graphics/sp/sp_shader_manager.hpp"
#include "graphics/sp/sp_texture_manager.hpp"
#include "graphics/central_settings.hpp"
#include "graphics/irr_driver.hpp"
#include "graphics/material.hpp"
#include "utils/log.hpp"
#include "utils/string_utils.hpp"

#include <IImageLoader.h>
#include <IReadFile.h>
#include <IVideoDriver.h>
#include <IWriteFile.h>

#if !defined(SERVER_ONLY)
#include <squish.h>
static_assert(squish::kColourClusterFit == (1 << 5), "Wrong header");
static_assert(squish::kColourRangeFit == (1 << 6), "Wrong header");
static_assert(squish::kColourIterativeClusterFit == (1 << 8), "Wrong header");
#endif

#if !defined(SERVER_ONLY)
extern "C"
{
    #include <mipmap/img.h>
    #include <mipmap/imgresize.h>
}
#endif

#include <numeric>

static const uint8_t CACHE_VERSION = 2;

namespace SP
{
// ----------------------------------------------------------------------------
SPTexture::SPTexture(const std::string& path, Material* m, bool undo_srgb,
                     const std::string& container_id)
         : m_path(path), m_width(0), m_height(0), m_material(m),
           m_undo_srgb(undo_srgb)
{
#ifndef SERVER_ONLY
    glGenTextures(1, &m_texture_name);

    createWhite(false/*private_init*/);

    if (!CVS->isTextureCompressionEnabled() || container_id.empty())
    {
        return;
    }

    std::string cache_subdir = "hd";
    if ((UserConfigParams::m_high_definition_textures & 0x01) == 0x01)
    {
        cache_subdir = "hd";
    }
    else
    {
        cache_subdir = StringUtils::insertValues("resized_%i",
            (int)UserConfigParams::m_max_texture_size);
    }
    
#ifdef USE_GLES2
    if (m_undo_srgb && !CVS->isEXTTextureCompressionS3TCSRGBUsable())
    {
        cache_subdir += "-linear";
    }
#endif

    m_cache_directory = file_manager->getCachedTexturesDir() +
        cache_subdir + "/" + container_id;
    file_manager->checkAndCreateDirectoryP(m_cache_directory);

#endif
}   // SPTexture

// ----------------------------------------------------------------------------
SPTexture::SPTexture(bool white)
         : m_width(0), m_height(0), m_undo_srgb(false)
{
#ifndef SERVER_ONLY
    glGenTextures(1, &m_texture_name);
    if (white)
    {
        createWhite();
    }
    else
    {
        createTransparent();
    }
#endif
}   // SPTexture

// ----------------------------------------------------------------------------
SPTexture::~SPTexture()
{
#ifndef SERVER_ONLY
    if (m_texture_name != 0)
    {
        glDeleteTextures(1, &m_texture_name);
    }
#endif
}   // ~SPTexture

// ----------------------------------------------------------------------------
std::shared_ptr<video::IImage> SPTexture::getImageFromPath
                                                (const std::string& path) const
{
    video::IImageLoader* img_loader =
        irr_driver->getVideoDriver()->getImageLoaderForFile(path.c_str());
    if (img_loader == NULL)
    {
        Log::error("SPTexture", "No image loader for %s", path.c_str());
        return NULL;
    }

    io::IReadFile* file = irr::io::createReadFile(path.c_str());
    video::IImage* image = img_loader->loadImage(file);
    if (image == NULL || image->getDimension().Width == 0 ||
        image->getDimension().Height == 0)
    {
        Log::error("SPTexture", "Failed to load image %s", path.c_str());
        if (image)
        {
            image->drop();
        }
        if (file)
        {
            file->drop();
        }
        return NULL;
    }
    file->drop();
    assert(image->getReferenceCount() == 1);
    return std::shared_ptr<video::IImage>(image);
}   // getImagefromPath

// ----------------------------------------------------------------------------
std::shared_ptr<video::IImage> SPTexture::getTextureImage() const
{
    std::shared_ptr<video::IImage> image;
#ifndef SERVER_ONLY
    image = getImageFromPath(m_path);
    if (!image)
    {
        return NULL;
    }
    core::dimension2du img_size = image->getDimension();
    core::dimension2du tex_size = img_size.getOptimalSize
        (true/*requirePowerOfTwo*/, false/*requireSquare*/, false/*larger*/);
    unsigned max = sp_max_texture_size.load();
    core::dimension2du max_size = core::dimension2du(max, max);

    if (tex_size.Width > max_size.Width)
    {
        tex_size.Width = max_size.Width;
    }
    if (tex_size.Height > max_size.Height)
    {
        tex_size.Height = max_size.Height;
    }
    if (image->getColorFormat() != video::ECF_A8R8G8B8 ||
        tex_size != img_size)
    {
        video::IImage* new_texture = irr_driver
            ->getVideoDriver()->createImage(video::ECF_A8R8G8B8, tex_size);
        if (tex_size != img_size)
        {
            image->copyToScaling(new_texture);
        }
        else
        {
            image->copyTo(new_texture);
        }
        assert(new_texture->getReferenceCount() == 1);
        image.reset(new_texture);
    }

    uint8_t* data = (uint8_t*)image->lock();
    for (unsigned int i = 0; i < image->getDimension().Width *
        image->getDimension().Height; i++)
    {
        const bool use_tex_compress = CVS->isTextureCompressionEnabled() &&
            !m_cache_directory.empty();
#ifndef USE_GLES2
        if (use_tex_compress)
        {
#endif
            // to RGBA for libsquish or for gles it's always true
            uint8_t tmp_val = data[i * 4];
            data[i * 4] = data[i * 4 + 2];
            data[i * 4 + 2] = tmp_val;
#ifndef USE_GLES2
        }
#endif

        bool force_undo_srgb = use_tex_compress && 
                                  !CVS->isEXTTextureCompressionS3TCSRGBUsable();

        if (m_undo_srgb && (!use_tex_compress || force_undo_srgb))
        {
            data[i * 4] = srgb255ToLinear(data[i * 4]);
            data[i * 4 + 1] = srgb255ToLinear(data[i * 4 + 1]);
            data[i * 4 + 2] = srgb255ToLinear(data[i * 4 + 2]);
        }
    }
#endif
    return image;
}   // getTextureImage

// ----------------------------------------------------------------------------
bool SPTexture::compressedTexImage2d(std::shared_ptr<video::IImage> texture,
                                     const std::vector<std::pair
                                     <core::dimension2du, unsigned> >&
                                     mipmap_sizes)
{
#if !defined(SERVER_ONLY)
    unsigned format = GL_COMPRESSED_RGBA_S3TC_DXT5_EXT;
    if (m_undo_srgb && CVS->isEXTTextureCompressionS3TCSRGBUsable())
    {
        format = GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT5_EXT;
    }
    glDeleteTextures(1, &m_texture_name);
    glGenTextures(1, &m_texture_name);
    glBindTexture(GL_TEXTURE_2D, m_texture_name);
    uint8_t* compressed = (uint8_t*)texture->lock();
    unsigned cur_mipmap_size = 0;
    for (unsigned i = 0; i < mipmap_sizes.size(); i++)
    {
        cur_mipmap_size = mipmap_sizes[i].second;
        glCompressedTexImage2D(GL_TEXTURE_2D, i, format,
            mipmap_sizes[i].first.Width, mipmap_sizes[i].first.Height, 0,
            cur_mipmap_size, compressed);
        compressed += cur_mipmap_size;
    }
    glBindTexture(GL_TEXTURE_2D, 0);
    m_width.store(mipmap_sizes[0].first.Width);
    m_height.store(mipmap_sizes[0].first.Height);
#endif
    return true;
}   // compressedTexImage2d

// ----------------------------------------------------------------------------
bool SPTexture::texImage2d(std::shared_ptr<video::IImage> texture,
                           std::shared_ptr<video::IImage> mipmaps)
{
#ifndef SERVER_ONLY
    if (texture)
    {
#ifdef USE_GLES2
        unsigned upload_format = GL_RGBA;
#else
        unsigned upload_format = GL_BGRA;
#endif
        glDeleteTextures(1, &m_texture_name);
        glGenTextures(1, &m_texture_name);
        glBindTexture(GL_TEXTURE_2D, m_texture_name);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA,
            texture->getDimension().Width, texture->getDimension().Height,
            0, upload_format, GL_UNSIGNED_BYTE, texture->lock());
        if (mipmaps)
        {
            std::vector<std::pair<core::dimension2du, unsigned> >
                mipmap_sizes;
            unsigned width = texture->getDimension().Width;
            unsigned height = texture->getDimension().Height;
            mipmap_sizes.emplace_back(core::dimension2du(width, height),
                width * height * 4);
            while (true)
            {
                width = width < 2 ? 1 : width >> 1;
                height = height < 2 ? 1 : height >> 1;
                mipmap_sizes.emplace_back
                    (core::dimension2du(width, height), width * height * 4);
                if (width == 1 && height == 1)
                {
                    break;
                }
            }
            uint8_t* ptr = (uint8_t*)mipmaps->lock();
            for (unsigned i = 1; i < mipmap_sizes.size(); i++)
            {
                glTexImage2D(GL_TEXTURE_2D, i, GL_RGBA,
                    mipmap_sizes[i].first.Width, mipmap_sizes[i].first.Height,
                    0, upload_format, GL_UNSIGNED_BYTE, ptr);
                ptr += mipmap_sizes[i].second;
            }
        }
        else
        {
            glGenerateMipmap(GL_TEXTURE_2D);
        }
        glBindTexture(GL_TEXTURE_2D, 0);
    }
    if (texture)
    {
        m_width.store(texture->getDimension().Width);
        m_height.store(texture->getDimension().Height);
    }
    else
    {
        m_width.store(2);
        m_height.store(2);
    }
#endif
    return true;
}   // texImage2d

// ----------------------------------------------------------------------------
bool SPTexture::saveCompressedTexture(std::shared_ptr<video::IImage> texture,
                                      const std::vector<std::pair
                                      <core::dimension2du, unsigned> >& sizes,
                                      const std::string& cache_location)
{
#if !defined(SERVER_ONLY)
    const unsigned total_size = std::accumulate(sizes.begin(), sizes.end(), 0,
        [] (const unsigned int previous, const std::pair
        <core::dimension2du, unsigned>& cur_sizes)
       { return previous + cur_sizes.second; });
    io::IWriteFile* file = irr::io::createWriteFile(cache_location.c_str(),
        false);
    if (file == NULL)
    {
        return true;
    }
    file->write(&CACHE_VERSION, 1);
    const unsigned mm_sizes = (unsigned)sizes.size();
    file->write(&mm_sizes, 4);
    for (auto& p : sizes)
    {
        file->write(&p.first.Width, 4);
        file->write(&p.first.Height, 4);
        file->write(&p.second, 4);
    }
    file->write(texture->lock(), total_size);
    file->drop();
#endif
    return true;
}   // saveCompressedTexture

// ----------------------------------------------------------------------------
bool SPTexture::useTextureCache(const std::string& full_path,
                                std::string* cache_loc)
{
#ifndef SERVER_ONLY
    if (!CVS->isTextureCompressionEnabled() || m_cache_directory.empty())
    {
        return false;
    }

    std::string basename = StringUtils::getBasename(m_path);
    *cache_loc = m_cache_directory + "/" + basename + ".sptz";

    if (file_manager->fileExists(*cache_loc) &&
        file_manager->fileIsNewer(*cache_loc, m_path))
    {
        if (m_material && (!m_material->getColorizationMask().empty() ||
            m_material->getAlphaMask().empty()))
        {
            std::string mask_path = StringUtils::getPath(m_path) + "/" +
                (!m_material->getColorizationMask().empty() ?
                m_material->getColorizationMask() :
                m_material->getAlphaMask());
            if (!file_manager->fileIsNewer(*cache_loc, mask_path))
            {
                return false;
            }
        }
        return true;
    }
#endif
    return false;
}   // useTextureCache

// ----------------------------------------------------------------------------
std::shared_ptr<video::IImage> SPTexture::getTextureCache(const std::string& p,
    std::vector<std::pair<core::dimension2du, unsigned> >* sizes)
{
    std::shared_ptr<video::IImage> cache;
#if !defined(SERVER_ONLY)
    io::IReadFile* file = irr::io::createReadFile(p.c_str());
    if (file == NULL)
    {
        return cache;
    }

    uint8_t cache_version;
    file->read(&cache_version, 1);
    if (cache_version != CACHE_VERSION)
    {
        return cache;
    }

    unsigned mm_sizes;
    file->read(&mm_sizes, 4);
    sizes->resize(mm_sizes);
    for (unsigned i = 0; i < mm_sizes; i++)
    {
        file->read(&((*sizes)[i].first.Width), 4);
        file->read(&((*sizes)[i].first.Height), 4);
        file->read(&((*sizes)[i].second), 4);
    }

    const unsigned total_cache_size = std::accumulate(sizes->begin(),
        sizes->end(), 0,[] (const unsigned int previous, const std::pair
        <core::dimension2du, unsigned>& cur_sizes)
       { return previous + cur_sizes.second; });
    cache.reset(irr_driver->getVideoDriver()->createImage(video::ECF_A8R8G8B8,
        (*sizes)[0].first));
    assert(cache->getReferenceCount() == 1);
    file->read(cache->lock(), total_cache_size);
    file->drop();
#endif
    return cache;
}   // getTextureCache

// ----------------------------------------------------------------------------
bool SPTexture::threadedLoad()
{
#ifndef SERVER_ONLY
    std::string cache_loc;
    if (useTextureCache(m_path, &cache_loc))
    {
        std::vector<std::pair<core::dimension2du, unsigned> > sizes;
        std::shared_ptr<video::IImage> cache = getTextureCache(cache_loc,
            &sizes);
        if (cache)
        {
            SPTextureManager::get()->increaseGLCommandFunctionCount(1);
            SPTextureManager::get()->addGLCommandFunction(
                [this, cache, sizes]()->bool
                { return compressedTexImage2d(cache, sizes); });
            return true;
        }
    }

    std::shared_ptr<video::IImage> image = getTextureImage();
    if (!image)
    {
        m_width.store(2);
        m_height.store(2);
        return true;
    }
    std::shared_ptr<video::IImage> mask = getMask(image->getDimension());
    if (mask)
    {
        applyMask(image.get(), mask.get());
    }
    std::shared_ptr<video::IImage> mipmaps;

    if (!m_cache_directory.empty() && CVS->isTextureCompressionEnabled() &&
        image->getDimension().Width >= 4 && image->getDimension().Height >= 4)
    {
        auto r = compressTexture(image);
        SPTextureManager::get()->increaseGLCommandFunctionCount(1);
        SPTextureManager::get()->addGLCommandFunction(
            [this, image, r]()->bool
            { return compressedTexImage2d(image, r); });
        if (!cache_loc.empty())
        {
            SPTextureManager::get()->addThreadedFunction(
                [this, image, r, cache_loc]()->bool
                {
                    return saveCompressedTexture(image, r, cache_loc);
                });
        }
    }
    else
    {
        if (UserConfigParams::m_hq_mipmap && image->getDimension().Width > 1 &&
            image->getDimension().Height > 1)
        {
            std::vector<std::pair<core::dimension2du, unsigned> >
                mipmap_sizes;
            unsigned width = image->getDimension().Width;
            unsigned height = image->getDimension().Height;
            mipmap_sizes.emplace_back(core::dimension2du(width, height),
                0);
            while (true)
            {
                width = width < 2 ? 1 : width >> 1;
                height = height < 2 ? 1 : height >> 1;
                mipmap_sizes.emplace_back
                    (core::dimension2du(width, height), 0);
                if (width == 1 && height == 1)
                {
                    break;
                }
            }
            mipmaps.reset(irr_driver->getVideoDriver()->createImage
                (video::ECF_A8R8G8B8, mipmap_sizes[0].first));
            generateHQMipmap(image->lock(), mipmap_sizes,
                (uint8_t*)mipmaps->lock());
        }
        SPTextureManager::get()->increaseGLCommandFunctionCount(1);
        SPTextureManager::get()->addGLCommandFunction(
            [this, image, mipmaps]()->bool
            { return texImage2d(image, mipmaps); });
    }

#endif
    return true;
}   // threadedLoad

// ----------------------------------------------------------------------------
std::shared_ptr<video::IImage>
    SPTexture::getMask(const core::dimension2du& s) const
{
#ifndef SERVER_ONLY
    if (!m_material)
    {
        return NULL;
    }
    const unsigned total_size = s.Width * s.Height;
    if (!m_material->getColorizationMask().empty() ||
        m_material->getColorizationFactor() > 0.0f ||
        m_material->isColorizable())
    {
        // Load colorization mask
        std::shared_ptr<video::IImage> mask;
        std::shared_ptr<SPShader> sps =
            SPShaderManager::get()->getSPShader(m_material->getShaderName());
        if (sps && sps->useAlphaChannel())
        {
            Log::debug("SPTexture", "Don't use colorization mask or factor"
                " with shader using alpha channel for %s", m_path.c_str());
            // Shader using alpha channel will be colorized as a whole
            return NULL;
        }

        uint8_t colorization_factor_encoded = uint8_t
            (irr::core::clamp(
            int(m_material->getColorizationFactor() * 0.4f * 255.0f), 0, 255));

        if (!m_material->getColorizationMask().empty())
        {
            // Assume all maskes are in the same directory
            std::string mask_path = StringUtils::getPath(m_path) + "/" +
                m_material->getColorizationMask();
            mask = getImageFromPath(mask_path);
            if (!mask)
            {
                return NULL;
            }
            core::dimension2du img_size = mask->getDimension();
            if (mask->getColorFormat() != video::ECF_A8R8G8B8 ||
                s != img_size)
            {
                video::IImage* new_mask = irr_driver
                    ->getVideoDriver()->createImage(video::ECF_A8R8G8B8, s);
                if (s != img_size)
                {
                    mask->copyToScaling(new_mask);
                }
                else
                {
                    mask->copyTo(new_mask);
                }
                assert(new_mask->getReferenceCount() == 1);
                mask.reset(new_mask);
            }
        }
        else
        {
            video::IImage* tmp = irr_driver
                ->getVideoDriver()->createImage(video::ECF_A8R8G8B8, s);
            memset(tmp->lock(), 0, total_size * 4);
            assert(tmp->getReferenceCount() == 1);
            mask.reset(tmp);
        }
        uint8_t* data = (uint8_t*)mask->lock();
        for (unsigned int i = 0; i < total_size; i++)
        {
            if (!m_material->getColorizationMask().empty()
                && data[i * 4 + 3] > 127)
            {
                continue;
            }
            data[i * 4 + 3] = colorization_factor_encoded;
        }
        return mask;
    }
    else if (!m_material->getAlphaMask().empty())
    {
        std::string mask_path = StringUtils::getPath(m_path) + "/" +
            m_material->getAlphaMask();
        std::shared_ptr<video::IImage> mask = getImageFromPath(mask_path);
        if (!mask)
        {
            return NULL;
        }
        core::dimension2du img_size = mask->getDimension();
        if (mask->getColorFormat() != video::ECF_A8R8G8B8 ||
            s != img_size)
        {
            video::IImage* new_mask = irr_driver
                ->getVideoDriver()->createImage(video::ECF_A8R8G8B8, s);
            if (s != img_size)
            {
                mask->copyToScaling(new_mask);
            }
            else
            {
                mask->copyTo(new_mask);
            }
            assert(new_mask->getReferenceCount() == 1);
            mask.reset(new_mask);
        }
        uint8_t* data = (uint8_t*)mask->lock();
        for (unsigned int i = 0; i < total_size; i++)
        {
            // Red channel to alpha channel
            data[i * 4 + 3] = data[i * 4];
        }
        return mask;
    }
#endif
    return NULL;
}   // getMask

// ----------------------------------------------------------------------------
void SPTexture::applyMask(video::IImage* texture, video::IImage* mask)
{
    assert(texture->getDimension() == mask->getDimension());
    const core::dimension2du& dim = texture->getDimension();
    for (unsigned int x = 0; x < dim.Width; x++)
    {
        for (unsigned int y = 0; y < dim.Height; y++)
        {
            video::SColor col = texture->getPixel(x, y);
            video::SColor alpha = mask->getPixel(x, y);
            col.setAlpha(alpha.getAlpha());
            texture->setPixel(x, y, col, false);
        }
    }
}   // applyMask

// ----------------------------------------------------------------------------
void SPTexture::generateQuickMipmap(std::shared_ptr<video::IImage> first_image,
                                    const std::vector<std::pair
                                    <core::dimension2du, unsigned> >& mms,
                                    uint8_t* out)
{
#ifndef SERVER_ONLY
    for (unsigned mip = 1; mip < mms.size(); mip++)
    {
        video::IImage* ti = irr_driver->getVideoDriver()
            ->createImage(video::ECF_A8R8G8B8,
            core::dimension2du(mms[mip].first.Width,
            mms[mip].first.Height));
        first_image->copyToScaling(ti);
        const unsigned copy_size = ti->getDimension().getArea() * 4;
        memcpy(out, ti->lock(), copy_size);
        ti->drop();
        out += copy_size;
    }
#endif
}   // generateQuickMipmap

// ----------------------------------------------------------------------------
void SPTexture::generateHQMipmap(void* in,
                                 const std::vector<std::pair
                                 <core::dimension2du, unsigned> >& mms,
                                 uint8_t* out)
{
#if !defined(SERVER_ONLY)
    imMipmapCascade cascade;
    imReduceOptions options;
    imReduceSetOptions(&options,
        m_path.find("_Normal.") != std::string::npos ?
        IM_REDUCE_FILTER_NORMALMAP: IM_REDUCE_FILTER_LINEAR/*filter*/,
        2/*hopcount*/, 2.0f/*alpha*/, 1.0f/*amplifynormal*/,
        0.0f/*normalsustainfactor*/);
#ifdef DEBUG
    int ret = imBuildMipmapCascade(&cascade, in, mms[0].first.Width,
        mms[0].first.Height, 1/*layercount*/, 4, mms[0].first.Width * 4,
        &options, 0);
    assert(ret == 1);
#else
    imBuildMipmapCascade(&cascade, in, mms[0].first.Width,
        mms[0].first.Height, 1/*layercount*/, 4, mms[0].first.Width * 4,
        &options, 0);
#endif
    for (unsigned int i = 1; i < mms.size(); i++)
    {
        const unsigned copy_size = mms[i].first.getArea() * 4;
        memcpy(out, cascade.mipmap[i], copy_size);
        out += copy_size;
    }
    imFreeMipmapCascade(&cascade);
#endif
}   // generateHQMipmap

// ============================================================================
extern "C" void squishCompressImage(uint8_t* rgba, int width, int height,
                                    int pitch, void* blocks, unsigned flags);
// ----------------------------------------------------------------------------
std::vector<std::pair<core::dimension2du, unsigned> >
               SPTexture::compressTexture(std::shared_ptr<video::IImage>& image)
{
    std::vector<std::pair<core::dimension2du, unsigned> > mipmap_sizes;

#if !defined(SERVER_ONLY)
    unsigned width = image->getDimension().Width;
    unsigned height = image->getDimension().Height;
    mipmap_sizes.emplace_back(core::dimension2du(width, height), 0);
    while (true)
    {
        width = width < 2 ? 1 : width >> 1;
        height = height < 2 ? 1 : height >> 1;
        mipmap_sizes.emplace_back(core::dimension2du(width, height), 0);
        if (width == 1 && height == 1)
        {
            break;
        }
    }

    const unsigned tc_flag = squish::kDxt5 | stk_config->m_tc_quality;
    for (auto& size : mipmap_sizes)
    {
        size.second = squish::GetStorageRequirements(
            size.first.Width, size.first.Height, tc_flag);
    }
    const unsigned total_size = std::accumulate(mipmap_sizes.begin(),
        mipmap_sizes.end(), 0,
        [] (const unsigned int previous, const std::pair
        <core::dimension2du, unsigned>& cur_sizes)
       { return previous + cur_sizes.second; });
    video::IImage* c = irr_driver->getVideoDriver()->
        createImage(video::ECF_A8R8G8B8,core::dimension2du(total_size / 4, 1));
    assert(c->getReferenceCount() == 1);
    std::shared_ptr<video::IImage> compressed(c);

    uint8_t* mipmaps = new uint8_t[image->getDimension().getArea() * 4]();
    uint8_t* mipmaps_loc = mipmaps;
    uint8_t* compressed_loc = (uint8_t*)compressed->lock();
    squishCompressImage((uint8_t*)image->lock(),
        mipmap_sizes[0].first.Width, mipmap_sizes[0].first.Height,
        mipmap_sizes[0].first.Width * 4, compressed->lock(), tc_flag);

    // Now compress mipmap
    generateHQMipmap(image->lock(), mipmap_sizes, mipmaps);
    compressed_loc += mipmap_sizes[0].second;
    for (unsigned mip = 1; mip < mipmap_sizes.size(); mip++)
    {
        squishCompressImage(mipmaps_loc,
            mipmap_sizes[mip].first.Width, mipmap_sizes[mip].first.Height,
            mipmap_sizes[mip].first.Width * 4, compressed_loc, tc_flag);
        mipmaps_loc += mipmap_sizes[mip].first.Width *
            mipmap_sizes[mip].first.Height * 4;
        compressed_loc += mipmap_sizes[mip].second;
    }

    delete [] mipmaps;
    image.swap(compressed);
#endif
    return mipmap_sizes;
}

}
