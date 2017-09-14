//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2007-2015 Joerg Henrichs
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

#include "tracks/model_definition_loader.hpp"
using namespace irr;

#include "config/user_config.hpp"
#include "graphics/irr_driver.hpp"
#include "graphics/lod_node.hpp"
#include "graphics/mesh_tools.hpp"
#include "io/xml_node.hpp"
#include "modes/world.hpp"
#include "tracks/track.hpp"

#include <IMeshSceneNode.h>
#include <ISceneManager.h>
#include <IMeshManipulator.h>
#include <algorithm>

ModelDefinitionLoader::ModelDefinitionLoader(Track* track)
{
    m_track = track;
}   // ModelDefinitionLoader

// ----------------------------------------------------------------------------

void ModelDefinitionLoader::addModelDefinition(const XMLNode* xml)
{
    float lod_distance = -1.0f;
    xml->get("lod_distance", &lod_distance);

    std::string lodgroup;
    xml->get("lod_group", &lodgroup);

    bool skeletal_animation = false;
    xml->get("skeletal-animation", &skeletal_animation);

    std::string model_name;
    xml->get("model", &model_name);

    m_lod_groups[lodgroup].push_back(ModelDefinition(xml, (int)lod_distance, model_name, false, skeletal_animation));
}   // addModelDefinition

// ----------------------------------------------------------------------------

LODNode* ModelDefinitionLoader::instanciateAsLOD(const XMLNode* node, scene::ISceneNode* parent, RenderInfo* ri)
{
#ifndef SERVER_ONLY
    scene::ISceneManager* sm = irr_driver->getSceneManager();

    std::string groupname = "";
    node->get("lod_group", &groupname);

    std::vector< ModelDefinition >& group = m_lod_groups[groupname];

    if (group.size() > 0)
    {
        scene::ISceneNode* actual_parent = (parent == NULL ? sm->getRootSceneNode() : parent);
        LODNode* lod_node = new LODNode(groupname, actual_parent, sm);
        lod_node->updateAbsolutePosition();
        for (unsigned int m=0; m<group.size(); m++)
        {
            if (group[m].m_skeletal_animation &&
                (UserConfigParams::m_graphical_effects > 1 ||
                World::getWorld()->getIdent() == IDENT_CUTSCENE))
            {
                scene::IAnimatedMesh* a_mesh = irr_driver->getAnimatedMesh(group[m].m_model_file);
                if (!a_mesh)
                {
                    Log::warn("LODNodeLoad", "Warning: object model '%s' not found, ignored.\n",
                        group[m].m_model_file.c_str());
                    continue;
                }

                scene::ISkinnedMesh* sm =
                    dynamic_cast<scene::ISkinnedMesh*>(a_mesh);
                if (sm)
                {
                    MeshTools::createSkinnedMeshWithTangents(sm,
                        &MeshTools::isNormalMap);
                }

                a_mesh->grab();
                //cache.push_back(a_mesh);
                irr_driver->grabAllTextures(a_mesh);
                m_track->addCachedMesh(a_mesh);
                scene::IAnimatedMeshSceneNode* scene_node = irr_driver
                    ->addAnimatedMesh(a_mesh, group[m].m_model_file, NULL, ri);

                std::vector<int> frames_start;
                if (node)
                    node->get("frame-start", &frames_start);

                std::vector<int> frames_end;
                if (node)
                    node->get("frame-end", &frames_end);

                if (frames_start.empty() && frames_end.empty())
                {
                    frames_start.push_back(scene_node->getStartFrame());
                    frames_end.push_back(scene_node->getEndFrame());
                }
                assert(frames_start.size() == frames_end.size());
                for (unsigned int i = 0 ; i < frames_start.size() ; i++)
                    scene_node->addAnimationSet(frames_start[i], frames_end[i]);
                scene_node->useAnimationSet(0);

                m_track->handleAnimatedTextures(scene_node, *group[m].m_xml);

                lod_node->add(group[m].m_distance, scene_node, true);
            }
            else
            {
                scene::IMesh* a_mesh = irr_driver->getMesh(group[m].m_model_file);
                if (!a_mesh)
                {
                    Log::warn("LODNodeLoad", "Warning: object model '%s' not found, ignored.\n",
                        group[m].m_model_file.c_str());
                    continue;
                }

                a_mesh = MeshTools::createMeshWithTangents(a_mesh, &MeshTools::isNormalMap);
                irr_driver->setAllMaterialFlags(a_mesh);

                a_mesh->grab();
                //cache.push_back(a_mesh);
                irr_driver->grabAllTextures(a_mesh);
                m_track->addCachedMesh(a_mesh);
                scene::IMeshSceneNode* scene_node = irr_driver
                    ->addMesh(a_mesh, group[m].m_model_file, NULL, ri);

                m_track->handleAnimatedTextures(scene_node, *group[m].m_xml);

                lod_node->add(group[m].m_distance, scene_node, true);
            }
        }

#ifdef DEBUG
        std::string debug_name = groupname+" (LOD track-object)";
        lod_node->setName(debug_name.c_str());
#endif
        return lod_node;
    }
    else
    {
        Log::warn("ModelDefinitionLoader", "LOD group '%s' is empty", groupname.c_str());
        return NULL;
    }
#else
    return NULL;
#endif
}   // instanciateAsLOD

// ----------------------------------------------------------------------------

void ModelDefinitionLoader::clear()
{
    m_lod_groups.clear();
}   // clear

// ----------------------------------------------------------------------------

scene::IMesh* ModelDefinitionLoader::getFirstMeshFor(const std::string& name)
{
    if (name.size() > 0)
    {
        const std::vector<ModelDefinition>& md = m_lod_groups[name];
        if (!md.empty())
            return irr_driver->getMesh(md[0].m_model_file);
    }
    return NULL;
}   // getFirstMeshFor

// ----------------------------------------------------------------------------

void ModelDefinitionLoader::cleanLibraryNodesAfterLoad()
{
    for (std::map<std::string, XMLNode*>::iterator it = m_library_nodes.begin();
        it != m_library_nodes.end(); it++)
    {
        delete it->second;

        file_manager->popTextureSearchPath();
        file_manager->popModelSearchPath();
    }
}   // cleanLibraryNodesAfterLoad
