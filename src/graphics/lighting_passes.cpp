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
#include "graphics/frame_buffer.hpp"
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

#include <ICameraSceneNode.h>

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

        glVertexAttribDivisor(attrib_Position, 1);
        glVertexAttribDivisor(attrib_Energy, 1);
        glVertexAttribDivisor(attrib_Color, 1);
        glVertexAttribDivisor(attrib_Radius, 1);
    }   // PointLightShader
    ~PointLightShader()
    {
        glDeleteBuffers(1, &vbo);
        glDeleteVertexArrays(1, &vao);
    }   // PointLightShader
};   // PointLightShader




// ============================================================================
class PointLightScatterShader : public TextureShader<PointLightScatterShader,
                                                     1, float, core::vector3df>
{
public:
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

        glVertexAttribDivisor(attrib_Position, 1);
        glVertexAttribDivisor(attrib_Energy, 1);
        glVertexAttribDivisor(attrib_Color, 1);
        glVertexAttribDivisor(attrib_Radius, 1);
    }   // PointLightScatterShader
    ~PointLightScatterShader()
    {
        glDeleteVertexArrays(1, &vao);
    }   // ~PointLightScatterShader
};

// ============================================================================
class IBLShader : public TextureShader<IBLShader, 4>
{
public:
    IBLShader()
    {
        loadProgram(OBJECT, GL_VERTEX_SHADER, "screenquad.vert",
                            GL_FRAGMENT_SHADER, "IBL.frag");
        assignUniforms();
        assignSamplerNames(0, "ntex",  ST_NEAREST_FILTERED,
                           1, "dtex",  ST_NEAREST_FILTERED,
                           2, "probe", ST_TRILINEAR_CUBEMAP,
                           3, "albedo",ST_NEAREST_FILTERED);
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
                                                       float, float,
                                                       core::vector3df, video::SColorf>
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
        assignUniforms("split0", "split1", "split2", "splitmax", "shadow_res",
            "sundirection", "sun_color");
    }   // ShadowedSunLightShaderPCF
    // ------------------------------------------------------------------------
    void render(GLuint normal_depth_texture,
                GLuint depth_stencil_texture,
                const FrameBuffer* shadow_framebuffer,
                const core::vector3df &direction,
                const video::SColorf &col)
    {
        setTextureUnits(normal_depth_texture,
                        depth_stencil_texture,
                        shadow_framebuffer->getDepthTexture()                );
       drawFullScreenEffect(ShadowMatrices::m_shadow_split[1],
                            ShadowMatrices::m_shadow_split[2],
                            ShadowMatrices::m_shadow_split[3],
                            ShadowMatrices::m_shadow_split[4],
                            float(UserConfigParams::m_shadows_resolution),
                            direction, col);

    }    // render
};   // ShadowedSunLightShaderPCF

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
        assignUniforms("sundirection", "sun_color");
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

    glBindBuffer(GL_ARRAY_BUFFER, PointLightShader::getInstance()->vbo);
    glBufferSubData(GL_ARRAY_BUFFER, 0,
                     count * sizeof(LightBaseClass::PointLightInfo),
                     m_point_lights_info);

    glBindVertexArray(PointLightShader::getInstance()->vao);
    PointLightShader::getInstance()->setTextureUnits(
        normal_depth_rander_target,
        depth_stencil_texture);
    PointLightShader::getInstance()->setUniforms();

    glDrawArraysInstanced(GL_TRIANGLE_STRIP, 0, 4, count);
}   // renderPointLights

// ----------------------------------------------------------------------------
void LightingPasses::renderEnvMap(GLuint normal_depth_texture,
                                  GLuint depth_stencil_texture,
                                  GLuint specular_probe,
                                  GLuint albedo_buffer)
{
    glDisable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendEquation(GL_FUNC_ADD);
    glBlendFunc(GL_ONE, GL_ONE);

    if (specular_probe == 0 || UserConfigParams::m_degraded_IBL)
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
            specular_probe,
            albedo_buffer);
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
    bool multiplayer = (RaceManager::get()->getNumLocalPlayers() > 1);

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
void LightingPasses::renderLights(  bool has_shadow,
                                    GLuint normal_depth_texture,
                                    GLuint depth_stencil_texture,
                                    GLuint albedo_texture,
                                    const FrameBuffer* shadow_framebuffer,
                                    GLuint specular_probe)
{
    {
        ScopedGPUTimer timer(irr_driver->getGPUTimer(Q_ENVMAP));
        renderEnvMap(normal_depth_texture,
                     depth_stencil_texture,
                     specular_probe,
                     albedo_texture);
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
            ShadowedSunLightShaderPCF::getInstance()->render(normal_depth_texture,
                                                             depth_stencil_texture,
                                                             shadow_framebuffer,
                                                             irr_driver->getSunDirection(),
                                                             irr_driver->getSunColor());
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
void LightingPasses::renderLightsScatter(GLuint depth_stencil_texture,
                                         const FrameBuffer& half1_framebuffer,
                                         const FrameBuffer& half2_framebuffer,
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
}   // renderLightsScatter

#endif
