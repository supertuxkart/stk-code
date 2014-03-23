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
#include "graphics/camera.hpp"
#include "graphics/glow.hpp"
#include "graphics/glwrap.hpp"
#include "graphics/lens_flare.hpp"
#include "graphics/light.hpp"
#include "graphics/lod_node.hpp"
#include "graphics/material_manager.hpp"
#include "graphics/particle_kind_manager.hpp"
#include "graphics/per_camera_node.hpp"
#include "graphics/post_processing.hpp"
#include "graphics/referee.hpp"
#include "graphics/rtts.hpp"
#include "graphics/screenquad.hpp"
#include "graphics/shaders.hpp"
#include "graphics/shadow_importance.hpp"
#include "graphics/stkmeshscenenode.hpp"
#include "graphics/stkinstancedscenenode.hpp"
#include "graphics/wind.hpp"
#include "io/file_manager.hpp"
#include "items/item.hpp"
#include "items/item_manager.hpp"
#include "modes/world.hpp"
#include "physics/physics.hpp"
#include "tracks/track.hpp"
#include "utils/constants.hpp"
#include "utils/helpers.hpp"
#include "utils/log.hpp"
#include "utils/profiler.hpp"

#include <algorithm>

STKInstancedSceneNode *InstancedBox = 0;

void IrrDriver::renderGLSL(float dt)
{
    World *world = World::getWorld(); // Never NULL.

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
    std::vector<GlowNode *> transparent_glow_nodes;

    ItemManager * const items = ItemManager::get();
    const u32 itemcount = items->getNumberOfItems();
    u32 i;

    // For each static node, give it a glow representation
    const u32 staticglows = glows.size();
    for (i = 0; i < staticglows; i++)
    {
        scene::ISceneNode * const node = glows[i].node;

        const float radius = (node->getBoundingBox().getExtent().getLength() / 2) * 2.0f;
        GlowNode * const repnode = new GlowNode(irr_driver->getSceneManager(), radius);
        repnode->setPosition(node->getTransformedBoundingBox().getCenter());
        transparent_glow_nodes.push_back(repnode);
    }

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

        // Push back its representation too
        const float radius = (node->getBoundingBox().getExtent().getLength() / 2) * 2.0f;
        GlowNode * const repnode = new GlowNode(irr_driver->getSceneManager(), radius);
        repnode->setPosition(node->getTransformedBoundingBox().getCenter());
        transparent_glow_nodes.push_back(repnode);
    }

    if (!InstancedBox)
    {
        InstancedBox = new STKInstancedSceneNode(items->getItemModel(items->getItem(0)->getType()), m_scene_manager->getRootSceneNode(), m_scene_manager, -1);
        InstancedBox->addWorldMatrix(core::vector3df(0, 0, 0));
        InstancedBox->addWorldMatrix(core::vector3df(1., 0, 0));
        InstancedBox->addWorldMatrix(core::vector3df(0, 1., 0));
        InstancedBox->addWorldMatrix(core::vector3df(0, 0, 1.));
    }

    // Start the RTT for post-processing.
    // We do this before beginScene() because we want to capture the glClear()
    // because of tracks that do not have skyboxes (generally add-on tracks)
    m_post_processing->begin();
    m_video_driver->setRenderTarget(m_rtts->getRTT(RTT_COLOR), false, false);

    m_video_driver->beginScene(/*backBuffer clear*/ true, /*zBuffer*/ true,
                               world->getClearColor());

    // Clear normal and depth to zero
    m_video_driver->setRenderTarget(m_rtts->getRTT(RTT_NORMAL_AND_DEPTH), true, false, video::SColor(0,0,0,0));

    irr_driver->getVideoDriver()->enableMaterial2D();
    RaceGUIBase *rg = world->getRaceGUI();
    if (rg) rg->update(dt);


    for(unsigned int cam = 0; cam < Camera::getNumCameras(); cam++)
    {
        Camera * const camera = Camera::getCamera(cam);
        scene::ICameraSceneNode * const camnode = camera->getCameraSceneNode();

#ifdef ENABLE_PROFILER
        std::ostringstream oss;
        oss << "drawAll() for kart " << cam << std::flush;
        PROFILER_PUSH_CPU_MARKER(oss.str().c_str(), (cam+1)*60,
                                 0x00, 0x00);
#endif
        camera->activate();
        rg->preRenderCallback(camera);   // adjusts start referee

        const u32 bgnodes = m_background.size();
/*        if (bgnodes)
        {
            // If there are background nodes (3d skybox), draw them now.
            m_video_driver->setRenderTarget(m_rtts->getRTT(RTT_COLOR), false, false);

            m_renderpass = scene::ESNRP_SKY_BOX;
            m_scene_manager->drawAll(m_renderpass);

            const video::SOverrideMaterial prev = overridemat;
            overridemat.Enabled = 1;
            overridemat.EnableFlags = video::EMF_MATERIAL_TYPE;
            overridemat.Material.MaterialType = video::EMT_TRANSPARENT_ALPHA_CHANNEL_REF;

            for (i = 0; i < bgnodes; i++)
            {
                m_background[i]->setPosition(camnode->getPosition() * 0.97f);
                m_background[i]->updateAbsolutePosition();
                m_background[i]->render();
            }

            overridemat = prev;
            m_video_driver->setRenderTarget(m_rtts->getRTT(RTT_COLOR), false, true);
        }*/

        // Fire up the MRT
		irr_driver->getVideoDriver()->setRenderTarget(irr_driver->getRTT(RTT_NORMAL_AND_DEPTH), false, false);
		PROFILER_PUSH_CPU_MARKER("- Solid Pass 1", 0xFF, 0x00, 0x00);
        m_renderpass = scene::ESNRP_CAMERA | scene::ESNRP_SOLID;
		glDepthFunc(GL_LEQUAL);
		glEnable(GL_DEPTH_TEST);
		glDisable(GL_ALPHA_TEST);
		glDepthMask(GL_TRUE);
		glDisable(GL_BLEND);
        glEnable(GL_CULL_FACE);
        irr_driver->setPhase(SOLID_NORMAL_AND_DEPTH_PASS);
        m_scene_manager->drawAll(m_renderpass);
        irr_driver->setProjMatrix(irr_driver->getVideoDriver()->getTransform(video::ETS_PROJECTION));
        irr_driver->setViewMatrix(irr_driver->getVideoDriver()->getTransform(video::ETS_VIEW));
        irr_driver->genProjViewMatrix();
        InstancedBox->render();
		PROFILER_POP_CPU_MARKER();

        // Todo : reenable glow and shadows
        //ShadowImportanceProvider * const sicb = (ShadowImportanceProvider *)
        //                                         irr_driver->getCallback(ES_SHADOW_IMPORTANCE);
        //sicb->updateIPVMatrix();

        // Used to cull glowing items & lights
        const core::aabbox3df cambox = camnode->getViewFrustum()->getBoundingBox();

        PROFILER_PUSH_CPU_MARKER("- Shadow", 0x30, 0x6F, 0x90);
        // Shadows
        if (!m_mipviz && !m_wireframe && UserConfigParams::m_shadows)
           //&& World::getWorld()->getTrack()->hasShadows())
        {
            renderShadows(camnode, camera);
        }
        PROFILER_POP_CPU_MARKER();

        PROFILER_PUSH_CPU_MARKER("- Light", 0x00, 0xFF, 0x00);

        // Lights
        renderLights(cambox, camnode, overridemat, cam, dt);
		PROFILER_POP_CPU_MARKER();

		PROFILER_PUSH_CPU_MARKER("- Solid Pass 2", 0x00, 0x00, 0xFF);
        irr_driver->setPhase(SOLID_LIT_PASS);
		glEnable(GL_DEPTH_TEST);
		glDisable(GL_ALPHA_TEST);
		glDepthMask(GL_FALSE);
		glDisable(GL_BLEND);
        m_renderpass = scene::ESNRP_CAMERA | scene::ESNRP_SOLID;
        setTexture(0, getTextureGLuint(irr_driver->getRTT(RTT_TMP1)), GL_NEAREST, GL_NEAREST);
        setTexture(1, getTextureGLuint(irr_driver->getRTT(RTT_TMP2)), GL_NEAREST, GL_NEAREST);
        setTexture(2, getTextureGLuint(irr_driver->getRTT(RTT_SSAO)), GL_NEAREST, GL_NEAREST);
        if (!UserConfigParams::m_ssao)
        {
            GLint swizzleMask[] = { GL_ONE, GL_ONE, GL_ONE, GL_ONE };
            glTexParameteriv(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_RGBA, swizzleMask);
        }
        m_scene_manager->drawAll(m_renderpass);
        InstancedBox->render();
        PROFILER_POP_CPU_MARKER();

		  if (World::getWorld()->getTrack()->isFogEnabled())
		  {
			  PROFILER_PUSH_CPU_MARKER("- Fog", 0xFF, 0x00, 0x00);
			  m_post_processing->renderFog(irr_driver->getInvProjMatrix());
			  PROFILER_POP_CPU_MARKER();
		  }

        PROFILER_PUSH_CPU_MARKER("- Glow", 0xFF, 0xFF, 0x00);

		// Render anything glowing.
		if (!m_mipviz && !m_wireframe)
		{
			irr_driver->setPhase(GLOW_PASS);
			renderGlow(overridemat, glows, cambox, cam);
		} // end glow

        PROFILER_POP_CPU_MARKER();

		PROFILER_PUSH_CPU_MARKER("- Skybox", 0xFF, 0x00, 0xFF);
		renderSkybox();
		PROFILER_POP_CPU_MARKER();

        PROFILER_PUSH_CPU_MARKER("- Lensflare/godray", 0x00, 0xFF, 0xFF);
        // Is the lens flare enabled & visible? Check last frame's query.
        const bool hasflare = World::getWorld()->getTrack()->hasLensFlare();
        const bool hasgodrays = World::getWorld()->getTrack()->hasGodRays();
        if (true)//hasflare || hasgodrays)
        {
            irr::video::COpenGLDriver*	gl_driver = (irr::video::COpenGLDriver*)m_device->getVideoDriver();

            GLuint res = 0;
            if (m_query_issued)
                gl_driver->extGlGetQueryObjectuiv(m_lensflare_query, GL_QUERY_RESULT, &res);
            m_post_processing->setSunPixels(res);

            // Prepare the query for the next frame.
            glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
            gl_driver->extGlBeginQuery(GL_SAMPLES_PASSED_ARB, m_lensflare_query);
            m_scene_manager->setCurrentRendertime(scene::ESNRP_SOLID);
            m_scene_manager->drawAll(scene::ESNRP_CAMERA);
            irr_driver->setPhase(GLOW_PASS);
            m_sun_interposer->render();
            gl_driver->extGlEndQuery(GL_SAMPLES_PASSED_ARB);
            m_query_issued = true;

            m_lensflare->setStrength(res / 4000.0f);

            if (hasflare)
                m_lensflare->OnRegisterSceneNode();

            // Make sure the color mask is reset
            glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
        }
        PROFILER_POP_CPU_MARKER();

        // We need to re-render camera due to the per-cam-node hack.
        PROFILER_PUSH_CPU_MARKER("- Transparent Pass", 0xFF, 0x00, 0x00);
		irr_driver->setPhase(TRANSPARENT_PASS);
		m_renderpass = scene::ESNRP_CAMERA | scene::ESNRP_TRANSPARENT;
		glEnable(GL_DEPTH_TEST);
		glDisable(GL_ALPHA_TEST);
		glDepthMask(GL_FALSE);
		glEnable(GL_BLEND);
		glBlendEquation(GL_FUNC_ADD);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glDisable(GL_CULL_FACE);
        m_scene_manager->drawAll(m_renderpass);
		PROFILER_POP_CPU_MARKER();

		PROFILER_PUSH_CPU_MARKER("- Particles", 0xFF, 0xFF, 0x00);
		m_renderpass = scene::ESNRP_CAMERA | scene::ESNRP_TRANSPARENT_EFFECT;
		glDepthMask(GL_FALSE);
		glDisable(GL_CULL_FACE);
		glEnable(GL_BLEND);
		glBlendEquation(GL_FUNC_ADD);
		m_scene_manager->drawAll(m_renderpass);
		PROFILER_POP_CPU_MARKER();

        PROFILER_PUSH_CPU_MARKER("- Displacement", 0x00, 0x00, 0xFF);
        // Handle displacing nodes, if any
        const u32 displacingcount = m_displacing.size();
        if (displacingcount)
        {
            renderDisplacement(overridemat, cam);
        }
        PROFILER_POP_CPU_MARKER();

        // Drawing for this cam done, cleanup
        const u32 glowrepcount = transparent_glow_nodes.size();
        for (i = 0; i < glowrepcount; i++)
        {
            transparent_glow_nodes[i]->remove();
            transparent_glow_nodes[i]->drop();
        }

        PROFILER_POP_CPU_MARKER();

        // Note that drawAll must be called before rendering
        // the bullet debug view, since otherwise the camera
        // is not set up properly. This is only used for
        // the bullet debug view.
        if (UserConfigParams::m_artist_debug_mode)
            World::getWorld()->getPhysics()->draw();
    }   // for i<world->getNumKarts()

    PROFILER_PUSH_CPU_MARKER("Postprocessing", 0xFF, 0xFF, 0x00);
    // Render the post-processed scene
    m_post_processing->render();
    PROFILER_POP_CPU_MARKER();

    // Set the viewport back to the full screen for race gui
    m_video_driver->setViewPort(core::recti(0, 0,
                                            UserConfigParams::m_width,
                                            UserConfigParams::m_height));

    for(unsigned int i=0; i<Camera::getNumCameras(); i++)
    {
        Camera *camera = Camera::getCamera(i);
        char marker_name[100];
        sprintf(marker_name, "renderPlayerView() for kart %d", i);

        PROFILER_PUSH_CPU_MARKER(marker_name, 0x00, 0x00, (i+1)*60);
        rg->renderPlayerView(camera, dt);

        PROFILER_POP_CPU_MARKER();
    }  // for i<getNumKarts

    PROFILER_PUSH_CPU_MARKER("GUIEngine", 0x75, 0x75, 0x75);
    // Either render the gui, or the global elements of the race gui.
    GUIEngine::render(dt);
    PROFILER_POP_CPU_MARKER();

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

