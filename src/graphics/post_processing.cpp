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

#ifndef SERVER_ONLY

#include "graphics/post_processing.hpp"

#include "config/user_config.hpp"
#include "graphics/camera.hpp"
#include "graphics/central_settings.hpp"
#include "graphics/frame_buffer.hpp"
#include "graphics/graphics_restrictions.hpp"
#include "graphics/irr_driver.hpp"
#include "graphics/mlaa_areamap.hpp"
#include "graphics/glwrap.hpp"
#include "graphics/rtts.hpp"
#include "graphics/shared_gpu_objects.hpp"
#include "graphics/stk_tex_manager.hpp"
#include "graphics/texture_shader.hpp"
#include "graphics/weather.hpp"
#include "graphics/sp/sp_dynamic_draw_call.hpp"
#include "graphics/sp/sp_shader.hpp"
#include "graphics/sp/sp_uniform_assigner.hpp"
#include "io/file_manager.hpp"
#include "karts/abstract_kart.hpp"
#include "karts/kart_model.hpp"
#include "modes/world.hpp"
#include "physics/physics.hpp"
#include "race/race_manager.hpp"
#include "tracks/track.hpp"
#include "utils/log.hpp"
#include "utils/profiler.hpp"
#include "utils/cpp2011.hpp"

#include <IrrlichtDevice.h>
#include <IFileSystem.h>
#include <ISceneManager.h>
#include <SViewFrustum.h>
#include <ge_texture.hpp>

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

        assignSamplerNames(0, "tex", ST_BILINEAR_CLAMPED_FILTERED);
    }   // Gaussian3HBlurShader
    // ------------------------------------------------------------------------
    void render(const FrameBuffer &auxiliary, float inv_width,
                float inv_height)
    {
        setTextureUnits(auxiliary.getRTT()[0]);
        drawFullScreenEffect(core::vector2df(inv_width, inv_height));
    }   // render
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
#if !defined(USE_GLES2)
        loadProgram(OBJECT, GL_COMPUTE_SHADER, "blurshadowV.comp");
        m_dest_tu = 1;
        assignUniforms("pixel", "weights");
        assignSamplerNames(0, "source", ST_NEARED_CLAMPED_FILTERED);
        assignTextureUnit(m_dest_tu, "dest");
#endif
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

        assignSamplerNames(0, "tex", ST_BILINEAR_CLAMPED_FILTERED);
    }   // Gaussian6VBlurShader
    // ------------------------------------------------------------------------
    void render(GLuint layer_tex, int width, int height, float sigma_v)
    {
        setTextureUnits(layer_tex);
        drawFullScreenEffect(core::vector2df(1.f / width, 1.f / height),
                             sigma_v);
    }   // render
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

        assignSamplerNames(0, "tex", ST_BILINEAR_CLAMPED_FILTERED);
    }   // Gaussian3VBlurShader
    // ------------------------------------------------------------------------
    void render(const FrameBuffer &in_fbo, float inv_width, float inv_height)
    {
        setTextureUnits(in_fbo.getRTT()[0]);
        drawFullScreenEffect(core::vector2df(inv_width, inv_height));
    }   // render
};   // Gaussian3VBlurShader

#if !defined(USE_GLES2)
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
        assignSamplerNames(0, "source", ST_BILINEAR_CLAMPED_FILTERED);
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
        assignSamplerNames(0, "source",  ST_BILINEAR_CLAMPED_FILTERED);
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
        assignSamplerNames(0, "source", ST_NEARED_CLAMPED_FILTERED);
        assignTextureUnit(m_dest_tu, "dest");
    }   // ComputeShadowBlurHShader
};   // ComputeShadowBlurHShader
#endif

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

        assignSamplerNames(0, "tex", ST_BILINEAR_CLAMPED_FILTERED);
    }   // Gaussian6HBlurShader
    // ------------------------------------------------------------------------
    void render(const FrameBuffer &fb, int width, int height, float sigma_h)
    {
        setTextureUnits(fb.getRTT()[0]);
        drawFullScreenEffect(
            core::vector2df(1.f / width,
                            1.f / height),
            sigma_h);
    }   // renderq
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
        assignSamplerNames(0, "tex", ST_BILINEAR_CLAMPED_FILTERED,
                           1, "depth", ST_BILINEAR_CLAMPED_FILTERED);
    }   // Gaussian17TapHShader
    // ------------------------------------------------------------------------
    void render(const FrameBuffer &fb, const FrameBuffer &linear_depth,
                int width, int height)
    {
        setTextureUnits(fb.getRTT()[0],
                        linear_depth.getRTT()[0] );
        drawFullScreenEffect(core::vector2df(1.0f/width, 1.0f/height));

    }   // render
};   // Gaussian17TapHShader

