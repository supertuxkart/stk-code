//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2011-2015 the SuperTuxKart-Team
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
#include "central_settings.hpp"
#include "graphics/camera.hpp"
#include "graphics/glwrap.hpp"
#include "graphics/irr_driver.hpp"
#include "graphics/mlaa_areamap.hpp"
#include "graphics/shaders.hpp"
#include "graphics/shared_gpu_objects.hpp"
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

// ============================================================================
class Gaussian3HBlurShader : public TextureShader<Gaussian3HBlurShader, 1,
                                           core::vector2df>
{
public:
    Gaussian3HBlurShader()
    {
        loadProgram(OBJECT, GL_VERTEX_SHADER, "screenquad.vert",
                            GL_FRAGMENT_SHADER, "gaussian3h.frag");
        assignUniforms("pixel");

        assignSamplerNames(m_program, 0, "tex", ST_BILINEAR_CLAMPED_FILTERED);
    }   // Gaussian3HBlurShader
};   // Gaussian3HBlurShader

// ============================================================================
class ComputeShadowBlurVShader : public TextureShader<ComputeShadowBlurVShader, 1,
                                               core::vector2df,
                                               std::vector<float> >
{
public:
    GLuint m_dest_tu;
    ComputeShadowBlurVShader()
    {
        loadProgram(OBJECT, GL_COMPUTE_SHADER, "blurshadowV.comp");
        m_dest_tu = 1;
        assignUniforms("pixel", "weights");
        assignSamplerNames(m_program, 0, "source", ST_NEARED_CLAMPED_FILTERED);
        assignTextureUnit(m_dest_tu, "dest");
    }   // ComputeShadowBlurVShader

};   // ComputeShadowBlurVShader

// ============================================================================
class Gaussian6VBlurShader : public TextureShader<Gaussian6VBlurShader, 1,
                                           core::vector2df, float>
{
public:
    Gaussian6VBlurShader()
    {
        loadProgram(OBJECT, GL_VERTEX_SHADER, "screenquad.vert",
                            GL_FRAGMENT_SHADER, "gaussian6v.frag");
        assignUniforms("pixel", "sigma");

        assignSamplerNames(m_program, 0, "tex", ST_BILINEAR_CLAMPED_FILTERED);
    }   // Gaussian6VBlurShader
};   // Gaussian6VBlurShader

// ============================================================================
class Gaussian3VBlurShader : public TextureShader<Gaussian3VBlurShader, 1,
                                           core::vector2df>
{
public:
    Gaussian3VBlurShader()
    {
        loadProgram(OBJECT, GL_VERTEX_SHADER, "screenquad.vert",
                            GL_FRAGMENT_SHADER, "gaussian3v.frag");
        assignUniforms("pixel");

        assignSamplerNames(m_program, 0, "tex", ST_BILINEAR_CLAMPED_FILTERED);
    }   // Gaussian3VBlurShader
};   // Gaussian3VBlurShader

// ============================================================================
class ComputeGaussian6VBlurShader : public TextureShader<ComputeGaussian6VBlurShader, 1,
                                                  core::vector2df,
                                                  std::vector<float> >
{
public:
    GLuint m_dest_tu;
    ComputeGaussian6VBlurShader()
    {
        loadProgram(OBJECT, GL_COMPUTE_SHADER, "gaussian6v.comp");
        m_dest_tu = 1;
        assignUniforms("pixel", "weights");
        assignSamplerNames(m_program, 0, "source", 
                           ST_BILINEAR_CLAMPED_FILTERED);
        assignTextureUnit(m_dest_tu, "dest");
    }   // ComputeGaussian6VBlurShader
};   // ComputeGaussian6VBlurShader

// ============================================================================
class ComputeGaussian6HBlurShader : public TextureShader<ComputeGaussian6HBlurShader, 1,
                                                  core::vector2df, 
                                                  std::vector<float> >
{
public:
    GLuint m_dest_tu;
    ComputeGaussian6HBlurShader()
    {
        loadProgram(OBJECT, GL_COMPUTE_SHADER, "gaussian6h.comp");
        m_dest_tu = 1;
        assignUniforms("pixel", "weights");
        assignSamplerNames(m_program, 0, "source", 
                          ST_BILINEAR_CLAMPED_FILTERED);
        assignTextureUnit(m_dest_tu, "dest");
    }   // ComputeGaussian6HBlurShader
};   // ComputeGaussian6HBlurShader

// ============================================================================
class ComputeShadowBlurHShader : public TextureShader<ComputeShadowBlurHShader, 1,
                                               core::vector2df,
                                               std::vector<float> >
{
public:
    GLuint m_dest_tu;
    ComputeShadowBlurHShader()
    {
        loadProgram(OBJECT, GL_COMPUTE_SHADER, "blurshadowH.comp");
        m_dest_tu = 1;
        assignUniforms("pixel", "weights");
        assignSamplerNames(m_program, 0, "source", ST_NEARED_CLAMPED_FILTERED);
        assignTextureUnit(m_dest_tu, "dest");
    }   // ComputeShadowBlurHShader
};   // ComputeShadowBlurHShader

// ============================================================================
class Gaussian6HBlurShader : public TextureShader<Gaussian6HBlurShader, 1,
                                           core::vector2df, float>
{
public:
    Gaussian6HBlurShader()
    {
        loadProgram(OBJECT, GL_VERTEX_SHADER, "screenquad.vert",
                            GL_FRAGMENT_SHADER, "gaussian6h.frag");
        assignUniforms("pixel", "sigma");

        assignSamplerNames(m_program, 0, "tex", ST_BILINEAR_CLAMPED_FILTERED);
    }   // Gaussian6HBlurShader
};   // Gaussian6HBlurShader

// ============================================================================
class Gaussian17TapHShader : public TextureShader<Gaussian17TapHShader, 2,
                                           core::vector2df>
{
public:
    Gaussian17TapHShader()
    {
        loadProgram(OBJECT, GL_VERTEX_SHADER, "screenquad.vert",
                            GL_FRAGMENT_SHADER, "bilateralH.frag");
        assignUniforms("pixel");
        assignSamplerNames(m_program,0, "tex", ST_BILINEAR_CLAMPED_FILTERED,
                                     1, "depth", ST_BILINEAR_CLAMPED_FILTERED);
    }   // Gaussian17TapHShader
};   // Gaussian17TapHShader

