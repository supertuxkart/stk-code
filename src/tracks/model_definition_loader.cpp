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
#include "io/xml_node.hpp"
#include "modes/world.hpp"
#include "tracks/track.hpp"

#include <IAnimatedMeshSceneNode.h>
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

LODNode* ModelDefinitionLoader::instanciateAsLOD(const XMLNode* node, scene::ISceneNode* parent, std::shared_ptr<GE::GERenderInfo> ri)
{
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
#ifndef SERVER_ONLY
            if (group[m].m_skeletal_animation &&
                (UserConfigParams::m_animated_characters ||
                World::getWorld()->getIdent() == IDENT_CUTSCENE))
            {
                scene::IAnimatedMesh* a_mesh = irr_driver->getAnimatedMesh(group[m].m_model_file);
                if (!a_mesh)
                {
                    Log::warn("LODNodeLoad", "Warning: object model '%s' not found, ignored.\n",
                        group[m].m_model_file.c_str());
                    continue;
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

                Track::uploadNodeVertexBuffer(scene_node);
                lod_node->add(group[m].m_distance, scene_node, true);
            }
            else
#endif
            {
                scene::IMesh* a_mesh = irr_driver->getMesh(group[m].m_model_file);
                if (!a_mesh)
                {
                    Log::warn("LODNodeLoad", "Warning: object model '%s' not found, ignored.\n",
                        group[m].m_model_file.c_str());
                    continue;
                }

                irr_driver->setAllMaterialFlags(a_mesh);

                a_mesh->grab();
                //cache.push_back(a_mesh);
                irr_driver->grabAllTextures(a_mesh);
                m_track->addCachedMesh(a_mesh);
                scene::ISceneNode* scene_node = irr_driver
                    ->addMesh(a_mesh, group[m].m_model_file, NULL, ri);

                m_track->handleAnimatedTextures(scene_node, *group[m].m_xml);

                Track::uploadNodeVertexBuffer(scene_node);
                lod_node->add(group[m].m_distance, scene_node, true);
            }
        }
        if (lod_node->getAllNodes().empty())
        {
            Log::warn("ModelDefinitionLoader",
                "Nothing has been added to LOD node '%s'.", groupname.c_str());
            irr_driver->removeNode(lod_node);
            return NULL;
        }
        vector3df scale = vector3df(1.f, 1.f, 1.f);
        node->get("scale", &scale);
        lod_node->autoComputeLevel(scale.getLength());

#ifdef DEBUG
        std::string debug_name = groupname+" (LOD track-object)";
        lod_node->setName(debug_name.c_str());
#endif
        lod_node->setNeedsUpdateAbsTrans(true);
        return lod_node;
    }
    else
    {
        Log::warn("ModelDefinitionLoader", "LOD group '%s' is empty", groupname.c_str());
        return NULL;
    }
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
