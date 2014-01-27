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
    if (!areamap) Log::fatal("postprocessing", "Failed to load the areamap");
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
void renderBloom(ITexture *in)
{
	const float threshold = World::getWorld()->getTrack()->getBloomThreshold();
	glUseProgram(FullScreenShader::BloomShader::Program);
	glBindVertexArray(FullScreenShader::BloomShader::vao);
	glUniform1f(FullScreenShader::BloomShader::uniform_low, threshold);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, static_cast<irr::video::COpenGLTexture*>(in)->getOpenGLTextureName());
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glUniform1i(FullScreenShader::BloomShader::uniform_texture, 0);

	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

static
void renderBloomBlend(ITexture *in)
{
	glEnable(GL_BLEND);
	glBlendEquation(GL_FUNC_ADD);
	glBlendFunc(GL_ONE, GL_ONE);
	glDisable(GL_DEPTH_TEST);

	glUseProgram(FullScreenShader::BloomBlendShader::Program);
	glBindVertexArray(FullScreenShader::BloomBlendShader::vao);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, static_cast<irr::video::COpenGLTexture*>(in)->getOpenGLTextureName());
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glUniform1i(FullScreenShader::BloomBlendShader::uniform_texture, 0);

	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	glEnable(GL_DEPTH_TEST);
	glDisable(GL_BLEND);
}

static
void renderPPDisplace(ITexture *in)
{
	glEnable(GL_BLEND);
	glBlendEquation(GL_FUNC_ADD);
	glBlendFunc(GL_ONE, GL_ONE);
	glDisable(GL_DEPTH_TEST);

	glUseProgram(FullScreenShader::PPDisplaceShader::Program);
	glBindVertexArray(FullScreenShader::PPDisplaceShader::vao);
	glUniform1i(FullScreenShader::PPDisplaceShader::uniform_viz, irr_driver->getDistortViz());

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, static_cast<irr::video::COpenGLTexture*>(in)->getOpenGLTextureName());
	glUniform1i(FullScreenShader::PPDisplaceShader::uniform_tex, 0);

	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, static_cast<irr::video::COpenGLTexture*>(irr_driver->getRTT(RTT_DISPLACE))->getOpenGLTextureName());
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glUniform1i(FullScreenShader::PPDisplaceShader::uniform_dtex, 1);
	

	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	glEnable(GL_DEPTH_TEST);
	glDisable(GL_BLEND);
}

static
void renderColorLevel(ITexture *in)
{
	core::vector3df m_inlevel = World::getWorld()->getTrack()->getColorLevelIn();
	core::vector2df m_outlevel = World::getWorld()->getTrack()->getColorLevelOut();

	glDisable(GL_BLEND);
	glDisable(GL_DEPTH_TEST);

	glUseProgram(FullScreenShader::ColorLevelShader::Program);
	glBindVertexArray(FullScreenShader::ColorLevelShader::vao);
	glUniform3f(FullScreenShader::ColorLevelShader::uniform_inlevel, m_inlevel.X, m_inlevel.Y, m_inlevel.Z);
	glUniform2f(FullScreenShader::ColorLevelShader::uniform_outlevel, m_outlevel.X, m_outlevel.Y);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, static_cast<irr::video::COpenGLTexture*>(in)->getOpenGLTextureName());
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glUniform1i(FullScreenShader::ColorLevelShader::uniform_tex, 0);

	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	glEnable(GL_DEPTH_TEST);
}

void PostProcessing::renderPointlight(const std::vector<float> &positions, const std::vector<float> &colors, const std::vector<float> &energy)
{
	glEnable(GL_BLEND);
	glBlendEquation(GL_FUNC_ADD);
	glBlendFunc(GL_ONE, GL_ONE);
	glDisable(GL_DEPTH_TEST);

	glUseProgram(FullScreenShader::PointLightShader::Program);
	glBindVertexArray(FullScreenShader::PointLightShader::vao);

	setTexture(0, static_cast<irr::video::COpenGLTexture*>(irr_driver->getRTT(RTT_NORMAL_AND_DEPTH))->getOpenGLTextureName(), GL_NEAREST, GL_NEAREST);
	setTexture(1, static_cast<irr::video::COpenGLFBOTexture*>(irr_driver->getRTT(RTT_NORMAL_AND_DEPTH))->DepthBufferTexture, GL_NEAREST, GL_NEAREST);
	FullScreenShader::PointLightShader::setUniforms(irr_driver->getInvProjMatrix(), irr_driver->getViewMatrix(), positions, colors, energy, 200, 0, 1);

	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	glEnable(GL_DEPTH_TEST);
	glDisable(GL_BLEND);
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
  GLuint ntex_id = static_cast<irr::video::COpenGLTexture*>(irr_driver->getRTT(RTT_NORMAL_AND_DEPTH))->getOpenGLTextureName();
  setTexture(0, ntex_id, GL_NEAREST, GL_NEAREST);
  FullScreenShader::SunLightShader::setUniforms(cb->getPosition(), irr_driver->getInvProjMatrix(), cb->getRed(), cb->getGreen(), cb->getBlue(), 0);
  glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
  glBindVertexArray(0);
}