// ============================================================================
class ComputeGaussian17TapHShader : public TextureShader<ComputeGaussian17TapHShader, 2,
                                                  core::vector2df>
{
public:
    GLuint m_dest_tu;
    ComputeGaussian17TapHShader()
    {
#if !defined(USE_GLES2)
        loadProgram(OBJECT,  GL_COMPUTE_SHADER, "bilateralH.comp");
        m_dest_tu = 2;
        assignUniforms("pixel");
        assignSamplerNames(0, "source", ST_NEARED_CLAMPED_FILTERED,
                           1, "depth", ST_NEARED_CLAMPED_FILTERED);
        assignTextureUnit(m_dest_tu, "dest");
#endif
    }   // ComputeGaussian17TapHShader
    // ------------------------------------------------------------------------
    void render(const FrameBuffer &fb, const FrameBuffer &auxiliary,
                const FrameBuffer &linear_depth,
                int width, int height)
    {
#if !defined(USE_GLES2)
        use();
        glBindSampler(m_dest_tu, 0);
        setTextureUnits(fb.getRTT()[0],
                        linear_depth.getRTT()[0]);
        glBindImageTexture(m_dest_tu, auxiliary.getRTT()[0], 0, false,
                           0, GL_WRITE_ONLY, GL_R16F);
        setUniforms(core::vector2df(1.0f/width, 1.0f/height));
        glDispatchCompute((int)width / 8 + 1, (int)height / 8 + 1, 1);
#endif
    }   // render
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

        assignSamplerNames(0, "tex", ST_BILINEAR_CLAMPED_FILTERED,
                           1, "depth", ST_BILINEAR_CLAMPED_FILTERED);
    }   // Gaussian17TapVShader
    // ------------------------------------------------------------------------
    void render(const FrameBuffer &auxiliary, const FrameBuffer &linear_depth,
                int width, int height)
    {
        setTextureUnits(auxiliary.getRTT()[0],
                        linear_depth.getRTT()[0]);
        drawFullScreenEffect(core::vector2df(1.0f/width, 1.0f/height));

    }   // render
};   // Gaussian17TapVShader

// ============================================================================
class ComputeGaussian17TapVShader : public TextureShader<ComputeGaussian17TapVShader, 2,
                                                  core::vector2df>
{
public:
    GLuint m_dest_tu;

    ComputeGaussian17TapVShader()
    {
#if !defined(USE_GLES2)
        loadProgram(OBJECT, GL_COMPUTE_SHADER, "bilateralV.comp");
        m_dest_tu = 2;
        assignUniforms("pixel");
        assignSamplerNames(0, "source", ST_NEARED_CLAMPED_FILTERED,
                           1, "depth", ST_NEARED_CLAMPED_FILTERED);
        assignTextureUnit(m_dest_tu, "dest");
#endif
    }   // ComputeGaussian17TapVShader
    // ------------------------------------------------------------------------
    void render(const FrameBuffer &auxiliary, const FrameBuffer &fb,
                const FrameBuffer &linear_depth,
                int width, int height)
    {
#if !defined(USE_GLES2)

        use();
        glBindSampler(m_dest_tu, 0);
        setTextureUnits(auxiliary.getRTT()[0],
                        linear_depth.getRTT()[0]);
        glBindImageTexture(m_dest_tu, fb.getRTT()[0], 0, false, 0,
                           GL_WRITE_ONLY, GL_R16F);
        setUniforms(core::vector2df(1.0f/width, 1.0f/height));
        glDispatchCompute((int)fb.getWidth()  / 8 + 1,
                          (int)fb.getHeight() / 8 + 1, 1);
#endif
    }   // render
};   // ComputeGaussian17TapVShader

// ============================================================================
class BloomShader : public TextureShader<BloomShader, 1, float>
{
public:
    BloomShader()
    {
        loadProgram(OBJECT, GL_VERTEX_SHADER, "screenquad.vert",
                            GL_FRAGMENT_SHADER, "bloom.frag");
        assignUniforms("scale");
        assignSamplerNames(0, "tex", ST_NEAREST_FILTERED);
    }   // BloomShader
    // ------------------------------------------------------------------------
    void render(GLuint in)
    {
        BloomShader::getInstance()->setTextureUnits(in);
        drawFullScreenEffect(UserConfigParams::m_scale_rtts_factor);
    }   // render
};   // BloomShader

// ============================================================================
class BloomBlendShader : public TextureShader<BloomBlendShader, 4>
{
private:
    video::ITexture* m_lens_dust_tex;
public:
    BloomBlendShader()
    {
		m_lens_dust_tex =
            irr_driver->getTexture(FileManager::TEXTURE, "gfx_lensDust_a.png");

        loadProgram(OBJECT, GL_VERTEX_SHADER, "screenquad.vert",
                            GL_FRAGMENT_SHADER, "bloomblend.frag");
        assignUniforms();
        assignSamplerNames(0, "tex_128", ST_BILINEAR_FILTERED,
                           1, "tex_256", ST_BILINEAR_FILTERED,
                           2, "tex_512", ST_BILINEAR_FILTERED,
						   3, "tex_dust", ST_BILINEAR_FILTERED);
    }   // BloomBlendShader
    // ------------------------------------------------------------------------
    void render(GLuint render_target_bloom_128,
                GLuint render_target_bloom_256,
                GLuint render_target_bloom_512)
    {
        setTextureUnits(render_target_bloom_128,
                        render_target_bloom_256,
                        render_target_bloom_512,
						m_lens_dust_tex->getTextureHandler());
        drawFullScreenEffect();
    }   // render
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

        assignSamplerNames(0, "tex_128", ST_BILINEAR_FILTERED,
                           1, "tex_256", ST_BILINEAR_FILTERED,
                           2, "tex_512", ST_BILINEAR_FILTERED);
    }   // LensBlendShader
    // ------------------------------------------------------------------------
    void render(GLuint render_target_lens_128,
                GLuint render_target_lens_256,
                GLuint render_target_lens_512)
    {
        setTextureUnits(render_target_lens_128,
                        render_target_lens_256,
                        render_target_lens_512);
        drawFullScreenEffect();

    }   // render
};   // LensBlendShader

// ============================================================================
class ToneMapShader : public TextureShader<ToneMapShader, 1, float>
{
public:

