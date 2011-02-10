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
#include "graphics/per_camera_node.hpp"


PerCameraNode::PerCameraNode(scene::ISceneNode* parent, scene::ISceneManager* mgr, s32 id,
                             scene::ICameraSceneNode* camera, scene::IMesh* mesh)
    : IDummyTransformationSceneNode(parent, mgr, id)
{
#ifdef DEBUG
    if (camera)
        setName(camera->getName());
#endif
    
    m_camera = camera;
    //m_child = mgr->addMeshSceneNode(mesh, this);
    m_child = mgr->addCubeSceneNode(0.5f, this, -1);
    setAutomaticCulling(scene::EAC_OFF);
    
    parent->addChild(this);
}

PerCameraNode::~PerCameraNode()
{
}

void PerCameraNode::render()
{
    if (irr_driver->getSceneManager()->getSceneNodeRenderPass() != scene::ESNRP_SKY_BOX)
    {
        return;
    }
    
    scene::ICameraSceneNode* curr_cam = irr_driver->getSceneManager()->getActiveCamera();
    //printf("cam %s <--> %s\n", curr_cam->getName(), m_camera->getName());
    m_child->setVisible(curr_cam == m_camera);
}

void PerCameraNode::OnRegisterSceneNode()
{
    irr_driver->getSceneManager()->registerNodeForRendering(this, scene::ESNRP_SKY_BOX);
    ISceneNode::OnRegisterSceneNode();
}

void PerCameraNode::setCamera(scene::ICameraSceneNode* camera)
{
    m_camera = camera;
    
#ifdef DEBUG
    if (camera)
        setDebugName(camera->getDebugName());
#endif
}