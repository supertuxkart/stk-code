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
#include "graphics/glwrap.hpp"

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

// ----------------------------------------------------------------------------
/** Render the post-processed scene, solids only, color to color, no stencil */
void PostProcessing::renderSolid(const u32 cam)
{
    if (!irr_driver->isGLSL()) return;

    IVideoDriver * const drv = irr_driver->getVideoDriver();
    if (World::getWorld()->getTrack()->isFogEnabled())
    {
        m_material.MaterialType = irr_driver->getShader(ES_FOG);
        m_material.setTexture(0, irr_driver->getRTT(RTT_NORMAL_AND_DEPTH));

        // Overlay
        m_material.BlendOperation = EBO_ADD;
        m_material.MaterialTypeParam = pack_textureBlendFunc(EBF_SRC_ALPHA, EBF_ONE_MINUS_SRC_ALPHA);

        drv->setRenderTarget(irr_driver->getRTT(RTT_COLOR), false, false);
        drawQuad(cam, m_material);

        m_material.BlendOperation = EBO_NONE;
        m_material.MaterialTypeParam = 0;
    }
}

GLuint quad_vbo = 0;

static void initQuadVBO()
{
	initGL();
	const float quad_vertex[] = {
		-1., -1., 0., 0., // UpperLeft
		-1., 1., 0., 1., // LowerLeft
		1., -1., 1., 0., // UpperRight
		1., 1., 1., 1., // LowerRight
	};
	glGenBuffers(1, &quad_vbo);
	glBindBuffer(GL_ARRAY_BUFFER, quad_vbo);
	glBufferData(GL_ARRAY_BUFFER, 16 * sizeof(float), quad_vertex, GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}

namespace BloomShader
{
	GLuint Program = 0;
	GLuint attrib_position, attrib_texcoord;
	GLuint uniform_texture, uniform_low;
	GLuint vao = 0;

	void init()
	{
		initGL();
		Program = LoadProgram(file_manager->getAsset("shaders/screenquad.vert").c_str(), file_manager->getAsset("shaders/bloom.frag").c_str());
		attrib_position = glGetAttribLocation(Program, "Position");
		attrib_texcoord = glGetAttribLocation(Program, "Texcoord");
		uniform_texture = glGetUniformLocation(Program, "tex");
		uniform_low = glGetUniformLocation(Program, "low");

		if (!quad_vbo)
			initQuadVBO();
		glGenVertexArrays(1, &vao);
		glBindVertexArray(vao);
		glBindBuffer(GL_ARRAY_BUFFER, quad_vbo);
		glEnableVertexAttribArray(attrib_position);
		glEnableVertexAttribArray(attrib_texcoord);
		glVertexAttribPointer(attrib_position, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), 0);
		glVertexAttribPointer(attrib_texcoord, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (GLvoid*)(2 * sizeof(float)));
		glBindVertexArray(0);
	}
}


