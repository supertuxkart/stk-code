// Copyright (C) 2013      Patryk Nadrowski
// Copyright (C) 2016-2017 Dawid Gan
// Heavily based on the OpenGL driver implemented by Nikolaus Gebhardt
// OpenGL ES driver implemented by Christian Stehno and first OpenGL ES 2.0
// driver implemented by Amundis.
// This file is part of the "Irrlicht Engine".
// For conditions of distribution and use, see copyright notice in Irrlicht.h

#include "IrrCompileConfig.h"

#if defined(_IRR_COMPILE_WITH_EGL_)

#include <cassert>
#include <cstdio>
#include <cstring>
#include <vector>

#ifdef _IRR_COMPILE_WITH_ANDROID_DEVICE_
#include <android_native_app_glue.h>
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

    bool success = initDisplay();

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

    eglSwapInterval(m_egl_display, m_creation_params.vsync_enabled ? 1 : 0);

    m_initialized = true;
    return true;
}


bool ContextManagerEGL::initDisplay()
{
    NativeDisplayType display = (NativeDisplayType)(m_creation_params.display);

#ifdef _IRR_COMPILE_WITH_ANDROID_DEVICE_
    display = EGL_DEFAULT_DISPLAY;
#endif

    if (display != EGL_DEFAULT_DISPLAY)
    {
        m_egl_display = eglGetDisplay(display);
    }

    if (m_egl_display == EGL_NO_DISPLAY)
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
    // config_attribs.push_back(EGL_BUFFER_SIZE);
    // config_attribs.push_back(24);
    config_attribs.push_back(EGL_DEPTH_SIZE);
    config_attribs.push_back(16);
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

    if (!success || num_configs == 0)
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
    if (m_creation_params.surface_type == CEGL_SURFACE_WINDOW)
    {
        if (m_egl_surface == EGL_NO_SURFACE)
        {
            m_egl_surface = eglCreateWindowSurface(m_egl_display, m_egl_config,
                                                   m_egl_window, NULL);
        }

        if (m_egl_surface == EGL_NO_SURFACE)
        {
            m_egl_surface = eglCreateWindowSurface(m_egl_display, m_egl_config,
                                                   0, NULL);
        }
    }
    else if (m_creation_params.surface_type == CEGL_SURFACE_PBUFFER)
    {
        if (m_egl_surface == EGL_NO_SURFACE)
        {
			std::vector<EGLint> pbuffer_attribs;
			pbuffer_attribs.push_back(EGL_WIDTH);
			pbuffer_attribs.push_back(m_creation_params.pbuffer_width);
			pbuffer_attribs.push_back(EGL_HEIGHT);
			pbuffer_attribs.push_back(m_creation_params.pbuffer_height);
			pbuffer_attribs.push_back(EGL_NONE);
			pbuffer_attribs.push_back(0);

			m_egl_surface = eglCreatePbufferSurface(m_egl_display,
													m_egl_config,
													&pbuffer_attribs[0]);
        }

        if (m_egl_surface == EGL_NO_SURFACE)
        {
            std::vector<EGLint> pbuffer_attribs;
			pbuffer_attribs.push_back(EGL_WIDTH);
			pbuffer_attribs.push_back(m_creation_params.pbuffer_width);
			pbuffer_attribs.push_back(EGL_HEIGHT);
			pbuffer_attribs.push_back(m_creation_params.pbuffer_height);
            pbuffer_attribs.push_back(EGL_LARGEST_PBUFFER);
            pbuffer_attribs.push_back(EGL_TRUE);
            pbuffer_attribs.push_back(EGL_NONE);
            pbuffer_attribs.push_back(0);

            m_egl_surface = eglCreatePbufferSurface(m_egl_display,
                                                    m_egl_config,
                                                    &pbuffer_attribs[0]);
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
                context_attribs.push_back(EGL_CONTEXT_MAJOR_VERSION_KHR);
                context_attribs.push_back(4);
                context_attribs.push_back(EGL_CONTEXT_MINOR_VERSION_KHR);
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
                context_attribs.push_back(EGL_CONTEXT_MAJOR_VERSION_KHR);
                context_attribs.push_back(3);
                context_attribs.push_back(EGL_CONTEXT_MINOR_VERSION_KHR);
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
                context_attribs.push_back(EGL_CONTEXT_MAJOR_VERSION_KHR);
                context_attribs.push_back(3);
                context_attribs.push_back(EGL_CONTEXT_MINOR_VERSION_KHR);
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
            context_attribs.push_back(EGL_CONTEXT_MAJOR_VERSION_KHR);
            context_attribs.push_back(2);
            context_attribs.push_back(EGL_CONTEXT_MINOR_VERSION_KHR);
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
    eglMakeCurrent(m_egl_display, EGL_NO_SURFACE, EGL_NO_SURFACE,
                   EGL_NO_CONTEXT);

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


void ContextManagerEGL::reloadEGLSurface(void* window)
{
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
