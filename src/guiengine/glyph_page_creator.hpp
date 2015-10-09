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

#include <IVideoDriver.h>
#include <irrlicht.h>
#include <ft2build.h>
#include FT_FREETYPE_H

namespace irr
{
namespace gui
{

class GlyphPageCreator
{
public:
    GlyphPageCreator();
    ~GlyphPageCreator();

    static void              dumpGlyphPage(const core::stringc);
    static bool              checkEnoughSpace(FT_Bitmap);
    static void              clearGlyphPage();
    static void              createNewGlyphPage();
    static video::IImage*    getPage();
    bool                     insertGlyph(FT_Bitmap, core::rect<s32>&);

private:
    video::IImage*           image = 0;
    static  video::IImage*   page;
    static  u32              temp_height;
    static  u32              used_width;
    static  u32              used_height;
};

} // end namespace gui
} // end namespace irr
