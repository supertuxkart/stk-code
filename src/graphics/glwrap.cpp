#include "graphics/glwrap.hpp"

#include "config/user_config.hpp"
#include "config/hardware_stats.hpp"
#include "graphics/stkmesh.hpp"
#include "utils/profiler.hpp"
#include "utils/cpp2011.hpp"


#include "../../lib/irrlicht/source/Irrlicht/COpenGLTexture.h"

#include <fstream>
#include <string>
#include <sstream>

static bool is_gl_init = false;

#if DEBUG
bool GLContextDebugBit = true;
#else
bool GLContextDebugBit = false;
#endif


#ifdef DEBUG
#if !defined(__APPLE__)
#define ARB_DEBUG_OUTPUT
#endif
#endif

#ifdef ARB_DEBUG_OUTPUT
static void
#ifdef WIN32
CALLBACK
#endif
debugCallback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length,
              const GLchar* msg, const void *userparam)
{
#ifdef GL_DEBUG_SEVERITY_NOTIFICATION
    // ignore minor notifications sent by some drivers (notably the nvidia one)
    if (severity == GL_DEBUG_SEVERITY_NOTIFICATION)
        return;
#endif

    switch(source)
    {
    case GL_DEBUG_SOURCE_API_ARB:
        Log::warn("GLWrap", "OpenGL debug callback - API");
        break;
    case GL_DEBUG_SOURCE_WINDOW_SYSTEM_ARB:
        Log::warn("GLWrap", "OpenGL debug callback - WINDOW_SYSTEM");
        break;
    case GL_DEBUG_SOURCE_SHADER_COMPILER_ARB:
        Log::warn("GLWrap", "OpenGL debug callback - SHADER_COMPILER");
        break;
    case GL_DEBUG_SOURCE_THIRD_PARTY_ARB:
        Log::warn("GLWrap", "OpenGL debug callback - THIRD_PARTY");
        break;
    case GL_DEBUG_SOURCE_APPLICATION_ARB:
        Log::warn("GLWrap", "OpenGL debug callback - APPLICATION");
        break;
    case GL_DEBUG_SOURCE_OTHER_ARB:
        Log::warn("GLWrap", "OpenGL debug callback - OTHER");
        break;
    }

    switch(type)
    {
    case GL_DEBUG_TYPE_ERROR_ARB:
        Log::warn("GLWrap", "    Error type : ERROR");
        break;
    case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR_ARB:
        Log::warn("GLWrap", "    Error type : DEPRECATED_BEHAVIOR");
        break;
    case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR_ARB:
        Log::warn("GLWrap", "    Error type : UNDEFINED_BEHAVIOR");
        break;
    case GL_DEBUG_TYPE_PORTABILITY_ARB:
        Log::warn("GLWrap", "    Error type : PORTABILITY");
        break;
    case GL_DEBUG_TYPE_PERFORMANCE_ARB:
        Log::warn("GLWrap", "    Error type : PERFORMANCE");
        break;
    case GL_DEBUG_TYPE_OTHER_ARB:
        Log::warn("GLWrap", "    Error type : OTHER");
        break;
    }

    switch(severity)
    {
    case GL_DEBUG_SEVERITY_HIGH_ARB:
        Log::warn("GLWrap", "    Severity : HIGH");
        break;
    case GL_DEBUG_SEVERITY_MEDIUM_ARB:
        Log::warn("GLWrap", "    Severity : MEDIUM");
        break;
    case GL_DEBUG_SEVERITY_LOW_ARB:
        Log::warn("GLWrap", "    Severity : LOW");
        break;
    }

    if (msg)
        Log::warn("GLWrap", "    Message : %s", msg);
}
#endif

void initGL()
{
    if (is_gl_init)
        return;
    is_gl_init = true;
    // For Mesa extension reporting
#ifndef WIN32
    glewExperimental = GL_TRUE;
#endif
    GLenum err = glewInit();
    if (GLEW_OK != err)
        Log::fatal("GLEW", "Glew initialisation failed with error %s", glewGetErrorString(err));
#ifdef ARB_DEBUG_OUTPUT
    if (glDebugMessageCallbackARB)
        glDebugMessageCallbackARB((GLDEBUGPROCARB)debugCallback, NULL);
#endif
}

static std::string LoadHeader()
{
    std::string result;
    std::ifstream Stream(file_manager->getAsset("shaders/header.txt").c_str(), std::ios::in);

    if (Stream.is_open())
    {
        std::string Line = "";
        while (getline(Stream, Line))
            result += "\n" + Line;
        Stream.close();
    }

    return result;
}

// Mostly from shader tutorial
GLuint LoadShader(const char * file, unsigned type)
{
    GLuint Id = glCreateShader(type);
    char versionString[20];
    sprintf(versionString, "#version %d\n", irr_driver->getGLSLVersion());
    std::string Code = versionString;
    if (irr_driver->hasVSLayerExtension())
        Code += "#extension GL_AMD_vertex_shader_layer : enable\n";
    if (UserConfigParams::m_azdo)
        Code += "#extension GL_ARB_bindless_texture : enable\n";
    else
    {
        Code += "#extension GL_ARB_bindless_texture : disable\n";
        Code += "#undef GL_ARB_bindless_texture\n";
    }
    std::ifstream Stream(file, std::ios::in);
    Code += "//" + std::string(file) + "\n";
    if (irr_driver->needUBOWorkaround())
        Code += "#define UBO_DISABLED\n";
    if (irr_driver->hasVSLayerExtension())
        Code += "#define VSLayer\n";
    if (irr_driver->needsRGBBindlessWorkaround())
        Code += "#define SRGBBindlessFix\n";
    Code += LoadHeader();
    if (Stream.is_open())
    {
        std::string Line = "";
        while (getline(Stream, Line))
            Code += "\n" + Line;
        Stream.close();
    }
    GLint Result = GL_FALSE;
    int InfoLogLength;
    Log::info("GLWrap", "Compiling shader : %s", file);
    char const * SourcePointer = Code.c_str();
    int length = (int)strlen(SourcePointer);
    glShaderSource(Id, 1, &SourcePointer, &length);
    glCompileShader(Id);

    glGetShaderiv(Id, GL_COMPILE_STATUS, &Result);
    if (Result == GL_FALSE)
    {
        Log::error("GLWrap", "Error in shader %s", file);
        glGetShaderiv(Id, GL_INFO_LOG_LENGTH, &InfoLogLength);
        if(InfoLogLength<0)
            InfoLogLength = 1024;
        char *ErrorMessage = new char[InfoLogLength];
        ErrorMessage[0]=0;
        glGetShaderInfoLog(Id, InfoLogLength, NULL, ErrorMessage);
        Log::error("GLWrap", ErrorMessage);
        delete[] ErrorMessage;
    }

    glGetError();

    return Id;
}

