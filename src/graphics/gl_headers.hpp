#ifndef GL_HEADER_HPP
#define GL_HEADER_HPP

#define GLEW_STATIC

extern "C" {
#include <GL/glew.h>
}
#include <cinttypes>

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
#    include <windows.h>
#else
#define GL_GLEXT_PROTOTYPES
#define DEBUG_OUTPUT_DECLARED
#    include <GL/gl.h>
#    include <GL/glext.h>
#endif

struct DrawElementsIndirectCommand{
    GLuint count;
    GLuint instanceCount;
    GLuint firstIndex;
    GLuint baseVertex;
    GLuint baseInstance;
};

#endif
