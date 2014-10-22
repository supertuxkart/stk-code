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
#include "utils/cpp2011.hpp"

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

template<typename T,typename... Args>
static void DrawFullScreenEffect(Args...args)
{
    glUseProgram(T::getInstance()->Program);
    glBindVertexArray(SharedObject::FullScreenQuadVAO);
    T::getInstance()->setUniforms(args...);
    glDrawArrays(GL_TRIANGLES, 0, 3);
}

static
void renderBloom(GLuint in)
{
    FullScreenShader::BloomShader::getInstance()->SetTextureUnits(in);
    DrawFullScreenEffect<FullScreenShader::BloomShader>();
}

void PostProcessing::renderEnvMap(const float *bSHCoeff, const float *gSHCoeff, const float *rSHCoeff, GLuint skybox)
{
    glDisable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendEquation(GL_FUNC_ADD);
    glBlendFunc(GL_ONE, GL_ONE);

    glUseProgram(FullScreenShader::EnvMapShader::getInstance()->Program);
    glBindVertexArray(SharedObject::FullScreenQuadVAO);

    FullScreenShader::EnvMapShader::getInstance()->SetTextureUnits(irr_driver->getRenderTargetTexture(RTT_NORMAL_AND_DEPTH), irr_driver->getDepthStencilTexture(), skybox);
    core::matrix4 TVM = irr_driver->getViewMatrix().getTransposed();
    FullScreenShader::EnvMapShader::getInstance()->setUniforms(TVM, std::vector<float>(bSHCoeff, bSHCoeff + 9), std::vector<float>(gSHCoeff, gSHCoeff + 9), std::vector<float>(rSHCoeff, rSHCoeff + 9));

    glDrawArrays(GL_TRIANGLES, 0, 3);
    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    glEnable(GL_DEPTH_TEST);
    glDisable(GL_BLEND);
}


void PostProcessing::renderRHDebug(unsigned SHR, unsigned SHG, unsigned SHB, const core::matrix4 &rh_matrix, const core::vector3df &rh_extend)
{
    glEnable(GL_PROGRAM_POINT_SIZE);
    glUseProgram(FullScreenShader::RHDebug::getInstance()->Program);
    glActiveTexture(GL_TEXTURE0 + FullScreenShader::RHDebug::getInstance()->TU_SHR);
    glBindTexture(GL_TEXTURE_3D, SHR);
    glActiveTexture(GL_TEXTURE0 + FullScreenShader::RHDebug::getInstance()->TU_SHG);
    glBindTexture(GL_TEXTURE_3D, SHG);
    glActiveTexture(GL_TEXTURE0 + FullScreenShader::RHDebug::getInstance()->TU_SHB);
    glBindTexture(GL_TEXTURE_3D, SHB);
    FullScreenShader::RHDebug::getInstance()->setUniforms(rh_matrix, rh_extend);
    glDrawArrays(GL_POINTS, 0, 32 * 16 * 32);
    glDisable(GL_PROGRAM_POINT_SIZE);
}

void PostProcessing::renderGI(const core::matrix4 &RHMatrix, const core::vector3df &rh_extend, GLuint shr, GLuint shg, GLuint shb)
{
    core::matrix4 InvRHMatrix;
    RHMatrix.getInverse(InvRHMatrix);
    glDisable(GL_DEPTH_TEST);
    FullScreenShader::GlobalIlluminationReconstructionShader::getInstance()->SetTextureUnits(
        irr_driver->getRenderTargetTexture(RTT_NORMAL_AND_DEPTH), irr_driver->getDepthStencilTexture(), shr, shg, shb);
    DrawFullScreenEffect<FullScreenShader::GlobalIlluminationReconstructionShader>(RHMatrix, InvRHMatrix, rh_extend);
}

void PostProcessing::renderSunlight()
{
  SunLightProvider * const cb = (SunLightProvider *) irr_driver->getCallback(ES_SUNLIGHT);

  glEnable(GL_BLEND);
  glDisable(GL_DEPTH_TEST);
  glBlendFunc(GL_ONE, GL_ONE);
  glBlendEquation(GL_FUNC_ADD);

  FullScreenShader::SunLightShader::getInstance()->SetTextureUnits(irr_driver->getRenderTargetTexture(RTT_NORMAL_AND_DEPTH), irr_driver->getDepthStencilTexture());
  DrawFullScreenEffect<FullScreenShader::SunLightShader>(cb->getPosition(), video::SColorf(cb->getRed(), cb->getGreen(), cb->getBlue()));
}