void setAttribute(AttributeType Tp, GLuint ProgramID)
{
    switch (Tp)
    {
    case OBJECT:
        glBindAttribLocation(ProgramID, 0, "Position");
        glBindAttribLocation(ProgramID, 1, "Normal");
        glBindAttribLocation(ProgramID, 2, "Color");
        glBindAttribLocation(ProgramID, 3, "Texcoord");
        glBindAttribLocation(ProgramID, 4, "SecondTexcoord");
        glBindAttribLocation(ProgramID, 5, "Tangent");
        glBindAttribLocation(ProgramID, 6, "Bitangent");
        glBindAttribLocation(ProgramID, 7, "Origin");
        glBindAttribLocation(ProgramID, 8, "Orientation");
        glBindAttribLocation(ProgramID, 9, "Scale");
        break;
    case PARTICLES_SIM:
        glBindAttribLocation(ProgramID, 0, "particle_position");
        glBindAttribLocation(ProgramID, 1, "lifetime");
        glBindAttribLocation(ProgramID, 2, "particle_velocity");
        glBindAttribLocation(ProgramID, 3, "size");
        glBindAttribLocation(ProgramID, 4, "particle_position_initial");
        glBindAttribLocation(ProgramID, 5, "lifetime_initial");
        glBindAttribLocation(ProgramID, 6, "particle_velocity_initial");
        glBindAttribLocation(ProgramID, 7, "size_initial");
        break;
    case PARTICLES_RENDERING:
        glBindAttribLocation(ProgramID, 1, "lifetime");
        glBindAttribLocation(ProgramID, 2, "size");
        glBindAttribLocation(ProgramID, 4, "quadcorner");
        glBindAttribLocation(ProgramID, 5, "rotationvec");
        glBindAttribLocation(ProgramID, 6, "anglespeed");
        break;
    }
}

GLuint LoadTFBProgram(const char * vertex_file_path, const char **varyings, unsigned varyingscount)
{
    GLuint Program = glCreateProgram();
    loadAndAttach(Program, GL_VERTEX_SHADER, vertex_file_path);
    if (irr_driver->getGLSLVersion() < 330)
        setAttribute(PARTICLES_SIM, Program);
    glTransformFeedbackVaryings(Program, varyingscount, varyings, GL_INTERLEAVED_ATTRIBS);
    glLinkProgram(Program);

    GLint Result = GL_FALSE;
    int InfoLogLength;
    glGetProgramiv(Program, GL_LINK_STATUS, &Result);
    if (Result == GL_FALSE)
    {
        glGetProgramiv(Program, GL_INFO_LOG_LENGTH, &InfoLogLength);
        char *ErrorMessage = new char[InfoLogLength];
        glGetProgramInfoLog(Program, InfoLogLength, NULL, ErrorMessage);
        Log::error("GLWrap", ErrorMessage);
        delete[] ErrorMessage;
    }

    glGetError();

    return Program;
}

void setTexture(unsigned TextureUnit, GLuint TextureId, GLenum MagFilter, GLenum MinFilter, bool allowAF)
{
    glActiveTexture(GL_TEXTURE0 + TextureUnit);
    glBindTexture(GL_TEXTURE_2D, TextureId);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, MagFilter);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, MinFilter);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    int aniso = UserConfigParams::m_anisotropic;
    if (aniso == 0) aniso = 1;
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, allowAF ? (float)aniso : 1.0f);

    glGetError();
}

ScopedGPUTimer::ScopedGPUTimer(GPUTimer &t) : timer(t)
{
    if (!UserConfigParams::m_profiler_enabled) return;
    if (profiler.isFrozen()) return;
    if (!timer.canSubmitQuery) return;
#ifdef GL_TIME_ELAPSED
    if (!timer.initialised)
    {
        glGenQueries(1, &timer.query);
        timer.initialised = true;
    }
    glBeginQuery(GL_TIME_ELAPSED, timer.query);
#endif
}
ScopedGPUTimer::~ScopedGPUTimer()
{
    if (!UserConfigParams::m_profiler_enabled) return;
    if (profiler.isFrozen()) return;
    if (!timer.canSubmitQuery) return;
#ifdef GL_TIME_ELAPSED
    glEndQuery(GL_TIME_ELAPSED);
    timer.canSubmitQuery = false;
#endif
}

GPUTimer::GPUTimer() : initialised(false), lastResult(0), canSubmitQuery(true)
{
}

unsigned GPUTimer::elapsedTimeus()
{
    if (!initialised)
        return 0;
    GLuint result;
    glGetQueryObjectuiv(query, GL_QUERY_RESULT_AVAILABLE, &result);
    if (result == GL_FALSE)
        return lastResult;
    glGetQueryObjectuiv(query, GL_QUERY_RESULT, &result);
    lastResult = result / 1000;
    canSubmitQuery = true;
    return result / 1000;
}

FrameBuffer::FrameBuffer() {}

FrameBuffer::FrameBuffer(const std::vector<GLuint> &RTTs, size_t w, size_t h, bool layered) :
    RenderTargets(RTTs), DepthTexture(0), width(w), height(h)
{
    glGenFramebuffers(1, &fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    if (layered)
    {
        for (unsigned i = 0; i < RTTs.size(); i++)
            glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, RTTs[i], 0);
    }
    else
    {
        for (unsigned i = 0; i < RTTs.size(); i++)
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, GL_TEXTURE_2D, RTTs[i], 0);
    }
    GLenum result = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    assert(result == GL_FRAMEBUFFER_COMPLETE_EXT);
}

