//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2009-2015 Joerg Henrichs
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

#include "config/user_config.hpp"
#include "graphics/callbacks.hpp"
#include "graphics/central_settings.hpp"
#include "graphics/glwrap.hpp"
#include "graphics/graphics_restrictions.hpp"
#include "graphics/lod_node.hpp"
#include "graphics/post_processing.hpp"
#include "graphics/referee.hpp"
#include "graphics/rtts.hpp"
#include "graphics/screen_quad.hpp"
#include "graphics/shaders.hpp"
#include "graphics/shadow_matrices.hpp"
#include "graphics/shared_gpu_objects.hpp"
#include "graphics/stk_mesh_scene_node.hpp"
#include "graphics/stk_scene_manager.hpp"
#include "items/item_manager.hpp"
#include "items/powerup_manager.hpp"
#include "modes/world.hpp"
#include "physics/physics.hpp"
#include "tracks/track.hpp"
#include "utils/profiler.hpp"

#define MAX2(a, b) ((a) > (b) ? (a) : (b))
#define MIN2(a, b) ((a) > (b) ? (b) : (a))



// ============================================================================
class InstancedColorizeShader : public Shader<InstancedColorizeShader>
{
public:
    InstancedColorizeShader()
    {
        loadProgram(OBJECT, GL_VERTEX_SHADER,   "utils/getworldmatrix.vert",
                            GL_VERTEX_SHADER,   "glow_object.vert",
                            GL_FRAGMENT_SHADER, "glow_object.frag");
        assignUniforms();
    }   // InstancedColorizeShader
};   // InstancedColorizeShader

