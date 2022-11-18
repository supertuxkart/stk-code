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

#include "graphics/material_manager.hpp"

#include <stdexcept>
#include <sstream>

#include "config/user_config.hpp"
#include "graphics/central_settings.hpp"
#include "graphics/material.hpp"
#include "graphics/particle_kind_manager.hpp"
#include "graphics/sp/sp_texture_manager.hpp"
#include "io/file_manager.hpp"
#include "io/xml_node.hpp"
#include "modes/world.hpp"
#include "tracks/track.hpp"
#include "utils/string_utils.hpp"

#include <IFileSystem.h>
#include <ITexture.h>
#include <SMaterial.h>
#include <IMeshBuffer.h>

MaterialManager *material_manager=0;

MaterialManager::MaterialManager()
{
    /* Create list - and default material zero */

    m_materials.reserve(256);
    // We can't call init/loadMaterial here, since the global variable
    // material_manager has not yet been initialised, and
    // material_manager is used in the Material constructor.
    // Therefore, the code for loading the material had to
    // be moved into a separate function.
}

//-----------------------------------------------------------------------------
/** Frees all allocated data structures.
 */
MaterialManager::~MaterialManager()
{
#ifndef SERVER_ONLY
    if (CVS->isGLSL())
        SP::SPTextureManager::get()->stopThreads();
#endif

    for(unsigned int i=0; i<m_materials.size(); i++)
    {
        delete m_materials[i];
    }
    m_materials.clear();

    for (std::map<std::string, Material*> ::iterator it =
         m_default_sp_materials.begin(); it != m_default_sp_materials.end();
         it++)
    {
        delete it->second;
    }
    m_default_sp_materials.clear();
}   // ~MaterialManager

//-----------------------------------------------------------------------------

Material* MaterialManager::getMaterialFor(video::ITexture* t,
    scene::IMeshBuffer *mb)
{
    return getMaterialFor(t, mb->getMaterial().MaterialType);
}

//-----------------------------------------------------------------------------
Material* MaterialManager::getMaterialSPM(std::string lay_one_tex_lc,
                                          std::string lay_two_tex_lc,
                                          const std::string& def_shader_name)
{
    std::string original_layer_one = lay_one_tex_lc;
    core::stringc lc(lay_one_tex_lc.c_str());
    lc.make_lower();
    lay_one_tex_lc = lc.c_str();
    lc = lay_two_tex_lc.c_str();
    lc.make_lower();
    lay_two_tex_lc = lc.c_str();
    const bool is_full_path = !lay_one_tex_lc.empty() &&
        (lay_one_tex_lc.find('/') != std::string::npos ||
        lay_one_tex_lc.find('\\') != std::string::npos);
    if (is_full_path)
    {
        // Search backward so that temporary (track) textures are found first
        for (int i = (int)m_materials.size() - 1; i >= 0; i--)
        {
            if (m_materials[i]->getTexFullPath() == lay_one_tex_lc)
            {
                const std::string& mat_lay_two = m_materials[i]->getUVTwoTexture();
                if (mat_lay_two.empty() && lay_two_tex_lc.empty())
                {
                    return m_materials[i];
                }
                else if (!mat_lay_two.empty() && !lay_two_tex_lc.empty())
                {
                    if (mat_lay_two == lay_two_tex_lc)
                    {
                        return m_materials[i];
                    }
                }
            }
        }
    }
    else if (!lay_one_tex_lc.empty())
    {
        for (int i = (int)m_materials.size() - 1; i >= 0; i--)
        {
            if (m_materials[i]->getTexFname() == lay_one_tex_lc)
            {
                const std::string& mat_lay_two = m_materials[i]->getUVTwoTexture();
                if (mat_lay_two.empty() && lay_two_tex_lc.empty())
                {
                    return m_materials[i];
                }
                else if (!mat_lay_two.empty() && !lay_two_tex_lc.empty())
                {
                    if (mat_lay_two == lay_two_tex_lc)
                    {
                        return m_materials[i];
                    }
                }
            }
        }   // for i
    }
    Log::debug("MaterialManager", "Couldn't find cached SP material! Opening default %s!", original_layer_one.c_str());
    return getDefaultSPMaterial(def_shader_name,
        is_full_path ?
        original_layer_one : StringUtils::getBasename(original_layer_one),
        is_full_path);
}

