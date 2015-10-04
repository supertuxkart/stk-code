//
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

#include "utils/translation.hpp"
#include <IVideoDriver.h>
#include <ft2build.h>
#include FT_FREETYPE_H

#include <cwctype>
#include <clocale>

namespace irr
{
namespace gui
{

enum TTFLoadingType {Normal, Digit, Bold};

enum FontUse {F_DEFAULT, F_CJK, F_AR, F_LAYNE, F_BOLD, F_DIGIT};

typedef struct
{
    std::set<wchar_t> usedchar;
    unsigned short size;
    std::string filename;
}TTFfile;

TTFfile getTTFAndChar(const std::string &langname, TTFLoadingType, FontUse&);
video::IImage* generateTTFImage (FT_Bitmap, video::IVideoDriver*);
void loadChar(const std::string, TTFfile*, FontUse&, float);
void loadNumber(TTFfile*, float);
void loadBoldChar(TTFfile*, float);

} // end namespace gui
} // end namespace irr
