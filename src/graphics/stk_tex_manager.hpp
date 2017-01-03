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

#ifndef HEADER_STK_TEX_MANAGER_HPP
#define HEADER_STK_TEX_MANAGER_HPP

#include "graphics/gl_headers.hpp"
#include "utils/no_copy.hpp"
#include "utils/singleton.hpp"

#include <algorithm>
#include <string>
#include <unordered_map>

class STKTexture;

class STKTexManager : public Singleton<STKTexManager>, NoCopy
{
private:
    std::unordered_map<std::string, STKTexture*> m_all_textures;

public:
    // ------------------------------------------------------------------------
    STKTexManager() {}
    // ------------------------------------------------------------------------
    ~STKTexManager();
    // ------------------------------------------------------------------------
    STKTexture* findTexturePathless(const std::string& filename);
    // ------------------------------------------------------------------------
    STKTexture* findTextureInFileSystem(const std::string& filename,
                                        std::string* full_path);
    // ------------------------------------------------------------------------
    STKTexture* getTexture(const std::string& path, bool srgb = false,
                           bool premul_alpha = false,
                           bool set_material = false,
                           bool mesh_tex = false, bool no_upload = false);
    // ------------------------------------------------------------------------
    void addTexture(STKTexture* t);
    // ------------------------------------------------------------------------
    void removeTexture(STKTexture* t);
    // ------------------------------------------------------------------------
    void dumpAllTexture();
    // ------------------------------------------------------------------------
    int dumpTextureUsage();
    // ------------------------------------------------------------------------
    void clean();

};   // STKTexManager

#endif