// ============================================================================
class ComputeGaussian17TapHShader : public TextureShader<ComputeGaussian17TapHShader, 2,
                                                  core::vector2df>
{
public:
    GLuint m_dest_tu;
    ComputeGaussian17TapHShader()
    {
        loadProgram(OBJECT,  GL_COMPUTE_SHADER, "bilateralH.comp");
        m_dest_tu = 2;
        assignUniforms("pixel");
        assignSamplerNames(m_program, 0, "source", ST_NEARED_CLAMPED_FILTERED,
                                      1, "depth", ST_NEARED_CLAMPED_FILTERED);
        assignTextureUnit(m_dest_tu, "dest");
    }   // ComputeGaussian17TapHShader
};   // ComputeGaussian17TapHShader


// ============================================================================
class Gaussian17TapVShader : public TextureShader<Gaussian17TapVShader, 2,
                                           core::vector2df>
{
public:
    Gaussian17TapVShader()
    {
        loadProgram(OBJECT, GL_VERTEX_SHADER, "screenquad.vert",
                            GL_FRAGMENT_SHADER, "bilateralV.frag");
        assignUniforms("pixel");

        assignSamplerNames(m_program,0, "tex", ST_BILINEAR_CLAMPED_FILTERED,
                                     1, "depth", ST_BILINEAR_CLAMPED_FILTERED);
    }   // Gaussian17TapVShader
};   // Gaussian17TapVShader

// ============================================================================
class ComputeGaussian17TapVShader : public TextureShader<ComputeGaussian17TapVShader, 2, 
                                                  core::vector2df>
{
public:
    GLuint m_dest_tu;

    ComputeGaussian17TapVShader()
    {
        loadProgram(OBJECT, GL_COMPUTE_SHADER, "bilateralV.comp");
        m_dest_tu = 2;
        assignUniforms("pixel");
        assignSamplerNames(m_program, 0, "source", ST_NEARED_CLAMPED_FILTERED,
                                      1, "depth", ST_NEARED_CLAMPED_FILTERED);
        assignTextureUnit(m_dest_tu, "dest");
    }   // ComputeGaussian17TapVShader
};   // ComputeGaussian17TapVShader

// ============================================================================
class BloomShader : public TextureShader<BloomShader, 1>
{
public:
    BloomShader()
    {
        loadProgram(OBJECT, GL_VERTEX_SHADER, "screenquad.vert",
                            GL_FRAGMENT_SHADER, "utils/getCIEXYZ.frag",
                            GL_FRAGMENT_SHADER, "utils/getRGBfromCIEXxy.frag",
                            GL_FRAGMENT_SHADER, "bloom.frag");
        assignUniforms();
        assignSamplerNames(m_program, 0, "tex", ST_NEAREST_FILTERED);
    }   // BloomShader
};   // BloomShader

// ============================================================================
class BloomBlendShader : public TextureShader<BloomBlendShader, 3>
{
public:
    BloomBlendShader()
    {
        loadProgram(OBJECT, GL_VERTEX_SHADER, "screenquad.vert",
                            GL_FRAGMENT_SHADER, "bloomblend.frag");
        assignUniforms();
        assignSamplerNames(m_program, 0, "tex_128", ST_BILINEAR_FILTERED,
                                      1, "tex_256", ST_BILINEAR_FILTERED,
                                      2, "tex_512", ST_BILINEAR_FILTERED);
    }   // BloomBlendShader
};   // BloomBlendShader

// ============================================================================
class LensBlendShader : public TextureShader<LensBlendShader, 3>
{
public:
    LensBlendShader()
    {
        loadProgram(OBJECT, GL_VERTEX_SHADER, "screenquad.vert",
                            GL_FRAGMENT_SHADER, "lensblend.frag");
        assignUniforms();

        assignSamplerNames(m_program, 0, "tex_128", ST_BILINEAR_FILTERED,
                                      1, "tex_256", ST_BILINEAR_FILTERED,
                                      2, "tex_512", ST_BILINEAR_FILTERED);
    }   // LensBlendShader
};   // LensBlendShader

// ============================================================================
class ToneMapShader : public TextureShader<ToneMapShader, 1, float>
{
public:

    ToneMapShader()
    {
        loadProgram(OBJECT, GL_VERTEX_SHADER, "screenquad.vert",
                            GL_FRAGMENT_SHADER, "utils/getRGBfromCIEXxy.frag",
                            GL_FRAGMENT_SHADER, "utils/getCIEXYZ.frag",
                            GL_FRAGMENT_SHADER, "tonemap.frag");
        assignUniforms("vignette_weight");
        assignSamplerNames(m_program, 0, "text", ST_NEAREST_FILTERED);
    }   // ToneMapShader
};   // ToneMapShader

// ============================================================================
class DepthOfFieldShader : public TextureShader<DepthOfFieldShader, 2>
{
public:
    DepthOfFieldShader()
    {
        loadProgram(OBJECT, GL_VERTEX_SHADER, "screenquad.vert",
                            GL_FRAGMENT_SHADER, "dof.frag");

        assignUniforms();
        assignSamplerNames(m_program, 0, "tex", ST_BILINEAR_FILTERED,
                                      1, "dtex", ST_NEAREST_FILTERED);
    }   // DepthOfFieldShader
};   // DepthOfFieldShader