    ToneMapShader()
    {
        loadProgram(OBJECT, GL_VERTEX_SHADER, "screenquad.vert",
                            GL_FRAGMENT_SHADER, "tonemap.frag");
        assignUniforms("vignette_weight");
        assignSamplerNames(0, "text", ST_NEAREST_FILTERED);
    }   // ToneMapShader
    // ----------------------------------------------------------------------------
    void render(const FrameBuffer &fbo, GLuint rtt, float vignette_weight)
    {
        fbo.bind();
        glClear(GL_COLOR_BUFFER_BIT);
        setTextureUnits(rtt);
        drawFullScreenEffect(vignette_weight);
    }   // render
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
        assignSamplerNames(0, "tex", ST_BILINEAR_FILTERED,
                           1, "dtex", ST_NEAREST_FILTERED);
    }   // DepthOfFieldShader
    // ------------------------------------------------------------------------
    void render(const FrameBuffer &framebuffer, GLuint color_texture, GLuint depth_stencil_texture)
    {
        framebuffer.bind();
        setTextureUnits(color_texture, depth_stencil_texture);
        drawFullScreenEffect();

    }   // render
};   // DepthOfFieldShader

// ============================================================================
class PassThroughShader : public TextureShader<PassThroughShader, 1, int, int>
{
public:
    PassThroughShader()
    {
        loadProgram(OBJECT, GL_VERTEX_SHADER, "screenquad.vert",
                            GL_FRAGMENT_SHADER, "passthrough.frag");
        assignUniforms("width", "height");
        assignSamplerNames(0, "tex", ST_BILINEAR_FILTERED);
    }   // PassThroughShader
    // ------------------------------------------------------------------------
    void render(GLuint tex, unsigned width, unsigned height)
    {
        PassThroughShader::getInstance()->setTextureUnits(tex);
        drawFullScreenEffect(width, height);
    }   // render

};   // PassThroughShader

// ============================================================================
class LayerPassThroughShader : public Shader<LayerPassThroughShader, int>
{
private:
    GLuint m_tu_texture;
    GLuint m_vao;

public:
    LayerPassThroughShader()
    {
        loadProgram(OBJECT, GL_VERTEX_SHADER, "screenquad.vert",
                            GL_FRAGMENT_SHADER, "layertexturequad.frag");
        m_tu_texture = 0;
        assignUniforms("layer");
        assignTextureUnit(m_tu_texture, "tex");
        m_vao = createVAO();
    }   // LayerPassThroughShader
    // ------------------------------------------------------------------------
    void bindVertexArray()
    {
        glBindVertexArray(m_vao);
    }   // bindVertexArray
    // ------------------------------------------------------------------------
    void activateTexture()
    {
        glActiveTexture(GL_TEXTURE0 + m_tu_texture);
    }   // activateTexture
};   // LayerPassThroughShader

// ============================================================================
class LinearizeDepthShader : public TextureShader<LinearizeDepthShader, 1,
                                                  float, float>
{
public:
    LinearizeDepthShader()
    {
        loadProgram(OBJECT, GL_VERTEX_SHADER, "screenquad.vert",
                            GL_FRAGMENT_SHADER, "linearizedepth.frag");
        assignUniforms("zn", "zf");
        assignSamplerNames(0, "texture", ST_BILINEAR_FILTERED);
    }   // LinearizeDepthShader
    // ------------------------------------------------------------------------
    void render(GLuint depth_stencil_texture)
    {
        setTextureUnits(depth_stencil_texture);
        scene::ICameraSceneNode *c = irr_driver->getSceneManager()->getActiveCamera();
        drawFullScreenEffect(c->getNearValue(), c->getFarValue()  );

    }   // render
};   // LinearizeDepthShader

// ============================================================================
class GlowShader : public TextureShader < GlowShader, 1 >
{
private:
    GLuint m_vao;
public:

    GlowShader()
    {
        loadProgram(OBJECT, GL_VERTEX_SHADER, "screenquad.vert",
                            GL_FRAGMENT_SHADER, "glow.frag");
        assignUniforms();
        assignSamplerNames(0, "tex", ST_BILINEAR_FILTERED);
        m_vao = createVAO();
    }   // GlowShader
    // ------------------------------------------------------------------------
    void render(unsigned tex)
    {
        use();
        glBindVertexArray(m_vao);
        setTextureUnits(tex);
        setUniforms();
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    }   // render
};   // GlowShader

// ============================================================================
class SSAOShader : public TextureShader<SSAOShader, 1, float, float, float>
{
public:
    SSAOShader()
    {
        loadProgram(OBJECT, GL_VERTEX_SHADER, "screenquad.vert",
                            GL_FRAGMENT_SHADER, "ssao.frag");

        assignUniforms("radius", "k", "sigma");
        assignSamplerNames(0, "dtex", ST_SEMI_TRILINEAR);
    }   // SSAOShader
    // ------------------------------------------------------------------------
    void render(GLuint render_target_linear_depth)
    {
        setTextureUnits(render_target_linear_depth);
        glGenerateMipmap(GL_TEXTURE_2D);

        drawFullScreenEffect(irr_driver->getSSAORadius(),
                             irr_driver->getSSAOK(),
                             irr_driver->getSSAOSigma());

    }   // render
};   // SSAOShader

