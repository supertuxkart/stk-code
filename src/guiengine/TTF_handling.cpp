//
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
#include "io/file_manager.hpp"
#include "guiengine/TTF_handling.hpp"
#include "graphics/irr_driver.hpp"
#include <algorithm>

namespace irr
{
namespace gui
{
TTFfile getTTFAndChar(const std::string &langname, TTFLoadingType type, FontUse& fu)
{
    //Borrowed from engine.cpp:
    // font size is resolution-dependent.
    // normal text will range from 0.8, in 640x* resolutions (won't scale
    // below that) to 1.0, in 1024x* resolutions, and linearly up
    // normal text will range from 0.2, in 640x* resolutions (won't scale
    // below that) to 0.4, in 1024x* resolutions, and linearly up
    const int screen_width = irr_driver->getFrameSize().Width;
    const int screen_height = irr_driver->getFrameSize().Height;
    float scale = std::max(0, screen_width - 640)/564.0f;

    // attempt to compensate for small screens
    if (screen_width < 1200) scale = std::max(0, screen_width - 640) / 750.0f;
    if (screen_width < 900 || screen_height < 700) scale = std::min(scale, 0.05f);

    float normal_text_scale = 0.7f + 0.2f*scale;
    float title_text_scale = 0.2f + 0.2f*scale;

    TTFfile ttf_file;
    switch(type)
    {
        case Normal:
            loadChar(langname, &ttf_file, fu, normal_text_scale);
            break;
        case Digit:
            fu = F_DIGIT;
            loadNumber(&ttf_file, normal_text_scale);
            break;
        case Bold:
            fu = F_BOLD;
            loadBoldChar(&ttf_file, title_text_scale);
            break;
    }
    return ttf_file;
}

void loadChar(const std::string langname, TTFfile* ttf_file, FontUse& fu, float scale)
{
    //Determine which ttf file to load first
    if (langname.compare("ar") == 0 || langname.compare("fa") == 0)
        fu = F_AR;

    else if (langname.compare("sq") == 0 || langname.compare("eu") == 0
          || langname.compare("br") == 0 || langname.compare("da") == 0
          || langname.compare("nl") == 0 || langname.compare("en") == 0
          || langname.compare("gd") == 0 || langname.compare("gl") == 0
          || langname.compare("de") == 0 || langname.compare("is") == 0
          || langname.compare("id") == 0 || langname.compare("it") == 0
          || langname.compare("nb") == 0 || langname.compare("nn") == 0
          || langname.compare("pt") == 0 || langname.compare("es") == 0
          || langname.compare("sv") == 0 || langname.compare("uz") == 0)
        //They are sorted out by running fc-list :lang="name" |grep Layne
        fu = F_LAYNE;

    else if (langname.compare("zh") == 0 || langname.compare("ko") == 0
          || langname.compare("ja") == 0)
        fu = F_CJK;

    else
        fu = F_DEFAULT; //Default font file

    ttf_file->size = (int)(29*scale); //Set to default size

    ttf_file->usedchar = translations->getCurrentAllChar(); //Loading unique characters
    for (int i = 33; i < 256; ++i)
        ttf_file->usedchar.insert((wchar_t)i); //Include basic Latin too
    ttf_file->usedchar.insert((wchar_t)160);   //Non-breaking space
    ttf_file->usedchar.insert((wchar_t)215);   //Used on resolution selection screen (X).

    //There's specific handling for some language, we may need more after more translation are added or problems found out.
    if (langname.compare("el") == 0)
        ttf_file->size = (int)(28*scale); //Set lower size of font for Greek as it uses lots amount of space.
}

void loadNumber(TTFfile* ttf_file, float scale)
{
    ttf_file->size = (int)(40*scale); //Set default size for Big Digit Text
    for (int i = 46; i < 59; ++i) //Include chars used by timer and laps count only
        ttf_file->usedchar.insert((wchar_t)i); //FIXME have to load 46 " . " to make 47 " / " display correctly, why?
}

void loadBoldChar(TTFfile* ttf_file, float scale)
{
    ttf_file->size = (int)(120*scale); //Set default size for Bold Text
    for (int i = 33; i < 256; ++i)
        ttf_file->usedchar.insert((wchar_t)i);

    setlocale(LC_ALL, "en_US.UTF8");
    std::set<wchar_t>::iterator it = ttf_file->usedchar.begin();
    while (it != ttf_file->usedchar.end())
    {
        if (iswlower((wchar_t)*it))
            it = ttf_file->usedchar.erase(it);
        else
            ++it;
    }
}

video::IImage* generateTTFImage(FT_Bitmap bits, video::IVideoDriver* Driver)
{
    core::dimension2du d(bits.width + 1, bits.rows + 1);
    core::dimension2du texture_size;

    video::IImage* image = 0;
    switch (bits.pixel_mode)
    {
        case FT_PIXEL_MODE_MONO:
        {
            // Create a blank image and fill it with transparent pixels.
            texture_size = d.getOptimalSize(true, true);
            image = Driver->createImage(video::ECF_A1R5G5B5, texture_size);
            image->fill(video::SColor(0, 255, 255, 255));

            // Load the monochrome data in.
            const u32 image_pitch = image->getPitch() / sizeof(u16);
            u16* image_data = (u16*)image->lock();
            u8* glyph_data = bits.buffer;
            for (u32 y = 0; y < bits.rows; ++y)
            {
                u16* row = image_data;
                for (u32 x = 0; x < bits.width; ++x)
                {
                    // Monochrome bitmaps store 8 pixels per byte.  The left-most pixel is the bit 0x80.
                    // So, we go through the data each bit at a time.
                    if ((glyph_data[y * bits.pitch + (x / 8)] & (0x80 >> (x % 8))) != 0)
                        *row = 0xFFFF;
                    ++row;
                }
                image_data += image_pitch;
            }
            image->unlock();
            break;
        }

        case FT_PIXEL_MODE_GRAY:
        {
            // Create our blank image.
            texture_size = d.getOptimalSize(true, true, true, 0); //Enable force power of 2 texture size to gain performance,
                                                                  //But may increase vram usage???
            image = Driver->createImage(video::ECF_A8R8G8B8, texture_size);
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
            Log::error("ScalableFont::loadTTF", "Freetype failed to create bitmap!!");
            return 0;
    }
    return image;
}

} // end namespace gui
} // end namespace irr
#endif // ENABLE_FREETYPE
