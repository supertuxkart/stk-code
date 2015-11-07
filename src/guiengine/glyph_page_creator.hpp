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
#include <set>

#include "utils/leak_check.hpp"

using namespace irr;

/**
 * \ingroup guiengine
 */
namespace GUIEngine
{
    /**
     * \brief Create glyph pages for different fonts.
     */
    class GlyphPageCreator
    {
    public:

        LEAK_CHECK()

        GlyphPageCreator();
        ~GlyphPageCreator();

        /** Write the current glyph page into png on current running directory.
         *  Mainly for debug use.
         *  \param fn The file name.
         */
        void                     dumpGlyphPage(const core::stringc fn);

        /** Check whether it is ok the fit the inputted glyph into the current glyph page.
         *  \param bits The Glyph bitmap inputted.
         *  \return True if there is enough space.
         */
        bool                     checkEnoughSpace(FT_Bitmap bits);

        /** Reset position of glyph on the current glyph page.
         */
        void                     clearGlyphPage();

        /** Reset characters holder for lazy loading char.
         */
        void                     clearNewCharHolder();

        /** Clear (fill it with transparent content) the current glyph page.
         */
        void                     createNewGlyphPage();

        /** Used to get a glyph page which is loaded later for texture
         *  \return Glyph page image.
         */
        video::IImage*           getPage();

        /** Used to get the string of new characters inside set m_new_char_holder for lazy char loading.
         *  \return string of wild-character.
         */
        core::stringw            getNewChar();

        /** Used to insert a single new character into glyph page used for lazy char loading.
         *  \param c A new character.
         */
        void                     insertChar(const wchar_t c);

        /** Used to insert a single glyph bitmap into the glyph page
         *  \param bits The Glyph bitmap inputted.
         *  \param rect Give the rectangle of the glyph on the page.
         *  \return True if a glyph is loaded.
         */
        bool                     insertGlyph(FT_Bitmap bits, core::rect<s32>& rect);

    private:
        /** A temporary storage for a single glyph.
         */
        video::IImage*           m_image;

        /** A temporary holder stored new char to be inserted.
         */
        std::set<wchar_t>        m_new_char_holder;

        /** A full glyph page.
         */
        video::IImage*           m_page;

        u32                      m_temp_height;
        u32                      m_used_width;
        u32                      m_used_height;
    };

}   // guiengine
