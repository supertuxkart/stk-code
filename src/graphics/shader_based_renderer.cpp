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
#include "graphics/light.hpp"
#include "graphics/lod_node.hpp"
#include "graphics/post_processing.hpp"
#include "graphics/rtts.hpp"
#include "graphics/shaders.hpp"
#include "graphics/shadow_matrices.hpp"
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

class LightBaseClass
{
public:
    struct PointLightInfo
    {
        float posX;
        float posY;
        float posZ;
        float energy;
        float red;
        float green;
        float blue;
        float radius;
    };
public:
    static const unsigned int MAXLIGHT = 32;
public:
    static struct PointLightInfo m_point_lights_info[MAXLIGHT];
};   // LightBaseClass

const unsigned int LightBaseClass::MAXLIGHT;

// ============================================================================
LightBaseClass::PointLightInfo m_point_lights_info[LightBaseClass::MAXLIGHT];


// ============================================================================
class PointLightShader : public TextureShader < PointLightShader, 2 >
{
public:
    GLuint vbo;
    GLuint vao;
    PointLightShader()
    {
        loadProgram(OBJECT, GL_VERTEX_SHADER, "pointlight.vert",
                            GL_FRAGMENT_SHADER, "utils/decodeNormal.frag",
                            GL_FRAGMENT_SHADER, "utils/SpecularBRDF.frag",
                            GL_FRAGMENT_SHADER, "utils/DiffuseBRDF.frag",
                            GL_FRAGMENT_SHADER, "utils/getPosFromUVDepth.frag",
                            GL_FRAGMENT_SHADER, "pointlight.frag");

        assignUniforms();
        assignSamplerNames(0, "ntex", ST_NEAREST_FILTERED, 
                           1, "dtex", ST_NEAREST_FILTERED);
        glGenVertexArrays(1, &vao);
        glBindVertexArray(vao);

        glGenBuffers(1, &vbo);
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glBufferData(GL_ARRAY_BUFFER, 
                     LightBaseClass::MAXLIGHT * sizeof(LightBaseClass::PointLightInfo),
                     0, GL_DYNAMIC_DRAW);

        GLuint attrib_Position = glGetAttribLocation(m_program, "Position");
        GLuint attrib_Color = glGetAttribLocation(m_program, "Color");
        GLuint attrib_Energy = glGetAttribLocation(m_program, "Energy");
        GLuint attrib_Radius = glGetAttribLocation(m_program, "Radius");

        glEnableVertexAttribArray(attrib_Position);
        glVertexAttribPointer(attrib_Position, 3, GL_FLOAT, GL_FALSE, 
                              sizeof(LightBaseClass::PointLightInfo), 0);
        glEnableVertexAttribArray(attrib_Energy);
        glVertexAttribPointer(attrib_Energy, 1, GL_FLOAT, GL_FALSE,
                              sizeof(LightBaseClass::PointLightInfo),
                              (GLvoid*)(3 * sizeof(float)));
        glEnableVertexAttribArray(attrib_Color);
        glVertexAttribPointer(attrib_Color, 3, GL_FLOAT, GL_FALSE,
                              sizeof(LightBaseClass::PointLightInfo),
                              (GLvoid*)(4 * sizeof(float)));
        glEnableVertexAttribArray(attrib_Radius);
        glVertexAttribPointer(attrib_Radius, 1, GL_FLOAT, GL_FALSE, 
                              sizeof(LightBaseClass::PointLightInfo),
                              (GLvoid*)(7 * sizeof(float)));

        glVertexAttribDivisorARB(attrib_Position, 1);
        glVertexAttribDivisorARB(attrib_Energy, 1);
        glVertexAttribDivisorARB(attrib_Color, 1);
        glVertexAttribDivisorARB(attrib_Radius, 1);
    }   // PointLightShader
};   // PointLightShader


