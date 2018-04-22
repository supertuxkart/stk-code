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

#include <string>
#include <vector>

#include <ft2build.h>
#include FT_FREETYPE_H

/** This class will load a list of TTF files from \ref STKConfig, and save
 *  them inside \ref m_faces for \ref FontWithFace to load glyph.
 *  Each FaceTTF can be used more than once in each instantiation of \ref
 *  FontWithFace, so it can render characters differently using the same TTF
 *  file to save memory, for example different outline size.
 *  \ingroup font
 */
class FaceTTF : public NoCopy
{
private:
    /** Contains all TTF files loaded. */
    std::vector<FT_Face> m_faces;

public:
    LEAK_CHECK()
    // ------------------------------------------------------------------------
    FaceTTF(const std::vector<std::string>& ttf_list);
    // ------------------------------------------------------------------------
    ~FaceTTF();
    // ------------------------------------------------------------------------
    FT_Face getFace(unsigned int i) const;
    // ------------------------------------------------------------------------
    /** Return the total TTF files loaded. */
    unsigned int getTotalFaces() const { return (unsigned int)m_faces.size(); }

};   // FaceTTF

#endif
/* EOF */
