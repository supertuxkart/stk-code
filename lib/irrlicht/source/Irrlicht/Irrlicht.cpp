// Copyright (C) 2002-2012 Nikolaus Gebhardt
// This file is part of the "Irrlicht Engine".
// For conditions of distribution and use, see copyright notice in irrlicht.h

#include "IrrCompileConfig.h"

//static const char* const copyright = "Irrlicht Engine (c) 2002-2012 Nikolaus Gebhardt";

#ifdef _IRR_WINDOWS_
	#include <windows.h>
	#if defined(_DEBUG) && !defined(__GNUWIN32__) && !defined(_WIN32_WCE)
		#include <crtdbg.h>
	#endif // _DEBUG
#endif

#include "irrlicht.h"
#ifdef _IRR_COMPILE_WITH_WINDOWS_DEVICE_
#include "CIrrDeviceWin32.h"
#endif

#ifdef _IRR_COMPILE_WITH_OSX_DEVICE_
#include "MacOSX/CIrrDeviceMacOSX.h"
#endif

#ifdef _IRR_COMPILE_WITH_WAYLAND_DEVICE_
#include "CIrrDeviceWayland.h"
#endif

#ifdef _IRR_COMPILE_WITH_X11_DEVICE_
#include "CIrrDeviceLinux.h"
#endif

#ifdef _IRR_COMPILE_WITH_SDL_DEVICE_
#include "CIrrDeviceSDL.h"
#endif

#ifdef _IRR_COMPILE_WITH_ANDROID_DEVICE_
#include "CIrrDeviceAndroid.h"
#include <android/log.h>
#endif

#include <stdio.h>

namespace irr
{
	//! stub for calling createDeviceEx
	IRRLICHT_API IrrlichtDevice* IRRCALLCONV createDevice(video::E_DRIVER_TYPE driverType,
			const core::dimension2d<u32>& windowSize,
			u32 bits, bool fullscreen,
			bool stencilbuffer, bool vsync, IEventReceiver* res,
            io::IFileSystem *file_system)
	{
		SIrrlichtCreationParameters p;
		p.DriverType = driverType;
		p.WindowSize = windowSize;
		p.Bits = (u8)bits;
		p.Fullscreen = fullscreen;
		p.Stencilbuffer = stencilbuffer;
		p.Vsync = vsync;
		p.EventReceiver = res;
        p.FileSystem = file_system;

		return createDeviceEx(p);
	}
	
	static void overrideDeviceType(E_DEVICE_TYPE& device_type)
	{
		const char* irr_device_type = getenv("IRR_DEVICE_TYPE");
		
		if (irr_device_type == NULL)
			return;

#ifdef _IRR_COMPILE_WITH_WINDOWS_DEVICE_
		if (strcmp(irr_device_type, "win32") == 0)
		{
			device_type = EIDT_WIN32;
		}
#endif
#ifdef _IRR_COMPILE_WITH_OSX_DEVICE_
		if (strcmp(irr_device_type, "osx") == 0)
		{
			device_type = EIDT_OSX;
		}
#endif
#ifdef _IRR_COMPILE_WITH_WAYLAND_DEVICE_
		if (strcmp(irr_device_type, "wayland") == 0)
		{
			device_type = EIDT_WAYLAND;
		}
#endif
#ifdef _IRR_COMPILE_WITH_X11_DEVICE_
		if (strcmp(irr_device_type, "x11") == 0)
		{
			device_type = EIDT_X11;
		}
#endif
#ifdef _IRR_COMPILE_WITH_SDL_DEVICE_
		if (strcmp(irr_device_type, "sdl") == 0)
		{
			device_type = EIDT_SDL;
		}
#endif
#ifdef _IRR_COMPILE_WITH_ANDROID_DEVICE_
		if (strcmp(irr_device_type, "android") == 0)
		{
			device_type = EIDT_ANDROID;
		}
#endif
	}

	extern "C" IRRLICHT_API IrrlichtDevice* IRRCALLCONV createDeviceEx(const SIrrlichtCreationParameters& params)
	{

		IrrlichtDevice* dev = 0;
		
		SIrrlichtCreationParameters creation_params = params;
		overrideDeviceType(creation_params.DeviceType);

#ifdef _IRR_COMPILE_WITH_WINDOWS_DEVICE_
		if (creation_params.DeviceType == EIDT_WIN32 || (!dev && creation_params.DeviceType == EIDT_BEST))
			dev = new CIrrDeviceWin32(creation_params);
#endif

#ifdef _IRR_COMPILE_WITH_OSX_DEVICE_
		if (creation_params.DeviceType == EIDT_OSX || (!dev && creation_params.DeviceType == EIDT_BEST))
			dev = new CIrrDeviceMacOSX(creation_params);
#endif

#ifdef _IRR_COMPILE_WITH_WAYLAND_DEVICE_
		if (creation_params.DeviceType == EIDT_WAYLAND || (!dev && creation_params.DeviceType == EIDT_BEST))
		{
			if (CIrrDeviceWayland::isWaylandDeviceWorking())
			{
				dev = new CIrrDeviceWayland(creation_params);
			}
		}
#endif

#ifdef _IRR_COMPILE_WITH_X11_DEVICE_
		if (creation_params.DeviceType == EIDT_X11 || (!dev && creation_params.DeviceType == EIDT_BEST))
			dev = new CIrrDeviceLinux(creation_params);
#endif

#ifdef _IRR_COMPILE_WITH_SDL_DEVICE_
		if (creation_params.DeviceType == EIDT_SDL || (!dev && creation_params.DeviceType == EIDT_BEST))
			dev = new CIrrDeviceSDL(creation_params);
#endif

#ifdef _IRR_COMPILE_WITH_ANDROID_DEVICE_
		if (creation_params.DeviceType == EIDT_ANDROID || (!dev && creation_params.DeviceType == EIDT_BEST))
			dev = new CIrrDeviceAndroid(creation_params);
#endif

		if (dev && !dev->getVideoDriver() && creation_params.DriverType != video::EDT_NULL)
		{
			dev->closeDevice(); // destroy window
			dev->run(); // consume quit message
			dev->drop();
			dev = 0;
		}

		return dev;
	}

namespace core
{
	const matrix4 IdentityMatrix(matrix4::EM4CONST_IDENTITY);
	irr::core::stringc LOCALE_DECIMAL_POINTS(".");
}

namespace video
{
	SMaterial IdentityMaterial;
}

} // end namespace irr


#if defined(_IRR_WINDOWS_API_) && !defined(_IRR_STATIC_LIB_)

BOOL APIENTRY DllMain( HANDLE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved )
{
	// _crtBreakAlloc = 139;

    switch (ul_reason_for_call)
	{
		case DLL_PROCESS_ATTACH:
			#if defined(_DEBUG) && !defined(__GNUWIN32__) && !defined(__BORLANDC__) && !defined (_WIN32_WCE) && !defined (_IRR_XBOX_PLATFORM_)
				_CrtSetDbgFlag(_CRTDBG_LEAK_CHECK_DF | _CRTDBG_ALLOC_MEM_DF);
			#endif
			break;
		case DLL_THREAD_ATTACH:
		case DLL_THREAD_DETACH:
		case DLL_PROCESS_DETACH:
			break;
    }
    return TRUE;
}

#endif // defined(_IRR_WINDOWS_)