// ============================================================================
class PointLightScatterShader : public TextureShader<PointLightScatterShader,
                                                     1, float, core::vector3df>
{
public:
    GLuint vbo;
    GLuint vao;
    PointLightScatterShader()
    {
        loadProgram(OBJECT, GL_VERTEX_SHADER, "pointlight.vert",
                            GL_FRAGMENT_SHADER, "utils/getPosFromUVDepth.frag",
                            GL_FRAGMENT_SHADER, "pointlightscatter.frag");

        assignUniforms("density", "fogcol");
        assignSamplerNames(0, "dtex", ST_NEAREST_FILTERED);

        glGenVertexArrays(1, &vao);
        glBindVertexArray(vao);

        glBindBuffer(GL_ARRAY_BUFFER, PointLightShader::getInstance()->vbo);

        GLuint attrib_Position = glGetAttribLocation(m_program, "Position");
        GLuint attrib_Color = glGetAttribLocation(m_program, "Color");
        GLuint attrib_Energy = glGetAttribLocation(m_program, "Energy");
        GLuint attrib_Radius = glGetAttribLocation(m_program, "Radius");

        glEnableVertexAttribArray(attrib_Position);
        glVertexAttribPointer(attrib_Position, 3, GL_FLOAT, GL_FALSE,
                              sizeof(LightBaseClass::PointLightInfo), 0);
        glEnableVertexAttribArray(attrib_Energy);
        glVertexAttribPointer(attrib_Energy, 1, GL_FLOAT, GL_FALSE,
                              sizeof(LightBaseClass::PointLightInfo),
                              (GLvoid*)(3 * sizeof(float)));
        glEnableVertexAttribArray(attrib_Color);
        glVertexAttribPointer(attrib_Color, 3, GL_FLOAT, GL_FALSE,
                              sizeof(LightBaseClass::PointLightInfo),
                              (GLvoid*)(4 * sizeof(float)));
        glEnableVertexAttribArray(attrib_Radius);
        glVertexAttribPointer(attrib_Radius, 1, GL_FLOAT, GL_FALSE,
                              sizeof(LightBaseClass::PointLightInfo),
                              (GLvoid*)(7 * sizeof(float)));

        glVertexAttribDivisorARB(attrib_Position, 1);
        glVertexAttribDivisorARB(attrib_Energy, 1);
        glVertexAttribDivisorARB(attrib_Color, 1);
        glVertexAttribDivisorARB(attrib_Radius, 1);
    }   // PointLightScatterShader
};

// ============================================================================
class ShadowedSunLightShaderPCF : public TextureShader<ShadowedSunLightShaderPCF,
                                                       3,  float, float, float,
                                                       float, float>
{
public:
    ShadowedSunLightShaderPCF()
    {
        loadProgram(OBJECT, GL_VERTEX_SHADER, "screenquad.vert",
                            GL_FRAGMENT_SHADER, "utils/decodeNormal.frag",
                            GL_FRAGMENT_SHADER, "utils/SpecularBRDF.frag",
                            GL_FRAGMENT_SHADER, "utils/DiffuseBRDF.frag",
                            GL_FRAGMENT_SHADER, "utils/getPosFromUVDepth.frag",
                            GL_FRAGMENT_SHADER, "utils/SunMRP.frag",
                            GL_FRAGMENT_SHADER, "sunlightshadow.frag");

        // Use 8 to circumvent a catalyst bug when binding sampler
        assignSamplerNames(0, "ntex", ST_NEAREST_FILTERED,
                           1, "dtex", ST_NEAREST_FILTERED,
                           8, "shadowtex", ST_SHADOW_SAMPLER);
        assignUniforms("split0", "split1", "split2", "splitmax", "shadow_res");
    }   // ShadowedSunLightShaderPCF
    // ------------------------------------------------------------------------
    void render(RTT *rtts)
    {
        setTextureUnits(irr_driver->getRenderTargetTexture(RTT_NORMAL_AND_DEPTH),
                        irr_driver->getDepthStencilTexture(),
                        rtts->getShadowFBO().getDepthTexture()                );
       drawFullScreenEffect(ShadowMatrices::m_shadow_split[1],
                            ShadowMatrices::m_shadow_split[2],
                            ShadowMatrices::m_shadow_split[3],
                            ShadowMatrices::m_shadow_split[4], 
                            float(UserConfigParams::m_shadows_resolution)   );

    }    // render
};   // ShadowedSunLightShaderPCF

