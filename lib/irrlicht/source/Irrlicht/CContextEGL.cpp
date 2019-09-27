//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2016-2017 SuperTuxKart-Team
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

#include "IrrCompileConfig.h"

#if defined(_IRR_COMPILE_WITH_EGL_)

#include <cassert>
#include <cstdio>
#include <cstring>
#include <vector>

#ifdef _IRR_COMPILE_WITH_ANDROID_DEVICE_
#include "stk_android_native_app_glue.h"
#endif

#include "CContextEGL.h"
#include "os.h"

using namespace irr;

ContextManagerEGL::ContextManagerEGL()
{
    m_egl_window = 0;
    m_egl_display = EGL_NO_DISPLAY;
    m_egl_surface = EGL_NO_SURFACE;
    m_egl_context = EGL_NO_CONTEXT;
    m_egl_config = 0;
    m_egl_version = 0;
    m_is_legacy_device = false;
    m_initialized = false;
    eglGetPlatformDisplay = NULL;

    memset(&m_creation_params, 0, sizeof(ContextEGLParams));
}


ContextManagerEGL::~ContextManagerEGL()
{
    close();
}


bool ContextManagerEGL::init(const ContextEGLParams& params)
{
    if (m_initialized)
        return false;

    m_creation_params = params;
    m_egl_window = m_creation_params.window;
    
    bool success = initExtensions();
    
    if (!success)
    {
        os::Printer::log("Error: Could not initialize EGL extensions.\n");
        close();
        return false;
    }

    success = initDisplay();

    if (!success)
    {
        os::Printer::log("Error: Could not initialize EGL display.\n");
        close();
        return false;
    }

    bool has_minimum_requirements = false;

    if (m_creation_params.opengl_api == CEGL_API_OPENGL)
    {
        if (hasEGLExtension("EGL_KHR_create_context") || m_egl_version >= 150)
        {
            has_minimum_requirements = true;
            eglBindAPI(EGL_OPENGL_API);
        }
    }
    else if (m_creation_params.opengl_api == CEGL_API_OPENGL_ES)
    {
        if (m_egl_version >= 130)
        {
            has_minimum_requirements = true;
            eglBindAPI(EGL_OPENGL_ES_API);
        }
    }

    if (!has_minimum_requirements)
    {
        os::Printer::log("Error: EGL version is too old.\n");
        close();
        return false;
    }

    success = chooseConfig();

    if (!success)
    {
        os::Printer::log("Error: Couldn't get EGL config.\n");
        close();
        return false;
    }

    success = createSurface();

    if (!success)
    {
        os::Printer::log("Error: Couldn't create EGL surface.\n");
        close();
        return false;
    }

    success = createContext();

    if (!success)
    {
        os::Printer::log("Error: Couldn't create OpenGL context.\n");
        close();
        return false;
    }

    success = eglMakeCurrent(m_egl_display, m_egl_surface, m_egl_surface,
                             m_egl_context);

    if (!success)
    {
        checkEGLError();
        os::Printer::log("Error: Couldn't make context current for EGL display.\n");
        close();
        return false;
    }

    eglSwapInterval(m_egl_display, m_creation_params.swap_interval);

    m_initialized = true;
    return true;
}


bool ContextManagerEGL::initExtensions()
{
    if (hasEGLExtension("EGL_KHR_platform_base"))
    {
        eglGetPlatformDisplay = (eglGetPlatformDisplay_t)
                                     eglGetProcAddress("eglGetPlatformDisplay");
    }
    else if (hasEGLExtension("EGL_EXT_platform_base"))
    {
        eglGetPlatformDisplay = (eglGetPlatformDisplay_t)
                                  eglGetProcAddress("eglGetPlatformDisplayEXT");
    }
    
    return true;
}


