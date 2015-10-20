//  SuperTuxKart - a fun racing game with go-kart
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

#include "graphics/irr_driver.hpp"
#include "guiengine/get_font_properties.hpp"
#include "utils/translation.hpp"

#include <algorithm>
#include <clocale>
#include <cwctype>

namespace irr
{
namespace gui
{

getFontProperties::getFontProperties (const core::stringc &langname, TTFLoadingType type, FontUse &fu)
{
    findScale();

    switch(type)
    {
        case T_NORMAL:
            loadChar(langname, fu, normal_text_scale);
            break;
        case T_DIGIT:
            fu = F_DIGIT;
            loadNumber(normal_text_scale);
            break;
        case T_BOLD:
            fu = F_BOLD;
            loadBoldChar(title_text_scale);
            break;
    }
}

void getFontProperties::findScale()
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

    normal_text_scale = 0.7f + 0.2f*scale;
    title_text_scale = 0.2f + 0.2f*scale;
}

void getFontProperties::loadChar(const core::stringc langname, FontUse& fu, float scale)
{
    fu = F_DEFAULT; //Default font file

    for (int i = 32; i < 128; ++i)
        usedchar.insert((wchar_t)i); //Include basic Latin too
    usedchar.insert((wchar_t)160);   //Non-breaking space
    usedchar.insert((wchar_t)215);   //Used on resolution selection screen (X).

    //There's specific handling for some language, we may need more after more translation are added or problems found out.
    //if (langname == "el" || langname == "fr" || langname == "gd")
        size = (int)(27*scale); //Lower scale for them as they're space-consuming.
    //else
    //    size = (int)(29*scale); //Set to default size
}

void getFontProperties::loadNumber(float scale)
{
    size = (int)(40*scale); //Set default size for Big Digit Text
    for (int i = 46; i < 59; ++i) //Include chars used by timer and laps count only
        usedchar.insert((wchar_t)i); //FIXME have to load 46 " . " to make 47 " / " display correctly, why?
}

void getFontProperties::loadBoldChar(float scale)
{
    size = (int)(120*scale); //Set default size for Bold Text

    usedchar = translations->getCurrentAllChar(); //Loading unique characters
    for (int i = 65; i < 256; ++i)
        usedchar.insert((wchar_t)i); //Include basic Latin too, starting from A (char code 65)

    setlocale(LC_ALL, "en_US.UTF8");
    std::set<wchar_t>::iterator it = usedchar.begin();
    while (it != usedchar.end())
    {
        //Only use all capital letter for bold char with latin (<640 of char code).
        //Remove all characters (>char code 8191) not used by the title
        if (((iswlower((wchar_t)*it) || !iswalpha((wchar_t)*it)) && *it < 640) || *it > 8191)
            it = usedchar.erase(it);
        else
            ++it;
    }

    //Final hack to make stk display title properly
    for (int i = 32; i < 65; ++i)
        usedchar.insert((wchar_t)i); //Include basic symbol (from space (char code 32) to @(char code 64))
    usedchar.insert((wchar_t)160);   //Non-breaking space

    //Remove Ordinal indicator (char code 170 and 186)
    usedchar.erase((wchar_t)170);
    usedchar.erase((wchar_t)186);

    usedchar.erase((wchar_t)304);    //Remove Capital I-dotted (char code 304) with using "I" altogether.
}

} // end namespace gui
} // end namespace irr
