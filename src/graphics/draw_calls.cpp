//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2015 SuperTuxKart-Team
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
#include "graphics/draw_calls.hpp"

#include "config/stk_config.hpp"
#include "config/user_config.hpp"
#include "graphics/cpu_particle_manager.hpp"
#include "graphics/irr_driver.hpp"
#include "graphics/lod_node.hpp"
#include "graphics/shaders.hpp"
#include "graphics/stk_particle.hpp"
#include "graphics/stk_text_billboard.hpp"
#include "graphics/text_billboard_drawer.hpp"
#include "graphics/sp/sp_base.hpp"
#include "graphics/sp/sp_mesh_node.hpp"
#include "tracks/track.hpp"
#include "utils/profiler.hpp"

#include <numeric>

#include <ICameraSceneNode.h>
#include <ISceneManager.h>
#include <SViewFrustum.h>

// ----------------------------------------------------------------------------
bool DrawCalls::isCulledPrecise(const scene::ICameraSceneNode *cam,
                                const scene::ISceneNode* node,
                                bool visualization)
{
    if (!node->getAutomaticCulling() && !visualization)
        return false;

    const core::matrix4 &trans = node->getAbsoluteTransformation();
    core::vector3df edges[8];
    node->getBoundingBox().getEdges(edges);
    for (unsigned i = 0; i < 8; i++)
        trans.transformVect(edges[i]);

    /* From irrlicht
       /3--------/7
      / |       / |
     /  |      /  |
    1---------5   |
    |  /2- - -|- -6
    | /       |  /
    |/        | /
    0---------4/
    */

    if (visualization)
    {
        addEdgeForViz(edges[0], edges[1]);
        addEdgeForViz(edges[1], edges[5]);
        addEdgeForViz(edges[5], edges[4]);
        addEdgeForViz(edges[4], edges[0]);
        addEdgeForViz(edges[2], edges[3]);
        addEdgeForViz(edges[3], edges[7]);
        addEdgeForViz(edges[7], edges[6]);
        addEdgeForViz(edges[6], edges[2]);
        addEdgeForViz(edges[0], edges[2]);
        addEdgeForViz(edges[1], edges[3]);
        addEdgeForViz(edges[5], edges[7]);
        addEdgeForViz(edges[4], edges[6]);
        if (!node->getAutomaticCulling())
        {
            return false;
        }
    }

    const scene::SViewFrustum &frust = *cam->getViewFrustum();
    for (s32 i = 0; i < scene::SViewFrustum::VF_PLANE_COUNT; i++)
    {
        if (isBoxInFrontOfPlane(frust.planes[i], edges))
        {
            return true;
        }
    }
    return false;

}   // isCulledPrecise

// ----------------------------------------------------------------------------
bool DrawCalls::isBoxInFrontOfPlane(const core::plane3df &plane,
                                    const core::vector3df* edges)
{
    for (u32 i = 0; i < 8; i++)
    {
        if (plane.classifyPointRelation(edges[i]) != core::ISREL3D_FRONT)
            return false;
    }
    return true;
}   // isBoxInFrontOfPlane

// ----------------------------------------------------------------------------
void DrawCalls::addEdgeForViz(const core::vector3df &p0,
                              const core::vector3df &p1)
{
    m_bounding_boxes.push_back(p0.X);
    m_bounding_boxes.push_back(p0.Y);
    m_bounding_boxes.push_back(p0.Z);
    m_bounding_boxes.push_back(p1.X);
    m_bounding_boxes.push_back(p1.Y);
    m_bounding_boxes.push_back(p1.Z);
}   // addEdgeForViz

// ----------------------------------------------------------------------------
void DrawCalls::renderBoundingBoxes()
{
    Shaders::ColoredLine *line = Shaders::ColoredLine::getInstance();
    line->use();
    line->bindVertexArray();
    line->bindBuffer();
    line->setUniforms(irr::video::SColor(255, 255, 0, 0));
    const float *tmp = m_bounding_boxes.data();
    for (unsigned int i = 0; i < m_bounding_boxes.size(); i += 1024 * 6)
    {
        unsigned count = std::min((unsigned)m_bounding_boxes.size() - i,
            (unsigned)1024 * 6);
        glBufferSubData(GL_ARRAY_BUFFER, 0, count * sizeof(float), &tmp[i]);

        glDrawArrays(GL_LINES, 0, count / 3);
    }
    m_bounding_boxes.clear();
}   // renderBoundingBoxes