void PostProcessing::renderLightbBlend(ITexture *diffuse, ITexture *specular, ITexture *ao, ITexture *specmap, bool debug)
{
	const SColorf s = irr_driver->getSceneManager()->getAmbientLight();
	glStencilFunc(GL_EQUAL, 1, ~0);
	glEnable(GL_STENCIL_TEST);
	glEnable(GL_BLEND);
	glBlendEquation(GL_FUNC_ADD);
	if (debug)
		glBlendFunc(GL_ONE, GL_ZERO);
	else
		glBlendFunc(GL_DST_COLOR, GL_ZERO);
	glDisable(GL_DEPTH_TEST);

	glUseProgram(FullScreenShader::LightBlendShader::Program);
	glBindVertexArray(FullScreenShader::LightBlendShader::vao);

	glUniform3f(FullScreenShader::LightBlendShader::uniform_ambient, s.r, s.g, s.b);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, static_cast<irr::video::COpenGLTexture*>(diffuse)->getOpenGLTextureName());
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glUniform1i(FullScreenShader::LightBlendShader::uniform_diffuse, 0);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, static_cast<irr::video::COpenGLTexture*>(specular)->getOpenGLTextureName());
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glUniform1i(FullScreenShader::LightBlendShader::uniform_specular, 1);
	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, static_cast<irr::video::COpenGLTexture*>(ao)->getOpenGLTextureName());
	glUniform1i(FullScreenShader::LightBlendShader::uniform_ambient_occlusion, 2);
	glActiveTexture(GL_TEXTURE3);
	if (!UserConfigParams::m_ssao)
	{
	  GLint swizzleMask[] = {GL_ONE, GL_ONE, GL_ONE, GL_ONE};
	  glTexParameteriv(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_RGBA, swizzleMask);
	}
	glBindTexture(GL_TEXTURE_2D, static_cast<irr::video::COpenGLTexture*>(specmap)->getOpenGLTextureName());
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glUniform1i(FullScreenShader::LightBlendShader::uniform_specular_map, 3);

	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	glEnable(GL_DEPTH_TEST);
	glDisable(GL_BLEND);
	glDisable(GL_STENCIL_TEST);
}


void PostProcessing::renderGaussian3Blur(video::ITexture *in, video::ITexture *temprtt, float inv_width, float inv_height)
{
	glDisable(GL_BLEND);
	glDisable(GL_DEPTH_TEST);
	{
		irr_driver->getVideoDriver()->setRenderTarget(temprtt, false, false);
		glUseProgram(FullScreenShader::Gaussian3VBlurShader::Program);
		glBindVertexArray(FullScreenShader::Gaussian3VBlurShader::vao);

		glUniform2f(FullScreenShader::Gaussian3VBlurShader::uniform_pixel, inv_width, inv_height);

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, static_cast<irr::video::COpenGLTexture*>(in)->getOpenGLTextureName());
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glUniform1i(FullScreenShader::Gaussian3VBlurShader::uniform_tex, 0);

		glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
	}
	{
		irr_driver->getVideoDriver()->setRenderTarget(in, false, false);
		glUseProgram(FullScreenShader::Gaussian3HBlurShader::Program);
		glBindVertexArray(FullScreenShader::Gaussian3HBlurShader::vao);

		glUniform2f(FullScreenShader::Gaussian3HBlurShader::uniform_pixel, inv_width, inv_height);

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, static_cast<irr::video::COpenGLTexture*>(temprtt)->getOpenGLTextureName());
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glUniform1i(FullScreenShader::Gaussian3HBlurShader::uniform_tex, 0);

		glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
	}
	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	glEnable(GL_DEPTH_TEST);
}