// ============================================================================
void IrrDriver::renderScene(scene::ICameraSceneNode * const camnode, unsigned pointlightcount, std::vector<GlowData>& glows, float dt, bool hasShadow, bool forceRTT)
{
    glBindBufferBase(GL_UNIFORM_BUFFER, 0, SharedGPUObjects::getViewProjectionMatricesUBO());
    glBindBufferBase(GL_UNIFORM_BUFFER, 1, SharedGPUObjects::getLightingDataUBO());
    m_scene_manager->setActiveCamera(camnode);

    PROFILER_PUSH_CPU_MARKER("- Draw Call Generation", 0xFF, 0xFF, 0xFF);
    PrepareDrawCalls(camnode);
    PROFILER_POP_CPU_MARKER();
    // Shadows
    {
        // To avoid wrong culling, use the largest view possible
        m_scene_manager->setActiveCamera(getShadowMatrices()->getSunCam());
        if (CVS->isDefferedEnabled() &&
            CVS->isShadowEnabled() && hasShadow)
        {
            PROFILER_PUSH_CPU_MARKER("- Shadow", 0x30, 0x6F, 0x90);
            renderShadows();
            PROFILER_POP_CPU_MARKER();
            if (CVS->isGlobalIlluminationEnabled())
            {
                PROFILER_PUSH_CPU_MARKER("- RSM", 0xFF, 0x0, 0xFF);
                renderRSM();
                PROFILER_POP_CPU_MARKER();
            }
        }
        m_scene_manager->setActiveCamera(camnode);

    }

    PROFILER_PUSH_CPU_MARKER("- Solid Pass 1", 0xFF, 0x00, 0x00);
    glDepthMask(GL_TRUE);
    glDepthFunc(GL_LEQUAL);
    glEnable(GL_DEPTH_TEST);
    glDisable(GL_BLEND);
    glEnable(GL_CULL_FACE);
    if (CVS->isDefferedEnabled() || forceRTT)
    {
        m_rtts->getFBO(FBO_NORMAL_AND_DEPTHS).bind();
        glClearColor(0., 0., 0., 0.);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
        renderSolidFirstPass();
    }
    else
    {
        // We need a cleared depth buffer for some effect (eg particles depth blending)
        if (GraphicsRestrictions::isDisabled(GraphicsRestrictions::GR_FRAMEBUFFER_SRGB_WORKING))
            glDisable(GL_FRAMEBUFFER_SRGB);
        m_rtts->getFBO(FBO_NORMAL_AND_DEPTHS).bind();
        // Bind() modifies the viewport. In order not to affect anything else,
        // the viewport is just reset here and not removed in Bind().
        const core::recti &vp = Camera::getActiveCamera()->getViewport();
        glViewport(vp.UpperLeftCorner.X,
                   irr_driver->getActualScreenSize().Height - vp.LowerRightCorner.Y,
                   vp.LowerRightCorner.X - vp.UpperLeftCorner.X,
                   vp.LowerRightCorner.Y - vp.UpperLeftCorner.Y);
        glClear(GL_DEPTH_BUFFER_BIT);
        if (GraphicsRestrictions::isDisabled(GraphicsRestrictions::GR_FRAMEBUFFER_SRGB_WORKING))
            glEnable(GL_FRAMEBUFFER_SRGB);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }
    PROFILER_POP_CPU_MARKER();



    // Lights
    {
        PROFILER_PUSH_CPU_MARKER("- Light", 0x00, 0xFF, 0x00);
        if (CVS->isDefferedEnabled())
            renderLights(pointlightcount, hasShadow);
        PROFILER_POP_CPU_MARKER();
    }

    // Handle SSAO
    {
        PROFILER_PUSH_CPU_MARKER("- SSAO", 0xFF, 0xFF, 0x00);
        ScopedGPUTimer Timer(getGPUTimer(Q_SSAO));
        if (UserConfigParams::m_ssao)
            renderSSAO();
        PROFILER_POP_CPU_MARKER();
    }

    PROFILER_PUSH_CPU_MARKER("- Solid Pass 2", 0x00, 0x00, 0xFF);
    if (CVS->isDefferedEnabled() || forceRTT)
    {
        m_rtts->getFBO(FBO_COLORS).bind();
        SColor clearColor(0, 150, 150, 150);
        if (World::getWorld() != NULL)
            clearColor = World::getWorld()->getClearColor();

        glClearColor(clearColor.getRed() / 255.f, clearColor.getGreen() / 255.f,
            clearColor.getBlue() / 255.f, clearColor.getAlpha() / 255.f);
        glClear(GL_COLOR_BUFFER_BIT);
        glDepthMask(GL_FALSE);
    }
    renderSolidSecondPass();
    PROFILER_POP_CPU_MARKER();

    if (getNormals())
    {
        m_rtts->getFBO(FBO_NORMAL_AND_DEPTHS).bind();
        renderNormalsVisualisation();
        m_rtts->getFBO(FBO_COLORS).bind();
    }

    // Render ambient scattering
    if (CVS->isDefferedEnabled() && World::getWorld() != NULL &&
        World::getWorld()->isFogEnabled())
    {
        PROFILER_PUSH_CPU_MARKER("- Ambient scatter", 0xFF, 0x00, 0x00);
        ScopedGPUTimer Timer(getGPUTimer(Q_FOG));
        renderAmbientScatter();
        PROFILER_POP_CPU_MARKER();
    }

    {
        PROFILER_PUSH_CPU_MARKER("- Skybox", 0xFF, 0x00, 0xFF);
        ScopedGPUTimer Timer(getGPUTimer(Q_SKYBOX));
        renderSkybox(camnode);
        PROFILER_POP_CPU_MARKER();
    }

    // Render discrete lights scattering
    if (CVS->isDefferedEnabled() && World::getWorld() != NULL &&
        World::getWorld()->isFogEnabled())
    {
        PROFILER_PUSH_CPU_MARKER("- PointLight Scatter", 0xFF, 0x00, 0x00);
        ScopedGPUTimer Timer(getGPUTimer(Q_FOG));
        renderLightsScatter(pointlightcount);
        PROFILER_POP_CPU_MARKER();
    }

    if (getRH())
    {
        glDisable(GL_BLEND);
        m_rtts->getFBO(FBO_COLORS).bind();
        m_post_processing->renderRHDebug(m_rtts->getRH().getRTT()[0],
                                         m_rtts->getRH().getRTT()[1], 
                                         m_rtts->getRH().getRTT()[2],
                                         getShadowMatrices()->getRHMatrix(),
                                         getShadowMatrices()->getRHExtend());
    }

    if (getGI())
    {
        glDisable(GL_BLEND);
        m_rtts->getFBO(FBO_COLORS).bind();
        m_post_processing->renderGI(getShadowMatrices()->getRHMatrix(),
                                    getShadowMatrices()->getRHExtend(),
                                    m_rtts->getRH());
    }

    PROFILER_PUSH_CPU_MARKER("- Glow", 0xFF, 0xFF, 0x00);
    // Render anything glowing.
    if (!m_mipviz && !m_wireframe && UserConfigParams::m_glow)
    {
        ScopedGPUTimer Timer(getGPUTimer(Q_GLOW));
        irr_driver->setPhase(GLOW_PASS);
        renderGlow(glows);
    } // end glow
    PROFILER_POP_CPU_MARKER();

    // Render transparent
    {
        PROFILER_PUSH_CPU_MARKER("- Transparent Pass", 0xFF, 0x00, 0x00);
        ScopedGPUTimer Timer(getGPUTimer(Q_TRANSPARENT));
        renderTransparent();
        PROFILER_POP_CPU_MARKER();
    }

    m_sync = glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE, 0);

    // Render particles
    {
        PROFILER_PUSH_CPU_MARKER("- Particles", 0xFF, 0xFF, 0x00);
        ScopedGPUTimer Timer(getGPUTimer(Q_PARTICLES));
        renderParticles();
        PROFILER_POP_CPU_MARKER();
    }
    if (!CVS->isDefferedEnabled() && !forceRTT)
    {
        glDisable(GL_FRAMEBUFFER_SRGB);
        glDisable(GL_DEPTH_TEST);
        glDepthMask(GL_FALSE);
        return;
    }

    // Ensure that no object will be drawn after that by using invalid pass
    irr_driver->setPhase(PASS_COUNT);
}

