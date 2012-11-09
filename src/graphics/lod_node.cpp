//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2011 Marianne Gagnon
//  based on code Copyright (C) 2002-2010 Nikolaus Gebhardt
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

#include "graphics/irr_driver.hpp"
#include "graphics/lod_node.hpp"
#include "graphics/hardware_skinning.hpp"
#include "graphics/material_manager.hpp"
#include "graphics/material.hpp"
#include "config/user_config.hpp"

#include <ISceneManager.h>
#include <ICameraSceneNode.h>
#include <IMeshSceneNode.h>
#include <IAnimatedMeshSceneNode.h>

/**
  * @param group_name Only useful for getGroupName()
  */
LODNode::LODNode(std::string group_name, scene::ISceneNode* parent,
                 scene::ISceneManager* mgr, s32 id)
    : ISceneNode(parent, mgr, id) //: IDummyTransformationSceneNode(parent, mgr, id)
{
    assert(mgr != NULL);
    assert(parent != NULL);
    
    m_group_name = group_name;
    
    m_previous_visibility = FIRST_PASS;
    
    // At this stage refcount is two: one because of the object being 
    // created, and once because it is a child of the parent. Drop once,
    // so that only the reference from the parent is active, causing this
    // node to be deleted when it is removed from the parent.
    drop();
}

LODNode::~LODNode()
{
}

void LODNode::render()
{
    //ISceneNode::render();
}

/** Returns the level to use, or -1 if the object is too far
 *  away.
 */
int LODNode::getLevel()
{
    // TODO: optimize this, there is no need to check every frame
    scene::ICameraSceneNode* curr_cam = irr_driver->getSceneManager()->getActiveCamera();

    // Assumes all children are at the same location
    const int dist = 
        (int)((getPosition() + m_nodes[0]->getPosition()).getDistanceFromSQ( curr_cam->getPosition() ));
    
    for (unsigned int n=0; n<m_detail.size(); n++)
    {
        if (dist < m_detail[n])
          return n;
    }
    return -1;
}  // getLevel


void LODNode::OnAnimate(u32 timeMs)
{
    if (isVisible())
    {
        // update absolute position
        updateAbsolutePosition();
        
        int level = getLevel();
        // Assume all the scene node have the same bouding box
        if(level>=0)
            m_nodes[level]->OnAnimate(timeMs);

        Box = m_nodes[m_detail.size()-1]->getBoundingBox();

        // If this node has children other than the LOD nodes, animate it
        core::list<ISceneNode*>::Iterator it;
        for (it = Children.begin(); it != Children.end(); it++)
        {
            if (m_nodes_set.find(*it) == m_nodes_set.end())
            {
                assert(*it != NULL);
                if ((*it)->isVisible())
                {
                    (*it)->OnAnimate(timeMs);
                }
            }
        }
			
    }
}