// ============================================================================
class ShadowedSunLightShaderESM : public TextureShader<ShadowedSunLightShaderESM,
                                                       3, float, float, float,
                                                       float>
{
public:
    ShadowedSunLightShaderESM() 
    {
        loadProgram(OBJECT, GL_VERTEX_SHADER, "screenquad.vert",
                            GL_FRAGMENT_SHADER, "utils/decodeNormal.frag",
                            GL_FRAGMENT_SHADER, "utils/SpecularBRDF.frag",
                            GL_FRAGMENT_SHADER, "utils/DiffuseBRDF.frag",
                            GL_FRAGMENT_SHADER, "utils/getPosFromUVDepth.frag",
                            GL_FRAGMENT_SHADER, "utils/SunMRP.frag",
                            GL_FRAGMENT_SHADER, "sunlightshadowesm.frag");

        // Use 8 to circumvent a catalyst bug when binding sampler
        assignSamplerNames(0, "ntex", ST_NEAREST_FILTERED,
                           1, "dtex", ST_NEAREST_FILTERED,
                           8, "shadowtex", ST_TRILINEAR_CLAMPED_ARRAY2D);
            
        assignUniforms("split0", "split1", "split2", "splitmax");
    }   // ShadowedSunLightShaderESM
    // ------------------------------------------------------------------------
    void render(RTT *rtt)
    {
        setTextureUnits(irr_driver->getRenderTargetTexture(RTT_NORMAL_AND_DEPTH),
                        irr_driver->getDepthStencilTexture(),
                        rtt->getShadowFBO().getRTT()[0]);
        drawFullScreenEffect(ShadowMatrices::m_shadow_split[1],
                             ShadowMatrices::m_shadow_split[2],
                             ShadowMatrices::m_shadow_split[3],
                             ShadowMatrices::m_shadow_split[4]);
    }   // render
};   // ShadowedSunLightShaderESM

// ============================================================================
class RadianceHintsConstructionShader
    : public TextureShader<RadianceHintsConstructionShader, 3, core::matrix4, 
                          core::matrix4, core::vector3df, video::SColorf>
{
public:
    RadianceHintsConstructionShader()
    {
        if (CVS->isAMDVertexShaderLayerUsable())
        {
            loadProgram(OBJECT, GL_VERTEX_SHADER, "slicedscreenquad.vert",
                                GL_FRAGMENT_SHADER, "rh.frag");
        }
        else
        {
            loadProgram(OBJECT, GL_VERTEX_SHADER, "slicedscreenquad.vert",
                                GL_GEOMETRY_SHADER, "rhpassthrough.geom",
                                GL_FRAGMENT_SHADER, "rh.frag");
        }

        assignUniforms("RSMMatrix", "RHMatrix", "extents", "suncol");
        assignSamplerNames(0, "ctex", ST_BILINEAR_FILTERED,
                           1, "ntex", ST_BILINEAR_FILTERED,
                           2, "dtex", ST_BILINEAR_FILTERED);
    }   // RadianceHintsConstructionShader
};   // RadianceHintsConstructionShader

// ============================================================================
// Workaround for a bug found in kepler nvidia linux and fermi nvidia windows
class NVWorkaroundRadianceHintsConstructionShader
    : public TextureShader<NVWorkaroundRadianceHintsConstructionShader,
                           3, core::matrix4, core::matrix4, core::vector3df,
                           int, video::SColorf >
{
public:
    NVWorkaroundRadianceHintsConstructionShader()
    {
        loadProgram(OBJECT,GL_VERTEX_SHADER,"slicedscreenquad_nvworkaround.vert",
                           GL_GEOMETRY_SHADER, "rhpassthrough.geom",
                           GL_FRAGMENT_SHADER, "rh.frag");

        assignUniforms("RSMMatrix", "RHMatrix", "extents", "slice", "suncol");

        assignSamplerNames(0, "ctex", ST_BILINEAR_FILTERED,
                           1, "ntex", ST_BILINEAR_FILTERED,
                           2, "dtex", ST_BILINEAR_FILTERED);
    }   // NVWorkaroundRadianceHintsConstructionShader
};   // NVWorkaroundRadianceHintsConstructionShader

