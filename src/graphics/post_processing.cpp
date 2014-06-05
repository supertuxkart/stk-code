//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2011-2013 the SuperTuxKart-Team
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
//  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA

#include "post_processing.hpp"

#include "config/user_config.hpp"
#include "graphics/callbacks.hpp"
#include "graphics/camera.hpp"
#include "graphics/glwrap.hpp"
#include "graphics/irr_driver.hpp"
#include "graphics/mlaa_areamap.hpp"
#include "graphics/shaders.hpp"
#include "graphics/stkmeshscenenode.hpp"
#include "io/file_manager.hpp"
#include "karts/abstract_kart.hpp"
#include "karts/kart_model.hpp"
#include "modes/world.hpp"
#include "race/race_manager.hpp"
#include "tracks/track.hpp"
#include "utils/log.hpp"
#include "utils/profiler.hpp"

#include <SViewFrustum.h>

using namespace video;
using namespace scene;

PostProcessing::PostProcessing(IVideoDriver* video_driver)
{
    // Initialization
    m_material.Wireframe = false;
    m_material.Lighting = false;
    m_material.ZWriteEnable = false;
    m_material.ZBuffer = ECFN_ALWAYS;
    m_material.setFlag(EMF_TRILINEAR_FILTER, true);

    for (u32 i = 0; i < MATERIAL_MAX_TEXTURES; ++i)
    {
        m_material.TextureLayer[i].TextureWrapU =
        m_material.TextureLayer[i].TextureWrapV = ETC_CLAMP_TO_EDGE;
        }

    // Load the MLAA area map
    io::IReadFile *areamap = irr_driver->getDevice()->getFileSystem()->
                         createMemoryReadFile((void *) AreaMap33, sizeof(AreaMap33),
                         "AreaMap33", false);
    if (!areamap)
    {
        Log::fatal("postprocessing", "Failed to load the areamap");
        return;
    }
    m_areamap = irr_driver->getVideoDriver()->getTexture(areamap);
    areamap->drop();

}   // PostProcessing

// ----------------------------------------------------------------------------
PostProcessing::~PostProcessing()
{
    // TODO: do we have to delete/drop anything?
}   // ~PostProcessing

// ----------------------------------------------------------------------------
/** Initialises post processing at the (re-)start of a race. This sets up
 *  the vertices, normals and texture coordinates for each
 */
void PostProcessing::reset()
{
    const u32 n = Camera::getNumCameras();
    m_boost_time.resize(n);
    m_vertices.resize(n);
    m_center.resize(n);
    m_direction.resize(n);

    MotionBlurProvider * const cb = (MotionBlurProvider *) irr_driver->
                                                           getCallback(ES_MOTIONBLUR);

    for(unsigned int i=0; i<n; i++)
    {
        m_boost_time[i] = 0.0f;

        const core::recti &vp = Camera::getCamera(i)->getViewport();
        // Map viewport to [-1,1] x [-1,1]. First define the coordinates
        // left, right, top, bottom:
        float right  = vp.LowerRightCorner.X < UserConfigParams::m_width
                     ? 0.0f : 1.0f;
        float left   = vp.UpperLeftCorner.X  > 0.0f ? 0.0f : -1.0f;
        float top    = vp.UpperLeftCorner.Y  > 0.0f ? 0.0f : 1.0f;
        float bottom = vp.LowerRightCorner.Y < UserConfigParams::m_height
                     ? 0.0f : -1.0f;

        // Use left etc to define 4 vertices on which the rendered screen
        // will be displayed:
        m_vertices[i].v0.Pos = core::vector3df(left,  bottom, 0);
        m_vertices[i].v1.Pos = core::vector3df(left,  top,    0);
        m_vertices[i].v2.Pos = core::vector3df(right, top,    0);
        m_vertices[i].v3.Pos = core::vector3df(right, bottom, 0);
        // Define the texture coordinates of each vertex, which must
        // be in [0,1]x[0,1]
        m_vertices[i].v0.TCoords  = core::vector2df(left  ==-1.0f ? 0.0f : 0.5f,
                                                    bottom==-1.0f ? 0.0f : 0.5f);
        m_vertices[i].v1.TCoords  = core::vector2df(left  ==-1.0f ? 0.0f : 0.5f,
                                                    top   == 1.0f ? 1.0f : 0.5f);
        m_vertices[i].v2.TCoords  = core::vector2df(right == 0.0f ? 0.5f : 1.0f,
                                                    top   == 1.0f ? 1.0f : 0.5f);
        m_vertices[i].v3.TCoords  = core::vector2df(right == 0.0f ? 0.5f : 1.0f,
                                                    bottom==-1.0f ? 0.0f : 0.5f);
        // Set normal and color:
        core::vector3df normal(0,0,1);
        m_vertices[i].v0.Normal = m_vertices[i].v1.Normal =
        m_vertices[i].v2.Normal = m_vertices[i].v3.Normal = normal;
        SColor white(0xFF, 0xFF, 0xFF, 0xFF);
        m_vertices[i].v0.Color  = m_vertices[i].v1.Color  =
        m_vertices[i].v2.Color  = m_vertices[i].v3.Color  = white;

        m_center[i].X=(m_vertices[i].v0.TCoords.X
                      +m_vertices[i].v2.TCoords.X) * 0.5f;

        // Center is around 20 percent from bottom of screen:
        const float tex_height = m_vertices[i].v1.TCoords.Y
                         - m_vertices[i].v0.TCoords.Y;
        m_direction[i].X = m_center[i].X;
        m_direction[i].Y = m_vertices[i].v0.TCoords.Y + 0.7f*tex_height;

        setMotionBlurCenterY(i, 0.2f);

        cb->setDirection(i, m_direction[i].X, m_direction[i].Y);
        cb->setMaxHeight(i, m_vertices[i].v1.TCoords.Y);
    }  // for i <number of cameras
}   // reset

