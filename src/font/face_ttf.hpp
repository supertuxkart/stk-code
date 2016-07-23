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

#include "font/font_manager.hpp"

class FaceTTF : public NoCopy
{
private:
    std::vector<FT_Face> m_faces;

public:
    LEAK_CHECK();
    // ------------------------------------------------------------------------
    FaceTTF(const std::vector<std::string>& ttf_list);
    // ------------------------------------------------------------------------
    ~FaceTTF();
    // ------------------------------------------------------------------------
    FT_Face getFace(unsigned int i) const;
    // ------------------------------------------------------------------------
    unsigned int getTotalFaces() const               { return m_faces.size(); }

};   // FaceTTF

#endif
/* EOF */
