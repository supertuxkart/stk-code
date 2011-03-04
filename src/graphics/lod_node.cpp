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


LODNode::LODNode(scene::ISceneNode* parent, scene::ISceneManager* mgr, s32 id)
    : IDummyTransformationSceneNode(parent, mgr, id)
{
    parent->addChild(this);
}

LODNode::~LODNode()
{
}

void LODNode::render()
{
    ISceneNode::OnRegisterSceneNode();
    //ISceneNode::render();
}

void LODNode::OnRegisterSceneNode()
{
    // TODO: optimize this, there is no need to check every frame
    scene::ICameraSceneNode* curr_cam = irr_driver->getSceneManager()->getActiveCamera();

    // Assumes all children are at the same location
    const int dist = m_nodes[0]->getPosition().getDistanceFromSQ( curr_cam->getPosition() );
        
    for (unsigned int n=0; n<m_detail.size(); n++)
    {
        if (dist < m_detail[n])
        {
            m_nodes[n]->OnRegisterSceneNode();
            return;
        }
    }
    
    if (dist >= m_detail[m_nodes.size()-1])
    {
        m_nodes[m_nodes.size()-1]->OnRegisterSceneNode();
    }
    else
    {
        // This probably means the first level of detail is not 0
        assert(false);
    }
    //ISceneNode::OnRegisterSceneNode();
}

void LODNode::add(int level, scene::ISceneNode* node, bool reparent)
{
    // FIXME : this class assumes that 'm_detail' is sorted, this may not always be the case
    node->grab();
    node->remove();
    m_detail.push_back(level*level);
    m_nodes.push_back(node);
    node->setParent(this);
    node->drop();
}