void PostProcessing::setMotionBlurCenterY(const u32 num, const float y)
{
    MotionBlurProvider * const cb = (MotionBlurProvider *) irr_driver->
                                                           getCallback(ES_MOTIONBLUR);

    const float tex_height = m_vertices[num].v1.TCoords.Y - m_vertices[num].v0.TCoords.Y;
    m_center[num].Y = m_vertices[num].v0.TCoords.Y + y * tex_height;

    cb->setCenter(num, m_center[num].X, m_center[num].Y);
}

// ----------------------------------------------------------------------------
/** Setup some PP data.
  */
void PostProcessing::begin()
{
    m_any_boost = false;
    for (u32 i = 0; i < m_boost_time.size(); i++)
        m_any_boost |= m_boost_time[i] > 0.01f;
}   // beginCapture

// ----------------------------------------------------------------------------
/** Set the boost amount according to the speed of the camera */
void PostProcessing::giveBoost(unsigned int camera_index)
{
    if (irr_driver->isGLSL())
    {
        m_boost_time[camera_index] = 0.75f;

        MotionBlurProvider * const cb = (MotionBlurProvider *)irr_driver->
            getCallback(ES_MOTIONBLUR);
        cb->setBoostTime(camera_index, m_boost_time[camera_index]);
    }
}   // giveBoost

// ----------------------------------------------------------------------------
/** Updates the boost times for all cameras, called once per frame.
 *  \param dt Time step size.
 */
void PostProcessing::update(float dt)
{
    if (!irr_driver->isGLSL())
        return;

    MotionBlurProvider* const cb =
        (MotionBlurProvider*) irr_driver->getCallback(ES_MOTIONBLUR);

    if (cb == NULL) return;

    for (unsigned int i=0; i<m_boost_time.size(); i++)
    {
        if (m_boost_time[i] > 0.0f)
        {
            m_boost_time[i] -= dt;
            if (m_boost_time[i] < 0.0f) m_boost_time[i] = 0.0f;
        }

        cb->setBoostTime(i, m_boost_time[i]);
    }
}   // update

static
void renderBloom(GLuint in)
{
    const float threshold = World::getWorld()->getTrack()->getBloomThreshold();
    glUseProgram(FullScreenShader::BloomShader::Program);
    glBindVertexArray(FullScreenShader::BloomShader::vao);

    setTexture(0, in, GL_NEAREST, GL_NEAREST);
    FullScreenShader::BloomShader::setUniforms(0);

    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
}

static
void renderColorLevel(GLuint in)
{
    core::vector3df m_inlevel = World::getWorld()->getTrack()->getColorLevelIn();
    core::vector2df m_outlevel = World::getWorld()->getTrack()->getColorLevelOut();


    glUseProgram(FullScreenShader::ColorLevelShader::Program);
    glBindVertexArray(FullScreenShader::ColorLevelShader::vao);
    glUniform3f(FullScreenShader::ColorLevelShader::uniform_inlevel, m_inlevel.X, m_inlevel.Y, m_inlevel.Z);
    glUniform2f(FullScreenShader::ColorLevelShader::uniform_outlevel, m_outlevel.X, m_outlevel.Y);

    setTexture(0, in, GL_NEAREST, GL_NEAREST);
    setTexture(1, irr_driver->getDepthStencilTexture(), GL_NEAREST, GL_NEAREST);
    glUniform1i(FullScreenShader::ColorLevelShader::uniform_tex, 0);
    glUniform1i(FullScreenShader::ColorLevelShader::uniform_dtex, 1);
    setTexture(2, irr_driver->getRenderTargetTexture(RTT_LOG_LUMINANCE), GL_NEAREST, GL_NEAREST_MIPMAP_NEAREST);
    glUniformMatrix4fv(FullScreenShader::ColorLevelShader::uniform_invprojm, 1, GL_FALSE, irr_driver->getInvProjMatrix().pointer());

    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
}