void PostProcessing::renderShadowedSunlight(const std::vector<core::matrix4> &sun_ortho_matrix, GLuint depthtex)
{
    SunLightProvider * const cb = (SunLightProvider *)irr_driver->getCallback(ES_SUNLIGHT);

    glEnable(GL_BLEND);
    glDisable(GL_DEPTH_TEST);
    glBlendFunc(GL_ONE, GL_ONE);
    glBlendEquation(GL_FUNC_ADD);

    FullScreenShader::ShadowedSunLightShader::getInstance()->SetTextureUnits(irr_driver->getRenderTargetTexture(RTT_NORMAL_AND_DEPTH), irr_driver->getDepthStencilTexture(), depthtex);
    DrawFullScreenEffect<FullScreenShader::ShadowedSunLightShader>(cb->getPosition(), video::SColorf(cb->getRed(), cb->getGreen(), cb->getBlue()));
}


void PostProcessing::renderGaussian3Blur(FrameBuffer &in_fbo, FrameBuffer &auxiliary)
{
    assert(in_fbo.getWidth() == auxiliary.getWidth() && in_fbo.getHeight() == auxiliary.getHeight());
    float inv_width = 1.0f / in_fbo.getWidth(), inv_height = 1.0f / in_fbo.getHeight();
    {
        auxiliary.Bind();

        FullScreenShader::Gaussian3VBlurShader::getInstance()->SetTextureUnits(in_fbo.getRTT()[0]);
        DrawFullScreenEffect<FullScreenShader::Gaussian3VBlurShader>(core::vector2df(inv_width, inv_height));
    }
    {
        in_fbo.Bind();

        FullScreenShader::Gaussian3HBlurShader::getInstance()->SetTextureUnits(auxiliary.getRTT()[0]);
        DrawFullScreenEffect<FullScreenShader::Gaussian3HBlurShader>(core::vector2df(inv_width, inv_height));
    }
}

void PostProcessing::renderGaussian6Blur(FrameBuffer &in_fbo, FrameBuffer &auxiliary)
{
    assert(in_fbo.getWidth() == auxiliary.getWidth() && in_fbo.getHeight() == auxiliary.getHeight());
    float inv_width = 1.0f / in_fbo.getWidth(), inv_height = 1.0f / in_fbo.getHeight();
    {
        auxiliary.Bind();

        FullScreenShader::Gaussian6VBlurShader::getInstance()->SetTextureUnits(in_fbo.getRTT()[0]);
        DrawFullScreenEffect<FullScreenShader::Gaussian6VBlurShader>(core::vector2df(inv_width, inv_height));
    }
    {
        in_fbo.Bind();

        FullScreenShader::Gaussian6HBlurShader::getInstance()->SetTextureUnits(auxiliary.getRTT()[0]);
        DrawFullScreenEffect<FullScreenShader::Gaussian6HBlurShader>(core::vector2df(inv_width, inv_height));
    }
}

