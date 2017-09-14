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

#include "graphics/lighting_passes.hpp"
#include "config/user_config.hpp"
#include "graphics/central_settings.hpp"
#include "graphics/glwrap.hpp"
#include "graphics/irr_driver.hpp"
#include "graphics/light.hpp"
#include "graphics/post_processing.hpp"
#include "graphics/rtts.hpp"
#include "graphics/shadow_matrices.hpp"
#include "graphics/texture_shader.hpp"
#include "modes/world.hpp"
#include "tracks/track.hpp"
#include "utils/profiler.hpp"

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
    void render(float start, const core::vector3df &color, GLuint depth_stencil_texture)
    {
        setTextureUnits(depth_stencil_texture);
        drawFullScreenEffect(1.f / (40.f * start), color);

    }   // render
};   // FogShader

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
#endif // !defined(USE_GLES2)

// ============================================================================
class GlobalIlluminationReconstructionShader
    : public TextureShader<GlobalIlluminationReconstructionShader, 5,
                           core::matrix4, core::matrix4, core::vector3df >
{
public:
    GlobalIlluminationReconstructionShader()
    {
        loadProgram(OBJECT, GL_VERTEX_SHADER, "screenquad.vert",
                            GL_FRAGMENT_SHADER, "gi.frag");

        assignUniforms("rh_matrix", "inv_rh_matrix", "extents");
        assignSamplerNames(0, "ntex", ST_NEAREST_FILTERED,
                           1, "dtex", ST_NEAREST_FILTERED,
                           2, "SHR", ST_VOLUME_LINEAR_FILTERED,
                           3, "SHG", ST_VOLUME_LINEAR_FILTERED,
                           4, "SHB", ST_VOLUME_LINEAR_FILTERED);
    }   // GlobalIlluminationReconstructionShader

    // ------------------------------------------------------------------------
    void render(const core::matrix4 &rh_matrix,
                const core::vector3df &rh_extend, const FrameBuffer &fb,
                GLuint normal_depth_texture,
                GLuint depth_stencil_texture)
    {
        core::matrix4 inv_rh_matrix;
        rh_matrix.getInverse(inv_rh_matrix);
        glDisable(GL_DEPTH_TEST);
        setTextureUnits(normal_depth_texture,
                        depth_stencil_texture,
                        fb.getRTT()[0], fb.getRTT()[1], fb.getRTT()[2]);
        drawFullScreenEffect(rh_matrix, inv_rh_matrix, rh_extend);
    }   // render
};   // GlobalIlluminationReconstructionShader

// ============================================================================
class IBLShader : public TextureShader<IBLShader, 3>
{
public:
    IBLShader()
    {
        loadProgram(OBJECT, GL_VERTEX_SHADER, "screenquad.vert",
                            GL_FRAGMENT_SHADER, "IBL.frag");
        assignUniforms();
        assignSamplerNames(0, "ntex",  ST_NEAREST_FILTERED,
                           1, "dtex",  ST_NEAREST_FILTERED,
                           2, "probe", ST_TRILINEAR_CUBEMAP);
    }   // IBLShader
};   // IBLShader

// ============================================================================
class DegradedIBLShader : public TextureShader<DegradedIBLShader, 1>
{
public:
    DegradedIBLShader()
    {
        loadProgram(OBJECT, GL_VERTEX_SHADER, "screenquad.vert",
                            GL_FRAGMENT_SHADER, "degraded_ibl.frag");
        assignUniforms();
        assignSamplerNames(0, "ntex", ST_NEAREST_FILTERED);
    }   // DegradedIBLShader
};   // DegradedIBLShader

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
    void render(GLuint normal_depth_texture,
                GLuint depth_stencil_texture,
                const FrameBuffer& shadow_framebuffer)
    {
        setTextureUnits(normal_depth_texture,
                        depth_stencil_texture,
                        shadow_framebuffer.getDepthTexture()                );
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
    void render(GLuint normal_depth_texture,
                GLuint depth_stencil_texture,
                const FrameBuffer& shadow_framebuffer)
    {
        setTextureUnits(normal_depth_texture,
                        depth_stencil_texture,
                        shadow_framebuffer.getRTT()[0]);
        drawFullScreenEffect(ShadowMatrices::m_shadow_split[1],
                             ShadowMatrices::m_shadow_split[2],
                             ShadowMatrices::m_shadow_split[3],
                             ShadowMatrices::m_shadow_split[4]);
    }   // render
};   // ShadowedSunLightShaderESM


// ============================================================================
class SunLightShader : public TextureShader<SunLightShader, 2,
                                            core::vector3df, video::SColorf>
{
public:
    SunLightShader()
    {
        loadProgram(OBJECT, GL_VERTEX_SHADER, "screenquad.vert",
                            GL_FRAGMENT_SHADER, "sunlight.frag");

        assignSamplerNames(0, "ntex", ST_NEAREST_FILTERED,
                           1, "dtex", ST_NEAREST_FILTERED);
        assignUniforms("direction", "col");
    }   // SunLightShader
    // ------------------------------------------------------------------------
    void render(const core::vector3df &direction, const video::SColorf &col,
                GLuint normal_depth_texture,
                GLuint depth_stencil_texture)
    {
        glEnable(GL_BLEND);
        glDisable(GL_DEPTH_TEST);
        glBlendFunc(GL_ONE, GL_ONE);
        glBlendEquation(GL_FUNC_ADD);

        setTextureUnits(normal_depth_texture, depth_stencil_texture);
        drawFullScreenEffect(direction, col);
    }   // render
};   // SunLightShader