// ============================================================================
class MotionBlurShader : public TextureShader<MotionBlurShader, 2,
                                              core::matrix4, core::vector2df,
                                              float, float>
{
public:
    MotionBlurShader()
    {
        loadProgram(OBJECT, GL_VERTEX_SHADER, "screenquad.vert",
                            GL_FRAGMENT_SHADER, "motion_blur.frag");
        assignUniforms("previous_viewproj", "center", "boost_amount",
                        "mask_radius");
        assignSamplerNames(0, "color_buffer", ST_BILINEAR_CLAMPED_FILTERED,
                           1, "dtex", ST_NEAREST_FILTERED);
    }    // MotionBlurShader
    // ------------------------------------------------------------------------
    void render(const FrameBuffer &fb, float boost_time, GLuint depth_stencil_texture)
    {
        setTextureUnits(fb.getRTT()[0],  depth_stencil_texture);
        Camera *cam = Camera::getActiveCamera();
        // Todo : use a previousPVMatrix per cam, not global
        drawFullScreenEffect(cam->getPreviousPVMatrix(),
                             core::vector2df(0.5, 0.5),
                             boost_time, // Todo : should be framerate dependent=
                             0.15f);
    }   // render
};   // MotionBlurShader

// ============================================================================
class GodFadeShader : public TextureShader<GodFadeShader, 1, video::SColorf>
{
public:
    GodFadeShader()
    {
        loadProgram(OBJECT, GL_VERTEX_SHADER, "screenquad.vert",
                            GL_FRAGMENT_SHADER, "godfade.frag");
        assignUniforms("col");
        assignSamplerNames(0, "tex", ST_BILINEAR_FILTERED);
    }   // GodFadeShader
    // ----------------------------------------------------------------------------
    void render(GLuint tex, const SColor &col)
    {
        setTextureUnits(tex);
        drawFullScreenEffect(col);
    }   // render
};   // GodFadeShader

// ============================================================================
class GodRayShader : public TextureShader<GodRayShader, 1, core::vector2df>
{
public:
    GodRayShader()
    {
        loadProgram(OBJECT, GL_VERTEX_SHADER, "screenquad.vert",
                            GL_FRAGMENT_SHADER, "godray.frag");

        assignUniforms("sunpos");
        assignSamplerNames(0, "tex", ST_BILINEAR_FILTERED);
    }   // GodRayShader
    // ----------------------------------------------------------------------------
    void render(GLuint tex, const core::vector2df &sunpos)
    {
        setTextureUnits(tex);
        drawFullScreenEffect(sunpos);
    }   // render
};   // GodRayShader

// ============================================================================
class MLAAColorEdgeDetectionSHader
    : public TextureShader<MLAAColorEdgeDetectionSHader, 1, core::vector2df>
{
public:
    MLAAColorEdgeDetectionSHader()
    {
        loadProgram(OBJECT, GL_VERTEX_SHADER, "screenquad.vert",
                            GL_FRAGMENT_SHADER, "mlaa_color1.frag");
        assignUniforms("PIXEL_SIZE");
        assignSamplerNames(0, "colorMapG", ST_NEAREST_FILTERED);
    }   // MLAAColorEdgeDetectionSHader
    // ------------------------------------------------------------------------
    void render(const core::vector2df &pixel_size, GLuint rtt_mlaa_colors)
    {
        use();
        setTextureUnits(rtt_mlaa_colors);
        drawFullScreenEffect(pixel_size);
    }   // render
};   // MLAAColorEdgeDetectionSHader

// ============================================================================
class MLAABlendWeightSHader : public TextureShader<MLAABlendWeightSHader,
                                                   2, core::vector2df>
{
public:
    MLAABlendWeightSHader()
    {
        loadProgram(OBJECT, GL_VERTEX_SHADER, "screenquad.vert",
                            GL_FRAGMENT_SHADER, "mlaa_blend2.frag");
        assignUniforms("PIXEL_SIZE");

        assignSamplerNames(0, "edgesMap", ST_BILINEAR_FILTERED,
                           1, "areaMap", ST_NEAREST_FILTERED);
    }   // MLAABlendWeightSHader
    // ------------------------------------------------------------------------
    void render(video::ITexture *area_map,
                const core::vector2df &pixel_size,
                GLuint rtt_mlaa_tmp)
    {
        use();
        setTextureUnits(rtt_mlaa_tmp, area_map->getTextureHandler());
        drawFullScreenEffect(pixel_size);

    }   // render
};   // MLAABlendWeightSHader

// ============================================================================
class MLAAGatherSHader : public TextureShader<MLAAGatherSHader, 2,
                                              core::vector2df>
{
public:
    MLAAGatherSHader()
    {
        loadProgram(OBJECT, GL_VERTEX_SHADER, "screenquad.vert",
                            GL_FRAGMENT_SHADER, "mlaa_neigh3.frag");
        assignUniforms("PIXEL_SIZE");
        assignSamplerNames(0, "blendMap", ST_NEAREST_FILTERED,
                           1, "colorMap", ST_NEAREST_FILTERED);
    }   // MLAAGatherSHader
    // ------------------------------------------------------------------------
    void render(const core::vector2df &pixel_size,
                GLuint rtt_mlaa_blend,
                GLuint rtt_mlaa_tmp)
    {
        use();
        setTextureUnits(rtt_mlaa_blend,
                        rtt_mlaa_tmp);
        drawFullScreenEffect(pixel_size);

    }   // render
};   // MLAAGatherSHader

// ============================================================================
class LightningShader : public TextureShader<LightningShader, 1, 
                                             core::vector3df>
{
public:
    LightningShader()
    {
        loadProgram(OBJECT, GL_VERTEX_SHADER, "screenquad.vert",
                            GL_FRAGMENT_SHADER, "lightning.frag");
        assignUniforms("intensity");
    }   // LightningShader
    // ------------------------------------------------------------------------
    void render(core::vector3df intensity)
    {
        drawFullScreenEffect(intensity);
    }   // render
};   // LightningShader

// ============================================================================