#ifdef ENABLE_PROFILER
        std::ostringstream oss;
        oss << "drawAll() for kart " << i << std::flush;
        PROFILER_PUSH_CPU_MARKER(oss.str().c_str(), (i+1)*60,
                                 0x00, 0x00);
#endif
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
        char marker_name[100];
        sprintf(marker_name, "renderPlayerView() for kart %d", i);

        PROFILER_PUSH_CPU_MARKER(marker_name, 0x00, 0x00, (i+1)*60);
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

void IrrDriver::renderShadows(//ShadowImportanceProvider * const sicb,
                              scene::ICameraSceneNode * const camnode,
                              //video::SOverrideMaterial &overridemat,
                              Camera * const camera)
{
    m_scene_manager->setCurrentRendertime(scene::ESNRP_SOLID);

    const Vec3 *vmin, *vmax;
    World::getWorld()->getTrack()->getAABB(&vmin, &vmax);

    const float oldfar = camnode->getFarValue();
    const float oldnear = camnode->getNearValue();
    float FarValues[] =
    {
        20.,
        50.,
        100.,
        oldfar,
    };
    float NearValues[] =
    {
        oldnear,
        20.,
        50.,
        100.,
    };

    const core::matrix4 &SunCamViewMatrix = m_suncam->getViewMatrix();
    sun_ortho_matrix.clear();

    // Build the 3 ortho projection (for the 3 shadow resolution levels)
    for (unsigned i = 0; i < 4; i++)
    {
        camnode->setFarValue(FarValues[i]);
        camnode->setNearValue(NearValues[i]);
        camnode->render();
        const core::aabbox3df smallcambox = camnode->
            getViewFrustum()->getBoundingBox();
        core::aabbox3df trackbox(vmin->toIrrVector(), vmax->toIrrVector() -
            core::vector3df(0, 30, 0));


        // Set up a nice ortho projection that contains our camera frustum
        core::aabbox3df box = smallcambox;
        box = box.intersect(trackbox);


        SunCamViewMatrix.transformBoxEx(trackbox);
        SunCamViewMatrix.transformBoxEx(box);

        core::vector3df extent = trackbox.getExtent();
        const float w = fabsf(extent.X);
        const float h = fabsf(extent.Y);
        float z = box.MaxEdge.Z;

        // Snap to texels
        const float units_per_w = w / 1024;
        const float units_per_h = h / 1024;

        float left = box.MinEdge.X;
        float right = box.MaxEdge.X;
        float up = box.MaxEdge.Y;
        float down = box.MinEdge.Y;

        left -= fmodf(left, units_per_w);
        right -= fmodf(right, units_per_w);
        up -= fmodf(up, units_per_h);
        down -= fmodf(down, units_per_h);
        z -= fmodf(z, 0.5f);

        // FIXME: quick and dirt (and wrong) workaround to avoid division by zero
        if (left == right) right += 0.1f;
        if (up == down) down += 0.1f;
        if (z == 30) z += 0.1f;

        core::matrix4 tmp_matrix;

        tmp_matrix.buildProjectionMatrixOrthoLH(left, right,
            up, down,
            30, z);
        m_suncam->setProjectionMatrix(tmp_matrix, true);
        m_scene_manager->setActiveCamera(m_suncam);
        m_suncam->render();

        sun_ortho_matrix.push_back(getVideoDriver()->getTransform(video::ETS_PROJECTION) * getVideoDriver()->getTransform(video::ETS_VIEW));
    }
    assert(sun_ortho_matrix.size() == 4);

    irr_driver->setPhase(SHADOW_PASS);
    glDisable(GL_BLEND);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_FRONT);
    glBindFramebuffer(GL_FRAMEBUFFER, m_rtts->getShadowFBO());
    glViewport(0, 0, 1024, 1024);
    glClear(GL_DEPTH_BUFFER_BIT);
    glDrawBuffer(GL_NONE);
    m_scene_manager->drawAll(scene::ESNRP_SOLID);
    glCullFace(GL_BACK);

    camnode->setNearValue(oldnear);
    camnode->setFarValue(oldfar);
    camnode->render();
    camera->activate();
    m_scene_manager->drawAll(scene::ESNRP_CAMERA);


    //sun_ortho_matrix *= m_suncam->getViewMatrix();
