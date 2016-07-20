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

#include "font/font_manager.hpp"

#include "font/bold_face.hpp"
#include "font/digit_face.hpp"
#include "font/regular_face.hpp"

FontManager *font_manager = NULL;
// ----------------------------------------------------------------------------
FontManager::FontManager()
{
    checkFTError(FT_Init_FreeType(&m_ft_library), "loading freetype library");
}   // FontManager

// ----------------------------------------------------------------------------
FontManager::~FontManager()
{
    m_fonts.clearAndDeleteAll();
    checkFTError(FT_Done_FreeType(m_ft_library), "removing freetype library");
    m_ft_library = NULL;
}   // ~FontManager

// ----------------------------------------------------------------------------
void FontManager::loadFonts()
{
    RegularFace* regular = new RegularFace();
    regular->init();
    m_fonts.push_back(regular);
    BoldFace* bold = new BoldFace();
    bold->init();
    m_fonts.push_back(bold);
    DigitFace* digit = new DigitFace();
    digit->init();
    m_fonts.push_back(digit);
}   // loadFonts

// ----------------------------------------------------------------------------
void FontManager::checkFTError(FT_Error err, const std::string& desc) const
{
    if (err > 0)
    {
        Log::error("FontManager", "Something wrong when %s!", desc.c_str());
    }
}   // checkFTError
