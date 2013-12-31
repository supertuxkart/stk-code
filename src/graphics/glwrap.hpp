#ifndef GLWRAP_HEADER_H
#define GLWRAP_HEADER_H

#if defined(__APPLE__)
#    include <OpenGL/gl.h>
#    include <OpenGL/gl3.h>
#elif defined(ANDROID)
#    include <GLES/gl.h>
#elif defined(WIN32)
#    define _WINSOCKAPI_
// has to be included before gl.h because of WINGDIAPI and APIENTRY definitions
#    include <windows.h>
#    include <GL/gl.h>
#else
#    include <GL/gl.h>
#endif

// already includes glext.h, which defines useful GL constants.
// COpenGLDriver has already loaded the extension GL functions we use (e.g glBeginQuery)
#include "../../lib/irrlicht/source/Irrlicht/COpenGLDriver.h"

#endif
