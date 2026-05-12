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
#include "graphics/material.hpp"
#include "graphics/material_manager.hpp"
#include "graphics/moving_texture.hpp"
#include "graphics/sp/sp_mesh.hpp"
#include "graphics/sp/sp_mesh_buffer.hpp"
#include "graphics/sp/sp_mesh_node.hpp"
#include "graphics/sp/sp_shader_manager.hpp"
#include "io/file_manager.hpp"
#include "utils/log.hpp"
#include "utils/string_utils.hpp"

AttachableLibraryManager* AttachableLibraryManager::m_attachable_lib_manager = NULL;

// --------------------------------------------------------------------------------------------
/** Private constructor, used by the static create() function.
 */
AttachableLibraryManager::AttachableLibraryManager()
{
    m_init_xyz   = core::vector3df(0,0,0);
    m_init_hpr   = core::vector3df(0,0,0);
    m_init_scale = core::vector3df(1,1,1);
}   // AttachableLibraryManager

// --------------------------------------------------------------------------------------------
/** Destructor, which frees cached data
 */
AttachableLibraryManager::~AttachableLibraryManager()
{
    cleanAll();
}   // ~AttachableLibraryManager

// --------------------------------------------------------------------------------------------
/* This function loads a library node, which is an empty parent node to which
 * are attached a series of nodes.
 * The folder_name parameter tells us where the game should look to load the library node.
 * The identifier parameter tells us which identifier to associate it with. When the game
 * will request the loading of an attachable library node, it will give the identifier.
 */
void AttachableLibraryManager::loadLibraryNode(const std::string& folder_name,
                                               const std::string& identifier)
{
    if (m_attachable_lib_nodes.count(identifier) > 0)
    {
        Log::info("AttachableLibraryManager",
            "Attachable library '%s' is already loaded!", identifier.c_str());
        return;
    }

    m_attachable_lib_nodes[identifier] = irr_driver->getSceneManager()->addEmptySceneNode();

    XMLNode* libroot;
    // During reloads, the folder map is preserved but the attachable nodes aren't
    if (m_folder_map.count(identifier) == 0)
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

    delete libroot;

    // Prevent the nodes from being cleared
    m_attachable_lib_nodes[identifier]->grab(); // parent node

    for (unsigned int i = 0; i < m_all_objects[identifier].size(); i++)
    {
        m_all_objects[identifier][i]->grabNode();
    }
}   // loadLibraryNode

// --------------------------------------------------------------------------------------------
/* This function loads the library objects corresponding with a cached library node,
 * when we need to create a new instance of the library attachment.
 * The identifier parameter tells us which cached library node should be used
 */
scene::ISceneNode* AttachableLibraryManager::loadLibraryInstance(const std::string& identifier,
                                                                 std::string& lib_instance)
{
    if (m_attachable_lib_nodes.count(identifier) == 0)
    {
        Log::error("AttachableLibraryManager",
            "Attachable library '%s' was not loaded!", identifier.c_str());
        return nullptr;
    }

    unsigned int instance_count = 0;
    // We create a unique ID for this new instance
    // If an ID was used then vacated, we can reuse it.
    do
    {
        lib_instance = identifier + "_" + StringUtils::toString(instance_count);
        instance_count++;
    } while(m_attachable_lib_nodes.count(lib_instance) > 0);

    m_attachable_lib_nodes[lib_instance] = irr_driver->getSceneManager()->addEmptySceneNode();

    // This might be unnecessary
    m_attachable_lib_nodes[lib_instance]->setPosition(m_init_xyz);
    m_attachable_lib_nodes[lib_instance]->setRotation(m_init_hpr);
    m_attachable_lib_nodes[lib_instance]->setScale(m_init_scale);
    m_attachable_lib_nodes[lib_instance]->updateAbsolutePosition();
    m_attachable_lib_nodes[lib_instance]->setNeedsUpdateAbsTrans(true);

    // Loop over all objects from the reference lib node and do a deep copy
    // with the correct new parent
    for (unsigned int i = 0; i < m_all_objects[identifier].size(); i++)
    {
        AttachableLibraryObject *copied_obj =
            m_all_objects[identifier][i]->clone(m_attachable_lib_nodes[lib_instance],
                            m_folder_map[identifier], identifier, instance_count);
        m_all_objects[lib_instance].push_back(copied_obj);
    }

    return m_attachable_lib_nodes[lib_instance];
} // loadLibraryInstance

