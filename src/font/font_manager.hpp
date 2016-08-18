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

#ifndef HEADER_FONT_MANAGER_HPP
#define HEADER_FONT_MANAGER_HPP

#include "utils/leak_check.hpp"
#include "utils/log.hpp"
#include "utils/no_copy.hpp"
#include "utils/ptr_vector.hpp"

#include <string>
#include <typeindex>
#include <unordered_map>

#include <ft2build.h>
#include FT_FREETYPE_H

class FaceTTF;
class FontWithFace;

class FontManager : public NoCopy
{
private:
    PtrVector<FontWithFace>                  m_fonts;

    FT_Library                               m_ft_library;

    FaceTTF*                                 m_normal_ttf;

    FaceTTF*                                 m_digit_ttf;

    std::unordered_map<std::type_index, int> m_font_type_map;

public:
    LEAK_CHECK()
    // ------------------------------------------------------------------------
    FontManager();
    // ------------------------------------------------------------------------
    ~FontManager();
    // ------------------------------------------------------------------------
    template <typename T> T* getFont()
    {
        T* out = NULL;
        const unsigned int n = m_font_type_map[std::type_index(typeid(T))];
        out = dynamic_cast<T*>(m_fonts.get(n));
        if (out != NULL)
        {
            return out;
        }
        Log::fatal("FontManager", "Can't get a font!");
        return out;
    }
    // ------------------------------------------------------------------------
    /** Check for any error discovered in a freetype function that will return
     *  a FT_Error value.
     *  \param err The Freetype function.
     *  \param desc The description of what is the function doing.
     */
    void checkFTError(FT_Error err, const std::string& desc) const;
    // ------------------------------------------------------------------------
    void loadFonts();
    // ------------------------------------------------------------------------
    void unitTesting();
    // ------------------------------------------------------------------------
    FT_Library getFTLibrary() const                    { return m_ft_library; }

};   // FontManager

extern FontManager *font_manager;

#endif
/* EOF */
