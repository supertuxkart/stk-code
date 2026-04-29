//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2026 Alayan
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

#include "graphics/attachable_library_manager.hpp"

#include "config/user_config.hpp"
#include "graphics/central_settings.hpp"
#include "graphics/irr_driver.hpp"
#include "graphics/material_manager.hpp"
#include "graphics/moving_texture.hpp"
#include "graphics/sp/sp_shader_manager.hpp"
#include "io/file_manager.hpp"
#include "utils/log.hpp"
#include "utils/string_utils.hpp"

AttachableLibraryManager* AttachableLibraryManager::m_attachable_lib_manager = NULL;

// ------------------------------------------------------------------------
/** Private constructor, used by the static create() function.
 */
AttachableLibraryManager::AttachableLibraryManager()
{
    m_init_xyz   = core::vector3df(0,0,0);
    m_init_hpr   = core::vector3df(0,0,0);
    m_init_scale = core::vector3df(1,1,1);
}   // AttachableLibraryManager

// ------------------------------------------------------------------------
/** Destructor, which frees cached data
 */
AttachableLibraryManager::~AttachableLibraryManager()
{
    // TODO the instances should be cleaned after use instead of waiting until here

    // Decrement back the reference counters so the library nodes can be cleaned
    for (auto const& node : m_attachable_lib_nodes)
    {
        if (m_attachable_lib_nodes[node.first]->getReferenceCount() > 0)
            m_attachable_lib_nodes[node.first]->drop();
    }

    for (unsigned int i = 0; i < m_animated_textures.size(); i++)
    {
        delete m_animated_textures[i];
    }
    m_animated_textures.clear();
}   // ~AttachableLibraryManager

// ----------------------------------------------------------------------------
/* This function loads a library node, which is an empty parent node to which
 * are attached a series of nodes.
 * The folder_name parameter tells us where the game should look to load the library node.
 * The identifier parameter tells us which identifier to associate it with. When the game
 * will request the loading of an attachable library node, it will give the identifier.
 */
void AttachableLibraryManager::loadLibraryNode(const std::string& folder_name, const std::string& identifier)
{
    if (m_attachable_lib_nodes.count(identifier) > 0)
    {
        Log::info("AttachableLibraryManager", "Attachable library '%s' is already loaded!", identifier.c_str());
        return;
    }

    m_attachable_lib_nodes[identifier] = irr_driver->getSceneManager()->addEmptySceneNode();

    XMLNode* libroot;
    m_folder_map[identifier] = folder_name;
    std::string lib_path = file_manager->getAsset(FileManager::LIBRARY, folder_name) + "/";
    std::string lib_node_path = lib_path + "node.xml";

    if (file_manager->fileExists(lib_node_path))
    {
        libroot = file_manager->createXMLTree(lib_node_path);
        if (libroot == NULL)
            goto missing_library;
    }
    else
    {
        missing_library:
        Log::fatal("AttachableLibraryManager", "Cannot find library '%s'", lib_node_path.c_str());
        return;
    }

    std::string unique_path = StringUtils::insertValues("library/%s", folder_name.c_str());
    file_manager->pushTextureSearchPath(lib_path + "/", unique_path);
    file_manager->pushModelSearchPath(lib_path);
    material_manager->pushTempMaterial(lib_path + "/materials.xml");
    if (CVS->isGLSL())
        SP::SPShaderManager::get()->loadSPShaders(lib_path);

    // based on Track::loadObjects
    unsigned int node_count = libroot->getNumNodes();

    for (unsigned int i = 0; i < node_count; i++)
    {
        const XMLNode *xml_node = libroot->getNode(i);
        const std::string name = xml_node->getName();

        if (name == "object" || name == "library")
        {
            add(*xml_node, identifier);
        }
        else if (name == "particle-emitter" &&
                 UserConfigParams::m_particles_effects > 1)
        {
            add(*xml_node, identifier);
        }
        else if (name == "light")
        {
            add(*xml_node, identifier);
        }
        else
        {
            Log::warn("AttachLibraryManager", "While loading library with id '%s', element '%s' was "
                      "met but is unknown.", identifier.c_str(), xml_node->getName().c_str());
        }

    }   // for i<libroot->getNumNodes()

    // Prevent the parent node from being cleared
    // We might need to do more so that animations, etc. work.
    m_attachable_lib_nodes[identifier]->grab();
}   // loadLibraryNode

// ----------------------------------------------------------------------------
/* This function loads the library objects corresponding with a cached library node,
 * when we need to create a new instance of the library attachment.
 * The identifier parameter tells us which cached library node should be used
 */
