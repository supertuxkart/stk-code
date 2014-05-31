#ifndef GLWRAP_HEADER_H
#define GLWRAP_HEADER_H

#if defined(__APPLE__)
#    include <OpenGL/gl.h>
#    include <OpenGL/gl3.h>
#    define OGL32CTX
#    ifdef GL_ARB_instanced_arrays
#        define glVertexAttribDivisor glVertexAttribDivisorARB
#    endif
#    ifndef GL_TEXTURE_SWIZZLE_RGBA
#        define GL_TEXTURE_SWIZZLE_RGBA 0x8E46
#    endif
#elif defined(ANDROID)
#    include <GLES/gl.h>
#elif defined(WIN32)
#    define _WINSOCKAPI_
// has to be included before gl.h because of WINGDIAPI and APIENTRY definitions
#    include <windows.h>
#    include <GL/gl.h>
#else
#define GL_GLEXT_PROTOTYPES
#define DEBUG_OUTPUT_DECLARED
#    include <GL/gl.h>
#endif

#include <vector>
#include "utils/log.hpp"

// already includes glext.h, which defines useful GL constants.
// COpenGLDriver has already loaded the extension GL functions we use (e.g glBeginQuery)
#include "../../lib/irrlicht/source/Irrlicht/COpenGLDriver.h"
#ifdef WIN32
extern PFNGLGENTRANSFORMFEEDBACKSPROC glGenTransformFeedbacks;
extern PFNGLBINDTRANSFORMFEEDBACKPROC glBindTransformFeedback;
extern PFNGLDRAWTRANSFORMFEEDBACKPROC glDrawTransformFeedback;
extern PFNGLBEGINTRANSFORMFEEDBACKPROC glBeginTransformFeedback;
extern PFNGLENDTRANSFORMFEEDBACKPROC glEndTransformFeedback;
extern PFNGLTRANSFORMFEEDBACKVARYINGSPROC glTransformFeedbackVaryings;
extern PFNGLBINDBUFFERBASEPROC glBindBufferBase;
extern PFNGLGENBUFFERSPROC glGenBuffers;
extern PFNGLBINDBUFFERPROC glBindBuffer;
extern PFNGLBUFFERDATAPROC glBufferData;
extern PFNGLVERTEXATTRIBPOINTERPROC glVertexAttribPointer;
extern PFNGLCREATESHADERPROC glCreateShader;
extern PFNGLCOMPILESHADERPROC glCompileShader;
extern PFNGLSHADERSOURCEPROC glShaderSource;
extern PFNGLCREATEPROGRAMPROC glCreateProgram;
extern PFNGLATTACHSHADERPROC glAttachShader;
extern PFNGLLINKPROGRAMPROC glLinkProgram;
extern PFNGLUSEPROGRAMPROC glUseProgram;
extern PFNGLENABLEVERTEXATTRIBARRAYPROC glEnableVertexAttribArray;
extern PFNGLDISABLEVERTEXATTRIBARRAYPROC glDisableVertexAttribArray;
extern PFNGLGETUNIFORMLOCATIONPROC glGetUniformLocation;
extern PFNGLUNIFORMMATRIX4FVPROC glUniformMatrix4fv;
extern PFNGLUNIFORM1FPROC glUniform1f;
extern PFNGLUNIFORM3FPROC glUniform3f;
extern PFNGLUNIFORM1FVPROC glUniform1fv;
extern PFNGLUNIFORM4FVPROC glUniform4fv;
extern PFNGLDELETESHADERPROC glDeleteShader;
extern PFNGLGETSHADERIVPROC glGetShaderiv;
extern PFNGLGETSHADERINFOLOGPROC glGetShaderInfoLog;
extern PFNGLACTIVETEXTUREPROC glActiveTexture;
extern PFNGLUNIFORM2FPROC glUniform2f;
extern PFNGLUNIFORM1IPROC glUniform1i;
extern PFNGLUNIFORM3IPROC glUniform3i;
extern PFNGLUNIFORM4IPROC glUniform4i;
extern PFNGLGETPROGRAMIVPROC glGetProgramiv;
extern PFNGLGETPROGRAMINFOLOGPROC glGetProgramInfoLog;
extern PFNGLGETATTRIBLOCATIONPROC glGetAttribLocation;
extern PFNGLBLENDEQUATIONPROC glBlendEquation;
extern PFNGLVERTEXATTRIBDIVISORPROC glVertexAttribDivisor;
extern PFNGLDRAWARRAYSINSTANCEDPROC glDrawArraysInstanced;
extern PFNGLDRAWELEMENTSINSTANCEDPROC glDrawElementsInstanced;
extern PFNGLDELETEBUFFERSPROC glDeleteBuffers;
extern PFNGLGENVERTEXARRAYSPROC glGenVertexArrays;
extern PFNGLBINDVERTEXARRAYPROC glBindVertexArray;
extern PFNGLDELETEVERTEXARRAYSPROC glDeleteVertexArrays;
extern PFNGLTEXBUFFERPROC glTexBuffer;
extern PFNGLBUFFERSUBDATAPROC glBufferSubData;
extern PFNGLVERTEXATTRIBIPOINTERPROC glVertexAttribIPointer;
extern PFNGLGENFRAMEBUFFERSPROC glGenFramebuffers;
extern PFNGLDELETEFRAMEBUFFERSPROC glDeleteFramebuffers;
extern PFNGLBINDFRAMEBUFFERPROC glBindFramebuffer;
extern PFNGLFRAMEBUFFERTEXTURE2DPROC glFramebufferTexture2D;
extern PFNGLFRAMEBUFFERTEXTUREPROC glFramebufferTexture;
extern PFNGLTEXIMAGE3DPROC glTexImage3D;
extern PFNGLGENERATEMIPMAPPROC glGenerateMipmap;
extern PFNGLCHECKFRAMEBUFFERSTATUSPROC glCheckFramebufferStatus;
extern PFNGLTEXIMAGE2DMULTISAMPLEPROC glTexImage2DMultisample;
extern PFNGLBLITFRAMEBUFFERPROC glBlitFramebuffer;
extern PFNGLGETUNIFORMBLOCKINDEXPROC glGetUniformBlockIndex;
extern PFNGLUNIFORMBLOCKBINDINGPROC glUniformBlockBinding;
extern PFNGLBLENDCOLORPROC glBlendColor;
extern PFNGLCOMPRESSEDTEXIMAGE2DPROC glCompressedTexImage2D;
extern PFNGLGETCOMPRESSEDTEXIMAGEPROC glGetCompressedTexImage;
#ifdef DEBUG
extern PFNGLDEBUGMESSAGECALLBACKARBPROC glDebugMessageCallbackARB;
#endif
#endif