// ============================================================================
class IBLShader : public TextureShader<IBLShader, 3>
{
public:
    IBLShader()
    {
        loadProgram(OBJECT, GL_VERTEX_SHADER, "screenquad.vert",
                            GL_FRAGMENT_SHADER, "utils/decodeNormal.frag",
                            GL_FRAGMENT_SHADER, "utils/getPosFromUVDepth.frag",
                            GL_FRAGMENT_SHADER, "utils/DiffuseIBL.frag",
                            GL_FRAGMENT_SHADER, "utils/SpecularIBL.frag",
                            GL_FRAGMENT_SHADER, "IBL.frag");
        assignUniforms();
        assignSamplerNames(m_program, 0, "ntex",  ST_NEAREST_FILTERED,
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
                            GL_FRAGMENT_SHADER, "utils/decodeNormal.frag",
                            GL_FRAGMENT_SHADER, "utils/getPosFromUVDepth.frag",
                            GL_FRAGMENT_SHADER, "utils/DiffuseIBL.frag",
                            GL_FRAGMENT_SHADER, "utils/SpecularIBL.frag",
                            GL_FRAGMENT_SHADER, "degraded_ibl.frag");
        assignUniforms();
        assignSamplerNames(m_program, 0, "ntex", ST_NEAREST_FILTERED);
    }   // DegradedIBLShader
};   // DegradedIBLShader

