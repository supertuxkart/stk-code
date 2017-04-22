//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2013-2015 Lauri Kasanen
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

#ifndef SERVER_ONLY

#include "graphics/water.hpp"

#include "graphics/callbacks.hpp"
#include "graphics/irr_driver.hpp"
#include "graphics/shaders.hpp"

using namespace video;
using namespace scene;
using namespace core;

WaterNode::WaterNode(scene::ISceneManager* mgr, IMesh *mesh, float height, float speed,
                     float length):
                     IMeshSceneNode(mgr->getRootSceneNode(), mgr, -1)
{
    m_mat = mesh->getMeshBuffer(0)->getMaterial();

    m_mat.Lighting = false;

    if (m_mat.MaterialType != Shaders::getShader(ES_WATER))
    {
        m_mat.MaterialType = Shaders::getShader(ES_WATER_SURFACE);
    } else
    {
        m_mat.BlendOperation = EBO_ADD;
    }

    m_mat.setFlag(EMF_ZWRITE_ENABLE, false);

    m_mesh = mesh;
    mesh->grab();
    mesh->setHardwareMappingHint(EHM_STATIC);
    m_box = mesh->getBoundingBox();

    m_height = height;
    m_speed = speed;
    m_length = length;
}

WaterNode::~WaterNode()
{
    m_mesh->drop();
}

void WaterNode::render()
{
    if (SceneManager->getSceneNodeRenderPass() != scene::ESNRP_TRANSPARENT)
        return;

    WaterShaderProvider * const cb = 
        (WaterShaderProvider *) Shaders::getCallback(ES_WATER);
    cb->setSpeed(m_speed);
    cb->setHeight(m_height);
    cb->setLength(m_length);

    IVideoDriver * const drv = irr_driver->getVideoDriver();
    drv->setTransform(ETS_WORLD, AbsoluteTransformation);
    drv->setMaterial(m_mat);

    const u32 max = m_mesh->getMeshBufferCount();
    for (u32 i = 0; i < max; i++)
    {
        drv->drawMeshBuffer(m_mesh->getMeshBuffer(i));
    }
}

void WaterNode::OnRegisterSceneNode()
{
    if (IsVisible)
    {
        SceneManager->registerNodeForRendering(this, scene::ESNRP_TRANSPARENT);
        ISceneNode::OnRegisterSceneNode();
    }
}

#endif   // !SERVER_ONLY