// ============================================================================
class FogShader : public TextureShader<FogShader, 1, float, core::vector3df>
{
public:
    FogShader()
    {
        loadProgram(OBJECT, GL_VERTEX_SHADER, "screenquad.vert",
                            GL_FRAGMENT_SHADER, "utils/getPosFromUVDepth.frag",
                            GL_FRAGMENT_SHADER, "fog.frag");
        assignUniforms("density", "col");
        assignSamplerNames(0, "tex", ST_NEAREST_FILTERED);
    }   // FogShader
    // ------------------------------------------------------------------------
    void render(float start, const core::vector3df &color)
    {
        setTextureUnits(irr_driver->getDepthStencilTexture());
        drawFullScreenEffect(1.f / (40.f * start), color);

    }   // render
};   // FogShader


// ============================================================================
static void renderPointLights(unsigned count)
{
    glEnable(GL_BLEND);
    glBlendEquation(GL_FUNC_ADD);
    glBlendFunc(GL_ONE, GL_ONE);
    glEnable(GL_DEPTH_TEST);
    glDepthMask(GL_FALSE);

    PointLightShader::getInstance()->use();
    glBindVertexArray(PointLightShader::getInstance()->vao);
    glBindBuffer(GL_ARRAY_BUFFER, PointLightShader::getInstance()->vbo);
    glBufferSubData(GL_ARRAY_BUFFER, 0, 
                     count * sizeof(LightBaseClass::PointLightInfo),
                     m_point_lights_info);

    PointLightShader::getInstance()->setTextureUnits(
        irr_driver->getRenderTargetTexture(RTT_NORMAL_AND_DEPTH),
        irr_driver->getDepthStencilTexture());
    PointLightShader::getInstance()->setUniforms();

    glDrawArraysInstanced(GL_TRIANGLE_STRIP, 0, 4, count);
}   // renderPointLights




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
}

std::vector<GlowData> ShaderBasedRenderer::updateGlowingList()
{
    // Get a list of all glowing things. The driver's list contains the static ones,
    // here we add items, as they may disappear each frame.
    std::vector<GlowData> glows = irr_driver->getGlowingNodes();

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
    
    return glows;
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
unsigned ShaderBasedRenderer::updateLightsInfo(scene::ICameraSceneNode * const camnode,
                                               float dt)
{
    std::vector<LightNode *> lights = irr_driver->getLights();
    const u32 lightcount = (u32)lights.size();
    const core::vector3df &campos = camnode->getAbsolutePosition();

    std::vector<LightNode *> BucketedLN[15];
    for (unsigned int i = 0; i < lightcount; i++)
    {
        if (!lights[i]->isVisible())
            continue;

        if (!lights[i]->isPointLight())
        {
            lights[i]->render();
            continue;
        }
        const core::vector3df &lightpos = 
                                 (lights[i]->getAbsolutePosition() - campos);
        unsigned idx = (unsigned)(lightpos.getLength() / 10);
        if (idx > 14)
            idx = 14;
        BucketedLN[idx].push_back(lights[i]);
    }

    unsigned lightnum = 0;
    bool multiplayer = (race_manager->getNumLocalPlayers() > 1);

    for (unsigned i = 0; i < 15; i++)
    {
        for (unsigned j = 0; j < BucketedLN[i].size(); j++)
        {
            if (++lightnum >= LightBaseClass::MAXLIGHT)
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
                    // In single-player, fade-in lights.
                    // In multi-player, can't do that, the light objects are shared by all players
                    if (multiplayer)
                        light_node->setEnergyMultiplier(1.0f);
                    else
                        light_node->setEnergyMultiplier(std::min(1.0f, em + dt));
                }

                const core::vector3df &pos = light_node->getAbsolutePosition();
                m_point_lights_info[lightnum].posX = pos.X;
                m_point_lights_info[lightnum].posY = pos.Y;
                m_point_lights_info[lightnum].posZ = pos.Z;

                m_point_lights_info[lightnum].energy = 
                                              light_node->getEffectiveEnergy();

                const core::vector3df &col = light_node->getColor();
                m_point_lights_info[lightnum].red = col.X;
                m_point_lights_info[lightnum].green = col.Y;
                m_point_lights_info[lightnum].blue = col.Z;

                // Light radius
                m_point_lights_info[lightnum].radius = light_node->getRadius();
            }
        }
        if (lightnum > LightBaseClass::MAXLIGHT)
        {
            irr_driver->setLastLightBucketDistance(i * 10);
            break;
        }
    }

    lightnum++;
    return lightnum;
}   // updateLightsInfo

