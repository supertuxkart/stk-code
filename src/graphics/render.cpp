//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2009 Joerg Henrichs
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
#include "graphics/glwrap.hpp"
#include "graphics/lens_flare.hpp"
#include "graphics/lod_node.hpp"
#include "graphics/post_processing.hpp"
#include "graphics/referee.hpp"
#include "graphics/rtts.hpp"
#include "graphics/screenquad.hpp"
#include "graphics/shaders.hpp"
#include "graphics/stkmeshscenenode.hpp"
#include "items/item_manager.hpp"
#include "modes/world.hpp"
#include "physics/physics.hpp"
#include "physics/triangle_mesh.hpp"
#include "tracks/track.hpp"
#include "utils/profiler.hpp"
#include "stkscenemanager.hpp"
#include "items/powerup_manager.hpp"
#include "../../lib/irrlicht/source/Irrlicht/CSceneManager.h"
#include "../../lib/irrlicht/source/Irrlicht/os.h"

#include <limits>

#define MAX2(a, b) ((a) > (b) ? (a) : (b))
#define MIN2(a, b) ((a) > (b) ? (b) : (a))

void IrrDriver::renderGLSL(float dt)
{
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
        overridemat.Material.MaterialType = m_shaders->getShader(ES_MIPVIZ);
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

    if (!UserConfigParams::m_dynamic_lights)
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
        if (!UserConfigParams::m_dynamic_lights)
            camera->activate();
        rg->preRenderCallback(camera);   // adjusts start referee
        m_scene_manager->setActiveCamera(camnode);

        const core::recti &viewport = camera->getViewport();

        if (World::getWorld() && World::getWorld()->getTrack()->hasShadows() && !SphericalHarmonicsTextures.empty())
            irr_driver->getSceneManager()->setAmbientLight(SColor(0, 0, 0, 0));

        // TODO: put this outside of the rendering loop
        generateDiffuseCoefficients();
        if (!UserConfigParams::m_dynamic_lights)
            glEnable(GL_FRAMEBUFFER_SRGB);

        PROFILER_PUSH_CPU_MARKER("Update Light Info", 0xFF, 0x0, 0x0);
        unsigned plc = UpdateLightsInfo(camnode, dt);
        PROFILER_POP_CPU_MARKER();
        PROFILER_PUSH_CPU_MARKER("Compute camera matrix", 0x0, 0xFF, 0x0);
        computeCameraMatrix(camnode, viewport.LowerRightCorner.X - viewport.UpperLeftCorner.X, viewport.LowerRightCorner.Y - viewport.UpperLeftCorner.Y);
        PROFILER_POP_CPU_MARKER();
        renderScene(camnode, plc, glows, dt, track->hasShadows(), false);

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


                glUseProgram(UtilShader::ColoredLine::Program);
                glBindVertexArray(UtilShader::ColoredLine::vao);
                glBindBuffer(GL_ARRAY_BUFFER, UtilShader::ColoredLine::vbo);
                for (it = lines.begin(); it != lines.end(); it++)
                {
                    UtilShader::ColoredLine::setUniforms(it->first);
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
        if (UserConfigParams::m_dynamic_lights)
        {
            bool isRace = StateManager::get()->getGameState() == GUIEngine::GAME;
            FrameBuffer *fbo = m_post_processing->render(camnode, isRace);

            if (irr_driver->getNormals())
                irr_driver->getFBO(FBO_NORMAL_AND_DEPTHS).BlitToDefault(viewport.UpperLeftCorner.X, viewport.UpperLeftCorner.Y, viewport.LowerRightCorner.X, viewport.LowerRightCorner.Y);
            else if (irr_driver->getSSAOViz())
            {
                glBindFramebuffer(GL_FRAMEBUFFER, 0);
                glViewport(viewport.UpperLeftCorner.X, viewport.UpperLeftCorner.Y, viewport.LowerRightCorner.X, viewport.LowerRightCorner.Y);
                m_post_processing->renderPassThrough(m_rtts->getFBO(FBO_HALF1_R).getRTT()[0]);
            }
            else if (irr_driver->getRSM())
            {
                glBindFramebuffer(GL_FRAMEBUFFER, 0);
                glViewport(viewport.UpperLeftCorner.X, viewport.UpperLeftCorner.Y, viewport.LowerRightCorner.X, viewport.LowerRightCorner.Y);
                m_post_processing->renderPassThrough(m_rtts->getRSM().getRTT()[0]);
            }
            else if (irr_driver->getShadowViz())
            {
                renderShadowsDebug();
            }
            else
            {
                glEnable(GL_FRAMEBUFFER_SRGB);
                glBindFramebuffer(GL_FRAMEBUFFER, 0);
                if (UserConfigParams::m_dynamic_lights)
                    camera->activate();
                m_post_processing->renderPassThrough(fbo->getRTT()[0]);
                glDisable(GL_FRAMEBUFFER_SRGB);
            }
        }

        PROFILER_POP_CPU_MARKER();
    }   // for i<world->getNumKarts()

    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    glUseProgram(0);

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
    glBindBufferBase(GL_UNIFORM_BUFFER, 0, SharedObject::ViewProjectionMatrixesUBO);
    m_scene_manager->setActiveCamera(camnode);

    PROFILER_PUSH_CPU_MARKER("- Draw Call Generation", 0xFF, 0xFF, 0xFF);
    PrepareDrawCalls(camnode);
    PROFILER_POP_CPU_MARKER();
    // Shadows
    {
        // To avoid wrong culling, use the largest view possible
        m_scene_manager->setActiveCamera(m_suncam);
        if (!m_mipviz && !m_wireframe && UserConfigParams::m_dynamic_lights &&
            UserConfigParams::m_shadows && !irr_driver->needUBOWorkaround() && hasShadow)
        {
            PROFILER_PUSH_CPU_MARKER("- Shadow", 0x30, 0x6F, 0x90);
            renderShadows();
            PROFILER_POP_CPU_MARKER();
            if (UserConfigParams::m_gi)
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
    if (UserConfigParams::m_dynamic_lights || forceRTT)
    {
        m_rtts->getFBO(FBO_NORMAL_AND_DEPTHS).Bind();
        glClearColor(0., 0., 0., 0.);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
        renderSolidFirstPass();
    }
    PROFILER_POP_CPU_MARKER();



    // Lights
    {
        PROFILER_PUSH_CPU_MARKER("- Light", 0x00, 0xFF, 0x00);
        if (UserConfigParams::m_dynamic_lights)
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
    if (UserConfigParams::m_dynamic_lights || forceRTT)
    {
        m_rtts->getFBO(FBO_COLORS).Bind();
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
        m_rtts->getFBO(FBO_NORMAL_AND_DEPTHS).Bind();
        renderNormalsVisualisation();
        m_rtts->getFBO(FBO_COLORS).Bind();
    }

    if (UserConfigParams::m_dynamic_lights && World::getWorld() != NULL &&
        World::getWorld()->isFogEnabled())
    {
        PROFILER_PUSH_CPU_MARKER("- Fog", 0xFF, 0x00, 0x00);
        m_post_processing->renderFog();
        PROFILER_POP_CPU_MARKER();
    }

    PROFILER_PUSH_CPU_MARKER("- Skybox", 0xFF, 0x00, 0xFF);
    renderSkybox(camnode);
    PROFILER_POP_CPU_MARKER();

    if (getRH())
    {
        m_rtts->getFBO(FBO_COLORS).Bind();
        m_post_processing->renderRHDebug(m_rtts->getRH().getRTT()[0], m_rtts->getRH().getRTT()[1], m_rtts->getRH().getRTT()[2], rh_matrix, rh_extend);
    }

    if (getGI())
    {
        m_rtts->getFBO(FBO_COLORS).Bind();
        m_post_processing->renderGI(rh_matrix, rh_extend, m_rtts->getRH().getRTT()[0], m_rtts->getRH().getRTT()[1], m_rtts->getRH().getRTT()[2]);
    }

    PROFILER_PUSH_CPU_MARKER("- Glow", 0xFF, 0xFF, 0x00);
    // Render anything glowing.
    if (!m_mipviz && !m_wireframe && UserConfigParams::m_glow)
    {
        irr_driver->setPhase(GLOW_PASS);
        renderGlow(glows);
    } // end glow
    PROFILER_POP_CPU_MARKER();

    PROFILER_PUSH_CPU_MARKER("- Lensflare/godray", 0x00, 0xFF, 0xFF);
    computeSunVisibility();
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
    if (!UserConfigParams::m_dynamic_lights && !forceRTT)
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

void IrrDriver::computeSunVisibility()
{
    // Is the lens flare enabled & visible? Check last frame's query.
    bool hasgodrays = false;

    if (World::getWorld() != NULL)
    {
        hasgodrays = World::getWorld()->getTrack()->hasGodRays();
    }

    if (UserConfigParams::m_light_shaft && hasgodrays)
    {
        GLuint res = 0;
        if (m_query_issued)
            glGetQueryObjectuiv(m_lensflare_query, GL_QUERY_RESULT, &res);
        m_post_processing->setSunPixels(res);

        // Prepare the query for the next frame.
        glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
        glBeginQuery(GL_SAMPLES_PASSED_ARB, m_lensflare_query);
        m_scene_manager->setCurrentRendertime(scene::ESNRP_SOLID);
        m_scene_manager->drawAll(scene::ESNRP_CAMERA);
        irr_driver->setPhase(GLOW_PASS);
        m_sun_interposer->render();
        glEndQuery(GL_SAMPLES_PASSED_ARB);
        m_query_issued = true;

        m_lensflare->setStrength(res / 4000.0f);

        // Make sure the color mask is reset
        glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
    }
}

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

/** Given a matrix transform and a set of points returns an orthogonal projection matrix that maps coordinates of
    transformed points between -1 and 1.
*  \param transform a transform matrix.
*  \param pointsInside a vector of point in 3d space.
*/
core::matrix4 getTighestFitOrthoProj(const core::matrix4 &transform, const std::vector<vector3df> &pointsInside)
{
    float xmin = std::numeric_limits<float>::infinity();
    float xmax = -std::numeric_limits<float>::infinity();
    float ymin = std::numeric_limits<float>::infinity();
    float ymax = -std::numeric_limits<float>::infinity();
    float zmin = std::numeric_limits<float>::infinity();
    float zmax = -std::numeric_limits<float>::infinity();

    for (unsigned i = 0; i < pointsInside.size(); i++)
    {
        vector3df TransformedVector;
        transform.transformVect(TransformedVector, pointsInside[i]);
        xmin = MIN2(xmin, TransformedVector.X);
        xmax = MAX2(xmax, TransformedVector.X);
        ymin = MIN2(ymin, TransformedVector.Y);
        ymax = MAX2(ymax, TransformedVector.Y);
        zmin = MIN2(zmin, TransformedVector.Z);
        zmax = MAX2(zmax, TransformedVector.Z);
    }

    float left = xmin;
    float right = xmax;
    float up = ymin;
    float down = ymax;

    core::matrix4 tmp_matrix;
    // Prevent Matrix without extend
    if (left == right || up == down)
        return tmp_matrix;
    tmp_matrix.buildProjectionMatrixOrthoLH(left, right,
        down, up,
        30, zmax);
    return tmp_matrix;
}

void IrrDriver::computeCameraMatrix(scene::ICameraSceneNode * const camnode, size_t width, size_t height)
{
    static_cast<scene::CSceneManager *>(m_scene_manager)->OnAnimate(os::Timer::getTime());
    camnode->render();
    irr_driver->setProjMatrix(irr_driver->getVideoDriver()->getTransform(video::ETS_PROJECTION));
    irr_driver->setViewMatrix(irr_driver->getVideoDriver()->getTransform(video::ETS_VIEW));
    irr_driver->genProjViewMatrix();

    m_current_screen_size = core::vector2df(float(width), float(height));

    const float oldfar = camnode->getFarValue();
    const float oldnear = camnode->getNearValue();
    float FarValues[] =
    {
        6.,
        21.,
        55.,
        150.,
    };
    float NearValues[] =
    {
        oldnear,
        5.,
        20.,
        50.,
    };


    float tmp[16 * 9 + 2];
    memcpy(tmp, irr_driver->getViewMatrix().pointer(), 16 * sizeof(float));
    memcpy(&tmp[16], irr_driver->getProjMatrix().pointer(), 16 * sizeof(float));
    memcpy(&tmp[32], irr_driver->getInvViewMatrix().pointer(), 16 * sizeof(float));
    memcpy(&tmp[48], irr_driver->getInvProjMatrix().pointer(), 16 * sizeof(float));
    memcpy(&tmp[64], irr_driver->getProjViewMatrix().pointer(), 16 * sizeof(float));

    const core::matrix4 &SunCamViewMatrix = m_suncam->getViewMatrix();
    for (unsigned i = 0; i < 4; i++)
    {
        if (!m_shadow_camnodes[i])
            m_shadow_camnodes[i] = (scene::ICameraSceneNode *) m_suncam->clone();
    }
    sun_ortho_matrix.clear();

    if (World::getWorld() && World::getWorld()->getTrack())
    {
        btVector3 btmin, btmax;
        if (World::getWorld()->getTrack()->getPtrTriangleMesh())
        {
            World::getWorld()->getTrack()->getTriangleMesh().getCollisionShape().getAabb(btTransform::getIdentity(), btmin, btmax);
        }
        const Vec3 vmin = btmin , vmax = btmax;

        // Build the 3 ortho projection (for the 3 shadow resolution levels)
        for (unsigned i = 0; i < 4; i++)
        {
            camnode->setFarValue(FarValues[i]);
            camnode->setNearValue(NearValues[i]);
            camnode->render();
            const scene::SViewFrustum *frustrum = camnode->getViewFrustum();
            float tmp[24] = {
                frustrum->getFarLeftDown().X,
                frustrum->getFarLeftDown().Y,
                frustrum->getFarLeftDown().Z,
                frustrum->getFarLeftUp().X,
                frustrum->getFarLeftUp().Y,
                frustrum->getFarLeftUp().Z,
                frustrum->getFarRightDown().X,
                frustrum->getFarRightDown().Y,
                frustrum->getFarRightDown().Z,
                frustrum->getFarRightUp().X,
                frustrum->getFarRightUp().Y,
                frustrum->getFarRightUp().Z,
                frustrum->getNearLeftDown().X,
                frustrum->getNearLeftDown().Y,
                frustrum->getNearLeftDown().Z,
                frustrum->getNearLeftUp().X,
                frustrum->getNearLeftUp().Y,
                frustrum->getNearLeftUp().Z,
                frustrum->getNearRightDown().X,
                frustrum->getNearRightDown().Y,
                frustrum->getNearRightDown().Z,
                frustrum->getNearRightUp().X,
                frustrum->getNearRightUp().Y,
                frustrum->getNearRightUp().Z,
            };
            memcpy(m_shadows_cam[i], tmp, 24 * sizeof(float));
            const core::aabbox3df smallcambox = camnode->
                getViewFrustum()->getBoundingBox();
            core::aabbox3df trackbox(vmin.toIrrVector(), vmax.toIrrVector() -
                core::vector3df(0, 30, 0));

            // Set up a nice ortho projection that contains our camera frustum
            core::aabbox3df box = smallcambox;
            box = box.intersect(trackbox);


            std::vector<vector3df> vectors;
            vectors.push_back(frustrum->getFarLeftDown());
            vectors.push_back(frustrum->getFarLeftUp());
            vectors.push_back(frustrum->getFarRightDown());
            vectors.push_back(frustrum->getFarRightUp());
            vectors.push_back(frustrum->getNearLeftDown());
            vectors.push_back(frustrum->getNearLeftUp());
            vectors.push_back(frustrum->getNearRightDown());
            vectors.push_back(frustrum->getNearRightUp());

/*            SunCamViewMatrix.transformBoxEx(trackbox);
            SunCamViewMatrix.transformBoxEx(box);

            core::vector3df extent = box.getExtent();
            const float w = fabsf(extent.X);
            const float h = fabsf(extent.Y);
            float z = box.MaxEdge.Z;

            // Snap to texels
            const float units_per_w = w / 1024;
            const float units_per_h = h / 1024;*/

            m_shadow_camnodes[i]->setProjectionMatrix(getTighestFitOrthoProj(SunCamViewMatrix, vectors) , true);
            m_shadow_camnodes[i]->render();

            sun_ortho_matrix.push_back(getVideoDriver()->getTransform(video::ETS_PROJECTION) * getVideoDriver()->getTransform(video::ETS_VIEW));
        }

        {
            core::aabbox3df trackbox(vmin.toIrrVector(), vmax.toIrrVector() -
                core::vector3df(0, 30, 0));
            if (trackbox.MinEdge.X != trackbox.MaxEdge.X &&
                trackbox.MinEdge.Y != trackbox.MaxEdge.Y &&
                // Cover the case where SunCamViewMatrix is null
                SunCamViewMatrix.getScale() != core::vector3df(0., 0., 0.))
            {
                SunCamViewMatrix.transformBoxEx(trackbox);
                core::matrix4 tmp_matrix;
                tmp_matrix.buildProjectionMatrixOrthoLH(trackbox.MinEdge.X, trackbox.MaxEdge.X,
                    trackbox.MaxEdge.Y, trackbox.MinEdge.Y,
                    30, trackbox.MaxEdge.Z);
                m_suncam->setProjectionMatrix(tmp_matrix, true);
                m_suncam->render();
            }
            rsm_matrix = getVideoDriver()->getTransform(video::ETS_PROJECTION) * getVideoDriver()->getTransform(video::ETS_VIEW);
        }
        rh_extend = core::vector3df(128, 64, 128);
        core::vector3df campos = camnode->getAbsolutePosition();
        core::vector3df translation(8 * floor(campos.X / 8), 8 * floor(campos.Y / 8), 8 * floor(campos.Z / 8));
        rh_matrix.setTranslation(translation);


        assert(sun_ortho_matrix.size() == 4);
        camnode->setNearValue(oldnear);
        camnode->setFarValue(oldfar);
        camnode->render();

        size_t size = irr_driver->getShadowViewProj().size();
        for (unsigned i = 0; i < size; i++)
            memcpy(&tmp[16 * i + 80], irr_driver->getShadowViewProj()[i].pointer(), 16 * sizeof(float));
    }

    tmp[144] = float(width);
    tmp[145] = float(height);
    glBindBuffer(GL_UNIFORM_BUFFER, SharedObject::ViewProjectionMatrixesUBO);
    glBufferSubData(GL_UNIFORM_BUFFER, 0, (16 * 9 + 2) * sizeof(float), tmp);
}



static void renderWireFrameFrustrum(float *tmp, unsigned i)
{
    glUseProgram(MeshShader::ViewFrustrumShader::Program);
    glBindVertexArray(MeshShader::ViewFrustrumShader::frustrumvao);
    glBindBuffer(GL_ARRAY_BUFFER, SharedObject::frustrumvbo);

    glBufferSubData(GL_ARRAY_BUFFER, 0, 8 * 3 * sizeof(float), (void *)tmp);
    MeshShader::ViewFrustrumShader::setUniforms(video::SColor(255, 0, 255, 0), i);
    glDrawElements(GL_LINES, 24, GL_UNSIGNED_INT, 0);
}


void IrrDriver::renderShadowsDebug()
{
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glViewport(0, UserConfigParams::m_height / 2, UserConfigParams::m_width / 2, UserConfigParams::m_height / 2);
    m_post_processing->renderTextureLayer(m_rtts->getShadowDepthTex(), 0);
    renderWireFrameFrustrum(m_shadows_cam[0], 0);
    glViewport(UserConfigParams::m_width / 2, UserConfigParams::m_height / 2, UserConfigParams::m_width / 2, UserConfigParams::m_height / 2);
    m_post_processing->renderTextureLayer(m_rtts->getShadowDepthTex(), 1);
    renderWireFrameFrustrum(m_shadows_cam[1], 1);
    glViewport(0, 0, UserConfigParams::m_width / 2, UserConfigParams::m_height / 2);
    m_post_processing->renderTextureLayer(m_rtts->getShadowDepthTex(), 2);
    renderWireFrameFrustrum(m_shadows_cam[2], 2);
    glViewport(UserConfigParams::m_width / 2, 0, UserConfigParams::m_width / 2, UserConfigParams::m_height / 2);
    m_post_processing->renderTextureLayer(m_rtts->getShadowDepthTex(), 3);
    renderWireFrameFrustrum(m_shadows_cam[3], 3);
    glViewport(0, 0, UserConfigParams::m_width, UserConfigParams::m_height);
}

// ----------------------------------------------------------------------------

void IrrDriver::renderGlow(std::vector<GlowData>& glows)
{
    m_scene_manager->setCurrentRendertime(scene::ESNRP_SOLID);
    m_rtts->getFBO(FBO_TMP1_WITH_DS).Bind();
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

    if (irr_driver->hasARB_base_instance())
        glBindVertexArray(VAOManager::getInstance()->getVAO(EVT_STANDARD));
    for (u32 i = 0; i < glowcount; i++)
    {
        const GlowData &dat = glows[i];
        scene::ISceneNode * cur = dat.node;

        STKMeshSceneNode *node = static_cast<STKMeshSceneNode *>(cur);
        node->setGlowColors(SColor(0, (unsigned) (dat.b * 255.f), (unsigned)(dat.g * 255.f), (unsigned)(dat.r * 255.f)));
        if (!irr_driver->hasARB_draw_indirect())
            node->render();
    }

    if (irr_driver->hasARB_draw_indirect())
    {
        glBindBuffer(GL_DRAW_INDIRECT_BUFFER, GlowPassCmd::getInstance()->drawindirectcmd);
        glUseProgram(MeshShader::InstancedColorizeShader::getInstance()->Program);

        glBindVertexArray(VAOManager::getInstance()->getInstanceVAO(video::EVT_STANDARD, InstanceTypeGlow));
        if (UserConfigParams::m_azdo)
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
    m_rtts->getFBO(FBO_COLORS).Bind();
    m_post_processing->renderGlow(m_rtts->getRenderTarget(RTT_QUARTER1));
    glDisable(GL_STENCIL_TEST);
}