void PostProcessing::renderDiffuseEnvMap(const float *bSHCoeff, const float *gSHCoeff, const float *rSHCoeff)
{
    glDisable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendEquation(GL_FUNC_ADD);
    glBlendFunc(GL_ONE, GL_ONE);

    glUseProgram(FullScreenShader::DiffuseEnvMapShader::Program);
    glBindVertexArray(FullScreenShader::DiffuseEnvMapShader::vao);

    setTexture(0, irr_driver->getRenderTargetTexture(RTT_NORMAL_AND_DEPTH), GL_NEAREST, GL_NEAREST);
    core::matrix4 TVM = irr_driver->getViewMatrix().getTransposed();
    FullScreenShader::DiffuseEnvMapShader::setUniforms(TVM, bSHCoeff, gSHCoeff, rSHCoeff, 0);

    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    glEnable(GL_DEPTH_TEST);
    glDisable(GL_BLEND);
}

void PostProcessing::renderGI(const core::matrix4 &RHMatrix, const core::vector3df &rh_extend, GLuint shr, GLuint shg, GLuint shb)
{
    glDisable(GL_DEPTH_TEST);
    glUseProgram(FullScreenShader::GlobalIlluminationReconstructionShader::Program);
    glBindVertexArray(FullScreenShader::GlobalIlluminationReconstructionShader::vao);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_3D, shr);
    {
        glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    }
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_3D, shg);
    {
        glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    }
    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_3D, shb);
    {
        glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    }
    setTexture(3, irr_driver->getRenderTargetTexture(RTT_NORMAL_AND_DEPTH), GL_NEAREST, GL_NEAREST);
    setTexture(4, irr_driver->getDepthStencilTexture(), GL_NEAREST, GL_NEAREST);
    FullScreenShader::GlobalIlluminationReconstructionShader::setUniforms(RHMatrix, rh_extend, 3, 4, 0, 1, 2);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
}

void PostProcessing::renderSunlight()
{
  SunLightProvider * const cb = (SunLightProvider *) irr_driver->getCallback(ES_SUNLIGHT);

  glEnable(GL_BLEND);
  glDisable(GL_DEPTH_TEST);
  glBlendFunc(GL_ONE, GL_ONE);
  glBlendEquation(GL_FUNC_ADD);

  glUseProgram(FullScreenShader::SunLightShader::Program);
  glBindVertexArray(FullScreenShader::SunLightShader::vao);
  setTexture(0, irr_driver->getRenderTargetTexture(RTT_NORMAL_AND_DEPTH), GL_NEAREST, GL_NEAREST);
  setTexture(1, irr_driver->getDepthStencilTexture(), GL_NEAREST, GL_NEAREST);
  FullScreenShader::SunLightShader::setUniforms(cb->getPosition(), cb->getRed(), cb->getGreen(), cb->getBlue(), 0, 1);
  glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
  glBindVertexArray(0);
}

void PostProcessing::renderShadowedSunlight(const std::vector<core::matrix4> &sun_ortho_matrix, GLuint depthtex)
{
    SunLightProvider * const cb = (SunLightProvider *)irr_driver->getCallback(ES_SUNLIGHT);

    glEnable(GL_BLEND);
    glDisable(GL_DEPTH_TEST);
    glBlendFunc(GL_ONE, GL_ONE);
    glBlendEquation(GL_FUNC_ADD);

    setTexture(0, irr_driver->getRenderTargetTexture(RTT_NORMAL_AND_DEPTH), GL_NEAREST, GL_NEAREST);
    setTexture(1, irr_driver->getDepthStencilTexture(), GL_NEAREST, GL_NEAREST);
    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D_ARRAY, depthtex);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameterf(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_REF_TO_TEXTURE);
    glTexParameterf(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL);

    if (irr_driver->getShadowViz())
    {
        glUseProgram(FullScreenShader::ShadowedSunLightDebugShader::Program);
        glBindVertexArray(FullScreenShader::ShadowedSunLightDebugShader::vao);
        FullScreenShader::ShadowedSunLightDebugShader::setUniforms(cb->getPosition(), cb->getRed(), cb->getGreen(), cb->getBlue(), 0, 1, 2);

    }
    else
    {
        glUseProgram(FullScreenShader::ShadowedSunLightShader::Program);
        glBindVertexArray(FullScreenShader::ShadowedSunLightShader::vao);
        FullScreenShader::ShadowedSunLightShader::setUniforms(cb->getPosition(), cb->getRed(), cb->getGreen(), cb->getBlue(), 0, 1, 2);
    }
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    glBindVertexArray(0);
}


