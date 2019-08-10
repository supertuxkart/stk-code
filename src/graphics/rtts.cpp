//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2013-2015 Lauri Kasanen
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

#include "graphics/rtts.hpp"

#include "config/user_config.hpp"
#include "graphics/central_settings.hpp"
#include "graphics/frame_buffer_layer.hpp"
#include "utils/log.hpp"

#include <dimension2d.h>

using namespace irr;
static GLuint generateRTT3D(GLenum target, unsigned int w, unsigned int h, 
                            unsigned int d, GLint internalFormat, GLint format,
                            GLint type, unsigned mipmaplevel = 1)
{
    GLuint result;
    glGenTextures(1, &result);
    glBindTexture(target, result);
    if (CVS->isARBTextureStorageUsable())
        glTexStorage3D(target, mipmaplevel, internalFormat, w, h, d);
    else
        glTexImage3D(target, 0, internalFormat, w, h, d, 0, format, type, 0);
    return result;
}

static GLuint generateRTT(const core::dimension2du &res, GLint internalFormat, GLint format, GLint type, unsigned mipmaplevel = 1)
{
    GLuint result;
    glGenTextures(1, &result);
    glBindTexture(GL_TEXTURE_2D, result);
    if (CVS->isARBTextureStorageUsable())
        glTexStorage2D(GL_TEXTURE_2D, mipmaplevel, internalFormat, res.Width, res.Height);
    else
        glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, res.Width, res.Height, 0, format, type, 0);
    return result;
}

