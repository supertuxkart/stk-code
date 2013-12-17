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

#include "graphics/sun.hpp"

#include "graphics/callbacks.hpp"
#include "graphics/glwrap.hpp"
#include "graphics/irr_driver.hpp"
#include "graphics/material_manager.hpp"
#include "graphics/material.hpp"
#include "graphics/rtts.hpp"
#include "graphics/screenquad.hpp"
#include "graphics/shaders.hpp"
#include "io/file_manager.hpp"
#include "modes/world.hpp"
#include "tracks/track.hpp"

using namespace video;
using namespace scene;
using namespace core;

SunNode::SunNode(scene::ISceneManager* mgr, float r, float g, float b):
                     LightNode(mgr, 10000, r, g, b)
{

    sq = new ScreenQuad(irr_driver->getVideoDriver());

    SMaterial &m = sq->getMaterial();

    m.MaterialType = irr_driver->getShader(ES_SUNLIGHT);
    m.setTexture(0, irr_driver->getRTT(RTT_NORMAL));
    m.setTexture(1, irr_driver->getRTT(RTT_DEPTH));
    m.setTexture(2, irr_driver->getTexture(file_manager->getTextureFile("cloudshadow.png")));
    m.setFlag(EMF_BILINEAR_FILTER, false);
    m.MaterialTypeParam = pack_textureBlendFunc(EBF_ONE, EBF_ONE);
    m.BlendOperation = EBO_ADD;

    if (UserConfigParams::m_shadows)
    {
        m.setTexture(3, irr_driver->getRTT(RTT_SHADOW));
        m.setTexture(4, irr_driver->getRTT(RTT_WARPH));
        m.setTexture(5, irr_driver->getRTT(RTT_WARPV));

        m.TextureLayer[4].BilinearFilter =
        m.TextureLayer[5].BilinearFilter = true;

        m.MaterialType = irr_driver->getShader(ES_SUNLIGHT_SHADOW);
    }

    for (u32 i = 0; i < MATERIAL_MAX_TEXTURES; i++)
    {
        m.TextureLayer[i].TextureWrapU = m.TextureLayer[i].TextureWrapV =
            ETC_CLAMP_TO_EDGE;
    }

    m.TextureLayer[2].TextureWrapU = m.TextureLayer[2].TextureWrapV = ETC_REPEAT;

    m.TextureLayer[2].TrilinearFilter = true;

    m_color[0] = r;
    m_color[1] = g;
    m_color[2] = b;
}

SunNode::~SunNode()
{
    delete sq;
}

void SunNode::render()
{
    SunLightProvider * const cb = (SunLightProvider *) irr_driver->getCallback(ES_SUNLIGHT);
    cb->setColor(m_color[0], m_color[1], m_color[2]);

    vector3df pos = getPosition();
    pos.normalize();
    cb->setPosition(pos.X, pos.Y, pos.Z);

    if (!UserConfigParams::m_shadows || !World::getWorld()->getTrack()->hasShadows())
    {
        sq->render(false);
        return;
    }

    array<IRenderTarget> mrt;
    mrt.reallocate(2);
    mrt.push_back(irr_driver->getRTT(RTT_TMP2));
    mrt.push_back(irr_driver->getRTT(RTT_TMP3));
    irr_driver->getVideoDriver()->setRenderTarget(mrt, true, false);

    // Render the sun lighting to tmp2, shadow map to tmp3
    sq->render(false);

    // Filter the shadow map for soft shadows
    // Note that quarter1 is reserved for glow during this time.
    // Having a separate RTT for glow would work, but be wasted VRAM due to less reuse.
    ScreenQuad tmpsq(irr_driver->getVideoDriver());
    GaussianBlurProvider * const gcb = (GaussianBlurProvider *) irr_driver->getCallback(ES_GAUSSIAN3H);

    gcb->setResolution(UserConfigParams::m_width / 2, UserConfigParams::m_height / 2);
    tmpsq.setMaterialType(irr_driver->getShader(ES_PENUMBRAH));
    tmpsq.setTexture(irr_driver->getRTT(RTT_TMP3));
    tmpsq.render(irr_driver->getRTT(RTT_HALF1));
    tmpsq.setMaterialType(irr_driver->getShader(ES_PENUMBRAV));
    tmpsq.setTexture(irr_driver->getRTT(RTT_HALF1));
    tmpsq.render(irr_driver->getRTT(RTT_HALF2));

    gcb->setResolution(UserConfigParams::m_width / 4, UserConfigParams::m_height / 4);
    tmpsq.setMaterialType(irr_driver->getShader(ES_PENUMBRAH));
    tmpsq.setTexture(irr_driver->getRTT(RTT_HALF2));
    tmpsq.render(irr_driver->getRTT(RTT_QUARTER2));
    tmpsq.setMaterialType(irr_driver->getShader(ES_PENUMBRAV));
    tmpsq.setTexture(irr_driver->getRTT(RTT_QUARTER2));
    tmpsq.render(irr_driver->getRTT(RTT_QUARTER3));

    gcb->setResolution(UserConfigParams::m_width / 8, UserConfigParams::m_height / 8);
    tmpsq.setMaterialType(irr_driver->getShader(ES_PENUMBRAH));
    tmpsq.setTexture(irr_driver->getRTT(RTT_QUARTER3));
    tmpsq.render(irr_driver->getRTT(RTT_EIGHTH1));
    tmpsq.setMaterialType(irr_driver->getShader(ES_PENUMBRAV));
    tmpsq.setTexture(irr_driver->getRTT(RTT_EIGHTH1));
    tmpsq.render(irr_driver->getRTT(RTT_EIGHTH2));

    // Use these to generate a new soft shadow map
    tmpsq.setMaterialType(irr_driver->getShader(ES_SHADOWGEN));
    tmpsq.setTexture(irr_driver->getRTT(RTT_HALF2), 0);
    tmpsq.setTexture(irr_driver->getRTT(RTT_QUARTER3), 1);
    tmpsq.setTexture(irr_driver->getRTT(RTT_EIGHTH2), 2);

    irr_driver->getVideoDriver()->setRenderTarget(irr_driver->getRTT(RTT_HALF_SOFT), true, false);
    tmpsq.render(false);

    tmpsq.setTexture(0, 0);
    tmpsq.setTexture(0, 1);
    tmpsq.setTexture(0, 2);
    tmpsq.setTexture(0, 3);

    // Combine them back to the lighting RTT
    tmpsq.setMaterialType(irr_driver->getShader(ES_MULTIPLY_ADD));
    tmpsq.setTexture(irr_driver->getRTT(RTT_TMP2), 0);
    tmpsq.setTexture(irr_driver->getRTT(RTT_HALF_SOFT), 1);

    tmpsq.getMaterial().MaterialTypeParam = pack_textureBlendFunc(EBF_ONE, EBF_ONE);
    tmpsq.getMaterial().BlendOperation = EBO_ADD;

    irr_driver->getVideoDriver()->setRenderTarget(irr_driver->getRTT(RTT_TMP1), false, false);
    tmpsq.render(false);
}