// ----------------------------------------------------------------------------
void ShaderBasedRenderer::renderLights(unsigned pointlightcount, bool hasShadow)
{
    RTT *rtts = irr_driver->getRTT();
    ShadowMatrices *shadow_matrices = irr_driver->getShadowMatrices();
    
    //RH
    if (CVS->isGlobalIlluminationEnabled() && hasShadow)
    {
        ScopedGPUTimer timer(irr_driver->getGPUTimer(Q_RH));
        glDisable(GL_BLEND);
        rtts->getRH().bind();
        glBindVertexArray(SharedGPUObjects::getFullScreenQuadVAO());
        if (CVS->needRHWorkaround())
        {
            NVWorkaroundRadianceHintsConstructionShader::getInstance()->use();
            NVWorkaroundRadianceHintsConstructionShader::getInstance()
                ->setTextureUnits(
                    rtts->getRSM().getRTT()[0],
                    rtts->getRSM().getRTT()[1],
                    rtts->getRSM().getDepthTexture());
            for (unsigned i = 0; i < 32; i++)
            {
                NVWorkaroundRadianceHintsConstructionShader::getInstance()
                    ->setUniforms(shadow_matrices->getRSMMatrix(), 
                                  shadow_matrices->getRHMatrix(),
                                  shadow_matrices->getRHExtend(), i,
                                  irr_driver->getSunColor());
                glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
            }
        }
        else
        {
            RadianceHintsConstructionShader::getInstance()->use();
            RadianceHintsConstructionShader::getInstance()
                ->setTextureUnits(
                    rtts->getRSM().getRTT()[0],
                    rtts->getRSM().getRTT()[1],
                    rtts->getRSM().getDepthTexture()
            );
            RadianceHintsConstructionShader::getInstance()
                ->setUniforms(shadow_matrices->getRSMMatrix(),
                              shadow_matrices->getRHMatrix(), 
                              shadow_matrices->getRHExtend(),
                              irr_driver->getSunColor());
            glDrawArraysInstanced(GL_TRIANGLE_STRIP, 0, 4, 32);
        }
    }
    shadow_matrices->updateSunOrthoMatrices();
    rtts->getFBO(FBO_COMBINED_DIFFUSE_SPECULAR).bind();
    glClear(GL_COLOR_BUFFER_BIT);

    rtts->getFBO(FBO_DIFFUSE).bind();
    PostProcessing *post_processing = irr_driver->getPostProcessing();
    if (CVS->isGlobalIlluminationEnabled() && hasShadow)
    {
        ScopedGPUTimer timer(irr_driver->getGPUTimer(Q_GI));
        post_processing->renderGI(shadow_matrices->getRHMatrix(),
                                  shadow_matrices->getRHExtend(),
                                  rtts->getRH());
    }

    rtts->getFBO(FBO_COMBINED_DIFFUSE_SPECULAR).bind();

    {
        ScopedGPUTimer timer(irr_driver->getGPUTimer(Q_ENVMAP));
        Skybox *skybox = irr_driver->getSkybox();
        if(skybox)
        {
            post_processing->renderEnvMap(skybox->getSpecularProbe());
        }
        else 
        {
            post_processing->renderEnvMap(0);
        }           
    }

    // Render sunlight if and only if track supports shadow
    if (!World::getWorld() || World::getWorld()->getTrack()->hasShadows())
    {
        ScopedGPUTimer timer(irr_driver->getGPUTimer(Q_SUN));
        if (World::getWorld() && CVS->isShadowEnabled() && hasShadow)
        {
            glEnable(GL_BLEND);
            glDisable(GL_DEPTH_TEST);
            glBlendFunc(GL_ONE, GL_ONE);
            glBlendEquation(GL_FUNC_ADD);

            if (CVS->isESMEnabled())
            {
                ShadowedSunLightShaderESM::getInstance()->render(rtts);
            }
            else
            {
                ShadowedSunLightShaderPCF::getInstance()->render(rtts);
            }
        }
        else
            post_processing->renderSunlight(irr_driver->getSunDirection(),
                                            irr_driver->getSunColor());
    }
    {
        ScopedGPUTimer timer(irr_driver->getGPUTimer(Q_POINTLIGHTS));
        renderPointLights(std::min(pointlightcount, LightBaseClass::MAXLIGHT));
    }
}   // renderLights