static
void renderBloom(ITexture *in)
{
	if (!BloomShader::Program)
		BloomShader::init();

	const float threshold = World::getWorld()->getTrack()->getBloomThreshold();
	glUseProgram(BloomShader::Program);
	glBindVertexArray(BloomShader::vao);
	glUniform1f(BloomShader::uniform_low, threshold);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, static_cast<irr::video::COpenGLTexture*>(in)->getOpenGLTextureName());
	glUniform1i(BloomShader::uniform_texture, 0);

	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
	glBindVertexArray(0);
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

        if (1) // bloom
        {
            // Blit the base to tmp1
            m_material.MaterialType = EMT_SOLID;
            m_material.setTexture(0, in);
            drv->setRenderTarget(out, true, false);

            drawQuad(cam, m_material);

            const bool globalbloom = World::getWorld()->getTrack()->getBloom();

            BloomPowerProvider * const bloomcb = (BloomPowerProvider *)
                                                 irr_driver->
                                                 getCallback(ES_BLOOM_POWER);

            if (globalbloom)
            {
				drv->setRenderTarget(irr_driver->getRTT(RTT_TMP3), true, false);
				renderBloom(in);
            }

            // Do we have any forced bloom nodes? If so, draw them now
            const std::vector<IrrDriver::BloomData> &blooms = irr_driver->getForcedBloom();
            const u32 bloomsize = blooms.size();

            if (!globalbloom && bloomsize)
                drv->setRenderTarget(irr_driver->getRTT(RTT_TMP3), true, false);


            if (globalbloom || bloomsize)
            {
                // Clear the alpha to a suitable value, stencil
                glClearColor(0, 0, 0, 0.1f);
                glColorMask(0, 0, 0, 1);

                glClear(GL_STENCIL_BUFFER_BIT | GL_COLOR_BUFFER_BIT);

                glClearColor(0, 0, 0, 0);
                glColorMask(1, 1, 1, 1);

                // The forced-bloom objects are drawn again, to know which pixels to pick.
                // While it's more drawcalls, there's a cost to using four MRTs over three,
                // and there shouldn't be many such objects in a track.
                // The stencil is already in use for the glow. The alpha channel is best
                // reserved for other use (specular, etc).
                //
                // They are drawn with depth and color writes off, giving 4x-8x drawing speed.
                if (bloomsize)
                {
                    const core::aabbox3df &cambox = camnode->
                                                    getViewFrustum()->
                                                    getBoundingBox();

                    irr_driver->getSceneManager()->setCurrentRendertime(ESNRP_SOLID);
                    SOverrideMaterial &overridemat = drv->getOverrideMaterial();
                    overridemat.EnablePasses = ESNRP_SOLID;
                    overridemat.EnableFlags = EMF_MATERIAL_TYPE | EMF_ZWRITE_ENABLE | EMF_COLOR_MASK;
                    overridemat.Enabled = true;

                    overridemat.Material.MaterialType = irr_driver->getShader(ES_BLOOM_POWER);
                    overridemat.Material.ZWriteEnable = false;
                    overridemat.Material.ColorMask = ECP_ALPHA;

                    glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
                    glStencilFunc(GL_ALWAYS, 1, ~0);
                    glEnable(GL_STENCIL_TEST);

                    camnode->render();

                    for (u32 i = 0; i < bloomsize; i++)
                    {
                        scene::ISceneNode * const cur = blooms[i].node;

                        // Quick box-based culling
                        const core::aabbox3df nodebox = cur->getTransformedBoundingBox();
                        if (!nodebox.intersectsWithBox(cambox))
                            continue;

                        bloomcb->setPower(blooms[i].power);

                        cur->render();
                    }

                    // Second pass for transparents. No-op for solids.
                    irr_driver->getSceneManager()->setCurrentRendertime(ESNRP_TRANSPARENT);
                    for (u32 i = 0; i < bloomsize; i++)
                    {
                        scene::ISceneNode * const cur = blooms[i].node;

                        // Quick box-based culling
                        const core::aabbox3df nodebox = cur->getTransformedBoundingBox();
                        if (!nodebox.intersectsWithBox(cambox))
                            continue;

                        bloomcb->setPower(blooms[i].power);

                        cur->render();
                    }

                    overridemat.Enabled = 0;
                    overridemat.EnablePasses = 0;

                    // Ok, we have the stencil; now use it to blit from color to bloom tex
                    glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);
                    glStencilFunc(GL_EQUAL, 1, ~0);
                    m_material.MaterialType = EMT_SOLID;
                    m_material.setTexture(0, irr_driver->getRTT(RTT_COLOR));

                    // Just in case.
                    glColorMask(1, 1, 1, 0);
                    drv->setRenderTarget(irr_driver->getRTT(RTT_TMP3), false, false);

                    m_material.ColorMask = ECP_RGB;
                    drawQuad(cam, m_material);
                    m_material.ColorMask = ECP_ALL;

                    glColorMask(1, 1, 1, 1);
                    glDisable(GL_STENCIL_TEST);
                } // end forced bloom

                // To half
                m_material.MaterialType = EMT_SOLID;
                m_material.setTexture(0, irr_driver->getRTT(RTT_TMP3));
                drv->setRenderTarget(irr_driver->getRTT(RTT_HALF1), true, false);

                drawQuad(cam, m_material);

                // To quarter
                m_material.MaterialType = EMT_SOLID;
                m_material.setTexture(0, irr_driver->getRTT(RTT_HALF1));
                drv->setRenderTarget(irr_driver->getRTT(RTT_QUARTER1), true, false);

                drawQuad(cam, m_material);

                // To eighth
                m_material.MaterialType = EMT_SOLID;
                m_material.setTexture(0, irr_driver->getRTT(RTT_QUARTER1));
                drv->setRenderTarget(irr_driver->getRTT(RTT_EIGHTH1), true, false);

                drawQuad(cam, m_material);

                // Blur it for distribution.
                {
                    gacb->setResolution(UserConfigParams::m_width / 8,
                                        UserConfigParams::m_height / 8);
                    m_material.MaterialType = irr_driver->getShader(ES_GAUSSIAN6V);
                    m_material.setTexture(0, irr_driver->getRTT(RTT_EIGHTH1));
                    drv->setRenderTarget(irr_driver->getRTT(RTT_EIGHTH2), true, false);

                    drawQuad(cam, m_material);

                    m_material.MaterialType = irr_driver->getShader(ES_GAUSSIAN6H);
                    m_material.setTexture(0, irr_driver->getRTT(RTT_EIGHTH2));
                    drv->setRenderTarget(irr_driver->getRTT(RTT_EIGHTH1), false, false);

                    drawQuad(cam, m_material);
                }

                // Additively blend on top of tmp1
                m_material.BlendOperation = EBO_ADD;
                m_material.MaterialType = irr_driver->getShader(ES_BLOOM_BLEND);
                m_material.setTexture(0, irr_driver->getRTT(RTT_EIGHTH1));
                drv->setRenderTarget(out, false, false);

                drawQuad(cam, m_material);

                m_material.BlendOperation = EBO_NONE;
            } // end if bloom

            in = irr_driver->getRTT(RTT_TMP1);
            out = irr_driver->getRTT(RTT_TMP2);
        }

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
            {
                gacb->setResolution(UserConfigParams::m_width / 4,
                                    UserConfigParams::m_height / 4);
                m_material.MaterialType = irr_driver->getShader(ES_GAUSSIAN3V);
                m_material.setTexture(0, irr_driver->getRTT(RTT_QUARTER1));
                drv->setRenderTarget(irr_driver->getRTT(RTT_QUARTER2), true, false);

                drawQuad(cam, m_material);

                m_material.MaterialType = irr_driver->getShader(ES_GAUSSIAN3H);
                m_material.setTexture(0, irr_driver->getRTT(RTT_QUARTER2));
                drv->setRenderTarget(irr_driver->getRTT(RTT_QUARTER1), false, false);

                drawQuad(cam, m_material);
            }

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

        if (UserConfigParams::m_motionblur && m_any_boost) // motion blur
        {
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
        }

        if (irr_driver->getDisplacingNodes().size()) // Displacement
        {
            m_material.MaterialType = irr_driver->getShader(ES_PPDISPLACE);
            m_material.setFlag(EMF_BILINEAR_FILTER, false);
            m_material.setTexture(0, in);
            m_material.setTexture(1, irr_driver->getRTT(RTT_DISPLACE));
            drv->setRenderTarget(out, true, false);

            drawQuad(cam, m_material);

            m_material.setTexture(1, 0);
            m_material.setFlag(EMF_BILINEAR_FILTER, true);

            ITexture *tmp = in;
            in = out;
            out = tmp;
        }

        if (UserConfigParams::m_mlaa) // MLAA. Must be the last pp filter.
        {
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
        }

        // Final blit

        if (irr_driver->getNormals())
        {
            m_material.MaterialType = irr_driver->getShader(ES_FLIP);
            m_material.setTexture(0, irr_driver->getRTT(RTT_NORMAL_AND_DEPTH));
        } else if (irr_driver->getSSAOViz())
        {
            m_material.MaterialType = irr_driver->getShader(ES_FLIP);
            m_material.setTexture(0, irr_driver->getRTT(RTT_SSAO));
        } else if (irr_driver->getShadowViz())
        {
            m_material.MaterialType = irr_driver->getShader(ES_FLIP);
            m_material.setTexture(0, irr_driver->getRTT(RTT_SHADOW));
        } else
        {
            m_material.MaterialType = irr_driver->getShader(ES_COLOR_LEVELS);
            m_material.setTexture(0, in);
        }

        drv->setRenderTarget(ERT_FRAME_BUFFER, false, false);

        drawQuad(cam, m_material);
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