void PostProcessing::renderGaussian17TapBlur(FrameBuffer &in_fbo, FrameBuffer &auxiliary)
{
    assert(in_fbo.getWidth() == auxiliary.getWidth() && in_fbo.getHeight() == auxiliary.getHeight());
    if (irr_driver->hasARBComputeShaders())
        glMemoryBarrier(GL_FRAMEBUFFER_BARRIER_BIT);
    float inv_width = 1.0f / in_fbo.getWidth(), inv_height = 1.0f / in_fbo.getHeight();
    {
        if (!irr_driver->hasARBComputeShaders())
        {
            auxiliary.Bind();
            FullScreenShader::Gaussian17TapHShader::getInstance()->SetTextureUnits(in_fbo.getRTT()[0], irr_driver->getFBO(FBO_LINEAR_DEPTH).getRTT()[0]);
            DrawFullScreenEffect<FullScreenShader::Gaussian17TapHShader>(core::vector2df(inv_width, inv_height));
        }
        else
        {
            glUseProgram(FullScreenShader::ComputeGaussian17TapHShader::getInstance()->Program);
            glBindSampler(FullScreenShader::ComputeGaussian17TapHShader::getInstance()->TU_dest, 0);
            FullScreenShader::ComputeGaussian17TapHShader::getInstance()->SetTextureUnits(in_fbo.getRTT()[0], irr_driver->getFBO(FBO_LINEAR_DEPTH).getRTT()[0]);
            glBindImageTexture(FullScreenShader::ComputeGaussian17TapHShader::getInstance()->TU_dest, auxiliary.getRTT()[0], 0, false, 0, GL_WRITE_ONLY, GL_R16F);
            FullScreenShader::ComputeGaussian17TapHShader::getInstance()->setUniforms(core::vector2df(inv_width, inv_height));
            glDispatchCompute((int)in_fbo.getWidth() / 8 + 1, (int)in_fbo.getHeight() / 8 + 1, 1);
        }
    }
    if (irr_driver->hasARBComputeShaders())
        glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
    {
        if (!irr_driver->hasARBComputeShaders())
        {
            in_fbo.Bind();

            FullScreenShader::Gaussian17TapVShader::getInstance()->SetTextureUnits(auxiliary.getRTT()[0], irr_driver->getFBO(FBO_LINEAR_DEPTH).getRTT()[0]);
            DrawFullScreenEffect<FullScreenShader::Gaussian17TapVShader>(core::vector2df(inv_width, inv_height));
        }
        else
        {
            glUseProgram(FullScreenShader::ComputeGaussian17TapVShader::getInstance()->Program);
            glBindSampler(FullScreenShader::ComputeGaussian17TapVShader::getInstance()->TU_dest, 0);
            FullScreenShader::ComputeGaussian17TapVShader::getInstance()->SetTextureUnits(auxiliary.getRTT()[0], irr_driver->getFBO(FBO_LINEAR_DEPTH).getRTT()[0]);
            glBindImageTexture(FullScreenShader::ComputeGaussian17TapVShader::getInstance()->TU_dest, in_fbo.getRTT()[0], 0, false, 0, GL_WRITE_ONLY, GL_R16F);
            FullScreenShader::ComputeGaussian17TapVShader::getInstance()->setUniforms(core::vector2df(inv_width, inv_height));
            glDispatchCompute((int)in_fbo.getWidth() / 8 + 1, (int)in_fbo.getHeight() / 8 + 1, 1);
        }
    }
    if (irr_driver->hasARBComputeShaders())
        glMemoryBarrier(GL_TEXTURE_FETCH_BARRIER_BIT);
}

void PostProcessing::renderPassThrough(GLuint tex)
{
    glUseProgram(FullScreenShader::PassThroughShader::getInstance()->Program);
    glBindVertexArray(FullScreenShader::PassThroughShader::getInstance()->vao);

    FullScreenShader::PassThroughShader::getInstance()->SetTextureUnits(tex);
    FullScreenShader::PassThroughShader::getInstance()->setUniforms();

    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
}

void PostProcessing::renderTextureLayer(unsigned tex, unsigned layer)
{
    glUseProgram(FullScreenShader::LayerPassThroughShader::getInstance()->Program);
    glBindVertexArray(FullScreenShader::LayerPassThroughShader::getInstance()->vao);

    glActiveTexture(GL_TEXTURE0 + FullScreenShader::LayerPassThroughShader::getInstance()->TU_texture);
    glBindTexture(GL_TEXTURE_2D_ARRAY, tex);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    FullScreenShader::LayerPassThroughShader::getInstance()->setUniforms(layer);

    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
}

void PostProcessing::renderGlow(unsigned tex)
{
    glUseProgram(FullScreenShader::GlowShader::getInstance()->Program);
    glBindVertexArray(FullScreenShader::GlowShader::getInstance()->vao);

    FullScreenShader::GlowShader::getInstance()->SetTextureUnits(tex);
    FullScreenShader::GlowShader::getInstance()->setUniforms();

    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
}

