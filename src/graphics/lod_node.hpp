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

#ifndef HEADER_LOD_NODE_HPP
#define HEADER_LOD_NODE_HPP

#include <aabbox3d.h>
#include <matrix4.h>
#include <ISceneNode.h>
#include <vector>
#include <string>

namespace irr
{
    namespace scene { class ISceneManager; class ISceneNode; }
}
using namespace irr;

#include <set>

namespace irr
{
    namespace scene
    {
        const int ESNT_LOD_NODE = MAKE_IRR_ID('l','o','d','n');
    }
}

/**
 * \brief manages level-of-detail
 * \ingroup graphics
 */
class LODNode : public scene::ISceneNode
{
private:
    core::matrix4 RelativeTransformationMatrix;
    core::aabbox3d<f32> Box;

    std::vector<int> m_detail;
    std::vector<irr::scene::ISceneNode*> m_nodes;

    std::set<scene::ISceneNode*> m_nodes_set;

    std::string m_group_name;

    /** The normal level of detail can be overwritten. If
     *  m_forced_lod is >=0, only this level is be used. */
    int m_forced_lod;

    enum PreviousVisibility
    {
        FIRST_PASS,
        WAS_SHOWN,
        WAS_HIDDEN
    };

    PreviousVisibility m_previous_visibility;

    u32 m_last_tick;

public:

    LODNode(std::string group_name, scene::ISceneNode* parent, scene::ISceneManager* mgr, s32 id=-1);
    virtual     ~LODNode();

    //! returns the axis aligned bounding box of this node
    virtual const core::aabbox3d<f32>& getBoundingBox() const { return Box; }

    int getLevel();

    void updateVisibility(bool* shown = NULL);

    /*
    //! Returns a reference to the current relative transformation matrix.
    //! This is the matrix, this scene node uses instead of scale, translation
    //! and rotation.
    virtual core::matrix4& getRelativeTransformationMatrix() { return RelativeTransformationMatrix; }

    //! Returns the relative transformation of the scene node.
    virtual core::matrix4 getRelativeTransformation() const { return RelativeTransformationMatrix; }
    */

    /**
      * Adds a node associated with a level of detail.
      * \note The LOD levels must be added in ascending order.
      * \param level Distance (number of units) from which this level of detail kicks in
      * \param node  The node to show at this level
      * \param reparent If true, node will be removed from its current parent first
      */
    void add(int level, scene::ISceneNode* node, bool reparent);

    void forceLevelOfDetail(int n);

    /** Get the highest level of detail node */
    scene::ISceneNode* getFirstNode()
    {
        if (m_nodes.size() > 0) return m_nodes[0];
        else                    return NULL;
    }

    std::vector<scene::ISceneNode*>& getAllNodes() { return m_nodes; }

    //! OnAnimate() is called just before rendering the whole scene.
    /** This method will be called once per frame, independent
        of whether the scene node is visible or not. */
    virtual void OnAnimate(u32 timeMs);

    virtual void OnRegisterSceneNode();
    virtual void render();

    virtual scene::ESCENE_NODE_TYPE getType() const { return (scene::ESCENE_NODE_TYPE)scene::ESNT_LOD_NODE; }

    const std::string& getGroupName() const { return m_group_name; }
};

#endif

