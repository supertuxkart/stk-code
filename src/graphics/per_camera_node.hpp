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

#ifndef HEADER_PER_CAMERA_HPP
#define HEADER_PER_CAMERA_HPP

#include <matrix4.h>
#include <aabbox3d.h>
#include <IDummyTransformationSceneNode.h>
#include <ESceneNodeTypes.h>
namespace irr
{
    namespace scene { class ICameraSceneNode; class ISceneNode; class ISceneManager; class IMesh; }
}
using namespace irr;


namespace irr
{
    namespace scene
    {
        const int ESNT_PER_CAMERA_NODE = MAKE_IRR_ID('p','c','a','m');
    }
}

/**
 * \brief manages smoke particle effects
 * \ingroup graphics
 */
class PerCameraNode : public scene::IDummyTransformationSceneNode
{
private:
    core::matrix4 RelativeTransformationMatrix;
    core::aabbox3d<f32> Box;

    scene::ICameraSceneNode* m_camera;
    scene::ISceneNode* m_child;

public:

    PerCameraNode(scene::ISceneNode* parent, scene::ISceneManager* mgr, s32 id,
                  scene::ICameraSceneNode* camera, scene::ISceneNode* node);
    virtual     ~PerCameraNode();

    //! returns the axis aligned bounding box of this node
    virtual const core::aabbox3d<f32>& getBoundingBox() const { return Box; }

    //! Returns a reference to the current relative transformation matrix.
    //! This is the matrix, this scene node uses instead of scale, translation
    //! and rotation.
    virtual core::matrix4& getRelativeTransformationMatrix() { return RelativeTransformationMatrix; }

    //! Returns the relative transformation of the scene node.
    virtual core::matrix4 getRelativeTransformation() const { return RelativeTransformationMatrix; }

    void setCamera(scene::ICameraSceneNode* camera);

    virtual void OnRegisterSceneNode();
    virtual void render();

    virtual scene::ESCENE_NODE_TYPE getType() const { return (scene::ESCENE_NODE_TYPE)scene::ESNT_PER_CAMERA_NODE; }

    scene::ISceneNode* getChild() { return m_child; }
};

#endif

