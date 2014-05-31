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

RTT::RTT(size_t width, size_t height)
{
    m_shadow_FBO = NULL;
    m_RSM = NULL;
    m_RH_FBO = NULL;
    using namespace video;
    using namespace core;

    IVideoDriver * const drv = irr_driver->getVideoDriver();
    const dimension2du res(width, height);
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
    RenderTargetTextures[RTT_LINEAR_DEPTH] = generateRTT(res, GL_R32F, GL_RED, GL_FLOAT);
    RenderTargetTextures[RTT_NORMAL_AND_DEPTH] = generateRTT(res, GL_RGBA16F, GL_RGBA, GL_FLOAT);
    RenderTargetTextures[RTT_COLOR] = generateRTT(res, GL_RGBA16F, GL_BGRA, GL_FLOAT);
    RenderTargetTextures[RTT_MLAA_COLORS] = generateRTT(res, GL_SRGB, GL_BGR, GL_UNSIGNED_BYTE);
    RenderTargetTextures[RTT_SSAO] = generateRTT(res, GL_R16F, GL_RED, GL_FLOAT);
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
    RenderTargetTextures[RTT_LOG_LUMINANCE] = generateRTT(shadowsize0, GL_R16F, GL_RED, GL_FLOAT);

    std::vector<GLuint> somevector;
    somevector.push_back(RenderTargetTextures[RTT_SSAO]);

    FrameBuffers.push_back(new FrameBuffer(somevector, res.Width, res.Height));
    // Clear this FBO to 1s so that if no SSAO is computed we can still use it.
    glClearColor(1., 1., 1., 1.);
    glClear(GL_COLOR_BUFFER_BIT);

    somevector.clear();
    somevector.push_back(RenderTargetTextures[RTT_NORMAL_AND_DEPTH]);
    FrameBuffers.push_back(new FrameBuffer(somevector, DepthStencilTexture, res.Width, res.Height));
    somevector.clear();
    somevector.push_back(RenderTargetTextures[RTT_TMP1]);
    somevector.push_back(RenderTargetTextures[RTT_TMP2]);
    FrameBuffers.push_back(new FrameBuffer(somevector, DepthStencilTexture, res.Width, res.Height));
    somevector.clear();
    somevector.push_back(RenderTargetTextures[RTT_COLOR]);
    FrameBuffers.push_back(new FrameBuffer(somevector, DepthStencilTexture, res.Width, res.Height));
    somevector.clear();
    somevector.push_back(RenderTargetTextures[RTT_LOG_LUMINANCE]);
    FrameBuffers.push_back(new FrameBuffer(somevector, res.Width, res.Height));
    somevector.clear();
    somevector.push_back(RenderTargetTextures[RTT_MLAA_COLORS]);
    FrameBuffers.push_back(new FrameBuffer(somevector, res.Width, res.Height));
    somevector.clear();
    somevector.push_back(RenderTargetTextures[RTT_TMP1]);
    FrameBuffers.push_back(new FrameBuffer(somevector, DepthStencilTexture, res.Width, res.Height));
    somevector.clear();
    somevector.push_back(RenderTargetTextures[RTT_TMP2]);
    FrameBuffers.push_back(new FrameBuffer(somevector, DepthStencilTexture, res.Width, res.Height));
    somevector.clear();
    somevector.push_back(RenderTargetTextures[RTT_TMP4]);
    FrameBuffers.push_back(new FrameBuffer(somevector, res.Width, res.Height));
    somevector.clear();
    somevector.push_back(RenderTargetTextures[RTT_LINEAR_DEPTH]);
    FrameBuffers.push_back(new FrameBuffer(somevector, res.Width, res.Height));
    somevector.clear();
    somevector.push_back(RenderTargetTextures[RTT_HALF1]);
    FrameBuffers.push_back(new FrameBuffer(somevector, half.Width, half.Height));
    somevector.clear();
    somevector.push_back(RenderTargetTextures[RTT_HALF2]);
    FrameBuffers.push_back(new FrameBuffer(somevector, half.Width, half.Height));
    somevector.clear();
    somevector.push_back(RenderTargetTextures[RTT_QUARTER1]);
    FrameBuffers.push_back(new FrameBuffer(somevector, quarter.Width, quarter.Height));
    somevector.clear();
    somevector.push_back(RenderTargetTextures[RTT_QUARTER2]);
    FrameBuffers.push_back(new FrameBuffer(somevector, quarter.Width, quarter.Height));
    somevector.clear();
    somevector.push_back(RenderTargetTextures[RTT_EIGHTH1]);
    FrameBuffers.push_back(new FrameBuffer(somevector, eighth.Width, eighth.Height));
    somevector.clear();
    somevector.push_back(RenderTargetTextures[RTT_EIGHTH2]);
    FrameBuffers.push_back(new FrameBuffer(somevector, eighth.Width, eighth.Height));
    somevector.clear();
    somevector.push_back(RenderTargetTextures[RTT_DISPLACE]);
    FrameBuffers.push_back(new FrameBuffer(somevector, DepthStencilTexture, res.Width, res.Height));
    somevector.clear();
    somevector.push_back(RenderTargetTextures[RTT_BLOOM_1024]);
    FrameBuffers.push_back(new FrameBuffer(somevector, 1024, 1024));
    somevector.clear();
    somevector.push_back(RenderTargetTextures[RTT_BLOOM_512]);
    FrameBuffers.push_back(new FrameBuffer(somevector, 512, 512));
    somevector.clear();
    somevector.push_back(RenderTargetTextures[RTT_TMP_512]);
    FrameBuffers.push_back(new FrameBuffer(somevector, 512, 512));
    somevector.clear();
    somevector.push_back(RenderTargetTextures[RTT_BLOOM_256]);
    FrameBuffers.push_back(new FrameBuffer(somevector, 256, 256));
    somevector.clear();
    somevector.push_back(RenderTargetTextures[RTT_TMP_256]);
    FrameBuffers.push_back(new FrameBuffer(somevector, 256, 256));
    somevector.clear();
    somevector.push_back(RenderTargetTextures[RTT_BLOOM_128]);
    FrameBuffers.push_back(new FrameBuffer(somevector, 128, 128));
    somevector.clear();
    somevector.push_back(RenderTargetTextures[RTT_TMP_128]);
    FrameBuffers.push_back(new FrameBuffer(somevector, 128, 128));

    if (irr_driver->getGLSLVersion() >= 150)
    {
        glGenTextures(1, &shadowColorTex);
        glBindTexture(GL_TEXTURE_2D_ARRAY, shadowColorTex);
        glTexImage3D(GL_TEXTURE_2D_ARRAY, 0, GL_R8, 1024, 1024, 4, 0, GL_RED, GL_UNSIGNED_BYTE, 0);
        glGenTextures(1, &shadowDepthTex);
        glBindTexture(GL_TEXTURE_2D_ARRAY, shadowDepthTex);
        glTexImage3D(GL_TEXTURE_2D_ARRAY, 0, GL_DEPTH_STENCIL, 1024, 1024, 4, 0, GL_DEPTH_STENCIL, GL_UNSIGNED_INT_24_8, 0);

        somevector.clear();
        somevector.push_back(shadowColorTex);
        m_shadow_FBO = new FrameBuffer(somevector, shadowDepthTex, 1024, 1024, true);

        //Todo : use "normal" shadowtex
        glGenTextures(1, &RSM_Color);
        glBindTexture(GL_TEXTURE_2D, RSM_Color);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, 1024, 1024, 0, GL_RGB, GL_UNSIGNED_BYTE, 0);
        glGenTextures(1, &RSM_Normal);
        glBindTexture(GL_TEXTURE_2D, RSM_Normal);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, 1024, 1024, 0, GL_RGB, GL_FLOAT, 0);
        glGenTextures(1, &RSM_Depth);
        glBindTexture(GL_TEXTURE_2D, RSM_Depth);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_STENCIL, 1024, 1024, 0, GL_DEPTH_STENCIL, GL_UNSIGNED_INT_24_8, 0);

        somevector.clear();
        somevector.push_back(RSM_Color);
        somevector.push_back(RSM_Normal);
        m_RSM = new FrameBuffer(somevector, RSM_Depth, 1024, 1024, true);

        glGenTextures(1, &RH_Red);
        glBindTexture(GL_TEXTURE_3D, RH_Red);
        glTexImage3D(GL_TEXTURE_3D, 0, GL_RGBA16F, 32, 16, 32, 0, GL_RGBA, GL_FLOAT, 0);
        glGenTextures(1, &RH_Green);
        glBindTexture(GL_TEXTURE_3D, RH_Green);
        glTexImage3D(GL_TEXTURE_3D, 0, GL_RGBA16F, 32, 16, 32, 0, GL_RGBA, GL_FLOAT, 0);
        glGenTextures(1, &RH_Blue);
        glBindTexture(GL_TEXTURE_3D, RH_Blue);
        glTexImage3D(GL_TEXTURE_3D, 0, GL_RGBA16F, 32, 16, 32, 0, GL_RGBA, GL_FLOAT, 0);

        somevector.clear();
        somevector.push_back(RH_Red);
        somevector.push_back(RH_Green);
        somevector.push_back(RH_Blue);
        m_RH_FBO = new FrameBuffer(somevector, 32, 16, true);
    }
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

RTT::~RTT()
{
    glDeleteTextures(RTT_COUNT, RenderTargetTextures);
    glDeleteTextures(1, &DepthStencilTexture);
    if (irr_driver->getGLSLVersion() >= 150)
    {
        glDeleteTextures(1, &shadowColorTex);
        glDeleteTextures(1, &shadowDepthTex);
        glDeleteTextures(1, &RSM_Color);
        glDeleteTextures(1, &RSM_Normal);
        glDeleteTextures(1, &RSM_Depth);
        glDeleteTextures(1, &RH_Red);
        glDeleteTextures(1, &RH_Green);
        glDeleteTextures(1, &RH_Blue);

        delete m_shadow_FBO;
        delete m_RH_FBO;
        delete m_RSM;
    }
}