// --------------------------------------------------------------------------------------------
/** Handles animated textures. This is used when first registering a library node.
 */
void AttachableLibraryManager::handleAnimatedTextures(scene::ISceneNode *node,
    const XMLNode &xml, const std::string& ident)
{
    std::vector<MovingTexture*> new_animated_textures;
    new_animated_textures = MovingTextureUtils::processTextures(node, xml, "attachable_library");
    for (unsigned int i = 0; i < new_animated_textures.size(); i++)
    {
        m_animated_textures[ident].push_back(new_animated_textures[i]);
    }
}   // handleAnimatedTextures

// --------------------------------------------------------------------------------------------
/** Handles animated textures. This is used when copying a library node
 */
void AttachableLibraryManager::handleAnimatedTextures(scene::ISceneNode *node,
    const std::string& full_ident, const std::string& instance_ident)
{
    for (unsigned int i = 0; i < m_animated_textures[full_ident].size(); i++)
    {
        MovingTexture* mt = m_animated_textures[full_ident][i]->clone();
        SP::SPMeshNode* spmn = dynamic_cast<SP::SPMeshNode*>(node);
        if (spmn)
        {
            for (unsigned i = 0; i < spmn->getSPM()->getMeshBufferCount(); i++)
            {
                SP::SPMeshBuffer* spmb = spmn->getSPM()->getSPMeshBuffer(i);
                const std::vector<Material*>& m = spmb->getAllSTKMaterials();
                bool found = false;
                for (unsigned j = 0; j < m.size(); j++)
                {
                    Material* mat = m[j];
                    std::string mat_name = StringUtils::getBasename(mat->getSamplerPath(0));
                    mat_name = StringUtils::toLowerCase(mat_name);
                    if (mat_name == m_animated_textures[full_ident][i]->getMatName())
                    {
                        found = true;
                        spmb->enableTextureMatrix(j);
                        mt->setSPTM(spmn->getTextureMatrix(i).data());
                        break;
                    }
                }
                if (found)
                {
                    break;
                }
            }
        }
        else
        {
            Log::error("AttachableLibraryManager",
                "Only SP meshes are supported for animated textures in attachable libraries.");
        }

        m_animated_textures[instance_ident].push_back(mt);
    }
}   // handleAnimatedTextures

void AttachableLibraryManager::add(const XMLNode &xml_node, const std::string& parent_id)
{
    try
    {
        AttachableLibraryObject *obj =
            new AttachableLibraryObject(xml_node, m_attachable_lib_nodes[parent_id], parent_id);
        m_all_objects[parent_id].push_back(obj);
    }
    catch (std::exception& e)
    {
        Log::warn("AttachableLibraryManager", "Could not load library object. Reason : %s",
                  e.what());
    }
}   // add

// --------------------------------------------------------------------------------------------
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

    for (auto const& pair : m_animated_textures)
    {
        for (unsigned int i = 0; i < pair.second.size(); i++)
        {
            pair.second[i]->update(dt);
        }
    }
}   // updateGraphics

// --------------------------------------------------------------------------------------------
scene::ISceneNode* AttachableLibraryManager::createInstance(const std::string& name,
                                                            std::string& lib_instance)
{
    if (m_attachable_lib_nodes.count(name) == 0)
        Log::fatal("AttachableLibraryManager", "Library for '%s' is not loaded.", name.c_str());
    //else
        //Log::info("AttachableLibraryManager", "Creating instance for '%s'.", name.c_str());

    // This function creates a new library instance
    // and returns the parent scene node
    return loadLibraryInstance(name, lib_instance);
}   // createInstance