//-----------------------------------------------------------------------------
Material* MaterialManager::getMaterialFor(video::ITexture* t)
{
    const io::path& img_path = t->getName().getInternalName();

    if (!img_path.empty() && (img_path.findFirst('/') != -1 || img_path.findFirst('\\') != -1))
    {
        // Search backward so that temporary (track) textures are found first
        for (int i = (int)m_materials.size() - 1; i >= 0; i--)
        {
            if (m_materials[i]->getTexFullPath() == img_path.c_str())
            {
                return m_materials[i];
            }
        }
    }
    else
    {
        core::stringc image(StringUtils::getBasename(img_path.c_str()).c_str());
        image.make_lower();

        for (int i = (int)m_materials.size() - 1; i >= 0; i--)
        {
            if (m_materials[i]->getTexFname() == image.c_str())
            {
                return m_materials[i];
            }
        }   // for i
    }
    return NULL;
}

//-----------------------------------------------------------------------------
Material* MaterialManager::getMaterialFor(video::ITexture* t,
    video::E_MATERIAL_TYPE material_type)
{
    if (t == NULL)
        return getDefaultSPMaterial("solid");

    Material* m = getMaterialFor(t);
    if (m != NULL)
        return m;

    return getDefaultSPMaterial("solid");
}

//-----------------------------------------------------------------------------
/** Searches for the material in the given texture, and calls a function
 *  in the material to set the irrlicht material flags.
 *  \param t Pointer to the texture.
 *  \param mb Pointer to the mesh buffer.
*/
void MaterialManager::setAllMaterialFlags(video::ITexture* t,
                                          scene::IMeshBuffer *mb)
{
    Material* mat = getMaterialFor(t, mb);
    if (mat != NULL)
    {
        mat->setMaterialProperties(&(mb->getMaterial()), mb);
        return;
    }
}   // setAllMaterialFlags

//-----------------------------------------------------------------------------
Material* MaterialManager::getDefaultSPMaterial(const std::string& shader_name,
                                                const std::string& l1_lc,
                                                bool full_path)
{
    core::stringc lc(l1_lc.c_str());
    lc.make_lower();
    const std::string key = shader_name + lc.c_str();
    auto ret = m_default_sp_materials.find(key);
    if (ret != m_default_sp_materials.end())
    {
        return ret->second;
    }
    Material* m = new Material(l1_lc.empty() ? "unicolor_white" :
        l1_lc, full_path, false, false, shader_name);
    m_default_sp_materials[key] = m;
    return m;
}   // getDefaultSPMaterial

//-----------------------------------------------------------------------------
void MaterialManager::setAllUntexturedMaterialFlags(scene::IMeshBuffer *mb)
{
    irr::video::SMaterial& material = mb->getMaterial();
    if (material.getTexture(0) == NULL)
    {
        //material.AmbientColor = video::SColor(255, 50, 50, 50);
        //material.DiffuseColor = video::SColor(255, 150, 150, 150);
        material.EmissiveColor = video::SColor(255, 0, 0, 0);
        material.SpecularColor = video::SColor(255, 0, 0, 0);
        //material.Shininess = 0.0f;
        material.ColorMaterial = irr::video::ECM_DIFFUSE_AND_AMBIENT;
        material.MaterialType = irr::video::EMT_SOLID;
    }

}

//-----------------------------------------------------------------------------
int MaterialManager::addEntity(Material *m)
{
    m_materials.push_back(m);
    return (int)m_materials.size()-1;
}

//-----------------------------------------------------------------------------
void MaterialManager::loadMaterial()
{
    // Use temp material for reading, but then set the shared
    // material index later, so that these materials are not popped
    //
    addSharedMaterial(file_manager->getAssetChecked(FileManager::TEXTURE,
                                                    "materials.xml", true));
    std::string deprecated = file_manager->getAssetChecked(FileManager::TEXTURE,
                                                           "deprecated/materials.xml");
    if(deprecated.size()>0)
        addSharedMaterial(deprecated, true);

    // Save index of shared textures
    m_shared_material_index = (int)m_materials.size();
}   // MaterialManager

//-----------------------------------------------------------------------------
void MaterialManager::addSharedMaterial(const std::string& filename, bool deprecated)
{
    // Use temp material for reading, but then set the shared
    // material index later, so that these materials are not popped
    if(filename=="")
    {
        std::ostringstream msg;
        msg<<"FATAL: File '"<<filename<<"' not found\n";
        throw std::runtime_error(msg.str());
    }
    if(!pushTempMaterial(filename, deprecated))
    {
        std::ostringstream msg;
        msg <<"FATAL: Parsing error in '"<<filename<<"'\n";
        throw std::runtime_error(msg.str());
    }
    m_shared_material_index = (int)m_materials.size();
}   // addSharedMaterial

//-----------------------------------------------------------------------------
bool MaterialManager::pushTempMaterial(const std::string& filename, bool deprecated)
{
    XMLNode *root = file_manager->createXMLTree(filename);
    if(!root || root->getName()!="materials")
    {
        if(root) delete root;
        return true;
    }
    const bool success = pushTempMaterial(root, filename, deprecated);
    delete root;
    return success;
}   // pushTempMaterial

