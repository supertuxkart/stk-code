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
#include "graphics/post_processing.hpp"
#include "utils/log.hpp"
#include <ISceneManager.h>

static GLuint generateRTT3D(GLenum target, size_t w, size_t h, size_t d, GLint internalFormat, GLint format, GLint type)
{
    GLuint result;
    glGenTextures(1, &result);
    glBindTexture(target, result);
    if (irr_driver->hasARBTextureStorage())
        glTexStorage3D(target, 1, internalFormat, w, h, d);
    else
        glTexImage3D(target, 0, internalFormat, w, h, d, 0, format, type, 0);
    return result;
}

static GLuint generateRTT(const core::dimension2du &res, GLint internalFormat, GLint format, GLint type, unsigned mipmaplevel = 1)
{
    GLuint result;
    glGenTextures(1, &result);
    glBindTexture(GL_TEXTURE_2D, result);
    if (irr_driver->hasARBTextureStorage())
        glTexStorage2D(GL_TEXTURE_2D, mipmaplevel, internalFormat, res.Width, res.Height);
    else
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
    m_width = width;
    m_height = height;
    m_shadow_FBO = NULL;
    m_RH_FBO = NULL;
    m_RSM = NULL;
    m_RH_FBO = NULL;
    using namespace video;
    using namespace core;

    const dimension2du res(width, height);
    const dimension2du half = res/2;
    const dimension2du quarter = res/4;
    const dimension2du eighth = res/8;

    const u16 shadowside = 1024;
    const dimension2du shadowsize0(shadowside, shadowside);
    const dimension2du shadowsize1(shadowside / 2, shadowside / 2);
    const dimension2du shadowsize2(shadowside / 4, shadowside / 4);
    const dimension2du shadowsize3(shadowside / 8, shadowside / 8);

    unsigned linear_depth_mip_levels = int(ceilf(log2f( float(max_(res.Width, res.Height)) )));

    DepthStencilTexture = generateRTT(res, GL_DEPTH24_STENCIL8, GL_DEPTH_STENCIL, GL_UNSIGNED_INT_24_8);

    // All RTTs are currently RGBA16F mostly with stencil. The four tmp RTTs are the same size
    // as the screen, for use in post-processing.

    RenderTargetTextures[RTT_TMP1] = generateRTT(res, GL_RGBA16F, GL_BGRA, GL_FLOAT);
    RenderTargetTextures[RTT_TMP2] = generateRTT(res, GL_RGBA16F, GL_BGRA, GL_FLOAT);
    RenderTargetTextures[RTT_TMP3] = generateRTT(res, GL_RGBA16F, GL_BGRA, GL_FLOAT);
    RenderTargetTextures[RTT_TMP4] = generateRTT(res, GL_R16F, GL_RED, GL_FLOAT);
    RenderTargetTextures[RTT_LINEAR_DEPTH] = generateRTT(res, GL_R32F, GL_RED, GL_FLOAT, linear_depth_mip_levels);
    RenderTargetTextures[RTT_NORMAL_AND_DEPTH] = generateRTT(res, GL_RGBA16F, GL_RGBA, GL_FLOAT);
    RenderTargetTextures[RTT_COLOR] = generateRTT(res, GL_RGBA16F, GL_BGRA, GL_FLOAT);
    RenderTargetTextures[RTT_MLAA_COLORS] = generateRTT(res, GL_SRGB8_ALPHA8, GL_BGR, GL_UNSIGNED_BYTE);
    RenderTargetTextures[RTT_MLAA_TMP] = generateRTT(res, GL_SRGB8_ALPHA8, GL_BGR, GL_UNSIGNED_BYTE);
    RenderTargetTextures[RTT_MLAA_BLEND] = generateRTT(res, GL_SRGB8_ALPHA8, GL_BGR, GL_UNSIGNED_BYTE);
    RenderTargetTextures[RTT_SSAO] = generateRTT(res, GL_R16F, GL_RED, GL_FLOAT);
    RenderTargetTextures[RTT_DISPLACE] = generateRTT(res, GL_RGBA16F, GL_BGRA, GL_FLOAT);
    RenderTargetTextures[RTT_DIFFUSE] = generateRTT(res, GL_R11F_G11F_B10F, GL_BGR, GL_FLOAT);
    RenderTargetTextures[RTT_SPECULAR] = generateRTT(res, GL_R11F_G11F_B10F, GL_BGR, GL_FLOAT);

    RenderTargetTextures[RTT_HALF1] = generateRTT(half, GL_RGBA16F, GL_BGRA, GL_FLOAT);
    RenderTargetTextures[RTT_QUARTER1] = generateRTT(quarter, GL_RGBA16F, GL_BGRA, GL_FLOAT);
    RenderTargetTextures[RTT_EIGHTH1] = generateRTT(eighth, GL_RGBA16F, GL_BGRA, GL_FLOAT);
    RenderTargetTextures[RTT_HALF1_R] = generateRTT(half, GL_R16F, GL_RED, GL_FLOAT);

    RenderTargetTextures[RTT_HALF2] = generateRTT(half, GL_RGBA16F, GL_BGRA, GL_FLOAT);
    RenderTargetTextures[RTT_QUARTER2] = generateRTT(quarter, GL_RGBA16F, GL_BGRA, GL_FLOAT);
    RenderTargetTextures[RTT_EIGHTH2] = generateRTT(eighth, GL_RGBA16F, GL_BGRA, GL_FLOAT);
    RenderTargetTextures[RTT_HALF2_R] = generateRTT(half, GL_R16F, GL_RED, GL_FLOAT);

    RenderTargetTextures[RTT_BLOOM_1024] = generateRTT(shadowsize0, GL_RGBA16F, GL_BGR, GL_FLOAT);
    RenderTargetTextures[RTT_BLOOM_512] = generateRTT(shadowsize1, GL_RGBA16F, GL_BGR, GL_FLOAT);
    RenderTargetTextures[RTT_TMP_512] = generateRTT(shadowsize1, GL_RGBA16F, GL_BGR, GL_FLOAT);
    RenderTargetTextures[RTT_BLOOM_256] = generateRTT(shadowsize2, GL_RGBA16F, GL_BGR, GL_FLOAT);
    RenderTargetTextures[RTT_TMP_256] = generateRTT(shadowsize2, GL_RGBA16F, GL_BGR, GL_FLOAT);
    RenderTargetTextures[RTT_BLOOM_128] = generateRTT(shadowsize3, GL_RGBA16F, GL_BGR, GL_FLOAT);
    RenderTargetTextures[RTT_TMP_128] = generateRTT(shadowsize3, GL_RGBA16F, GL_BGR, GL_FLOAT);

    std::vector<GLuint> somevector;
    somevector.push_back(RenderTargetTextures[RTT_SSAO]);

    FrameBuffers.push_back(new FrameBuffer(somevector, res.Width, res.Height));
    somevector.clear();
    somevector.push_back(RenderTargetTextures[RTT_NORMAL_AND_DEPTH]);
    FrameBuffers.push_back(new FrameBuffer(somevector, DepthStencilTexture, res.Width, res.Height));
    somevector.clear();
    somevector.push_back(RenderTargetTextures[RTT_DIFFUSE]);
    somevector.push_back(RenderTargetTextures[RTT_SPECULAR]);
    FrameBuffers.push_back(new FrameBuffer(somevector, DepthStencilTexture, res.Width, res.Height));
    somevector.clear();
    somevector.push_back(RenderTargetTextures[RTT_COLOR]);
    FrameBuffers.push_back(new FrameBuffer(somevector, DepthStencilTexture, res.Width, res.Height));
    somevector.clear();
    somevector.push_back(RenderTargetTextures[RTT_DIFFUSE]);
    FrameBuffers.push_back(new FrameBuffer(somevector, res.Width, res.Height));
    somevector.clear();
    somevector.push_back(RenderTargetTextures[RTT_SPECULAR]);
    FrameBuffers.push_back(new FrameBuffer(somevector, res.Width, res.Height));
    somevector.clear();
    somevector.push_back(RenderTargetTextures[RTT_MLAA_COLORS]);
    FrameBuffers.push_back(new FrameBuffer(somevector, res.Width, res.Height));
    somevector.clear();
    somevector.push_back(RenderTargetTextures[RTT_MLAA_BLEND]);
    FrameBuffers.push_back(new FrameBuffer(somevector, res.Width, res.Height));
    somevector.clear();
    somevector.push_back(RenderTargetTextures[RTT_MLAA_TMP]);
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
    somevector.push_back(RenderTargetTextures[RTT_HALF1_R]);
    FrameBuffers.push_back(new FrameBuffer(somevector, half.Width, half.Height));

    somevector.clear();
    somevector.push_back(RenderTargetTextures[RTT_HALF2]);
    FrameBuffers.push_back(new FrameBuffer(somevector, half.Width, half.Height));
    somevector.clear();
    somevector.push_back(RenderTargetTextures[RTT_HALF2_R]);
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

    if (UserConfigParams::m_shadows && !irr_driver->needUBOWorkaround())
    {
        shadowColorTex = generateRTT3D(GL_TEXTURE_2D_ARRAY, 1024, 1024, 4, GL_R8, GL_RED, GL_UNSIGNED_BYTE);
        shadowDepthTex = generateRTT3D(GL_TEXTURE_2D_ARRAY, 1024, 1024, 4, GL_DEPTH24_STENCIL8, GL_DEPTH_STENCIL, GL_UNSIGNED_INT_24_8);

        somevector.clear();
        somevector.push_back(shadowColorTex);
        m_shadow_FBO = new FrameBuffer(somevector, shadowDepthTex, 1024, 1024, true);
    }

    if (UserConfigParams::m_gi)
    {
        //Todo : use "normal" shadowtex
        RSM_Color = generateRTT(shadowsize0, GL_RGB8, GL_RGB, GL_UNSIGNED_BYTE);
        RSM_Normal = generateRTT(shadowsize0, GL_RGB16F, GL_RGB, GL_FLOAT);
        RSM_Depth = generateRTT(shadowsize0, GL_DEPTH24_STENCIL8, GL_DEPTH_STENCIL, GL_UNSIGNED_INT_24_8);

        somevector.clear();
        somevector.push_back(RSM_Color);
        somevector.push_back(RSM_Normal);
        m_RSM = new FrameBuffer(somevector, RSM_Depth, 1024, 1024, true);

        RH_Red = generateRTT3D(GL_TEXTURE_3D, 32, 16, 32, GL_RGBA16F, GL_RGBA, GL_FLOAT);
        RH_Green = generateRTT3D(GL_TEXTURE_3D, 32, 16, 32, GL_RGBA16F, GL_RGBA, GL_FLOAT);
        RH_Blue = generateRTT3D(GL_TEXTURE_3D, 32, 16, 32, GL_RGBA16F, GL_RGBA, GL_FLOAT);

        somevector.clear();
        somevector.push_back(RH_Red);
        somevector.push_back(RH_Green);
        somevector.push_back(RH_Blue);
        m_RH_FBO = new FrameBuffer(somevector, 32, 16, true);
    }

    // Clear this FBO to 1s so that if no SSAO is computed we can still use it.
    getFBO(FBO_HALF1_R).Bind();
    glClearColor(1., 1., 1., 1.);
    glClear(GL_COLOR_BUFFER_BIT);

    getFBO(FBO_COMBINED_DIFFUSE_SPECULAR).Bind();
    glClearColor(.5, .5, .5, .5);
    glClear(GL_COLOR_BUFFER_BIT);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

RTT::~RTT()
{
    glDeleteTextures(RTT_COUNT, RenderTargetTextures);
    glDeleteTextures(1, &DepthStencilTexture);
    if (UserConfigParams::m_shadows && !irr_driver->needUBOWorkaround())
    {
        delete m_shadow_FBO;
        glDeleteTextures(1, &shadowColorTex);
        glDeleteTextures(1, &shadowDepthTex);
    }
    if (UserConfigParams::m_gi)
    {
        delete m_RH_FBO;
        delete m_RSM;
        glDeleteTextures(1, &RSM_Color);
        glDeleteTextures(1, &RSM_Normal);
        glDeleteTextures(1, &RSM_Depth);
        glDeleteTextures(1, &RH_Red);
        glDeleteTextures(1, &RH_Green);
        glDeleteTextures(1, &RH_Blue);
    }
}

FrameBuffer* RTT::render(scene::ICameraSceneNode* camera, float dt)
{
    irr_driver->setRTT(this);

    irr_driver->getSceneManager()->setActiveCamera(camera);

    std::vector<IrrDriver::GlowData> glows;
    // TODO: put this outside of the rendering loop
    irr_driver->generateDiffuseCoefficients();
    irr_driver->computeCameraMatrix(camera, m_width, m_height);
    unsigned plc = irr_driver->UpdateLightsInfo(camera, dt);
    irr_driver->renderScene(camera, plc, glows, dt, false, true);
    FrameBuffer* frame_buffer = irr_driver->getPostProcessing()->render(camera, false);

    // reset
    glViewport(0, 0, UserConfigParams::m_width, UserConfigParams::m_height);
    irr_driver->setRTT(NULL);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    irr_driver->getSceneManager()->setActiveCamera(NULL);
    return frame_buffer;
}