void PostProcessing::renderGaussian6Blur(video::ITexture *in, video::ITexture *temprtt, float inv_width, float inv_height)
{
	glDisable(GL_BLEND);
	glDisable(GL_DEPTH_TEST);
	{
		irr_driver->getVideoDriver()->setRenderTarget(temprtt, false, false);
		glUseProgram(FullScreenShader::Gaussian6VBlurShader::Program);
		glBindVertexArray(FullScreenShader::Gaussian6VBlurShader::vao);

		glUniform2f(FullScreenShader::Gaussian6VBlurShader::uniform_pixel, inv_width, inv_height);

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, static_cast<irr::video::COpenGLTexture*>(in)->getOpenGLTextureName());
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glUniform1i(FullScreenShader::Gaussian6VBlurShader::uniform_tex, 0);

		glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
	}
	{
		irr_driver->getVideoDriver()->setRenderTarget(in, false, false);
		glUseProgram(FullScreenShader::Gaussian6HBlurShader::Program);
		glBindVertexArray(FullScreenShader::Gaussian6HBlurShader::vao);

		glUniform2f(FullScreenShader::Gaussian6HBlurShader::uniform_pixel, inv_width, inv_height);

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, static_cast<irr::video::COpenGLTexture*>(temprtt)->getOpenGLTextureName());
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glUniform1i(FullScreenShader::Gaussian6HBlurShader::uniform_tex, 0);

		glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
	}
	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	glEnable(GL_DEPTH_TEST);
}

void PostProcessing::renderPassThrough(ITexture *tex)
{
	glDisable(GL_DEPTH_TEST);

	glUseProgram(FullScreenShader::PassThroughShader::Program);
	glBindVertexArray(FullScreenShader::PassThroughShader::vao);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, static_cast<irr::video::COpenGLTexture*>(tex)->getOpenGLTextureName());
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glUniform1i(FullScreenShader::PassThroughShader::uniform_texture, 0);

	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	glEnable(GL_DEPTH_TEST);
	glDisable(GL_BLEND);
}

void PostProcessing::renderGlow(ITexture *tex)
{
	glDisable(GL_DEPTH_TEST);

	glUseProgram(FullScreenShader::GlowShader::Program);
	glBindVertexArray(FullScreenShader::GlowShader::vao);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, static_cast<irr::video::COpenGLTexture*>(tex)->getOpenGLTextureName());
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glUniform1i(FullScreenShader::GlowShader::uniform_tex, 0);

	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	glEnable(GL_DEPTH_TEST);
	glDisable(GL_BLEND);
}

ITexture *noise_tex = 0;

void PostProcessing::renderSSAO(const core::matrix4 &invprojm, const core::matrix4 &projm)
{
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_BLEND);
	glDisable(GL_ALPHA_TEST);

	if (!noise_tex)
		noise_tex = irr_driver->getTexture(file_manager->getAsset("textures/noise.png").c_str());

	glUseProgram(FullScreenShader::SSAOShader::Program);
	glBindVertexArray(FullScreenShader::SSAOShader::vao);
	setTexture(0, static_cast<irr::video::COpenGLTexture*>(irr_driver->getRTT(RTT_NORMAL_AND_DEPTH))->getOpenGLTextureName(), GL_NEAREST, GL_NEAREST);
	setTexture(1, static_cast<irr::video::COpenGLFBOTexture*>(irr_driver->getRTT(RTT_NORMAL_AND_DEPTH))->DepthBufferTexture, GL_LINEAR, GL_LINEAR);
	setTexture(2, static_cast<irr::video::COpenGLTexture*>(noise_tex)->getOpenGLTextureName(), GL_NEAREST, GL_NEAREST);

	FullScreenShader::SSAOShader::setUniforms(projm, invprojm, 0, 1, 2);

	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	glEnable(GL_DEPTH_TEST);
}

void PostProcessing::renderFog(const core::vector3df &campos, const core::matrix4 &ipvmat)
{
	irr_driver->getVideoDriver()->setRenderTarget(irr_driver->getRTT(RTT_COLOR), false, false);
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

	setTexture(0, static_cast<irr::video::COpenGLTexture*>(irr_driver->getRTT(RTT_NORMAL_AND_DEPTH))->getOpenGLTextureName(), GL_NEAREST, GL_NEAREST);
	FullScreenShader::FogShader::setUniforms(ipvmat, fogmax, startH, endH, start, end, col, campos, 0);

	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	glEnable(GL_DEPTH_TEST);
	glDisable(GL_BLEND);
}