// ============================================================================

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

    MotionBlurProvider * const cb = 
        (MotionBlurProvider *) Shaders::getCallback(ES_MOTIONBLUR);

    for(unsigned int i=0; i<n; i++)
    {
        m_boost_time[i] = 0.0f;

        const core::recti &vp = Camera::getCamera(i)->getViewport();
        // Map viewport to [-1,1] x [-1,1]. First define the coordinates
        // left, right, top, bottom:
        float right = vp.LowerRightCorner.X < (int)irr_driver->getActualScreenSize().Width
                     ? 0.0f : 1.0f;
        float left   = vp.UpperLeftCorner.X  > 0.0f ? 0.0f : -1.0f;
        float top    = vp.UpperLeftCorner.Y  > 0.0f ? 0.0f : 1.0f;
        float bottom = vp.LowerRightCorner.Y < (int)irr_driver->getActualScreenSize().Height
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

// ----------------------------------------------------------------------------
void PostProcessing::setMotionBlurCenterY(const u32 num, const float y)
{
    MotionBlurProvider * const cb = 
        (MotionBlurProvider *) Shaders::getCallback(ES_MOTIONBLUR);

    const float tex_height =
                m_vertices[num].v1.TCoords.Y - m_vertices[num].v0.TCoords.Y;
    m_center[num].Y = m_vertices[num].v0.TCoords.Y + y * tex_height;

    cb->setCenter(num, m_center[num].X, m_center[num].Y);
}   // setMotionBlurCenterY

// ----------------------------------------------------------------------------
/** Setup some PP data.
  */
void PostProcessing::begin()
{
    m_any_boost = false;
    for (u32 i = 0; i < m_boost_time.size(); i++)
        m_any_boost |= m_boost_time[i] > 0.01f;
}   // begin

// ----------------------------------------------------------------------------
/** Set the boost amount according to the speed of the camera */
void PostProcessing::giveBoost(unsigned int camera_index)
{
    if (CVS->isGLSL())
    {
        m_boost_time[camera_index] = 0.75f;

        MotionBlurProvider * const cb =
            (MotionBlurProvider *)Shaders::getCallback(ES_MOTIONBLUR);
        cb->setBoostTime(camera_index, m_boost_time[camera_index]);
    }
}   // giveBoost

// ----------------------------------------------------------------------------
/** Updates the boost times for all cameras, called once per frame.
 *  \param dt Time step size.
 */
void PostProcessing::update(float dt)
{
    if (!CVS->isGLSL())
        return;

    MotionBlurProvider* const cb =
        (MotionBlurProvider*) Shaders::getCallback(ES_MOTIONBLUR);

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
static void renderBloom(GLuint in)
{
    BloomShader::getInstance()->setTextureUnits(in);
    DrawFullScreenEffect<BloomShader>();
}   // renderBloom

// ----------------------------------------------------------------------------
void PostProcessing::renderEnvMap(const float *bSHCoeff, const float *gSHCoeff,
                                  const float *rSHCoeff, GLuint skybox)
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
            ->setTextureUnits(irr_driver
                              ->getRenderTargetTexture(RTT_NORMAL_AND_DEPTH));
        DegradedIBLShader::getInstance()->setUniforms();
    }
    else
    {
        IBLShader::getInstance()->use();
        glBindVertexArray(SharedGPUObjects::getFullScreenQuadVAO());

        IBLShader::getInstance()->setTextureUnits(
            irr_driver->getRenderTargetTexture(RTT_NORMAL_AND_DEPTH),
            irr_driver->getDepthStencilTexture(), skybox);
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
void PostProcessing::renderRHDebug(unsigned SHR, unsigned SHG, unsigned SHB,
                                   const core::matrix4 &rh_matrix,
                                   const core::vector3df &rh_extend)
{
    glEnable(GL_PROGRAM_POINT_SIZE);
    FullScreenShader::RHDebug::getInstance()->use();
    glActiveTexture(GL_TEXTURE0 + FullScreenShader::RHDebug::getInstance()->TU_SHR);
    glBindTexture(GL_TEXTURE_3D, SHR);
    glActiveTexture(GL_TEXTURE0 + FullScreenShader::RHDebug::getInstance()->TU_SHG);
    glBindTexture(GL_TEXTURE_3D, SHG);
    glActiveTexture(GL_TEXTURE0 + FullScreenShader::RHDebug::getInstance()->TU_SHB);
    glBindTexture(GL_TEXTURE_3D, SHB);
    FullScreenShader::RHDebug::getInstance()->setUniforms(rh_matrix, rh_extend);
    glDrawArrays(GL_POINTS, 0, 32 * 16 * 32);
    glDisable(GL_PROGRAM_POINT_SIZE);
}   // renderRHDebug

// ----------------------------------------------------------------------------
void PostProcessing::renderGI(const core::matrix4 &RHMatrix,
                              const core::vector3df &rh_extend, GLuint shr,
                              GLuint shg, GLuint shb)
{
    core::matrix4 InvRHMatrix;
    RHMatrix.getInverse(InvRHMatrix);
    glDisable(GL_DEPTH_TEST);
    FullScreenShader::GlobalIlluminationReconstructionShader::getInstance()
        ->setTextureUnits(irr_driver->getRenderTargetTexture(RTT_NORMAL_AND_DEPTH), 
                          irr_driver->getDepthStencilTexture(), shr, shg, shb);
    DrawFullScreenEffect<FullScreenShader::GlobalIlluminationReconstructionShader>
                                     (RHMatrix, InvRHMatrix, rh_extend);
}   // renderGI

// ----------------------------------------------------------------------------
void PostProcessing::renderSunlight(const core::vector3df &direction,
                                    const video::SColorf &col)
{
    glEnable(GL_BLEND);
    glDisable(GL_DEPTH_TEST);
    glBlendFunc(GL_ONE, GL_ONE);
    glBlendEquation(GL_FUNC_ADD);

    FullScreenShader::SunLightShader::getInstance()
        ->setTextureUnits(irr_driver->getRenderTargetTexture(RTT_NORMAL_AND_DEPTH),
                          irr_driver->getDepthStencilTexture());
    DrawFullScreenEffect<FullScreenShader::SunLightShader>(direction, col);
}   // renderSunlight

// ----------------------------------------------------------------------------
static std::vector<float> getGaussianWeight(float sigma, size_t count)
{
    float g0, g1, g2, total;

    std::vector<float> weights;
    g0 = 1.f / (sqrtf(2.f * 3.14f) * sigma);
    g1 = exp(-.5f / (sigma * sigma));
    g2 = g1 * g1;
    total = g0;
    for (unsigned i = 0; i < count; i++)
    {
        weights.push_back(g0);
        g0 *= g1;
        g1 *= g2;
        total += 2 * g0;
    }
    for (float &weight : weights)
        weight /= total;
    return weights;
}   // getGaussianWeight

// ----------------------------------------------------------------------------
void PostProcessing::renderGaussian3Blur(FrameBuffer &in_fbo, 
                                         FrameBuffer &auxiliary)
{
    assert(in_fbo.getWidth() == auxiliary.getWidth() && 
           in_fbo.getHeight() == auxiliary.getHeight());
    float inv_width  = 1.0f / in_fbo.getWidth();
    float inv_height = 1.0f / in_fbo.getHeight();
    {
        auxiliary.Bind();

        Gaussian3VBlurShader::getInstance()
            ->setTextureUnits(in_fbo.getRTT()[0]);
        DrawFullScreenEffect<Gaussian3VBlurShader>
                                     (core::vector2df(inv_width, inv_height));
    }
    {
        in_fbo.Bind();

        Gaussian3HBlurShader::getInstance()
            ->setTextureUnits(auxiliary.getRTT()[0]);
        DrawFullScreenEffect<Gaussian3HBlurShader>
                                      (core::vector2df(inv_width, inv_height));
    }
}   // renderGaussian3Blur

// ----------------------------------------------------------------------------
void PostProcessing::renderGaussian6BlurLayer(FrameBuffer &in_fbo,
                                              size_t layer, float sigmaH,
                                              float sigmaV)
{
    GLuint LayerTex;
    glGenTextures(1, &LayerTex);
    glTextureView(LayerTex, GL_TEXTURE_2D, in_fbo.getRTT()[0],
                  GL_R32F, 0, 1, layer, 1);
    if (!CVS->supportsComputeShadersFiltering())
    {
        // Used as temp
        irr_driver->getFBO(FBO_SCALAR_1024).Bind();
        Gaussian6VBlurShader::getInstance()->setTextureUnits(LayerTex);
        DrawFullScreenEffect<Gaussian6VBlurShader>
            (core::vector2df(1.f / UserConfigParams::m_shadows_resolution, 
                             1.f / UserConfigParams::m_shadows_resolution), 
            sigmaV);
        in_fbo.BindLayer(layer);
        Gaussian6HBlurShader::getInstance()
            ->setTextureUnits(irr_driver->getFBO(FBO_SCALAR_1024).getRTT()[0]);
        DrawFullScreenEffect<Gaussian6HBlurShader>
            (core::vector2df(1.f / UserConfigParams::m_shadows_resolution,
                             1.f / UserConfigParams::m_shadows_resolution),
             sigmaH);
    }
    else
    {
        const std::vector<float> &weightsV = getGaussianWeight(sigmaV, 7);
        glMemoryBarrier(GL_TEXTURE_FETCH_BARRIER_BIT);
        ComputeShadowBlurVShader::getInstance()->use();
        ComputeShadowBlurVShader::getInstance()->setTextureUnits(LayerTex);
        glBindSampler(ComputeShadowBlurVShader::getInstance()->m_dest_tu, 0);
        glBindImageTexture(ComputeShadowBlurVShader::getInstance()->m_dest_tu,
                           irr_driver->getFBO(FBO_SCALAR_1024).getRTT()[0], 0,
                           false, 0, GL_WRITE_ONLY, GL_R32F);
        ComputeShadowBlurVShader::getInstance()->setUniforms
            (core::vector2df(1.f / UserConfigParams::m_shadows_resolution,
                             1.f / UserConfigParams::m_shadows_resolution),
             weightsV);
        glDispatchCompute((int)UserConfigParams::m_shadows_resolution / 8 + 1,
                          (int)UserConfigParams::m_shadows_resolution / 8 + 1, 1);

        const std::vector<float> &weightsH = getGaussianWeight(sigmaH, 7);
        glMemoryBarrier(  GL_TEXTURE_FETCH_BARRIER_BIT 
                        | GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
        ComputeShadowBlurHShader::getInstance()->use();
        ComputeShadowBlurHShader::getInstance()
            ->setTextureUnits(irr_driver->getFBO(FBO_SCALAR_1024).getRTT()[0]);
        glBindSampler(ComputeShadowBlurHShader::getInstance()->m_dest_tu, 0);
        glBindImageTexture(ComputeShadowBlurHShader::getInstance()->m_dest_tu,
                           LayerTex, 0, false, 0, GL_WRITE_ONLY, GL_R32F);
        ComputeShadowBlurHShader::getInstance()->setUniforms
            (core::vector2df(1.f / UserConfigParams::m_shadows_resolution,
                            1.f / UserConfigParams::m_shadows_resolution),
              weightsH);
        glDispatchCompute((int)UserConfigParams::m_shadows_resolution / 8 + 1,
                          (int)UserConfigParams::m_shadows_resolution / 8 + 1, 1);
        glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
    }
    glDeleteTextures(1, &LayerTex);
}   // renderGaussian6BlurLayer

// ----------------------------------------------------------------------------
void PostProcessing::renderGaussian6Blur(FrameBuffer &in_fbo,
                                         FrameBuffer &auxiliary, float sigmaV,
                                         float sigmaH)
{
    assert(in_fbo.getWidth() == auxiliary.getWidth() && 
           in_fbo.getHeight() == auxiliary.getHeight());
    float inv_width = 1.0f / in_fbo.getWidth();
    float inv_height = 1.0f / in_fbo.getHeight();

    if (!CVS->supportsComputeShadersFiltering())
    {
        auxiliary.Bind();

        Gaussian6VBlurShader::getInstance()->setTextureUnits(in_fbo.getRTT()[0]);
        DrawFullScreenEffect<Gaussian6VBlurShader>
                        (core::vector2df(inv_width, inv_height), sigmaV);

        in_fbo.Bind();

        Gaussian6HBlurShader::getInstance()->setTextureUnits(auxiliary.getRTT()[0]);
        DrawFullScreenEffect<Gaussian6HBlurShader>
                        (core::vector2df(inv_width, inv_height), sigmaH);
    }
    else
    {
        const std::vector<float> &weightsV = getGaussianWeight(sigmaV, 7);
        glMemoryBarrier(GL_TEXTURE_FETCH_BARRIER_BIT);
        ComputeGaussian6VBlurShader::getInstance()->use();
        ComputeGaussian6VBlurShader::getInstance()
            ->setTextureUnits(in_fbo.getRTT()[0]);
        glBindSampler(ComputeGaussian6VBlurShader::getInstance()->m_dest_tu, 0);
        glBindImageTexture(ComputeGaussian6VBlurShader::getInstance()->m_dest_tu,
                            auxiliary.getRTT()[0], 0, false, 0,
                            GL_WRITE_ONLY, GL_RGBA16F);
        ComputeGaussian6VBlurShader::getInstance()
            ->setUniforms(core::vector2df(inv_width, inv_height), weightsV);
        glDispatchCompute((int)in_fbo.getWidth() / 8 + 1, 
                          (int)in_fbo.getHeight() / 8 + 1, 1);

        const std::vector<float> &weightsH = getGaussianWeight(sigmaH, 7);
        glMemoryBarrier(  GL_TEXTURE_FETCH_BARRIER_BIT 
                        | GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
        ComputeGaussian6HBlurShader::getInstance()->use();
        ComputeGaussian6HBlurShader::getInstance()
                         ->setTextureUnits(auxiliary.getRTT()[0]);
        glBindSampler(ComputeGaussian6HBlurShader::getInstance()->m_dest_tu, 0);
        glBindImageTexture(ComputeGaussian6HBlurShader::getInstance()->m_dest_tu,
                           in_fbo.getRTT()[0], 0, false, 0,
                           GL_WRITE_ONLY, GL_RGBA16F);
        ComputeGaussian6HBlurShader::getInstance()
                ->setUniforms(core::vector2df(inv_width, inv_height), weightsH);
        glDispatchCompute((int)in_fbo.getWidth() / 8 + 1,
                          (int)in_fbo.getHeight() / 8 + 1, 1);
        glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
    }

}   // renderGaussian6Blur

// ----------------------------------------------------------------------------
void PostProcessing::renderHorizontalBlur(FrameBuffer &in_fbo,
                                          FrameBuffer &auxiliary)
{
    assert(in_fbo.getWidth() == auxiliary.getWidth() &&
           in_fbo.getHeight() == auxiliary.getHeight());
    float inv_width  = 1.0f / in_fbo.getWidth();
    float inv_height = 1.0f / in_fbo.getHeight();
    {
        auxiliary.Bind();

        Gaussian6HBlurShader::getInstance()
                                ->setTextureUnits(in_fbo.getRTT()[0]);
        DrawFullScreenEffect<Gaussian6HBlurShader>
                         (core::vector2df(inv_width, inv_height), 2.0f);
    }
    {
        in_fbo.Bind();

        Gaussian6HBlurShader::getInstance()
                      ->setTextureUnits(auxiliary.getRTT()[0]);
        DrawFullScreenEffect<Gaussian6HBlurShader>
                   (core::vector2df(inv_width, inv_height), 2.0f);
    }
}   // renderHorizontalBlur


// ----------------------------------------------------------------------------
void PostProcessing::renderGaussian17TapBlur(FrameBuffer &in_fbo,
                                             FrameBuffer &auxiliary)
{
    assert(in_fbo.getWidth() == auxiliary.getWidth() &&
           in_fbo.getHeight() == auxiliary.getHeight());
    if (CVS->supportsComputeShadersFiltering())
        glMemoryBarrier(GL_FRAMEBUFFER_BARRIER_BIT);
    float inv_width = 1.0f / in_fbo.getWidth();
    float inv_height = 1.0f / in_fbo.getHeight();
    {
        if (!CVS->supportsComputeShadersFiltering())
        {
            auxiliary.Bind();
            Gaussian17TapHShader::getInstance()
                ->setTextureUnits(in_fbo.getRTT()[0], 
                             irr_driver->getFBO(FBO_LINEAR_DEPTH).getRTT()[0]);
            DrawFullScreenEffect<Gaussian17TapHShader>
                                      (core::vector2df(inv_width, inv_height));
        }
        else
        {
            ComputeGaussian17TapHShader::getInstance()->use();
            glBindSampler(ComputeGaussian17TapHShader::getInstance()
                          ->m_dest_tu, 0);
            ComputeGaussian17TapHShader::getInstance()
                ->setTextureUnits(in_fbo.getRTT()[0], 
                             irr_driver->getFBO(FBO_LINEAR_DEPTH).getRTT()[0]);
            glBindImageTexture(ComputeGaussian17TapHShader::getInstance()
                               ->m_dest_tu, auxiliary.getRTT()[0], 0, false,
                               0, GL_WRITE_ONLY, GL_R16F);
            ComputeGaussian17TapHShader::getInstance()
                ->setUniforms(core::vector2df(inv_width, inv_height));
            glDispatchCompute((int)in_fbo.getWidth()  / 8 + 1,
                              (int)in_fbo.getHeight() / 8 + 1, 1);
        }
    }
    if (CVS->supportsComputeShadersFiltering())
        glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
    {
        if (!CVS->supportsComputeShadersFiltering())
        {
            in_fbo.Bind();

            Gaussian17TapVShader::getInstance()
                ->setTextureUnits(auxiliary.getRTT()[0], 
                             irr_driver->getFBO(FBO_LINEAR_DEPTH).getRTT()[0]);
            DrawFullScreenEffect<Gaussian17TapVShader>
                              (core::vector2df(inv_width, inv_height));
        }
        else
        {
            ComputeGaussian17TapVShader::getInstance()->use();
            glBindSampler(ComputeGaussian17TapVShader::getInstance()
                          ->m_dest_tu, 0);
            ComputeGaussian17TapVShader::getInstance()
                ->setTextureUnits(auxiliary.getRTT()[0],
                             irr_driver->getFBO(FBO_LINEAR_DEPTH).getRTT()[0]);
            glBindImageTexture(ComputeGaussian17TapVShader::getInstance()
                                ->m_dest_tu, in_fbo.getRTT()[0], 0, false, 0,
                                GL_WRITE_ONLY, GL_R16F);
            ComputeGaussian17TapVShader::getInstance()
                ->setUniforms(core::vector2df(inv_width, inv_height));
            glDispatchCompute((int)in_fbo.getWidth() / 8 + 1,
                              (int)in_fbo.getHeight() / 8 + 1, 1);
        }
    }
    if (CVS->supportsComputeShadersFiltering())
        glMemoryBarrier(GL_TEXTURE_FETCH_BARRIER_BIT);
}   // renderGaussian17TapBlur

// ----------------------------------------------------------------------------
void PostProcessing::renderPassThrough(GLuint tex, unsigned width,
                                       unsigned height)
{
    FullScreenShader::PassThroughShader::getInstance()->setTextureUnits(tex);
    DrawFullScreenEffect<FullScreenShader::PassThroughShader>(width, height);
}   // renderPassThrough

// ----------------------------------------------------------------------------
void PostProcessing::renderTextureLayer(unsigned tex, unsigned layer)
{
    FullScreenShader::LayerPassThroughShader::getInstance()->use();
    glBindVertexArray(FullScreenShader::LayerPassThroughShader::getInstance()->vao);

    glActiveTexture(GL_TEXTURE0 + FullScreenShader::LayerPassThroughShader
                                                  ::getInstance()->TU_texture);
    glBindTexture(GL_TEXTURE_2D_ARRAY, tex);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    FullScreenShader::LayerPassThroughShader::getInstance()->setUniforms(layer);

    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
}   // renderTextureLayer

// ----------------------------------------------------------------------------
void PostProcessing::renderGlow(unsigned tex)
{
    FullScreenShader::GlowShader::getInstance()->use();
    glBindVertexArray(FullScreenShader::GlowShader::getInstance()->vao);

    FullScreenShader::GlowShader::getInstance()->setTextureUnits(tex);
    FullScreenShader::GlowShader::getInstance()->setUniforms();

    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
}   // renderGlow

// ----------------------------------------------------------------------------
void PostProcessing::renderSSAO()
{
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_BLEND);

    // Generate linear depth buffer
    irr_driver->getFBO(FBO_LINEAR_DEPTH).Bind();
    FullScreenShader::LinearizeDepthShader::getInstance()
        ->setTextureUnits(irr_driver->getDepthStencilTexture());
    DrawFullScreenEffect<FullScreenShader::LinearizeDepthShader>
        (irr_driver->getSceneManager()->getActiveCamera()->getNearValue(), 
         irr_driver->getSceneManager()->getActiveCamera()->getFarValue()  );
    irr_driver->getFBO(FBO_SSAO).Bind();

    FullScreenShader::SSAOShader::getInstance()
        ->setTextureUnits(irr_driver->getRenderTargetTexture(RTT_LINEAR_DEPTH));
    glGenerateMipmap(GL_TEXTURE_2D);

    DrawFullScreenEffect<FullScreenShader::SSAOShader>
        (irr_driver->getSSAORadius(), irr_driver->getSSAOK(),
         irr_driver->getSSAOSigma()                           );
}
// ----------------------------------------------------------------------------
void PostProcessing::renderMotionBlur(unsigned , FrameBuffer &in_fbo,
                                      FrameBuffer &out_fbo)
{
    MotionBlurProvider * const cb =
                      (MotionBlurProvider *)Shaders::getCallback(ES_MOTIONBLUR);
    Camera *cam = Camera::getActiveCamera();
    unsigned camID = cam->getIndex();

    scene::ICameraSceneNode * const camnode = cam->getCameraSceneNode();

    // Calculate the kart's Y position on screen
    if (cam->getKart())
    {
        const core::vector3df pos = cam->getKart()->getNode()->getPosition();
        float ndc[4];
        core::matrix4 trans = camnode->getProjectionMatrix();
        trans *= camnode->getViewMatrix();

        trans.transformVect(ndc, pos);
        const float karty = (ndc[1] / ndc[3]) * 0.5f + 0.5f;
        setMotionBlurCenterY(camID, karty);
    }
    else
        setMotionBlurCenterY(camID, 0.5f);

    out_fbo.Bind();
    glClear(GL_COLOR_BUFFER_BIT);

    FullScreenShader::MotionBlurShader::getInstance()
        ->setTextureUnits(in_fbo.getRTT()[0], 
                          irr_driver->getDepthStencilTexture());
    DrawFullScreenEffect<FullScreenShader::MotionBlurShader>
        (// Todo : use a previousPVMatrix per cam, not global
         cam->getPreviousPVMatrix(),
         core::vector2df(0.5, 0.5),
         cb->getBoostTime(cam->getIndex()) * 10, // Todo : should be framerate dependent
         0.15f);
}   // renderMotionBlur

// ----------------------------------------------------------------------------
static void renderGodFade(GLuint tex, const SColor &col)
{
    FullScreenShader::GodFadeShader::getInstance()->setTextureUnits(tex);
    DrawFullScreenEffect<FullScreenShader::GodFadeShader>(col);
}   // renderGodFade

// ----------------------------------------------------------------------------
static void renderGodRay(GLuint tex, const core::vector2df &sunpos)
{
    FullScreenShader::GodRayShader::getInstance()->setTextureUnits(tex);
    DrawFullScreenEffect<FullScreenShader::GodRayShader>(sunpos);
}   // renderGodRay

// ----------------------------------------------------------------------------
static void toneMap(FrameBuffer &fbo, GLuint rtt, float vignette_weight)
{
    fbo.Bind();
    ToneMapShader::getInstance()->setTextureUnits(rtt);
    DrawFullScreenEffect<ToneMapShader>(vignette_weight);
}   // toneMap

// ----------------------------------------------------------------------------
static void renderDoF(FrameBuffer &fbo, GLuint rtt)
{
    fbo.Bind();
    DepthOfFieldShader::getInstance()
        ->setTextureUnits(rtt, irr_driver->getDepthStencilTexture());
    DrawFullScreenEffect<DepthOfFieldShader>();
}   // renderDoF

// ----------------------------------------------------------------------------
void PostProcessing::applyMLAA()
{
    const core::vector2df &PIXEL_SIZE =
                     core::vector2df(1.0f / UserConfigParams::m_width,
                                     1.0f / UserConfigParams::m_height);

    irr_driver->getFBO(FBO_MLAA_TMP).Bind();
    glEnable(GL_STENCIL_TEST);
    glClearColor(0.0, 0.0, 0.0, 1.0);
    glClear(GL_STENCIL_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
    glStencilFunc(GL_ALWAYS, 1, ~0);
    glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);

    // Pass 1: color edge detection
    FullScreenShader::MLAAColorEdgeDetectionSHader::getInstance()->use();
    FullScreenShader::MLAAColorEdgeDetectionSHader::getInstance()
        ->setTextureUnits(irr_driver->getRenderTargetTexture(RTT_MLAA_COLORS));
    DrawFullScreenEffect<FullScreenShader::MLAAColorEdgeDetectionSHader>
                                                                  (PIXEL_SIZE);

    glStencilFunc(GL_EQUAL, 1, ~0);
    glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);

    // Pass 2: blend weights
    irr_driver->getFBO(FBO_MLAA_BLEND).Bind();
    glClear(GL_COLOR_BUFFER_BIT);

    FullScreenShader::MLAABlendWeightSHader::getInstance()->use();
    FullScreenShader::MLAABlendWeightSHader::getInstance()
        ->setTextureUnits(irr_driver->getRenderTargetTexture(RTT_MLAA_TMP),
                          getTextureGLuint(m_areamap));
    DrawFullScreenEffect<FullScreenShader::MLAABlendWeightSHader>(PIXEL_SIZE);

    // Blit in to tmp1
    FrameBuffer::Blit(irr_driver->getFBO(FBO_MLAA_COLORS),
                      irr_driver->getFBO(FBO_MLAA_TMP));

    // Pass 3: gather
    irr_driver->getFBO(FBO_MLAA_COLORS).Bind();

    FullScreenShader::MLAAGatherSHader::getInstance()->use();
    FullScreenShader::MLAAGatherSHader::getInstance()
        ->setTextureUnits(irr_driver->getRenderTargetTexture(RTT_MLAA_BLEND),
                          irr_driver->getRenderTargetTexture(RTT_MLAA_TMP));
    DrawFullScreenEffect<FullScreenShader::MLAAGatherSHader>(PIXEL_SIZE);

    // Done.
    glDisable(GL_STENCIL_TEST);
}   // applyMLAA

// ----------------------------------------------------------------------------
/** Render the post-processed scene */
FrameBuffer *PostProcessing::render(scene::ICameraSceneNode * const camnode,
                                    bool isRace)
{
    FrameBuffer *in_fbo = &irr_driver->getFBO(FBO_COLORS);
    FrameBuffer *out_fbo = &irr_driver->getFBO(FBO_TMP1_WITH_DS);
    // Each effect uses these as named, and sets them up for the next effect.
    // This allows chaining effects where some may be disabled.

    // As the original color shouldn't be touched, the first effect
    // can't be disabled.
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
            glViewport(0, 0, irr_driver->getActualScreenSize().Width / 4,
                             irr_driver->getActualScreenSize().Height / 4);
            renderGodFade(out_fbo->getRTT()[0], col);

            // Blur
            renderGaussian3Blur(irr_driver->getFBO(FBO_QUARTER1),
                                irr_driver->getFBO(FBO_QUARTER2));

            // Calculate the sun's position in texcoords
            const core::vector3df pos = track->getGodRaysPosition();
            float ndc[4];
            core::matrix4 trans = camnode->getProjectionMatrix();
            trans *= camnode->getViewMatrix();

            trans.transformVect(ndc, pos);

            const float texh = 
                m_vertices[0].v1.TCoords.Y - m_vertices[0].v0.TCoords.Y;
            const float texw = 
                m_vertices[0].v3.TCoords.X - m_vertices[0].v0.TCoords.X;

            const float sunx = ((ndc[0] / ndc[3]) * 0.5f + 0.5f) * texw;
            const float suny = ((ndc[1] / ndc[3]) * 0.5f + 0.5f) * texh;

            // Rays please
            irr_driver->getFBO(FBO_QUARTER2).Bind();
            renderGodRay(irr_driver->getRenderTargetTexture(RTT_QUARTER1),
                         core::vector2df(sunx, suny));

            // Blur
            renderGaussian3Blur(irr_driver->getFBO(FBO_QUARTER2),
                                irr_driver->getFBO(FBO_QUARTER1));

            // Blend
            glEnable(GL_BLEND);
            glBlendColor(0., 0., 0., track->getGodRaysOpacity());
            glBlendFunc(GL_CONSTANT_ALPHA, GL_ONE);
            glBlendEquation(GL_FUNC_ADD);

            in_fbo->Bind();
            renderPassThrough(irr_driver->getRenderTargetTexture(RTT_QUARTER2),
                              in_fbo->getWidth(), in_fbo->getHeight());
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

            FrameBuffer::Blit(*in_fbo, irr_driver->getFBO(FBO_BLOOM_1024),
                              GL_COLOR_BUFFER_BIT, GL_LINEAR);

            irr_driver->getFBO(FBO_BLOOM_512).Bind();
            renderBloom(irr_driver->getRenderTargetTexture(RTT_BLOOM_1024));

            // Downsample
            FrameBuffer::Blit(irr_driver->getFBO(FBO_BLOOM_512),
                              irr_driver->getFBO(FBO_BLOOM_256), 
                              GL_COLOR_BUFFER_BIT, GL_LINEAR);
            FrameBuffer::Blit(irr_driver->getFBO(FBO_BLOOM_256),
                              irr_driver->getFBO(FBO_BLOOM_128),
                              GL_COLOR_BUFFER_BIT, GL_LINEAR);

			// Copy for lens flare
			FrameBuffer::Blit(irr_driver->getFBO(FBO_BLOOM_512),
                              irr_driver->getFBO(FBO_LENS_512), 
                              GL_COLOR_BUFFER_BIT, GL_LINEAR);
			FrameBuffer::Blit(irr_driver->getFBO(FBO_BLOOM_256),
                              irr_driver->getFBO(FBO_LENS_256),
                              GL_COLOR_BUFFER_BIT, GL_LINEAR);
			FrameBuffer::Blit(irr_driver->getFBO(FBO_BLOOM_128),
                              irr_driver->getFBO(FBO_LENS_128),
                              GL_COLOR_BUFFER_BIT, GL_LINEAR);
			

            // Blur
            renderGaussian6Blur(irr_driver->getFBO(FBO_BLOOM_512),
                                irr_driver->getFBO(FBO_TMP_512), 1., 1.);
            renderGaussian6Blur(irr_driver->getFBO(FBO_BLOOM_256),
                                irr_driver->getFBO(FBO_TMP_256), 1., 1.);
            renderGaussian6Blur(irr_driver->getFBO(FBO_BLOOM_128),
                                irr_driver->getFBO(FBO_TMP_128), 1., 1.);

            renderHorizontalBlur(irr_driver->getFBO(FBO_LENS_512), 
                                 irr_driver->getFBO(FBO_TMP_512));
            renderHorizontalBlur(irr_driver->getFBO(FBO_LENS_256),
                                 irr_driver->getFBO(FBO_TMP_256));
            renderHorizontalBlur(irr_driver->getFBO(FBO_LENS_128), 
                                 irr_driver->getFBO(FBO_TMP_128));
            

            // Additively blend on top of tmp1
            in_fbo->Bind();
            glEnable(GL_BLEND);
            glBlendFunc(GL_ONE, GL_ONE);
            glBlendEquation(GL_FUNC_ADD);
            
            BloomBlendShader::getInstance()->setTextureUnits(
                irr_driver->getRenderTargetTexture(RTT_BLOOM_128),
                irr_driver->getRenderTargetTexture(RTT_BLOOM_256),
                irr_driver->getRenderTargetTexture(RTT_BLOOM_512)  );
            DrawFullScreenEffect<BloomBlendShader>();

            LensBlendShader::getInstance()->setTextureUnits(
                irr_driver->getRenderTargetTexture(RTT_LENS_128),
                irr_driver->getRenderTargetTexture(RTT_LENS_256),
                irr_driver->getRenderTargetTexture(RTT_LENS_512));
            DrawFullScreenEffect<LensBlendShader>();


            glDisable(GL_BLEND);
            glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
        } // end if bloom
        PROFILER_POP_CPU_MARKER();
    }

    //computeLogLuminance(in_rtt);
    {
        PROFILER_PUSH_CPU_MARKER("- Tonemap", 0xFF, 0x00, 0x00);
        ScopedGPUTimer Timer(irr_driver->getGPUTimer(Q_TONEMAP));
		// only enable vignette during race
		if(isRace)
		{
			toneMap(*out_fbo, in_fbo->getRTT()[0], 1.0);
		}
		else
		{
			toneMap(*out_fbo, in_fbo->getRTT()[0], 0.0);
		}
        std::swap(in_fbo, out_fbo);
        PROFILER_POP_CPU_MARKER();
    }

    {
        PROFILER_PUSH_CPU_MARKER("- Motion blur", 0xFF, 0x00, 0x00);
        ScopedGPUTimer Timer(irr_driver->getGPUTimer(Q_MOTIONBLUR));
        MotionBlurProvider * const cb = 
            (MotionBlurProvider *)Shaders::getCallback(ES_MOTIONBLUR);

        if (isRace && UserConfigParams::m_motionblur && World::getWorld() &&
            cb->getBoostTime(Camera::getActiveCamera()->getIndex()) > 0.) // motion blur
        {
            renderMotionBlur(0, *in_fbo, *out_fbo);
            std::swap(in_fbo, out_fbo);
        }
        PROFILER_POP_CPU_MARKER();
    }

    // Workaround a bug with srgb fbo on sandy bridge windows
    if (!CVS->isARBUniformBufferObjectUsable())
        return in_fbo;

    glEnable(GL_FRAMEBUFFER_SRGB);
    irr_driver->getFBO(FBO_MLAA_COLORS).Bind();
    renderPassThrough(in_fbo->getRTT()[0],
                      irr_driver->getFBO(FBO_MLAA_COLORS).getWidth(),
                      irr_driver->getFBO(FBO_MLAA_COLORS).getHeight());
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
