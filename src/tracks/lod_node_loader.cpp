//  $Id$
//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2007 Joerg Henrichs
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

#include "tracks/lod_node_loader.hpp"
using namespace irr;

#include "graphics/irr_driver.hpp"
#include "graphics/lod_node.hpp"
#include "io/xml_node.hpp"

#include <IAnimatedMeshSceneNode.h>
#include <ISceneManager.h>

LodNodeLoader::LodNodeLoader()
{
}

// ----------------------------------------------------------------------------

bool PairCompare(const std::pair<int, std::string>& i, const std::pair<int, std::string>& j)
{
    return (i.first < j.first);
}

// ----------------------------------------------------------------------------

/** Check a XML node in case it contains a LOD object and if so remember it */
bool LodNodeLoader::check(const XMLNode* xml)
{
    float lod_distance = -1.0f;
    xml->get("lod_distance", &lod_distance);
    
    bool lod_instance = false;
    xml->get("lod_instance", &lod_instance);
    
    std::string lodgroup;
    xml->get("lod_group", &lodgroup);
    
    if (!lodgroup.empty())
    {
        if (lod_instance)
        {
            lod_instances[lodgroup].push_back(xml);
        }
        else
        {
            std::string model_name;
            xml->get("model", &model_name);
            
            lod_groups[lodgroup][(int)lod_distance] = model_name;
        }
        return true;
    }
    else
    {
        return false;
    }
}

// ----------------------------------------------------------------------------

/**
  * Call when the XML file is fully parsed and we're ready to create the node
  * @param cache the individual meshes will be added there
  * @param[out] out the nodes are added here
  */
void LodNodeLoader::done(std::string directory,
                         std::vector<scene::IMesh*>& cache,
                         std::vector<LODNode*>& out)
{
    scene::ISceneManager* sm = irr_driver->getSceneManager();
    scene::ISceneNode* sroot = sm->getRootSceneNode();
    
    // Creating LOD nodes is more complicated than one might have hoped, on the C++ side;
    // but it was done this way to minimize the work needed on the side of the artists
    
    // 1. Sort LOD groups (highest detail first, lowest detail last)
    std::map<std::string, std::vector< std::pair<int, std::string> > > sorted_lod_groups;
    
    std::map<std::string, std::map<int, std::string> >::iterator it;
    for (it = lod_groups.begin(); it != lod_groups.end(); it++)
    {
        std::map<int, std::string>::iterator it2;
        for (it2 = it->second.begin(); it2 != it->second.end(); it2++)
        {
            //printf("Copying before sort : (%i) %s is in group %s\n", it2->first, it2->second.c_str(), it->first.c_str());
            sorted_lod_groups[it->first].push_back( std::pair<int, std::string>(it2->first, it2->second) );
        }
        std::sort( sorted_lod_groups[it->first].begin(), sorted_lod_groups[it->first].end(), PairCompare );
        
        //printf("Group '%s' :\n", it->first.c_str());
        //for (unsigned int x=0; x<sorted_lod_groups[it->first].size(); x++)
        //{
        //    printf("  - (%i) %s\n", sorted_lod_groups[it->first][x].first, sorted_lod_groups[it->first][x].second.c_str());
        //}
    }
    
    // 2. Read the XML nodes and instanciate LOD scene nodes where relevant
    std::string groupname;
    std::map< std::string, std::vector< const XMLNode* > >::iterator it3;
    for (it3 = lod_instances.begin(); it3 != lod_instances.end(); it3++)
    {
        std::vector< std::pair<int, std::string> >& group = sorted_lod_groups[it3->first];
        
        std::vector< const XMLNode* >& v = it3->second;
        for (unsigned int n=0; n<v.size(); n++)
        {
            const XMLNode* node = v[n];
            
            if(node->getName()!="static-object")
            {
                fprintf(stderr, "Incorrect tag '%s' used in LOD instance - ignored\n",
                        node->getName().c_str());
                continue;
            }
            
            groupname = "";
            node->get("lod_group", &groupname);
            //if (model_name != sorted_lod_groups[it3->first][0].second) continue;
            
            core::vector3df xyz(0,0,0);
            node->get("xyz", &xyz);
            core::vector3df hpr(0,0,0);
            node->get("hpr", &hpr);
            core::vector3df scale(1.0f, 1.0f, 1.0f);
            node->get("scale", &scale);
            
            std::string full_path;
            
            if (group.size() > 0)
            {
                LODNode* lod_node = new LODNode(sroot, sm);
                for (unsigned int m=0; m<group.size(); m++)
                {
                    full_path = directory + "/" + group[m].second;
                    
                    // TODO: check whether the mesh contains animations or not, and use a static
                    //       mesh when there are no animations?
                    scene::IAnimatedMesh *a_mesh = irr_driver->getAnimatedMesh(full_path);
                    if(!a_mesh)
                    {
                        fprintf(stderr, "Warning: object model '%s' not found, ignored.\n",
                                full_path.c_str());
                        continue;
                    }
                    a_mesh->grab();
                    cache.push_back(a_mesh);
                    irr_driver->grabAllTextures(a_mesh);
                    scene::IAnimatedMeshSceneNode* scene_node = 
                        irr_driver->addAnimatedMesh(a_mesh);
                    scene_node->setPosition(xyz);
                    scene_node->setRotation(hpr);
                    scene_node->setScale(scale);
                    
                    lod_node->add( group[m].first, scene_node, true );
                }
                
#ifdef DEBUG
                std::string debug_name = groupname+" (LOD track-object)";
                lod_node->setName(debug_name.c_str());
#endif
                
                out.push_back(lod_node);
            }
            else
            {
                fprintf(stderr, "[LodNodeLoader] WARNING, LOD group '%s' is empty\n", groupname.c_str());
            }
        }
    } // end for
}
