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

#include "graphics/shader_based_renderer.hpp"

#include "graphics/central_settings.hpp"
#include "graphics/glwrap.hpp"
#include "graphics/graphics_restrictions.hpp"
#include "graphics/lod_node.hpp"
#include "graphics/post_processing.hpp"
#include "graphics/rtts.hpp"
#include "graphics/shaders.hpp"
#include "graphics/stk_scene_manager.hpp"
#include "graphics/texture_manager.hpp"
#include "items/item_manager.hpp"
#include "items/powerup_manager.hpp"
#include "modes/world.hpp"
#include "physics/physics.hpp"
#include "states_screens/race_gui_base.hpp"
#include "tracks/track.hpp"
#include "utils/profiler.hpp"

#include <algorithm> 

extern std::vector<float> BoundingBoxes; //TODO


void ShaderBasedRenderer::compressPowerUpTextures()
{
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
}

void ShaderBasedRenderer::setOverrideMaterial()
{
    // Overrides
    video::SOverrideMaterial &overridemat = irr_driver->getVideoDriver()->getOverrideMaterial();
    overridemat.EnablePasses = scene::ESNRP_SOLID | scene::ESNRP_TRANSPARENT;
    overridemat.EnableFlags = 0;

    if (irr_driver->getWireframe())
    {
        overridemat.Material.Wireframe = 1;
        overridemat.EnableFlags |= video::EMF_WIREFRAME;
    }
    if (irr_driver->getMipViz())
    {
        overridemat.Material.MaterialType = Shaders::getShader(ES_MIPVIZ);
        overridemat.EnableFlags |= video::EMF_MATERIAL_TYPE;
        overridemat.EnablePasses = scene::ESNRP_SOLID;
    }       
}

//Add glowing items, they may appear or disappear each frame.
void ShaderBasedRenderer::addItemsInGlowingList()
{
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

        STKMeshSceneNode *stk_node = static_cast<STKMeshSceneNode *>(node);
        stk_node->setGlowColors(irr::video::SColor(0, (unsigned) (dat.b * 255.f), (unsigned)(dat.g * 255.f), (unsigned)(dat.r * 255.f)));

        m_glowing.push_back(dat);
    }    
}

//Remove all non static glowing things
void ShaderBasedRenderer::removeItemsInGlowingList()
{
    while(m_glowing.size() > m_nb_static_glowing)
        m_glowing.pop_back();    
}


void ShaderBasedRenderer::prepareForwardRenderer()
{
    irr::video::SColor clearColor(0, 150, 150, 150);
    if (World::getWorld() != NULL)
        clearColor = World::getWorld()->getClearColor();

    glClear(GL_COLOR_BUFFER_BIT);
    glDepthMask(GL_TRUE);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glClearColor(clearColor.getRed() / 255.f, clearColor.getGreen() / 255.f,
        clearColor.getBlue() / 255.f, clearColor.getAlpha() / 255.f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);    
}

// ----------------------------------------------------------------------------
void ShaderBasedRenderer::updateLightsInfo(scene::ICameraSceneNode * const camnode,
                                           float dt)
{
    m_lighting_passes.updateLightsInfo(camnode, dt);
}

// ----------------------------------------------------------------------------
/** Upload lighting info to the dedicated uniform buffer
 */
void ShaderBasedRenderer::uploadLightingData() const
{
    float Lighting[36];
    
    core::vector3df sun_direction = irr_driver->getSunDirection();
    video::SColorf sun_color = irr_driver->getSunColor();

    Lighting[0] = sun_direction.X;
    Lighting[1] = sun_direction.Y;
    Lighting[2] = sun_direction.Z;
    Lighting[4] = sun_color.getRed();
    Lighting[5] = sun_color.getGreen();
    Lighting[6] = sun_color.getBlue();
    Lighting[7] = 0.54f;

    const SHCoefficients* sh_coeff = irr_driver->getSHCoefficients();

    if(sh_coeff) {
        memcpy(&Lighting[8], sh_coeff->blue_SH_coeff, 9 * sizeof(float));
        memcpy(&Lighting[17], sh_coeff->green_SH_coeff, 9 * sizeof(float));
        memcpy(&Lighting[26], sh_coeff->red_SH_coeff, 9 * sizeof(float));
    }

    glBindBuffer(GL_UNIFORM_BUFFER, SharedGPUObjects::getLightingDataUBO());
    glBufferSubData(GL_UNIFORM_BUFFER, 0, 36 * sizeof(float), Lighting);
}   // uploadLightingData