scene::ISceneNode* AttachableLibraryManager::loadLibraryInstance(const std::string& identifier)
{
    if (m_attachable_lib_nodes.count(identifier) == 0)
    {
        Log::error("AttachableLibraryManager",
            "Attachable library '%s' was not loaded!", identifier.c_str());
        return nullptr;
    }

    std::string unique_id;
    unsigned int instance_count = 0;
    // We create a unique ID for this new instance
    // If an ID was used then vacated, we can reuse it.
    // FIXME IDs are currently not vacated
    do
    {
        unique_id = identifier + "_" + StringUtils::toString(instance_count);
        instance_count++;
    } while(m_attachable_lib_nodes.count(unique_id) > 0);

    Log::info("AttachableLibraryManager","Unique id selected is %s!", unique_id.c_str());

    m_attachable_lib_nodes[unique_id] = irr_driver->getSceneManager()->addEmptySceneNode();

    // This might be unnecessary
    m_attachable_lib_nodes[unique_id]->setPosition(m_init_xyz);
    m_attachable_lib_nodes[unique_id]->setRotation(m_init_hpr);
    m_attachable_lib_nodes[unique_id]->setScale(m_init_scale);
    m_attachable_lib_nodes[unique_id]->updateAbsolutePosition();
    m_attachable_lib_nodes[unique_id]->setNeedsUpdateAbsTrans(true);

    // Loop over all objects from the reference lib node and do a deep copy
    // with the correct new parent
    for (unsigned int i = 0; i < m_all_objects[identifier].size(); i++)
    {
        AttachableLibraryObject *copied_obj =
            m_all_objects[identifier][i]->clone(m_attachable_lib_nodes[unique_id],
                                                m_folder_map[identifier]);
        m_all_objects[unique_id].push_back(copied_obj);
    }

    // Prevent it from being cleared
    // Might not be necessary with the new structure!
    m_attachable_lib_nodes[unique_id]->grab();

    return m_attachable_lib_nodes[unique_id];
} // loadLibraryInstance

// ----------------------------------------------------------------------------
/** Handles animated textures. This is used when first registering a library node.
 */
void AttachableLibraryManager::handleAnimatedTextures(scene::ISceneNode *node, const XMLNode &xml)
{
    std::vector<MovingTexture*> new_animated_textures;
    new_animated_textures = MovingTextureUtils::processTextures(node, xml, "attachable_library");
    for (unsigned int i = 0; i < new_animated_textures.size(); i++)
    {
        m_animated_textures.push_back(new_animated_textures[i]);
    }
}   // handleAnimatedTextures

// ----------------------------------------------------------------------------
/** Handles animated textures. This is used when copying a library node
 */
void AttachableLibraryManager::handleAnimatedTextures(const std::string& ident)
{
    // TODO copy the animated texture from the reference ident
}   // handleAnimatedTextures

void AttachableLibraryManager::add(const XMLNode &xml_node, const std::string& parent_id)
{
    try
    {
        AttachableLibraryObject *obj =
            new AttachableLibraryObject(xml_node, m_attachable_lib_nodes[parent_id]);
        m_all_objects[parent_id].push_back(obj);
    }
    catch (std::exception& e)
    {
        Log::warn("AttachableLibraryManager", "Could not load library object. Reason : %s",
                  e.what());
    }
}   // add

// ----------------------------------------------------------------------------
/** Updates all attached library objects
 * Most objects don't need to run custom update graphics code but a few do.
 */
void AttachableLibraryManager::updateGraphics(float dt)
{
    for (auto& map_entry : m_all_objects)
    {
        AttachableLibraryObject* curr;

        for(unsigned int i = 0; i < map_entry.second.size(); i++)
        {
            curr = map_entry.second[i];
            curr->updateGraphics(dt);
        }
    }

    for (unsigned int i = 0; i<m_animated_textures.size(); i++)
    {
        m_animated_textures[i]->update(dt);
    }
}   // updateGraphics

scene::ISceneNode* AttachableLibraryManager::createInstance(const std::string& name)
{
    if (m_attachable_lib_nodes.count(name) == 0)
        Log::fatal("AttachableLibraryManager", "Library for '%s' is not loaded.", name.c_str());
    else
        Log::info("AttachableLibraryManager", "Creating instance for '%s'.", name.c_str());

    // This function creates a new library instance
    // and returns the parent scene node
    return loadLibraryInstance(name);
}   // createInstance