/*    ((SunLightProvider *) m_shaders->m_callbacks[ES_SUNLIGHT])->setShadowMatrix(ortho);
    sicb->setShadowMatrix(ortho);

    overridemat.Enabled = 0;

    // Render the importance map
    m_video_driver->setRenderTarget(m_rtts->getRTT(RTT_COLLAPSE), true, true);

    m_shadow_importance->render();

    CollapseProvider * const colcb = (CollapseProvider *)
                                            m_shaders->
                                            m_callbacks[ES_COLLAPSE];
    ScreenQuad sq(m_video_driver);
    sq.setMaterialType(m_shaders->getShader(ES_COLLAPSE));
    sq.setTexture(m_rtts->getRTT(RTT_COLLAPSE));
    sq.getMaterial().setFlag(EMF_BILINEAR_FILTER, false);

    const TypeRTT oldh = tick ? RTT_COLLAPSEH : RTT_COLLAPSEH2;
    const TypeRTT oldv = tick ? RTT_COLLAPSEV : RTT_COLLAPSEV2;
    const TypeRTT curh = tick ? RTT_COLLAPSEH2 : RTT_COLLAPSEH;
    const TypeRTT curv = tick ? RTT_COLLAPSEV2 : RTT_COLLAPSEV;

    colcb->setResolution(1, m_rtts->getRTT(RTT_WARPV)->getSize().Height);
    sq.setTexture(m_rtts->getRTT(oldh), 1);
    sq.render(m_rtts->getRTT(RTT_WARPH));

    colcb->setResolution(m_rtts->getRTT(RTT_WARPV)->getSize().Height, 1);
    sq.setTexture(m_rtts->getRTT(oldv), 1);
    sq.render(m_rtts->getRTT(RTT_WARPV));

    sq.setTexture(0, 1);
    ((GaussianBlurProvider *) m_shaders->m_callbacks[ES_GAUSSIAN3H])->setResolution(
                m_rtts->getRTT(RTT_WARPV)->getSize().Height,
                m_rtts->getRTT(RTT_WARPV)->getSize().Height);

    sq.setMaterialType(m_shaders->getShader(ES_GAUSSIAN6H));
    sq.setTexture(m_rtts->getRTT(RTT_WARPH));
    sq.render(m_rtts->getRTT(curh));

    sq.setMaterialType(m_shaders->getShader(ES_GAUSSIAN6V));
    sq.setTexture(m_rtts->getRTT(RTT_WARPV));
    sq.render(m_rtts->getRTT(curv));*/

    // Convert importance maps to warp maps
    //
    // It should be noted that while they do repeated work
    // calculating the min, max, and total, it's several hundred us
    // faster to do that than to do it once in a separate shader
    // (shader switch overhead, measured).
    /*colcb->setResolution(m_rtts->getRTT(RTT_WARPV)->getSize().Height,
                            m_rtts->getRTT(RTT_WARPV)->getSize().Height);

    sq.setMaterialType(m_shaders->getShader(ES_SHADOW_WARPH));
    sq.setTexture(m_rtts->getRTT(curh));
    sq.render(m_rtts->getRTT(RTT_WARPH));

    sq.setMaterialType(m_shaders->getShader(ES_SHADOW_WARPV));
    sq.setTexture(m_rtts->getRTT(curv));
    sq.render(m_rtts->getRTT(RTT_WARPV));*/

    // Actual shadow map


