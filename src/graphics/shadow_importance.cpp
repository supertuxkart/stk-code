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

#include "config/user_config.hpp"
#include "graphics/glwrap.hpp"
#include "graphics/irr_driver.hpp"
#include "graphics/large_mesh_buffer.hpp"
#include "graphics/material_manager.hpp"
#include "graphics/material.hpp"
#include "graphics/per_camera_node.hpp"
#include "graphics/shadow_importance.hpp"
#include "graphics/shaders.hpp"
#include "graphics/rtts.hpp"
#include "utils/vs.hpp"

#include <ISceneManager.h>

using namespace video;
using namespace scene;
using namespace core;

// The actual ShadowImportance node
class ShadowImportanceNode: public scene::ISceneNode
{
public:
    ShadowImportanceNode(scene::ISceneManager* mgr)
            : scene::ISceneNode(0, mgr, -1)
    {
        mat.Lighting = false;
        mat.ZWriteEnable = false;
        mat.MaterialType = irr_driver->getShader(ES_SHADOW_IMPORTANCE);

        mat.setTexture(0, irr_driver->getRTT(RTT_NORMAL));
        mat.setTexture(1, irr_driver->getRTT(RTT_DEPTH));
        mat.setTexture(2, irr_driver->getRTT(RTT_COLOR));

        mat.setFlag(EMF_BILINEAR_FILTER, false);

        u32 i;
        for (i = 0; i < MATERIAL_MAX_TEXTURES; i++)
        {
            mat.TextureLayer[i].TextureWrapU =
            mat.TextureLayer[i].TextureWrapV = ETC_CLAMP_TO_EDGE;
        }

        // Low shadows only back-project every other pixel
        const u32 incr = UserConfigParams::m_shadows < 2 ? 2 : 1;

        count = (UserConfigParams::m_width * UserConfigParams::m_height) / (incr * incr);

        // Fill in the mesh buffer
        buf.Vertices.clear();
        buf.Indices.clear();

        buf.Vertices.set_used(count);
        buf.Indices.set_used(count);

        buf.Primitive = EPT_POINTS;
        buf.setHardwareMappingHint(EHM_STATIC);

        const float halfx = 0.5f / UserConfigParams::m_width;
        const float halfy = 0.5f / UserConfigParams::m_height;

        list = glGenLists(1);

        s32 x, y;
        i = 0;
        glNewList(list, GL_COMPILE);
        glBegin(GL_POINTS);
        for (x = 0; x < UserConfigParams::m_width; x += incr)
        {
            const float xpos = ((float) x) / UserConfigParams::m_width + halfx;

            for (y = 0; y < UserConfigParams::m_height; y += incr)
            {
                const float ypos = ((float) y) / UserConfigParams::m_height + halfy;

                buf.Indices[i] = i;
                buf.Vertices[i] = S3DVertex(xpos, ypos, 0, 0, 0, 0,
                                            SColor(255, 255, 255, 255), 0, 0);

                glVertex2s((int)roundf(xpos * 32767), (int)roundf(ypos * 32767));
                i++;
            }
        }
        glEnd();
        glEndList();

        box.addInternalPoint(vector3df(-1));
        box.addInternalPoint(vector3df(1));
    }

    ~ShadowImportanceNode()
    {
    }

    virtual void render()
    {
        IVideoDriver * const drv = irr_driver->getVideoDriver();
        drv->setMaterial(mat);

        drv->setTransform(ETS_WORLD, IdentityMatrix);

//        drv->drawMeshBuffer(&buf);
        // Setup the env for drawing our list by drawing one point
        drv->drawVertexPrimitiveList(buf.getVertices(), 1, buf.getIndices(), 1,
                                     EVT_STANDARD, EPT_POINTS);
        glCallList(list);
    }

    virtual const core::aabbox3d<f32>& getBoundingBox() const
    {
        return box;
    }

    virtual void OnRegisterSceneNode()
    {
        ISceneNode::OnRegisterSceneNode();
    }

    virtual u32 getMaterialCount() const { return 1; }
    virtual video::SMaterial& getMaterial(u32 i) { return mat; }

private:
    video::SMaterial mat;
    core::aabbox3d<f32> box;
    u32 count;
    GLuint list;

    scene::LargeMeshBuffer buf;
};

// The ShadowImportance manager

ShadowImportance::ShadowImportance()
{
    m_node = new ShadowImportanceNode(irr_driver->getSceneManager());
    m_node->setAutomaticCulling(0);
}   // ShadowImportance

// ----------------------------------------------------------------------------

ShadowImportance::~ShadowImportance()
{
    m_node->drop();      // drop STK's reference
    m_node->remove();    // Then remove it from the scene graph.
}

// ----------------------------------------------------------------------------

void ShadowImportance::render()
{
    m_node->render();
}
