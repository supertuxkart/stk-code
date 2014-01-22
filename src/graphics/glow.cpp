//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2013 Lauri Kasanen
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

#include "graphics/glow.hpp"

#include "graphics/callbacks.hpp"
#include "graphics/glwrap.hpp"
#include "graphics/irr_driver.hpp"
#include "graphics/material_manager.hpp"
#include "graphics/material.hpp"
#include "graphics/rtts.hpp"
#include "graphics/shaders.hpp"

using namespace video;
using namespace scene;
using namespace core;

IMesh * GlowNode::sphere = NULL;
SMaterial GlowNode::mat;
aabbox3df GlowNode::box;


GlowNode::GlowNode(scene::ISceneManager* mgr, float radius): ISceneNode(mgr->getRootSceneNode(), mgr, -1)
{
    if (!sphere)
    {
        mat.Lighting = false;
        mat.MaterialType = irr_driver->getShader(ES_GLOW);

        mat.setTexture(0, irr_driver->getRTT(RTT_QUARTER1));
        mat.TextureLayer[0].TextureWrapU =
        mat.TextureLayer[0].TextureWrapV = ETC_CLAMP_TO_EDGE;
        mat.setFlag(EMF_TRILINEAR_FILTER, true);
        mat.BlendOperation = EBO_ADD;

        sphere = mgr->getGeometryCreator()->createSphereMesh(1, 4, 4);
        box = sphere->getBoundingBox();
    }

    setScale(vector3df(radius));
}

GlowNode::~GlowNode()
{
}

void GlowNode::render()
{
}

void GlowNode::OnRegisterSceneNode()
{
    if (IsVisible)
    {
        SceneManager->registerNodeForRendering(this, ESNRP_TRANSPARENT);
    }

    ISceneNode::OnRegisterSceneNode();
}
