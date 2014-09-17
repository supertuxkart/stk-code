#include "graphics/glwrap.hpp"
#include <fstream>
#include <string>
#include "config/user_config.hpp"
#include "utils/profiler.hpp"
#include "utils/cpp2011.hpp"
#include "graphics/stkmesh.hpp"


#include "../../lib/irrlicht/source/Irrlicht/COpenGLTexture.h"

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
    return;
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
    int length = strlen(SourcePointer);
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
    glViewport(0, 0, width, height);
    irr::video::COpenGLDriver *gl_driver = (irr::video::COpenGLDriver*)irr_driver->getDevice()->getVideoDriver();
    GLenum bufs[] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2 };
    glDrawBuffers(RenderTargets.size(), bufs);
}

void FrameBuffer::Blit(const FrameBuffer &Src, FrameBuffer &Dst, GLbitfield mask, GLenum filter)
{
    glBindFramebuffer(GL_READ_FRAMEBUFFER, Src.fbo);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, Dst.fbo);
    glBlitFramebuffer(0, 0, Src.width, Src.height, 0, 0, Dst.width, Dst.height, mask, filter);
    glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
}

void FrameBuffer::BlitToDefault(size_t x0, size_t y0, size_t x1, size_t y1)
{
    glBindFramebuffer(GL_READ_FRAMEBUFFER, fbo);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
    glBlitFramebuffer(0, 0, width, height, x0, y0, x1, y1, GL_COLOR_BUFFER_BIT, GL_NEAREST);
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

    getSize(texture_w, texture_h, true,
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
