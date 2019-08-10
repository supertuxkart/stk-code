//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2016 SuperTuxKart-Team
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

#include "font/digit_face.hpp"

// ----------------------------------------------------------------------------
/** Constructor of DigitFace.
 *  \param ttf \ref FaceTTF for DigitFace to use.
 */
DigitFace::DigitFace() : FontWithFace("DigitFace")
{
}   // DigitFace

// ----------------------------------------------------------------------------
void DigitFace::init()
{
    FontWithFace::init();
    // Reserve some space for characters added later in the next line
    m_font_max_height = m_glyph_max_height * 7 / 5;
}   // init

// ----------------------------------------------------------------------------
void DigitFace::reset()
{
    FontWithFace::reset();

    core::stringw preload_chars;
    for (int i = 32; i < 64; i++)
    {
        // No lazy loading for digit font, include the least characters
        preload_chars.append((wchar_t)i);
    }
    // Used when displaying multiple items, e.g. 6x
    preload_chars.append((wchar_t)120);

    insertCharacters(preload_chars.c_str(), true/*first_load*/);
    updateCharactersList();
}   // reset
