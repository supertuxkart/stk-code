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

#include <ft2build.h>
#include FT_FREETYPE_H

#define FONTNUM 5

/**
 * \ingroup guiengine
 */
namespace GUIEngine
{
    /**
     * \brief Initialize a freetype environment with a single freetype library.
     */
    class FTEnvironment
    {
    public:
        FTEnvironment();
        ~FTEnvironment();
        FT_Face           ft_face[FONTNUM];

    private:
        /** Load font face into memory, but don't create glyph yet.
         */
        void              loadFont();

        static FT_Library ft_lib;
        static FT_Error   ft_err;
    };

}   // guiengine