bool ContextManagerEGL::initDisplay()
{
    EGLNativeDisplayType display = m_creation_params.display;

#ifdef _IRR_COMPILE_WITH_ANDROID_DEVICE_
    display = EGL_DEFAULT_DISPLAY;
#endif

    EGLenum platform = 0;
    
    switch (m_creation_params.platform)
    {
    case CEGL_PLATFORM_ANDROID:
        platform = EGL_PLATFORM_ANDROID;
        break;
    case CEGL_PLATFORM_GBM:
        platform = EGL_PLATFORM_GBM;
        break;
    case CEGL_PLATFORM_WAYLAND:
        platform = EGL_PLATFORM_WAYLAND;
        break;
    case CEGL_PLATFORM_X11:
        platform = EGL_PLATFORM_X11;
        break;
    case CEGL_PLATFORM_DEFAULT:
        break;
    }
    
    if (m_creation_params.platform != CEGL_PLATFORM_DEFAULT &&
        eglGetPlatformDisplay != NULL)
    {
        m_egl_display = eglGetPlatformDisplay(platform, (void*)display, NULL);
    }

    if (m_egl_display == EGL_NO_DISPLAY)
    {
        m_egl_display = eglGetDisplay(display);
    }

    if (m_egl_display == EGL_NO_DISPLAY && display != EGL_DEFAULT_DISPLAY)
    {
        m_egl_display = eglGetDisplay(EGL_DEFAULT_DISPLAY);
    }

    if (m_egl_display == EGL_NO_DISPLAY)
    {
        return false;
    }

    int egl_version_major = 0;
    int egl_version_minor = 0;

    bool success = eglInitialize(m_egl_display, &egl_version_major,
                                                &egl_version_minor);

    if (success)
    {
        m_egl_version = 100 * egl_version_major + 10 * egl_version_minor;
    }

    return success;
}


bool ContextManagerEGL::chooseConfig()
{
    std::vector<EGLint> config_attribs;
    config_attribs.push_back(EGL_RED_SIZE);
    config_attribs.push_back(8);
    config_attribs.push_back(EGL_GREEN_SIZE);
    config_attribs.push_back(8);
    config_attribs.push_back(EGL_BLUE_SIZE);
    config_attribs.push_back(8);
    config_attribs.push_back(EGL_ALPHA_SIZE);
    config_attribs.push_back(m_creation_params.with_alpha_channel ? 8 : 0);
    config_attribs.push_back(EGL_DEPTH_SIZE);
    config_attribs.push_back(16);
    // config_attribs.push_back(EGL_BUFFER_SIZE);
    // config_attribs.push_back(24);
    // config_attribs.push_back(EGL_STENCIL_SIZE);
    // config_attribs.push_back(stencil_buffer);
    // config_attribs.push_back(EGL_SAMPLE_BUFFERS);
    // config_attribs.push_back(antialias ? 1 : 0);
    // config_attribs.push_back(EGL_SAMPLES);
    // config_attribs.push_back(antialias);

    if (m_creation_params.opengl_api == CEGL_API_OPENGL)
    {
        config_attribs.push_back(EGL_RENDERABLE_TYPE);
        config_attribs.push_back(EGL_OPENGL_BIT);
    }
    else if (m_creation_params.opengl_api == CEGL_API_OPENGL_ES)
    {
        config_attribs.push_back(EGL_RENDERABLE_TYPE);
        config_attribs.push_back(EGL_OPENGL_ES2_BIT);
    }

    if (m_creation_params.surface_type == CEGL_SURFACE_WINDOW)
    {
        config_attribs.push_back(EGL_SURFACE_TYPE);
        config_attribs.push_back(EGL_WINDOW_BIT);
    }
    else if (m_creation_params.surface_type == CEGL_SURFACE_PBUFFER)
    {
        config_attribs.push_back(EGL_SURFACE_TYPE);
        config_attribs.push_back(EGL_PBUFFER_BIT);
    }

    config_attribs.push_back(EGL_NONE);
    config_attribs.push_back(0);

    EGLint num_configs = 0;

    bool success = eglChooseConfig(m_egl_display, &config_attribs[0],
                                   &m_egl_config, 1, &num_configs);

    if (!success || m_egl_config == NULL || num_configs < 1)
    {
        config_attribs[1] = 5; //EGL_RED_SIZE
        config_attribs[3] = 6; //EGL_GREEN_SIZE
        config_attribs[5] = 5; //EGL_BLUE_SIZE
        config_attribs[7] = 0; //EGL_ALPHA_SIZE
        config_attribs[9] = 1; //EGL_DEPTH_SIZE
        
        success = eglChooseConfig(m_egl_display, &config_attribs[0],
                                  &m_egl_config, 1, &num_configs);
    }

    if (!success || m_egl_config == NULL || num_configs < 1)
    {
        return false;
    }

#ifdef _IRR_COMPILE_WITH_ANDROID_DEVICE_
    EGLint format = 0;
    eglGetConfigAttrib(m_egl_display, m_egl_config, EGL_NATIVE_VISUAL_ID,
                       &format);
    ANativeWindow_setBuffersGeometry(m_egl_window, 0, 0, format);
#endif

    return true;
}


