// Copyright (C) 2013      Patryk Nadrowski
// Copyright (C) 2016-2017 Dawid Gan
// Heavily based on the OpenGL driver implemented by Nikolaus Gebhardt
// OpenGL ES driver implemented by Christian Stehno and first OpenGL ES 2.0
// driver implemented by Amundis.
// This file is part of the "Irrlicht Engine".
// For conditions of distribution and use, see copyright notice in Irrlicht.h

#ifndef __C_CONTEXT_EGL_H_INCLUDED__
#define __C_CONTEXT_EGL_H_INCLUDED__

#include "IrrCompileConfig.h"

#if defined(_IRR_COMPILE_WITH_EGL_)

#include "SIrrCreationParameters.h"
#include "SExposedVideoData.h"

#if defined(_IRR_COMPILE_WITH_ANDROID_DEVICE_)
#include <EGL/egl.h>
#else
#include <EGL/egl.h>
#include <EGL/eglplatform.h>
#endif

#if defined(_IRR_WINDOWS_API_)
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#endif

#ifdef _MSC_VER
#pragma comment(lib, "libEGL.lib")
#endif


namespace irr
{
namespace video
{
	
class ContextEGL
{
private:
	NativeWindowType EglWindow;
	void* EglDisplay;
	void* EglSurface;
	void* EglContext;
	EGLConfig EglConfig;
	u16 EGLVersion;
	
#ifdef _IRR_COMPILE_WITH_WINDOWS_DEVICE_
	HDC HDc;
#endif

	bool IsCoreContext;
	
	bool testEGLError();

public:
	ContextEGL(const SIrrlichtCreationParameters& params,
			   const SExposedVideoData& data);
	~ContextEGL();

	void reloadEGLSurface(void* window);
	bool swapBuffers();
	bool isCoreContext() {return IsCoreContext;}
};

}
}

#endif

#endif
