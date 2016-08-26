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
        loadProgram(OBJECT, GL_VERTEX_SHADER,   "glow_object.vert",
                            GL_FRAGMENT_SHADER, "glow_object.frag");
        assignUniforms();
    }   // InstancedColorizeShader
};   // InstancedColorizeShader

// ============================================================================

extern std::vector<float> BoundingBoxes;

void IrrDriver::renderGLSL(float dt)
{
    BoundingBoxes.clear();
    World *world = World::getWorld(); // Never NULL.

    Track *track = world->getTrack();

    for (unsigned i = 0; i < PowerupManager::POWERUP_MAX; i++)
    {
        scene::IMesh *mesh = powerup_manager->m_all_meshes[i];
        if (!mesh)
            continue;
        for (unsigned j = 0; j < mesh->getMeshBufferCount(); j++)
        {
            scene::IMeshBuffer *mb = mesh->getMeshBuffer(j);
            if (!mb)
                continue;
            for (unsigned k = 0; k < 4; k++)
            {
                video::ITexture *tex = mb->getMaterial().getTexture(k);
                if (!tex)
                    continue;
                compressTexture(tex, true);
            }
        }
    }


    // Overrides
    video::SOverrideMaterial &overridemat = m_video_driver->getOverrideMaterial();
    overridemat.EnablePasses = scene::ESNRP_SOLID | scene::ESNRP_TRANSPARENT;
    overridemat.EnableFlags = 0;

    if (m_wireframe)
    {
        overridemat.Material.Wireframe = 1;
        overridemat.EnableFlags |= video::EMF_WIREFRAME;
    }
    if (m_mipviz)
    {
        overridemat.Material.MaterialType = Shaders::getShader(ES_MIPVIZ);
        overridemat.EnableFlags |= video::EMF_MATERIAL_TYPE;
        overridemat.EnablePasses = scene::ESNRP_SOLID;
    }

    // Get a list of all glowing things. The driver's list contains the static ones,
    // here we add items, as they may disappear each frame.
    std::vector<GlowData> glows = m_glowing;

    ItemManager * const items = ItemManager::get();
    const u32 itemcount = items->getNumberOfItems();
    u32 i;

    for (i = 0; i < itemcount; i++)
    {
        Item * const item = items->getItem(i);
        if (!item) continue;
        const Item::ItemType type = item->getType();

        if (type != Item::ITEM_NITRO_BIG && type != Item::ITEM_NITRO_SMALL &&
            type != Item::ITEM_BONUS_BOX && type != Item::ITEM_BANANA && type != Item::ITEM_BUBBLEGUM)
            continue;

        LODNode * const lod = (LODNode *) item->getSceneNode();
        if (!lod->isVisible()) continue;

        const int level = lod->getLevel();
        if (level < 0) continue;

        scene::ISceneNode * const node = lod->getAllNodes()[level];
        node->updateAbsolutePosition();

        GlowData dat;
        dat.node = node;

        dat.r = 1.0f;
        dat.g = 1.0f;
        dat.b = 1.0f;

        const video::SColorf &c = ItemManager::getGlowColor(type);
        dat.r = c.getRed();
        dat.g = c.getGreen();
        dat.b = c.getBlue();

        glows.push_back(dat);
    }

    // Start the RTT for post-processing.
    // We do this before beginScene() because we want to capture the glClear()
    // because of tracks that do not have skyboxes (generally add-on tracks)
    m_post_processing->begin();

    RaceGUIBase *rg = world->getRaceGUI();
    if (rg) rg->update(dt);

    if (!CVS->isDefferedEnabled())
    {
        SColor clearColor(0, 150, 150, 150);
        if (World::getWorld() != NULL)
            clearColor = World::getWorld()->getClearColor();

        glClear(GL_COLOR_BUFFER_BIT);
        glDepthMask(GL_TRUE);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glClearColor(clearColor.getRed() / 255.f, clearColor.getGreen() / 255.f,
            clearColor.getBlue() / 255.f, clearColor.getAlpha() / 255.f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
    }

    for(unsigned int cam = 0; cam < Camera::getNumCameras(); cam++)
    {
        Camera * const camera = Camera::getCamera(cam);
        scene::ICameraSceneNode * const camnode = camera->getCameraSceneNode();

        std::ostringstream oss;
        oss << "drawAll() for kart " << cam;
        PROFILER_PUSH_CPU_MARKER(oss.str().c_str(), (cam+1)*60,
                                 0x00, 0x00);
        camera->activate(!CVS->isDefferedEnabled());
        rg->preRenderCallback(camera);   // adjusts start referee
        m_scene_manager->setActiveCamera(camnode);

        const core::recti &viewport = camera->getViewport();

        if (World::getWorld() && World::getWorld()->getTrack()->hasShadows() && m_spherical_harmonics->has6Textures())
            irr_driver->getSceneManager()->setAmbientLight(SColor(0, 0, 0, 0));

        if (!CVS->isDefferedEnabled())
            glEnable(GL_FRAMEBUFFER_SRGB);

        PROFILER_PUSH_CPU_MARKER("Update Light Info", 0xFF, 0x0, 0x0);
        unsigned plc = updateLightsInfo(camnode, dt);
        PROFILER_POP_CPU_MARKER();
        PROFILER_PUSH_CPU_MARKER("UBO upload", 0x0, 0xFF, 0x0);
        computeMatrixesAndCameras(camnode, viewport.LowerRightCorner.X - viewport.UpperLeftCorner.X, viewport.LowerRightCorner.Y - viewport.UpperLeftCorner.Y);
        uploadLightingData();
        PROFILER_POP_CPU_MARKER();
        renderScene(camnode, plc, glows, dt, track->hasShadows(), false);

        // Render bounding boxes
        if (irr_driver->getBoundingBoxesViz())
        {
            Shaders::ColoredLine *line = Shaders::ColoredLine::getInstance();
            line->use();
            line->bindVertexArray();
            line->bindBuffer();
            line->setUniforms(SColor(255, 255, 0, 0));
            const float *tmp = BoundingBoxes.data();
            for (unsigned int i = 0; i < BoundingBoxes.size(); i += 1024 * 6)
            {
                unsigned count = MIN2((int)BoundingBoxes.size() - i, 1024 * 6);
                glBufferSubData(GL_ARRAY_BUFFER, 0, count * sizeof(float), &tmp[i]);

                glDrawArrays(GL_LINES, 0, count / 3);
            }
        }

        // Debug physic
        // Note that drawAll must be called before rendering
        // the bullet debug view, since otherwise the camera
        // is not set up properly. This is only used for
        // the bullet debug view.
        if (UserConfigParams::m_artist_debug_mode)
            World::getWorld()->getPhysics()->draw();
        if (world != NULL && world->getPhysics() != NULL)
        {
            IrrDebugDrawer* debug_drawer = world->getPhysics()->getDebugDrawer();
            if (debug_drawer != NULL && debug_drawer->debugEnabled())
            {
                const std::map<video::SColor, std::vector<float> >& lines = debug_drawer->getLines();
                std::map<video::SColor, std::vector<float> >::const_iterator it;

                Shaders::ColoredLine *line = Shaders::ColoredLine::getInstance();
                line->use();
                line->bindVertexArray();
                line->bindBuffer();
                for (it = lines.begin(); it != lines.end(); it++)
                {
                    line->setUniforms(it->first);
                    const std::vector<float> &vertex = it->second;
                    const float *tmp = vertex.data();
                    for (unsigned int i = 0; i < vertex.size(); i += 1024 * 6)
                    {
                        unsigned count = MIN2((int)vertex.size() - i, 1024 * 6);
                        glBufferSubData(GL_ARRAY_BUFFER, 0, count * sizeof(float), &tmp[i]);

                        glDrawArrays(GL_LINES, 0, count / 3);
                    }
                }
                glUseProgram(0);
                glBindVertexArray(0);
            }
        }

        // Render the post-processed scene
        if (CVS->isDefferedEnabled())
        {
            bool isRace = StateManager::get()->getGameState() == GUIEngine::GAME;
            FrameBuffer *fbo = m_post_processing->render(camnode, isRace);

            if (irr_driver->getNormals())
                irr_driver->getFBO(FBO_NORMAL_AND_DEPTHS).BlitToDefault(viewport.UpperLeftCorner.X, viewport.UpperLeftCorner.Y, viewport.LowerRightCorner.X, viewport.LowerRightCorner.Y);
            else if (irr_driver->getSSAOViz())
            {
                glBindFramebuffer(GL_FRAMEBUFFER, 0);
                glViewport(viewport.UpperLeftCorner.X, viewport.UpperLeftCorner.Y, viewport.LowerRightCorner.X, viewport.LowerRightCorner.Y);
                m_post_processing->renderPassThrough(m_rtts->getFBO(FBO_HALF1_R).getRTT()[0], viewport.LowerRightCorner.X - viewport.UpperLeftCorner.X, viewport.LowerRightCorner.Y - viewport.UpperLeftCorner.Y);
            }
            else if (irr_driver->getRSM())
            {
                glBindFramebuffer(GL_FRAMEBUFFER, 0);
                glViewport(viewport.UpperLeftCorner.X, viewport.UpperLeftCorner.Y, viewport.LowerRightCorner.X, viewport.LowerRightCorner.Y);
                m_post_processing->renderPassThrough(m_rtts->getRSM().getRTT()[0], viewport.LowerRightCorner.X - viewport.UpperLeftCorner.X, viewport.LowerRightCorner.Y - viewport.UpperLeftCorner.Y);
            }
            else if (irr_driver->getShadowViz())
            {
                getShadowMatrices()->renderShadowsDebug();
            }
            else
            {
                glEnable(GL_FRAMEBUFFER_SRGB);
                glBindFramebuffer(GL_FRAMEBUFFER, 0);
                if (CVS->isDefferedEnabled())
                    camera->activate();
                glViewport(viewport.UpperLeftCorner.X, viewport.UpperLeftCorner.Y, viewport.LowerRightCorner.X, viewport.LowerRightCorner.Y);
                m_post_processing->renderPassThrough(fbo->getRTT()[0], viewport.LowerRightCorner.X - viewport.UpperLeftCorner.X, viewport.LowerRightCorner.Y - viewport.UpperLeftCorner.Y);
                glDisable(GL_FRAMEBUFFER_SRGB);
            }
        }
        // Save projection-view matrix for the next frame
        camera->setPreviousPVMatrix(m_ProjViewMatrix);

        PROFILER_POP_CPU_MARKER();
    }   // for i<world->getNumKarts()

    // Use full screen size
    float tmp[2];
    tmp[0] = float(m_actual_screen_size.Width);
    tmp[1] = float(m_actual_screen_size.Height);
    glBindBuffer(GL_UNIFORM_BUFFER,
                 SharedGPUObjects::getViewProjectionMatricesUBO());
    glBufferSubData(GL_UNIFORM_BUFFER, (16 * 9) * sizeof(float),
                    2 * sizeof(float), tmp);

    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    glUseProgram(0);

    // Set the viewport back to the full screen for race gui
    m_video_driver->setViewPort(core::recti(0, 0,
        irr_driver->getActualScreenSize().Width,
        irr_driver->getActualScreenSize().Height));

    for(unsigned int i=0; i<Camera::getNumCameras(); i++)
    {
        Camera *camera = Camera::getCamera(i);
        std::ostringstream oss;
        oss << "renderPlayerView() for kart " << i;

        PROFILER_PUSH_CPU_MARKER(oss.str().c_str(), 0x00, 0x00, (i+1)*60);
        rg->renderPlayerView(camera, dt);

        PROFILER_POP_CPU_MARKER();
    }  // for i<getNumKarts

    {
        ScopedGPUTimer Timer(getGPUTimer(Q_GUI));
        PROFILER_PUSH_CPU_MARKER("GUIEngine", 0x75, 0x75, 0x75);
        // Either render the gui, or the global elements of the race gui.
        GUIEngine::render(dt);
        PROFILER_POP_CPU_MARKER();
    }

    // Render the profiler
    if(UserConfigParams::m_profiler_enabled)
    {
        PROFILER_DRAW();
    }


#ifdef DEBUG
    drawDebugMeshes();
#endif


    PROFILER_PUSH_CPU_MARKER("EndSccene", 0x45, 0x75, 0x45);
    m_video_driver->endScene();
    PROFILER_POP_CPU_MARKER();

    getPostProcessing()->update(dt);
}

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

// --------------------------------------------

void IrrDriver::renderFixed(float dt)
{
    World *world = World::getWorld(); // Never NULL.

    m_video_driver->beginScene(/*backBuffer clear*/ true, /*zBuffer*/ true,
                               world->getClearColor());

    irr_driver->getVideoDriver()->enableMaterial2D();

    RaceGUIBase *rg = world->getRaceGUI();
    if (rg) rg->update(dt);


    for(unsigned int i=0; i<Camera::getNumCameras(); i++)
    {
        Camera *camera = Camera::getCamera(i);

        std::ostringstream oss;
        oss << "drawAll() for kart " << i;
        PROFILER_PUSH_CPU_MARKER(oss.str().c_str(), (i+1)*60,
                                 0x00, 0x00);
        camera->activate();
        rg->preRenderCallback(camera);   // adjusts start referee

        m_renderpass = ~0;
        m_scene_manager->drawAll();

        PROFILER_POP_CPU_MARKER();

        // Note that drawAll must be called before rendering
        // the bullet debug view, since otherwise the camera
        // is not set up properly. This is only used for
        // the bullet debug view.
        if (UserConfigParams::m_artist_debug_mode)
            World::getWorld()->getPhysics()->draw();
    }   // for i<world->getNumKarts()

    // Set the viewport back to the full screen for race gui
    m_video_driver->setViewPort(core::recti(0, 0,
                                            UserConfigParams::m_width,
                                            UserConfigParams::m_height));

    for(unsigned int i=0; i<Camera::getNumCameras(); i++)
    {
        Camera *camera = Camera::getCamera(i);
        std::ostringstream oss;
        oss << "renderPlayerView() for kart " << i;

        PROFILER_PUSH_CPU_MARKER(oss.str().c_str(), 0x00, 0x00, (i+1)*60);
        rg->renderPlayerView(camera, dt);
        PROFILER_POP_CPU_MARKER();

    }  // for i<getNumKarts

    // Either render the gui, or the global elements of the race gui.
    GUIEngine::render(dt);

    // Render the profiler
    if(UserConfigParams::m_profiler_enabled)
    {
        PROFILER_DRAW();
    }

#ifdef DEBUG
    drawDebugMeshes();
#endif

    m_video_driver->endScene();
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

    if (CVS->supportsIndirectInstancingRendering() && CVS->isARBExplicitAttribLocationUsable())
    {
#if !defined(USE_GLES2)
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
#endif
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
