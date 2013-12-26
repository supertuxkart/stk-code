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

#include "graphics/light.hpp"

#include "graphics/callbacks.hpp"
#include "graphics/glwrap.hpp"
#include "graphics/irr_driver.hpp"
#include "graphics/material_manager.hpp"
#include "graphics/material.hpp"
#include "graphics/rtts.hpp"
#include "graphics/shaders.hpp"
#include "graphics/screenquad.hpp"

using namespace video;
using namespace scene;
using namespace core;

IMesh * LightNode::sphere = NULL;
SMaterial LightNode::mat;
aabbox3df LightNode::box;


LightNode::LightNode(scene::ISceneManager* mgr, float radius, float e, float r, float g, float b):
                     ISceneNode(mgr->getRootSceneNode(), mgr, -1)
{
    sq = new ScreenQuad(irr_driver->getVideoDriver());
    SMaterial &mat = sq->getMaterial();

    mat.Lighting = false;
    mat.MaterialType = irr_driver->getShader(ES_POINTLIGHT);

    mat.setTexture(0, irr_driver->getRTT(RTT_NORMAL_AND_DEPTH));

    for (u32 i = 0; i < MATERIAL_MAX_TEXTURES; ++i)
    {
        mat.TextureLayer[i].TextureWrapU =
        mat.TextureLayer[i].TextureWrapV = ETC_CLAMP_TO_EDGE;
    }

    mat.setFlag(EMF_BILINEAR_FILTER, false);
    mat.setFlag(EMF_ZWRITE_ENABLE, false);

    mat.MaterialTypeParam = pack_textureBlendFunc(EBF_ONE, EBF_ONE);
    mat.BlendOperation = EBO_ADD;

    sphere = mgr->getGeometryCreator()->createSphereMesh(1, 16, 16);
    box = sphere->getBoundingBox();

    setScale(vector3df(radius));
    m_radius = radius;
    energy = e;

    m_color[0] = r;
    m_color[1] = g;
    m_color[2] = b;
}

LightNode::~LightNode()
{
}

void LightNode::render()
{
    PointLightProvider * const cb = (PointLightProvider *) irr_driver->getCallback(ES_POINTLIGHT);
    cb->setColor(m_color[0], m_color[1], m_color[2]);
    cb->setPosition(getPosition().X, getPosition().Y, getPosition().Z);
    cb->setRadius(m_radius);
    cb->setEnergy(energy);
    // Irrlicht's ScreenQuad reset the matrixes, we need to keep them
    IVideoDriver * const drv = irr_driver->getVideoDriver();
    matrix4 tmpworld = drv->getTransform(ETS_WORLD);
    matrix4 tmpview = drv->getTransform(ETS_VIEW);
    matrix4 tmpproj = drv->getTransform(ETS_PROJECTION);
    sq->render(false);
    drv->setTransform(ETS_WORLD, tmpworld);
    drv->setTransform(ETS_VIEW, tmpview);
    drv->setTransform(ETS_PROJECTION, tmpproj);
    return;
}

void LightNode::OnRegisterSceneNode()
{ // This node is only drawn manually.
}