/*    overridemat.Material.MaterialType = m_shaders->getShader(ES_SHADOWPASS);
    overridemat.EnableFlags = video::EMF_MATERIAL_TYPE | video::EMF_TEXTURE1 |
                                video::EMF_TEXTURE2;
    overridemat.EnablePasses = scene::ESNRP_SOLID;
    overridemat.Material.setTexture(1, m_rtts->getRTT(RTT_WARPH));
    overridemat.Material.setTexture(2, m_rtts->getRTT(RTT_WARPV));
    overridemat.Material.TextureLayer[1].TextureWrapU =
    overridemat.Material.TextureLayer[1].TextureWrapV =
    overridemat.Material.TextureLayer[2].TextureWrapU =
    overridemat.Material.TextureLayer[2].TextureWrapV = video::ETC_CLAMP_TO_EDGE;
    overridemat.Material.TextureLayer[1].BilinearFilter =
    overridemat.Material.TextureLayer[2].BilinearFilter = true;
    overridemat.Material.TextureLayer[1].TrilinearFilter =
    overridemat.Material.TextureLayer[2].TrilinearFilter = false;
    overridemat.Material.TextureLayer[1].AnisotropicFilter =
    overridemat.Material.TextureLayer[2].AnisotropicFilter = 0;
    overridemat.Material.Wireframe = 1;
    overridemat.Enabled = true;*/



//    overridemat.EnablePasses = 0;
//    overridemat.Enabled = false;
}

// ----------------------------------------------------------------------------

void IrrDriver::renderGlow(video::SOverrideMaterial &overridemat,
                           std::vector<GlowData>& glows,
                           const core::aabbox3df& cambox,
                           int cam)
{
    m_scene_manager->setCurrentRendertime(scene::ESNRP_SOLID);

    m_video_driver->setRenderTarget(m_rtts->getRTT(RTT_TMP1), false, false);
    glClearColor(0, 0, 0, 0);
    glClear(GL_STENCIL_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
    const u32 glowcount = glows.size();
    ColorizeProvider * const cb = (ColorizeProvider *) m_shaders->m_callbacks[ES_COLORIZE];

    glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
    glStencilFunc(GL_ALWAYS, 1, ~0);
    glEnable(GL_STENCIL_TEST);

	glEnable(GL_DEPTH_TEST);
	glDisable(GL_ALPHA_TEST);
	glDepthMask(GL_FALSE);
	glDisable(GL_BLEND);

    for (u32 i = 0; i < glowcount; i++)
    {
        const GlowData &dat = glows[i];
        scene::ISceneNode * const cur = dat.node;

        // Quick box-based culling
        const core::aabbox3df nodebox = cur->getTransformedBoundingBox();
        if (!nodebox.intersectsWithBox(cambox))
            continue;

        cb->setColor(dat.r, dat.g, dat.b);
        cur->render();
    }

    glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);
    glDisable(GL_STENCIL_TEST);

    // Cool, now we have the colors set up. Progressively minify.
    video::SMaterial minimat;
    minimat.Lighting = false;
    minimat.ZWriteEnable = false;
    minimat.ZBuffer = video::ECFN_ALWAYS;
    minimat.setFlag(video::EMF_TRILINEAR_FILTER, true);

    minimat.TextureLayer[0].TextureWrapU =
    minimat.TextureLayer[0].TextureWrapV = video::ETC_CLAMP_TO_EDGE;

    // To half
    m_video_driver->setRenderTarget(m_rtts->getRTT(RTT_HALF1), false, false);
	m_post_processing->renderPassThrough(m_rtts->getRTT(RTT_TMP1));

    // To quarter
    m_video_driver->setRenderTarget(m_rtts->getRTT(RTT_QUARTER1), false, false);
	m_post_processing->renderPassThrough(m_rtts->getRTT(RTT_HALF1));

    // Blur it
	m_post_processing->renderGaussian6Blur(m_rtts->getRTT(RTT_QUARTER1), m_rtts->getRTT(RTT_QUARTER2), 4.f / UserConfigParams::m_width, 4.f / UserConfigParams::m_height);

	glEnable(GL_BLEND);
	glBlendEquation(GL_FUNC_ADD);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glStencilFunc(GL_EQUAL, 0, ~0);
	glEnable(GL_STENCIL_TEST);
    m_video_driver->setRenderTarget(m_rtts->getRTT(RTT_COLOR), false, false);
	m_post_processing->renderGlow(m_rtts->getRTT(RTT_QUARTER1));
    glDisable(GL_STENCIL_TEST);
}