void PostProcessing::renderGaussian3Blur(FrameBuffer &in_fbo, FrameBuffer &auxiliary)
{
    assert(in_fbo.getWidth() == auxiliary.getWidth() && in_fbo.getHeight() == auxiliary.getHeight());
    float inv_width = 1.0f / in_fbo.getWidth(), inv_height = 1.0f / in_fbo.getHeight();
    {
        auxiliary.Bind();
        glUseProgram(FullScreenShader::Gaussian3VBlurShader::Program);
        glBindVertexArray(FullScreenShader::Gaussian3VBlurShader::vao);

        glUniform2f(FullScreenShader::Gaussian3VBlurShader::uniform_pixel, inv_width, inv_height);

        setTexture(0, in_fbo.getRTT()[0], GL_LINEAR, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glUniform1i(FullScreenShader::Gaussian3VBlurShader::uniform_tex, 0);

        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    }
    {
        in_fbo.Bind();
        glUseProgram(FullScreenShader::Gaussian3HBlurShader::Program);
        glBindVertexArray(FullScreenShader::Gaussian3HBlurShader::vao);

        glUniform2f(FullScreenShader::Gaussian3HBlurShader::uniform_pixel, inv_width, inv_height);

        setTexture(0, auxiliary.getRTT()[0], GL_LINEAR, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glUniform1i(FullScreenShader::Gaussian3HBlurShader::uniform_tex, 0);

        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    }
}

void PostProcessing::renderGaussian6Blur(FrameBuffer &in_fbo, FrameBuffer &auxiliary)
{
    assert(in_fbo.getWidth() == auxiliary.getWidth() && in_fbo.getHeight() == auxiliary.getHeight());
    float inv_width = 1.0f / in_fbo.getWidth(), inv_height = 1.0f / in_fbo.getHeight();
    {
        auxiliary.Bind();
        glUseProgram(FullScreenShader::Gaussian6VBlurShader::Program);
        glBindVertexArray(FullScreenShader::Gaussian6VBlurShader::vao);

        glUniform2f(FullScreenShader::Gaussian6VBlurShader::uniform_pixel, inv_width, inv_height);

        setTexture(0, in_fbo.getRTT()[0], GL_LINEAR, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glUniform1i(FullScreenShader::Gaussian6VBlurShader::uniform_tex, 0);

        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    }
    {
        in_fbo.Bind();
        glUseProgram(FullScreenShader::Gaussian6HBlurShader::Program);
        glBindVertexArray(FullScreenShader::Gaussian6HBlurShader::vao);

        glUniform2f(FullScreenShader::Gaussian6HBlurShader::uniform_pixel, inv_width, inv_height);

        setTexture(0, auxiliary.getRTT()[0], GL_LINEAR, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glUniform1i(FullScreenShader::Gaussian6HBlurShader::uniform_tex, 0);

        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    }
}

void PostProcessing::renderGaussian17TapBlur(FrameBuffer &in_fbo, FrameBuffer &auxiliary)
{
    assert(in_fbo.getWidth() == auxiliary.getWidth() && in_fbo.getHeight() == auxiliary.getHeight());
    float inv_width = 1.0f / in_fbo.getWidth(), inv_height = 1.0f / in_fbo.getHeight();
    {
        auxiliary.Bind();
        glUseProgram(FullScreenShader::Gaussian17TapHShader::Program);
        glBindVertexArray(FullScreenShader::Gaussian17TapHShader::vao);

        glUniform2f(FullScreenShader::Gaussian17TapHShader::uniform_pixel, inv_width, inv_height);

        setTexture(0, in_fbo.getRTT()[0], GL_LINEAR, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glUniform1i(FullScreenShader::Gaussian17TapHShader::uniform_tex, 0);

        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    }
    {
        in_fbo.Bind();
        glUseProgram(FullScreenShader::Gaussian17TapVShader::Program);
        glBindVertexArray(FullScreenShader::Gaussian17TapVShader::vao);

        glUniform2f(FullScreenShader::Gaussian17TapVShader::uniform_pixel, inv_width, inv_height);

        setTexture(0, auxiliary.getRTT()[0], GL_LINEAR, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glUniform1i(FullScreenShader::Gaussian17TapVShader::uniform_tex, 0);

        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    }
}

void PostProcessing::renderPassThrough(GLuint tex)
{

    glUseProgram(FullScreenShader::PassThroughShader::Program);
    glBindVertexArray(FullScreenShader::PassThroughShader::vao);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, tex);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glUniform1i(FullScreenShader::PassThroughShader::uniform_texture, 0);

    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
}

void PostProcessing::renderGlow(unsigned tex)
{

    glUseProgram(FullScreenShader::GlowShader::Program);
    glBindVertexArray(FullScreenShader::GlowShader::vao);

    setTexture(0, tex, GL_LINEAR, GL_LINEAR);
    glUniform1i(FullScreenShader::GlowShader::uniform_tex, 0);

    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
}

ITexture *noise_tex = 0;

void PostProcessing::renderSSAO()
{
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_BLEND);
    glDisable(GL_ALPHA_TEST);

    // Generate linear depth buffer
    irr_driver->getFBO(FBO_LINEAR_DEPTH).Bind();
    glUseProgram(FullScreenShader::LinearizeDepthShader::Program);
    glBindVertexArray(FullScreenShader::LinearizeDepthShader::vao);
    setTexture(0, irr_driver->getDepthStencilTexture(), GL_LINEAR, GL_LINEAR);
    FullScreenShader::LinearizeDepthShader::setUniforms(irr_driver->getSceneManager()->getActiveCamera()->getNearValue(), irr_driver->getSceneManager()->getActiveCamera()->getFarValue(), 0);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    irr_driver->getFBO(FBO_SSAO).Bind();

    if (!noise_tex)
        noise_tex = irr_driver->getTexture(file_manager->getAsset("textures/noise.png").c_str());

    glUseProgram(FullScreenShader::SSAOShader::Program);
    glBindVertexArray(FullScreenShader::SSAOShader::vao);

    setTexture(0, irr_driver->getRenderTargetTexture(RTT_LINEAR_DEPTH), GL_LINEAR, GL_LINEAR_MIPMAP_LINEAR);
    glGenerateMipmap(GL_TEXTURE_2D);
    setTexture(1, getTextureGLuint(noise_tex), GL_LINEAR, GL_LINEAR);

    FullScreenShader::SSAOShader::setUniforms(core::vector2df(float(UserConfigParams::m_width), 
                                                              float(UserConfigParams::m_height)),
                                              0, 1);

    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
}

void PostProcessing::renderFog()
{
    const Track * const track = World::getWorld()->getTrack();

    // This function is only called once per frame - thus no need for setters.
    const float fogmax = track->getFogMax();
    const float startH = track->getFogStartHeight();
    const float endH = track->getFogEndHeight();
    const float start = track->getFogStart();
    const float end = track->getFogEnd();
    const SColor tmpcol = track->getFogColor();

    core::vector3df col( tmpcol.getRed() / 255.0f,
        tmpcol.getGreen() / 255.0f,
        tmpcol.getBlue() / 255.0f );

    glDisable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendEquation(GL_FUNC_ADD);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glUseProgram(FullScreenShader::FogShader::Program);
    glBindVertexArray(FullScreenShader::FogShader::vao);

    setTexture(0, irr_driver->getDepthStencilTexture(), GL_NEAREST, GL_NEAREST);
    FullScreenShader::FogShader::setUniforms(fogmax, startH, endH, start, end, col, 0);

    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    glEnable(GL_DEPTH_TEST);
    glDisable(GL_BLEND);
}

void PostProcessing::renderMotionBlur(unsigned cam, FrameBuffer &in_fbo, FrameBuffer &out_fbo)
{
    MotionBlurProvider * const cb = (MotionBlurProvider *)irr_driver->
        getCallback(ES_MOTIONBLUR);

    scene::ICameraSceneNode * const camnode =
        Camera::getCamera(cam)->getCameraSceneNode();
    // Calculate the kart's Y position on screen
    const core::vector3df pos =
        Camera::getCamera(cam)->getKart()->getNode()->getPosition();
    float ndc[4];
    core::matrix4 trans = camnode->getProjectionMatrix();
    trans *= camnode->getViewMatrix();

    trans.transformVect(ndc, pos);
    const float karty = (ndc[1] / ndc[3]) * 0.5f + 0.5f;
    setMotionBlurCenterY(cam, karty);

    out_fbo.Bind();
    glClear(GL_COLOR_BUFFER_BIT);

    glUseProgram(FullScreenShader::MotionBlurShader::Program);
    glBindVertexArray(FullScreenShader::MotionBlurShader::vao);

    setTexture(0, in_fbo.getRTT()[0], GL_NEAREST, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    FullScreenShader::MotionBlurShader
                    ::setUniforms(cb->getBoostTime(cam), cb->getCenter(cam),
                                  cb->getDirection(cam), 0.15f,
                                  cb->getMaxHeight(cam) * 0.7f, 0);

    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
}

static void renderGodFade(GLuint tex, const SColor &col)
{
    glUseProgram(FullScreenShader::GodFadeShader::Program);
    glBindVertexArray(FullScreenShader::GodFadeShader::vao);
    setTexture(0, tex, GL_LINEAR, GL_LINEAR);
    FullScreenShader::GodFadeShader::setUniforms(col, 0);

    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
}

static void renderGodRay(GLuint tex, const core::vector2df &sunpos)
{
    glUseProgram(FullScreenShader::GodRayShader::Program);
    glBindVertexArray(FullScreenShader::GodRayShader::vao);
    setTexture(0, tex, GL_LINEAR, GL_LINEAR);
    FullScreenShader::GodRayShader::setUniforms(sunpos, 0);

    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
}

static void toneMap(FrameBuffer &fbo, GLuint rtt)
{
    fbo.Bind();
    glUseProgram(FullScreenShader::ToneMapShader::Program);
    glBindVertexArray(FullScreenShader::ToneMapShader::vao);
    setTexture(0, rtt, GL_NEAREST, GL_NEAREST);
    setTexture(1, irr_driver->getRenderTargetTexture(RTT_LOG_LUMINANCE), GL_NEAREST, GL_NEAREST_MIPMAP_NEAREST);
    FullScreenShader::ToneMapShader::setUniforms(irr_driver->getExposure(), irr_driver->getLwhite(), 0, 1);

    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
}

static void renderDoF(FrameBuffer &fbo, GLuint rtt)
{
    fbo.Bind();
    glUseProgram(FullScreenShader::DepthOfFieldShader::Program);
    glBindVertexArray(FullScreenShader::DepthOfFieldShader::vao);
    setTexture(0, rtt, GL_LINEAR, GL_LINEAR);
    setTexture(1, irr_driver->getDepthStencilTexture(), GL_NEAREST, GL_NEAREST);
    FullScreenShader::DepthOfFieldShader::setUniforms(0, 1);

    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
}

static void averageTexture(GLuint tex)
{
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, tex);
    glGenerateMipmap(GL_TEXTURE_2D);
}

static void computeLogLuminance(GLuint tex)
{
    irr_driver->getFBO(FBO_LOG_LUMINANCE).Bind();
    glUseProgram(FullScreenShader::LogLuminanceShader::Program);
    glBindVertexArray(FullScreenShader::LogLuminanceShader::vao);
    setTexture(0, tex, GL_LINEAR, GL_LINEAR);
    FullScreenShader::LogLuminanceShader::setUniforms(0);

    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    averageTexture(irr_driver->getRenderTargetTexture(RTT_LOG_LUMINANCE));
    glViewport(0, 0, UserConfigParams::m_width, UserConfigParams::m_height);
}

void PostProcessing::applyMLAA()
{
    const core::vector2df &PIXEL_SIZE = core::vector2df(1.0f / UserConfigParams::m_width, 1.0f / UserConfigParams::m_height);
    IVideoDriver *const drv = irr_driver->getVideoDriver();
    irr_driver->getFBO(FBO_TMP1_WITH_DS).Bind();
    glEnable(GL_STENCIL_TEST);
    glClearColor(0.0, 0.0, 0.0, 1.0);
    glClear(GL_STENCIL_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
    glStencilFunc(GL_ALWAYS, 1, ~0);
    glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);

    // Pass 1: color edge detection
    setTexture(0, irr_driver->getRenderTargetTexture(RTT_MLAA_COLORS), GL_NEAREST, GL_NEAREST);
    glUseProgram(FullScreenShader::MLAAColorEdgeDetectionSHader::Program);
    FullScreenShader::MLAAColorEdgeDetectionSHader::setUniforms(PIXEL_SIZE, 0);

    glBindVertexArray(FullScreenShader::MLAAColorEdgeDetectionSHader::vao);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

    glStencilFunc(GL_EQUAL, 1, ~0);
    glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);

    // Pass 2: blend weights
    irr_driver->getFBO(FBO_TMP2_WITH_DS).Bind();
    glClear(GL_COLOR_BUFFER_BIT);

    glUseProgram(FullScreenShader::MLAABlendWeightSHader::Program);
    setTexture(0, irr_driver->getRenderTargetTexture(RTT_TMP1), GL_LINEAR, GL_LINEAR);
    setTexture(1, getTextureGLuint(m_areamap), GL_NEAREST, GL_NEAREST);
    FullScreenShader::MLAABlendWeightSHader::setUniforms(PIXEL_SIZE, 0, 1);

    glBindVertexArray(FullScreenShader::MLAABlendWeightSHader::vao);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

    // Blit in to tmp1
    FrameBuffer::Blit(irr_driver->getFBO(FBO_MLAA_COLORS), irr_driver->getFBO(FBO_TMP1_WITH_DS));

    // Pass 3: gather
    irr_driver->getFBO(FBO_MLAA_COLORS).Bind();

    glUseProgram(FullScreenShader::MLAAGatherSHader::Program);
    setTexture(0, irr_driver->getRenderTargetTexture(RTT_TMP1), GL_NEAREST, GL_NEAREST);
    setTexture(1, irr_driver->getRenderTargetTexture(RTT_TMP2), GL_NEAREST, GL_NEAREST);
    FullScreenShader::MLAAGatherSHader::setUniforms(PIXEL_SIZE, 0, 1);

    glBindVertexArray(FullScreenShader::MLAAGatherSHader::vao);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

    // Done.
    glDisable(GL_STENCIL_TEST);

}

// ----------------------------------------------------------------------------
/** Render the post-processed scene */
FrameBuffer *PostProcessing::render(scene::ICameraSceneNode * const camnode)
{
    IVideoDriver * const drv = irr_driver->getVideoDriver();

    MotionBlurProvider * const mocb = (MotionBlurProvider *) irr_driver->
                                                           getCallback(ES_MOTIONBLUR);
    GaussianBlurProvider * const gacb = (GaussianBlurProvider *) irr_driver->
                                                                 getCallback(ES_GAUSSIAN3H);

    FrameBuffer *in_fbo = &irr_driver->getFBO(FBO_COLORS);
    FrameBuffer *out_fbo = &irr_driver->getFBO(FBO_TMP1_WITH_DS);
    // Each effect uses these as named, and sets them up for the next effect.
    // This allows chaining effects where some may be disabled.

    // As the original color shouldn't be touched, the first effect can't be disabled.
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_BLEND);

    if (UserConfigParams::m_dof)
    {
        PROFILER_PUSH_CPU_MARKER("- DoF", 0xFF, 0x00, 0x00);
        ScopedGPUTimer Timer(irr_driver->getGPUTimer(Q_DOF));
        renderDoF(*out_fbo, in_fbo->getRTT()[0]);
        std::swap(in_fbo, out_fbo);
        PROFILER_POP_CPU_MARKER();
    }

    {
        PROFILER_PUSH_CPU_MARKER("- Godrays", 0xFF, 0x00, 0x00);
        ScopedGPUTimer Timer(irr_driver->getGPUTimer(Q_GODRAYS));
        const bool hasgodrays = World::getWorld()->getTrack()->hasGodRays();
        if (UserConfigParams::m_light_shaft && m_sunpixels > 30 && hasgodrays)
        {
            glEnable(GL_DEPTH_TEST);
            // Grab the sky
            out_fbo->Bind();
            glClear(GL_COLOR_BUFFER_BIT);
            irr_driver->renderSkybox(camnode);

            // Set the sun's color
            const SColor col = World::getWorld()->getTrack()->getSunColor();
            ColorizeProvider * const colcb = (ColorizeProvider *)irr_driver->getCallback(ES_COLORIZE);
            colcb->setColor(col.getRed() / 255.0f, col.getGreen() / 255.0f, col.getBlue() / 255.0f);

            // The sun interposer
            STKMeshSceneNode *sun = irr_driver->getSunInterposer();
            irr_driver->getSceneManager()->drawAll(ESNRP_CAMERA);
            irr_driver->setPhase(GLOW_PASS);
            sun->render();
            glDisable(GL_DEPTH_TEST);

            // Fade to quarter
            irr_driver->getFBO(FBO_QUARTER1).Bind();
            glViewport(0, 0, UserConfigParams::m_width / 4, UserConfigParams::m_height / 4);
            renderGodFade(out_fbo->getRTT()[0], col);

            // Blur
            renderGaussian3Blur(irr_driver->getFBO(FBO_QUARTER1), irr_driver->getFBO(FBO_QUARTER2));

            // Calculate the sun's position in texcoords
            const core::vector3df pos = sun->getPosition();
            float ndc[4];
            core::matrix4 trans = camnode->getProjectionMatrix();
            trans *= camnode->getViewMatrix();

            trans.transformVect(ndc, pos);

            const float texh = m_vertices[0].v1.TCoords.Y - m_vertices[0].v0.TCoords.Y;
            const float texw = m_vertices[0].v3.TCoords.X - m_vertices[0].v0.TCoords.X;

            const float sunx = ((ndc[0] / ndc[3]) * 0.5f + 0.5f) * texw;
            const float suny = ((ndc[1] / ndc[3]) * 0.5f + 0.5f) * texh;

            // Rays please
            irr_driver->getFBO(FBO_QUARTER2).Bind();
            renderGodRay(irr_driver->getRenderTargetTexture(RTT_QUARTER1), core::vector2df(sunx, suny));

            // Blur
            renderGaussian3Blur(irr_driver->getFBO(FBO_QUARTER2), irr_driver->getFBO(FBO_QUARTER1));

            // Blend
            glEnable(GL_BLEND);
            glBlendFunc(GL_ONE, GL_ONE);
            glBlendEquation(GL_FUNC_ADD);

            in_fbo->Bind();
            renderPassThrough(irr_driver->getRenderTargetTexture(RTT_QUARTER2));
            glDisable(GL_BLEND);
        }
        PROFILER_POP_CPU_MARKER();
    }

    // Simulate camera defects from there

    {
        PROFILER_PUSH_CPU_MARKER("- Bloom", 0xFF, 0x00, 0x00);
        ScopedGPUTimer Timer(irr_driver->getGPUTimer(Q_BLOOM));
        if (UserConfigParams::m_bloom)
        {
            glClear(GL_STENCIL_BUFFER_BIT);

            FrameBuffer::Blit(*in_fbo, irr_driver->getFBO(FBO_BLOOM_1024), GL_COLOR_BUFFER_BIT, GL_LINEAR);

            irr_driver->getFBO(FBO_BLOOM_512).Bind();
            renderBloom(irr_driver->getRenderTargetTexture(RTT_BLOOM_1024));

            // Downsample
            FrameBuffer::Blit(irr_driver->getFBO(FBO_BLOOM_512), irr_driver->getFBO(FBO_BLOOM_256), GL_COLOR_BUFFER_BIT, GL_LINEAR);
            FrameBuffer::Blit(irr_driver->getFBO(FBO_BLOOM_256), irr_driver->getFBO(FBO_BLOOM_128), GL_COLOR_BUFFER_BIT, GL_LINEAR);

            // Blur
            renderGaussian6Blur(irr_driver->getFBO(FBO_BLOOM_512), irr_driver->getFBO(FBO_TMP_512));

            renderGaussian6Blur(irr_driver->getFBO(FBO_BLOOM_256), irr_driver->getFBO(FBO_TMP_256));

            renderGaussian6Blur(irr_driver->getFBO(FBO_BLOOM_128), irr_driver->getFBO(FBO_TMP_128));

            // Additively blend on top of tmp1
            in_fbo->Bind();
            glEnable(GL_BLEND);
            glBlendFunc(GL_CONSTANT_COLOR, GL_ONE);
            glBlendEquation(GL_FUNC_ADD);
            glBlendColor(.125, .125, .125, .125);
            renderPassThrough(irr_driver->getRenderTargetTexture(RTT_BLOOM_128));
            glBlendColor(.25, .25, .25, .25);
            renderPassThrough(irr_driver->getRenderTargetTexture(RTT_BLOOM_256));
            glBlendColor(.5, .5, .5, .5);
            renderPassThrough(irr_driver->getRenderTargetTexture(RTT_BLOOM_512));
            glDisable(GL_BLEND);
        } // end if bloom
        PROFILER_POP_CPU_MARKER();
    }

    //computeLogLuminance(in_rtt);
    {
        PROFILER_PUSH_CPU_MARKER("- Tonemap", 0xFF, 0x00, 0x00);
        ScopedGPUTimer Timer(irr_driver->getGPUTimer(Q_TONEMAP));
        toneMap(*out_fbo, in_fbo->getRTT()[0]);
        std::swap(in_fbo, out_fbo);
        PROFILER_POP_CPU_MARKER();
    }

    {
        PROFILER_PUSH_CPU_MARKER("- Motion blur", 0xFF, 0x00, 0x00);
        ScopedGPUTimer Timer(irr_driver->getGPUTimer(Q_MOTIONBLUR));
        if (UserConfigParams::m_motionblur && m_any_boost) // motion blur
        {
            renderMotionBlur(0, *in_fbo, *out_fbo);
            std::swap(in_fbo, out_fbo);
        }
        PROFILER_POP_CPU_MARKER();
    }

    if (UserConfigParams::m_mlaa) // MLAA. Must be the last pp filter.
    {
        PROFILER_PUSH_CPU_MARKER("- MLAA", 0xFF, 0x00, 0x00);
        glEnable(GL_FRAMEBUFFER_SRGB);
        irr_driver->getFBO(FBO_MLAA_COLORS).Bind();
        renderPassThrough(in_fbo->getRTT()[0]);
        glDisable(GL_FRAMEBUFFER_SRGB);
        applyMLAA();
        out_fbo = &irr_driver->getFBO(FBO_MLAA_COLORS);
        PROFILER_POP_CPU_MARKER();
    }

    return out_fbo;
}   // render