// ----------------------------------------------------------------------------
void ShaderBasedRenderer::renderAmbientScatter()
{
    const Track * const track = World::getWorld()->getTrack();

    // This function is only called once per frame - thus no need for setters.
    float start = track->getFogStart() + .001f;
    const video::SColor tmpcol = track->getFogColor();

    core::vector3df col(tmpcol.getRed() / 255.0f,
        tmpcol.getGreen() / 255.0f,
        tmpcol.getBlue() / 255.0f);

    glDisable(GL_DEPTH_TEST);
    glDepthMask(GL_FALSE);
    glEnable(GL_BLEND);
    glBlendEquation(GL_FUNC_ADD);
    glBlendFunc(GL_ONE, GL_ONE);

    FogShader::getInstance()->render(start, col);
}   // renderAmbientScatter

// ----------------------------------------------------------------------------
void ShaderBasedRenderer::renderLightsScatter(unsigned pointlightcount)
{
    irr_driver->getFBO(FBO_HALF1).bind();
    glClearColor(0., 0., 0., 0.);
    glClear(GL_COLOR_BUFFER_BIT);

    const Track * const track = World::getWorld()->getTrack();

    // This function is only called once per frame - thus no need for setters.
    float start = track->getFogStart() + .001f;
    const video::SColor tmpcol = track->getFogColor();

    core::vector3df col(tmpcol.getRed() / 255.0f,
        tmpcol.getGreen() / 255.0f,
        tmpcol.getBlue() / 255.0f);

    glDepthMask(GL_FALSE);
    glEnable(GL_BLEND);
    glBlendEquation(GL_FUNC_ADD);
    glBlendFunc(GL_ONE, GL_ONE);

    glEnable(GL_DEPTH_TEST);
    core::vector3df col2(1., 1., 1.);

    PointLightScatterShader::getInstance()->use();
    glBindVertexArray(PointLightScatterShader::getInstance()->vao);

    PointLightScatterShader::getInstance()
        ->setTextureUnits(irr_driver->getDepthStencilTexture());
    PointLightScatterShader::getInstance()
        ->setUniforms(1.f / (40.f * start), col2);

    glDrawArraysInstanced(GL_TRIANGLE_STRIP, 0, 4,
                          std::min(pointlightcount, LightBaseClass::MAXLIGHT));

    glDisable(GL_BLEND);
    PostProcessing *post_processing = irr_driver->getPostProcessing();
    post_processing->renderGaussian6Blur(irr_driver->getFBO(FBO_HALF1),
                                         irr_driver->getFBO(FBO_HALF2), 5., 5.);
    glEnable(GL_BLEND);

    glDisable(GL_DEPTH_TEST);
    glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
    irr_driver->getFBO(FBO_COLORS).bind();
    post_processing->renderPassThrough(irr_driver->getRenderTargetTexture(RTT_HALF1),
                                       irr_driver->getFBO(FBO_COLORS).getWidth(),
                                       irr_driver->getFBO(FBO_COLORS).getHeight());
}   // renderLightsScatter