// ============================================================================
static void renderPointLights(unsigned count,
                              GLuint normal_depth_rander_target,
                              GLuint depth_stencil_texture)
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
        normal_depth_rander_target,
        depth_stencil_texture);
    PointLightShader::getInstance()->setUniforms();

    glDrawArraysInstanced(GL_TRIANGLE_STRIP, 0, 4, count);
}   // renderPointLights

// ----------------------------------------------------------------------------
void LightingPasses::renderEnvMap(GLuint normal_depth_texture,
                                  GLuint depth_stencil_texture,
                                  GLuint specular_probe)
{
    glDisable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendEquation(GL_FUNC_ADD);
    glBlendFunc(GL_ONE, GL_ONE);

    if (UserConfigParams::m_degraded_IBL)
    {
        DegradedIBLShader::getInstance()->use();
        glBindVertexArray(SharedGPUObjects::getFullScreenQuadVAO());

        DegradedIBLShader::getInstance()
            ->setTextureUnits(normal_depth_texture);
        DegradedIBLShader::getInstance()->setUniforms();
    }
    else
    {
        IBLShader::getInstance()->use();
        glBindVertexArray(SharedGPUObjects::getFullScreenQuadVAO());

        IBLShader::getInstance()->setTextureUnits(
            normal_depth_texture,
            depth_stencil_texture,
            specular_probe);
        IBLShader::getInstance()->setUniforms();
    }

    glDrawArrays(GL_TRIANGLES, 0, 3);
    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    glEnable(GL_DEPTH_TEST);
    glDisable(GL_BLEND);
}   // renderEnvMap

// ----------------------------------------------------------------------------
void LightingPasses::renderSunlight(const core::vector3df &direction,
                                    const video::SColorf &col,
                                    GLuint normal_depth_texture,
                                    GLuint depth_stencil_texture)
{
    SunLightShader::getInstance()->render(direction, col,
                                          normal_depth_texture,
                                          depth_stencil_texture);
}   // renderSunlight


// ----------------------------------------------------------------------------
void LightingPasses::updateLightsInfo(scene::ICameraSceneNode * const camnode,
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

    m_point_light_count = 0;
    bool multiplayer = (race_manager->getNumLocalPlayers() > 1);

    for (unsigned i = 0; i < 15; i++)
    {
        for (unsigned j = 0; j < BucketedLN[i].size(); j++)
        {
            if (++m_point_light_count >= LightBaseClass::MAXLIGHT)
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
                m_point_lights_info[m_point_light_count].posX = pos.X;
                m_point_lights_info[m_point_light_count].posY = pos.Y;
                m_point_lights_info[m_point_light_count].posZ = pos.Z;

                m_point_lights_info[m_point_light_count].energy =
                                              light_node->getEffectiveEnergy();

                const core::vector3df &col = light_node->getColor();
                m_point_lights_info[m_point_light_count].red = col.X;
                m_point_lights_info[m_point_light_count].green = col.Y;
                m_point_lights_info[m_point_light_count].blue = col.Z;

                // Light radius
                m_point_lights_info[m_point_light_count].radius = light_node->getRadius();
            }
        }
        if (m_point_light_count > LightBaseClass::MAXLIGHT)
        {
            irr_driver->setLastLightBucketDistance(i * 10);
            break;
        }
    }

    m_point_light_count++;
}   // updateLightsInfo

// ----------------------------------------------------------------------------
void LightingPasses::renderRadianceHints(  const ShadowMatrices& shadow_matrices,
                                           const FrameBuffer& radiance_hint_framebuffer,
                                           const FrameBuffer& reflective_shadow_map_framebuffer)
{
#if !defined(USE_GLES2)
    glDisable(GL_BLEND);
    radiance_hint_framebuffer.bind();
    glBindVertexArray(SharedGPUObjects::getFullScreenQuadVAO());
    if (CVS->needRHWorkaround())
    {
        NVWorkaroundRadianceHintsConstructionShader::getInstance()->use();
        NVWorkaroundRadianceHintsConstructionShader::getInstance()
            ->setTextureUnits(
                reflective_shadow_map_framebuffer.getRTT()[0],
                reflective_shadow_map_framebuffer.getRTT()[1],
                reflective_shadow_map_framebuffer.getDepthTexture());
        for (unsigned i = 0; i < 32; i++)
        {
            NVWorkaroundRadianceHintsConstructionShader::getInstance()
                ->setUniforms(shadow_matrices.getRSMMatrix(),
                              shadow_matrices.getRHMatrix(),
                              shadow_matrices.getRHExtend(), i,
                              irr_driver->getSunColor());
            glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
        }
    }
    else
    {
        RadianceHintsConstructionShader::getInstance()->use();
        RadianceHintsConstructionShader::getInstance()
            ->setTextureUnits(
                reflective_shadow_map_framebuffer.getRTT()[0],
                reflective_shadow_map_framebuffer.getRTT()[1],
                reflective_shadow_map_framebuffer.getDepthTexture()
        );
        RadianceHintsConstructionShader::getInstance()
            ->setUniforms(shadow_matrices.getRSMMatrix(),
                          shadow_matrices.getRHMatrix(),
                          shadow_matrices.getRHExtend(),
                          irr_driver->getSunColor());
        glDrawArraysInstanced(GL_TRIANGLE_STRIP, 0, 4, 32);
    }
#endif //!defined(USE_GLES2)
}   // renderRadianceHints