PostProcessing::PostProcessing()
{
    // Load the MLAA area map
    io::IReadFile *areamap = irr_driver->getDevice()->getFileSystem()->
                         createMemoryReadFile((void *) AreaMap33, sizeof(AreaMap33),
                         "AreaMap33", false);
    video::IImage* img = irr_driver->getVideoDriver()->createImageFromFile(areamap);
    m_areamap = GE::createTexture(img, "AreaMap33");
    if (m_areamap->getTextureHandler() == 0)
    {
        Log::fatal("postprocessing", "Failed to load the areamap");
        return;
    }
    STKTexManager::getInstance()->addTexture(m_areamap);
    areamap->drop();

    // For preloading shaders
    MotionBlurShader::getInstance();
    LightningShader::getInstance();
}   // PostProcessing

// ----------------------------------------------------------------------------
/** Initialises post processing at the (re-)start of a race. This sets up
 *  the vertices, normals and texture coordinates for each
 */
void PostProcessing::reset()
{
    m_boost_time.resize(Camera::getNumCameras());
    for (unsigned int i = 0; i < Camera::getNumCameras(); i++)
    {
        m_boost_time[i] = 0.0f;
    }  // for i <number of cameras
}   // reset

// ----------------------------------------------------------------------------
/** Set the boost amount according to the speed of the camera */
void PostProcessing::giveBoost(unsigned int camera_index)
{
    if (CVS->isGLSL())
    {
        m_boost_time.at(camera_index) = 0.75f;
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
    for (unsigned int i=0; i<m_boost_time.size(); i++)
    {
        if (m_boost_time[i] > 0.0f)
        {
            m_boost_time[i] -= dt;
            if (m_boost_time[i] < 0.0f) m_boost_time[i] = 0.0f;
        }
    }
}   // update

// ----------------------------------------------------------------------------
void PostProcessing::renderBloom(GLuint in)
{
    BloomShader::getInstance()->render(in);
}   // renderBloom

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
void PostProcessing::renderGaussian3Blur(const FrameBuffer &in_fbo, 
                                         const FrameBuffer &auxiliary) const
{
    assert(in_fbo.getWidth() == auxiliary.getWidth() &&
           in_fbo.getHeight() == auxiliary.getHeight());
    float inv_width  = 1.0f / in_fbo.getWidth();
    float inv_height = 1.0f / in_fbo.getHeight();
    auxiliary.bind();
    glClear(GL_COLOR_BUFFER_BIT);
    Gaussian3VBlurShader::getInstance()->render(in_fbo, inv_width,
                                                inv_height);
    in_fbo.bind();
    glClear(GL_COLOR_BUFFER_BIT);
    Gaussian3HBlurShader::getInstance()->render(auxiliary, inv_width,
                                                inv_height);
}   // renderGaussian3Blur

// ----------------------------------------------------------------------------
void PostProcessing::renderGaussian6Blur(const FrameBuffer &in_fbo,
                                         const FrameBuffer &auxiliary, float sigma_v,
                                         float sigma_h) const
{
    assert(in_fbo.getWidth() == auxiliary.getWidth() &&
           in_fbo.getHeight() == auxiliary.getHeight());
    float inv_width = 1.0f / in_fbo.getWidth();
    float inv_height = 1.0f / in_fbo.getHeight();

    if (!CVS->supportsComputeShadersFiltering())
    {
        auxiliary.bind();
        glClear(GL_COLOR_BUFFER_BIT);
        Gaussian6VBlurShader::getInstance()
            ->render(in_fbo.getRTT()[0], in_fbo.getWidth(), in_fbo.getHeight(),
                     sigma_v);

        in_fbo.bind();
        glClear(GL_COLOR_BUFFER_BIT);
        Gaussian6HBlurShader::getInstance()->setTextureUnits(auxiliary.getRTT()[0]);
        Gaussian6HBlurShader::getInstance()->render(auxiliary, in_fbo.getWidth(),
                                                   in_fbo.getHeight(), sigma_h);
    }
#if !defined(USE_GLES2)
    else
    {
        const std::vector<float> &weightsV = getGaussianWeight(sigma_v, 7);
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

        const std::vector<float> &weightsH = getGaussianWeight(sigma_h, 7);
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
#endif

}   // renderGaussian6Blur

// ----------------------------------------------------------------------------
void PostProcessing::renderHorizontalBlur(const FrameBuffer &in_fbo,
                                          const FrameBuffer &auxiliary) const
{
    assert(in_fbo.getWidth() == auxiliary.getWidth() &&
           in_fbo.getHeight() == auxiliary.getHeight());

    auxiliary.bind();
    glClear(GL_COLOR_BUFFER_BIT);
    Gaussian6HBlurShader::getInstance()->render(in_fbo, in_fbo.getWidth(),
                                                in_fbo.getHeight(), 2.0f );
    in_fbo.bind();
    glClear(GL_COLOR_BUFFER_BIT);
    Gaussian6HBlurShader::getInstance()->render(auxiliary, in_fbo.getWidth(),
                                                in_fbo.getHeight(), 2.0f);
}   // renderHorizontalBlur


// ----------------------------------------------------------------------------
void PostProcessing::renderGaussian17TapBlur(const FrameBuffer &in_fbo,
                                             const FrameBuffer &auxiliary,
                                             const FrameBuffer &linear_depth) const
{
    assert(in_fbo.getWidth() == auxiliary.getWidth() &&
           in_fbo.getHeight() == auxiliary.getHeight());
           
#if !defined(USE_GLES2)
    if (CVS->supportsComputeShadersFiltering())
        glMemoryBarrier(GL_FRAMEBUFFER_BARRIER_BIT);
#endif

    {
        if (!CVS->supportsComputeShadersFiltering())
        {
            auxiliary.bind();
            Gaussian17TapHShader::getInstance()->render(in_fbo,
                                                        linear_depth,
                                                        in_fbo.getWidth(),
                                                        in_fbo.getHeight());
        }
        else
        {
            ComputeGaussian17TapHShader::getInstance()->render(in_fbo,
                                                               auxiliary,
                                                               linear_depth,
                                                               in_fbo.getWidth(),
                                                               in_fbo.getHeight());
        }
    }
    
#if !defined(USE_GLES2)
    if (CVS->supportsComputeShadersFiltering())
        glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
#endif
        
    {
        if (!CVS->supportsComputeShadersFiltering())
        {
            in_fbo.bind();
            Gaussian17TapVShader::getInstance()->render(auxiliary,
                                                        linear_depth,
                                                        in_fbo.getWidth(),
                                                        in_fbo.getHeight());
        }
        else
        {
            ComputeGaussian17TapVShader::getInstance()->render(auxiliary,
                                                               in_fbo, 
                                                               linear_depth,
                                                               in_fbo.getWidth(),
                                                               in_fbo.getHeight());
        }
    }
    
#if !defined(USE_GLES2)
    if (CVS->supportsComputeShadersFiltering())
        glMemoryBarrier(GL_TEXTURE_FETCH_BARRIER_BIT);
#endif
}   // renderGaussian17TapBlur

// ----------------------------------------------------------------------------
void PostProcessing::renderPassThrough(GLuint tex, unsigned width,
                                       unsigned height) const
{
    PassThroughShader::getInstance()->render(tex, width, height);
}   // renderPassThrough

// ----------------------------------------------------------------------------
void PostProcessing::renderTextureLayer(unsigned tex, unsigned layer) const
{
    LayerPassThroughShader::getInstance()->use();
    LayerPassThroughShader::getInstance()->bindVertexArray();
    LayerPassThroughShader::getInstance()->activateTexture();
    glBindTexture(GL_TEXTURE_2D_ARRAY, tex);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    LayerPassThroughShader::getInstance()->setUniforms(layer);

    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
}   // renderTextureLayer

// ----------------------------------------------------------------------------
void PostProcessing::renderGlow(const FrameBuffer& quarter_framebuffer) const
{
    glEnable(GL_BLEND);
    glBlendEquation(GL_FUNC_ADD);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glStencilFunc(GL_EQUAL, 0, ~0);
    glEnable(GL_STENCIL_TEST);
    GlowShader::getInstance()->render(quarter_framebuffer.getRTT()[0]);
    glDisable(GL_STENCIL_TEST);
    glDisable(GL_BLEND);
}   // renderGlow

// ----------------------------------------------------------------------------
void PostProcessing::renderSSAO(const FrameBuffer& linear_depth_framebuffer,
                                const FrameBuffer& ssao_framebuffer,
                                GLuint depth_stencil_texture)
{
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_BLEND);

    // Generate linear depth buffer
    linear_depth_framebuffer.bind();
    LinearizeDepthShader::getInstance()->render(depth_stencil_texture);
    ssao_framebuffer.bind();
    SSAOShader::getInstance()->render(linear_depth_framebuffer.getRTT()[0]);
}   // renderSSAO

// ----------------------------------------------------------------------------
void PostProcessing::renderMotionBlur(const FrameBuffer &in_fbo,
                                      FrameBuffer &out_fbo,
                                      GLuint depth_stencil_texture)
{
    Camera *cam = Camera::getActiveCamera();
    out_fbo.bind();
    glClear(GL_COLOR_BUFFER_BIT);

    float boost_time = m_boost_time.at(cam->getIndex()) * 10.0f;
    MotionBlurShader::getInstance()->render(in_fbo, boost_time, depth_stencil_texture);
}   // renderMotionBlur


// ----------------------------------------------------------------------------
void PostProcessing::renderDoF(const FrameBuffer &framebuffer, GLuint color_texture, GLuint depth_stencil_texture)
{
    DepthOfFieldShader::getInstance()->render(framebuffer, color_texture, depth_stencil_texture);
}   // renderDoF

// ----------------------------------------------------------------------------
void PostProcessing::renderGodRays(scene::ICameraSceneNode * const camnode,
                                   const FrameBuffer &in_fbo,
                                   const FrameBuffer &tmp_fbo,
                                   const FrameBuffer &quarter1_fbo,
                                   const FrameBuffer &quarter2_fbo)
{
    Track* track = Track::getCurrentTrack();

    glEnable(GL_DEPTH_TEST);
    // Grab the sky
    tmp_fbo.bind();
    glClear(GL_COLOR_BUFFER_BIT);

    // Set the sun's color
    const SColor col = track->getGodRaysColor();

    // The sun interposer
    SP::SPDynamicDrawCall* sun_ = irr_driver->getSunInterposer();
    // This will only do thing when you update the sun position
    sun_->uploadInstanceData();
    SP::SPShader* glow_shader = SP::getGlowShader();
    glow_shader->use();
    SP::SPUniformAssigner* glow_color_assigner = glow_shader
        ->getUniformAssigner("col");
    assert(glow_color_assigner != NULL);
    video::SColorf cf(track->getGodRaysColor());
    glow_color_assigner->setValue(core::vector3df(cf.r, cf.g, cf.b));
    sun_->draw();
    glow_shader->unuse();
    glDisable(GL_DEPTH_TEST);

    // Fade to quarter
    quarter1_fbo.bind();
    glClear(GL_COLOR_BUFFER_BIT);
    GodFadeShader::getInstance()->render(tmp_fbo.getRTT()[0], col);

    // Blur
    renderGaussian3Blur(quarter1_fbo, quarter2_fbo);

    // Calculate the sun's position in texcoords
    const core::vector3df pos = track->getGodRaysPosition();
    float ndc[4];
    core::matrix4 trans = camnode->getProjectionMatrix();
    trans *= camnode->getViewMatrix();

    trans.transformVect(ndc, pos);
    const float sunx = ((ndc[0] / ndc[3]) * 0.5f + 0.5f);
    const float suny = ((ndc[1] / ndc[3]) * 0.5f + 0.5f);

    // Rays please
    quarter2_fbo.bind();
    glClear(GL_COLOR_BUFFER_BIT);
    GodRayShader::getInstance()
        ->render(quarter1_fbo.getRTT()[0], core::vector2df(sunx, suny));

    // Blur
    renderGaussian3Blur(quarter2_fbo, quarter1_fbo);

    // Blend
    glEnable(GL_BLEND);
    glBlendColor(0., 0., 0., track->getGodRaysOpacity());
    glBlendFunc(GL_CONSTANT_ALPHA, GL_ONE);
    glBlendEquation(GL_FUNC_ADD);

    in_fbo.bind();
    renderPassThrough(quarter2_fbo.getRTT()[0], in_fbo.getWidth(), in_fbo.getHeight());
    glDisable(GL_BLEND);

}


// ----------------------------------------------------------------------------
void PostProcessing::applyMLAA(const FrameBuffer& mlaa_tmp_framebuffer,
                               const FrameBuffer& mlaa_blend_framebuffer,
                               const FrameBuffer& mlaa_colors_framebuffer)
{
    const core::vector2df &PIXEL_SIZE =
                     core::vector2df(1.0f / UserConfigParams::m_width,
                                     1.0f / UserConfigParams::m_height);

    mlaa_tmp_framebuffer.bind();
    glClearColor(0.0, 0.0, 0.0, 1.0);
    glClear(GL_COLOR_BUFFER_BIT);

    // Pass 1: color edge detection
    MLAAColorEdgeDetectionSHader::getInstance()->render(PIXEL_SIZE, mlaa_colors_framebuffer.getRTT()[0]);

    // Pass 2: blend weights
    mlaa_blend_framebuffer.bind();
    glClear(GL_COLOR_BUFFER_BIT);

    MLAABlendWeightSHader::getInstance()->render(m_areamap, PIXEL_SIZE, mlaa_tmp_framebuffer.getRTT()[0]);

    // Blit in to tmp1
    FrameBuffer::blit(mlaa_colors_framebuffer,
                      mlaa_tmp_framebuffer);

    // Pass 3: gather
    mlaa_colors_framebuffer.bind();
    MLAAGatherSHader::getInstance()
        ->render(PIXEL_SIZE, mlaa_blend_framebuffer.getRTT()[0], mlaa_tmp_framebuffer.getRTT()[0]);

}   // applyMLAA

// ----------------------------------------------------------------------------
void PostProcessing::renderLightning(core::vector3df intensity)
{
    glEnable(GL_BLEND);
    glBlendFunc(GL_ONE, GL_ONE);
    glBlendEquation(GL_FUNC_ADD);

    LightningShader::getInstance()->render(intensity);

    glDisable(GL_BLEND);
}

// ----------------------------------------------------------------------------
/** Render the post-processed scene */
FrameBuffer *PostProcessing::render(scene::ICameraSceneNode * const camnode,
                                    bool isRace,
                                    RTT *rtts)
{
    FrameBuffer *in_fbo = &rtts->getFBO(FBO_COLORS);
    FrameBuffer *out_fbo = &rtts->getFBO(FBO_TMP1_WITH_DS);
    // Each effect uses these as named, and sets them up for the next effect.
    // This allows chaining effects where some may be disabled.

    // As the original color shouldn't be touched, the first effect
    // can't be disabled.
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_BLEND);

    Physics *physics = Physics::get();

    if (isRace && UserConfigParams::m_dof && (physics == NULL || !physics->isDebug()))
    {
        PROFILER_PUSH_CPU_MARKER("- DoF", 0xFF, 0x00, 0x00);
        ScopedGPUTimer Timer(irr_driver->getGPUTimer(Q_DOF));
        renderDoF(*out_fbo, in_fbo->getRTT()[0], rtts->getDepthStencilTexture());
        std::swap(in_fbo, out_fbo);
        PROFILER_POP_CPU_MARKER();
    }

    bool hasgodrays = false;
    const Track * const track = Track::getCurrentTrack();
    if (track)
        hasgodrays = track->hasGodRays();

    if (isRace && UserConfigParams::m_light_shaft && hasgodrays)
    {
        PROFILER_PUSH_CPU_MARKER("- Godrays", 0xFF, 0x00, 0x00);
        ScopedGPUTimer Timer(irr_driver->getGPUTimer(Q_GODRAYS));
        renderGodRays(camnode, *in_fbo, rtts->getFBO(FBO_RGBA_1),
            rtts->getFBO(FBO_QUARTER1), rtts->getFBO(FBO_QUARTER2));
        PROFILER_POP_CPU_MARKER();
    }

    // Simulate camera defects from there
    {
        PROFILER_PUSH_CPU_MARKER("- Bloom", 0xFF, 0x00, 0x00);
        ScopedGPUTimer Timer(irr_driver->getGPUTimer(Q_BLOOM));

        if (isRace && UserConfigParams::m_bloom && (physics == NULL || !physics->isDebug()))
        {
            glClear(GL_STENCIL_BUFFER_BIT);
            glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_FALSE);

            FrameBuffer::blit(*in_fbo, rtts->getFBO(FBO_BLOOM_1024),
                              GL_COLOR_BUFFER_BIT, GL_LINEAR);

            rtts->getFBO(FBO_BLOOM_512).bind();
            renderBloom(rtts->getRenderTarget(RTT_BLOOM_1024));



            // Downsample
            FrameBuffer::blit(rtts->getFBO(FBO_BLOOM_512),
                              rtts->getFBO(FBO_BLOOM_256), 
                              GL_COLOR_BUFFER_BIT, GL_LINEAR);
            FrameBuffer::blit(rtts->getFBO(FBO_BLOOM_256),
                              rtts->getFBO(FBO_BLOOM_128),
                              GL_COLOR_BUFFER_BIT, GL_LINEAR);

			// Copy for lens flare
			FrameBuffer::blit(rtts->getFBO(FBO_BLOOM_512),
                              rtts->getFBO(FBO_LENS_512), 
                              GL_COLOR_BUFFER_BIT, GL_LINEAR);
			FrameBuffer::blit(rtts->getFBO(FBO_BLOOM_256),
                              rtts->getFBO(FBO_LENS_256),
                              GL_COLOR_BUFFER_BIT, GL_LINEAR);
			FrameBuffer::blit(rtts->getFBO(FBO_BLOOM_128),
                              rtts->getFBO(FBO_LENS_128),
                              GL_COLOR_BUFFER_BIT, GL_LINEAR);


            // Blur
            renderGaussian6Blur(rtts->getFBO(FBO_BLOOM_512),
                                rtts->getFBO(FBO_TMP_512), 1., 1.);
            renderGaussian6Blur(rtts->getFBO(FBO_BLOOM_256),
                                rtts->getFBO(FBO_TMP_256), 1., 1.);
            renderGaussian6Blur(rtts->getFBO(FBO_BLOOM_128),
                                rtts->getFBO(FBO_TMP_128), 1., 1.);

            renderHorizontalBlur(rtts->getFBO(FBO_LENS_512), 
                                 rtts->getFBO(FBO_TMP_512));
            renderHorizontalBlur(rtts->getFBO(FBO_LENS_256),
                                 rtts->getFBO(FBO_TMP_256));
            renderHorizontalBlur(rtts->getFBO(FBO_LENS_128), 
                                 rtts->getFBO(FBO_TMP_128));

            // Additively blend on top of tmp1
            in_fbo->bind();
            glEnable(GL_BLEND);
            glBlendFunc(GL_ONE, GL_ONE);
            glBlendEquation(GL_FUNC_ADD);

            BloomBlendShader::getInstance()
                ->render(rtts->getRenderTarget(RTT_BLOOM_128),
                         rtts->getRenderTarget(RTT_BLOOM_256),
                         rtts->getRenderTarget(RTT_BLOOM_512));
            LensBlendShader::getInstance()
                ->render(rtts->getRenderTarget(RTT_LENS_128),
                         rtts->getRenderTarget(RTT_LENS_256),
                         rtts->getRenderTarget(RTT_LENS_512));

            glDisable(GL_BLEND);
            glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
        } // end if bloom
        PROFILER_POP_CPU_MARKER();
    }

    {
        PROFILER_PUSH_CPU_MARKER("- Tonemap", 0xFF, 0x00, 0x00);
        ScopedGPUTimer Timer(irr_driver->getGPUTimer(Q_TONEMAP));
		// only enable vignette during race

        out_fbo = &rtts->getFBO(FBO_RGBA_1);
        ToneMapShader::getInstance()->render(*out_fbo, in_fbo->getRTT()[0],
                                             isRace ? 1.0f : 0.0f);
        in_fbo = &rtts->getFBO(FBO_RGBA_2);
        PROFILER_POP_CPU_MARKER();
    }

    {
        PROFILER_PUSH_CPU_MARKER("- Motion blur", 0xFF, 0x00, 0x00);
        ScopedGPUTimer Timer(irr_driver->getGPUTimer(Q_MOTIONBLUR));

        if (isRace && UserConfigParams::m_motionblur && World::getWorld() &&
            m_boost_time.at(Camera::getActiveCamera()->getIndex()) > 0.0f) // motion blur
        {
            in_fbo = &rtts->getFBO(FBO_RGBA_1);
            out_fbo = &rtts->getFBO(FBO_RGBA_2);
            renderMotionBlur(*in_fbo, *out_fbo, irr_driver->getDepthStencilTexture());
        }
        PROFILER_POP_CPU_MARKER();
    }

    // Handle lightning rendering
    {
        PROFILER_PUSH_CPU_MARKER("- Lightning", 0xFF, 0x00, 0x00);
        ScopedGPUTimer Timer(irr_driver->getGPUTimer(Q_LIGHTNING));
        Weather* weather = Weather::getInstance();
        if (weather && weather->shouldLightning())
        {
            renderLightning(weather->getIntensity());
        }
        PROFILER_POP_CPU_MARKER();
    }

    if (UserConfigParams::m_mlaa) // MLAA. Must be the last pp filter.
    {
        PROFILER_PUSH_CPU_MARKER("- MLAA", 0xFF, 0x00, 0x00);
        ScopedGPUTimer Timer(irr_driver->getGPUTimer(Q_MLAA));
        applyMLAA(*in_fbo,
                  rtts->getFBO(FBO_RGBA_3),
                  *out_fbo);
        PROFILER_POP_CPU_MARKER();
    }

    return out_fbo;
}   // render

#endif   // !SERVER_ONLY
