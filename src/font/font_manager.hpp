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

/** \defgroup font Font
 *  This module stores font files and tools used to draw characters in STK.
 */

#include "utils/leak_check.hpp"
#include "utils/log.hpp"
#include "utils/no_copy.hpp"

#include <string>
#include <map>
#include <typeindex>
#include <unordered_map>
#include <vector>

#ifndef SERVER_ONLY
#include <ft2build.h>
#include <harfbuzz/hb.h>
#include FT_FREETYPE_H

#include "irrString.h"
#include "GlyphLayout.h"
#endif

class FontWithFace;

/** This class stores all font files required in STK.
 *  \ingroup font
 */
class FontManager : public NoCopy
{
private:
    /** Stores all \ref FontWithFace used in STK. */
    std::vector<FontWithFace*>               m_fonts;

#ifndef SERVER_ONLY
    /** A FreeType library, it holds the FT_Face internally inside freetype. */
    FT_Library                               m_ft_library;

    /** List of TTF files for complex text shaping. */
    std::vector<FT_Face>                     m_faces;

    /** TTF file for digit font in STK. */
    FT_Face                                  m_digit_face;

    /** DPI used when shaping, each face will apply an inverse of this and the
     *  dpi of itself when rendering, so all faces can share the same glyph
     *  layout cache. */
    unsigned m_shaping_dpi;

    /** Map FT_Face to index for quicker layout. */
    std::map<FT_Face, uint16_t> m_ft_faces_to_index;

    /** Text drawn to glyph layouts cache. */
    std::map<irr::core::stringw,
        std::vector<irr::gui::GlyphLayout> > m_cached_gls;

    bool m_has_color_emoji;
    // ------------------------------------------------------------------------
    std::vector<FT_Face> loadTTF(const std::vector<std::string>& ttf_list);

    hb_buffer_t* m_hb_buffer;

    std::vector<hb_font_t*> m_hb_fonts;
#endif

    /** Map type for each \ref FontWithFace with a index, save getting time in
     *  \ref getFont. */
    std::unordered_map<std::type_index, int> m_font_type_map;
public:
    LEAK_CHECK()
    // ------------------------------------------------------------------------
    FontManager();
    // ------------------------------------------------------------------------
    ~FontManager();
    // ------------------------------------------------------------------------
    /** Return a specfic type of \ref FontWithFace found in \ref m_fonts. */
    template <typename T> T* getFont()
    {
        T* out = NULL;
        const unsigned int n = m_font_type_map[std::type_index(typeid(T))];
        out = dynamic_cast<T*>(m_fonts[n]);
        if (out != NULL)
        {
            return out;
        }
        Log::fatal("FontManager", "Can't get a font!");
        return out;
    }
    // ------------------------------------------------------------------------
#ifndef SERVER_ONLY
    /** Check for any error discovered in a freetype function that will return
     *  a FT_Error value, and log into the terminal.
     *  \param err The Freetype function.
     *  \param desc The description of what is the function doing. */
    void checkFTError(FT_Error err, const std::string& desc) const
    {
        if (err > 0)
        {
            Log::error("FontManager", "Something wrong when %s! The error "
                "code was %d.", desc.c_str(), err);
        }
    }
    // ------------------------------------------------------------------------
    bool hasColorEmoji() const                    { return m_has_color_emoji; }
    // ------------------------------------------------------------------------
    /** Return the \ref m_ft_library. */
    FT_Library getFTLibrary() const                    { return m_ft_library; }
    // ------------------------------------------------------------------------
    FT_Face loadColorEmoji();
    // ------------------------------------------------------------------------
    unsigned getShapingDPI() const                    { return m_shaping_dpi; }
    // ------------------------------------------------------------------------
    void shape(const std::u32string& text,
               std::vector<irr::gui::GlyphLayout>& gls,
               irr::u32 shape_flag = 0);
    // ------------------------------------------------------------------------
    std::vector<irr::gui::GlyphLayout>& getCachedLayouts
                  (const irr::core::stringw& str);
    // ------------------------------------------------------------------------
    void clearCachedLayouts()                         { m_cached_gls.clear(); }
    // ------------------------------------------------------------------------
    void initGlyphLayouts(const irr::core::stringw& text,
                          std::vector<irr::gui::GlyphLayout>& gls,
                          irr::u32 shape_flag = 0);
#endif
    // ------------------------------------------------------------------------
    void loadFonts();
    // ------------------------------------------------------------------------
    void unitTesting();

};   // FontManager

extern FontManager *font_manager;

#endif
/* EOF */
