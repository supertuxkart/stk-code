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

#include "graphics/irr_driver.hpp"
#include "graphics/per_camera_node.hpp"

#include <ICameraSceneNode.h>
#include <ISceneManager.h>
#include <IMeshSceneNode.h>

PerCameraNode::PerCameraNode(scene::ISceneNode* parent, scene::ISceneManager* mgr, s32 id,
                             scene::ICameraSceneNode* camera, scene::ISceneNode *node)
    : IDummyTransformationSceneNode(parent, mgr, id)
{
#ifdef DEBUG
    if (camera)
        setName(camera->getName());
#endif

    m_camera = camera;

    node->setParent(this);
    m_child = node;

    //m_child = mgr->addCubeSceneNode(0.5f, this, -1, core::vector3df(0,0,0), core::vector3df(0,0,0), core::vector3df(3.0f,0.2f,3.0f));
    //RelativeTransformationMatrix.setTranslation( core::vector3df(-0.5,-1,3) );

    setAutomaticCulling(scene::EAC_OFF);

    parent->addChild(this);
}

PerCameraNode::~PerCameraNode()
{
}

// How to show/hide a child node is not as easy as one might think.
// setVisible(false) is effective starting from the NEXT render so we
// can't easily use it; deciding which nodes go into the render list
// from OnRegisterSceneNode doesn't work either, presumably because
// this method is called before the active camera is set or for some
// other obscure reason (?). So my solution is to add no children
// nodes from OnRegisterSceneNode, but register the PerCameraNode to
// be "rendered" in the camera phase (which is very early in the render
// pipe). then, in the render callback, I can decide whether I add
// the children nodes to the render list.

void PerCameraNode::render()
{
    scene::ICameraSceneNode* curr_cam = irr_driver->getSceneManager()->getActiveCamera();

    // Only register children nodes if the right camera is in use
    if (curr_cam == m_camera) ISceneNode::OnRegisterSceneNode();
}

void PerCameraNode::OnRegisterSceneNode()
{
    if (m_camera == NULL)
        ISceneNode::OnRegisterSceneNode();
    else
        irr_driver->getSceneManager()->registerNodeForRendering(this, scene::ESNRP_CAMERA);
}

void PerCameraNode::setCamera(scene::ICameraSceneNode* camera)
{
    m_camera = camera;

#ifdef DEBUG
    if (camera)
        setName(camera->getName());
#endif
}