// ----------------------------------------------------------------------------
void ShaderBasedRenderer::computeMatrixesAndCameras(scene::ICameraSceneNode *const camnode,
                                                    size_t width, size_t height)
{
    m_current_screen_size = core::vector2df(float(width), float(height));
    m_shadow_matrices.computeMatrixesAndCameras(camnode, width, height, m_rtts->getDepthStencilTexture());
}   // computeMatrixesAndCameras

// ----------------------------------------------------------------------------
void ShaderBasedRenderer::renderSkybox(const scene::ICameraSceneNode *camera) const
{
    if(m_skybox)
    {
        m_skybox->render(camera);
    }
}   // renderSkybox

// ============================================================================
void ShaderBasedRenderer::renderSSAO() const
{
    m_rtts->getFBO(FBO_SSAO).bind();
    glClearColor(1., 1., 1., 1.);
    glClear(GL_COLOR_BUFFER_BIT);
    irr_driver->getPostProcessing()->renderSSAO();
    // Blur it to reduce noise.
    FrameBuffer::Blit(m_rtts->getFBO(FBO_SSAO),
                      m_rtts->getFBO(FBO_HALF1_R), 
                      GL_COLOR_BUFFER_BIT, GL_LINEAR);
    irr_driver->getPostProcessing()->renderGaussian17TapBlur(m_rtts->getFBO(FBO_HALF1_R), 
                                                             m_rtts->getFBO(FBO_HALF2_R));

}   // renderSSAO

