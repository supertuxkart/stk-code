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
    m_update_box_every_frame = false;
    assert(mgr != NULL);
    assert(parent != NULL);

    m_group_name = group_name;

    // At this stage refcount is two: one because of the object being
    // created, and once because it is a child of the parent. Drop once,
    // so that only the reference from the parent is active, causing this
    // node to be deleted when it is removed from the parent.
    drop();

    m_forced_lod = -1;
    m_area = 0;
#ifndef SERVER_ONLY
    if (!CVS->isGLSL())
    {
        m_current_level.reset(new int);
        *m_current_level = -1;
    }
#endif
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

    const int dist =
        (int)((m_nodes[0]->getAbsolutePosition()).getDistanceFromSQ(pos.toIrrVector() ));

    for (unsigned int n=0; n<m_detail.size(); n++)
    {
        if (dist < m_detail[n])
            return n;
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
            *m_current_level = level;
            // Assume all the scene node have the same bouding box
            if(level>=0)
            {
                m_nodes[level]->setVisible(true);
                m_nodes[level]->OnAnimate(timeMs);
            }
        }

        if (m_update_box_every_frame)
            Box = m_nodes[m_detail.size() - 1]->getBoundingBox();

        // If this node has children other than the LOD nodes, animate it
        for (unsigned i = 0; i < Children.size(); ++i)
        {
            if (m_nodes_set.find(Children[i]) == m_nodes_set.end())
            {
                assert(Children[i] != NULL);
                if (Children[i]->isVisible())
                {
                    Children[i]->OnAnimate(timeMs);
                }
            }
        }

    }
}

void LODNode::updateVisibility(bool* shown)
{
    if (!isVisible()) return;
    if (m_nodes.size() == 0) return;

    unsigned int level = 0;
    if (m_current_level)
        level = *m_current_level;
    else
        level = getLevel();
    for (size_t i = 0; i < m_nodes.size(); i++)
    {
        m_nodes[i]->setVisible(i == level);
        if (i == level && shown != NULL)
            *shown = (i > 0);
    }
}

void LODNode::OnRegisterSceneNode()
{
    bool shown = false;
    updateVisibility(&shown);

#ifndef SERVER_ONLY
    if (CVS->isGLSL())
    {
        return;
    }
#endif

#ifndef SERVER_ONLY
    if (!CVS->isGLSL())
    {
        for (unsigned i = 0; i < Children.size(); ++i)
            Children[i]->updateAbsolutePosition();
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
        max_draw = 250 + (max_draw * 0.06);
    }

    max_draw *= agressivity;

    int step = (int) (max_draw * max_draw) / m_detail.size();

    // Then we recompute the level of detail culling distance
    int biais = m_detail.size();
    for(unsigned i = 0; i < m_detail.size(); i++)
    {
        m_detail[i] = ((step / biais) * (i + 1));
        biais--;
    }
    const size_t max_level = m_detail.size() - 1;

    // Only animated mesh needs to be updated bounding box every frame,
    // which only affects culling
    m_update_box_every_frame =
        m_nodes[max_level]->getType() == scene::ESNT_ANIMATED_MESH ||
        m_nodes[max_level]->getType() == scene::ESNT_LOD_NODE;
    Box = m_nodes[max_level]->getBoundingBox();
}

void LODNode::add(int level, scene::ISceneNode* node, bool reparent)
{
    Box = node->getBoundingBox();
    m_area = Box.getArea();

    // samuncle suggested to put a slight randomisation in LOD
    // I'm not convinced (Auria) but he's the artist pro, so I listen ;P
    // The last level should not be randomized because after that the object disappears,
    // and the location is disapparition needs to be deterministic
    if (m_detail.size() > 0 && m_detail.back() < level * level)
    {
        m_detail[m_detail.size() - 1] += (int)(((rand()%1000)-500)/500.0f*(m_detail[m_detail.size() - 1]*0.2f));
    }

    assert(node != NULL);

    node->grab();
    node->remove();
    node->setPosition(core::vector3df(0,0,0));
    m_detail.push_back(level*level);
    m_nodes.push_back(node);
    m_nodes_set.insert(node);
    node->setParent(this);

    node->drop();

    node->updateAbsolutePosition();
    node->setNeedsUpdateAbsTrans(true);
}