// ----------------------------------------------------------------------------
#define MAX2(a, b) ((a) > (b) ? (a) : (b))
#define MIN2(a, b) ((a) > (b) ? (b) : (a))
static LightShader::PointLightInfo PointLightsInfo[MAXLIGHT];

static void renderPointLights(unsigned count)
{
    glEnable(GL_BLEND);
    glBlendEquation(GL_FUNC_ADD);
    glBlendFunc(GL_ONE, GL_ONE);
    glEnable(GL_DEPTH_TEST);
    glDepthMask(GL_FALSE);

    glUseProgram(LightShader::PointLightShader::Program);
    glBindVertexArray(LightShader::PointLightShader::vao);
    glBindBuffer(GL_ARRAY_BUFFER, LightShader::PointLightShader::vbo);
    glBufferSubData(GL_ARRAY_BUFFER, 0, count * sizeof(LightShader::PointLightInfo), PointLightsInfo);

    setTexture(0, getTextureGLuint(irr_driver->getRTT(RTT_NORMAL_AND_DEPTH)), GL_NEAREST, GL_NEAREST);
    setTexture(1, getDepthTexture(irr_driver->getRTT(RTT_NORMAL_AND_DEPTH)), GL_NEAREST, GL_NEAREST);
    LightShader::PointLightShader::setUniforms(irr_driver->getViewMatrix(), irr_driver->getProjMatrix(), irr_driver->getInvProjMatrix(), core::vector2df(UserConfigParams::m_width, UserConfigParams::m_height), 200, 0, 1);

    glDrawArraysInstanced(GL_TRIANGLE_STRIP, 0, 4, count);
}

void IrrDriver::renderLights(const core::aabbox3df& cambox,
                             scene::ICameraSceneNode * const camnode,
                             video::SOverrideMaterial &overridemat,
                             int cam, float dt)
{
    if (SkyboxCubeMap)
        irr_driver->getSceneManager()->setAmbientLight(SColor(0, 0, 0, 0));
    for (unsigned i = 0; i < sun_ortho_matrix.size(); i++)
        sun_ortho_matrix[i] *= getInvViewMatrix();
    core::array<video::IRenderTarget> rtts;
    // Diffuse
    rtts.push_back(m_rtts->getRTT(RTT_TMP1));
    // Specular
    rtts.push_back(m_rtts->getRTT(RTT_TMP2));

    m_video_driver->setRenderTarget(rtts, true, false,
                                        video::SColor(0, 0, 0, 0));

    const u32 lightcount = m_lights.size();
    const core::vector3df &campos =
        irr_driver->getSceneManager()->getActiveCamera()->getAbsolutePosition();

    std::vector<LightNode *> BucketedLN[15];
    for (unsigned int i = 0; i < lightcount; i++)
    {
        if (!m_lights[i]->isPointLight())
        {
          m_lights[i]->render();
          if (UserConfigParams::m_shadows)
              m_post_processing->renderShadowedSunlight(sun_ortho_matrix, m_rtts->getShadowDepthTex());
          else
              m_post_processing->renderSunlight();
          continue;
        }
        const core::vector3df &lightpos = (m_lights[i]->getAbsolutePosition() - campos);
        unsigned idx = (unsigned)(lightpos.getLength() / 10);
        if (idx > 14)
          idx = 14;
        BucketedLN[idx].push_back(m_lights[i]);
    }

    unsigned lightnum = 0;

    for (unsigned i = 0; i < 15; i++)
    {
        for (unsigned j = 0; j < BucketedLN[i].size(); j++)
        {
            if (++lightnum >= MAXLIGHT)
            {
                LightNode* light_node = BucketedLN[i].at(j);
                light_node->setEnergyMultiplier(0.0f);
            }
            else
            {
                LightNode* light_node = BucketedLN[i].at(j);

                float em = light_node->getEnergyMultiplier();
                if (em < 1.0f)
                {
                    light_node->setEnergyMultiplier(std::min(1.0f, em + dt));
                }

                const core::vector3df &pos = light_node->getAbsolutePosition();
                PointLightsInfo[lightnum].posX = pos.X;
                PointLightsInfo[lightnum].posY = pos.Y;
                PointLightsInfo[lightnum].posZ = pos.Z;

                PointLightsInfo[lightnum].energy = light_node->getEffectiveEnergy();

                const core::vector3df &col = light_node->getColor();
                PointLightsInfo[lightnum].red = col.X;
                PointLightsInfo[lightnum].green = col.Y;
                PointLightsInfo[lightnum].blue = col.Z;
            }
        }
        if (lightnum > MAXLIGHT)
        {
          irr_driver->setLastLightBucketDistance(i * 10);
          break;
        }
    }

    lightnum++;

    renderPointLights(MIN2(lightnum, MAXLIGHT));
    if (SkyboxCubeMap)
        m_post_processing->renderDiffuseEnvMap(blueSHCoeff, greenSHCoeff, redSHCoeff);
    // Handle SSAO
    m_video_driver->setRenderTarget(irr_driver->getRTT(RTT_SSAO), true, false,
                         SColor(255, 255, 255, 255));

    if(UserConfigParams::m_ssao)
        m_post_processing->renderSSAO(irr_driver->getInvProjMatrix(), irr_driver->getProjMatrix());

    // Blur it to reduce noise.
    if(UserConfigParams::m_ssao == 1)
		m_post_processing->renderGaussian3Blur(irr_driver->getRTT(RTT_SSAO), irr_driver->getRTT(RTT_QUARTER4), 4.f / UserConfigParams::m_width, 4.f / UserConfigParams::m_height);
    else if (UserConfigParams::m_ssao == 2)
		m_post_processing->renderGaussian6Blur(irr_driver->getRTT(RTT_SSAO), irr_driver->getRTT(RTT_TMP4), 1.f / UserConfigParams::m_width, 1.f / UserConfigParams::m_height);

    m_video_driver->setRenderTarget(m_rtts->getRTT(RTT_COLOR), false, false);
}