// ----------------------------------------------------------------------------
void IrrDriver::renderSkybox(const scene::ICameraSceneNode *camera)
{
    if(m_skybox)
    {
        m_skybox->render(camera);
    }
}   // renderSkybox

// ----------------------------------------------------------------------------

void IrrDriver::renderParticles()
{
    glDepthMask(GL_FALSE);
    glDisable(GL_CULL_FACE);
    glEnable(GL_BLEND);
    glBlendEquation(GL_FUNC_ADD);
    for (unsigned i = 0; i < ParticlesList::getInstance()->size(); ++i)
        ParticlesList::getInstance()->at(i)->render();
//    m_scene_manager->drawAll(scene::ESNRP_TRANSPARENT_EFFECT);
}

// ----------------------------------------------------------------------------

void IrrDriver::renderGlow(std::vector<GlowData>& glows)
{
    m_scene_manager->setCurrentRendertime(scene::ESNRP_SOLID);
    m_rtts->getFBO(FBO_TMP1_WITH_DS).bind();
    glClearStencil(0);
    glClearColor(0, 0, 0, 0);
    glClear(GL_STENCIL_BUFFER_BIT | GL_COLOR_BUFFER_BIT);

    const u32 glowcount = (int)glows.size();

    glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
    glStencilFunc(GL_ALWAYS, 1, ~0);
    glEnable(GL_STENCIL_TEST);

    glEnable(GL_DEPTH_TEST);
    glDepthMask(GL_FALSE);
    glDisable(GL_BLEND);

    if (CVS->isARBBaseInstanceUsable())
        glBindVertexArray(VAOManager::getInstance()->getVAO(EVT_STANDARD));
    for (u32 i = 0; i < glowcount; i++)
    {
        const GlowData &dat = glows[i];
        scene::ISceneNode * cur = dat.node;

        STKMeshSceneNode *node = static_cast<STKMeshSceneNode *>(cur);
        node->setGlowColors(SColor(0, (unsigned) (dat.b * 255.f), (unsigned)(dat.g * 255.f), (unsigned)(dat.r * 255.f)));
        if (!CVS->supportsIndirectInstancingRendering())
            node->render();
    }

    if (CVS->supportsIndirectInstancingRendering())
    {
        glBindBuffer(GL_DRAW_INDIRECT_BUFFER, GlowPassCmd::getInstance()->drawindirectcmd);
        InstancedColorizeShader::getInstance()->use();

        glBindVertexArray(VAOManager::getInstance()->getInstanceVAO(video::EVT_STANDARD, InstanceTypeGlow));
        if (CVS->isAZDOEnabled())
        {
            if (GlowPassCmd::getInstance()->Size)
            {
                glMultiDrawElementsIndirect(GL_TRIANGLES, GL_UNSIGNED_SHORT,
                    (const void*)(GlowPassCmd::getInstance()->Offset * sizeof(DrawElementsIndirectCommand)),
                    (int)GlowPassCmd::getInstance()->Size,
                    sizeof(DrawElementsIndirectCommand));
            }
        }
        else
        {
            for (unsigned i = 0; i < ListInstancedGlow::getInstance()->size(); i++)
                glDrawElementsIndirect(GL_TRIANGLES, GL_UNSIGNED_SHORT, (const void*)((GlowPassCmd::getInstance()->Offset + i) * sizeof(DrawElementsIndirectCommand)));
        }
    }

    glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);
    glDisable(GL_STENCIL_TEST);
    glDisable(GL_BLEND);

    // To half
    FrameBuffer::Blit(irr_driver->getFBO(FBO_TMP1_WITH_DS), irr_driver->getFBO(FBO_HALF1), GL_COLOR_BUFFER_BIT, GL_LINEAR);

    // To quarter
    FrameBuffer::Blit(irr_driver->getFBO(FBO_HALF1), irr_driver->getFBO(FBO_QUARTER1), GL_COLOR_BUFFER_BIT, GL_LINEAR);


    glEnable(GL_BLEND);
    glBlendEquation(GL_FUNC_ADD);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glStencilFunc(GL_EQUAL, 0, ~0);
    glEnable(GL_STENCIL_TEST);
    m_rtts->getFBO(FBO_COLORS).bind();
    m_post_processing->renderGlow(m_rtts->getRenderTarget(RTT_QUARTER1));
    glDisable(GL_STENCIL_TEST);
}