// --------------------------------------------------------------------------------------------
void AttachableLibraryManager::cleanInstance(const std::string& instance_id, bool is_template)
{
    if (m_attachable_lib_nodes.count(instance_id) == 0)
    {
        Log::error("AttachableLibraryManager",
            "Trying to clean instance '%s', which is not loaded.", instance_id.c_str());
        return;
    }

    // Loop over all objects linked to this lib instance and delete them
    for (unsigned int i = 0; i < m_all_objects[instance_id].size(); i++)
    {
        if (is_template)
            m_all_objects[instance_id][i]->dropNode();

        delete m_all_objects[instance_id][i];
    }

    // Remove the parent node of the library instance
    irr_driver->removeNode(m_attachable_lib_nodes[instance_id]);

    // Delete the vector and std::map key too
    m_all_objects[instance_id].clear();
    auto keymatch = m_all_objects.find(instance_id);
    m_all_objects.erase(keymatch);
    auto keymatch_2 = m_attachable_lib_nodes.find(instance_id);
    m_attachable_lib_nodes.erase(keymatch_2);

    // Clear the animated textures associated with this lib_instance
    std::vector<std::string> ids_to_clear;
    ids_to_clear.push_back(instance_id);
    cleanTextures(ids_to_clear);
}   // cleanInstance

// --------------------------------------------------------------------------------------------
/** Clean everything during an irr_driver reload. The folder_map is preserved.
 * Note that while we clean many things, some data related to mesh/textures
 * needs to be cleaned in IrrDriver before reloadAll is safe. */
void AttachableLibraryManager::cleanAll()
{
    std::vector<std::string> ids_to_clear;
    for (auto const& node : m_attachable_lib_nodes)
    {
        // If needed, decrement the reference counters so templates can be cleaned
        if (m_attachable_lib_nodes[node.first]->getReferenceCount() > 1)
            m_attachable_lib_nodes[node.first]->drop();
        ids_to_clear.push_back(node.first);
    }

    // We can't clean instance in the previous loop
    // because cleanInstance() directly modifies m_attachable_lib_nodes
    for (unsigned int i = 0; i < ids_to_clear.size(); i++)
    {
        bool is_template = (m_folder_map.count(ids_to_clear[i]) > 0);
        cleanInstance(ids_to_clear[i], is_template);
    }

    // The animated textures for templates still need to be cleaned
    ids_to_clear.clear();
    for (auto pair : m_animated_textures)
    {
        ids_to_clear.push_back(pair.first);
    }

    cleanTextures(ids_to_clear);
}   // cleanAll

void AttachableLibraryManager::cleanTextures(std::vector<std::string> ids_to_clear)
{
    for (unsigned int i = 0; i < ids_to_clear.size(); i++)
    {
        if (m_animated_textures.count(ids_to_clear[i]) == 0)
            continue;

        auto tex_vector = m_animated_textures[ids_to_clear[i]];

        for (unsigned int j = 0; j < tex_vector.size(); j++)
        {
            delete tex_vector[j];
        }
        tex_vector.clear();
        m_animated_textures.erase(ids_to_clear[i]);
    }
}   // cleanTextures

// --------------------------------------------------------------------------------------------
/** Rebuild the library templates based on the folder map
 * Note that by the time reloadAll is called, some other classes may have
 * already requested some of the mapped library nodes to be loaded, this is safe.*/
void AttachableLibraryManager::reloadAll()
{
    for (auto const& lib_pair : m_folder_map)
    {
        loadLibraryNode(lib_pair.second /* folder name */, lib_pair.first  /* identifier */);
    }
}   // reloadAll