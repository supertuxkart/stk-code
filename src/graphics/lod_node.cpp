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

#include "graphics/camera/camera.hpp"
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
    m_current_level = -1;
    m_current_level_dirty = true;
    m_lod_distances_updated = false;
    m_min_switch_distance = 0;
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
    if (m_forced_lod >- 1)
        return m_forced_lod;
    
    if (!m_current_level_dirty)
        return m_current_level;
    m_current_level_dirty = false;

    Camera* camera = Camera::getActiveCamera();
    if (camera == NULL)
        return (int)m_detail.size() - 1;
    const Vec3 &pos = camera->getCameraSceneNode()->getAbsolutePosition();

    const int squared_dist =
        (int)((m_nodes[0]->getAbsolutePosition()).getDistanceFromSQ(pos.toIrrVector() ));

    if (!m_lod_distances_updated)
    {
        for (unsigned int n=0; n<m_detail.size(); n++)
        {
            m_detail[n] = (int)((float)m_detail[n] * irr_driver->getLODMultiplier());
        }
        m_lod_distances_updated = true;
    }

    // The LoD levels are ordered from highest quality to lowest
    for (unsigned int n=0; n<m_detail.size(); n++)
    {
        // Display this level if close enough.
        // If a high-level of detail would only be triggered from too close,
        // and there are lower levels available, skip it completely.
        // It's better to display a low level than to have the high-level pop suddenly.
        if (squared_dist < m_detail[n] &&
            ((n + 1 >= m_detail.size()) || (m_min_switch_distance < m_detail[n])))
        {
            m_current_level = n;
            return n;
        }
    }
    m_current_level = -1;
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

        for (size_t i = 0; i < m_nodes.size(); i++)
        {
            m_nodes[i]->setVisible(true);
            m_nodes[i]->OnAnimate(timeMs);
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
    } // if isVisible() && m_nodes.size() > 0
}

void LODNode::updateVisibility()
{
    if (!isVisible()) return;
    if (m_nodes.size() == 0) return;

    m_current_level_dirty = true;
    unsigned int level = getLevel();

    for (size_t i = 0; i < m_nodes.size(); i++)
    {
        m_nodes[i]->setVisible(i == level);
    }
}

void LODNode::OnRegisterSceneNode()
{
    updateVisibility();

#ifndef SERVER_ONLY
    if (CVS->isGLSL())
    {
        return;
    }
#endif

    if (isVisible() && m_nodes.size() > 0)
    {
        int level = getLevel();
        
        if (level >= 0)
        {
            m_nodes[level]->OnRegisterSceneNode();
        }
        for (unsigned i = 0; i < Children.size(); i++)
        {
            if (m_nodes_set.find(Children[i]) == m_nodes_set.end())
                Children[i]->OnRegisterSceneNode();
        }
    } // if isVisible() && m_nodes.size() > 0
}

/* Each model with LoD has specific distances beyond which it is rendered at a lower 
* detail level. This function compute the distances associated with the various
* LoD levels for a given model.
* @param scale The model's scale*/
void LODNode::autoComputeLevel(float scale)
{
    m_area *= scale;

    // We want to determine two things:
    // 1.) beyond which distance the object should disappear entirely
    // 2.) at which distances we should switch between a lower-quality model and a higher quality model,
    //     if these distances are too low we stick to a lower-quality model
    // To do so, we use a mix between a power 2 (quadratic) formula and a power 4 formula.

    // Step 1a - compute the base formulas
    float p2_component = 20.0f + 7.0f * sqrtf(m_area + 10);
    float p4_component = 50.0f + 22.0f * sqrtf(sqrtf(m_area + 2));

    // Step 1b - combine them
    float p4_ratio = p4_component;
    if (p4_component < 210.0f)
        p4_ratio = p4_component / 220.0f;
    else if (p4_component < 230.0f) // Smooth the transition
        p4_ratio = (105.0f + p4_component * 0.5f) / 220.0f;
    else
        p4_ratio = 1.0f;

    float max_draw = p4_ratio * p4_component + (1.0f - p4_ratio) * p2_component;

    // Step 2a - Distance multiplier based on the user's input
    float aggressivity = 1.0;
    if(     UserConfigParams::m_geometry_level == 0) aggressivity = 1.0;
    else if(UserConfigParams::m_geometry_level == 1) aggressivity = 1.42;
    else if(UserConfigParams::m_geometry_level == 2) aggressivity = 2.0;
    else if(UserConfigParams::m_geometry_level == 3) aggressivity = 2.84;
    else if(UserConfigParams::m_geometry_level == 4) aggressivity = 4.0;
    else if(UserConfigParams::m_geometry_level == 5) aggressivity = 5.7;

    max_draw *= aggressivity;

    // Step 2b - Determine the minimum distance for a model switch based on user input
    float temp_switch_dist = max_draw;
    if(     UserConfigParams::m_geometry_level == 0) temp_switch_dist *= 0.75f;
    else if(UserConfigParams::m_geometry_level == 1) temp_switch_dist *= 0.55f;
    else if(UserConfigParams::m_geometry_level == 2) temp_switch_dist *= 0.4f;
    else if(UserConfigParams::m_geometry_level == 3) temp_switch_dist *= 0.3f;
    else if(UserConfigParams::m_geometry_level == 4) temp_switch_dist *= 0.23f;
    else if(UserConfigParams::m_geometry_level == 5) temp_switch_dist *= 0.18f;

    temp_switch_dist = (temp_switch_dist + 100.0f) / 2.0f;

    m_min_switch_distance = (int)temp_switch_dist;

    // Step 3 - As it is faster to compute the squared distance than distance, at runtime
    //          we compare the distance saved in the LoD node with the square of the distance
    //          between the camera and the object. Therefore, we apply squaring here.
    max_draw *= max_draw;
    m_min_switch_distance *= m_min_switch_distance;

    // Step 4 - Then we recompute the level of detail culling distance
    //          If there are N levels of detail, the highest level
    //          is displayed when under 1/Nth of the max display distance.
    //          Other levels are spaced approximately at the geometrical mean points.
    int step = (int) (max_draw) / m_detail.size();
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
