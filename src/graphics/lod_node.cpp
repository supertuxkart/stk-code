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
#include "config/user_config.hpp"

#include <ISceneManager.h>
#include <ICameraSceneNode.h>

LODNode::LODNode(scene::ISceneNode* parent, scene::ISceneManager* mgr, s32 id)
: ISceneNode(parent, mgr, id) //: IDummyTransformationSceneNode(parent, mgr, id)
{
    assert(mgr != NULL);
    assert(parent != NULL);
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
    if (isVisible())
        ISceneNode::OnRegisterSceneNode();
    //ISceneNode::render();
}

void LODNode::OnRegisterSceneNode()
{
    if (!isVisible()) return;
    
    // TODO: optimize this, there is no need to check every frame
    scene::ICameraSceneNode* curr_cam = irr_driver->getSceneManager()->getActiveCamera();

    // Assumes all children are at the same location
    const int dist = 
        (int)((getPosition() + m_nodes[0]->getPosition()).getDistanceFromSQ( curr_cam->getPosition() ));
        
    for (unsigned int n=0; n<m_detail.size(); n++)
    {
        if (dist < m_detail[n])
        {
            m_nodes[n]->OnRegisterSceneNode();
            break;
        }
    }
    
    // If this node has children other than the LOD nodes, draw them
    core::list<ISceneNode*>::Iterator it;
    
    for (it = Children.begin(); it != Children.end(); it++)
    {
        if (m_nodes_set.find(*it) == m_nodes_set.end())
        {
            assert(*it != NULL);
            (*it)->OnRegisterSceneNode();
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
