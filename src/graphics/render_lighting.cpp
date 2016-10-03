//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2014-2015 SuperTuxKart-Team
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
#include "graphics/callbacks.hpp"
#include "graphics/central_settings.hpp"
#include "graphics/glwrap.hpp"
#include "graphics/irr_driver.hpp"
#include "graphics/light.hpp"
#include "graphics/post_processing.hpp"
#include "graphics/rtts.hpp"
#include "graphics/shaders.hpp"
#include "graphics/shadow_matrices.hpp"
#include "graphics/shared_gpu_objects.hpp"
#include "modes/world.hpp"
#include "tracks/track.hpp"
#include "utils/profiler.hpp"

#define MAX2(a, b) ((a) > (b) ? (a) : (b))
#define MIN2(a, b) ((a) > (b) ? (b) : (a))

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

#if !defined(USE_GLES2)
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
#endif

// ============================================================================
class FogShader : public TextureShader<FogShader, 1, float, core::vector3df>
{
public:
    FogShader()
    {
        loadProgram(OBJECT, GL_VERTEX_SHADER, "screenquad.vert",
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

// ----------------------------------------------------------------------------
unsigned IrrDriver::updateLightsInfo(scene::ICameraSceneNode * const camnode,
                                     float dt)
{
    const u32 lightcount = (u32)m_lights.size();
    const core::vector3df &campos = camnode->getAbsolutePosition();

    std::vector<LightNode *> BucketedLN[15];
    for (unsigned int i = 0; i < lightcount; i++)
    {
        if (!m_lights[i]->isVisible())
            continue;

        if (!m_lights[i]->isPointLight())
        {
            m_lights[i]->render();
            continue;
        }
        const core::vector3df &lightpos =
                                 (m_lights[i]->getAbsolutePosition() - campos);
        unsigned idx = (unsigned)(lightpos.getLength() / 10);
        if (idx > 14)
            idx = 14;
        BucketedLN[idx].push_back(m_lights[i]);
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
/** Upload lighting info to the dedicated uniform buffer
 */
void IrrDriver::uploadLightingData()
{
    float Lighting[36];
    Lighting[0] = m_sun_direction.X;
    Lighting[1] = m_sun_direction.Y;
    Lighting[2] = m_sun_direction.Z;
    Lighting[4] = m_suncolor.getRed();
    Lighting[5] = m_suncolor.getGreen();
    Lighting[6] = m_suncolor.getBlue();
    Lighting[7] = 0.54f;

    if(m_spherical_harmonics) {
        memcpy(&Lighting[8], m_spherical_harmonics->getBlueSHCoeff(), 9 * sizeof(float));
        memcpy(&Lighting[17], m_spherical_harmonics->getGreenSHCoeff(), 9 * sizeof(float));
        memcpy(&Lighting[26], m_spherical_harmonics->getRedSHCoeff(), 9 * sizeof(float));
    }

    glBindBuffer(GL_UNIFORM_BUFFER, SharedGPUObjects::getLightingDataUBO());
    glBufferSubData(GL_UNIFORM_BUFFER, 0, 36 * sizeof(float), Lighting);
}   // uploadLightingData

// ----------------------------------------------------------------------------
void IrrDriver::renderLights(unsigned pointlightcount, bool hasShadow)
{
    //RH
#if !defined(USE_GLES2)
    if (CVS->isGlobalIlluminationEnabled() && hasShadow)
    {
        ScopedGPUTimer timer(irr_driver->getGPUTimer(Q_RH));
        glDisable(GL_BLEND);
        m_rtts->getRH().bind();
        glBindVertexArray(SharedGPUObjects::getFullScreenQuadVAO());
        if (CVS->needRHWorkaround())
        {
            NVWorkaroundRadianceHintsConstructionShader::getInstance()->use();
            NVWorkaroundRadianceHintsConstructionShader::getInstance()
                ->setTextureUnits(
                    m_rtts->getRSM().getRTT()[0],
                    m_rtts->getRSM().getRTT()[1],
                    m_rtts->getRSM().getDepthTexture());
            for (unsigned i = 0; i < 32; i++)
            {
                NVWorkaroundRadianceHintsConstructionShader::getInstance()
                    ->setUniforms(getShadowMatrices()->getRSMMatrix(),
                                  getShadowMatrices()->getRHMatrix(),
                                  getShadowMatrices()->getRHExtend(), i,
                                  irr_driver->getSunColor());
                glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
            }
        }
        else
        {
            RadianceHintsConstructionShader::getInstance()->use();
            RadianceHintsConstructionShader::getInstance()
                ->setTextureUnits(
                    m_rtts->getRSM().getRTT()[0],
                    m_rtts->getRSM().getRTT()[1],
                    m_rtts->getRSM().getDepthTexture()
            );
            RadianceHintsConstructionShader::getInstance()
                ->setUniforms(getShadowMatrices()->getRSMMatrix(),
                              getShadowMatrices()->getRHMatrix(),
                              getShadowMatrices()->getRHExtend(),
                              irr_driver->getSunColor());
            glDrawArraysInstanced(GL_TRIANGLE_STRIP, 0, 4, 32);
        }
    }
#endif
    getShadowMatrices()->updateSunOrthoMatrices();
    m_rtts->getFBO(FBO_COMBINED_DIFFUSE_SPECULAR).bind();
    glClear(GL_COLOR_BUFFER_BIT);

    m_rtts->getFBO(FBO_DIFFUSE).bind();
    if (CVS->isGlobalIlluminationEnabled() && hasShadow)
    {
        ScopedGPUTimer timer(irr_driver->getGPUTimer(Q_GI));
        m_post_processing->renderGI(getShadowMatrices()->getRHMatrix(),
                                    getShadowMatrices()->getRHExtend(),
                                    m_rtts->getRH());
    }

    m_rtts->getFBO(FBO_COMBINED_DIFFUSE_SPECULAR).bind();

    {
        ScopedGPUTimer timer(irr_driver->getGPUTimer(Q_ENVMAP));
        if(m_skybox)
        {
            m_post_processing->renderEnvMap(m_skybox->getSpecularProbe());
        }
        else
        {
            m_post_processing->renderEnvMap(0);
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
                ShadowedSunLightShaderESM::getInstance()->render(m_rtts);
            }
            else
            {
                ShadowedSunLightShaderPCF::getInstance()->render(m_rtts);
            }
        }
        else
            m_post_processing->renderSunlight(irr_driver->getSunDirection(),
                                              irr_driver->getSunColor());
    }
    {
        ScopedGPUTimer timer(irr_driver->getGPUTimer(Q_POINTLIGHTS));
        renderPointLights(MIN2(pointlightcount, LightBaseClass::MAXLIGHT));
    }
}   // renderLights

// ----------------------------------------------------------------------------
void IrrDriver::renderSSAO()
{
#if defined(USE_GLES2)
    if (!CVS->isEXTColorBufferFloatUsable())
        return;
#endif

    m_rtts->getFBO(FBO_SSAO).bind();
    glClearColor(1., 1., 1., 1.);
    glClear(GL_COLOR_BUFFER_BIT);
    m_post_processing->renderSSAO();
    // Blur it to reduce noise.
    FrameBuffer::Blit(m_rtts->getFBO(FBO_SSAO),
                      m_rtts->getFBO(FBO_HALF1_R),
                      GL_COLOR_BUFFER_BIT, GL_LINEAR);
    m_post_processing->renderGaussian17TapBlur(irr_driver->getFBO(FBO_HALF1_R),
                                               irr_driver->getFBO(FBO_HALF2_R));

}   // renderSSAO

// ----------------------------------------------------------------------------
void IrrDriver::renderAmbientScatter()
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
void IrrDriver::renderLightsScatter(unsigned pointlightcount)
{
    getFBO(FBO_HALF1).bind();
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
                          MIN2(pointlightcount, LightBaseClass::MAXLIGHT));

    glDisable(GL_BLEND);
    m_post_processing->renderGaussian6Blur(getFBO(FBO_HALF1),
                                           getFBO(FBO_HALF2), 5., 5.);
    glEnable(GL_BLEND);

    glDisable(GL_DEPTH_TEST);
    glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
    getFBO(FBO_COLORS).bind();
    m_post_processing->renderPassThrough(getRenderTargetTexture(RTT_HALF1),
                                         getFBO(FBO_COLORS).getWidth(),
                                         getFBO(FBO_COLORS).getHeight());
}   // renderLightsScatter