// ============================================================================
void ShaderBasedRenderer::renderScene(scene::ICameraSceneNode * const camnode,
                                      float dt,
                                      bool hasShadow,
                                      bool forceRTT)
{
    PostProcessing *post_processing = irr_driver->getPostProcessing();
    m_wind_dir = getWindDir(); //TODO
    
    glBindBufferBase(GL_UNIFORM_BUFFER, 0, SharedGPUObjects::getViewProjectionMatricesUBO());
    glBindBufferBase(GL_UNIFORM_BUFFER, 1, SharedGPUObjects::getLightingDataUBO());
    irr_driver->getSceneManager()->setActiveCamera(camnode);

    PROFILER_PUSH_CPU_MARKER("- Draw Call Generation", 0xFF, 0xFF, 0xFF);
    m_draw_calls.prepareDrawCalls(m_shadow_matrices, camnode);
    PROFILER_POP_CPU_MARKER();
    // Shadows
    {
        // To avoid wrong culling, use the largest view possible
        irr_driver->getSceneManager()->setActiveCamera(m_shadow_matrices.getSunCam());
        if (CVS->isDefferedEnabled() &&
            CVS->isShadowEnabled() && hasShadow)
        {
            PROFILER_PUSH_CPU_MARKER("- Shadow", 0x30, 0x6F, 0x90);
            m_geometry_passes->renderShadows(m_draw_calls,
                                             m_shadow_matrices,
                                             m_rtts->getShadowFrameBuffer());
            PROFILER_POP_CPU_MARKER();
            if (CVS->isGlobalIlluminationEnabled())
            {
                if (!m_shadow_matrices.isRSMMapAvail())
                {
                    PROFILER_PUSH_CPU_MARKER("- RSM", 0xFF, 0x0, 0xFF);
                    m_geometry_passes->renderReflectiveShadowMap(m_draw_calls,
                                                                 m_shadow_matrices, 
                                                                 m_rtts->getReflectiveShadowMapFrameBuffer()); //TODO: move somewhere else as RSM are computed only once per track
                    m_shadow_matrices.setRSMMapAvail(true);
                    PROFILER_POP_CPU_MARKER();
                }
            }
        }
        irr_driver->getSceneManager()->setActiveCamera(camnode);

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
        m_geometry_passes->renderSolidFirstPass(m_draw_calls);
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
            if (CVS->isGlobalIlluminationEnabled() && hasShadow)
            {    
                m_lighting_passes.renderGlobalIllumination( m_shadow_matrices,
                                                            m_rtts->getRadianceHintFrameBuffer(),
                                                            m_rtts->getReflectiveShadowMapFrameBuffer(),
                                                            m_rtts->getFBO(FBO_DIFFUSE));
            }            
            
            GLuint specular_probe;
            if(m_skybox)
            {
                specular_probe = m_skybox->getSpecularProbe();
            }
            else 
            {
                specular_probe = 0;
            }              
            
            m_lighting_passes.renderLights( hasShadow,
                                            m_rtts->getRenderTarget(RTT_NORMAL_AND_DEPTH),
                                            m_rtts->getDepthStencilTexture(),
                                            m_rtts->getShadowFrameBuffer(),
                                            m_rtts->getFBO(FBO_COMBINED_DIFFUSE_SPECULAR),
                                            specular_probe);
        PROFILER_POP_CPU_MARKER();
    }

    // Handle SSAO
    {
        PROFILER_PUSH_CPU_MARKER("- SSAO", 0xFF, 0xFF, 0x00);
        ScopedGPUTimer Timer(irr_driver->getGPUTimer(Q_SSAO));
        if (UserConfigParams::m_ssao)
            renderSSAO();
        PROFILER_POP_CPU_MARKER();
    }

    PROFILER_PUSH_CPU_MARKER("- Solid Pass 2", 0x00, 0x00, 0xFF);
    if (CVS->isDefferedEnabled() || forceRTT)
    {
        m_rtts->getFBO(FBO_COLORS).bind();
        video::SColor clearColor(0, 150, 150, 150);
        if (World::getWorld() != NULL)
            clearColor = World::getWorld()->getClearColor();

        glClearColor(clearColor.getRed() / 255.f, clearColor.getGreen() / 255.f,
            clearColor.getBlue() / 255.f, clearColor.getAlpha() / 255.f);
        glClear(GL_COLOR_BUFFER_BIT);
        glDepthMask(GL_FALSE);
    }
    
        std::vector<GLuint> prefilled_textures =
            createVector<GLuint>(m_rtts->getRenderTarget(RTT_DIFFUSE),
                                 m_rtts->getRenderTarget(RTT_SPECULAR),
                                 m_rtts->getRenderTarget(RTT_HALF1_R),
                                 m_rtts->getDepthStencilTexture());
        m_geometry_passes->setFirstPassRenderTargets(prefilled_textures);
        //TODO: no need to update it every frame
    
    m_geometry_passes->renderSolidSecondPass(m_draw_calls);
    PROFILER_POP_CPU_MARKER();

    if (irr_driver->getNormals())
    {
        m_rtts->getFBO(FBO_NORMAL_AND_DEPTHS).bind();
        m_geometry_passes->renderNormalsVisualisation(m_draw_calls);
        m_rtts->getFBO(FBO_COLORS).bind();
    }

    // Render ambient scattering
    if (CVS->isDefferedEnabled() && World::getWorld() != NULL &&
        World::getWorld()->isFogEnabled())
    {
        PROFILER_PUSH_CPU_MARKER("- Ambient scatter", 0xFF, 0x00, 0x00);
        ScopedGPUTimer Timer(irr_driver->getGPUTimer(Q_FOG));
        m_lighting_passes.renderAmbientScatter(m_rtts->getDepthStencilTexture());
        PROFILER_POP_CPU_MARKER();
    }

    {
        PROFILER_PUSH_CPU_MARKER("- Skybox", 0xFF, 0x00, 0xFF);
        ScopedGPUTimer Timer(irr_driver->getGPUTimer(Q_SKYBOX));
        renderSkybox(camnode);
        PROFILER_POP_CPU_MARKER();
    }

    // Render discrete lights scattering
    if (CVS->isDefferedEnabled() && World::getWorld() != NULL &&
        World::getWorld()->isFogEnabled())
    {
        PROFILER_PUSH_CPU_MARKER("- PointLight Scatter", 0xFF, 0x00, 0x00);
        ScopedGPUTimer Timer(irr_driver->getGPUTimer(Q_FOG));
        m_lighting_passes.renderLightsScatter(m_rtts->getFBO(FBO_HALF1),
                                              m_rtts->getFBO(FBO_HALF2),
                                              m_rtts->getFBO(FBO_COLORS),
                                              m_rtts->getRenderTarget(RTT_HALF1));
        PROFILER_POP_CPU_MARKER();
    }

    if (irr_driver->getRH())
    {
        glDisable(GL_BLEND);
        m_rtts->getFBO(FBO_COLORS).bind();
        post_processing->renderRHDebug(m_rtts->getRadianceHintFrameBuffer().getRTT()[0],
                                       m_rtts->getRadianceHintFrameBuffer().getRTT()[1], 
                                       m_rtts->getRadianceHintFrameBuffer().getRTT()[2],
                                       m_shadow_matrices.getRHMatrix(),
                                       m_shadow_matrices.getRHExtend());
    }

    if (irr_driver->getGI())
    {
        glDisable(GL_BLEND);
        m_rtts->getFBO(FBO_COLORS).bind();
        post_processing->renderGI(m_shadow_matrices.getRHMatrix(),
                                  m_shadow_matrices.getRHExtend(),
                                  m_rtts->getRadianceHintFrameBuffer());
    }

    PROFILER_PUSH_CPU_MARKER("- Glow", 0xFF, 0xFF, 0x00);
    // Render anything glowing.
    if (!irr_driver->getWireframe() && !irr_driver->getMipViz() && UserConfigParams::m_glow)
    {
        ScopedGPUTimer Timer(irr_driver->getGPUTimer(Q_GLOW));
        irr_driver->setPhase(GLOW_PASS);
        m_geometry_passes->renderGlow(m_draw_calls, m_glowing,
                                      m_rtts->getFBO(FBO_TMP1_WITH_DS),
                                      m_rtts->getFBO(FBO_HALF1),
                                      m_rtts->getFBO(FBO_QUARTER1),
                                      m_rtts->getFBO(FBO_COLORS),
                                      m_rtts->getRenderTarget(RTT_QUARTER1));
    } // end glow
    PROFILER_POP_CPU_MARKER();

    // Render transparent
    {
        PROFILER_PUSH_CPU_MARKER("- Transparent Pass", 0xFF, 0x00, 0x00);
        ScopedGPUTimer Timer(irr_driver->getGPUTimer(Q_TRANSPARENT));
        m_geometry_passes->renderTransparent(m_draw_calls,
                                             m_rtts->getRenderTarget(RTT_DISPLACE));
        PROFILER_POP_CPU_MARKER();
    }

    m_draw_calls.m_sync = glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE, 0);

    // Render particles
    {
        PROFILER_PUSH_CPU_MARKER("- Particles", 0xFF, 0xFF, 0x00);
        ScopedGPUTimer Timer(irr_driver->getGPUTimer(Q_PARTICLES));
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

void ShaderBasedRenderer::renderParticles()
{
    glDepthMask(GL_FALSE);
    glDisable(GL_CULL_FACE);
    glEnable(GL_BLEND);
    glBlendEquation(GL_FUNC_ADD);
    m_draw_calls.renderParticlesList();

//    m_scene_manager->drawAll(scene::ESNRP_TRANSPARENT_EFFECT);
}


void ShaderBasedRenderer::renderBoundingBoxes()
{
    Shaders::ColoredLine *line = Shaders::ColoredLine::getInstance();
    line->use();
    line->bindVertexArray();
    line->bindBuffer();
    line->setUniforms(irr::video::SColor(255, 255, 0, 0));
    const float *tmp = BoundingBoxes.data();
    for (unsigned int i = 0; i < BoundingBoxes.size(); i += 1024 * 6)
    {
        unsigned count = std::min((unsigned)BoundingBoxes.size() - i, (unsigned)1024 * 6);
        glBufferSubData(GL_ARRAY_BUFFER, 0, count * sizeof(float), &tmp[i]);

        glDrawArrays(GL_LINES, 0, count / 3);
    }
}

void ShaderBasedRenderer::debugPhysics()
{
    // Note that drawAll must be called before rendering
    // the bullet debug view, since otherwise the camera
    // is not set up properly. This is only used for
    // the bullet debug view.
    World *world = World::getWorld();
    if (UserConfigParams::m_artist_debug_mode)
        world->getPhysics()->draw();
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
                    unsigned count = std::min((unsigned)vertex.size() - i, (unsigned)1024 * 6);
                    glBufferSubData(GL_ARRAY_BUFFER, 0, count * sizeof(float), &tmp[i]);

                    glDrawArrays(GL_LINES, 0, count / 3);
                }
            }
            glUseProgram(0);
            glBindVertexArray(0);
        }
    }
}