// ============================================================================
void ShaderBasedRenderer::renderScene(scene::ICameraSceneNode * const camnode, unsigned pointlightcount, std::vector<GlowData>& glows, float dt, bool hasShadow, bool forceRTT)
{
    ShadowMatrices *shadow_matrices = irr_driver->getShadowMatrices();
    PostProcessing *post_processing = irr_driver->getPostProcessing();
    
    glBindBufferBase(GL_UNIFORM_BUFFER, 0, SharedGPUObjects::getViewProjectionMatricesUBO());
    glBindBufferBase(GL_UNIFORM_BUFFER, 1, SharedGPUObjects::getLightingDataUBO());
    irr_driver->getSceneManager()->setActiveCamera(camnode);

    PROFILER_PUSH_CPU_MARKER("- Draw Call Generation", 0xFF, 0xFF, 0xFF);
    irr_driver->PrepareDrawCalls(camnode);
    PROFILER_POP_CPU_MARKER();
    // Shadows
    {
        // To avoid wrong culling, use the largest view possible
        irr_driver->getSceneManager()->setActiveCamera(shadow_matrices->getSunCam());
        if (CVS->isDefferedEnabled() &&
            CVS->isShadowEnabled() && hasShadow)
        {
            PROFILER_PUSH_CPU_MARKER("- Shadow", 0x30, 0x6F, 0x90);
            irr_driver->renderShadows();
            PROFILER_POP_CPU_MARKER();
            if (CVS->isGlobalIlluminationEnabled())
            {
                PROFILER_PUSH_CPU_MARKER("- RSM", 0xFF, 0x0, 0xFF);
                irr_driver->renderRSM();
                PROFILER_POP_CPU_MARKER();
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
    RTT *rtts = irr_driver->getRTT();
    if (CVS->isDefferedEnabled() || forceRTT)
    {
        rtts->getFBO(FBO_NORMAL_AND_DEPTHS).bind();
        glClearColor(0., 0., 0., 0.);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
        irr_driver->renderSolidFirstPass();
    }
    else
    {
        // We need a cleared depth buffer for some effect (eg particles depth blending)
        if (GraphicsRestrictions::isDisabled(GraphicsRestrictions::GR_FRAMEBUFFER_SRGB_WORKING))
            glDisable(GL_FRAMEBUFFER_SRGB);
        rtts->getFBO(FBO_NORMAL_AND_DEPTHS).bind();
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
        ScopedGPUTimer Timer(irr_driver->getGPUTimer(Q_SSAO));
        if (UserConfigParams::m_ssao)
            irr_driver->renderSSAO();
        PROFILER_POP_CPU_MARKER();
    }

    PROFILER_PUSH_CPU_MARKER("- Solid Pass 2", 0x00, 0x00, 0xFF);
    if (CVS->isDefferedEnabled() || forceRTT)
    {
        rtts->getFBO(FBO_COLORS).bind();
        video::SColor clearColor(0, 150, 150, 150);
        if (World::getWorld() != NULL)
            clearColor = World::getWorld()->getClearColor();

        glClearColor(clearColor.getRed() / 255.f, clearColor.getGreen() / 255.f,
            clearColor.getBlue() / 255.f, clearColor.getAlpha() / 255.f);
        glClear(GL_COLOR_BUFFER_BIT);
        glDepthMask(GL_FALSE);
    }
    irr_driver->renderSolidSecondPass();
    PROFILER_POP_CPU_MARKER();

    if (irr_driver->getNormals())
    {
        rtts->getFBO(FBO_NORMAL_AND_DEPTHS).bind();
        irr_driver->renderNormalsVisualisation();
        rtts->getFBO(FBO_COLORS).bind();
    }

    // Render ambient scattering
    if (CVS->isDefferedEnabled() && World::getWorld() != NULL &&
        World::getWorld()->isFogEnabled())
    {
        PROFILER_PUSH_CPU_MARKER("- Ambient scatter", 0xFF, 0x00, 0x00);
        ScopedGPUTimer Timer(irr_driver->getGPUTimer(Q_FOG));
        renderAmbientScatter();
        PROFILER_POP_CPU_MARKER();
    }

    {
        PROFILER_PUSH_CPU_MARKER("- Skybox", 0xFF, 0x00, 0xFF);
        ScopedGPUTimer Timer(irr_driver->getGPUTimer(Q_SKYBOX));
        irr_driver->renderSkybox(camnode);
        PROFILER_POP_CPU_MARKER();
    }

    // Render discrete lights scattering
    if (CVS->isDefferedEnabled() && World::getWorld() != NULL &&
        World::getWorld()->isFogEnabled())
    {
        PROFILER_PUSH_CPU_MARKER("- PointLight Scatter", 0xFF, 0x00, 0x00);
        ScopedGPUTimer Timer(irr_driver->getGPUTimer(Q_FOG));
        renderLightsScatter(pointlightcount);
        PROFILER_POP_CPU_MARKER();
    }

    if (irr_driver->getRH())
    {
        glDisable(GL_BLEND);
        rtts->getFBO(FBO_COLORS).bind();
        post_processing->renderRHDebug(rtts->getRH().getRTT()[0],
                                       rtts->getRH().getRTT()[1], 
                                       rtts->getRH().getRTT()[2],
                                       shadow_matrices->getRHMatrix(),
                                       shadow_matrices->getRHExtend());
    }

    if (irr_driver->getGI())
    {
        glDisable(GL_BLEND);
        rtts->getFBO(FBO_COLORS).bind();
        post_processing->renderGI(shadow_matrices->getRHMatrix(),
                                  shadow_matrices->getRHExtend(),
                                  rtts->getRH());
    }

    PROFILER_PUSH_CPU_MARKER("- Glow", 0xFF, 0xFF, 0x00);
    // Render anything glowing.
    if (!m_mipviz && !m_wireframe && UserConfigParams::m_glow)
    {
        ScopedGPUTimer Timer(irr_driver->getGPUTimer(Q_GLOW));
        irr_driver->setPhase(GLOW_PASS);
        irr_driver->renderGlow(glows);
    } // end glow
    PROFILER_POP_CPU_MARKER();

    // Render transparent
    {
        PROFILER_PUSH_CPU_MARKER("- Transparent Pass", 0xFF, 0x00, 0x00);
        ScopedGPUTimer Timer(irr_driver->getGPUTimer(Q_TRANSPARENT));
        irr_driver->renderTransparent();
        PROFILER_POP_CPU_MARKER();
    }

    irr_driver->m_sync = glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE, 0);

    // Render particles
    {
        PROFILER_PUSH_CPU_MARKER("- Particles", 0xFF, 0xFF, 0x00);
        ScopedGPUTimer Timer(irr_driver->getGPUTimer(Q_PARTICLES));
        irr_driver->renderParticles();
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
        post_processing->renderPassThrough(irr_driver->getRTT()->getFBO(FBO_HALF1_R).getRTT()[0], viewport.LowerRightCorner.X - viewport.UpperLeftCorner.X, viewport.LowerRightCorner.Y - viewport.UpperLeftCorner.Y);
    }
    else if (irr_driver->getRSM())
    {
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glViewport(viewport.UpperLeftCorner.X, viewport.UpperLeftCorner.Y, viewport.LowerRightCorner.X, viewport.LowerRightCorner.Y);
        post_processing->renderPassThrough(irr_driver->getRTT()->getRSM().getRTT()[0], viewport.LowerRightCorner.X - viewport.UpperLeftCorner.X, viewport.LowerRightCorner.Y - viewport.UpperLeftCorner.Y);
    }
    else if (irr_driver->getShadowViz())
    {
        irr_driver->getShadowMatrices()->renderShadowsDebug();
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
}

void ShaderBasedRenderer::render(float dt)
{
    BoundingBoxes.clear(); //TODO: what is it doing here?
    
    compressPowerUpTextures(); //TODO: is it useful every frame?
    
    setOverrideMaterial(); //TODO: is it useful every frame?
    
    std::vector<GlowData> glows = updateGlowingList();
    
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
        unsigned plc = updateLightsInfo(camnode, dt); //TODO: replace plc by a more explicit name
        PROFILER_POP_CPU_MARKER();
        PROFILER_PUSH_CPU_MARKER("UBO upload", 0x0, 0xFF, 0x0);
        irr_driver->computeMatrixesAndCameras(camnode, viewport.LowerRightCorner.X - viewport.UpperLeftCorner.X, viewport.LowerRightCorner.Y - viewport.UpperLeftCorner.Y);
        irr_driver->uploadLightingData(); //TODO: move method; update "global" lighting (sun and spherical harmonics)
        PROFILER_POP_CPU_MARKER();
        renderScene(camnode, plc, glows, dt, track->hasShadows(), false); 
        
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
}