bool ContextManagerEGL::createSurface()
{
    unsigned int colorspace_attr_pos = 0;
    unsigned int largest_pbuffer_attr_pos = 0;
    
    std::vector<EGLint> attribs;

    if (m_creation_params.opengl_api == CEGL_API_OPENGL &&
        m_creation_params.handle_srgb == true)
    {
        if (hasEGLExtension("EGL_KHR_gl_colorspace") || m_egl_version >= 150)
        {
            attribs.push_back(EGL_GL_COLORSPACE);
            attribs.push_back(EGL_GL_COLORSPACE_SRGB);
            colorspace_attr_pos = attribs.size() - 1;
        }
    }
    
    if (m_creation_params.surface_type == CEGL_SURFACE_PBUFFER)
    {
        attribs.push_back(EGL_WIDTH);
        attribs.push_back(m_creation_params.pbuffer_width);
        attribs.push_back(EGL_HEIGHT);
        attribs.push_back(m_creation_params.pbuffer_height);
        attribs.push_back(EGL_LARGEST_PBUFFER);
        attribs.push_back(EGL_FALSE);
        largest_pbuffer_attr_pos = attribs.size() - 1;
    }
    
    attribs.push_back(EGL_NONE);
    attribs.push_back(0);
    
    if (m_egl_surface == EGL_NO_SURFACE)
    {
        if (m_creation_params.surface_type == CEGL_SURFACE_WINDOW)
        {
            m_egl_surface = eglCreateWindowSurface(m_egl_display, m_egl_config,
                                                   m_egl_window, &attribs[0]);
        }
        else if (m_creation_params.surface_type == CEGL_SURFACE_PBUFFER)
        {
            m_egl_surface = eglCreatePbufferSurface(m_egl_display, m_egl_config,
                                                    &attribs[0]);
        }
    }
        
    if (m_egl_surface == EGL_NO_SURFACE && colorspace_attr_pos > 0)
    {
        attribs[colorspace_attr_pos] = EGL_GL_COLORSPACE_LINEAR;
        
        if (m_creation_params.surface_type == CEGL_SURFACE_WINDOW)
        {
            m_egl_surface = eglCreateWindowSurface(m_egl_display, m_egl_config,
                                                   m_egl_window, &attribs[0]);
        }
        else if (m_creation_params.surface_type == CEGL_SURFACE_PBUFFER)
        {
            m_egl_surface = eglCreatePbufferSurface(m_egl_display, m_egl_config,
                                                    &attribs[0]);
        }
    }
    
    if (m_egl_surface == EGL_NO_SURFACE && largest_pbuffer_attr_pos > 0)
    {
        attribs[largest_pbuffer_attr_pos] = EGL_TRUE;
        
        if (m_creation_params.surface_type == CEGL_SURFACE_PBUFFER)
        {
            m_egl_surface = eglCreatePbufferSurface(m_egl_display, m_egl_config,
                                                    &attribs[0]);
        }
    }

    return m_egl_surface != EGL_NO_SURFACE;
}


