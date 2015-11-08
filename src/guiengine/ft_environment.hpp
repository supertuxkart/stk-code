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

#ifndef HEADER_FT_ENVIRONMENT_HPP
#define HEADER_FT_ENVIRONMENT_HPP

#include <ft2build.h>
#include FT_FREETYPE_H
#include <irrlicht.h>

#include "utils/leak_check.hpp"

/**
 * \ingroup guiengine
 */
namespace GUIEngine
{

enum FontUse
{
    F_DEFAULT = 0,
    F_DEFAULT_FALLBACK = 1,
    F_CJK = 2,
    F_AR = 3,
    F_LAST_REGULAR_FONT = F_AR,

    F_BOLD = 4,
    F_BOLD_FALLBACK = 5,
    F_DIGIT = 6,
    F_COUNT = 7
};

enum TTFLoadingType {T_NORMAL, T_DIGIT, T_BOLD};

    /**
     * \brief Initialize a freetype environment with a single freetype library.
     */
    class FTEnvironment
    {
    public:

        LEAK_CHECK()

        FTEnvironment();
        ~FTEnvironment();

        /** Get a face with a suitable font type.
         */
        FT_Face           getFace(const FontUse font);

    private:
        /** Check for any error discovered in a freetype function that will return a FT_Error value.
         *  \param err The Freetype function.
         *  \param desc The description of what is the function doing.
         */
        void              checkError(FT_Error err, const irr::core::stringc desc);

        /** Load font face into memory, but don't create glyph yet.
         */
        void              loadFont();

        FT_Face           m_ft_face[F_COUNT];
        static FT_Library m_ft_lib;
    };

}   // guiengine
#endif // HEADER_FT_ENVIRONMENT_HPP
