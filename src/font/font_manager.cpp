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

#include "config/stk_config.hpp"
#include "font/bold_face.hpp"
#include "font/digit_face.hpp"
#include "font/face_ttf.hpp"
#include "font/regular_face.hpp"
#include "utils/string_utils.hpp"
#include "utils/translation.hpp"

FontManager *font_manager = NULL;
// ----------------------------------------------------------------------------
/** Constructor. It will initialize the \ref m_ft_library.
 */
FontManager::FontManager()
{
    checkFTError(FT_Init_FreeType(&m_ft_library), "loading freetype library");
}   // FontManager

// ----------------------------------------------------------------------------
/** Destructor. Clears all fonts and related stuff.
 */
FontManager::~FontManager()
{
    for (unsigned int i = 0; i < m_fonts.size(); i++)
        delete m_fonts[i];
    m_fonts.clear();

    delete m_normal_ttf;
    m_normal_ttf = NULL;
    delete m_digit_ttf;
    m_digit_ttf = NULL;

    checkFTError(FT_Done_FreeType(m_ft_library), "removing freetype library");
    m_ft_library = NULL;
}   // ~FontManager

// ----------------------------------------------------------------------------
/** Initialize all \ref FaceTTF and \ref FontWithFace members.
 */
void FontManager::loadFonts()
{
    // First load the TTF files required by each font
    m_normal_ttf = new FaceTTF(stk_config->m_normal_ttf);
    m_digit_ttf = new FaceTTF(stk_config->m_digit_ttf);

    // Now load fonts with settings of ttf file
    unsigned int font_loaded = 0;
    RegularFace* regular = new RegularFace(m_normal_ttf);
    regular->init();
    m_fonts.push_back(regular);
    m_font_type_map[std::type_index(typeid(RegularFace))] = font_loaded++;

    BoldFace* bold = new BoldFace(m_normal_ttf);
    bold->init();
    m_fonts.push_back(bold);
    m_font_type_map[std::type_index(typeid(BoldFace))] = font_loaded++;

    DigitFace* digit = new DigitFace(m_digit_ttf);
    digit->init();
    m_fonts.push_back(digit);
    m_font_type_map[std::type_index(typeid(DigitFace))] = font_loaded++;
}   // loadFonts

// ----------------------------------------------------------------------------
/** Unit testing that will try to load all translations in STK, and discover if
 *  there is any characters required by it are not supported in \ref
 *  m_normal_ttf.
 */
void FontManager::unitTesting()
{
    std::vector<std::string> list = *(translations->getLanguageList());
    const int cur_log_level = Log::getLogLevel();
    for (const std::string& lang : list)
    {
        // Hide gettext warning
        Log::setLogLevel(5);
        delete translations;
#ifdef WIN32
        std::string s=std::string("LANGUAGE=") + lang.c_str();
        _putenv(s.c_str());
#else
        setenv("LANGUAGE", lang.c_str(), 1);
#endif
        translations = new Translations();
        Log::setLogLevel(cur_log_level);
        std::set<wchar_t> used_chars = translations->getCurrentAllChar();
        for (const wchar_t& c : used_chars)
        {
            // Skip non-printing characters
            if (c < 32) continue;

            unsigned int font_number = 0;
            unsigned int glyph_index = 0;
            while (font_number < m_normal_ttf->getTotalFaces())
            {
                glyph_index =
                    FT_Get_Char_Index(m_normal_ttf->getFace(font_number), c);
                if (glyph_index > 0) break;
                font_number++;
            }
            if (glyph_index > 0)
            {
                Log::debug("UnitTest", "Character %s in language %s"
                    " use face %s",
                    StringUtils::wideToUtf8(core::stringw(&c, 1)).c_str(),
                    lang.c_str(),
                    m_normal_ttf->getFace(font_number)->family_name);
            }
            else
            {
                Log::warn("UnitTest", "Character %s in language %s"
                    " is not supported by all fonts!",
                    StringUtils::wideToUtf8(core::stringw(&c, 1)).c_str(),
                    lang.c_str());
            }
        }
    }

}   // unitTesting