static void getXYZ(GLenum face, float i, float j, float &x, float &y, float &z)
{
    switch (face)
    {
    case GL_TEXTURE_CUBE_MAP_POSITIVE_X:
        x = 1.;
        y = -i;
        z = -j;
        break;
    case GL_TEXTURE_CUBE_MAP_NEGATIVE_X:
        x = -1.;
        y = -i;
        z = j;
        break;
    case GL_TEXTURE_CUBE_MAP_POSITIVE_Y:
        x = j;
        y = 1.;
        z = i;
        break;
    case GL_TEXTURE_CUBE_MAP_NEGATIVE_Y:
        x = j;
        y = -1;
        z = -i;
        break;
    case GL_TEXTURE_CUBE_MAP_POSITIVE_Z:
        x = j;
        y = -i;
        z = 1;
        break;
    case GL_TEXTURE_CUBE_MAP_NEGATIVE_Z:
        x = -j;
        y = -i;
        z = -1;
        break;
    }

    float norm = sqrt(x * x + y * y + z * z);
    x /= norm, y /= norm, z /= norm;
    return;
}

static void getYml(GLenum face, size_t width, size_t height,
    float *Y00,
    float *Y1minus1, float *Y10, float *Y11,
    float *Y2minus2, float *Y2minus1, float *Y20, float *Y21, float *Y22)
{
    for (unsigned i = 0; i < width; i++)
    {
        for (unsigned j = 0; j < height; j++)
        {
            float x, y, z;
            float fi = i, fj = j;
            fi /= width, fj /= height;
            fi = 2 * fi - 1, fj = 2 * fj - 1;
            getXYZ(face, fi, fj, x, y, z);

            // constant part of Ylm
            float c00 = 0.282095;
            float c1minus1 = 0.488603;
            float c10 = 0.488603;
            float c11 = 0.488603;
            float c2minus2 = 1.092548;
            float c2minus1 = 1.092548;
            float c21 = 1.092548;
            float c20 = 0.315392;
            float c22 = 0.546274;

            size_t idx = i * height + j;

            Y00[idx] = c00;
            Y1minus1[idx] = c1minus1 * y;
            Y10[idx] = c10 * z;
            Y11[idx] = c11 * x;
            Y2minus2[idx] = c2minus2 * x * y;
            Y2minus1[idx] = c2minus1 * y * z;
            Y21[idx] = c21 * x * z;
            Y20[idx] = c20 * (3 * z * z - 1);
            Y22[idx] = c22 * (x * x - y * y);
        }
    }
}

static float getTexelValue(unsigned i, unsigned j, size_t width, size_t height, float *Coeff, float *Y00, float *Y1minus1, float *Y10, float *Y11,
    float *Y2minus2, float * Y2minus1, float * Y20, float *Y21, float *Y22)
{
    float d = sqrt((float)(i * i + j * j + 1));
    float solidangle = 1.;
    size_t idx = i * height + j;
    float reconstructedVal = Y00[idx] * Coeff[0];
    reconstructedVal += Y1minus1[i * height + j] * Coeff[1] + Y10[i * height + j] * Coeff[2] +  Y11[i * height + j] * Coeff[3];
    reconstructedVal += Y2minus2[idx] * Coeff[4] + Y2minus1[idx] * Coeff[5] + Y20[idx] * Coeff[6] + Y21[idx] * Coeff[7] + Y22[idx] * Coeff[8];
    reconstructedVal /= solidangle;
    return MAX2(255 * reconstructedVal, 0.);
}

static void unprojectSH(float *output[], size_t width, size_t height,
    float *Y00[],
    float *Y1minus1[], float *Y10[], float *Y11[],
    float *Y2minus2[], float *Y2minus1[], float * Y20[],float *Y21[], float *Y22[],
    float *blueSHCoeff, float *greenSHCoeff, float *redSHCoeff)
{
    for (unsigned face = 0; face < 6; face++)
    {
        for (unsigned i = 0; i < width; i++)
        {
            for (unsigned j = 0; j < height; j++)
            {
                float fi = i, fj = j;
                fi /= width, fj /= height;
                fi = 2 * fi - 1, fj = 2 * fj - 1;

                output[face][4 * height * i + 4 * j + 2] = getTexelValue(i, j, width, height,
                    redSHCoeff,
                    Y00[face], Y1minus1[face], Y10[face], Y11[face], Y2minus2[face], Y2minus1[face], Y20[face], Y21[face], Y22[face]);
                output[face][4 * height * i + 4 * j + 1] = getTexelValue(i, j, width, height,
                    greenSHCoeff,
                    Y00[face], Y1minus1[face], Y10[face], Y11[face], Y2minus2[face], Y2minus1[face], Y20[face], Y21[face], Y22[face]);
                output[face][4 * height * i + 4 * j] = getTexelValue(i, j, width, height,
                    blueSHCoeff,
                    Y00[face], Y1minus1[face], Y10[face], Y11[face], Y2minus2[face], Y2minus1[face], Y20[face], Y21[face], Y22[face]);
            }
        }
    }
}