void initGL();
GLuint LoadTFBProgram(const char * vertex_file_path, const char **varyings, unsigned varyingscount);
void setTexture(unsigned TextureUnit, GLuint TextureId, GLenum MagFilter, GLenum MinFilter, bool allowAF = false);
GLuint LoadShader(const char * file, unsigned type);

template<typename ... Types>
void loadAndAttach(GLint ProgramID)
{
    return;
}

template<typename ... Types>
void loadAndAttach(GLint ProgramID, GLint ShaderType, const char *filepath, Types ... args)
{
    GLint ShaderID = LoadShader(filepath, ShaderType);
    glAttachShader(ProgramID, ShaderID);
    glDeleteShader(ShaderID);
    loadAndAttach(ProgramID, args...);
}

template<typename ...Types>
void printFileList()
{
    return;
}

template<typename ...Types>
void printFileList(GLint ShaderType, const char *filepath, Types ... args)
{
    Log::error("GLWrapp", filepath);
    printFileList(args...);
}

template<typename ... Types>
GLint LoadProgram(Types ... args)
{
    GLint ProgramID = glCreateProgram();
    loadAndAttach(ProgramID, args...);
    glLinkProgram(ProgramID);

    GLint Result = GL_FALSE;
    int InfoLogLength;
    glGetProgramiv(ProgramID, GL_LINK_STATUS, &Result);
    if (Result == GL_FALSE) {
        Log::error("GLWrapp", "Error when linking these shaders :");
        printFileList(args...);
        glGetProgramiv(ProgramID, GL_INFO_LOG_LENGTH, &InfoLogLength);
        char *ErrorMessage = new char[InfoLogLength];
        glGetProgramInfoLog(ProgramID, InfoLogLength, NULL, ErrorMessage);
        Log::error("GLWrapp", ErrorMessage);
        delete[] ErrorMessage;
    }

    GLenum glErr = glGetError();
    if (glErr != GL_NO_ERROR)
    {
        Log::warn("IrrDriver", "GLWrap : OpenGL error %i\n", glErr);
    }

    return ProgramID;
}

class GPUTimer;

class ScopedGPUTimer
{
public:
    ScopedGPUTimer(GPUTimer &);
    ~ScopedGPUTimer();
};

class GPUTimer
{
    friend class ScopedGPUTimer;
    GLuint query;
    bool initialised;
public:
    GPUTimer();
    unsigned elapsedTimeus();
};

class FrameBuffer
{
private:
    GLuint fbo;
    std::vector<GLuint> RenderTargets;
    GLuint DepthTexture;
    size_t width, height;
public:
    FrameBuffer();
    FrameBuffer(const std::vector <GLuint> &RTTs, size_t w, size_t h, bool layered = false);
    FrameBuffer(const std::vector <GLuint> &RTTs, GLuint DS, size_t w, size_t h, bool layered = false);
    ~FrameBuffer();
    void Bind();
    std::vector<GLuint> &getRTT() { return RenderTargets; }
    GLuint &getDepthTexture() { assert(DepthTexture); return DepthTexture; }
    size_t getWidth() const { return width; }
    size_t getHeight() const { return height; }
    static void Blit(const FrameBuffer &Src, FrameBuffer &Dst, GLbitfield mask = GL_COLOR_BUFFER_BIT, GLenum filter = GL_NEAREST);
    void BlitToDefault(size_t, size_t, size_t, size_t);
};

// core::rect<s32> needs these includes
#include <rect.h>
#include "utils/vec3.hpp"

GLuint getTextureGLuint(irr::video::ITexture *tex);
GLuint getDepthTexture(irr::video::ITexture *tex);
void resetTextureTable();
void compressTexture(irr::video::ITexture *tex, bool srgb, bool premul_alpha = false);
bool loadCompressedTexture(const std::string& compressed_tex);
void saveCompressedTexture(const std::string& compressed_tex);

void draw3DLine(const core::vector3df& start,
    const core::vector3df& end, irr::video::SColor color);

void draw2DImage(const irr::video::ITexture* texture, const irr::core::rect<s32>& destRect,
    const irr::core::rect<s32>& sourceRect, const irr::core::rect<s32>* clipRect,
    const irr::video::SColor &color, bool useAlphaChannelOfTexture);

void draw2DImage(const irr::video::ITexture* texture, const irr::core::rect<s32>& destRect,
	const irr::core::rect<s32>& sourceRect, const irr::core::rect<s32>* clipRect,
	const irr::video::SColor* const colors, bool useAlphaChannelOfTexture);

void GL32_draw2DRectangle(irr::video::SColor color, const irr::core::rect<s32>& position,
	const irr::core::rect<s32>* clip = 0);
#endif
