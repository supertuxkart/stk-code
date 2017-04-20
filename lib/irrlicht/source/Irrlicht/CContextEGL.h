// Copyright (C) 2013      Patryk Nadrowski
// Copyright (C) 2016-2017 Dawid Gan
// Heavily based on the OpenGL driver implemented by Nikolaus Gebhardt
// OpenGL ES driver implemented by Christian Stehno and first OpenGL ES 2.0
// driver implemented by Amundis.
// This file is part of the "Irrlicht Engine".
// For conditions of distribution and use, see copyright notice in Irrlicht.h

#ifndef CONTEXT_EGL_HPP
#define CONTEXT_EGL_HPP

#include "IrrCompileConfig.h"

#if defined(_IRR_COMPILE_WITH_EGL_)

#include <EGL/egl.h>
#include <EGL/eglext.h>

enum ContextEGLOpenGLAPI
{
    CEGL_API_OPENGL,
    CEGL_API_OPENGL_ES
};

enum ContextEGLSurfaceType
{
    CEGL_SURFACE_WINDOW,
    CEGL_SURFACE_PBUFFER
};

struct ContextEGLParams
{
    ContextEGLOpenGLAPI opengl_api;
    ContextEGLSurfaceType surface_type;
    EGLNativeWindowType window;
    EGLNativeDisplayType display;
    bool force_legacy_device;
    bool with_alpha_channel;
    bool vsync_enabled;
    int pbuffer_width;
    int pbuffer_height;
};


class ContextManagerEGL
{
private:
    NativeWindowType m_egl_window;
    EGLDisplay m_egl_display;
    EGLSurface m_egl_surface;
    EGLContext m_egl_context;
    EGLConfig m_egl_config;

    ContextEGLParams m_creation_params;
    bool m_is_legacy_device;
    bool m_initialized;
    int m_egl_version;

    bool initDisplay();
    bool chooseConfig();
    bool createSurface();
    bool createContext();
    bool hasEGLExtension(const char* extension);
    bool checkEGLError();

public:
    ContextManagerEGL();
    ~ContextManagerEGL();

    bool init(const ContextEGLParams& params);
    void close();

    void reloadEGLSurface(void* window);
    bool swapBuffers();
    bool isLegacyDevice() {return m_is_legacy_device;}
    bool getSurfaceDimensions(int* width, int* height);
};

#endif

#endif