void ShaderBasedRenderer::renderPostProcessing(Camera * const camera)
{
    scene::ICameraSceneNode * const camnode = camera->getCameraSceneNode();
    const core::recti &viewport = camera->getViewport();
    
    bool isRace = StateManager::get()->getGameState() == GUIEngine::GAME;
    PostProcessing * post_processing = irr_driver->getPostProcessing();
    FrameBuffer *fbo = post_processing->render(camnode, isRace);

    if (irr_driver->getNormals())
        irr_driver->getFBO(FBO_NORMAL_AND_DEPTHS).BlitToDefault(viewport.UpperLeftCorner.X, viewport.UpperLeftCorner.Y, viewport.LowerRightCorner.X, viewport.LowerRightCorner.Y);
    else if (irr_driver->getSSAOViz())
    {
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glViewport(viewport.UpperLeftCorner.X, viewport.UpperLeftCorner.Y, viewport.LowerRightCorner.X, viewport.LowerRightCorner.Y);
        post_processing->renderPassThrough(m_rtts->getFBO(FBO_HALF1_R).getRTT()[0], viewport.LowerRightCorner.X - viewport.UpperLeftCorner.X, viewport.LowerRightCorner.Y - viewport.UpperLeftCorner.Y);
    }
    else if (irr_driver->getRSM())
    {
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glViewport(viewport.UpperLeftCorner.X, viewport.UpperLeftCorner.Y, viewport.LowerRightCorner.X, viewport.LowerRightCorner.Y);
        post_processing->renderPassThrough(m_rtts->getReflectiveShadowMapFrameBuffer().getRTT()[0], viewport.LowerRightCorner.X - viewport.UpperLeftCorner.X, viewport.LowerRightCorner.Y - viewport.UpperLeftCorner.Y);
    }
    else if (irr_driver->getShadowViz())
    {
        m_shadow_matrices.renderShadowsDebug(m_rtts->getShadowFrameBuffer());
    }
    else
    {
        glEnable(GL_FRAMEBUFFER_SRGB);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        camera->activate();
        post_processing->renderPassThrough(fbo->getRTT()[0], viewport.LowerRightCorner.X - viewport.UpperLeftCorner.X, viewport.LowerRightCorner.Y - viewport.UpperLeftCorner.Y);
        glDisable(GL_FRAMEBUFFER_SRGB);
    }
}


