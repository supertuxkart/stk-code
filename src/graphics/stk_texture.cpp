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
#include "io/file_manager.hpp"
#include "utils/string_utils.hpp"
#include "utils/log.hpp"

#include <fstream>
#include <sstream>

// ----------------------------------------------------------------------------
STKTexture::STKTexture(const std::string& path, bool srgb, bool premul_alpha,
                       bool set_material)
          : video::ITexture(path.c_str()), m_texture_handle(0), m_srgb(srgb),
            m_premul_alpha(premul_alpha), m_mesh_texture(set_material),
            m_material(NULL), m_texture_name(0), m_texture_size(0),
            m_texture_image(NULL)
{
    if (set_material)
    {
        m_material = material_manager->getMaterialFor(this);
    }
}   // STKTexture

// ----------------------------------------------------------------------------
STKTexture::STKTexture(video::IImage* image)
          : video::ITexture(""), m_texture_handle(0), m_srgb(false),
            m_premul_alpha(false),
            m_mesh_texture(false), m_texture_name(0), m_texture_size(0),
            m_texture_image(image)
{
    m_size = m_texture_image->getDimension();
    m_orig_size = m_texture_image->getDimension();
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
void STKTexture::reload()
{
#ifndef SERVER_ONLY
    irr_driver->getDevice()->getLogger()->setLogLevel(ELL_NONE);
    video::IImage* orig_img =
        irr_driver->getVideoDriver()->createImageFromFile(NamedPath);
    if (orig_img == NULL)
    {
        Log::warn("STKTexture", "No image %s.", NamedPath.getPtr());
        return;
    }

    if (orig_img->getDimension().Width  == 0 ||
        orig_img->getDimension().Height == 0)
    {
        Log::warn("STKTexture", "image %s has 0 size.", NamedPath.getPtr());
        return;
    }

    video::IImage* new_texture = convertImage(&orig_img);
    applyMask(new_texture);

    const bool reload = m_texture_name != 0;
    if (!reload)
        glGenTextures(1, &m_texture_name);

    glBindTexture(GL_TEXTURE_2D, m_texture_name);
    if (!reload)
    {
        glTexImage2D(GL_TEXTURE_2D, 0, m_srgb ? GL_SRGB_ALPHA : GL_RGBA,
            new_texture->getDimension().Width,
            new_texture->getDimension().Height, 0, GL_BGRA, GL_UNSIGNED_BYTE,
            new_texture->lock());
    }
    else
    {
        glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0,
            new_texture->getDimension().Width,
            new_texture->getDimension().Height, GL_BGRA, GL_UNSIGNED_BYTE,
            new_texture->lock());
    }
    new_texture->unlock();
    m_size = new_texture->getDimension();
    m_orig_size = orig_img->getDimension();
    m_texture_size = m_size.Width * m_size.Height * 4 /*BRGA*/;
    new_texture->drop();
    orig_img->drop();
    glGenerateMipmap(GL_TEXTURE_2D);
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
    }
#endif   // !SERVER_ONLY
}   // applyMask