void PostProcessing::renderSSAO()
{
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_BLEND);

    // Generate linear depth buffer
    irr_driver->getFBO(FBO_LINEAR_DEPTH).Bind();
    FullScreenShader::LinearizeDepthShader::getInstance()->SetTextureUnits(irr_driver->getDepthStencilTexture());
    DrawFullScreenEffect<FullScreenShader::LinearizeDepthShader>(irr_driver->getSceneManager()->getActiveCamera()->getNearValue(), irr_driver->getSceneManager()->getActiveCamera()->getFarValue());
    irr_driver->getFBO(FBO_SSAO).Bind();

    FullScreenShader::SSAOShader::getInstance()->SetTextureUnits(irr_driver->getRenderTargetTexture(RTT_LINEAR_DEPTH));
    glGenerateMipmap(GL_TEXTURE_2D);

    DrawFullScreenEffect<FullScreenShader::SSAOShader>(irr_driver->getSSAORadius(), irr_driver->getSSAOK(), irr_driver->getSSAOSigma());
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


    FullScreenShader::FogShader::getInstance()->SetTextureUnits(irr_driver->getDepthStencilTexture());
    DrawFullScreenEffect<FullScreenShader::FogShader>(fogmax, startH, endH, start, end, col);

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
    if (Camera::getCamera(cam)->getKart())
    {
        const core::vector3df pos = Camera::getCamera(cam)->getKart()->getNode()->getPosition();
        float ndc[4];
        core::matrix4 trans = camnode->getProjectionMatrix();
        trans *= camnode->getViewMatrix();

        trans.transformVect(ndc, pos);
        const float karty = (ndc[1] / ndc[3]) * 0.5f + 0.5f;
        setMotionBlurCenterY(cam, karty);
    }
    else
        setMotionBlurCenterY(cam, 0.5f);

    out_fbo.Bind();
    glClear(GL_COLOR_BUFFER_BIT);

    FullScreenShader::MotionBlurShader::getInstance()->SetTextureUnits(in_fbo.getRTT()[0], irr_driver->getDepthStencilTexture());
    DrawFullScreenEffect<FullScreenShader::MotionBlurShader>(
                                  // Todo : use a previousPVMatrix per cam, not global
                                  irr_driver->getPreviousPVMatrix(),
                                  cb->getCenter(cam),
                                  cb->getBoostTime(0) * 10, // Todo : should be framerate dependent
                                  0.15f);
}

static void renderGodFade(GLuint tex, const SColor &col)
{
    glUseProgram(FullScreenShader::GodFadeShader::getInstance()->Program);
    glBindVertexArray(FullScreenShader::GodFadeShader::getInstance()->vao);
    FullScreenShader::GodFadeShader::getInstance()->SetTextureUnits(tex);
    FullScreenShader::GodFadeShader::getInstance()->setUniforms(col);

    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
}

static void renderGodRay(GLuint tex, const core::vector2df &sunpos)
{
    glUseProgram(FullScreenShader::GodRayShader::getInstance()->Program);
    glBindVertexArray(FullScreenShader::GodRayShader::getInstance()->vao);
    FullScreenShader::GodRayShader::getInstance()->SetTextureUnits(tex);
    FullScreenShader::GodRayShader::getInstance()->setUniforms(sunpos);

    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
}

static void toneMap(FrameBuffer &fbo, GLuint rtt)
{
    fbo.Bind();
    FullScreenShader::ToneMapShader::getInstance()->SetTextureUnits(rtt);
    DrawFullScreenEffect<FullScreenShader::ToneMapShader>();
}

static void renderDoF(FrameBuffer &fbo, GLuint rtt)
{
    fbo.Bind();
    FullScreenShader::DepthOfFieldShader::getInstance()->SetTextureUnits(rtt, irr_driver->getDepthStencilTexture());
    DrawFullScreenEffect<FullScreenShader::DepthOfFieldShader>();
}

void PostProcessing::applyMLAA()
{
    const core::vector2df &PIXEL_SIZE = core::vector2df(1.0f / UserConfigParams::m_width, 1.0f / UserConfigParams::m_height);

    irr_driver->getFBO(FBO_MLAA_TMP).Bind();
    glEnable(GL_STENCIL_TEST);
    glClearColor(0.0, 0.0, 0.0, 1.0);
    glClear(GL_STENCIL_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
    glStencilFunc(GL_ALWAYS, 1, ~0);
    glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);

    // Pass 1: color edge detection
    glUseProgram(FullScreenShader::MLAAColorEdgeDetectionSHader::getInstance()->Program);
    FullScreenShader::MLAAColorEdgeDetectionSHader::getInstance()->SetTextureUnits(irr_driver->getRenderTargetTexture(RTT_MLAA_COLORS));
    FullScreenShader::MLAAColorEdgeDetectionSHader::getInstance()->setUniforms(PIXEL_SIZE);

    glBindVertexArray(FullScreenShader::MLAAColorEdgeDetectionSHader::getInstance()->vao);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

    glStencilFunc(GL_EQUAL, 1, ~0);
    glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);

    // Pass 2: blend weights
    irr_driver->getFBO(FBO_MLAA_BLEND).Bind();
    glClear(GL_COLOR_BUFFER_BIT);

    glUseProgram(FullScreenShader::MLAABlendWeightSHader::getInstance()->Program);
    FullScreenShader::MLAABlendWeightSHader::getInstance()->SetTextureUnits(irr_driver->getRenderTargetTexture(RTT_MLAA_TMP), getTextureGLuint(m_areamap));
    FullScreenShader::MLAABlendWeightSHader::getInstance()->setUniforms(PIXEL_SIZE);

    glBindVertexArray(FullScreenShader::MLAABlendWeightSHader::getInstance()->vao);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

    // Blit in to tmp1
    FrameBuffer::Blit(irr_driver->getFBO(FBO_MLAA_COLORS), irr_driver->getFBO(FBO_MLAA_TMP));

    // Pass 3: gather
    irr_driver->getFBO(FBO_MLAA_COLORS).Bind();

    glUseProgram(FullScreenShader::MLAAGatherSHader::getInstance()->Program);
    FullScreenShader::MLAAGatherSHader::getInstance()->SetTextureUnits(irr_driver->getRenderTargetTexture(RTT_MLAA_BLEND), irr_driver->getRenderTargetTexture(RTT_MLAA_TMP));
    FullScreenShader::MLAAGatherSHader::getInstance()->setUniforms(PIXEL_SIZE);

    glBindVertexArray(FullScreenShader::MLAAGatherSHader::getInstance()->vao);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

    // Done.
    glDisable(GL_STENCIL_TEST);
}