RTT::RTT(unsigned int width, unsigned int height, float rtt_scale,
         bool use_default_fbo_only)
{
    m_width = (unsigned int)(width * rtt_scale);
    m_height = (unsigned int)(height * rtt_scale);
    m_shadow_fbo = NULL;

    using namespace core;

    dimension2du res(m_width, m_height);
    
    const dimension2du half = res/2;
    const dimension2du quarter = res/4;

    const u16 shadowside = u16(1024 * rtt_scale);
    const dimension2du shadowsize0(shadowside, shadowside);
    const dimension2du shadowsize1(shadowside / 2, shadowside / 2);
    const dimension2du shadowsize2(shadowside / 4, shadowside / 4);
    const dimension2du shadowsize3(shadowside / 8, shadowside / 8);

    unsigned linear_depth_mip_levels = int(ceilf(log2f( float(max_(res.Width, res.Height)) )));

    if (!use_default_fbo_only)
    {
        m_depth_stencil_tex = generateRTT(res, GL_DEPTH24_STENCIL8, GL_DEPTH_STENCIL, GL_UNSIGNED_INT_24_8);
    }

    // All RTTs are currently RGBA16F mostly with stencil. The four tmp RTTs are the same size
    // as the screen, for use in post-processing.

    GLint rgba_internal_format = GL_RGBA16F;
    GLint rgba_format = GL_BGRA;
    GLint red_internal_format = GL_R16F;
    GLint red32_internal_format = GL_R32F;
    GLint red_format = GL_RED;
    GLint rgb_format = GL_BGR;
    GLint diffuse_specular_internal_format = GL_R11F_G11F_B10F;
    GLint type = GL_FLOAT;

    if (!CVS->isEXTColorBufferFloatUsable())
    {
        rgba_internal_format = GL_RGBA8;
        rgba_format = GL_RGBA;
        red_internal_format = GL_R8;
        red32_internal_format = GL_R8;
        red_format = GL_RED;
        rgb_format = GL_RGB;
        diffuse_specular_internal_format = GL_RGBA8;
        type = GL_UNSIGNED_BYTE;
    }

    if (!CVS->isDeferredEnabled())
    {
        // RTT is used in only deferred shading which need hdr framebuffer
        rgba_internal_format = GL_RGBA8;
        type = GL_UNSIGNED_BYTE;
    }

    if (!use_default_fbo_only)
    {
        m_render_target_textures[RTT_COLOR] = generateRTT(res, rgba_internal_format, rgba_format, type);
    }
    if (CVS->isDeferredEnabled())
    {
        m_render_target_textures[RTT_NORMAL_AND_DEPTH] = generateRTT(res, GL_RGBA8, GL_RGBA, GL_UNSIGNED_BYTE);
        m_render_target_textures[RTT_SP_DIFF_COLOR] = generateRTT(res, GL_RGBA8, GL_RGBA, GL_UNSIGNED_BYTE);
        m_render_target_textures[RTT_RGBA_2] = generateRTT(res, GL_RGBA8, GL_RGBA, GL_UNSIGNED_BYTE);
        m_render_target_textures[RTT_DIFFUSE] = generateRTT(res, diffuse_specular_internal_format, rgb_format, type);
        m_render_target_textures[RTT_SPECULAR] = generateRTT(res, diffuse_specular_internal_format, rgb_format, type);
        m_render_target_textures[RTT_TMP1] = generateRTT(res, rgba_internal_format, rgba_format, type);
        m_render_target_textures[RTT_HALF1] = generateRTT(half, rgba_internal_format, rgba_format, type);
        m_render_target_textures[RTT_HALF1_R] = generateRTT(half, red_internal_format, red_format, type);
        m_render_target_textures[RTT_HALF2] = generateRTT(half, rgba_internal_format, rgba_format, type);

        if (UserConfigParams::m_mlaa)
        {
            m_render_target_textures[RTT_RGBA_3] = generateRTT(res, GL_RGBA8, GL_RGBA, GL_UNSIGNED_BYTE);
        }

        if (UserConfigParams::m_ssao)
        {
            m_render_target_textures[RTT_SSAO] = generateRTT(res, red_internal_format, red_format, type);
            m_render_target_textures[RTT_LINEAR_DEPTH] = generateRTT(res, red32_internal_format, red_format, type, linear_depth_mip_levels);
            m_render_target_textures[RTT_HALF2_R] = generateRTT(half, red_internal_format, red_format, type);
        }

        if (UserConfigParams::m_light_shaft || UserConfigParams::m_glow)
        {
            m_render_target_textures[RTT_QUARTER1] = generateRTT(quarter, rgba_internal_format, rgba_format, type);
        }
        if (UserConfigParams::m_light_shaft)
        {
            m_render_target_textures[RTT_QUARTER2] = generateRTT(quarter, rgba_internal_format, rgba_format, type);
        }

        if (UserConfigParams::m_bloom)
        {
            m_render_target_textures[RTT_BLOOM_1024] = generateRTT(shadowsize0, rgba_internal_format, rgb_format, type);
            m_render_target_textures[RTT_BLOOM_512] = generateRTT(shadowsize1, rgba_internal_format, rgb_format, type);
            m_render_target_textures[RTT_TMP_512] = generateRTT(shadowsize1, rgba_internal_format, rgb_format, type);
            m_render_target_textures[RTT_LENS_512] = generateRTT(shadowsize1, rgba_internal_format, rgb_format, type);
            m_render_target_textures[RTT_BLOOM_256] = generateRTT(shadowsize2, rgba_internal_format, rgb_format, type);
            m_render_target_textures[RTT_TMP_256] = generateRTT(shadowsize2, rgba_internal_format, rgb_format, type);
            m_render_target_textures[RTT_LENS_256] = generateRTT(shadowsize2, rgba_internal_format, rgb_format, type);
            m_render_target_textures[RTT_BLOOM_128] = generateRTT(shadowsize3, rgba_internal_format, rgb_format, type);
            m_render_target_textures[RTT_TMP_128] = generateRTT(shadowsize3, rgba_internal_format, rgb_format, type);
            m_render_target_textures[RTT_LENS_128] = generateRTT(shadowsize3, rgba_internal_format, rgb_format, type);
        }
    }

    std::vector<GLuint> somevector;
    if (!use_default_fbo_only)
    {
        somevector.push_back(m_render_target_textures[RTT_COLOR]);
        m_frame_buffers[FBO_COLORS] = new FrameBuffer(somevector, m_depth_stencil_tex, res.Width, res.Height);
    }

    if (CVS->isDeferredEnabled())
    {
        somevector.clear();
        somevector.push_back(m_render_target_textures[RTT_NORMAL_AND_DEPTH]);
        m_frame_buffers[FBO_NORMAL_AND_DEPTHS] = new FrameBuffer(somevector, m_depth_stencil_tex, res.Width, res.Height);

        somevector.clear();
        somevector.push_back(m_render_target_textures[RTT_SP_DIFF_COLOR]);
        somevector.push_back(m_render_target_textures[RTT_NORMAL_AND_DEPTH]);
        m_frame_buffers[FBO_SP] = new FrameBuffer(somevector, m_depth_stencil_tex, res.Width, res.Height);

        somevector.clear();
        somevector.push_back(m_render_target_textures[RTT_SP_DIFF_COLOR]);
        m_frame_buffers[FBO_RGBA_1] = new FrameBuffer(somevector, m_depth_stencil_tex, res.Width, res.Height);

        somevector.clear();
        somevector.push_back(m_render_target_textures[RTT_RGBA_2]);
        m_frame_buffers[FBO_RGBA_2] = new FrameBuffer(somevector, m_depth_stencil_tex, res.Width, res.Height);

        somevector.clear();
        somevector.push_back(m_render_target_textures[RTT_DIFFUSE]);
        somevector.push_back(m_render_target_textures[RTT_SPECULAR]);
        m_frame_buffers[FBO_COMBINED_DIFFUSE_SPECULAR] = new FrameBuffer(somevector, m_depth_stencil_tex, res.Width, res.Height);

        somevector.clear();
        somevector.push_back(m_render_target_textures[RTT_TMP1]);
        m_frame_buffers[FBO_TMP1_WITH_DS] = new FrameBuffer(somevector, m_depth_stencil_tex, res.Width, res.Height);

        somevector.clear();
        somevector.push_back(m_render_target_textures[RTT_HALF1_R]);
        m_frame_buffers[FBO_HALF1_R] = new FrameBuffer(somevector, half.Width, half.Height);

        somevector.clear();
        somevector.push_back(m_render_target_textures[RTT_HALF1]);
        m_frame_buffers[FBO_HALF1] = new FrameBuffer(somevector, half.Width, half.Height);

        somevector.clear();
        somevector.push_back(m_render_target_textures[RTT_HALF2]);
        m_frame_buffers[FBO_HALF2] = new FrameBuffer(somevector, half.Width, half.Height);

        if (m_render_target_textures[RTT_RGBA_3] != 0)
        {
            somevector.clear();
            somevector.push_back(m_render_target_textures[RTT_RGBA_3]);
            m_frame_buffers[FBO_RGBA_3] = new FrameBuffer(somevector, res.Width, res.Height);
        }

        if (m_render_target_textures[RTT_SSAO] != 0 &&
            m_render_target_textures[RTT_LINEAR_DEPTH] != 0 &&
            m_render_target_textures[RTT_HALF2_R] != 0)
        {
            somevector.clear();
            somevector.push_back(m_render_target_textures[RTT_SSAO]);
            m_frame_buffers[FBO_SSAO] = new FrameBuffer(somevector, res.Width, res.Height);

            somevector.clear();
            somevector.push_back(m_render_target_textures[RTT_LINEAR_DEPTH]);
            m_frame_buffers[FBO_LINEAR_DEPTH] = new FrameBuffer(somevector, res.Width, res.Height);

            somevector.clear();
            somevector.push_back(m_render_target_textures[RTT_HALF2_R]);
            m_frame_buffers[FBO_HALF2_R] = new FrameBuffer(somevector, half.Width, half.Height);
        }

        if (m_render_target_textures[RTT_QUARTER1] != 0)
        {
            somevector.clear();
            somevector.push_back(m_render_target_textures[RTT_QUARTER1]);
            m_frame_buffers[FBO_QUARTER1] = new FrameBuffer(somevector, quarter.Width, quarter.Height);
        }
        if (m_render_target_textures[RTT_QUARTER2] != 0)
        {
            somevector.clear();
            somevector.push_back(m_render_target_textures[RTT_QUARTER2]);
            m_frame_buffers[FBO_QUARTER2] = new FrameBuffer(somevector, quarter.Width, quarter.Height);
        }

        if (UserConfigParams::m_bloom)
        {
            somevector.clear();
            somevector.push_back(m_render_target_textures[RTT_BLOOM_1024]);
            m_frame_buffers[FBO_BLOOM_1024] = new FrameBuffer(somevector, shadowsize0.Width, shadowsize0.Height);

            somevector.clear();
            somevector.push_back(m_render_target_textures[RTT_BLOOM_512]);
            m_frame_buffers[FBO_BLOOM_512] = new FrameBuffer(somevector, shadowsize1.Width, shadowsize1.Height);

            somevector.clear();
            somevector.push_back(m_render_target_textures[RTT_TMP_512]);
            m_frame_buffers[FBO_TMP_512] = new FrameBuffer(somevector, shadowsize1.Width, shadowsize1.Height);

            somevector.clear();
            somevector.push_back(m_render_target_textures[RTT_LENS_512]);
            m_frame_buffers[FBO_LENS_512] = new FrameBuffer(somevector, shadowsize1.Width, shadowsize1.Height);

            somevector.clear();
            somevector.push_back(m_render_target_textures[RTT_BLOOM_256]);
            m_frame_buffers[FBO_BLOOM_256] = new FrameBuffer(somevector, shadowsize2.Width, shadowsize2.Height);

            somevector.clear();
            somevector.push_back(m_render_target_textures[RTT_TMP_256]);
            m_frame_buffers[FBO_TMP_256] = new FrameBuffer(somevector, shadowsize2.Width, shadowsize2.Height);

            somevector.clear();
            somevector.push_back(m_render_target_textures[RTT_LENS_256]);
            m_frame_buffers[FBO_LENS_256] = new FrameBuffer(somevector, shadowsize2.Width, shadowsize2.Height);

            somevector.clear();
            somevector.push_back(m_render_target_textures[RTT_BLOOM_128]);
            m_frame_buffers[FBO_BLOOM_128] = new FrameBuffer(somevector, shadowsize3.Width, shadowsize3.Height);

            somevector.clear();
            somevector.push_back(m_render_target_textures[RTT_TMP_128]);
            m_frame_buffers[FBO_TMP_128] = new FrameBuffer(somevector, shadowsize3.Width, shadowsize3.Height);

            somevector.clear();
            somevector.push_back(m_render_target_textures[RTT_LENS_128]);
            m_frame_buffers[FBO_LENS_128] = new FrameBuffer(somevector, shadowsize3.Width, shadowsize3.Height);
        }

        if (CVS->isShadowEnabled())
        {
            m_shadow_depth_tex = generateRTT3D(GL_TEXTURE_2D_ARRAY, UserConfigParams::m_shadows_resolution, UserConfigParams::m_shadows_resolution, 4, GL_DEPTH24_STENCIL8, GL_DEPTH_STENCIL, GL_UNSIGNED_INT_24_8, 1);
            somevector.clear();
            m_shadow_fbo = new FrameBufferLayer(somevector, m_shadow_depth_tex, UserConfigParams::m_shadows_resolution, UserConfigParams::m_shadows_resolution, 4);
        }
        // Clear this FBO to 1s so that if no SSAO is computed we can still use it.
        getFBO(FBO_HALF1_R).bind();
        glClearColor(1., 1., 1., 1.);
        glClear(GL_COLOR_BUFFER_BIT);
        // Raytracer reflexion use previous frame color frame buffer, so we clear it before to avoid
        // artifacts in the begining of race
        getFBO(FBO_COLORS).bind();
        glClearColor(1., 1., 1., 1.);
        glClear(GL_COLOR_BUFFER_BIT);
        getFBO(FBO_HALF1).bind();
        glClearColor(0., 0., 0., 0.);
        glClear(GL_COLOR_BUFFER_BIT);
    }
    glBindFramebuffer(GL_FRAMEBUFFER, irr_driver->getDefaultFramebuffer());
}

RTT::~RTT()
{
    glBindFramebuffer(GL_FRAMEBUFFER, irr_driver->getDefaultFramebuffer());
    glDeleteTextures(RTT_COUNT, m_render_target_textures);
    for (FrameBuffer* fb : m_frame_buffers)
    {
        delete fb;
    }
    glDeleteTextures(1, &m_depth_stencil_tex);
    if (CVS->isShadowEnabled())
    {
        delete m_shadow_fbo;
        glDeleteTextures(1, &m_shadow_depth_tex);
    }
}

#endif   // !SERVER_ONLY
