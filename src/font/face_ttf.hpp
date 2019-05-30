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

#ifndef HEADER_FACE_TTF_HPP
#define HEADER_FACE_TTF_HPP

#include "utils/leak_check.hpp"
#include "utils/no_copy.hpp"

#include <cassert>
#include <map>
#include <string>
#include <utility>
#include <vector>

#ifndef SERVER_ONLY
#include <ft2build.h>
#include FT_FREETYPE_H
#endif

/** Glyph metrics for each glyph loaded. */
struct FontArea
{
    FontArea() : advance_x(0), bearing_x(0), offset_y(0), offset_y_bt(0),
                 spriteno(0) {}
    /** Advance width for horizontal layout. */
    int advance_x;
    /** Left side bearing for horizontal layout. */
    int bearing_x;
    /** Top side bearing for horizontal layout. */
    int offset_y;
    /** Top side bearing for horizontal layout used in billboard text. */
    int offset_y_bt;
    /** Index number in sprite bank of \ref FontWithFace. */
    int spriteno;
};

/** This class will load a list of TTF files from \ref FontManager, and save
 *  them inside \ref m_faces for \ref FontWithFace to load glyph.
 *  \ingroup font
 */
class FaceTTF : public NoCopy
{
#ifndef SERVER_ONLY
private:
    /** Contains all FT_Face with a list of loaded glyph index with the
     *  \ref FontArea. */
    std::vector<std::pair<FT_Face, std::map<unsigned, FontArea> > > m_faces;
#endif
public:
    LEAK_CHECK()
    // ------------------------------------------------------------------------
    FaceTTF() {}
    // ------------------------------------------------------------------------
    ~FaceTTF() {}
    // ------------------------------------------------------------------------
    void reset()
    {
#ifndef SERVER_ONLY
        for (unsigned int i = 0; i < m_faces.size(); i++)
             m_faces[i].second.clear();
#endif
    }
    // ------------------------------------------------------------------------
    /* Return a white-space font area, which is the first glyph in ttf. */
    const FontArea* getFirstFontArea() const
    {
#ifdef SERVER_ONLY
        static FontArea area;
        return &area;
#else
        if (m_faces.empty())
            return NULL;
        if (m_faces.front().second.empty())
            return NULL;
        return &(m_faces.front().second.begin()->second);
#endif
    }
#ifndef SERVER_ONLY
    // ------------------------------------------------------------------------
    void loadFaces(std::vector<FT_Face> faces)
    {
        for (unsigned int i = 0; i < faces.size(); i++)
            m_faces.emplace_back(faces[i], std::map<unsigned, FontArea>());
    }
    // ------------------------------------------------------------------------
    /** Return a TTF in \ref m_faces.
    *  \param i index of TTF file in \ref m_faces.
    */
    FT_Face getFace(unsigned int i) const
    {
        assert(i < m_faces.size());
        return m_faces[i].first;
    }
    // ------------------------------------------------------------------------
    /** Return the total TTF files loaded. */
    unsigned int getTotalFaces() const { return (unsigned int)m_faces.size(); }
    // ------------------------------------------------------------------------
    void insertFontArea(const FontArea& a, unsigned font_index,
                        unsigned glyph_index)
    {
        auto& ttf = m_faces.at(font_index).second;
        ttf[glyph_index] = a;
    }
    // ------------------------------------------------------------------------
    const FontArea* getFontArea(unsigned font_index, unsigned glyph_index)
    {
        auto& ttf = m_faces.at(font_index).second;
        auto it = ttf.find(glyph_index);
        if (it != ttf.end())
            return &it->second;
        return NULL;
    }
    // ------------------------------------------------------------------------
    bool getFontAndGlyphFromChar(uint32_t c, unsigned* font, unsigned* glyph)
    {
        for (unsigned i = 0; i < m_faces.size(); i++)
        {
            unsigned glyph_index = FT_Get_Char_Index(m_faces[i].first, c);
            if (glyph_index > 0)
            {
                *font = i;
                *glyph = glyph_index;
                return true;
            }
        }
        return false;
    }
#endif

};   // FaceTTF

#endif
/* EOF */