FrameBuffer::FrameBuffer(const std::vector<GLuint> &RTTs, GLuint DS, size_t w, size_t h, bool layered) :
    RenderTargets(RTTs), DepthTexture(DS), width(w), height(h)
{
    glGenFramebuffers(1, &fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    if (layered)
    {
        for (unsigned i = 0; i < RTTs.size(); i++)
            glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, RTTs[i], 0);
        glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, DS, 0);
    }
    else
    {
        for (unsigned i = 0; i < RTTs.size(); i++)
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, GL_TEXTURE_2D, RTTs[i], 0);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_TEXTURE_2D, DS, 0);
    }
    GLenum result = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    assert(result == GL_FRAMEBUFFER_COMPLETE_EXT);
}

FrameBuffer::~FrameBuffer()
{
    glDeleteFramebuffers(1, &fbo);
}

void FrameBuffer::Bind()
{
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    glViewport(0, 0, (int)width, (int)height);
    GLenum bufs[] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2 };
    glDrawBuffers((int)RenderTargets.size(), bufs);
}

void FrameBuffer::Blit(const FrameBuffer &Src, FrameBuffer &Dst, GLbitfield mask, GLenum filter)
{
    glBindFramebuffer(GL_READ_FRAMEBUFFER, Src.fbo);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, Dst.fbo);
    glBlitFramebuffer(0, 0, (int)Src.width, (int)Src.height, 0, 0,
                      (int)Dst.width, (int)Dst.height, mask, filter);
    glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
}

void FrameBuffer::BlitToDefault(size_t x0, size_t y0, size_t x1, size_t y1)
{
    glBindFramebuffer(GL_READ_FRAMEBUFFER, fbo);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
    glBlitFramebuffer(0, 0, (int)width, (int)height, (int)x0, (int)y0, (int)x1, (int)y1, GL_COLOR_BUFFER_BIT, GL_NEAREST);
    glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
}


void draw3DLine(const core::vector3df& start,
                const core::vector3df& end, irr::video::SColor color)
{
    if (!irr_driver->isGLSL()) {
        irr_driver->getVideoDriver()->draw3DLine(start, end, color);
        return;
    }

    float vertex[6] = {
        start.X, start.Y, start.Z,
        end.X, end.Y, end.Z
    };

    glBindVertexArray(UtilShader::ColoredLine::vao);
    glBindBuffer(GL_ARRAY_BUFFER, UtilShader::ColoredLine::vbo);
    glBufferSubData(GL_ARRAY_BUFFER, 0, 6 * sizeof(float), vertex);

    glUseProgram(UtilShader::ColoredLine::Program);
    UtilShader::ColoredLine::setUniforms(color);
    glDrawArrays(GL_LINES, 0, 2);

    glGetError();
}

static void drawTexColoredQuad(const video::ITexture *texture, const video::SColor *col, float width, float height,
                               float center_pos_x, float center_pos_y, float tex_center_pos_x, float tex_center_pos_y,
                               float tex_width, float tex_height)
{
    unsigned colors[] = {
        col[0].getRed(), col[0].getGreen(), col[0].getBlue(), col[0].getAlpha(),
        col[1].getRed(), col[1].getGreen(), col[1].getBlue(), col[1].getAlpha(),
        col[2].getRed(), col[2].getGreen(), col[2].getBlue(), col[2].getAlpha(),
        col[3].getRed(), col[3].getGreen(), col[3].getBlue(), col[3].getAlpha(),
    };

    glBindBuffer(GL_ARRAY_BUFFER, UIShader::ColoredTextureRectShader::getInstance()->colorvbo);
    glBufferSubData(GL_ARRAY_BUFFER, 0, 16 * sizeof(unsigned), colors);

    glUseProgram(UIShader::ColoredTextureRectShader::getInstance()->Program);
    glBindVertexArray(UIShader::ColoredTextureRectShader::getInstance()->vao);

    UIShader::ColoredTextureRectShader::getInstance()->SetTextureUnits(createVector<GLuint>(static_cast<const irr::video::COpenGLTexture*>(texture)->getOpenGLTextureName()));
    UIShader::ColoredTextureRectShader::getInstance()->setUniforms(
        core::vector2df(center_pos_x, center_pos_y), core::vector2df(width, height),
        core::vector2df(tex_center_pos_x, tex_center_pos_y), core::vector2df(tex_width, tex_height));

    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    glGetError();
}

static
void drawTexQuad(GLuint texture, float width, float height,
                 float center_pos_x, float center_pos_y, float tex_center_pos_x, float tex_center_pos_y,
                 float tex_width, float tex_height)
{
    glUseProgram(UIShader::TextureRectShader::getInstance()->Program);
    glBindVertexArray(SharedObject::UIVAO);

    UIShader::TextureRectShader::getInstance()->SetTextureUnits(createVector<GLuint>(texture));
    UIShader::TextureRectShader::getInstance()->setUniforms(
        core::vector2df(center_pos_x, center_pos_y), core::vector2df(width, height),
        core::vector2df(tex_center_pos_x, tex_center_pos_y),
        core::vector2df(tex_width, tex_height));

    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    glGetError();
}