static void projectSH(float *color[], size_t width, size_t height,
    float *Y00[],
    float *Y1minus1[], float *Y10[], float *Y11[],
    float *Y2minus2[], float *Y2minus1[], float * Y20[], float *Y21[], float *Y22[],
    float *blueSHCoeff, float *greenSHCoeff, float *redSHCoeff
    )
{
    for (unsigned i = 0; i < 9; i++)
    {
        blueSHCoeff[i] = 0;
        greenSHCoeff[i] = 0;
        redSHCoeff[i] = 0;
    }
    float wh = width * height;
    for (unsigned face = 0; face < 6; face++)
    {
        for (unsigned i = 0; i < width; i++)
        {
            for (unsigned j = 0; j < height; j++)
            {
                size_t idx = i * height + j;
                float fi = i, fj = j;
                fi /= width, fj /= height;
                fi = 2 * fi - 1, fj = 2 * fj - 1;


                float d = sqrt(fi * fi + fj * fj + 1);

                // Constant obtained by projecting unprojected ref values
                float solidangle = 2.75 / (wh * pow(d, 1.5f));
                float b = color[face][4 * height * i + 4 * j] / 255.;
                float g = color[face][4 * height * i + 4 * j + 1] / 255.;
                float r = color[face][4 * height * i + 4 * j + 2] / 255.;

                assert(b >= 0.);

                blueSHCoeff[0] += b * Y00[face][idx] * solidangle;
                blueSHCoeff[1] += b * Y1minus1[face][idx] * solidangle;
                blueSHCoeff[2] += b * Y10[face][idx] * solidangle;
                blueSHCoeff[3] += b * Y11[face][idx] * solidangle;
                blueSHCoeff[4] += b * Y2minus2[face][idx] * solidangle;
                blueSHCoeff[5] += b * Y2minus1[face][idx] * solidangle;
                blueSHCoeff[6] += b * Y20[face][idx] * solidangle;
                blueSHCoeff[7] += b * Y21[face][idx] * solidangle;
                blueSHCoeff[8] += b * Y22[face][idx] * solidangle;

                greenSHCoeff[0] += g * Y00[face][idx] * solidangle;
                greenSHCoeff[1] += g * Y1minus1[face][idx] * solidangle;
                greenSHCoeff[2] += g * Y10[face][idx] * solidangle;
                greenSHCoeff[3] += g * Y11[face][idx] * solidangle;
                greenSHCoeff[4] += g * Y2minus2[face][idx] * solidangle;
                greenSHCoeff[5] += g * Y2minus1[face][idx] * solidangle;
                greenSHCoeff[6] += g * Y20[face][idx] * solidangle;
                greenSHCoeff[7] += g * Y21[face][idx] * solidangle;
                greenSHCoeff[8] += g * Y22[face][idx] * solidangle;


                redSHCoeff[0] += r * Y00[face][idx] * solidangle;
                redSHCoeff[1] += r * Y1minus1[face][idx] * solidangle;
                redSHCoeff[2] += r * Y10[face][idx] * solidangle;
                redSHCoeff[3] += r * Y11[face][idx] * solidangle;
                redSHCoeff[4] += r * Y2minus2[face][idx] * solidangle;
                redSHCoeff[5] += r * Y2minus1[face][idx] * solidangle;
                redSHCoeff[6] += r * Y20[face][idx] * solidangle;
                redSHCoeff[7] += r * Y21[face][idx] * solidangle;
                redSHCoeff[8] += r * Y22[face][idx] * solidangle;
            }
        }
    }
}

static void displayCoeff(float *SHCoeff)
{
    printf("L00:%f\n", SHCoeff[0]);
    printf("L1-1:%f, L10:%f, L11:%f\n", SHCoeff[1], SHCoeff[2], SHCoeff[3]);
    printf("L2-2:%f, L2-1:%f, L20:%f, L21:%f, L22:%f\n", SHCoeff[4], SHCoeff[5], SHCoeff[6], SHCoeff[7], SHCoeff[8]);
}

// Only for 9 coefficients
static void testSH(char *color[6], size_t width, size_t height,
    float *blueSHCoeff, float *greenSHCoeff, float *redSHCoeff)
{
    float *Y00[6];
    float *Y1minus1[6];
    float *Y10[6];
    float *Y11[6];
    float *Y2minus2[6];
    float *Y2minus1[6];
    float *Y20[6];
    float *Y21[6];
    float *Y22[6];

    float *testoutput[6];
    for (unsigned i = 0; i < 6; i++)
    {
        testoutput[i] = new float[width * height * 4];
        for (unsigned j = 0; j < width * height; j++)
        {
            testoutput[i][4 * j] = 0xFF & color[i][4 * j];
            testoutput[i][4 * j + 1] = 0xFF & color[i][4 * j + 1];
            testoutput[i][4 * j + 2] = 0xFF & color[i][4 * j + 2];
        }
    }

    for (unsigned face = 0; face < 6; face++)
    {
        Y00[face] = new float[width * height];
        Y1minus1[face] = new float[width * height];
        Y10[face] = new float[width * height];
        Y11[face] = new float[width * height];
        Y2minus2[face] = new float[width * height];
        Y2minus1[face] = new float[width * height];
        Y20[face] = new float[width * height];
        Y21[face] = new float[width * height];
        Y22[face] = new float[width * height];

        getYml(GL_TEXTURE_CUBE_MAP_POSITIVE_X + face, width, height, Y00[face], Y1minus1[face], Y10[face], Y11[face], Y2minus2[face], Y2minus1[face], Y20[face], Y21[face], Y22[face]);
    }

/*    blueSHCoeff[0] = 0.54,
    blueSHCoeff[1] = .6, blueSHCoeff[2] = -.27, blueSHCoeff[3] = .01,
    blueSHCoeff[4] = -.12, blueSHCoeff[5] = -.47, blueSHCoeff[6] = -.15, blueSHCoeff[7] = .14, blueSHCoeff[8] = -.3;
    greenSHCoeff[0] = .44,
    greenSHCoeff[1] = .35, greenSHCoeff[2] = -.18, greenSHCoeff[3] = -.06,
    greenSHCoeff[4] = -.05, greenSHCoeff[5] = -.22, greenSHCoeff[6] = -.09, greenSHCoeff[7] = .21, greenSHCoeff[8] = -.05;
    redSHCoeff[0] = .79,
    redSHCoeff[1] = .39, redSHCoeff[2] = -.34, redSHCoeff[3] = -.29,
    redSHCoeff[4] = -.11, redSHCoeff[5] = -.26, redSHCoeff[6] = -.16, redSHCoeff[7] = .56, redSHCoeff[8] = .21;

    printf("Blue:\n");
    displayCoeff(blueSHCoeff);
    printf("Green:\n");
    displayCoeff(greenSHCoeff);
    printf("Red:\n");
    displayCoeff(redSHCoeff);*/

    projectSH(testoutput, width, height,
        Y00,
        Y1minus1, Y10, Y11,
        Y2minus2, Y2minus1, Y20, Y21, Y22,
        blueSHCoeff, greenSHCoeff, redSHCoeff
        );

    printf("Blue:\n");
    displayCoeff(blueSHCoeff);
    printf("Green:\n");
    displayCoeff(greenSHCoeff);
    printf("Red:\n");
    displayCoeff(redSHCoeff);



    // Convolute in frequency space
/*    float A0 = 3.141593;
    float A1 = 2.094395;
    float A2 = 0.785398;
    blueSHCoeff[0] *= A0;
    greenSHCoeff[0] *= A0;
    redSHCoeff[0] *= A0;
    for (unsigned i = 0; i < 3; i++)
    {
        blueSHCoeff[1 + i] *= A1;
        greenSHCoeff[1 + i] *= A1;
        redSHCoeff[1 + i] *= A1;
    }
    for (unsigned i = 0; i < 5; i++)
    {
        blueSHCoeff[4 + i] *= A2;
        greenSHCoeff[4 + i] *= A2;
        redSHCoeff[4 + i] *= A2;
    }


    unprojectSH(testoutput, width, height,
        Y00,
        Y1minus1, Y10, Y11,
        Y2minus2, Y2minus1, Y20, Y21, Y22,
        blueSHCoeff, greenSHCoeff, redSHCoeff
        );*/


/*    printf("Blue:\n");
    displayCoeff(blueSHCoeff);
    printf("Green:\n");
    displayCoeff(greenSHCoeff);
    printf("Red:\n");
    displayCoeff(redSHCoeff);

    printf("\nAfter projection\n\n");*/



    for (unsigned i = 0; i < 6; i++)
    {
        for (unsigned j = 0; j < width * height; j++)
        {
            color[i][4 * j] = MIN2(testoutput[i][4 * j], 255);
            color[i][4 * j + 1] = MIN2(testoutput[i][4 * j + 1], 255);
            color[i][4 * j + 2] = MIN2(testoutput[i][4 * j + 2], 255);
        }
    }

    for (unsigned face = 0; face < 6; face++)
    {
        delete[] testoutput[face];
        delete[] Y00[face];
        delete[] Y1minus1[face];
        delete[] Y10[face];
        delete[] Y11[face];
    }
}

