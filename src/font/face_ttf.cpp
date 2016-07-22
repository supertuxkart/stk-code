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

#include "font/face_ttf.hpp"

#include "io/file_manager.hpp"

// ----------------------------------------------------------------------------
FaceTTF::FaceTTF(const std::vector<std::string>& ttf_list)
{
    for (const std::string& font : ttf_list)
    {
        FT_Face face = NULL;
        font_manager->checkFTError(FT_New_Face(font_manager->getFTLibrary(),
            (file_manager->getAssetChecked(FileManager::TTF,
            font.c_str(), true)).c_str(), 0, &face), "loading fonts");
        m_faces.push_back(face);
    }
}   // FaceTTF

// ----------------------------------------------------------------------------
FaceTTF::~FaceTTF()
{
    for (unsigned int i = 0; i < m_faces.size(); i++)
    {
        font_manager->checkFTError(FT_Done_Face(m_faces[i]), "removing face");
    }
}   // ~FaceTTF
// ----------------------------------------------------------------------------
FT_Face FaceTTF::getFace(unsigned int i) const
{
    assert(i < m_faces.size());
    return m_faces[i];
}   // getFace
