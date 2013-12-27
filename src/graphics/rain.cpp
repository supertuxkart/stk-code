//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2011-2013  Joerg Henrichs, Marianne Gagnon
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

#include "audio/sfx_base.hpp"
#include "audio/sfx_manager.hpp"
#include "graphics/glwrap.hpp"
#include "graphics/irr_driver.hpp"
#include "graphics/material_manager.hpp"
#include "graphics/material.hpp"
#include "graphics/per_camera_node.hpp"
#include "graphics/rain.hpp"
#include "graphics/shaders.hpp"
#include "modes/world.hpp"
#include "states_screens/race_gui.hpp"
#include "utils/constants.hpp"
#include "utils/random_generator.hpp"

#include <ISceneManager.h>
#include <SMeshBuffer.h>
#include "graphics/gpuparticles.h"

using namespace video;
using namespace scene;
using namespace core;

// The actual rain node
class RainNode: public scene::ISceneNode
{
private:
//    GPUParticle *gpupart;
public:
    RainNode(scene::ISceneManager* mgr, ITexture *tex)
            : scene::ISceneNode(0, mgr, -1)
    {
        mat.Lighting = false;
        mat.ZWriteEnable = false;
        mat.MaterialType = irr_driver->getShader(ES_RAIN);
        mat.Thickness = 200;
        mat.BlendOperation = EBO_ADD;

        mat.setTexture(0, tex);
        mat.TextureLayer[0].TextureWrapU =
        mat.TextureLayer[0].TextureWrapV = ETC_CLAMP_TO_EDGE;

        count = 2500;
        area = 3500;

        u32 i;
        float x, y, z, vertices[7500];
        for (i = 0; i < count; i++)
    {
            x = ((rand() % area) - area/2) / 100.0f;
            y = ((rand() % 2400)) / 100.0f;
            z = ((rand() % area) - area/2) / 100.0f;

            vertices[3 * i] = x;
            vertices[3 * i + 1] = y;
            vertices[3 * i + 2] = z;
        }
        //gpupart = new GPUParticle(count, vertices, getTextureGLuint(mat.getTexture(0)), getTextureGLuint(irr_driver->getRTT(RTT_NORMAL_AND_DEPTH)));

        box.addInternalPoint(vector3df((float)(-area/2)));
        box.addInternalPoint(vector3df((float)( area/2)));
    }

    ~RainNode()
    {
//        delete gpupart;
    }

    virtual void render()
    {
//        gpupart->simulate();
//        gpupart->render();
        // We need to force irrlicht to update its internal states
        IVideoDriver * const drv = irr_driver->getVideoDriver();
        drv->setMaterial(mat);
        static_cast<COpenGLDriver*>(drv)->setRenderStates3DMode();
    }

    virtual const core::aabbox3d<f32>& getBoundingBox() const
    {
        return box;
    }

    virtual void OnRegisterSceneNode()
            {
        if (IsVisible &&
           (irr_driver->getRenderPass() & ESNRP_TRANSPARENT) == ESNRP_TRANSPARENT)
        {
            SceneManager->registerNodeForRendering(this, ESNRP_TRANSPARENT);
            }

        ISceneNode::OnRegisterSceneNode();
        }

    virtual u32 getMaterialCount() const { return 1; }
    virtual video::SMaterial& getMaterial(u32 i) { return mat; }

private:
    video::SMaterial mat;
    core::aabbox3d<f32> box;
    u32 count;
    s32 area;
};

// The rain manager

Rain::Rain(Camera *camera, irr::scene::ISceneNode* parent)
{
    m_lightning = camera->getIndex()==0;

    if (m_lightning)
        m_thunder_sound = sfx_manager->createSoundSource("thunder");

    Material* m = material_manager->getMaterial("rain.png");
    assert(m != NULL);

    RandomGenerator g;
    m_next_lightning = (float)g.get(35);

    RainNode *node = new RainNode(irr_driver->getSceneManager(), m->getTexture());
    m_node = irr_driver->addPerCameraNode(node, camera->getCameraSceneNode(), parent);
    m_node->setAutomaticCulling(0);
}   // Rain

// ----------------------------------------------------------------------------

Rain::~Rain()
{
    m_node->drop();      // drop STK's reference
    m_node->remove();    // Then remove it from the scene graph.

    if (m_lightning && m_thunder_sound != NULL) sfx_manager->deleteSFX(m_thunder_sound);
}

// ----------------------------------------------------------------------------

void Rain::update(float dt)
{
    if (m_lightning)
    {
        m_next_lightning -= dt;

        if (m_next_lightning < 0.0f)
        {
            RaceGUIBase* gui_base = World::getWorld()->getRaceGUI();
            if (gui_base != NULL)
            {
                gui_base->doLightning();
                if (m_thunder_sound) m_thunder_sound->play();
            }

            RandomGenerator g;
            m_next_lightning = 35 + (float)g.get(35);
        }
    }

}   // update

// ----------------------------------------------------------------------------

void Rain::setPosition(const core::vector3df& position)
{
    m_node->getChild()->setPosition(position);
}   // setPosition

// ----------------------------------------------------------------------------

void Rain::setCamera(scene::ICameraSceneNode* camera)
{
    m_node->setCamera(camera);
}