static void
getSize(unsigned texture_width, unsigned texture_height, bool textureisRTT,
        const core::rect<s32>& destRect,
        const core::rect<s32>& sourceRect,
        float &width, float &height,
        float &center_pos_x, float &center_pos_y,
        float &tex_width, float &tex_height,
        float &tex_center_pos_x, float &tex_center_pos_y
    )
{
    core::dimension2d<u32> frame_size =
        irr_driver->getVideoDriver()->getCurrentRenderTargetSize();
    const int screen_w = frame_size.Width;
    const int screen_h = frame_size.Height;
    center_pos_x = float(destRect.UpperLeftCorner.X + destRect.LowerRightCorner.X);
    center_pos_x /= screen_w;
    center_pos_x -= 1.;
    center_pos_y = float(destRect.UpperLeftCorner.Y + destRect.LowerRightCorner.Y);
    center_pos_y /= screen_h;
    center_pos_y = float(1.f - center_pos_y);
    width = float(destRect.LowerRightCorner.X - destRect.UpperLeftCorner.X);
    width /= screen_w;
    height = float(destRect.LowerRightCorner.Y - destRect.UpperLeftCorner.Y);
    height /= screen_h;

    tex_center_pos_x = float(sourceRect.UpperLeftCorner.X + sourceRect.LowerRightCorner.X);
    tex_center_pos_x /= texture_width * 2.f;
    tex_center_pos_y = float(sourceRect.UpperLeftCorner.Y + sourceRect.LowerRightCorner.Y);
    tex_center_pos_y /= texture_height * 2.f;
    tex_width = float(sourceRect.LowerRightCorner.X - sourceRect.UpperLeftCorner.X);
    tex_width /= texture_width * 2.f;
    tex_height = float(sourceRect.LowerRightCorner.Y - sourceRect.UpperLeftCorner.Y);
    tex_height /= texture_height * 2.f;

    if (textureisRTT)
        tex_height = -tex_height;

    const f32 invW = 1.f / static_cast<f32>(texture_width);
    const f32 invH = 1.f / static_cast<f32>(texture_height);
    const core::rect<f32> tcoords(
        sourceRect.UpperLeftCorner.X * invW,
        sourceRect.UpperLeftCorner.Y * invH,
        sourceRect.LowerRightCorner.X * invW,
        sourceRect.LowerRightCorner.Y *invH);
}

void draw2DImage(const video::ITexture* texture, const core::rect<s32>& destRect,
                 const core::rect<s32>& sourceRect, const core::rect<s32>* clipRect,
                 const video::SColor &colors, bool useAlphaChannelOfTexture)
{
    if (!irr_driver->isGLSL()) {
        video::SColor duplicatedArray[4] = {
            colors, colors, colors, colors
        };
        draw2DImage(texture, destRect, sourceRect, clipRect, duplicatedArray, useAlphaChannelOfTexture);
        return;
    }

    float width, height,
        center_pos_x, center_pos_y,
        tex_width, tex_height,
        tex_center_pos_x, tex_center_pos_y;

    getSize(texture->getOriginalSize().Width, texture->getOriginalSize().Height, texture->isRenderTarget(),
            destRect, sourceRect, width, height, center_pos_x, center_pos_y,
            tex_width, tex_height, tex_center_pos_x, tex_center_pos_y);

    if (useAlphaChannelOfTexture)
    {
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    }
    else
    {
        glDisable(GL_BLEND);
    }
    if (clipRect)
    {
        if (!clipRect->isValid())
            return;

        glEnable(GL_SCISSOR_TEST);
        const core::dimension2d<u32>& renderTargetSize = irr_driver->getVideoDriver()->getCurrentRenderTargetSize();
        glScissor(clipRect->UpperLeftCorner.X, renderTargetSize.Height - clipRect->LowerRightCorner.Y,
                  clipRect->getWidth(), clipRect->getHeight());
    }

    glUseProgram(UIShader::UniformColoredTextureRectShader::getInstance()->Program);
    glBindVertexArray(SharedObject::UIVAO);

    UIShader::UniformColoredTextureRectShader::getInstance()->SetTextureUnits(createVector<GLuint>(static_cast<const irr::video::COpenGLTexture*>(texture)->getOpenGLTextureName()));
    UIShader::UniformColoredTextureRectShader::getInstance()->setUniforms(
        core::vector2df(center_pos_x, center_pos_y), core::vector2df(width, height), core::vector2df(tex_center_pos_x, tex_center_pos_y), core::vector2df(tex_width, tex_height), colors);

    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    if (clipRect)
        glDisable(GL_SCISSOR_TEST);
    glUseProgram(0);

    glGetError();
}