ShaderBasedRenderer::ShaderBasedRenderer():AbstractRenderer()
{    
    m_skybox                = NULL;
    m_spherical_harmonics   = new SphericalHarmonics(irr_driver->getAmbientLight().toSColor());
    m_nb_static_glowing     = 0;
    
    if (CVS->isAZDOEnabled())
        m_geometry_passes = new GeometryPasses<MultidrawPolicy>();
    else if (CVS->supportsIndirectInstancingRendering())
        m_geometry_passes = new GeometryPasses<IndirectDrawPolicy>();
    else
        m_geometry_passes = new GeometryPasses<GL3DrawPolicy>();
}

ShaderBasedRenderer::~ShaderBasedRenderer()
{
    delete m_geometry_passes;
    delete m_spherical_harmonics;
    delete m_skybox;
}

    
void ShaderBasedRenderer::onLoadWorld()
{
    const core::recti &viewport = Camera::getCamera(0)->getViewport();
    size_t width = viewport.LowerRightCorner.X - viewport.UpperLeftCorner.X;
    size_t height = viewport.LowerRightCorner.Y - viewport.UpperLeftCorner.Y;
    m_rtts = new RTT(width, height);    
}

void ShaderBasedRenderer::onUnloadWorld()
{
    delete m_rtts;
    m_rtts = NULL;
    removeSkyBox();    
}


void ShaderBasedRenderer::addSkyBox(const std::vector<video::ITexture*> &texture,
                                    const std::vector<video::ITexture*> &spherical_harmonics_textures)
{
    m_skybox = new Skybox(texture);
    if(spherical_harmonics_textures.size() == 6)
    {
        m_spherical_harmonics->setTextures(spherical_harmonics_textures);
    }    
}

void ShaderBasedRenderer::removeSkyBox()
{
    delete m_skybox;
    m_skybox = NULL;    
}

const SHCoefficients* ShaderBasedRenderer::getSHCoefficients() const
{
    return m_spherical_harmonics->getCoefficients();
}

