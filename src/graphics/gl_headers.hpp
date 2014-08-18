#ifndef GL_HEADER_HPP
#define GL_HEADER_HPP

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
#    include "../../lib/irrlicht/source/Irrlicht/glext.h"
#else
#define GL_GLEXT_PROTOTYPES
#define DEBUG_OUTPUT_DECLARED
#    include <GL/gl.h>
#    include <GL/glext.h>
#endif

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
extern PFNGLDELETEPROGRAMPROC glDeleteProgram;
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
extern PFNGLBINDATTRIBLOCATIONPROC glBindAttribLocation;
extern PFNGLBLENDEQUATIONPROC glBlendEquation;
extern PFNGLVERTEXATTRIBDIVISORPROC glVertexAttribDivisor;
extern PFNGLDRAWARRAYSINSTANCEDPROC glDrawArraysInstanced;
extern PFNGLDRAWELEMENTSBASEVERTEXPROC glDrawElementsBaseVertex;
extern PFNGLDRAWELEMENTSINSTANCEDPROC glDrawElementsInstanced;
extern PFNGLDRAWELEMENTSINSTANCEDBASEVERTEXPROC glDrawElementsInstancedBaseVertex;
extern PFNGLDRAWELEMENTSINSTANCEDBASEVERTEXBASEINSTANCEPROC glDrawElementsInstancedBaseVertexBaseInstance;
extern PFNGLMULTIDRAWELEMENTSINDIRECTPROC glMultiDrawElementsIndirect;
extern PFNGLDELETEBUFFERSPROC glDeleteBuffers;
extern PFNGLGENVERTEXARRAYSPROC glGenVertexArrays;
extern PFNGLBINDVERTEXARRAYPROC glBindVertexArray;
extern PFNGLDELETEVERTEXARRAYSPROC glDeleteVertexArrays;
extern PFNGLTEXBUFFERPROC glTexBuffer;
extern PFNGLBUFFERSUBDATAPROC glBufferSubData;
extern PFNGLMAPBUFFERPROC glMapBuffer;
extern PFNGLMAPBUFFERRANGEPROC glMapBufferRange;
extern PFNGLFLUSHMAPPEDBUFFERRANGEPROC glFlushMappedBufferRange;
extern PFNGLMEMORYBARRIERPROC glMemoryBarrier;
extern PFNGLBUFFERSTORAGEPROC glBufferStorage;
extern PFNGLUNMAPBUFFERPROC glUnmapBuffer;
extern PFNGLFENCESYNCPROC glFenceSync;
extern PFNGLCLIENTWAITSYNCPROC glClientWaitSync;
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
extern PFNGLTEXSTORAGE1DPROC glTexStorage1D;
extern PFNGLTEXSTORAGE2DPROC glTexStorage2D;
extern PFNGLTEXSTORAGE3DPROC glTexStorage3D;
extern PFNGLBINDIMAGETEXTUREPROC glBindImageTexture;
extern PFNGLDISPATCHCOMPUTEPROC glDispatchCompute;
extern PFNGLGENSAMPLERSPROC glGenSamplers;
extern PFNGLDELETESAMPLERSPROC glDeleteSamplers;
extern PFNGLBINDSAMPLERPROC glBindSampler;
extern PFNGLSAMPLERPARAMETERFPROC glSamplerParameterf;
extern PFNGLSAMPLERPARAMETERIPROC glSamplerParameteri;
extern PFNGLGETTEXTURESAMPLERHANDLEARBPROC glGetTextureSamplerHandleARB;
extern PFNGLMAKETEXTUREHANDLERESIDENTARBPROC glMakeTextureHandleResidentARB;
extern PFNGLMAKETEXTUREHANDLENONRESIDENTARBPROC glMakeTextureHandleNonResidentARB;
extern PFNGLUNIFORMHANDLEUI64ARBPROC glUniformHandleui64ARB;
extern PFNGLISTEXTUREHANDLERESIDENTARBPROC glIsTextureHandleResidentARB;
extern PFNGLVERTEXATTRIBLPOINTERPROC glVertexAttribLPointer;
#ifdef DEBUG
extern PFNGLDEBUGMESSAGECALLBACKARBPROC glDebugMessageCallbackARB;
#endif
#endif

#endif