void LODNode::OnRegisterSceneNode()
{
    if (!isVisible()) return;
    if (m_nodes.size() == 0) return;

    bool shown = false;
    int level = getLevel();
    if (level>=0)
    {
        m_nodes[level]->updateAbsolutePosition();
        m_nodes[level]->OnRegisterSceneNode();
        shown = true;
    }
        
    // support an optional, mostly hard-coded fade-in/out effect for objects with a single level
    if (m_nodes.size() == 1 && (m_nodes[0]->getType() == scene::ESNT_MESH ||
                                m_nodes[0]->getType() == scene::ESNT_ANIMATED_MESH))
    {
        if (m_previous_visibility == WAS_HIDDEN && shown)
        {            
            scene::IMesh* mesh;
            
            if (m_nodes[0]->getType() == scene::ESNT_MESH)
            {
                scene::IMeshSceneNode* node = (scene::IMeshSceneNode*)(m_nodes[0]);
                mesh = node->getMesh();
            }
            else
            {
                assert(m_nodes[0]->getType() == scene::ESNT_ANIMATED_MESH);
                scene::IAnimatedMeshSceneNode* node =
                    (scene::IAnimatedMeshSceneNode*)(m_nodes[0]);
                assert(node != NULL);
                mesh = node->getMesh();
            }
            
            for (unsigned int n=0; n<mesh->getMeshBufferCount(); n++)
            {
                scene::IMeshBuffer* mb = mesh->getMeshBuffer(n);
                video::ITexture* t = mb->getMaterial().getTexture(0);
                if (t == NULL) continue;
                
                Material* m = material_manager->getMaterialFor(t, mb);
                if (m != NULL)
                {
                    m->onMadeVisible(mb);
                }
            }
        }
        else if (m_previous_visibility == WAS_SHOWN && !shown)
        {           
            scene::IMesh* mesh;
            
            if (m_nodes[0]->getType() == scene::ESNT_MESH)
            {
                scene::IMeshSceneNode* node = (scene::IMeshSceneNode*)(m_nodes[0]);
                mesh = node->getMesh();
            }
            else
            {
                assert(m_nodes[0]->getType() == scene::ESNT_ANIMATED_MESH);
                scene::IAnimatedMeshSceneNode* node =
                    (scene::IAnimatedMeshSceneNode*)(m_nodes[0]);
                assert(node != NULL);
                mesh = node->getMesh();
            }
            
            for (unsigned int n=0; n<mesh->getMeshBufferCount(); n++)
            {
                scene::IMeshBuffer* mb = mesh->getMeshBuffer(n);
                video::ITexture* t = mb->getMaterial().getTexture(0);
                if (t == NULL) continue;
                Material* m = material_manager->getMaterialFor(t, mb);
                if (m != NULL)
                {
                    m->onHidden(mb);
                }
            }
        }
        else if (m_previous_visibility == FIRST_PASS && !shown)
        {            
            scene::IMesh* mesh;
            
            if (m_nodes[0]->getType() == scene::ESNT_MESH)
            {
                scene::IMeshSceneNode* node = (scene::IMeshSceneNode*)(m_nodes[0]);
                mesh = node->getMesh();
            }
            else
            {
                assert(m_nodes[0]->getType() == scene::ESNT_ANIMATED_MESH);
                scene::IAnimatedMeshSceneNode* node =
                (scene::IAnimatedMeshSceneNode*)(m_nodes[0]);
                assert(node != NULL);
                mesh = node->getMesh();
            }
            
            for (unsigned int n=0; n<mesh->getMeshBufferCount(); n++)
            {
                scene::IMeshBuffer* mb = mesh->getMeshBuffer(n);
                video::ITexture* t = mb->getMaterial().getTexture(0);
                if(!t) continue;
                Material* m = material_manager->getMaterialFor(t, mb);
                if (m != NULL)
                {
                    m->isInitiallyHidden(mb);
                }
            }
        }        
    }
    
    m_previous_visibility = (shown ? WAS_SHOWN : WAS_HIDDEN); 
    
    // If this node has children other than the LOD nodes, draw them
    core::list<ISceneNode*>::Iterator it;
    
    for (it = Children.begin(); it != Children.end(); it++)
    {
        if (m_nodes_set.find(*it) == m_nodes_set.end())
        {
            assert(*it != NULL);
            if ((*it)->isVisible())
            {
                (*it)->OnRegisterSceneNode();
            }
        }
    }
}

void LODNode::add(int level, scene::ISceneNode* node, bool reparent)
{
    // samuncle suggested to put a slight randomisation in LOD
    // I'm not convinced (Auria) but he's the artist pro, so I listen ;P
    // The last level should not be randomized because after that the object disappears,
    // and the location is disapparition needs to be deterministic
    if (m_detail.size() > 0)
    {
        assert(m_detail.back()<level*level);
        m_detail[m_detail.size() - 1] += (int)(((rand()%1000)-500)/500.0f*(m_detail[m_detail.size() - 1]*0.2f));
    }
    
    assert(node != NULL);
    
    node->grab();
    node->remove();
    m_detail.push_back(level*level);
    m_nodes.push_back(node);
    m_nodes_set.insert(node);
    node->setParent(this);
    
    if(UserConfigParams::m_hw_skinning_enabled && node->getType() == scene::ESNT_ANIMATED_MESH)
        HardwareSkinning::prepareNode((scene::IAnimatedMeshSceneNode*)node);
    
    node->drop();
}
