// Copyright (C) 2013      Patryk Nadrowski
// Copyright (C) 2016-2017 Dawid Gan
// Heavily based on the OpenGL driver implemented by Nikolaus Gebhardt
// OpenGL ES driver implemented by Christian Stehno and first OpenGL ES 2.0
// driver implemented by Amundis.
// This file is part of the "Irrlicht Engine".
// For conditions of distribution and use, see copyright notice in Irrlicht.h

#include "CContextEGL.h"
#include "os.h"
#include "fast_atof.h"

#if defined(_IRR_COMPILE_WITH_EGL_)

namespace irr
{
namespace video
{
	
ContextEGL::ContextEGL(const SIrrlichtCreationParameters& params,
					   const SExposedVideoData& data)
{
	EglDisplay = EGL_NO_DISPLAY;
	IsCoreContext = true;
	
#if defined(_IRR_COMPILE_WITH_WINDOWS_DEVICE_)
	HDc = 0;
#endif
	
#if defined(_IRR_COMPILE_WITH_WINDOWS_DEVICE_)
		EglWindow = (NativeWindowType)data.OpenGLWin32.HWnd;
		HDc = GetDC((HWND)EglWindow);
		EglDisplay = eglGetDisplay((NativeDisplayType)HDc);
#elif defined(_IRR_COMPILE_WITH_X11_DEVICE_)
		EglWindow = (NativeWindowType)data.OpenGLLinux.X11Window;
		EglDisplay = eglGetDisplay((NativeDisplayType)data.OpenGLLinux.X11Display);
#elif defined(_IRR_COMPILE_WITH_ANDROID_DEVICE_)
		EglWindow =	((struct android_app *)(params.PrivateData))->window;
		EglDisplay = EGL_NO_DISPLAY;
#endif

		if (EglDisplay == EGL_NO_DISPLAY)
		{
			os::Printer::log("Getting OpenGL-ES2 display.");
			EglDisplay = eglGetDisplay((NativeDisplayType) EGL_DEFAULT_DISPLAY);
		}
		if (EglDisplay == EGL_NO_DISPLAY)
		{
			os::Printer::log("Could not get OpenGL-ES2 display.");
		}

		EGLint majorVersion, minorVersion;
		if (!eglInitialize(EglDisplay, &majorVersion, &minorVersion))
		{
			os::Printer::log("Could not initialize OpenGL-ES2 display.");
		}
		else
		{
			char text[64];
			sprintf(text, "EglDisplay initialized. Egl version %d.%d\n", majorVersion, minorVersion);
			os::Printer::log(text);
		}
		
	EGLint EglOpenGLBIT = 0;

	// We need proper OpenGL BIT.
	switch (params.DriverType)
	{
	case EDT_OGLES2:
		EglOpenGLBIT = EGL_OPENGL_ES2_BIT;
		break;
	default:
		break;
	}

	EGLint Attribs[] =
	{
		EGL_RED_SIZE, 8,
		EGL_GREEN_SIZE, 8,
		EGL_BLUE_SIZE, 8,
		EGL_ALPHA_SIZE, params.WithAlphaChannel ? 1:0,
		EGL_BUFFER_SIZE, params.Bits,
		EGL_SURFACE_TYPE, EGL_WINDOW_BIT,
		EGL_DEPTH_SIZE, params.ZBufferBits,
		EGL_STENCIL_SIZE, params.Stencilbuffer,
		EGL_SAMPLE_BUFFERS, params.AntiAlias ? 1:0,
		EGL_SAMPLES, params.AntiAlias,
#ifdef EGL_VERSION_1_3
		EGL_RENDERABLE_TYPE, EglOpenGLBIT,
#endif
		EGL_NONE, 0
	};

	EglConfig = 0;
	EGLint NumConfigs = 0;
	u32 Steps = 5;

	// Choose the best EGL config.
	while (!eglChooseConfig(EglDisplay, Attribs, &EglConfig, 1, &NumConfigs) || !NumConfigs)
	{
		switch (Steps)
		{
		case 5: // samples
			if (Attribs[19] > 2)
				--Attribs[19];
			else
			{
				Attribs[17] = 0;
				Attribs[19] = 0;
				--Steps;
			}
			break;
		case 4: // alpha
			if (Attribs[7])
			{
				Attribs[7] = 0;

				if (params.AntiAlias)
				{
					Attribs[17] = 1;
					Attribs[19] = params.AntiAlias;
					Steps = 5;
				}
			}
			else
				--Steps;
			break;
		case 3: // stencil
			if (Attribs[15])
			{
				Attribs[15] = 0;

				if (params.AntiAlias)
				{
					Attribs[17] = 1;
					Attribs[19] = params.AntiAlias;
					Steps = 5;
				}
			}
			else
				--Steps;
			break;
		case 2: // depth size
			if (Attribs[13] > 16)
			{
				Attribs[13] -= 8;
			}
			else
				--Steps;
			break;
		case 1: // buffer size
			if (Attribs[9] > 16)
			{
				Attribs[9] -= 8;
			}
			else
				--Steps;
			break;
		default:
			os::Printer::log("Could not get config for EGL display.");
			return;
		}
	}

	if (params.AntiAlias && !Attribs[17])
		os::Printer::log("No multisampling.");

	if (params.WithAlphaChannel && !Attribs[7])
		os::Printer::log("No alpha.");

	if (params.Stencilbuffer && !Attribs[15])
		os::Printer::log("No stencil buffer.");

	if (params.ZBufferBits > Attribs[13])
		os::Printer::log("No full depth buffer.");

	if (params.Bits > Attribs[9])
		os::Printer::log("No full color buffer.");
		
		#if defined(_IRR_COMPILE_WITH_ANDROID_DEVICE_)
	   /* EGL_NATIVE_VISUAL_ID is an attribute of the EGLConfig that is
		* guaranteed to be accepted by ANativeWindow_setBuffersGeometry().
		* As soon as we picked a EGLConfig, we can safely reconfigure the
		* ANativeWindow buffers to match, using EGL_NATIVE_VISUAL_ID. */
	   EGLint format;
	   eglGetConfigAttrib(EglDisplay, EglConfig, EGL_NATIVE_VISUAL_ID, &format);

	   ANativeWindow_setBuffersGeometry(EglWindow, 0, 0, format);
	   #endif
		os::Printer::log(" Creating EglSurface with nativeWindow...");
		EglSurface = eglCreateWindowSurface(EglDisplay, EglConfig, EglWindow, NULL);
		if (EGL_NO_SURFACE == EglSurface)
		{
			os::Printer::log("FAILED\n");
			EglSurface = eglCreateWindowSurface(EglDisplay, EglConfig, 0, NULL);
			os::Printer::log("Creating EglSurface without nativeWindows...");
		}
		else
			os::Printer::log("SUCCESS\n");
		if (EGL_NO_SURFACE == EglSurface)
		{
			os::Printer::log("FAILED\n");
			os::Printer::log("Could not create surface for OpenGL-ES2 display.");
		}
		else
			os::Printer::log("SUCCESS\n");

#ifdef EGL_VERSION_1_2
		if (minorVersion>1)
			eglBindAPI(EGL_OPENGL_ES_API);
#endif
		os::Printer::log("Creating EglContext...");
		EglContext = EGL_NO_CONTEXT;

		if (!params.ForceLegacyDevice)
		{
			os::Printer::log("Trying to create Context for OpenGL-ES3.");

			EGLint contextAttrib[] =
			{
				#ifdef EGL_VERSION_1_3
				EGL_CONTEXT_CLIENT_VERSION, 3,
				#endif
				EGL_NONE, 0
			};

			EglContext = eglCreateContext(EglDisplay, EglConfig, EGL_NO_CONTEXT, contextAttrib);
		}

		if (EGL_NO_CONTEXT == EglContext)
		{
			os::Printer::log("Trying to create Context for OpenGL-ES2.");
			IsCoreContext = false;

			EGLint contextAttrib[] =
			{
				#ifdef EGL_VERSION_1_3
				EGL_CONTEXT_CLIENT_VERSION, 2,
				#endif
				EGL_NONE, 0
			};

			EglContext = eglCreateContext(EglDisplay, EglConfig, EGL_NO_CONTEXT, contextAttrib);
			if (EGL_NO_CONTEXT == EglContext)
			{
				os::Printer::log("FAILED\n");
				os::Printer::log("Could not create Context for OpenGL-ES2 display.");
			}
		}

		eglMakeCurrent(EglDisplay, EglSurface, EglSurface, EglContext);
		if (testEGLError())
		{
			os::Printer::log("Could not make Context current for OpenGL-ES2 display.");
		}

//~ #ifdef _IRR_COMPILE_WITH_ANDROID_DEVICE_
		//~ int backingWidth;
		//~ int backingHeight;
		//~ eglQuerySurface(EglDisplay, EglSurface, EGL_WIDTH, &backingWidth);
		//~ eglQuerySurface(EglDisplay, EglSurface, EGL_HEIGHT, &backingHeight);
        //~ core::dimension2d<u32> WindowSize(backingWidth, backingHeight);
        //~ CNullDriver::ScreenSize = WindowSize;
//~ #endif


		// set vsync
		if (params.Vsync)
			eglSwapInterval(EglDisplay, 1);
			
			
			
        const f32 egl_ver = core::fast_atof(reinterpret_cast<const c8*>(eglQueryString(EglDisplay, EGL_VERSION)));
        EGLVersion = static_cast<u16>(core::floor32(egl_ver)*100+core::round32(core::fract(egl_ver)*10.0f));
        core::stringc eglExtensions = eglQueryString(EglDisplay, EGL_EXTENSIONS);
        os::Printer::log(eglExtensions.c_str());
        
        os::Printer::log(eglQueryString(EglDisplay, EGL_CLIENT_APIS));
}


ContextEGL::~ContextEGL()
{
	eglMakeCurrent(EglDisplay, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);

	if (EglContext != EGL_NO_CONTEXT)
	{
		eglDestroyContext(EglDisplay, EglContext);
		EglContext = EGL_NO_CONTEXT;
	}

	if (EglSurface != EGL_NO_SURFACE)
	{
		eglDestroySurface(EglDisplay, EglSurface);
		EglSurface = EGL_NO_SURFACE;
	}

	if (EglDisplay != EGL_NO_DISPLAY)
	{
		eglTerminate(EglDisplay);
		EglDisplay = EGL_NO_DISPLAY;
	}

#if defined(_IRR_COMPILE_WITH_WINDOWS_DEVICE_)
	if (HDc)
		ReleaseDC((HWND)EglWindow, HDc);
#endif
}


bool ContextEGL::swapBuffers()
{
	eglSwapBuffers(EglDisplay, EglSurface);
	EGLint g = eglGetError();
	
	return g == EGL_SUCCESS;
}


void ContextEGL::reloadEGLSurface(void* window)
{
	os::Printer::log("Reload EGL surface.");

	#if defined(_IRR_COMPILE_WITH_ANDROID_DEVICE_)
		EglWindow = (ANativeWindow*)window;
	#endif

	if (!EglWindow)
		os::Printer::log("Invalid Egl window.");

	eglMakeCurrent(EglDisplay, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);

	eglDestroySurface(EglDisplay, EglSurface);

	EglSurface = eglCreateWindowSurface(EglDisplay, EglConfig, EglWindow, 0);

	if (EGL_NO_SURFACE == EglSurface)
		os::Printer::log("Could not create EGL surface.");

	eglMakeCurrent(EglDisplay, EglSurface, EglSurface, EglContext);
}


bool ContextEGL::testEGLError()
{
#if defined(_DEBUG)
	EGLint g = eglGetError();
	switch (g)
	{
		case EGL_SUCCESS:
			return false;
		case EGL_NOT_INITIALIZED :
			os::Printer::log("Not Initialized", ELL_ERROR);
			break;
		case EGL_BAD_ACCESS:
			os::Printer::log("Bad Access", ELL_ERROR);
			break;
		case EGL_BAD_ALLOC:
			os::Printer::log("Bad Alloc", ELL_ERROR);
			break;
		case EGL_BAD_ATTRIBUTE:
			os::Printer::log("Bad Attribute", ELL_ERROR);
			break;
		case EGL_BAD_CONTEXT:
			os::Printer::log("Bad Context", ELL_ERROR);
			break;
		case EGL_BAD_CONFIG:
			os::Printer::log("Bad Config", ELL_ERROR);
			break;
		case EGL_BAD_CURRENT_SURFACE:
			os::Printer::log("Bad Current Surface", ELL_ERROR);
			break;
		case EGL_BAD_DISPLAY:
			os::Printer::log("Bad Display", ELL_ERROR);
			break;
		case EGL_BAD_SURFACE:
			os::Printer::log("Bad Surface", ELL_ERROR);
			break;
		case EGL_BAD_MATCH:
			os::Printer::log("Bad Match", ELL_ERROR);
			break;
		case EGL_BAD_PARAMETER:
			os::Printer::log("Bad Parameter", ELL_ERROR);
			break;
		case EGL_BAD_NATIVE_PIXMAP:
			os::Printer::log("Bad Native Pixmap", ELL_ERROR);
			break;
		case EGL_BAD_NATIVE_WINDOW:
			os::Printer::log("Bad Native Window", ELL_ERROR);
			break;
		case EGL_CONTEXT_LOST:
			os::Printer::log("Context Lost", ELL_ERROR);
			break;
	};
	return true;
#else
	return false;
#endif
}

}

}

#endif