void IrrDriver::generateSkyboxCubemap()
{
    glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);

    glGenTextures(1, &SkyboxCubeMap);
    glGenTextures(1, &ConvolutedSkyboxCubeMap);

    GLint w = 0, h = 0;
    for (unsigned i = 0; i < 6; i++)
    {
        w = MAX2(w, SkyboxTextures[i]->getOriginalSize().Width);
        h = MAX2(h, SkyboxTextures[i]->getOriginalSize().Height);
    }

    const unsigned texture_permutation[] = { 2, 3, 0, 1, 5, 4 };
    char *rgba[6];
    for (unsigned i = 0; i < 6; i++)
        rgba[i] = new char[w * h * 4];
    for (unsigned i = 0; i < 6; i++)
    {
        unsigned idx = texture_permutation[i];

        video::IImage* image = getVideoDriver()->createImageFromData(
            SkyboxTextures[idx]->getColorFormat(),
            SkyboxTextures[idx]->getSize(),
            SkyboxTextures[idx]->lock(),
            false
            );
        SkyboxTextures[idx]->unlock();

        image->copyToScaling(rgba[i], w, h);
        image->drop();

        glBindTexture(GL_TEXTURE_CUBE_MAP, SkyboxCubeMap);
        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB, w, h, 0, GL_BGRA, GL_UNSIGNED_BYTE, (GLvoid*)rgba[i]);
    }

    testSH(rgba, w, h, blueSHCoeff, greenSHCoeff, redSHCoeff);

    for (unsigned i = 0; i < 6; i++)
    {
        glBindTexture(GL_TEXTURE_CUBE_MAP, ConvolutedSkyboxCubeMap);
        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB, w, h, 0, GL_BGRA, GL_UNSIGNED_BYTE, (GLvoid*)rgba[i]);
    }
    for (unsigned i = 0; i < 6; i++)
        delete[] rgba[i];
    glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
}


void IrrDriver::renderSkybox()
{
    if (SkyboxTextures.empty()) return;

    scene::ICameraSceneNode *camera = m_scene_manager->getActiveCamera();
    if (!SkyboxCubeMap)
        generateSkyboxCubemap();
    glBindVertexArray(MeshShader::SkyboxShader::cubevao);
	glDisable(GL_CULL_FACE);
	assert(SkyboxTextures.size() == 6);
	core::matrix4 transform = irr_driver->getProjViewMatrix();
	core::matrix4 translate;
	translate.setTranslation(camera->getAbsolutePosition());

	// Draw the sky box between the near and far clip plane
	const f32 viewDistance = (camera->getNearValue() + camera->getFarValue()) * 0.5f;
	core::matrix4 scale;
	scale.setScale(core::vector3df(viewDistance, viewDistance, viewDistance));
	transform *= translate * scale;
    core::matrix4 invtransform;
    transform.getInverse(invtransform);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_CUBE_MAP, ConvolutedSkyboxCubeMap);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glUseProgram(MeshShader::SkyboxShader::Program);
        MeshShader::SkyboxShader::setUniforms(transform, invtransform, core::vector2df(UserConfigParams::m_width, UserConfigParams::m_height), 0);
    glDrawElements(GL_TRIANGLES, 6 * 6, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
}

// ----------------------------------------------------------------------------

void IrrDriver::renderDisplacement(video::SOverrideMaterial &overridemat,
                                   int cam)
{
    irr_driver->getVideoDriver()->setRenderTarget(irr_driver->getRTT(RTT_TMP4), true, false);
    irr_driver->getVideoDriver()->setRenderTarget(irr_driver->getRTT(RTT_DISPLACE), true, false);

	DisplaceProvider * const cb = (DisplaceProvider *)irr_driver->getCallback(ES_DISPLACE);
	cb->update();

    const int displacingcount = m_displacing.size();
	irr_driver->setPhase(DISPLACEMENT_PASS);
	glEnable(GL_DEPTH_TEST);
	glDisable(GL_ALPHA_TEST);
	glDepthMask(GL_FALSE);
	glDisable(GL_BLEND);
    glClear(GL_STENCIL_BUFFER_BIT);
    glEnable(GL_STENCIL_TEST);
    glStencilFunc(GL_ALWAYS, 1, 0xFF);
    glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);

    for (int i = 0; i < displacingcount; i++)
    {
        m_scene_manager->setCurrentRendertime(scene::ESNRP_SOLID);
        m_displacing[i]->render();

        m_scene_manager->setCurrentRendertime(scene::ESNRP_TRANSPARENT);
        m_displacing[i]->render();
    }

    m_video_driver->setRenderTarget(m_rtts->getRTT(RTT_COLOR), false, false);
    glStencilFunc(GL_EQUAL, 1, 0xFF);
    m_post_processing->renderPassThrough(m_rtts->getRTT(RTT_DISPLACE));
    glDisable(GL_STENCIL_TEST);
}
