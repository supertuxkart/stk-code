//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2004-2015 Steve Baker <sjbaker1@airmail.net>
//  Copyright (C) 2010-2015 Steve Baker, Joerg Henrichs
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

#ifndef HEADER_MATERIAL_MANAGER_HPP
#define HEADER_MATERIAL_MANAGER_HPP

#include "utils/no_copy.hpp"

namespace irr
{
    namespace video { class ITexture; }
    namespace scene { class IMeshBuffer; class ISceneNode; }
}
using namespace irr;

#include <string>
#include <vector>
#include <map>
#include <EMaterialTypes.h>

class Material;
class XMLReader;
class XMLNode;

/**
  * \ingroup graphics
  */
class MaterialManager : public NoCopy
{
private:

    void    parseMaterialFile(const std::string& filename);
    int     m_shared_material_index;

    std::vector<Material*> m_materials;

    std::map<std::string, Material*> m_default_sp_materials;

public:
              MaterialManager();
             ~MaterialManager();
    void      loadMaterial     ();
    Material* getMaterialFor(video::ITexture* t,
                             scene::IMeshBuffer *mb);
    Material* getMaterialFor(video::ITexture* t,
                             video::E_MATERIAL_TYPE material_type);
    Material* getMaterialFor(video::ITexture* t);
    Material* getMaterialSPM(std::string lay_one_tex_lc,
                             std::string lay_two_tex_lc,
                             const std::string& def_shader_name = "solid");
    void      setAllMaterialFlags(video::ITexture* t,
                                  scene::IMeshBuffer *mb);
    void      setAllUntexturedMaterialFlags(scene::IMeshBuffer *mb);

    int       addEntity        (Material *m);
    Material *getMaterial      (const std::string& t,
                                bool is_full_path=false,
                                bool make_permanent=false,
                                bool complain_if_not_found=true,
                                bool strip_path=true, bool install=true, bool create_if_not_found=true);
    void      addSharedMaterial(const std::string& filename, bool deprecated = false);
    bool      pushTempMaterial (const std::string& filename, bool deprecated = false);
    bool      pushTempMaterial (const XMLNode *root, const std::string& filename, bool deprecated = false);
    void      popTempMaterial  ();
    void      makeMaterialsPermanent();
    bool      hasMaterial(const std::string& fname);

    void      unloadAllTextures();

    Material* getDefaultSPMaterial(const std::string& shader_name,
                                   const std::string& layer_one_lc = "",
                                   bool full_path = false);
    Material* getLatestMaterial() { return m_materials[m_materials.size()-1]; }
};   // MaterialManager

extern MaterialManager *material_manager;

#endif

/* EOF */