// ----------------------------------------------------------------------------
void LightingPasses::renderGlobalIllumination(  const ShadowMatrices& shadow_matrices,
                                                const FrameBuffer& radiance_hint_framebuffer,
                                                GLuint normal_depth_texture,
                                                GLuint depth_stencil_texture)
{
    GlobalIlluminationReconstructionShader::getInstance()
        ->render(shadow_matrices.getRHMatrix(), shadow_matrices.getRHExtend(),
                 radiance_hint_framebuffer, normal_depth_texture,
                 depth_stencil_texture);
}

// ----------------------------------------------------------------------------
void LightingPasses::renderLights(  bool has_shadow,
                                    GLuint normal_depth_texture,
                                    GLuint depth_stencil_texture,
                                    const FrameBuffer& shadow_framebuffer,
                                    GLuint specular_probe)
{
    {
        ScopedGPUTimer timer(irr_driver->getGPUTimer(Q_ENVMAP));
        renderEnvMap(normal_depth_texture,
                     depth_stencil_texture,
                     specular_probe);
    }

    // Render sunlight if and only if track supports shadow
    const Track* const track = Track::getCurrentTrack();
    if (!track|| track->hasShadows())
    {
        ScopedGPUTimer timer(irr_driver->getGPUTimer(Q_SUN));
        if (World::getWorld() && CVS->isShadowEnabled() && has_shadow)
        {
            glEnable(GL_BLEND);
            glDisable(GL_DEPTH_TEST);
            glBlendFunc(GL_ONE, GL_ONE);
            glBlendEquation(GL_FUNC_ADD);


            if (CVS->isESMEnabled())
            {
                ShadowedSunLightShaderESM::getInstance()->render(normal_depth_texture,
                                                                 depth_stencil_texture,
                                                                 shadow_framebuffer);
            }
            else
            {
                ShadowedSunLightShaderPCF::getInstance()->render(normal_depth_texture,
                                                                 depth_stencil_texture,
                                                                 shadow_framebuffer);
            }
        }
        else
            renderSunlight(irr_driver->getSunDirection(),
                           irr_driver->getSunColor(),
                           normal_depth_texture,
                           depth_stencil_texture);
    }

    //points lights
    {
        ScopedGPUTimer timer(irr_driver->getGPUTimer(Q_POINTLIGHTS));
        renderPointLights(std::min(m_point_light_count, LightBaseClass::MAXLIGHT),
                          normal_depth_texture,
                          depth_stencil_texture);
    }
}   // renderLights

// ----------------------------------------------------------------------------
void LightingPasses::renderAmbientScatter(GLuint depth_stencil_texture)
{
    const Track * const track = Track::getCurrentTrack();

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

    FogShader::getInstance()->render(start, col, depth_stencil_texture);
}   // renderAmbientScatter

// ----------------------------------------------------------------------------
void LightingPasses::renderLightsScatter(GLuint depth_stencil_texture,
                                         const FrameBuffer& half1_framebuffer,
                                         const FrameBuffer& half2_framebuffer,
                                         const FrameBuffer& colors_framebuffer,
                                         const PostProcessing* post_processing)
{
    half1_framebuffer.bind();
    glClearColor(0., 0., 0., 0.);
    glClear(GL_COLOR_BUFFER_BIT);

    const Track * const track = Track::getCurrentTrack();

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
        ->setTextureUnits(depth_stencil_texture);
    PointLightScatterShader::getInstance()
        ->setUniforms(1.f / (40.f * start), col2);

    glDrawArraysInstanced(GL_TRIANGLE_STRIP, 0, 4,
                          std::min(m_point_light_count,
                          LightBaseClass::MAXLIGHT));

    glDisable(GL_BLEND);
    post_processing->renderGaussian6Blur(half1_framebuffer,
                                         half2_framebuffer, 5., 5.);
    glEnable(GL_BLEND);

    glDisable(GL_DEPTH_TEST);
    glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
    colors_framebuffer.bind();
    post_processing->renderPassThrough(half1_framebuffer.getRTT()[0],
                                       colors_framebuffer.getWidth(),
                                       colors_framebuffer.getHeight());
}   // renderLightsScatter

#endif
