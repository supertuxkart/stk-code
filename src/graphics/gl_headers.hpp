//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2014-2015 SuperTuxKart-Team
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

#ifndef GL_HEADER_HPP
#define GL_HEADER_HPP

#ifndef SERVER_ONLY

#define GLEW_STATIC

extern "C" {
#if !defined(USE_GLES2)
#   include <GL/glew.h>
#endif
}
#include <cinttypes>

#if defined(__APPLE__)
#    include <OpenGL/gl.h>
#    include <OpenGL/gl3.h>
#    define OGL32CTX
#    ifdef GL_ARB_instanced_arrays
#        ifdef glVertexAttribDivisor
#            undef glVertexAttribDivisor
#        endif
#        define glVertexAttribDivisor glVertexAttribDivisorARB
#    endif
#    ifndef GL_TEXTURE_SWIZZLE_RGBA
#        define GL_TEXTURE_SWIZZLE_RGBA 0x8E46
#    endif
#elif defined(USE_GLES2)
#    define __gl2_h_
#    include <GLES3/gl3.h>
#    include <GLES3/gl3ext.h>
#    include <GLES2/gl2ext.h>
#    define glVertexAttribDivisorARB glVertexAttribDivisor
#elif defined(WIN32)
#    define WIN32_LEAN_AND_MEAN
#    include <windows.h>
#else
#define GL_GLEXT_PROTOTYPES
#define DEBUG_OUTPUT_DECLARED
#    include <GL/gl.h>
#    include <GL/glext.h>
#endif

#if defined(USE_GLES2)
#define GL_BGRA 0x80E1
#define GL_BGR 0x80E0
#define GL_FRAMEBUFFER_COMPLETE_EXT GL_FRAMEBUFFER_COMPLETE

// The glDrawElementsBaseVertex is available only in OpenGL ES 3.2. At this
// stage the 'basevertex' argument is always equal to 0 because features that
// use it are disabled in OpenGL ES renderer. We can simply use glDrawElements
// instead.
inline void glDrawElementsBaseVertex(GLenum mode, GLsizei count, GLenum type,
                                     GLvoid *indices, GLint basevertex)
{
    glDrawElements(mode, count, type, indices);
}
#endif

struct DrawElementsIndirectCommand{
    GLuint count;
    GLuint instanceCount;
    GLuint firstIndex;
    GLuint baseVertex;
    GLuint baseInstance;
};
#else
  typedef unsigned int GLuint;
  typedef unsigned int GLsync;
  typedef unsigned int GLenum;

#endif   // server only

#endif

