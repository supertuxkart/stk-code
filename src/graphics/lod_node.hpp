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

#ifndef HEADER_LOD_NODE_HPP
#define HEADER_LOD_NODE_HPP

#include "irrlicht.h"
using namespace irr;

namespace irr
{
    namespace scene
    {
        const int ESNT_LOD_NODE = MAKE_IRR_ID('l','o','d','n');
    }
}

/**
 * \brief manages smoke particle effects
 * \ingroup graphics
 */
class LODNode : public scene::IDummyTransformationSceneNode
{
private:
    core::matrix4 RelativeTransformationMatrix;
    core::aabbox3d<f32> Box;

    std::vector<int> m_detail;
    std::vector<scene::ISceneNode*> m_nodes;
        
public:
    
    LODNode(scene::ISceneNode* parent, scene::ISceneManager* mgr, s32 id);
    virtual     ~LODNode();
    
    //! returns the axis aligned bounding box of this node
    virtual const core::aabbox3d<f32>& getBoundingBox() const { return Box; }
    
    //! Returns a reference to the current relative transformation matrix.
    //! This is the matrix, this scene node uses instead of scale, translation
    //! and rotation.
    virtual core::matrix4& getRelativeTransformationMatrix() { return RelativeTransformationMatrix; }
    
    //! Returns the relative transformation of the scene node.
    virtual core::matrix4 getRelativeTransformation() const { return RelativeTransformationMatrix; }
    
    void add(int level, scene::ISceneNode* node, bool reparent);
    
    scene::ISceneNode* getFirstNode() { return m_nodes[0]; }
    
    virtual void OnRegisterSceneNode();
    virtual void render();
    
    virtual scene::ESCENE_NODE_TYPE getType() const { return (scene::ESCENE_NODE_TYPE)scene::ESNT_LOD_NODE; }

};

#endif