// ----------------------------------------------------------------------------
/** Render the post-processed scene */
void PostProcessing::render()
{
    if (!irr_driver->isGLSL()) return;

    IVideoDriver * const drv = irr_driver->getVideoDriver();
    drv->setTransform(ETS_WORLD, core::IdentityMatrix);
    drv->setTransform(ETS_VIEW, core::IdentityMatrix);
    drv->setTransform(ETS_PROJECTION, core::IdentityMatrix);

    MotionBlurProvider * const mocb = (MotionBlurProvider *) irr_driver->
                                                           getCallback(ES_MOTIONBLUR);
    GaussianBlurProvider * const gacb = (GaussianBlurProvider *) irr_driver->
                                                                 getCallback(ES_GAUSSIAN3H);

    const u32 cams = Camera::getNumCameras();
    for(u32 cam = 0; cam < cams; cam++)
    {
        scene::ICameraSceneNode * const camnode =
            Camera::getCamera(cam)->getCameraSceneNode();
        mocb->setCurrentCamera(cam);
        ITexture *in = irr_driver->getRTT(RTT_COLOR);
        ITexture *out = irr_driver->getRTT(RTT_TMP1);
	// Each effect uses these as named, and sets them up for the next effect.
	// This allows chaining effects where some may be disabled.

	// As the original color shouldn't be touched, the first effect can't be disabled.

        PROFILER_PUSH_CPU_MARKER("- Bloom", 0xFF, 0x00, 0x00);
        if (1) // bloom
        {
			// Blit the base to tmp1
			drv->setRenderTarget(out, true, false);
			renderPassThrough(in);

            const bool globalbloom = World::getWorld()->getTrack()->getBloom();

            if (globalbloom)
            {
				drv->setRenderTarget(irr_driver->getRTT(RTT_TMP3), true, false);
				renderBloom(in);
            }


            if (globalbloom)
            {
                // Clear the alpha to a suitable value, stencil
                glClearColor(0, 0, 0, 0.1f);
                glColorMask(0, 0, 0, 1);

                glClear(GL_STENCIL_BUFFER_BIT | GL_COLOR_BUFFER_BIT);

                glClearColor(0, 0, 0, 0);
                glColorMask(1, 1, 1, 1);

                // To half
                drv->setRenderTarget(irr_driver->getRTT(RTT_HALF1), true, false);
				renderPassThrough(irr_driver->getRTT(RTT_TMP3));


                // To quarter
				drv->setRenderTarget(irr_driver->getRTT(RTT_QUARTER1), true, false);
				renderPassThrough(irr_driver->getRTT(RTT_HALF1));
                
                // To eighth
				drv->setRenderTarget(irr_driver->getRTT(RTT_EIGHTH1), true, false);
				renderPassThrough(irr_driver->getRTT(RTT_QUARTER1));

                // Blur it for distribution.
				renderGaussian6Blur(irr_driver->getRTT(RTT_EIGHTH1), irr_driver->getRTT(RTT_EIGHTH2), 8.f / UserConfigParams::m_width, 8.f / UserConfigParams::m_height);

                // Additively blend on top of tmp1
				drv->setRenderTarget(out, false, false);
				renderBloomBlend(irr_driver->getRTT(RTT_EIGHTH1));
				m_material.MaterialType = irr_driver->getShader(ES_RAIN);
				drv->setMaterial(m_material);
				static_cast<irr::video::COpenGLDriver*>(drv)->setRenderStates3DMode();
            } // end if bloom

            in = irr_driver->getRTT(RTT_TMP1);
            out = irr_driver->getRTT(RTT_TMP2);
        }
        PROFILER_POP_CPU_MARKER();

        PROFILER_PUSH_CPU_MARKER("- Godrays", 0xFF, 0x00, 0x00);
        if (World::getWorld()->getTrack()->hasGodRays() && m_sunpixels > 30) // god rays
        {
            // Grab the sky
            drv->setRenderTarget(out, true, false);
            irr_driver->getSceneManager()->drawAll(ESNRP_SKY_BOX);

            // Set the sun's color
            ColorizeProvider * const colcb = (ColorizeProvider *) irr_driver->getCallback(ES_COLORIZE);
            const SColor col = World::getWorld()->getTrack()->getSunColor();
            colcb->setColor(col.getRed() / 255.0f, col.getGreen() / 255.0f, col.getBlue() / 255.0f);

            // The sun interposer
            IMeshSceneNode * const sun = irr_driver->getSunInterposer();
            sun->getMaterial(0).ColorMask = ECP_ALL;
            irr_driver->getSceneManager()->drawAll(ESNRP_CAMERA);
            irr_driver->getSceneManager()->setCurrentRendertime(ESNRP_SOLID);

            sun->render();

            sun->getMaterial(0).ColorMask = ECP_NONE;

            // Fade to quarter
            m_material.MaterialType = irr_driver->getShader(ES_GODFADE);
            m_material.setTexture(0, out);
            drv->setRenderTarget(irr_driver->getRTT(RTT_QUARTER1), false, false);

            drawQuad(cam, m_material);

            // Blur
			renderGaussian3Blur(irr_driver->getRTT(RTT_QUARTER1),
                                irr_driver->getRTT(RTT_QUARTER2),
                                4.f / UserConfigParams::m_width,
                                4.f / UserConfigParams::m_height);

            // Calculate the sun's position in texcoords
            const core::vector3df pos = sun->getPosition();
            float ndc[4];
            core::matrix4 trans = camnode->getProjectionMatrix();
            trans *= camnode->getViewMatrix();

            trans.transformVect(ndc, pos);

            const float texh = m_vertices[cam].v1.TCoords.Y - m_vertices[cam].v0.TCoords.Y;
            const float texw = m_vertices[cam].v3.TCoords.X - m_vertices[cam].v0.TCoords.X;

            const float sunx = ((ndc[0] / ndc[3]) * 0.5f + 0.5f) * texw;
            const float suny = ((ndc[1] / ndc[3]) * 0.5f + 0.5f) * texh;

            ((GodRayProvider *) irr_driver->getCallback(ES_GODRAY))->
                setSunPosition(sunx, suny);

            // Rays please
            m_material.MaterialType = irr_driver->getShader(ES_GODRAY);
            m_material.setTexture(0, irr_driver->getRTT(RTT_QUARTER1));
            drv->setRenderTarget(irr_driver->getRTT(RTT_QUARTER2), true, false);

            drawQuad(cam, m_material);

            // Blur
            {
                gacb->setResolution(UserConfigParams::m_width / 4,
                                    UserConfigParams::m_height / 4);
                m_material.MaterialType = irr_driver->getShader(ES_GAUSSIAN3V);
                m_material.setTexture(0, irr_driver->getRTT(RTT_QUARTER2));
                drv->setRenderTarget(irr_driver->getRTT(RTT_QUARTER1), true, false);

                drawQuad(cam, m_material);

                m_material.MaterialType = irr_driver->getShader(ES_GAUSSIAN3H);
                m_material.setTexture(0, irr_driver->getRTT(RTT_QUARTER1));
                drv->setRenderTarget(irr_driver->getRTT(RTT_QUARTER2), false, false);

                drawQuad(cam, m_material);
            }

            // Overlay
            m_material.MaterialType = EMT_TRANSPARENT_ADD_COLOR;
            m_material.setTexture(0, irr_driver->getRTT(RTT_QUARTER2));
            drv->setRenderTarget(in, false, false);

            drawQuad(cam, m_material);
        }
        PROFILER_POP_CPU_MARKER();

        if (UserConfigParams::m_motionblur && m_any_boost) // motion blur
        {
            PROFILER_PUSH_CPU_MARKER("- Motion blur", 0xFF, 0x00, 0x00);
            // Calculate the kart's Y position on screen
            const core::vector3df pos =
                Camera::getCamera(cam)->getKart()->getNode()->getPosition();
            float ndc[4];
            core::matrix4 trans = camnode->getProjectionMatrix();
            trans *= camnode->getViewMatrix();

            trans.transformVect(ndc, pos);
            const float karty = (ndc[1] / ndc[3]) * 0.5f + 0.5f;
            setMotionBlurCenterY(cam, karty);


            m_material.MaterialType = irr_driver->getShader(ES_MOTIONBLUR);
            m_material.setTexture(0, in);
            drv->setRenderTarget(out, true, false);

            drawQuad(cam, m_material);

            ITexture *tmp = in;
            in = out;
            out = tmp;
            PROFILER_POP_CPU_MARKER();
        }

        if (irr_driver->getDisplacingNodes().size()) // Displacement
        {
            PROFILER_PUSH_CPU_MARKER("- Displacement", 0xFF, 0x00, 0x00);
			drv->setRenderTarget(out, true, false);
			renderPPDisplace(in);

            ITexture *tmp = in;
            in = out;
            out = tmp;
            PROFILER_POP_CPU_MARKER();
        }

        if (UserConfigParams::m_mlaa) // MLAA. Must be the last pp filter.
        {
            PROFILER_PUSH_CPU_MARKER("- MLAA", 0xFF, 0x00, 0x00);
            drv->setRenderTarget(out, false, false);

            glEnable(GL_STENCIL_TEST);
            glClearColor(0.0, 0.0, 0.0, 1.0);
            glClear(GL_STENCIL_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
            glStencilFunc(GL_ALWAYS, 1, ~0);
            glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);

            // Pass 1: color edge detection
            m_material.setFlag(EMF_BILINEAR_FILTER, false);
            m_material.setFlag(EMF_TRILINEAR_FILTER, false);
            m_material.MaterialType = irr_driver->getShader(ES_MLAA_COLOR1);
            m_material.setTexture(0, in);

            drawQuad(cam, m_material);
            m_material.setFlag(EMF_BILINEAR_FILTER, true);
            m_material.setFlag(EMF_TRILINEAR_FILTER, true);

            glStencilFunc(GL_EQUAL, 1, ~0);
            glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);

            // Pass 2: blend weights
            drv->setRenderTarget(irr_driver->getRTT(RTT_TMP3), true, false);

            m_material.MaterialType = irr_driver->getShader(ES_MLAA_BLEND2);
            m_material.setTexture(0, out);
            m_material.setTexture(1, m_areamap);
            m_material.TextureLayer[1].BilinearFilter = false;
            m_material.TextureLayer[1].TrilinearFilter = false;

            drawQuad(cam, m_material);

            m_material.TextureLayer[1].BilinearFilter = true;
            m_material.TextureLayer[1].TrilinearFilter = true;
            m_material.setTexture(1, 0);

            // Pass 3: gather
            drv->setRenderTarget(in, false, false);

            m_material.setFlag(EMF_BILINEAR_FILTER, false);
            m_material.setFlag(EMF_TRILINEAR_FILTER, false);
            m_material.MaterialType = irr_driver->getShader(ES_MLAA_NEIGH3);
            m_material.setTexture(0, irr_driver->getRTT(RTT_TMP3));
            m_material.setTexture(1, irr_driver->getRTT(RTT_COLOR));

            drawQuad(cam, m_material);

            m_material.setFlag(EMF_BILINEAR_FILTER, true);
            m_material.setFlag(EMF_TRILINEAR_FILTER, true);
            m_material.setTexture(1, 0);

            // Done.
            glDisable(GL_STENCIL_TEST);
            PROFILER_POP_CPU_MARKER();
        }

        // Final blit
		// TODO : Use glBlitFramebuffer
		drv->setRenderTarget(ERT_FRAME_BUFFER, false, false);
        if (irr_driver->getNormals())
			renderPassThrough(irr_driver->getRTT(RTT_NORMAL_AND_DEPTH));
        else if (irr_driver->getSSAOViz())
			renderPassThrough(irr_driver->getRTT(RTT_SSAO));
        else if (irr_driver->getShadowViz())
			renderPassThrough(irr_driver->getRTT(RTT_SHADOW));
        else
			renderColorLevel(in);
    }
}   // render

void PostProcessing::drawQuad(u32 cam, const SMaterial &mat)
{
    const u16 indices[6] = {0, 1, 2, 3, 0, 2};
    IVideoDriver * const drv = irr_driver->getVideoDriver();

    drv->setTransform(ETS_WORLD, core::IdentityMatrix);
    drv->setTransform(ETS_VIEW, core::IdentityMatrix);
    drv->setTransform(ETS_PROJECTION, core::IdentityMatrix);

    drv->setMaterial(mat);
    drv->drawIndexedTriangleList(&(m_vertices[cam].v0),
                                      4, indices, 2);
}