//-----------------------------------------------------------------------------
bool MaterialManager::pushTempMaterial(const XMLNode *root,
                                       const std::string& filename,
                                       bool deprecated)
{
    for(unsigned int i=0; i<root->getNumNodes(); i++)
    {
        const XMLNode *node = root->getNode(i);
        if(!node)
        {
            // We don't have access to the filename at this stage anymore :(
            Log::warn("MaterialManager", "Unknown node in material.xml file.");
            continue;
        }
        try
        {
            m_materials.push_back(new Material(node, deprecated));
        }
        catch(std::exception& e)
        {
            // The message contains a '%s' for the filename
            Log::warn("MaterialManager", e.what(), filename.c_str());
        }
    }   // for i<xml->getNumNodes)(
    return true;
}   // pushTempMaterial


//-----------------------------------------------------------------------------
void MaterialManager::popTempMaterial()
{
    for(int i=(int)m_materials.size()-1; i>=this->m_shared_material_index; i--)
    {
        delete m_materials[i];
        m_materials.pop_back();
    }   // for i6
}   // popTempMaterial

//-----------------------------------------------------------------------------
/** Returns the material of a given name, if it doesn't exist, it is loaded.
 *  Materials that are just loaded are not permanent, and so get deleted after
 *  a race (this is used to load temporary, track specific materials). To make
 *  material permanent, make_permanent must be set to true. This is used for
 *  the powerup_manager, since not all icons for the powerups are listed in the
 *  materials.dat file, causing the missing ones to be temporary only (and
 *  then get deleted after one race, causing the powerup_manager to have
 *  invalid pointers.
 *  \param fname  Name of the material.
 *  \param is_full_path True if the name includes the path (defaults to false)
 *  \param make_permanent True if this material should be kept in memory
 *                        (defaults to false)
 */
Material *MaterialManager::getMaterial(const std::string& fname,
                                       bool is_full_path,
                                       bool make_permanent,
                                       bool complain_if_not_found,
                                       bool strip_path, bool install,
                                       bool create_if_not_found)
{
    if(fname=="")
    {
        // This happens while reading the stk_config file, which contains
        // kart_properties information (but no icon file): since at this
        // stage loadMaterial() hasn't been called, an exception can be
        // triggered here (as it happened with visual c++), when
        // m_materials[0] is accessed.
        if(m_materials.size()>=1) return m_materials[0];
        return NULL;
    }

    std::string basename;
    
    if (strip_path)
        basename = StringUtils::getBasename(fname);
    else
        basename = fname;
        
    core::stringc basename_lower(basename.c_str());
    basename_lower.make_lower();

    // Search backward so that temporary (track) textures are found first
    for (int i = (int)m_materials.size()-1; i>=0; i-- )
    {
        if (m_materials[i]->getTexFname() == basename_lower.c_str())
            return m_materials[i];
    }

    if (!create_if_not_found)
        return NULL;
    // Add the new material
    Material* m = new Material(fname, is_full_path, complain_if_not_found, install);
    m_materials.push_back(m);
    if(make_permanent)
    {
        assert(m_shared_material_index==(int)m_materials.size()-1);
        m_shared_material_index = (int)m_materials.size();
    }
    return m ;
}   // getMaterial


// ----------------------------------------------------------------------------
/** Makes all materials permanent. Used for overworld.
 */
void MaterialManager::makeMaterialsPermanent()
{
    m_shared_material_index = (int) m_materials.size();
}   // makeMaterialsPermanent

// ----------------------------------------------------------------------------
void MaterialManager::unloadAllTextures()
{
    std::string texture_folder =
        file_manager->getAssetDirectory(FileManager::TEXTURE);
    texture_folder = file_manager->getFileSystem()->getAbsolutePath
        (texture_folder.c_str()).c_str();
    core::stringc texfname(texture_folder.c_str());
    texfname.make_lower();
    texture_folder = texfname.c_str();
    for (int i = 0; i < m_shared_material_index; i++)
    {
        // Global particle textures will stay until exit
        // STK, which avoid hangs when lazy-loading the texture when being
        // triggered.
        Material* m = m_materials[i];
        if (!ParticleKindManager::get()->isGlobalParticleMaterial(m)
            && m->getTexFullPath().find(texture_folder) != std::string::npos)
        {
            m->unloadTexture();
        }
    }
}

// ----------------------------------------------------------------------------
bool MaterialManager::hasMaterial(const std::string& fname)
{
    std::string basename=StringUtils::getBasename(fname);

    // Search backward so that temporary (track) textures are found first
    for(int i = (int)m_materials.size()-1; i>=0; i-- )
    {
        if(m_materials[i]->getTexFname()==basename) return true;
    }
    return false;
}
