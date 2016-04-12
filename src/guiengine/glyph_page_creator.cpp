//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2009-2010 John Norman
//  Copyright (C) 2015 Ben Au
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
//
//  The image loading function is partially based on CGUITTFont.cpp by John Norman,
//  original version is located here:
//
//  http://irrlicht.suckerfreegames.com/

#include <irrlicht.h>
#include "guiengine/engine.hpp"

namespace GUIEngine
{

// ----------------------------------------------------------------------------

GlyphPageCreator::GlyphPageCreator()
{
    m_page = GUIEngine::getDriver()->createImage(video::ECF_A8R8G8B8, core::dimension2du(512, 512));
    m_image = 0;
}

// ----------------------------------------------------------------------------

GlyphPageCreator::~GlyphPageCreator()
{
    clearGlyphPage();
    clearNewCharHolder();
    m_page->drop();
    m_page = 0;
}

// ----------------------------------------------------------------------------

void GlyphPageCreator::dumpGlyphPage(const core::stringc fn)
{
    GUIEngine::getDriver()->writeImageToFile(m_page, fn + ".png");
}

// ----------------------------------------------------------------------------

bool GlyphPageCreator::checkEnoughSpace(FT_Bitmap bits)
{
    core::dimension2du d(bits.width + 1, bits.rows + 1);
    core::dimension2du texture_size;
    texture_size = d.getOptimalSize(!(GUIEngine::getDriver()->queryFeature(video::EVDF_TEXTURE_NPOT)),
                                    !(GUIEngine::getDriver()->queryFeature(video::EVDF_TEXTURE_NSQUARE)), true, 0);

    if ((m_used_width + texture_size.Width > 512 && m_used_height + m_temp_height + texture_size.Height > 512)
         || m_used_height + texture_size.Height > 512)
        return false;
    return true;
}

// ----------------------------------------------------------------------------

void GlyphPageCreator::clearNewCharHolder()
{
    m_new_char_holder.clear();
}

// ----------------------------------------------------------------------------

void GlyphPageCreator::clearGlyphPage()
{
    m_used_width  = 0;
    m_temp_height = 0;
    m_used_height = 0;
}

// ----------------------------------------------------------------------------

void GlyphPageCreator::createNewGlyphPage()
{
    //Clean the current glyph page by filling it with transparent content
    m_page->fill(video::SColor(0, 255, 255, 255));
}

// ----------------------------------------------------------------------------

video::IImage* GlyphPageCreator::getPage()
{
    return m_page;
}

// ----------------------------------------------------------------------------

core::stringw GlyphPageCreator::getNewChar()
{
    core::stringw c;
    for (std::set<wchar_t>::iterator it = m_new_char_holder.begin(); it != m_new_char_holder.end(); ++it)
        c += *it;

    return c;
}

// ----------------------------------------------------------------------------

void GlyphPageCreator::insertChar(const wchar_t c)
{
    m_new_char_holder.insert(c);
}

// ----------------------------------------------------------------------------

bool GlyphPageCreator::insertGlyph(FT_Bitmap bits, core::rect<s32>& rect)
{
    core::dimension2du d(bits.width + 1, bits.rows + 1);
    core::dimension2du texture_size;

    switch (bits.pixel_mode)
    {
        case FT_PIXEL_MODE_GRAY:
        {
            // Create our blank image.
            texture_size = d.getOptimalSize(!(GUIEngine::getDriver()->queryFeature(video::EVDF_TEXTURE_NPOT)),
                                            !(GUIEngine::getDriver()->queryFeature(video::EVDF_TEXTURE_NSQUARE)), true, 0);
            m_image = GUIEngine::getDriver()->createImage(video::ECF_A8R8G8B8, texture_size);
            m_image->fill(video::SColor(0, 255, 255, 255));

            // Load the grayscale data in.
            const float gray_count = static_cast<float>(bits.num_grays);
            const u32 image_pitch = m_image->getPitch() / sizeof(u32);
            u32* image_data = (u32*)m_image->lock();
            u8* glyph_data = bits.buffer;
            for (u32 y = 0; y < (unsigned)bits.rows; ++y)
            {
                u8* row = glyph_data;
                for (u32 x = 0; x < (unsigned)bits.width; ++x)
                {
                    image_data[y * image_pitch + x] |= static_cast<u32>(255.0f * (static_cast<float>(*row++) / gray_count)) << 24;
                    //data[y * image_pitch + x] |= ((u32)(*bitsdata++) << 24);
                }
                glyph_data += bits.pitch;
            }
            m_image->unlock();
            break;
        }
        default:
            return false;
    }
    if (!m_image)
        return false;

    //Done creating a single glyph, now copy to the glyph page...
    //Determine the linebreak location
    if (m_used_width + texture_size.Width > 512)
    {
        m_used_width  = 0;
        m_used_height += m_temp_height;
        m_temp_height = 0;
    }

    //Copy now
    m_image->copyTo(m_page, core::position2di(m_used_width, m_used_height));

    //Store the rectangle of current glyph
    rect = core::rect<s32> (m_used_width, m_used_height, m_used_width + bits.width, m_used_height + bits.rows);

    m_image->drop();
    m_image = 0;

    //Store used area
    m_used_width += texture_size.Width;
    if (m_temp_height < texture_size.Height)
        m_temp_height = texture_size.Height;
    return true;
}

}   // guiengine
