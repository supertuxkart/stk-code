//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2013 Lauri Kasanen
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

#include "graphics/rtts.hpp"

#include "config/user_config.hpp"
#include "graphics/glwrap.hpp"
#include "graphics/irr_driver.hpp"
#include "utils/log.hpp"

static GLuint generateRTT(const core::dimension2du &res, GLint internalFormat, GLint format, GLint type)
{
    GLuint result;
    glGenTextures(1, &result);
    glBindTexture(GL_TEXTURE_2D, result);
    glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, res.Width, res.Height, 0, format, type, 0);
    return result;
}

static GLuint generateFBO(GLuint ColorAttachement)
{
    GLuint fbo;
    glGenFramebuffers(1, &fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, ColorAttachement, 0);
    GLenum result = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    assert(result == GL_FRAMEBUFFER_COMPLETE_EXT);
    return fbo;
}

static GLuint generateFBO(GLuint ColorAttachement, GLuint DepthAttachement)
{
    GLuint fbo = generateFBO(ColorAttachement);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_TEXTURE_2D, DepthAttachement, 0);
    GLenum result = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    assert(result == GL_FRAMEBUFFER_COMPLETE_EXT);
    return fbo;
}

RTT::RTT()
{
    initGL();
    using namespace video;
    using namespace core;

    IVideoDriver * const drv = irr_driver->getVideoDriver();
    const dimension2du res(UserConfigParams::m_width, UserConfigParams::m_height);
    const dimension2du half = res/2;
    const dimension2du quarter = res/4;
    const dimension2du eighth = res/8;
    const dimension2du sixteenth = res/16;

    const u16 shadowside = 1024;
    const dimension2du shadowsize0(shadowside, shadowside);
    const dimension2du shadowsize1(shadowside / 2, shadowside / 2);
    const dimension2du shadowsize2(shadowside / 4, shadowside / 4);
    const dimension2du shadowsize3(shadowside / 8, shadowside / 8);
    const dimension2du warpvsize(1, 512);
    const dimension2du warphsize(512, 1);

    glGenTextures(1, &DepthStencilTexture);
    glBindTexture(GL_TEXTURE_2D, DepthStencilTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_STENCIL, res.Width, res.Height, 0, GL_DEPTH_STENCIL, GL_UNSIGNED_INT_24_8, 0);

    // All RTTs are currently RGBA16F mostly with stencil. The four tmp RTTs are the same size
    // as the screen, for use in post-processing.

    RenderTargetTextures[RTT_TMP1] = generateRTT(res, GL_RGBA16F, GL_BGRA, GL_FLOAT);
    RenderTargetTextures[RTT_TMP2] = generateRTT(res, GL_RGBA16F, GL_BGRA, GL_FLOAT);
    RenderTargetTextures[RTT_TMP3] = generateRTT(res, GL_RGBA16F, GL_BGRA, GL_FLOAT);
    RenderTargetTextures[RTT_TMP4] = generateRTT(res, GL_R16F, GL_RED, GL_FLOAT);
    RenderTargetTextures[RTT_NORMAL_AND_DEPTH] = generateRTT(res, GL_RGBA16F, GL_RGB, GL_FLOAT);
    RenderTargetTextures[RTT_COLOR] = generateRTT(res, GL_RGBA16F, GL_BGRA, GL_FLOAT);
    RenderTargetTextures[RTT_MLAA_COLORS] = generateRTT(res, GL_SRGB, GL_BGR, GL_UNSIGNED_BYTE);
    RenderTargetTextures[RTT_LOG_LUMINANCE] = generateRTT(res, GL_R16F, GL_RED, GL_FLOAT);
    RenderTargetTextures[RTT_SSAO] = generateRTT(half, GL_R16F, GL_RED, GL_FLOAT);
    RenderTargetTextures[RTT_DISPLACE] = generateRTT(res, GL_RGBA16F, GL_BGRA, GL_FLOAT);

    RenderTargetTextures[RTT_HALF1] = generateRTT(half, GL_RGBA16F, GL_BGRA, GL_FLOAT);
    RenderTargetTextures[RTT_QUARTER1] = generateRTT(quarter, GL_RGBA16F, GL_BGRA, GL_FLOAT);
    RenderTargetTextures[RTT_EIGHTH1] = generateRTT(eighth, GL_RGBA16F, GL_BGRA, GL_FLOAT);

    RenderTargetTextures[RTT_HALF2] = generateRTT(half, GL_RGBA16F, GL_BGRA, GL_FLOAT);
    RenderTargetTextures[RTT_QUARTER2] = generateRTT(quarter, GL_RGBA16F, GL_BGRA, GL_FLOAT);
    RenderTargetTextures[RTT_EIGHTH2] = generateRTT(eighth, GL_RGBA16F, GL_BGRA, GL_FLOAT);

    RenderTargetTextures[RTT_BLOOM_1024] = generateRTT(shadowsize0, GL_RGBA16F, GL_BGR, GL_FLOAT);
    RenderTargetTextures[RTT_BLOOM_512] = generateRTT(shadowsize1, GL_RGBA16F, GL_BGR, GL_FLOAT);
    RenderTargetTextures[RTT_TMP_512] = generateRTT(shadowsize1, GL_RGBA16F, GL_BGR, GL_FLOAT);
    RenderTargetTextures[RTT_BLOOM_256] = generateRTT(shadowsize2, GL_RGBA16F, GL_BGR, GL_FLOAT);
    RenderTargetTextures[RTT_TMP_256] = generateRTT(shadowsize2, GL_RGBA16F, GL_BGR, GL_FLOAT);
    RenderTargetTextures[RTT_BLOOM_128] = generateRTT(shadowsize3, GL_RGBA16F, GL_BGR, GL_FLOAT);
    RenderTargetTextures[RTT_TMP_128] = generateRTT(shadowsize3, GL_RGBA16F, GL_BGR, GL_FLOAT);

    FrameBuffers[FBO_SSAO] = generateFBO(RenderTargetTextures[RTT_SSAO]);
    // Clear this FBO to 1s so that if no SSAO is computed we can still use it.
    glClearColor(1., 1., 1., 1.);
    glClear(GL_COLOR_BUFFER_BIT);

    FrameBuffers[FBO_NORMAL_AND_DEPTHS] = generateFBO(RenderTargetTextures[RTT_NORMAL_AND_DEPTH], DepthStencilTexture);
    FrameBuffers[FBO_COLORS] = generateFBO(RenderTargetTextures[RTT_COLOR], DepthStencilTexture);
    FrameBuffers[FBO_MLAA_COLORS] = generateFBO(RenderTargetTextures[RTT_MLAA_COLORS], DepthStencilTexture);
    FrameBuffers[FBO_LOG_LUMINANCE] = generateFBO(RenderTargetTextures[RTT_LOG_LUMINANCE]);
    FrameBuffers[FBO_TMP1_WITH_DS] = generateFBO(RenderTargetTextures[RTT_TMP1], DepthStencilTexture);
    FrameBuffers[FBO_TMP2_WITH_DS] = generateFBO(RenderTargetTextures[RTT_TMP2], DepthStencilTexture);
    FrameBuffers[FBO_TMP4] = generateFBO(RenderTargetTextures[RTT_TMP4], DepthStencilTexture);
    FrameBuffers[FBO_DISPLACE] = generateFBO(RenderTargetTextures[RTT_DISPLACE], DepthStencilTexture);
    FrameBuffers[FBO_HALF1] = generateFBO(RenderTargetTextures[RTT_HALF1]);
    FrameBuffers[FBO_HALF2] = generateFBO(RenderTargetTextures[RTT_HALF2]);
    FrameBuffers[FBO_QUARTER1] = generateFBO(RenderTargetTextures[RTT_QUARTER1]);
    FrameBuffers[FBO_QUARTER2] = generateFBO(RenderTargetTextures[RTT_QUARTER2]);
    FrameBuffers[FBO_EIGHTH1] = generateFBO(RenderTargetTextures[RTT_EIGHTH1]);
    FrameBuffers[FBO_EIGHTH2] = generateFBO(RenderTargetTextures[RTT_EIGHTH2]);

    FrameBuffers[FBO_BLOOM_1024] = generateFBO(RenderTargetTextures[RTT_BLOOM_1024]);
    FrameBuffers[FBO_BLOOM_512] = generateFBO(RenderTargetTextures[RTT_BLOOM_512]);
    FrameBuffers[FBO_TMP_512] = generateFBO(RenderTargetTextures[RTT_TMP_512]);
    FrameBuffers[FBO_BLOOM_256] = generateFBO(RenderTargetTextures[RTT_BLOOM_256]);
    FrameBuffers[FBO_TMP_256] = generateFBO(RenderTargetTextures[RTT_TMP_256]);
    FrameBuffers[FBO_BLOOM_128] = generateFBO(RenderTargetTextures[RTT_BLOOM_128]);
    FrameBuffers[FBO_TMP_128] = generateFBO(RenderTargetTextures[RTT_TMP_128]);

    FrameBuffers[FBO_COMBINED_TMP1_TMP2] = generateFBO(RenderTargetTextures[RTT_TMP1], DepthStencilTexture);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, RenderTargetTextures[RTT_TMP2], 0);

    if (irr_driver->getGLSLVersion() >= 150)
    {
        glGenFramebuffers(1, &shadowFBO);
        glBindFramebuffer(GL_FRAMEBUFFER, shadowFBO);
        glGenTextures(1, &shadowColorTex);
        glBindTexture(GL_TEXTURE_2D_ARRAY, shadowColorTex);
        glTexImage3D(GL_TEXTURE_2D_ARRAY, 0, GL_R8, 1024, 1024, 4, 0, GL_RED, GL_UNSIGNED_BYTE, 0);
        glGenTextures(1, &shadowDepthTex);
        glBindTexture(GL_TEXTURE_2D_ARRAY, shadowDepthTex);
        glTexImage3D(GL_TEXTURE_2D_ARRAY, 0, GL_DEPTH_COMPONENT24, 1024, 1024, 4, 0, GL_DEPTH_COMPONENT, GL_FLOAT, 0);
        glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, shadowColorTex, 0);
        glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, shadowDepthTex, 0);
        GLenum result = glCheckFramebufferStatus(GL_FRAMEBUFFER);
        assert(result == GL_FRAMEBUFFER_COMPLETE_EXT);
    }
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

RTT::~RTT()
{
    glDeleteFramebuffers(FBO_COUNT, FrameBuffers);
    glDeleteTextures(RTT_COUNT, RenderTargetTextures);
    glDeleteTextures(1, &DepthStencilTexture);
    if (irr_driver->getGLSLVersion() >= 150)
    {
        glDeleteFramebuffers(1, &shadowFBO);
        glDeleteTextures(1, &shadowColorTex);
        glDeleteTextures(1, &shadowDepthTex);
    }
}