bool ContextManagerEGL::createContext()
{
    m_is_legacy_device = false;

    if (m_creation_params.opengl_api == CEGL_API_OPENGL_ES)
    {
        if (!m_creation_params.force_legacy_device)
        {
            if (m_egl_context == EGL_NO_CONTEXT)
            {
                std::vector<EGLint> context_attribs;
                context_attribs.push_back(EGL_CONTEXT_CLIENT_VERSION);
                context_attribs.push_back(3);
                context_attribs.push_back(EGL_NONE);
                context_attribs.push_back(0);

                m_egl_context = eglCreateContext(m_egl_display,
                                                 m_egl_config,
                                                 EGL_NO_CONTEXT,
                                                 &context_attribs[0]);
            }
        }

        if (m_egl_context == EGL_NO_CONTEXT)
        {
            m_is_legacy_device = true;

            std::vector<EGLint> context_attribs;
            context_attribs.push_back(EGL_CONTEXT_CLIENT_VERSION);
            context_attribs.push_back(2);
            context_attribs.push_back(EGL_NONE);
            context_attribs.push_back(0);

            m_egl_context = eglCreateContext(m_egl_display,
                                             m_egl_config,
                                             EGL_NO_CONTEXT,
                                             &context_attribs[0]);
        }
    }
    else if (m_creation_params.opengl_api == CEGL_API_OPENGL)
    {
        if (!m_creation_params.force_legacy_device)
        {
            if (m_egl_context == EGL_NO_CONTEXT)
            {
                std::vector<EGLint> context_attribs;
                context_attribs.push_back(EGL_CONTEXT_MAJOR_VERSION);
                context_attribs.push_back(4);
                context_attribs.push_back(EGL_CONTEXT_MINOR_VERSION);
                context_attribs.push_back(3);
                context_attribs.push_back(EGL_NONE);
                context_attribs.push_back(0);

                m_egl_context = eglCreateContext(m_egl_display,
                                                 m_egl_config,
                                                 EGL_NO_CONTEXT,
                                                 &context_attribs[0]);
            }

            if (m_egl_context == EGL_NO_CONTEXT)
            {
                std::vector<EGLint> context_attribs;
                context_attribs.push_back(EGL_CONTEXT_MAJOR_VERSION);
                context_attribs.push_back(3);
                context_attribs.push_back(EGL_CONTEXT_MINOR_VERSION);
                context_attribs.push_back(3);
                context_attribs.push_back(EGL_NONE);
                context_attribs.push_back(0);

                m_egl_context = eglCreateContext(m_egl_display,
                                                 m_egl_config,
                                                 EGL_NO_CONTEXT,
                                                 &context_attribs[0]);
            }

            if (m_egl_context == EGL_NO_CONTEXT)
            {
                std::vector<EGLint> context_attribs;
                context_attribs.push_back(EGL_CONTEXT_MAJOR_VERSION);
                context_attribs.push_back(3);
                context_attribs.push_back(EGL_CONTEXT_MINOR_VERSION);
                context_attribs.push_back(1);
                context_attribs.push_back(EGL_NONE);
                context_attribs.push_back(0);

                m_egl_context = eglCreateContext(m_egl_display,
                                                 m_egl_config,
                                                 EGL_NO_CONTEXT,
                                                 &context_attribs[0]);
            }
        }

        if (m_egl_context == EGL_NO_CONTEXT)
        {
            m_is_legacy_device = true;

            std::vector<EGLint> context_attribs;
            context_attribs.push_back(EGL_CONTEXT_MAJOR_VERSION);
            context_attribs.push_back(2);
            context_attribs.push_back(EGL_CONTEXT_MINOR_VERSION);
            context_attribs.push_back(1);
            context_attribs.push_back(EGL_NONE);
            context_attribs.push_back(0);

            m_egl_context = eglCreateContext(m_egl_display,
                                             m_egl_config,
                                             EGL_NO_CONTEXT,
                                             &context_attribs[0]);
        }
    }

    return m_egl_context != EGL_NO_CONTEXT;
}


void ContextManagerEGL::close()
{
    if (m_egl_display != EGL_NO_DISPLAY)
    {
        eglMakeCurrent(m_egl_display, EGL_NO_SURFACE, EGL_NO_SURFACE,
                       EGL_NO_CONTEXT);
    }

    if (m_egl_context != EGL_NO_CONTEXT)
    {
        eglDestroyContext(m_egl_display, m_egl_context);
        m_egl_context = EGL_NO_CONTEXT;
    }

    if (m_egl_surface != EGL_NO_SURFACE)
    {
        eglDestroySurface(m_egl_display, m_egl_surface);
        m_egl_surface = EGL_NO_SURFACE;
    }

    if (m_egl_display != EGL_NO_DISPLAY)
    {
        eglTerminate(m_egl_display);
        m_egl_display = EGL_NO_DISPLAY;
    }
    
    eglReleaseThread();
}