// ----------------------------------------------------------------------------
void DrawCalls::parseSceneManager(core::array<scene::ISceneNode*> &List,
                                  const scene::ICameraSceneNode *cam)
{
    for (unsigned i = 0; i < List.size(); ++i)
    {
        if (LODNode *node = dynamic_cast<LODNode *>(List[i]))
        {
            node->updateVisibility();
        }
        List[i]->updateAbsolutePosition();
        if (!List[i]->isVisible())
            continue;

        if (STKParticle *node = dynamic_cast<STKParticle*>(List[i]))
        {
            if (!isCulledPrecise(cam, List[i], irr_driver->getBoundingBoxesViz()))
                CPUParticleManager::getInstance()->addParticleNode(node);
            continue;
        }

        if (scene::IBillboardSceneNode *node =
            dynamic_cast<scene::IBillboardSceneNode*>(List[i]))
        {
            if (!isCulledPrecise(cam, List[i]))
                CPUParticleManager::getInstance()->addBillboardNode(node);
            continue;
        }

        if (STKTextBillboard *tb =
            dynamic_cast<STKTextBillboard*>(List[i]))
        {
            if (!isCulledPrecise(cam, List[i], irr_driver->getBoundingBoxesViz()))
                TextBillboardDrawer::addTextBillboard(tb);
            continue;
        }

        SP::SPMeshNode* node = dynamic_cast<SP::SPMeshNode*>(List[i]);
        if (node)
        {
            SP::addObject(node);
        }
        parseSceneManager(List[i]->getChildren(), cam);
    }
}

// ----------------------------------------------------------------------------
DrawCalls::DrawCalls()
{
    m_sync = 0;
} //DrawCalls

// ----------------------------------------------------------------------------
DrawCalls::~DrawCalls()
{
    CPUParticleManager::kill();
    STKParticle::destroyFlipsBuffer();
} //~DrawCalls

// ----------------------------------------------------------------------------
 /** Prepare draw calls before scene rendering
 */
void DrawCalls::prepareDrawCalls(scene::ICameraSceneNode *camnode)
{
    CPUParticleManager::getInstance()->reset();
    TextBillboardDrawer::reset();
    PROFILER_PUSH_CPU_MARKER("- culling", 0xFF, 0xFF, 0x0);
    SP::prepareDrawCalls();
    parseSceneManager(
        irr_driver->getSceneManager()->getRootSceneNode()->getChildren(),
        camnode);
    SP::handleDynamicDrawCall();
    SP::updateModelMatrix();
    PROFILER_POP_CPU_MARKER();

    PROFILER_PUSH_CPU_MARKER("- cpu particle generation", 0x2F, 0x1F, 0x11);
    CPUParticleManager::getInstance()->generateAll();
    PROFILER_POP_CPU_MARKER();

    // Add a 1 s timeout
    if (m_sync != 0)
    {
        PROFILER_PUSH_CPU_MARKER("- Sync Stall", 0xFF, 0x0, 0x0);
        GLenum reason = glClientWaitSync(m_sync, GL_SYNC_FLUSH_COMMANDS_BIT, 0);
        if (reason != GL_ALREADY_SIGNALED)
        {
            do
            {
                reason = glClientWaitSync(m_sync, GL_SYNC_FLUSH_COMMANDS_BIT, 1000000);
            }
            while (reason == GL_TIMEOUT_EXPIRED);
        }
        glDeleteSync(m_sync);
        m_sync = 0;
        PROFILER_POP_CPU_MARKER();
    }

    PROFILER_PUSH_CPU_MARKER("- particle and text billboard upload", 0x3F,
        0x03, 0x61);
    CPUParticleManager::getInstance()->uploadAll();
    TextBillboardDrawer::updateAll();
    PROFILER_POP_CPU_MARKER();

    PROFILER_PUSH_CPU_MARKER("- SP::upload instance and skinning matrices",
        0xFF, 0x0, 0xFF);
    SP::uploadAll();
    PROFILER_POP_CPU_MARKER();
}

#endif