void draw2DImageFromRTT(GLuint texture, size_t texture_w, size_t texture_h,
    const core::rect<s32>& destRect,
    const core::rect<s32>& sourceRect, const core::rect<s32>* clipRect,
    const video::SColor &colors, bool useAlphaChannelOfTexture)
{
    if (useAlphaChannelOfTexture)
    {
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    }
    float width, height,
        center_pos_x, center_pos_y,
        tex_width, tex_height,
        tex_center_pos_x, tex_center_pos_y;

    getSize((int)texture_w, (int)texture_h, true,
        destRect, sourceRect, width, height, center_pos_x, center_pos_y,
        tex_width, tex_height, tex_center_pos_x, tex_center_pos_y);

    glUseProgram(UIShader::UniformColoredTextureRectShader::getInstance()->Program);
    glBindVertexArray(SharedObject::UIVAO);

    UIShader::UniformColoredTextureRectShader::getInstance()->SetTextureUnits(createVector<GLuint>(texture));
    UIShader::UniformColoredTextureRectShader::getInstance()->setUniforms(
        core::vector2df(center_pos_x, center_pos_y), core::vector2df(width, height),
        core::vector2df(tex_center_pos_x, tex_center_pos_y), core::vector2df(tex_width, tex_height),
        colors);

    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void draw2DImage(const video::ITexture* texture, const core::rect<s32>& destRect,
                 const core::rect<s32>& sourceRect, const core::rect<s32>* clipRect,
                 const video::SColor* const colors, bool useAlphaChannelOfTexture)
{
    if (!irr_driver->isGLSL())
    {
        irr_driver->getVideoDriver()->draw2DImage(texture, destRect, sourceRect, clipRect, colors, useAlphaChannelOfTexture);
        return;
    }

    float width, height,
        center_pos_x, center_pos_y,
        tex_width, tex_height,
        tex_center_pos_x, tex_center_pos_y;

    getSize(texture->getOriginalSize().Width, texture->getOriginalSize().Height, texture->isRenderTarget(),
            destRect, sourceRect, width, height, center_pos_x, center_pos_y,
            tex_width, tex_height, tex_center_pos_x, tex_center_pos_y);

    if (useAlphaChannelOfTexture)
    {
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    }
    else
    {
        glDisable(GL_BLEND);
    }
    if (clipRect)
    {
        if (!clipRect->isValid())
            return;

        glEnable(GL_SCISSOR_TEST);
        const core::dimension2d<u32>& renderTargetSize = irr_driver->getVideoDriver()->getCurrentRenderTargetSize();
        glScissor(clipRect->UpperLeftCorner.X, renderTargetSize.Height - clipRect->LowerRightCorner.Y,
                  clipRect->getWidth(), clipRect->getHeight());
    }
    if (colors)
        drawTexColoredQuad(texture, colors, width, height, center_pos_x, center_pos_y,
                           tex_center_pos_x, tex_center_pos_y, tex_width, tex_height);
    else
        drawTexQuad(static_cast<const irr::video::COpenGLTexture*>(texture)->getOpenGLTextureName(), width, height, center_pos_x, center_pos_y,
                    tex_center_pos_x, tex_center_pos_y, tex_width, tex_height);
    if (clipRect)
        glDisable(GL_SCISSOR_TEST);
    glUseProgram(0);

    glGetError();
}

void GL32_draw2DRectangle(video::SColor color, const core::rect<s32>& position,
                          const core::rect<s32>* clip)
{

    if (!irr_driver->isGLSL())
    {
        irr_driver->getVideoDriver()->draw2DRectangle(color, position, clip);
        return;
    }

    core::dimension2d<u32> frame_size =
        irr_driver->getVideoDriver()->getCurrentRenderTargetSize();
    const int screen_w = frame_size.Width;
    const int screen_h = frame_size.Height;
    float center_pos_x = float(position.UpperLeftCorner.X + position.LowerRightCorner.X);
    center_pos_x /= screen_w;
    center_pos_x -= 1;
    float center_pos_y = float(position.UpperLeftCorner.Y + position.LowerRightCorner.Y);
    center_pos_y /= screen_h;
    center_pos_y = 1 - center_pos_y;
    float width = float(position.LowerRightCorner.X - position.UpperLeftCorner.X);
    width /= screen_w;
    float height = float(position.LowerRightCorner.Y - position.UpperLeftCorner.Y);
    height /= screen_h;

    if (color.getAlpha() < 255)
    {
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    }
    else
    {
        glDisable(GL_BLEND);
    }

    if (clip)
    {
        if (!clip->isValid())
            return;

        glEnable(GL_SCISSOR_TEST);
        const core::dimension2d<u32>& renderTargetSize = irr_driver->getVideoDriver()->getCurrentRenderTargetSize();
        glScissor(clip->UpperLeftCorner.X, renderTargetSize.Height - clip->LowerRightCorner.Y,
                  clip->getWidth(), clip->getHeight());
    }

    glUseProgram(UIShader::ColoredRectShader::getInstance()->Program);
    glBindVertexArray(SharedObject::UIVAO);
    UIShader::ColoredRectShader::getInstance()->setUniforms(core::vector2df(center_pos_x, center_pos_y), core::vector2df(width, height), color);

    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
    if (clip)
        glDisable(GL_SCISSOR_TEST);
    glUseProgram(0);

    glGetError();
}

bool hasGLExtension(const char* extension)
{
    if (glGetStringi != NULL)
    {
        GLint numExtensions = 0;
        glGetIntegerv(GL_NUM_EXTENSIONS, &numExtensions);
        for (GLint i = 0; i < numExtensions; i++)
        {
            const char* foundExtension =
                (const char*) glGetStringi(GL_EXTENSIONS, i);
            if (foundExtension && strcmp(foundExtension, extension) == 0)
            {
                return true;
            }
        }
    }
    else
    {
        const char* extensions = (const char*) glGetString(GL_EXTENSIONS);
        if (extensions && strstr(extensions, extension) != NULL)
        {
            return true;
        }
    }
    return false;
}   // hasGLExtension

// ----------------------------------------------------------------------------
/** Returns a space-separated list of all GL extensions. Used for hardware
 *  reporting.
 */
const std::string getGLExtensions()
{
    std::string result;
    if (glGetStringi != NULL)
    {
        GLint num_extensions = 0;
        glGetIntegerv(GL_NUM_EXTENSIONS, &num_extensions);
        for (GLint i = 0; i < num_extensions; i++)
        {
            const char* extension = (const char*)glGetStringi(GL_EXTENSIONS, i);
            if(result.size()>0)
                result += " ";
            result += extension;
        }
    }
    else
    {
        const char* extensions = (const char*) glGetString(GL_EXTENSIONS);
        result = extensions;
    }

    return result;
}   // getGLExtensions

// ----------------------------------------------------------------------------
/** Adds GL limits to the json data structure.
 *  (C) 2014 by Wildfire Games (0 A.D.), ported by Joerg Henrichs
 */
void getGLLimits(HardwareStats::Json *json)
{
    // Various macros to make the data assembly shorter.
#define INTEGER(id)                               \
    do {                                          \
        GLint i = -1;                             \
        glGetIntegerv(GL_##id, &i);               \
        if (glGetError()==GL_NO_ERROR)            \
            json->add("GL_"#id, i);               \
    } while (false)
#define INTEGER2(id)                              \
    do {                                          \
        GLint i[2] = { -1, -1 };                  \
        glGetIntegerv(GL_##id, i);                \
        if (glGetError()==GL_NO_ERROR) {          \
            json->add("GL_"#id"[0]", i[0]);       \
            json->add("GL_"#id"[1]", i[1]);       \
        }                                         \
    } while (false)
#define FLOAT(id)                                 \
    do {                                          \
        GLfloat f = -1.0f;                        \
        glGetFloatv(GL_##id, &f);                 \
        if (glGetError()==GL_NO_ERROR)            \
            json->add("GL_"#id, f);               \
    } while (false)
#define FLOAT2(id)                                \
    do {                                          \
        GLfloat f[2] = {-1.0f, -1.0f};            \
        glGetFloatv(GL_##id,  f);                 \
        if (glGetError()==GL_NO_ERROR)  {         \
            json->add("GL_"#id"[0]", f[0]);       \
            json->add("GL_"#id"[1]", f[1]);       \
        }                                         \
    } while (false)
#define STRING(id)                                         \
    do {                                                   \
        const char* c = (const char*)glGetString(GL_##id); \
        if(!c) c="";                                       \
        json->add("GL_"#id, c);                            \
    } while (false)


    STRING(VERSION);
    STRING(VENDOR);
    STRING(RENDERER);
    STRING(EXTENSIONS);
    INTEGER(SUBPIXEL_BITS);
    INTEGER(MAX_TEXTURE_SIZE);
    INTEGER(MAX_CUBE_MAP_TEXTURE_SIZE);
    INTEGER2(MAX_VIEWPORT_DIMS);
    FLOAT2(ALIASED_POINT_SIZE_RANGE);
    FLOAT2(ALIASED_LINE_WIDTH_RANGE);
    INTEGER(SAMPLE_BUFFERS);
    INTEGER(SAMPLES);
    // TODO: compressed texture formats
    INTEGER(RED_BITS);
    INTEGER(GREEN_BITS);
    INTEGER(BLUE_BITS);
    INTEGER(ALPHA_BITS);
    INTEGER(DEPTH_BITS);
    INTEGER(STENCIL_BITS);

    return;

#ifdef NOT_DONE_YET
#define QUERY(target, pname) do { \
    GLint i = -1; \
    pglGetQueryivARB(GL_##target, GL_##pname, &i); \
    if (ogl_SquelchError(GL_INVALID_ENUM)) \
    scriptInterface.SetProperty(settings, "GL_" #target ".GL_" #pname, errstr); \
else \
    scriptInterface.SetProperty(settings, "GL_" #target ".GL_" #pname, i); \
        } while (false)
#define VERTEXPROGRAM(id) do { \
    GLint i = -1; \
    pglGetProgramivARB(GL_VERTEX_PROGRAM_ARB, GL_##id, &i); \
    if (ogl_SquelchError(GL_INVALID_ENUM)) \
    scriptInterface.SetProperty(settings, "GL_VERTEX_PROGRAM_ARB.GL_" #id, errstr); \
else \
    scriptInterface.SetProperty(settings, "GL_VERTEX_PROGRAM_ARB.GL_" #id, i); \
        } while (false)
#define FRAGMENTPROGRAM(id) do { \
    GLint i = -1; \
    pglGetProgramivARB(GL_FRAGMENT_PROGRAM_ARB, GL_##id, &i); \
    if (ogl_SquelchError(GL_INVALID_ENUM)) \
    scriptInterface.SetProperty(settings, "GL_FRAGMENT_PROGRAM_ARB.GL_" #id, errstr); \
else \
    scriptInterface.SetProperty(settings, "GL_FRAGMENT_PROGRAM_ARB.GL_" #id, i); \
        } while (false)
#define BOOL(id) INTEGER(id)


#if !CONFIG2_GLES
        INTEGER(MAX_LIGHTS);
        INTEGER(MAX_CLIP_PLANES);
        // Skip MAX_COLOR_MATRIX_STACK_DEPTH (only in imaging subset)
        INTEGER(MAX_MODELVIEW_STACK_DEPTH);
        INTEGER(MAX_PROJECTION_STACK_DEPTH);
        INTEGER(MAX_TEXTURE_STACK_DEPTH);
#endif
#if !CONFIG2_GLES
        INTEGER(MAX_3D_TEXTURE_SIZE);
#endif
#if !CONFIG2_GLES
        INTEGER(MAX_PIXEL_MAP_TABLE);
        INTEGER(MAX_NAME_STACK_DEPTH);
        INTEGER(MAX_LIST_NESTING);
        INTEGER(MAX_EVAL_ORDER);
#endif
#if !CONFIG2_GLES
        INTEGER(MAX_ATTRIB_STACK_DEPTH);
        INTEGER(MAX_CLIENT_ATTRIB_STACK_DEPTH);
        INTEGER(AUX_BUFFERS);
        BOOL(RGBA_MODE);
        BOOL(INDEX_MODE);
        BOOL(DOUBLEBUFFER);
        BOOL(STEREO);
#endif
#if !CONFIG2_GLES
        FLOAT2(SMOOTH_POINT_SIZE_RANGE);
        FLOAT(SMOOTH_POINT_SIZE_GRANULARITY);
#endif
#if !CONFIG2_GLES
        FLOAT2(SMOOTH_LINE_WIDTH_RANGE);
        FLOAT(SMOOTH_LINE_WIDTH_GRANULARITY);
        // Skip MAX_CONVOLUTION_WIDTH, MAX_CONVOLUTION_HEIGHT (only in imaging subset)
        INTEGER(MAX_ELEMENTS_INDICES);
        INTEGER(MAX_ELEMENTS_VERTICES);
        INTEGER(MAX_TEXTURE_UNITS);
#endif
#if !CONFIG2_GLES
        INTEGER(INDEX_BITS);
#endif
#if !CONFIG2_GLES
        INTEGER(ACCUM_RED_BITS);
        INTEGER(ACCUM_GREEN_BITS);
        INTEGER(ACCUM_BLUE_BITS);
        INTEGER(ACCUM_ALPHA_BITS);
#endif
#if !CONFIG2_GLES
        // Core OpenGL 2.0 (treated as extensions):
        if (ogl_HaveExtension("GL_EXT_texture_lod_bias"))
        {
            FLOAT(MAX_TEXTURE_LOD_BIAS_EXT);
        }
        if (ogl_HaveExtension("GL_ARB_occlusion_query"))
        {
            QUERY(SAMPLES_PASSED, QUERY_COUNTER_BITS);
        }
        if (ogl_HaveExtension("GL_ARB_shading_language_100"))
        {
            STRING(SHADING_LANGUAGE_VERSION_ARB);
        }
        if (ogl_HaveExtension("GL_ARB_vertex_shader"))
        {
            INTEGER(MAX_VERTEX_ATTRIBS_ARB);
            INTEGER(MAX_VERTEX_UNIFORM_COMPONENTS_ARB);
            INTEGER(MAX_VARYING_FLOATS_ARB);
            INTEGER(MAX_COMBINED_TEXTURE_IMAGE_UNITS_ARB);
            INTEGER(MAX_VERTEX_TEXTURE_IMAGE_UNITS_ARB);
        }
        if (ogl_HaveExtension("GL_ARB_fragment_shader"))
        {
            INTEGER(MAX_FRAGMENT_UNIFORM_COMPONENTS_ARB);
        }
        if (ogl_HaveExtension("GL_ARB_vertex_shader") || ogl_HaveExtension("GL_ARB_fragment_shader") ||
            ogl_HaveExtension("GL_ARB_vertex_program") || ogl_HaveExtension("GL_ARB_fragment_program"))
        {
            INTEGER(MAX_TEXTURE_IMAGE_UNITS_ARB);
            INTEGER(MAX_TEXTURE_COORDS_ARB);
        }
        if (ogl_HaveExtension("GL_ARB_draw_buffers"))
        {
            INTEGER(MAX_DRAW_BUFFERS_ARB);
        }
        // Core OpenGL 3.0:
        if (ogl_HaveExtension("GL_EXT_gpu_shader4"))
        {
            INTEGER(MIN_PROGRAM_TEXEL_OFFSET); // no _EXT version of these in glext.h
            INTEGER(MAX_PROGRAM_TEXEL_OFFSET);
        }
        if (ogl_HaveExtension("GL_EXT_framebuffer_object"))
        {
            INTEGER(MAX_COLOR_ATTACHMENTS_EXT);
            INTEGER(MAX_RENDERBUFFER_SIZE_EXT);
        }
        if (ogl_HaveExtension("GL_EXT_framebuffer_multisample"))
        {
            INTEGER(MAX_SAMPLES_EXT);
        }
        if (ogl_HaveExtension("GL_EXT_texture_array"))
        {
            INTEGER(MAX_ARRAY_TEXTURE_LAYERS_EXT);
        }
        if (ogl_HaveExtension("GL_EXT_transform_feedback"))
        {
            INTEGER(MAX_TRANSFORM_FEEDBACK_INTERLEAVED_COMPONENTS_EXT);
            INTEGER(MAX_TRANSFORM_FEEDBACK_SEPARATE_ATTRIBS_EXT);
            INTEGER(MAX_TRANSFORM_FEEDBACK_SEPARATE_COMPONENTS_EXT);
        }
        // Other interesting extensions:
        if (ogl_HaveExtension("GL_EXT_timer_query") || ogl_HaveExtension("GL_ARB_timer_query"))
        {
            QUERY(TIME_ELAPSED, QUERY_COUNTER_BITS);
        }
        if (ogl_HaveExtension("GL_ARB_timer_query"))
        {
            QUERY(TIMESTAMP, QUERY_COUNTER_BITS);
        }
        if (ogl_HaveExtension("GL_EXT_texture_filter_anisotropic"))
        {
            FLOAT(MAX_TEXTURE_MAX_ANISOTROPY_EXT);
        }
        if (ogl_HaveExtension("GL_ARB_texture_rectangle"))
        {
            INTEGER(MAX_RECTANGLE_TEXTURE_SIZE_ARB);
        }
        if (ogl_HaveExtension("GL_ARB_vertex_program") || ogl_HaveExtension("GL_ARB_fragment_program"))
        {
            INTEGER(MAX_PROGRAM_MATRICES_ARB);
            INTEGER(MAX_PROGRAM_MATRIX_STACK_DEPTH_ARB);
        }
        if (ogl_HaveExtension("GL_ARB_vertex_program"))
        {
            VERTEXPROGRAM(MAX_PROGRAM_ENV_PARAMETERS_ARB);
            VERTEXPROGRAM(MAX_PROGRAM_LOCAL_PARAMETERS_ARB);
            VERTEXPROGRAM(MAX_PROGRAM_INSTRUCTIONS_ARB);
            VERTEXPROGRAM(MAX_PROGRAM_TEMPORARIES_ARB);
            VERTEXPROGRAM(MAX_PROGRAM_PARAMETERS_ARB);
            VERTEXPROGRAM(MAX_PROGRAM_ATTRIBS_ARB);
            VERTEXPROGRAM(MAX_PROGRAM_ADDRESS_REGISTERS_ARB);
            VERTEXPROGRAM(MAX_PROGRAM_NATIVE_INSTRUCTIONS_ARB);
            VERTEXPROGRAM(MAX_PROGRAM_NATIVE_TEMPORARIES_ARB);
            VERTEXPROGRAM(MAX_PROGRAM_NATIVE_PARAMETERS_ARB);
            VERTEXPROGRAM(MAX_PROGRAM_NATIVE_ATTRIBS_ARB);
            VERTEXPROGRAM(MAX_PROGRAM_NATIVE_ADDRESS_REGISTERS_ARB);
            if (ogl_HaveExtension("GL_ARB_fragment_program"))
            {
                // The spec seems to say these should be supported, but
                // Mesa complains about them so let's not bother
                /*
                VERTEXPROGRAM(MAX_PROGRAM_ALU_INSTRUCTIONS_ARB);
                VERTEXPROGRAM(MAX_PROGRAM_TEX_INSTRUCTIONS_ARB);
                VERTEXPROGRAM(MAX_PROGRAM_TEX_INDIRECTIONS_ARB);
                VERTEXPROGRAM(MAX_PROGRAM_NATIVE_ALU_INSTRUCTIONS_ARB);
                VERTEXPROGRAM(MAX_PROGRAM_NATIVE_TEX_INSTRUCTIONS_ARB);
                VERTEXPROGRAM(MAX_PROGRAM_NATIVE_TEX_INDIRECTIONS_ARB);
                */
            }
        }
        if (ogl_HaveExtension("GL_ARB_fragment_program"))
        {
            FRAGMENTPROGRAM(MAX_PROGRAM_ENV_PARAMETERS_ARB);
            FRAGMENTPROGRAM(MAX_PROGRAM_LOCAL_PARAMETERS_ARB);
            FRAGMENTPROGRAM(MAX_PROGRAM_INSTRUCTIONS_ARB);
            FRAGMENTPROGRAM(MAX_PROGRAM_ALU_INSTRUCTIONS_ARB);
            FRAGMENTPROGRAM(MAX_PROGRAM_TEX_INSTRUCTIONS_ARB);
            FRAGMENTPROGRAM(MAX_PROGRAM_TEX_INDIRECTIONS_ARB);
            FRAGMENTPROGRAM(MAX_PROGRAM_TEMPORARIES_ARB);
            FRAGMENTPROGRAM(MAX_PROGRAM_PARAMETERS_ARB);
            FRAGMENTPROGRAM(MAX_PROGRAM_ATTRIBS_ARB);
            FRAGMENTPROGRAM(MAX_PROGRAM_NATIVE_INSTRUCTIONS_ARB);
            FRAGMENTPROGRAM(MAX_PROGRAM_NATIVE_ALU_INSTRUCTIONS_ARB);
            FRAGMENTPROGRAM(MAX_PROGRAM_NATIVE_TEX_INSTRUCTIONS_ARB);
            FRAGMENTPROGRAM(MAX_PROGRAM_NATIVE_TEX_INDIRECTIONS_ARB);
            FRAGMENTPROGRAM(MAX_PROGRAM_NATIVE_TEMPORARIES_ARB);
            FRAGMENTPROGRAM(MAX_PROGRAM_NATIVE_PARAMETERS_ARB);
            FRAGMENTPROGRAM(MAX_PROGRAM_NATIVE_ATTRIBS_ARB);
            if (ogl_HaveExtension("GL_ARB_vertex_program"))
            {
                // The spec seems to say these should be supported, but
                // Intel drivers on Windows complain about them so let's not bother
                /*
                FRAGMENTPROGRAM(MAX_PROGRAM_ADDRESS_REGISTERS_ARB);
                FRAGMENTPROGRAM(MAX_PROGRAM_NATIVE_ADDRESS_REGISTERS_ARB);
                */
            }
        }
        if (ogl_HaveExtension("GL_ARB_geometry_shader4"))
        {
            INTEGER(MAX_GEOMETRY_TEXTURE_IMAGE_UNITS_ARB);
            INTEGER(MAX_GEOMETRY_OUTPUT_VERTICES_ARB);
            INTEGER(MAX_GEOMETRY_TOTAL_OUTPUT_COMPONENTS_ARB);
            INTEGER(MAX_GEOMETRY_UNIFORM_COMPONENTS_ARB);
            INTEGER(MAX_GEOMETRY_VARYING_COMPONENTS_ARB);
            INTEGER(MAX_VERTEX_VARYING_COMPONENTS_ARB);
        }
#else // CONFIG2_GLES
        // Core OpenGL ES 2.0:
        STRING(SHADING_LANGUAGE_VERSION);
        INTEGER(MAX_VERTEX_ATTRIBS);
        INTEGER(MAX_VERTEX_UNIFORM_VECTORS);
        INTEGER(MAX_VARYING_VECTORS);
        INTEGER(MAX_COMBINED_TEXTURE_IMAGE_UNITS);
        INTEGER(MAX_VERTEX_TEXTURE_IMAGE_UNITS);
        INTEGER(MAX_FRAGMENT_UNIFORM_VECTORS);
        INTEGER(MAX_TEXTURE_IMAGE_UNITS);
        INTEGER(MAX_RENDERBUFFER_SIZE);
#endif // CONFIG2_GLES
#ifdef SDL_VIDEO_DRIVER_X11
#define GLXQCR_INTEGER(id) do { \
    unsigned int i = UINT_MAX; \
    if (pglXQueryCurrentRendererIntegerMESA(id, &i)) \
    scriptInterface.SetProperty(settings, #id, i); \
        } while (false)
#define GLXQCR_INTEGER2(id) do { \
    unsigned int i[2] = { UINT_MAX, UINT_MAX }; \
    if (pglXQueryCurrentRendererIntegerMESA(id, i)) { \
    scriptInterface.SetProperty(settings, #id "[0]", i[0]); \
    scriptInterface.SetProperty(settings, #id "[1]", i[1]); \
    } \
        } while (false)
#define GLXQCR_INTEGER3(id) do { \
    unsigned int i[3] = { UINT_MAX, UINT_MAX, UINT_MAX }; \
    if (pglXQueryCurrentRendererIntegerMESA(id, i)) { \
    scriptInterface.SetProperty(settings, #id "[0]", i[0]); \
    scriptInterface.SetProperty(settings, #id "[1]", i[1]); \
    scriptInterface.SetProperty(settings, #id "[2]", i[2]); \
    } \
        } while (false)
#define GLXQCR_STRING(id) do { \
    const char* str = pglXQueryCurrentRendererStringMESA(id); \
    if (str) \
    scriptInterface.SetProperty(settings, #id ".string", str); \
        } while (false)
        SDL_SysWMinfo wminfo;
        SDL_VERSION(&wminfo.version);
        if (SDL_GetWMInfo(&wminfo) && wminfo.subsystem == SDL_SYSWM_X11)
        {
            Display* dpy = wminfo.info.x11.gfxdisplay;
            int scrnum = DefaultScreen(dpy);
            const char* glxexts = glXQueryExtensionsString(dpy, scrnum);
            scriptInterface.SetProperty(settings, "glx_extensions", glxexts);
            if (strstr(glxexts, "GLX_MESA_query_renderer") && pglXQueryCurrentRendererIntegerMESA && pglXQueryCurrentRendererStringMESA)
            {
                GLXQCR_INTEGER(GLX_RENDERER_VENDOR_ID_MESA);
                GLXQCR_INTEGER(GLX_RENDERER_DEVICE_ID_MESA);
                GLXQCR_INTEGER3(GLX_RENDERER_VERSION_MESA);
                GLXQCR_INTEGER(GLX_RENDERER_ACCELERATED_MESA);
                GLXQCR_INTEGER(GLX_RENDERER_VIDEO_MEMORY_MESA);
                GLXQCR_INTEGER(GLX_RENDERER_UNIFIED_MEMORY_ARCHITECTURE_MESA);
                GLXQCR_INTEGER(GLX_RENDERER_PREFERRED_PROFILE_MESA);
                GLXQCR_INTEGER2(GLX_RENDERER_OPENGL_CORE_PROFILE_VERSION_MESA);
                GLXQCR_INTEGER2(GLX_RENDERER_OPENGL_COMPATIBILITY_PROFILE_VERSION_MESA);
                GLXQCR_INTEGER2(GLX_RENDERER_OPENGL_ES_PROFILE_VERSION_MESA);
                GLXQCR_INTEGER2(GLX_RENDERER_OPENGL_ES2_PROFILE_VERSION_MESA);
                GLXQCR_STRING(GLX_RENDERER_VENDOR_ID_MESA);
                GLXQCR_STRING(GLX_RENDERER_DEVICE_ID_MESA);
            }
        }
#endif // SDL_VIDEO_DRIVER_X11

#endif  // ifdef XX
}   // getGLLimits
