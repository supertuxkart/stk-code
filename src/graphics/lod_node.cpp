//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2011-2015 Marianne Gagnon
//  based on code Copyright 2002-2010 Nikolaus Gebhardt
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

#include "graphics/camera.hpp"
#include "graphics/central_settings.hpp"
#include "graphics/irr_driver.hpp"
#include "graphics/lod_node.hpp"
#include "config/user_config.hpp"
#include "karts/abstract_kart.hpp"

#include <ISceneManager.h>
#include <ICameraSceneNode.h>
#include <IMeshSceneNode.h>
#include <IAnimatedMeshSceneNode.h>

/**
  * @param group_name Only useful for getGroupName()
  */
LODNode::LODNode(std::string group_name, scene::ISceneNode* parent,
                 scene::ISceneManager* mgr, s32 id)
    : ISceneNode(parent, mgr, id)
{
    assert(mgr != NULL);
    assert(parent != NULL);

    m_group_name = group_name;

    // At this stage refcount is two: one because of the object being
    // created, and once because it is a child of the parent. Drop once,
    // so that only the reference from the parent is active, causing this
    // node to be deleted when it is removed from the parent.
    drop();

    m_forced_lod = -1;
    m_last_tick = 0;
    m_area = 0;

    m_previous_level = 0;
    m_current_level  = 0;

    m_timer = 0;

    is_in_transition = false;

    //m_node_to_fade_out = 0;
    //m_node_to_fade_in = 0;
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
    if (m_nodes.size() == 0)
        return -1;

    // If a level is forced, use it
    if(m_forced_lod>-1)
        return m_forced_lod;

    Camera* camera = Camera::getActiveCamera();
    if (camera == NULL)
        return (int)m_detail.size() - 1;
    const Vec3 &pos = camera->getCameraSceneNode()->getAbsolutePosition();

    int dist =
        (int)((m_nodes[0]->getAbsolutePosition()).getDistanceFromSQ(pos.toIrrVector() ));
    
    // Based on the complexity of the track we are more or less aggressive with culling
    int complexity = irr_driver->getSceneComplexity();
    // The track has high complexity so we decrease the draw distance by 10%
    if (complexity > 3000 )
    {
        dist += (dist/10);
    }
    // The track has medium complexity, we can increase slightly the draw distance
    else if(complexity > 1500 )
    {
        dist -= (dist/100);
    }
    // The track has low complexity we can increase a lot the draw distance
    else
    {
        dist -= (dist/10);
    }

    for (unsigned int n=0; n<m_detail.size(); n++)
    {
        if (dist < m_detail[n])
        {
            return n;
        }
    }

    return -1;
}  // getLevel

// ---------------------------------------------------------------------------
/** Forces the level of detail to be n. If n>number of levels, the most
 *  detailed level is used. This is used to disable LOD when the end
 *  camera is activated, since it zooms in to the kart. */
void LODNode::forceLevelOfDetail(int n)
{
    m_forced_lod = (n >=(int)m_detail.size()) ? (int)m_detail.size()-1 : n;
}   // forceLevelOfDetail

// ----------------------------------------------------------------------------
void LODNode::OnAnimate(u32 timeMs)
{
    if (isVisible() && m_nodes.size() > 0)
    {
        // update absolute position
        updateAbsolutePosition();

#ifndef SERVER_ONLY
        if (CVS->isGLSL())
        {
            for (size_t i = 0; i < m_nodes.size(); i++)
            {
                m_nodes[i]->setVisible(true);
                m_nodes[i]->OnAnimate(timeMs);
            }
        }
        else
#endif
        {
            int level = getLevel();
            // Assume all the scene node have the same bouding box
            if(level>=0)
                m_nodes[level]->OnAnimate(timeMs);
        }

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

void LODNode::updateVisibility()
{
    if (!isVisible()) return;
    if (m_nodes.size() == 0) return;

    // Don't need to run the computation of the level everytime
    // FIXME actually we need this was causing weird issue in multiplayer
    // TO be fixed after RC (samuncle)
    /*if ((int)(rand()%10) == 1)
    {
        m_current_level = getLevel();
    }*/

    m_current_level = getLevel();
/*
    if (m_previous_level != m_current_level && !is_in_transition)
    {
        is_in_transition = true;
        m_timer = 0;
    }

    if (is_in_transition)
    {
        // Initially we display the previous one along the new one
        for (size_t i = 0; i < m_nodes.size(); i++)
        {
            if (m_current_level == i || m_previous_level == i)
                m_nodes[i]->setVisible(true);
        }

        // We reset counting
        if (m_timer > 20)
        {
            is_in_transition = false;
            m_previous_level = m_current_level;
        }
        m_timer ++;
    }
    else
    {
        for (size_t i = 0; i < m_nodes.size(); i++)
        {
            m_nodes[i]->setVisible(i == m_current_level);
        }
    }*/

    for (size_t i = 0; i < m_nodes.size(); i++)
    {
        m_nodes[i]->setVisible(i == m_current_level);
    }

}

void LODNode::OnRegisterSceneNode()
{

#ifndef SERVER_ONLY
    if (CVS->isGLSL())
    {
        return;
    }
#endif
    
#ifndef SERVER_ONLY
    if (!CVS->isGLSL())
    {
        for (core::list<ISceneNode*>::Iterator it = Children.begin();
            it != Children.end(); it++)
        {
            (*it)->updateAbsolutePosition();
        }
    }
#endif
    scene::ISceneNode::OnRegisterSceneNode();
}

void LODNode::autoComputeLevel(float scale)
{
    m_area *= scale;

    // Amount of details based on user's input
    float agressivity = 1.0;
    if(UserConfigParams::m_geometry_level == 0) agressivity = 1.25;
    if(UserConfigParams::m_geometry_level == 1) agressivity = 1.0;
    if(UserConfigParams::m_geometry_level == 2) agressivity = 0.75;

    // First we try to estimate how far away we need to draw
    float max_draw = 0.0;
    max_draw = sqrtf((0.5 * m_area + 10) * 200) - 10;
    // If the draw distance is too big we artificially reduce it
    if(max_draw > 250)
    {
        max_draw = 235 + (max_draw * 0.06);
    }

    max_draw *= agressivity;

    int step = (int) (max_draw * max_draw) / m_detail.size();

    // Then we recompute the level of detail culling distance
    int biais = m_detail.size();
    for(int i = 0; i < m_detail.size(); i++)
    {
        m_detail[i] = ((step / biais) * (i + 1));
        biais--;
    }
}

void LODNode::add(int level, scene::ISceneNode* node, bool reparent)
{
    Box = node->getBoundingBox();
    m_area = Box.getArea();

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
    node->setPosition(core::vector3df(0,0,0));
    node->setVisible(false);
    m_detail.push_back(level*level);
    m_nodes.push_back(node);
    m_nodes_set.insert(node);
    node->setParent(this);

    if (node->getType() == scene::ESNT_ANIMATED_MESH)
        ((scene::IAnimatedMeshSceneNode *) node)->setReadOnlyMaterials(true);
    if (node->getType() == scene::ESNT_MESH)
        ((scene::IMeshSceneNode *) node)->setReadOnlyMaterials(true);

    node->drop();

    node->updateAbsolutePosition();
}

