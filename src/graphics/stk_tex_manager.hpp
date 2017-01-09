//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2017 SuperTuxKart-Team
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

#include "irrString.h"

#include <algorithm>
#include <string>
#include <unordered_map>

class STKTexture;
namespace irr
{
    namespace video { class ITexture; class SColor; }
}

class STKTexManager : public Singleton<STKTexManager>, NoCopy
{
private:
    std::unordered_map<std::string, STKTexture*> m_all_textures;

    // ------------------------------------------------------------------------
    STKTexture* findTextureInFileSystem(const std::string& filename,
                                        std::string* full_path);
public:
    // ------------------------------------------------------------------------
    STKTexManager() {}
    // ------------------------------------------------------------------------
    ~STKTexManager();
    // ------------------------------------------------------------------------
    irr::video::ITexture* getTexture(const std::string& path,
                                     bool srgb = false,
                                     bool premul_alpha = false,
                                     bool set_material = false,
                                     bool mesh_tex = false,
                                     bool no_upload = false,
                                     bool single_channel = false);
    // ------------------------------------------------------------------------
    irr::video::ITexture* getUnicolorTexture(const irr::video::SColor &c);
    // ------------------------------------------------------------------------
    void addTexture(STKTexture* texture);
    // ------------------------------------------------------------------------
    void removeTexture(STKTexture* texture, bool remove_all = false);
    // ------------------------------------------------------------------------
    void dumpAllTexture(bool mesh_texture);
    // ------------------------------------------------------------------------
    int dumpTextureUsage();
    // ------------------------------------------------------------------------
    irr::core::stringw reloadTexture(const irr::core::stringw& name);
    // ------------------------------------------------------------------------
    void reset();

};   // STKTexManager

#endif