bool ContextManagerEGL::swapBuffers()
{
    bool success = eglSwapBuffers(m_egl_display, m_egl_surface);

#ifdef DEBUG
    if (!success)
    {
        eglGetError();
    }
#endif

    return success;
}


bool ContextManagerEGL::makeCurrent()
{
    bool success = eglMakeCurrent(m_egl_display, m_egl_surface, m_egl_surface,
                             m_egl_context);

    return success;
}


void ContextManagerEGL::reloadEGLSurface(void* window)
{
    if (!m_initialized)
        return;

#ifdef _IRR_COMPILE_WITH_ANDROID_DEVICE_
    m_egl_window = (ANativeWindow*)window;

    if (!m_egl_window)
    {
        os::Printer::log("Error: Invalid EGL window.\n");
    }
#endif

    eglMakeCurrent(m_egl_display, EGL_NO_SURFACE, EGL_NO_SURFACE,
                   EGL_NO_CONTEXT);

    eglDestroySurface(m_egl_display, m_egl_surface);
    m_egl_surface = EGL_NO_SURFACE;

    bool success = createSurface();

    if (!success)
    {
        os::Printer::log("Error: Could not create EGL surface.");
    }

    success = eglMakeCurrent(m_egl_display, m_egl_surface, m_egl_surface,
                             m_egl_context);

    if (!success)
    {
        checkEGLError();
        os::Printer::log("Error: Couldn't make context current for EGL display.\n");
    }
}


bool ContextManagerEGL::getSurfaceDimensions(int* width, int* height)
{
    if (!eglQuerySurface(m_egl_display, m_egl_surface, EGL_WIDTH, width))
        return false;

    if (!eglQuerySurface(m_egl_display, m_egl_surface, EGL_HEIGHT, height))
        return false;

    return true;
}


bool ContextManagerEGL::hasEGLExtension(const char* extension)
{
    const char* extensions = eglQueryString(m_egl_display, EGL_EXTENSIONS);

    if (extensions && strstr(extensions, extension) != NULL)
    {
        return true;
    }

    return false;
}


bool ContextManagerEGL::checkEGLError()
{
    bool result = true;

    switch (eglGetError())
    {
        case EGL_SUCCESS:
            result = false;
            break;
        case EGL_NOT_INITIALIZED:
            os::Printer::log("Error: EGL_NOT_INITIALIZED\n");
            break;
        case EGL_BAD_ACCESS:
            os::Printer::log("Error: EGL_BAD_ACCESS\n");
            break;
        case EGL_BAD_ALLOC:
            os::Printer::log("Error: EGL_BAD_ALLOC\n");
            break;
        case EGL_BAD_ATTRIBUTE:
            os::Printer::log("Error: EGL_BAD_ATTRIBUTE\n");
            break;
        case EGL_BAD_CONTEXT:
            os::Printer::log("Error: EGL_BAD_CONTEXT\n");
            break;
        case EGL_BAD_CONFIG:
            os::Printer::log("Error: EGL_BAD_CONFIG\n");
            break;
        case EGL_BAD_CURRENT_SURFACE:
            os::Printer::log("Error: EGL_BAD_CURRENT_SURFACE\n");
            break;
        case EGL_BAD_DISPLAY:
            os::Printer::log("Error: EGL_BAD_DISPLAY\n");
            break;
        case EGL_BAD_SURFACE:
            os::Printer::log("Error: EGL_BAD_SURFACE\n");
            break;
        case EGL_BAD_MATCH:
            os::Printer::log("Error: EGL_BAD_MATCH\n");
            break;
        case EGL_BAD_PARAMETER:
            os::Printer::log("Error: EGL_BAD_PARAMETER\n");
            break;
        case EGL_BAD_NATIVE_PIXMAP:
            os::Printer::log("Error: EGL_BAD_NATIVE_PIXMAP\n");
            break;
        case EGL_BAD_NATIVE_WINDOW:
            os::Printer::log("Error: EGL_BAD_NATIVE_WINDOW\n");
            break;
        case EGL_CONTEXT_LOST:
            os::Printer::log("Error: EGL_CONTEXT_LOST\n");
            break;
        default:
            os::Printer::log("Error: Unknown EGL error.\n");
            break;
    };

    return result;
}

#endif