void ShaderBasedRenderer::setAmbientLight(const video::SColorf &light,
                                          bool force_SH_computation)
{
    if(!m_spherical_harmonics->has6Textures() || force_SH_computation)
        m_spherical_harmonics->setAmbientLight(light.toSColor());
}

void ShaderBasedRenderer::addSunLight(const core::vector3df &pos)
{
    m_shadow_matrices.addLight(pos);
}

void ShaderBasedRenderer::addGlowingNode(scene::ISceneNode *n, float r, float g, float b)
{
    GlowData dat;
    dat.node = n;
    dat.r = r;
    dat.g = g;
    dat.b = b;
    
    STKMeshSceneNode *node = static_cast<STKMeshSceneNode *>(n);
    node->setGlowColors(irr::video::SColor(0, (unsigned) (dat.b * 255.f), (unsigned)(dat.g * 255.f), (unsigned)(dat.r * 255.f)));
    
    m_glowing.push_back(dat);
    m_nb_static_glowing++;
}

void ShaderBasedRenderer::clearGlowingNodes()
{
    m_glowing.clear();
    m_nb_static_glowing = 0;
}

void ShaderBasedRenderer::render(float dt)
{
    BoundingBoxes.clear(); //TODO: what is it doing here?
    
    compressPowerUpTextures(); //TODO: is it useful every frame?
    
    setOverrideMaterial(); //TODO: is it useful every frame?
    
    addItemsInGlowingList();
    
    // Start the RTT for post-processing.
    // We do this before beginScene() because we want to capture the glClear()
    // because of tracks that do not have skyboxes (generally add-on tracks)
    irr_driver->getPostProcessing()->begin();

    World *world = World::getWorld(); // Never NULL.
    Track *track = world->getTrack();
    
    RaceGUIBase *rg = world->getRaceGUI();
    if (rg) rg->update(dt);
    
    if (!CVS->isDefferedEnabled())
    {
        prepareForwardRenderer();
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
        irr_driver->getSceneManager()->setActiveCamera(camnode);

        const core::recti &viewport = camera->getViewport();

        if (!CVS->isDefferedEnabled())
            glEnable(GL_FRAMEBUFFER_SRGB);
        
        PROFILER_PUSH_CPU_MARKER("Update Light Info", 0xFF, 0x0, 0x0);
        m_lighting_passes.updateLightsInfo(camnode, dt);
        PROFILER_POP_CPU_MARKER();
        PROFILER_PUSH_CPU_MARKER("UBO upload", 0x0, 0xFF, 0x0);
        computeMatrixesAndCameras(camnode, viewport.LowerRightCorner.X - viewport.UpperLeftCorner.X, viewport.LowerRightCorner.Y - viewport.UpperLeftCorner.Y);
        m_shadow_matrices.updateSunOrthoMatrices();
        uploadLightingData();
        PROFILER_POP_CPU_MARKER();
        renderScene(camnode, dt, track->hasShadows(), false); 
        
        if (irr_driver->getBoundingBoxesViz())
        {        
            renderBoundingBoxes();
        }
        
        debugPhysics();
        
        if (CVS->isDefferedEnabled())
        {
            renderPostProcessing(camera);
        }
        
        // Save projection-view matrix for the next frame
        camera->setPreviousPVMatrix(irr_driver->getProjViewMatrix());

        PROFILER_POP_CPU_MARKER();
        
    }  // for i<world->getNumKarts()
    
    // Use full screen size
    float tmp[2];
    tmp[0] = float(irr_driver->getActualScreenSize().Width);
    tmp[1] = float(irr_driver->getActualScreenSize().Height);
    glBindBuffer(GL_UNIFORM_BUFFER, 
                 SharedGPUObjects::getViewProjectionMatricesUBO());
    glBufferSubData(GL_UNIFORM_BUFFER, (16 * 9) * sizeof(float),
                    2 * sizeof(float), tmp);

    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    glUseProgram(0);

    // Set the viewport back to the full screen for race gui
    irr_driver->getVideoDriver()->setViewPort(core::recti(0, 0,
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
        ScopedGPUTimer Timer(irr_driver->getGPUTimer(Q_GUI));
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
    irr_driver->getVideoDriver()->endScene();
    PROFILER_POP_CPU_MARKER();

    irr_driver->getPostProcessing()->update(dt);
    removeItemsInGlowingList();
}


