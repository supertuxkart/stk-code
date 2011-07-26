//  $Id$
//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2009 Joerg Henrichs
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

#include "tracks/track_object_manager.hpp"

#include "config/user_config.hpp"
#include "animations/billboard_animation.hpp"
#include "animations/three_d_animation.hpp"
#include "graphics/lod_node.hpp"
#include "graphics/material_manager.hpp"
#include "io/xml_node.hpp"
#include "physics/physical_object.hpp"
#include "tracks/track_object.hpp"

#include <IMeshSceneNode.h>

TrackObjectManager::TrackObjectManager()
{
}   // TrackObjectManager

// ----------------------------------------------------------------------------
TrackObjectManager::~TrackObjectManager()
{
}   // ~TrackObjectManager

// ----------------------------------------------------------------------------
/** Adds an object to the track object manager. The type to add is specified
 *  in the xml_node.
 */
void TrackObjectManager::add(const XMLNode &xml_node)
{
    std::string type;
    xml_node.get("type", &type);
    if(type=="movable")
    {
        m_all_objects.push_back(new PhysicalObject(xml_node));
    }
    else if(type=="animation")
    {
        m_all_objects.push_back(new ThreeDAnimation(xml_node));
    }
    else if(type=="billboard")
    {
        m_all_objects.push_back(new BillboardAnimation(xml_node));
    }
    else
    {
        fprintf(stderr, "Unknown track object: '%s' - ignored.\n", 
                type.c_str());
    }
}   // add

// ----------------------------------------------------------------------------
/** Initialises all track objects.
 */
void TrackObjectManager::init()
{
    TrackObject* curr;
    for_in (curr, m_all_objects)
    {
        curr->init();
    }
}   // reset
// ----------------------------------------------------------------------------
/** Initialises all track objects.
 */
void TrackObjectManager::reset()
{
    TrackObject* curr;
    for_in (curr, m_all_objects)
    {
        curr->reset();
    }
}   // reset

// ----------------------------------------------------------------------------
/** Handles an explosion, i.e. it makes sure that all physical objects are
 *  affected accordingly.
 *  \param pos  Position of the explosion.
 *  \param obj  If the hit was a physical object, this object will be affected
 *              more. Otherwise this is NULL.
 */

void TrackObjectManager::handleExplosion(const Vec3 &pos, const PhysicalObject *mp)
{
    TrackObject* curr;
    for_in (curr, m_all_objects)
    {
        curr->handleExplosion(pos, mp == curr);
    }
}   // handleExplosion

// ----------------------------------------------------------------------------
/** Updates all track objects.
 *  \param dt Time step size.
 */
void TrackObjectManager::update(float dt)
{
    if ( UserConfigParams::m_graphical_effects )
    {
        TrackObject* curr;
        for_in (curr, m_all_objects)
        {
            curr->update(dt);
        }
    }
}   // update

// ----------------------------------------------------------------------------
/** Enables or disables fog for a given scene node.
 *  \param node The node to adjust.
 *  \param enable True if fog is enabled, otherwise fog is disabled.
 */
void adjustForFog(scene::ISceneNode *node, bool enable)
{
    if (node->getType() == scene::ESNT_MESH   || 
        node->getType() == scene::ESNT_OCTREE || 
        node->getType() == scene::ESNT_ANIMATED_MESH)
    {
        scene::IMesh* mesh;
        if (node->getType() == scene::ESNT_ANIMATED_MESH) {
            mesh = ((scene::IAnimatedMeshSceneNode*)node)->getMesh();
        }
        else {
            mesh = ((scene::IMeshSceneNode*)node)->getMesh();
        }
        
        unsigned int n = mesh->getMeshBufferCount();
        for (unsigned int i=0; i<n; i++)
        {
            scene::IMeshBuffer *mb = mesh->getMeshBuffer(i);
            video::SMaterial &irr_material=mb->getMaterial();
            for (unsigned int j=0; j<video::MATERIAL_MAX_TEXTURES; j++)
            {
                video::ITexture* t = irr_material.getTexture(j);
                if (t) material_manager->adjustForFog(t, mb, node, enable);
                
            }   // for j<MATERIAL_MAX_TEXTURES
        }  // for i<getMeshBufferCount()
    }
    else
    {
        node->setMaterialFlag(video::EMF_FOG_ENABLE, enable);
    }
    
    if (node->getType() == scene::ESNT_LOD_NODE)
    {
        std::vector<scene::ISceneNode*>& 
            subnodes = ((LODNode*)node)->getAllNodes();
        for (unsigned int n=0; n<subnodes.size(); n++)
        {
            adjustForFog(subnodes[n], enable);
        }
    }
}   // adjustForFog


// ----------------------------------------------------------------------------
/** Enable or disable fog on objects.
  */
void TrackObjectManager::enableFog(bool enable)
{
    TrackObject* curr;
    for_in (curr, m_all_objects)
    {
        adjustForFog(curr->getNode(), enable);
    }
}   // enableFog