// ----------------------------------------------------------------------------
/** Render the post-processed scene */
FrameBuffer *PostProcessing::render(scene::ICameraSceneNode * const camnode, bool isRace)
{
    FrameBuffer *in_fbo = &irr_driver->getFBO(FBO_COLORS);
    FrameBuffer *out_fbo = &irr_driver->getFBO(FBO_TMP1_WITH_DS);
    // Each effect uses these as named, and sets them up for the next effect.
    // This allows chaining effects where some may be disabled.

    // As the original color shouldn't be touched, the first effect can't be disabled.
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_BLEND);

    if (isRace && UserConfigParams::m_dof)
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
        bool hasgodrays = false;
        if (World::getWorld() != NULL)
            hasgodrays = World::getWorld()->getTrack()->hasGodRays();

        if (isRace && UserConfigParams::m_light_shaft && hasgodrays)
        {
            Track* track = World::getWorld()->getTrack();

            glEnable(GL_DEPTH_TEST);
            // Grab the sky
            out_fbo->Bind();
            glClear(GL_COLOR_BUFFER_BIT);
//            irr_driver->renderSkybox(camnode);

            // Set the sun's color
            const SColor col = track->getGodRaysColor();

            // The sun interposer
            STKMeshSceneNode *sun = irr_driver->getSunInterposer();
            sun->setGlowColors(col);
            sun->setPosition(track->getGodRaysPosition());
            sun->updateAbsolutePosition();
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
            const core::vector3df pos = track->getGodRaysPosition();
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
            glBlendColor(0., 0., 0., track->getGodRaysOpacity());
            glBlendFunc(GL_CONSTANT_ALPHA, GL_ONE);
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
        if (isRace && UserConfigParams::m_bloom)
        {
            glClear(GL_STENCIL_BUFFER_BIT);
            glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_FALSE);

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
            glBlendFunc(GL_ONE, GL_ONE);
            glBlendEquation(GL_FUNC_ADD);
            FullScreenShader::BloomBlendShader::getInstance()->SetTextureUnits(
                irr_driver->getRenderTargetTexture(RTT_BLOOM_128), irr_driver->getRenderTargetTexture(RTT_BLOOM_256), irr_driver->getRenderTargetTexture(RTT_BLOOM_512));
            DrawFullScreenEffect<FullScreenShader::BloomBlendShader>();

            glDisable(GL_BLEND);
            glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
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
        if (isRace && UserConfigParams::m_motionblur && World::getWorld() != NULL) // motion blur
        {
            renderMotionBlur(0, *in_fbo, *out_fbo);
            std::swap(in_fbo, out_fbo);
        }
        PROFILER_POP_CPU_MARKER();
    }

    // Workaround a bug with srgb fbo on sandy bridge windows
    if (irr_driver->needUBOWorkaround())
        return in_fbo;

    glEnable(GL_FRAMEBUFFER_SRGB);
    irr_driver->getFBO(FBO_MLAA_COLORS).Bind();
    renderPassThrough(in_fbo->getRTT()[0]);
    out_fbo = &irr_driver->getFBO(FBO_MLAA_COLORS);

    if (UserConfigParams::m_mlaa) // MLAA. Must be the last pp filter.
    {
        PROFILER_PUSH_CPU_MARKER("- MLAA", 0xFF, 0x00, 0x00);
        ScopedGPUTimer Timer(irr_driver->getGPUTimer(Q_MLAA));
        applyMLAA();
        PROFILER_POP_CPU_MARKER();
    }
    glDisable(GL_FRAMEBUFFER_SRGB);

    return out_fbo;
}   // render
