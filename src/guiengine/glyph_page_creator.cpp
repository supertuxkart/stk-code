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

#ifdef ENABLE_FREETYPE
#include <irrlicht.h>
#include "guiengine/engine.hpp"

namespace irr
{
namespace gui
{

GlyphPageCreator::GlyphPageCreator()
{
    page = GUIEngine::getDriver()->createImage(video::ECF_A8R8G8B8, core::dimension2du(512, 512));
}

GlyphPageCreator::~GlyphPageCreator()
{
    clearGlyphPage();
    page->drop();
    page = 0;
}

bool GlyphPageCreator::checkEnoughSpace(FT_Bitmap bits)
{
    core::dimension2du d(bits.width + 1, bits.rows + 1);
    core::dimension2du texture_size;
    texture_size = d.getOptimalSize(!(GUIEngine::getDriver()->queryFeature(video::EVDF_TEXTURE_NPOT)),
                                    !(GUIEngine::getDriver()->queryFeature(video::EVDF_TEXTURE_NSQUARE)), true, 0);

    if (used_width + texture_size.Width > 512 && used_height + temp_height + texture_size.Height > 512)
        return false;
    return true;
}

void GlyphPageCreator::clearGlyphPage()
{
    used_width  = 0;
    temp_height = 0;
    used_height = 0;
}

void GlyphPageCreator::createNewGlyphPage()
{
    video::IImage* blank = GUIEngine::getDriver()->createImage(video::ECF_A8R8G8B8, core::dimension2du(512, 512));
    blank->copyTo(page, core::position2di(0, 0));
    blank->drop();
    blank = 0;
}

video::IImage* GlyphPageCreator::getPage()
{
    return page;
}

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
            image = GUIEngine::getDriver()->createImage(video::ECF_A8R8G8B8, texture_size);
            image->fill(video::SColor(0, 255, 255, 255));

            // Load the grayscale data in.
            const float gray_count = static_cast<float>(bits.num_grays);
            const u32 image_pitch = image->getPitch() / sizeof(u32);
            u32* image_data = (u32*)image->lock();
            u8* glyph_data = bits.buffer;
            for (u32 y = 0; y < bits.rows; ++y)
            {
                u8* row = glyph_data;
                for (u32 x = 0; x < bits.width; ++x)
                {
                    image_data[y * image_pitch + x] |= static_cast<u32>(255.0f * (static_cast<float>(*row++) / gray_count)) << 24;
                    //data[y * image_pitch + x] |= ((u32)(*bitsdata++) << 24);
                }
                glyph_data += bits.pitch;
            }
            image->unlock();
            break;
        }
        default:
            return false;
    }
    if (!image)
        return false;

    //Done creating a single glyph, now copy to the glyph page...
    //Determine the linebreak location
    if (used_width + texture_size.Width > 512)
    {
        used_width  = 0;
        used_height += temp_height;
        temp_height = 0;
    }

    //Copy now
    image->copyTo(page, core::position2di(used_width, used_height));

    //Store the rectangle of current glyph
    rect = core::rect<s32> (used_width, used_height, used_width + bits.width, used_height + bits.rows);

    image = 0;

    //Store used area
    used_width += texture_size.Width;
    if (temp_height < texture_size.Height)
        temp_height = texture_size.Height;
    return true;
}

u32 GlyphPageCreator::used_width            = 0;
u32 GlyphPageCreator::used_height           = 0;
u32 GlyphPageCreator::temp_height           = 0;
video::IImage* GlyphPageCreator::page       = 0;

} // end namespace gui
} // end namespace irr
#endif // ENABLE_FREETYPE
