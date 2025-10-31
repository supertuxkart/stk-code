// Copyright (C) 2002-2012 Nikolaus Gebhardt
// This file is part of the "Irrlicht Engine".
// For conditions of distribution and use, see copyright notice in irrlicht.h

#include "IrrCompileConfig.h"

#ifdef _IRR_COMPILE_WITH_SDL_DEVICE_

#include "CIrrDeviceSDL.h"
#include "IEventReceiver.h"
#include "irrList.h"
#include "os.h"
#include "CTimer.h"
#include "irrString.h"
#include "Keycodes.h"
#include "COSOperator.h"
#include <stdio.h>
#include <stdlib.h>
#include "SIrrCreationParameters.h"
#include "COpenGLExtensionHandler.h"
#include "COGLES2Driver.h"

#include "guiengine/engine.hpp"
#include "ge_main.hpp"
#include "glad/gl.h"
#include "ge_vulkan_driver.hpp"
#include "ge_vulkan_scene_manager.hpp"
#include "MoltenVK.h"

#include <SDL_vulkan.h>

extern bool GLContextDebugBit;

namespace irr
{
	namespace video
	{
		extern bool useCoreContext;
		IVideoDriver* createOpenGLDriver(const SIrrlichtCreationParameters& params,
			io::IFileSystem* io, CIrrDeviceSDL* device);
		IVideoDriver* createOGLES2Driver(const SIrrlichtCreationParameters& params,
			io::IFileSystem* io, CIrrDeviceSDL* device, u32 default_fb);
#ifdef _IRR_COMPILE_WITH_DIRECT3D_9_
		IVideoDriver* createDirectX9Driver(const SIrrlichtCreationParameters& params,
			io::IFileSystem* io, HWND window);
#endif
#ifdef _IRR_COMPILE_WITH_VULKAN_
		IVideoDriver* createVulkanDriver(const SIrrlichtCreationParameters& params,
			io::IFileSystem* io, SDL_Window* win, IrrlichtDevice* device);
#endif
	} // end namespace video

} // end namespace irr

extern "C" void init_objc(SDL_SysWMinfo* info, float* top, float* bottom, float* left, float* right);
extern "C" void enable_momentum_scroll();
extern "C" int handle_app_event(void* userdata, SDL_Event* event);
extern "C" void Android_initDisplayCutout(float* top, float* bottom, float* left, float* right, int* initial_orientation);
extern "C" int Android_disablePadding();

namespace irr
{
//! constructor
CIrrDeviceSDL::CIrrDeviceSDL(const SIrrlichtCreationParameters& param)
	: CIrrDeviceStub(param),
	Window(0), Context(0),
	MouseX(0), MouseY(0), MouseButtonStates(0),
	Width(param.WindowSize.Width), Height(param.WindowSize.Height),
	TopPadding(0), BottomPadding(0), LeftPadding(0), RightPadding(0),
	InitialOrientation(0), WindowHasFocus(false), WindowMinimized(false),
	Resizable(false), AccelerometerIndex(-1), AccelerometerInstance(-1),
	GyroscopeIndex(-1), GyroscopeInstance(-1)
{
	#ifdef _DEBUG
	setDebugName("CIrrDeviceSDL");
	#endif

#ifdef DLOPEN_MOLTENVK
	m_moltenvk = NULL;
#endif

	Operator = 0;
	// Initialize SDL... Timer for sleep, video for the obvious, and
	// noparachute prevents SDL from catching fatal errors.
	SDL_SetHint(SDL_HINT_NO_SIGNAL_HANDLERS, "1");
	SDL_SetHint(SDL_HINT_ACCELEROMETER_AS_JOYSTICK, "0");
#if SDL_VERSION_ATLEAST(2, 0, 18)
	SDL_SetHint(SDL_HINT_IME_SHOW_UI, "1");
#endif

	// Switch SDL disables this hint by default: https://github.com/devkitPro/SDL/pull/55#issuecomment-633775255
	SDL_SetHint(SDL_HINT_TOUCH_MOUSE_EVENTS, "1");

#ifdef ANDROID
	SDL_SetHint(SDL_HINT_ORIENTATIONS, "LandscapeLeft LandscapeRight");
#endif

#ifndef MOBILE_STK
	// Prevent fullscreen minimizes when losing focus
	if (CreationParams.Fullscreen)
	{
		if (!GE::getGEConfig()->m_fullscreen_desktop)
			SDL_SetHint(SDL_HINT_VIDEO_MINIMIZE_ON_FOCUS_LOSS, "0");
		else if (CreationParams.DriverType != video::EDT_VULKAN)
			SDL_SetHint(SDL_HINT_VIDEO_MINIMIZE_ON_FOCUS_LOSS, "0");
	}
#endif

	u32 init_flags = SDL_INIT_TIMER | SDL_INIT_VIDEO;
	if (SDL_InitSubSystem(init_flags) < 0)
	{
		os::Printer::log("Unable to initialize SDL!", SDL_GetError());
		Close = true;
	}

#if SDL_VERSION_ATLEAST(2, 0, 9)
	// Don't exit if failed to init sensor (doesn't work in wine)
	if (SDL_InitSubSystem(SDL_INIT_SENSOR) < 0)
	{
		os::Printer::log("Failed to init SDL sensor!", SDL_GetError());
	}
#endif

	// create keymap
	createKeyMap();

	// create window
	if (CreationParams.DriverType != video::EDT_NULL)
	{
#if defined(_IRR_OSX_PLATFORM_) && !defined(IOS_STK)
		enable_momentum_scroll();
#endif

		// create the window, only if we do not use the null device
		if (!Close && createWindow())
		{
			SDL_VERSION(&Info.version);

#if (defined(IOS_STK) || defined(_IRR_COMPILE_WITH_DIRECT3D_9_)) && !defined(__SWITCH__)
			// Only iOS or DirectX9 build uses the Info structure
			// Switch doesn't support GetWindowWMInfo
#ifdef IOS_STK
			if (!SDL_GetWindowWMInfo(Window, &Info))
#else
			if (CreationParams.DriverType == video::EDT_DIRECT3D9 && !SDL_GetWindowWMInfo(Window, &Info))
#endif
				return;
#endif
#ifdef IOS_STK
			init_objc(&Info, &TopPadding, &BottomPadding, &LeftPadding, &RightPadding);
#endif
#ifdef ANDROID
			Android_initDisplayCutout(&TopPadding, &BottomPadding, &LeftPadding, &RightPadding, &InitialOrientation);
#else
                        (void)InitialOrientation;
#endif
			core::stringc sdlversion = "Compiled SDL Version ";
			sdlversion += Info.version.major;
			sdlversion += ".";
			sdlversion += Info.version.minor;
			sdlversion += ".";
			sdlversion += Info.version.patch;

			Operator = new COSOperator(sdlversion);
			os::Printer::log(sdlversion.c_str(), ELL_INFORMATION);

			core::stringc cur_sdlversion = "Current SDL Version ";
			SDL_version version = {};
			SDL_GetVersion(&version);
			cur_sdlversion += version.major;
			cur_sdlversion += ".";
			cur_sdlversion += version.minor;
			cur_sdlversion += ".";
			cur_sdlversion += version.patch;

			os::Printer::log(cur_sdlversion.c_str(), ELL_INFORMATION);
#if SDL_VERSION_ATLEAST(2, 0, 9)
			for (int i = 0; i < SDL_NumSensors(); i++)
			{
				if (SDL_SensorGetDeviceType(i) == SDL_SENSOR_ACCEL)
					AccelerometerIndex = i;
				else if (SDL_SensorGetDeviceType(i) == SDL_SENSOR_GYRO)
					GyroscopeIndex = i;
			}
#endif
		}
		else
			return;
	}

	// create cursor control
	CursorControl = new CCursorControl(this);

	// create driver
	createDriver();

	if (VideoDriver)
	{
		if (CreationParams.DriverType == video::EDT_VULKAN)
			createGUIAndVulkanScene();
		else
			createGUIAndScene();
	}
#ifdef IOS_STK
	SDL_SetEventFilter(handle_app_event, NULL);
#endif
}


//! destructor
CIrrDeviceSDL::~CIrrDeviceSDL()
{
	if (VideoDriver)
	{
		// Irrlicht calls gl function when quiting, but SDL has dropped its context, manually clear the loaded GL functions
#ifdef _IRR_COMPILE_WITH_OPENGL_
		irr::video::COpenGLExtensionHandler* h = dynamic_cast<irr::video::COpenGLExtensionHandler*>(VideoDriver);
		if (h)
			h->clearGLExtensions();
#endif
#ifdef _IRR_COMPILE_WITH_OGLES2_
		irr::video::COGLES2Driver* es2 = dynamic_cast<irr::video::COGLES2Driver*>(VideoDriver);
		if (es2) {
			es2->cleanUp();
		}
#endif
		GE::GEVulkanDriver* gevk = dynamic_cast<GE::GEVulkanDriver*>(VideoDriver);
		if (gevk)
			gevk->destroyVulkan();
		VideoDriver->drop();
		VideoDriver = NULL;
	}
#ifdef DLOPEN_MOLTENVK
	delete m_moltenvk;
#endif
	if (Context)
		SDL_GL_DeleteContext(Context);
	if (Window)
		SDL_DestroyWindow(Window);
		
	u32 flags = SDL_INIT_TIMER | SDL_INIT_VIDEO;
	SDL_QuitSubSystem(flags);
}


bool CIrrDeviceSDL::activateAccelerometer(float updateInterval)
{
#if SDL_VERSION_ATLEAST(2, 0, 9)
	if (AccelerometerInstance == -1 && AccelerometerIndex != -1)
	{
		SDL_Sensor* accel = SDL_SensorOpen(AccelerometerIndex);
		if (accel)
			AccelerometerInstance = SDL_SensorGetInstanceID(accel);
	}
#endif
	return AccelerometerInstance != -1;
}


bool CIrrDeviceSDL::deactivateAccelerometer()
{
#if SDL_VERSION_ATLEAST(2, 0, 9)
	if (AccelerometerInstance == -1)
		return false;
	SDL_Sensor* accel = SDL_SensorFromInstanceID(AccelerometerInstance);
	if (!accel)
		return false;
	SDL_SensorClose(accel);
	AccelerometerInstance = -1;
#endif
	return true;
}


bool CIrrDeviceSDL::isAccelerometerActive()
{
	return AccelerometerInstance != -1;
}


bool CIrrDeviceSDL::isAccelerometerAvailable()
{
	return AccelerometerIndex != -1;
}


bool CIrrDeviceSDL::activateGyroscope(float updateInterval)
{
#if SDL_VERSION_ATLEAST(2, 0, 9)
	if (GyroscopeInstance == -1 && GyroscopeIndex != -1)
	{
		SDL_Sensor* gyro = SDL_SensorOpen(GyroscopeIndex);
		if (gyro)
			GyroscopeInstance = SDL_SensorGetInstanceID(gyro);
	}
#endif
	return GyroscopeInstance != -1;
}


bool CIrrDeviceSDL::deactivateGyroscope()
{
#if SDL_VERSION_ATLEAST(2, 0, 9)
	if (GyroscopeInstance == -1)
		return false;
	SDL_Sensor* gyro = SDL_SensorFromInstanceID(GyroscopeInstance);
	if (!gyro)
		return false;
	SDL_SensorClose(gyro);
	GyroscopeInstance = -1;
#endif
	return true;
}


bool CIrrDeviceSDL::isGyroscopeActive()
{
	return GyroscopeInstance != -1;
}


bool CIrrDeviceSDL::isGyroscopeAvailable()
{
	return GyroscopeIndex != -1;
}


bool versionCorrect(int major, int minor)
{
#ifdef _IRR_COMPILE_WITH_OGLES2_
	return true;
#else
	int created_major = 2;
	int created_minor = 0;
	glGetIntegerv(GL_MAJOR_VERSION, &created_major);
	glGetIntegerv(GL_MINOR_VERSION, &created_minor);
	if (created_major > major ||
		(created_major == major && created_minor >= minor))
		return true;
	return false;
#endif
}


// Used in OptionsScreenVideo for live fullscreen toggle for vulkan driver
extern "C" void update_fullscreen_desktop(int val)
{
	GE::GEVulkanDriver* gevk = GE::getVKDriver();
	if (!gevk || !GE::getGEConfig()->m_fullscreen_desktop)
		return;
	SDL_Window* window = gevk->getSDLWindow();

	int prev_width = 0;
	int prev_height = 0;
	SDL_GetWindowSize(window, &prev_width, &prev_height);

	if (val != 0)
		val = SDL_WINDOW_FULLSCREEN_DESKTOP;
	SDL_SetWindowFullscreen(window, val);
	if (val == 0)
	{
		SDL_SetWindowSize(window, prev_width * 0.8f, prev_height * 0.8f);
		SDL_RaiseWindow(window);
	}
}


// Used in OptionsScreenVideo for live updating vertical sync config
extern "C" void update_swap_interval(int swap_interval)
{
#ifndef IOS_STK
	// iOS always use vertical sync
	if (swap_interval > 1)
		swap_interval = 1;

	GE::GEVulkanDriver* gevk = GE::getVKDriver();
	if (gevk)
	{
		gevk->updateSwapInterval(swap_interval);
		return;
	}

	// Try adaptive vsync first if support
	if (swap_interval > 0)
	{
		int ret = SDL_GL_SetSwapInterval(-1);
		if (ret == 0)
			return;
	}
	SDL_GL_SetSwapInterval(swap_interval);
#endif
}


bool CIrrDeviceSDL::createWindow()
{
	CreationParams.m_sdl_window = NULL;
	// Ignore alpha size here, this follow irr_driver.cpp:450
	// Try 32 and, upon failure, 24 then 16 bit per pixels
	if (CreationParams.DriverType == video::EDT_OPENGL ||
		CreationParams.DriverType == video::EDT_OGLES2)
	{
		if (CreationParams.Bits == 32)
		{
			SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 8);
			SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 8);
			SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 8);
			SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
		}
		else if (CreationParams.Bits == 24)
		{
			SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 5);
			SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 5);
			SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 5);
			SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 16);
		}
		else
		{
			SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 3);
			SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 3);
			SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 2);
			SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 16);
		}
	}

	u32 flags = SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE;
#if !defined(ANDROID) && !defined(__SWITCH__)
	if (CreationParams.DriverType == video::EDT_OPENGL ||
		CreationParams.DriverType == video::EDT_OGLES2 ||
		CreationParams.DriverType == video::EDT_VULKAN)
		flags |= SDL_WINDOW_ALLOW_HIGHDPI;
#endif

	if (CreationParams.Fullscreen)
	{
		if (GE::getGEConfig()->m_fullscreen_desktop)
		{
			flags |= SDL_WINDOW_FULLSCREEN_DESKTOP;
			CreationParams.Fullscreen = false;
		}
		else
			flags |= SDL_WINDOW_FULLSCREEN;
	}

	if (CreationParams.DriverType == video::EDT_OPENGL ||
		CreationParams.DriverType == video::EDT_OGLES2)
		flags |= SDL_WINDOW_OPENGL;
	else if (CreationParams.DriverType == video::EDT_VULKAN)
	{
#ifdef DLOPEN_MOLTENVK
		m_moltenvk = new MoltenVK();
		if (!m_moltenvk->loaded())
		{
			os::Printer::log("Current MacOSX version doesn't support Vulkan or MoltenVK failed to load", ELL_WARNING);
			return false;
		}
#endif
#if SDL_VERSION_ATLEAST(2, 0, 12)
		SDL_SetHint(SDL_HINT_VIDEO_EXTERNAL_CONTEXT, "1");
#endif
		flags |= SDL_WINDOW_VULKAN;
	}

#ifdef MOBILE_STK
	flags |= SDL_WINDOW_BORDERLESS | SDL_WINDOW_MAXIMIZED;
#endif

	if (CreationParams.DriverType == video::EDT_OPENGL ||
		CreationParams.DriverType == video::EDT_OGLES2)
	{
		tryCreateOpenGLContext(flags);
		if (!Window || !Context)
		{
			os::Printer::log( "Could not initialize display!" );
			return false;
		}
		update_swap_interval(CreationParams.SwapInterval);
	}
	else
	{
		Window = SDL_CreateWindow("",
			(float)CreationParams.WindowPosition.X,
			(float)CreationParams.WindowPosition.Y,
			(float)CreationParams.WindowSize.Width,
			(float)CreationParams.WindowSize.Height, flags);
		if (!Window)
		{
			os::Printer::log( "Could not initialize display!" );
#if SDL_VERSION_ATLEAST(2, 0, 12)
			if (CreationParams.DriverType == video::EDT_VULKAN)
				SDL_SetHint(SDL_HINT_VIDEO_EXTERNAL_CONTEXT, "0");
#endif
			return false;
		}
	}
	CreationParams.m_sdl_window = Window;
	return true;
}


void CIrrDeviceSDL::tryCreateOpenGLContext(u32 flags)
{
start:
	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, CreationParams.Doublebuffer);
	irr::video::useCoreContext = true;

	if (GLContextDebugBit)
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_DEBUG_FLAG);

	if (CreationParams.DriverType == video::EDT_OGLES2)
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_ES);
	else
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);

	if (CreationParams.ForceLegacyDevice)
		goto legacy;

#ifdef _IRR_COMPILE_WITH_OGLES2_
	if (Context)
	{
		SDL_GL_DeleteContext(Context);
		Context = NULL;
	}
	if (Window)
	{
		SDL_DestroyWindow(Window);
		Window = NULL;
	}

	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
	Window = SDL_CreateWindow("",
		(float)CreationParams.WindowPosition.X,
		(float)CreationParams.WindowPosition.Y,
		(float)CreationParams.WindowSize.Width,
		(float)CreationParams.WindowSize.Height, flags);
	if (Window)
	{
		Context = SDL_GL_CreateContext(Window);
		if (Context && gladLoadGLES2((GLADloadfunc)SDL_GL_GetProcAddress) != 0 &&
			versionCorrect(3, 0)) return;
	}

#else
	if (Context)
	{
		SDL_GL_DeleteContext(Context);
		Context = NULL;
	}
	if (Window)
	{
		SDL_DestroyWindow(Window);
		Window = NULL;
	}

	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
	Window = SDL_CreateWindow("",
		(float)CreationParams.WindowPosition.X,
		(float)CreationParams.WindowPosition.Y,
		(float)CreationParams.WindowSize.Width,
		(float)CreationParams.WindowSize.Height, flags);
	if (Window)
	{
		Context = SDL_GL_CreateContext(Window);
		if (Context && gladLoadGL((GLADloadfunc)SDL_GL_GetProcAddress) != 0 &&
			versionCorrect(4, 3)) return;
	}

	if (Context)
	{
		SDL_GL_DeleteContext(Context);
		Context = NULL;
	}
	if (Window)
	{
		SDL_DestroyWindow(Window);
		Window = NULL;
	}

	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
	Window = SDL_CreateWindow("",
		(float)CreationParams.WindowPosition.X,
		(float)CreationParams.WindowPosition.Y,
		(float)CreationParams.WindowSize.Width,
		(float)CreationParams.WindowSize.Height, flags);
	if (Window)
	{
		Context = SDL_GL_CreateContext(Window);
		if (Context && gladLoadGL((GLADloadfunc)SDL_GL_GetProcAddress) != 0 &&
			versionCorrect(3, 3)) return;
	}

	if (Context)
	{
		SDL_GL_DeleteContext(Context);
		Context = NULL;
	}
	if (Window)
	{
		SDL_DestroyWindow(Window);
		Window = NULL;
	}

	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);
	Window = SDL_CreateWindow("",
		(float)CreationParams.WindowPosition.X,
		(float)CreationParams.WindowPosition.Y,
		(float)CreationParams.WindowSize.Width,
		(float)CreationParams.WindowSize.Height, flags);
	if (Window)
	{
		Context = SDL_GL_CreateContext(Window);
		if (Context && gladLoadGL((GLADloadfunc)SDL_GL_GetProcAddress) != 0 &&
			versionCorrect(3, 1)) return;
	}
#endif

legacy:
	irr::video::useCoreContext = false;
	if (Context)
	{
		SDL_GL_DeleteContext(Context);
		Context = NULL;
	}
	if (Window)
	{
		SDL_DestroyWindow(Window);
		Window = NULL;
	}

#ifdef _IRR_COMPILE_WITH_OGLES2_
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
#else
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);
#endif
	if (CreationParams.DriverType == video::EDT_OGLES2)
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_ES);
	else
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, 0);
	Window = SDL_CreateWindow("",
		(float)CreationParams.WindowPosition.X,
		(float)CreationParams.WindowPosition.Y,
		(float)CreationParams.WindowSize.Width,
		(float)CreationParams.WindowSize.Height, flags);
	if (Window)
	{
		Context = SDL_GL_CreateContext(Window);
#ifdef _IRR_COMPILE_WITH_OGLES2_
		if (Context && gladLoadGLES2((GLADloadfunc)SDL_GL_GetProcAddress) != 0) return;
#else
		if (Context && gladLoadGL((GLADloadfunc)SDL_GL_GetProcAddress) != 0) return;
#endif
	}

	if (CreationParams.Doublebuffer)
	{
		CreationParams.Doublebuffer = false;
		goto start;
	}
}

//! create the driver
void CIrrDeviceSDL::createDriver()
{
	switch(CreationParams.DriverType)
	{
	case video::EDT_OPENGL:
		#ifdef _IRR_COMPILE_WITH_OPENGL_
		VideoDriver = video::createOpenGLDriver(CreationParams, FileSystem, this);
		#else
		os::Printer::log("No OpenGL support compiled in.", ELL_ERROR);
		#endif
		break;

	case video::EDT_OGLES2:
	{
		#ifdef _IRR_COMPILE_WITH_OGLES2_
		u32 default_fb = 0;
		#ifdef IOS_STK
		default_fb = Info.info.uikit.framebuffer;
		#endif
		VideoDriver = video::createOGLES2Driver(CreationParams, FileSystem, this, default_fb);
		#else
		os::Printer::log("No OpenGL ES 2.0 support compiled in.", ELL_ERROR);
		#endif
		break;
	}

	case video::EDT_VULKAN:
	{
		#ifdef _IRR_COMPILE_WITH_VULKAN_
		try
		{
			VideoDriver = video::createVulkanDriver(CreationParams, FileSystem, Window, this);
		}
		catch (std::exception& e)
		{
#if SDL_VERSION_ATLEAST(2, 0, 12)
			SDL_SetHint(SDL_HINT_VIDEO_EXTERNAL_CONTEXT, "0");
#endif
			os::Printer::log("createVulkanDriver failed", e.what(), ELL_ERROR);
		}
		#else
		os::Printer::log("No Vulkan support compiled in.", ELL_ERROR);
		#endif
		break;
	}

	case video::EDT_DIRECT3D9:
	{
		#ifdef _IRR_COMPILE_WITH_DIRECT3D_9_
		VideoDriver = video::createDirectX9Driver(CreationParams, FileSystem, Info.info.win.window);
		#else
		os::Printer::log("No DirectX 9 support compiled in.", ELL_ERROR);
		#endif
		break;
	}

	case video::EDT_NULL:
		VideoDriver = video::createNullDriver(FileSystem, CreationParams.WindowSize);
		break;

	default:
		os::Printer::log("Unable to create video driver of unknown type.", ELL_ERROR);
		break;
	}
}

// In input_manager.cpp
extern "C" void handle_joystick(SDL_Event& event);
// In main_loop.cpp
extern "C" void pause_mainloop();
extern "C" void resume_mainloop();
extern "C" void reset_network_body();
//! runs the device. Returns false if device wants to be deleted
bool CIrrDeviceSDL::run()
{
	os::Timer::tick();

	SEvent irrevent;
	SDL_Event SDL_event;

	while ( !Close && SDL_PollEvent( &SDL_event ) )
	{
		switch ( SDL_event.type )
		{
#if defined(MOBILE_STK) && !defined(IOS_STK)
		case SDL_APP_WILLENTERBACKGROUND:
			pause_mainloop();
			break;
		case SDL_APP_DIDENTERFOREGROUND:
			resume_mainloop();
			break;
#ifdef ANDROID
		// From https://github.com/libsdl-org/SDL/blob/main/docs/README-android.md
		// However, there's a chance (on older hardware, or on systems under heavy load),
		// where the GL context can not be restored. In that case you have to
		// listen for a specific message (SDL_RENDER_DEVICE_RESET) and restore
		// your textures manually or quit the app.
		case SDL_RENDER_DEVICE_RESET:
			Close = true;
			break;
#endif
#endif
#if SDL_VERSION_ATLEAST(2, 0, 9)
		case SDL_SENSORUPDATE:
			if (SDL_event.sensor.which == AccelerometerInstance)
			{
				SDL_DisplayOrientation o = SDL_GetDisplayOrientation(0);
				irrevent.EventType = irr::EET_ACCELEROMETER_EVENT;
				if (o == SDL_ORIENTATION_LANDSCAPE ||
					o == SDL_ORIENTATION_LANDSCAPE_FLIPPED)
				{
					irrevent.AccelerometerEvent.X = SDL_event.sensor.data[0];
					irrevent.AccelerometerEvent.Y = SDL_event.sensor.data[1];
				}
				else
				{
					// For android multi-window mode vertically
					irrevent.AccelerometerEvent.X = -SDL_event.sensor.data[1];
					irrevent.AccelerometerEvent.Y = -SDL_event.sensor.data[0];
				}
				irrevent.AccelerometerEvent.Z = SDL_event.sensor.data[2];
				// Mobile STK specific
				if (irrevent.AccelerometerEvent.X < 0.0)
					irrevent.AccelerometerEvent.X *= -1.0;

				if (o == SDL_ORIENTATION_LANDSCAPE_FLIPPED ||
					o == SDL_ORIENTATION_PORTRAIT_FLIPPED)
					irrevent.AccelerometerEvent.Y *= -1.0;

				postEventFromUser(irrevent);
			}
			else if (SDL_event.sensor.which == GyroscopeInstance)
			{
				irrevent.EventType = irr::EET_GYROSCOPE_EVENT;
				irrevent.GyroscopeEvent.X = SDL_event.sensor.data[0];
				irrevent.GyroscopeEvent.Y = SDL_event.sensor.data[1];
				irrevent.GyroscopeEvent.Z = SDL_event.sensor.data[2];
				postEventFromUser(irrevent);
			}
			break;
#endif
		case SDL_FINGERMOTION:
		case SDL_FINGERDOWN:
		case SDL_FINGERUP:
			irrevent.EventType = irr::EET_TOUCH_INPUT_EVENT;
			irrevent.TouchInput.Event = SDL_event.type == SDL_FINGERMOTION ? irr::ETIE_MOVED :
				SDL_event.type == SDL_FINGERDOWN ? irr::ETIE_PRESSED_DOWN : irr::ETIE_LEFT_UP;
			irrevent.TouchInput.ID = getTouchId(SDL_event.tfinger.fingerId);
			if (SDL_event.type == SDL_FINGERUP)
				removeTouchId(SDL_event.tfinger.fingerId);
			irrevent.TouchInput.X = SDL_event.tfinger.x * getRealScreenSize().Width;
			irrevent.TouchInput.Y = SDL_event.tfinger.y * getRealScreenSize().Height;
			postEventFromUser(irrevent);
			break;

		case SDL_MOUSEWHEEL:
			irrevent.EventType = irr::EET_MOUSE_INPUT_EVENT;
			irrevent.MouseInput.Event = irr::EMIE_MOUSE_WHEEL;
			irrevent.MouseInput.X = MouseX;
			irrevent.MouseInput.Y = MouseY;

			irrevent.MouseInput.ButtonStates = MouseButtonStates;
#if SDL_VERSION_ATLEAST(2, 0, 18)
			irrevent.MouseInput.Wheel = 
				SDL_event.wheel.preciseX + SDL_event.wheel.preciseY;
#else
			irrevent.MouseInput.Wheel = irr::core::clamp<irr::f32>(
				SDL_event.wheel.x + SDL_event.wheel.y, -1.0f, 1.0f);
#endif

			postEventFromUser(irrevent);
			break;

		case SDL_MOUSEMOTION:
			irrevent.EventType = irr::EET_MOUSE_INPUT_EVENT;
			irrevent.MouseInput.Event = irr::EMIE_MOUSE_MOVED;
			MouseX = irrevent.MouseInput.X = SDL_event.motion.x * getNativeScaleX();
			MouseY = irrevent.MouseInput.Y = SDL_event.motion.y * getNativeScaleY();
			irrevent.MouseInput.ButtonStates = MouseButtonStates;

			postEventFromUser(irrevent);
			break;

		case SDL_MOUSEBUTTONDOWN:
		case SDL_MOUSEBUTTONUP:

			irrevent.EventType = irr::EET_MOUSE_INPUT_EVENT;
			irrevent.MouseInput.X = SDL_event.button.x * getNativeScaleX();
			irrevent.MouseInput.Y = SDL_event.button.y * getNativeScaleY();

			irrevent.MouseInput.Event = irr::EMIE_MOUSE_MOVED;

			switch(SDL_event.button.button)
			{
			case SDL_BUTTON_LEFT:
				if (SDL_event.type == SDL_MOUSEBUTTONDOWN)
				{
					irrevent.MouseInput.Event = irr::EMIE_LMOUSE_PRESSED_DOWN;
					MouseButtonStates |= irr::EMBSM_LEFT;
				}
				else
				{
					irrevent.MouseInput.Event = irr::EMIE_LMOUSE_LEFT_UP;
					MouseButtonStates &= ~irr::EMBSM_LEFT;
				}
				break;

			case SDL_BUTTON_RIGHT:
				if (SDL_event.type == SDL_MOUSEBUTTONDOWN)
				{
					irrevent.MouseInput.Event = irr::EMIE_RMOUSE_PRESSED_DOWN;
					MouseButtonStates |= irr::EMBSM_RIGHT;
				}
				else
				{
					irrevent.MouseInput.Event = irr::EMIE_RMOUSE_LEFT_UP;
					MouseButtonStates &= ~irr::EMBSM_RIGHT;
				}
				break;

			case SDL_BUTTON_MIDDLE:
				if (SDL_event.type == SDL_MOUSEBUTTONDOWN)
				{
					irrevent.MouseInput.Event = irr::EMIE_MMOUSE_PRESSED_DOWN;
					MouseButtonStates |= irr::EMBSM_MIDDLE;
				}
				else
				{
					irrevent.MouseInput.Event = irr::EMIE_MMOUSE_LEFT_UP;
					MouseButtonStates &= ~irr::EMBSM_MIDDLE;
				}
				break;
			}

			irrevent.MouseInput.ButtonStates = MouseButtonStates;

			if (irrevent.MouseInput.Event != irr::EMIE_MOUSE_MOVED)
			{
				postEventFromUser(irrevent);

				if ( irrevent.MouseInput.Event >= EMIE_LMOUSE_PRESSED_DOWN && irrevent.MouseInput.Event <= EMIE_MMOUSE_PRESSED_DOWN )
				{
					u32 clicks = checkSuccessiveClicks(irrevent.MouseInput.X, irrevent.MouseInput.Y, irrevent.MouseInput.Event);
					if ( clicks == 2 )
					{
						irrevent.MouseInput.Event = (EMOUSE_INPUT_EVENT)(EMIE_LMOUSE_DOUBLE_CLICK + irrevent.MouseInput.Event-EMIE_LMOUSE_PRESSED_DOWN);
						postEventFromUser(irrevent);
					}
					else if ( clicks == 3 )
					{
						irrevent.MouseInput.Event = (EMOUSE_INPUT_EVENT)(EMIE_LMOUSE_TRIPLE_CLICK + irrevent.MouseInput.Event-EMIE_LMOUSE_PRESSED_DOWN);
						postEventFromUser(irrevent);
					}
				}
			}
			break;

		case SDL_KEYDOWN:
		case SDL_KEYUP:
			{
				SKeyMap mp;
				mp.SDLKey = SDL_event.key.keysym.sym;
				s32 idx = KeyMap.binary_search(mp);

				EKEY_CODE key;
				if (idx == -1)
				{
					// Fallback to use scancode directly if not found, happens in
					// belarusian keyboard layout for example
					auto it = ScanCodeMap.find(SDL_event.key.keysym.scancode);
					if (it != ScanCodeMap.end())
						key = it->second;
					else
						key = (EKEY_CODE)0;
				}
				else
					key = (EKEY_CODE)KeyMap[idx].Win32Key;

				irrevent.EventType = irr::EET_KEY_INPUT_EVENT;
				irrevent.KeyInput.Char = 0;
				irrevent.KeyInput.Key = key;
				irrevent.KeyInput.PressedDown = (SDL_event.type == SDL_KEYDOWN);
				irrevent.KeyInput.Shift = (SDL_event.key.keysym.mod & KMOD_SHIFT) != 0;
				irrevent.KeyInput.Control = (SDL_event.key.keysym.mod & KMOD_CTRL ) != 0;
				postEventFromUser(irrevent);
			}
			break;

		case SDL_QUIT:
			Close = true;
			break;

		case SDL_WINDOWEVENT:
			{
				if (SDL_event.window.event == SDL_WINDOWEVENT_SIZE_CHANGED)
				{
					handleNewSize(SDL_event.window.data1, SDL_event.window.data2);
				}
				else if (SDL_event.window.event == SDL_WINDOWEVENT_MINIMIZED)
				{
					WindowMinimized = true;
				}
				else if (SDL_event.window.event == SDL_WINDOWEVENT_MAXIMIZED)
				{
					WindowMinimized = false;
				}
				else if (SDL_event.window.event == SDL_WINDOWEVENT_FOCUS_GAINED)
				{
					WindowHasFocus = true;
					reset_network_body();
#ifdef ANDROID
					if (VideoDriver)
						VideoDriver->unpauseRendering();
#endif
				}
				else if (SDL_event.window.event == SDL_WINDOWEVENT_FOCUS_LOST)
				{
					WindowHasFocus = false;
				}
				else if (SDL_event.window.event == SDL_WINDOWEVENT_MOVED)
				{
					// In windows the rendering is paused when window is moving
					reset_network_body();
				}
			}
			break;
		case SDL_TEXTEDITING:
			{
				irrevent.EventType = irr::EET_SDL_TEXT_EVENT;
				irrevent.SDLTextEvent.Type = SDL_event.type;
				const size_t size = sizeof(irrevent.SDLTextEvent.Text);
				const size_t other_size = sizeof(SDL_event.edit.text);
				static_assert(sizeof(size) == sizeof(other_size), "Wrong size");
				memcpy(irrevent.SDLTextEvent.Text, SDL_event.edit.text, size);
				irrevent.SDLTextEvent.Start = SDL_event.edit.start;
				irrevent.SDLTextEvent.Length = SDL_event.edit.length;
				postEventFromUser(irrevent);
			}
			break;
		case SDL_TEXTINPUT:
			{
				irrevent.EventType = irr::EET_SDL_TEXT_EVENT;
				irrevent.SDLTextEvent.Type = SDL_event.type;
				const size_t size = sizeof(irrevent.SDLTextEvent.Text);
				const size_t other_size = sizeof(SDL_event.text.text);
				static_assert(sizeof(size) == sizeof(other_size), "Wrong size");
				memcpy(irrevent.SDLTextEvent.Text, SDL_event.text.text, size);
				irrevent.SDLTextEvent.Start = 0;
				irrevent.SDLTextEvent.Length = 0;
				postEventFromUser(irrevent);
			}
			break;
		default:
			handle_joystick(SDL_event);
			break;
		} // end switch

	} // end while

	return !Close;
}


void CIrrDeviceSDL::handleNewSize(u32 width, u32 height)
{
	if (width != Width || height != Height)
	{
		Width = width;
		Height = height;
		if (VideoDriver)
			VideoDriver->OnResize(core::dimension2d<u32>(Width, Height));
		reset_network_body();
	}
}


//! Activate any joysticks, and generate events for them.
bool CIrrDeviceSDL::activateJoysticks(core::array<SJoystickInfo> & joystickInfo)
{
	return false;
}



//! pause execution temporarily
void CIrrDeviceSDL::yield()
{
	SDL_Delay(0);
}


//! pause execution for a specified time
void CIrrDeviceSDL::sleep(u32 timeMs, bool pauseTimer)
{
	const bool wasStopped = Timer ? Timer->isStopped() : true;
	if (pauseTimer && !wasStopped)
		Timer->stop();

	SDL_Delay(timeMs);

	if (pauseTimer && !wasStopped)
		Timer->start();
}


//! sets the caption of the window
void CIrrDeviceSDL::setWindowCaption(const wchar_t* text)
{
	core::stringc textc = text;
	SDL_SetWindowTitle(Window, textc.c_str());
}


//! presents a surface in the client area
bool CIrrDeviceSDL::present(video::IImage* surface, void* windowId, core::rect<s32>* srcClip)
{
	return false;
}


//! notifies the device that it should close itself
void CIrrDeviceSDL::closeDevice()
{
	Close = true;
}


//! \return Pointer to a list with all video modes supported
video::IVideoModeList* CIrrDeviceSDL::getVideoModeList()
{
	if (!VideoModeList.getVideoModeCount())
	{
		// enumerate video modes.
		int display_count = 0;
		if ((display_count = SDL_GetNumVideoDisplays()) < 1)
		{
			os::Printer::log("No display created: ", SDL_GetError(), ELL_ERROR);
			return &VideoModeList;
		}

		int mode_count = 0;
		if ((mode_count = SDL_GetNumDisplayModes(0)) < 1)
		{
			os::Printer::log("No display modes available: ", SDL_GetError(), ELL_ERROR);
			return &VideoModeList;
		}

		SDL_DisplayMode mode = { SDL_PIXELFORMAT_UNKNOWN, 0, 0, 0, 0 };
		if (SDL_GetDesktopDisplayMode(0, &mode) == 0)
		{
			VideoModeList.setDesktop(SDL_BITSPERPIXEL(mode.format),
				core::dimension2d<u32>(mode.w, mode.h));
		}

#ifdef MOBILE_STK
	// SDL2 will return w,h and h,w for mobile STK, as we only use landscape
	// so we just use desktop resolution for now
	VideoModeList.addMode(core::dimension2d<u32>(mode.w, mode.h),
		SDL_BITSPERPIXEL(mode.format));
#else
		for (int i = 0; i < mode_count; i++)
		{
			if (SDL_GetDisplayMode(0, i, &mode) == 0)
			{
				VideoModeList.addMode(core::dimension2d<u32>(mode.w, mode.h),
					SDL_BITSPERPIXEL(mode.format));
			}
		}
#endif
	}

	return &VideoModeList;
}


//! Sets if the window should be resizable in windowed mode.
void CIrrDeviceSDL::setResizable(bool resize)
{
#if SDL_VERSION_ATLEAST(2, 0, 5)
	if (CreationParams.Fullscreen)
		return;
	SDL_SetWindowResizable(Window, resize ? SDL_TRUE : SDL_FALSE);
	Resizable = resize;
#endif
}


bool CIrrDeviceSDL::isResizable() const
{
	if (CreationParams.Fullscreen)
		return false;
	return Resizable;
}


//! Minimizes window if possible
void CIrrDeviceSDL::minimizeWindow()
{
	// do nothing
}


//! Maximize window
void CIrrDeviceSDL::maximizeWindow()
{
	// do nothing
}


//! Restore original window size
void CIrrDeviceSDL::restoreWindow()
{
	// do nothing
}


//! Move window to requested position
bool CIrrDeviceSDL::moveWindow(int x, int y)
{
	if (Window)
	{
		SDL_SetWindowPosition(Window, x, y);
		return true;
	}
	return false;
}


//! Get current window position.
bool CIrrDeviceSDL::getWindowPosition(int* x, int* y)
{
	if (Window)
	{
		SDL_GetWindowPosition(Window, x, y);
		return true;
	}
	return false;
}


//! Get DPI of current display.
bool CIrrDeviceSDL::getDisplayDPI(float* ddpi, float* hdpi, float* vdpi)
{
	if (Window)
	{
        SDL_GetDisplayDPI(SDL_GetWindowDisplayIndex(Window), ddpi, hdpi, vdpi);
		return true;
	}
	return false;
}


//! returns if window is active. if not, nothing need to be drawn
bool CIrrDeviceSDL::isWindowActive() const
{
	return (WindowHasFocus && !WindowMinimized);
}


//! returns if window has focus.
bool CIrrDeviceSDL::isWindowFocused() const
{
	return WindowHasFocus;
}


//! returns if window is minimized.
bool CIrrDeviceSDL::isWindowMinimized() const
{
	return WindowMinimized;
}


//! Set the current Gamma Value for the Display
bool CIrrDeviceSDL::setGammaRamp( f32 red, f32 green, f32 blue, f32 brightness, f32 contrast )
{
	/*
	// todo: Gamma in SDL takes ints, what does Irrlicht use?
	return (SDL_SetGamma(red, green, blue) != -1);
	*/
	return false;
}

//! Get the current Gamma Value for the Display
bool CIrrDeviceSDL::getGammaRamp( f32 &red, f32 &green, f32 &blue, f32 &brightness, f32 &contrast )
{
/*	brightness = 0.f;
	contrast = 0.f;
	return (SDL_GetGamma(&red, &green, &blue) != -1);*/
	return false;
}

void CIrrDeviceSDL::setWindowMinimumSize(u32 width, u32 height)
{
	if (Window)
		SDL_SetWindowMinimumSize(Window, width, height);
}

//! returns color format of the window.
video::ECOLOR_FORMAT CIrrDeviceSDL::getColorFormat() const
{
	if (Window)
	{
		u32 pixel_format = SDL_GetWindowPixelFormat(Window);
		if (SDL_BITSPERPIXEL(pixel_format) == 16)
		{
			if (SDL_ISPIXELFORMAT_ALPHA(pixel_format))
				return video::ECF_A1R5G5B5;
			else
				return video::ECF_R5G6B5;
		}
		else
		{
			if (SDL_ISPIXELFORMAT_ALPHA(pixel_format))
				return video::ECF_A8R8G8B8;
			else
				return video::ECF_R8G8B8;
		}
	}
	else
		return CIrrDeviceStub::getColorFormat();
}


void CIrrDeviceSDL::createKeyMap()
{
	// I don't know if this is the best method  to create
	// the lookuptable, but I'll leave it like that until
	// I find a better version.

	KeyMap.reallocate(105);

	// buttons missing

	KeyMap.push_back(SKeyMap(SDLK_BACKSPACE, IRR_KEY_BACK));
	KeyMap.push_back(SKeyMap(SDLK_TAB, IRR_KEY_TAB));
	KeyMap.push_back(SKeyMap(SDLK_CLEAR, IRR_KEY_CLEAR));
	KeyMap.push_back(SKeyMap(SDLK_RETURN, IRR_KEY_RETURN));

	// combined modifiers missing

	KeyMap.push_back(SKeyMap(SDLK_PAUSE, IRR_KEY_PAUSE));
	KeyMap.push_back(SKeyMap(SDLK_CAPSLOCK, IRR_KEY_CAPITAL));

	// asian letter keys missing

	KeyMap.push_back(SKeyMap(SDLK_ESCAPE, IRR_KEY_ESCAPE));

	// asian letter keys missing

	KeyMap.push_back(SKeyMap(SDLK_SPACE, IRR_KEY_SPACE));
	KeyMap.push_back(SKeyMap(SDLK_PAGEUP, IRR_KEY_PRIOR));
	KeyMap.push_back(SKeyMap(SDLK_PAGEDOWN, IRR_KEY_NEXT));
	KeyMap.push_back(SKeyMap(SDLK_END, IRR_KEY_END));
	KeyMap.push_back(SKeyMap(SDLK_HOME, IRR_KEY_HOME));
	KeyMap.push_back(SKeyMap(SDLK_LEFT, IRR_KEY_LEFT));
	KeyMap.push_back(SKeyMap(SDLK_UP, IRR_KEY_UP));
	KeyMap.push_back(SKeyMap(SDLK_RIGHT, IRR_KEY_RIGHT));
	KeyMap.push_back(SKeyMap(SDLK_DOWN, IRR_KEY_DOWN));

	// select missing
	KeyMap.push_back(SKeyMap(SDLK_PRINTSCREEN, IRR_KEY_PRINT));
	// execute missing
	KeyMap.push_back(SKeyMap(SDLK_PRINTSCREEN, IRR_KEY_SNAPSHOT));

	KeyMap.push_back(SKeyMap(SDLK_INSERT, IRR_KEY_INSERT));
	KeyMap.push_back(SKeyMap(SDLK_DELETE, IRR_KEY_DELETE));
	KeyMap.push_back(SKeyMap(SDLK_HELP, IRR_KEY_HELP));

	KeyMap.push_back(SKeyMap(SDLK_0, IRR_KEY_0));
	KeyMap.push_back(SKeyMap(SDLK_1, IRR_KEY_1));
	KeyMap.push_back(SKeyMap(SDLK_2, IRR_KEY_2));
	KeyMap.push_back(SKeyMap(SDLK_3, IRR_KEY_3));
	KeyMap.push_back(SKeyMap(SDLK_4, IRR_KEY_4));
	KeyMap.push_back(SKeyMap(SDLK_5, IRR_KEY_5));
	KeyMap.push_back(SKeyMap(SDLK_6, IRR_KEY_6));
	KeyMap.push_back(SKeyMap(SDLK_7, IRR_KEY_7));
	KeyMap.push_back(SKeyMap(SDLK_8, IRR_KEY_8));
	KeyMap.push_back(SKeyMap(SDLK_9, IRR_KEY_9));

	KeyMap.push_back(SKeyMap(SDLK_a, IRR_KEY_A));
	KeyMap.push_back(SKeyMap(SDLK_b, IRR_KEY_B));
	KeyMap.push_back(SKeyMap(SDLK_c, IRR_KEY_C));
	KeyMap.push_back(SKeyMap(SDLK_d, IRR_KEY_D));
	KeyMap.push_back(SKeyMap(SDLK_e, IRR_KEY_E));
	KeyMap.push_back(SKeyMap(SDLK_f, IRR_KEY_F));
	KeyMap.push_back(SKeyMap(SDLK_g, IRR_KEY_G));
	KeyMap.push_back(SKeyMap(SDLK_h, IRR_KEY_H));
	KeyMap.push_back(SKeyMap(SDLK_i, IRR_KEY_I));
	KeyMap.push_back(SKeyMap(SDLK_j, IRR_KEY_J));
	KeyMap.push_back(SKeyMap(SDLK_k, IRR_KEY_K));
	KeyMap.push_back(SKeyMap(SDLK_l, IRR_KEY_L));
	KeyMap.push_back(SKeyMap(SDLK_m, IRR_KEY_M));
	KeyMap.push_back(SKeyMap(SDLK_n, IRR_KEY_N));
	KeyMap.push_back(SKeyMap(SDLK_o, IRR_KEY_O));
	KeyMap.push_back(SKeyMap(SDLK_p, IRR_KEY_P));
	KeyMap.push_back(SKeyMap(SDLK_q, IRR_KEY_Q));
	KeyMap.push_back(SKeyMap(SDLK_r, IRR_KEY_R));
	KeyMap.push_back(SKeyMap(SDLK_s, IRR_KEY_S));
	KeyMap.push_back(SKeyMap(SDLK_t, IRR_KEY_T));
	KeyMap.push_back(SKeyMap(SDLK_u, IRR_KEY_U));
	KeyMap.push_back(SKeyMap(SDLK_v, IRR_KEY_V));
	KeyMap.push_back(SKeyMap(SDLK_w, IRR_KEY_W));
	KeyMap.push_back(SKeyMap(SDLK_x, IRR_KEY_X));
	KeyMap.push_back(SKeyMap(SDLK_y, IRR_KEY_Y));
	KeyMap.push_back(SKeyMap(SDLK_z, IRR_KEY_Z));

	KeyMap.push_back(SKeyMap(SDLK_LGUI, IRR_KEY_LWIN));
	KeyMap.push_back(SKeyMap(SDLK_RGUI, IRR_KEY_RWIN));
	KeyMap.push_back(SKeyMap(SDLK_APPLICATION, IRR_KEY_APPS));
	KeyMap.push_back(SKeyMap(SDLK_POWER, IRR_KEY_SLEEP)); //??

	KeyMap.push_back(SKeyMap(SDLK_KP_0, IRR_KEY_NUMPAD0));
	KeyMap.push_back(SKeyMap(SDLK_KP_1, IRR_KEY_NUMPAD1));
	KeyMap.push_back(SKeyMap(SDLK_KP_2, IRR_KEY_NUMPAD2));
	KeyMap.push_back(SKeyMap(SDLK_KP_3, IRR_KEY_NUMPAD3));
	KeyMap.push_back(SKeyMap(SDLK_KP_4, IRR_KEY_NUMPAD4));
	KeyMap.push_back(SKeyMap(SDLK_KP_5, IRR_KEY_NUMPAD5));
	KeyMap.push_back(SKeyMap(SDLK_KP_6, IRR_KEY_NUMPAD6));
	KeyMap.push_back(SKeyMap(SDLK_KP_7, IRR_KEY_NUMPAD7));
	KeyMap.push_back(SKeyMap(SDLK_KP_8, IRR_KEY_NUMPAD8));
	KeyMap.push_back(SKeyMap(SDLK_KP_9, IRR_KEY_NUMPAD9));
	KeyMap.push_back(SKeyMap(SDLK_KP_MULTIPLY, IRR_KEY_MULTIPLY));
	KeyMap.push_back(SKeyMap(SDLK_KP_PLUS, IRR_KEY_ADD));
	KeyMap.push_back(SKeyMap(SDLK_SEPARATOR, IRR_KEY_SEPARATOR));
	KeyMap.push_back(SKeyMap(SDLK_KP_MINUS, IRR_KEY_SUBTRACT));
	KeyMap.push_back(SKeyMap(SDLK_KP_PERIOD, IRR_KEY_DECIMAL));
	KeyMap.push_back(SKeyMap(SDLK_KP_DIVIDE, IRR_KEY_DIVIDE));

	KeyMap.push_back(SKeyMap(SDLK_F1,  IRR_KEY_F1));
	KeyMap.push_back(SKeyMap(SDLK_F2,  IRR_KEY_F2));
	KeyMap.push_back(SKeyMap(SDLK_F3,  IRR_KEY_F3));
	KeyMap.push_back(SKeyMap(SDLK_F4,  IRR_KEY_F4));
	KeyMap.push_back(SKeyMap(SDLK_F5,  IRR_KEY_F5));
	KeyMap.push_back(SKeyMap(SDLK_F6,  IRR_KEY_F6));
	KeyMap.push_back(SKeyMap(SDLK_F7,  IRR_KEY_F7));
	KeyMap.push_back(SKeyMap(SDLK_F8,  IRR_KEY_F8));
	KeyMap.push_back(SKeyMap(SDLK_F9,  IRR_KEY_F9));
	KeyMap.push_back(SKeyMap(SDLK_F10, IRR_KEY_F10));
	KeyMap.push_back(SKeyMap(SDLK_F11, IRR_KEY_F11));
	KeyMap.push_back(SKeyMap(SDLK_F12, IRR_KEY_F12));
	KeyMap.push_back(SKeyMap(SDLK_F13, IRR_KEY_F13));
	KeyMap.push_back(SKeyMap(SDLK_F14, IRR_KEY_F14));
	KeyMap.push_back(SKeyMap(SDLK_F15, IRR_KEY_F15));
	// no higher F-keys

	KeyMap.push_back(SKeyMap(SDLK_NUMLOCKCLEAR, IRR_KEY_NUMLOCK));
	KeyMap.push_back(SKeyMap(SDLK_SCROLLLOCK, IRR_KEY_SCROLL));
	KeyMap.push_back(SKeyMap(SDLK_LSHIFT, IRR_KEY_LSHIFT));
	KeyMap.push_back(SKeyMap(SDLK_RSHIFT, IRR_KEY_RSHIFT));
	KeyMap.push_back(SKeyMap(SDLK_LCTRL,  IRR_KEY_LCONTROL));
	KeyMap.push_back(SKeyMap(SDLK_RCTRL,  IRR_KEY_RCONTROL));
	KeyMap.push_back(SKeyMap(SDLK_LALT,  IRR_KEY_LMENU));
	KeyMap.push_back(SKeyMap(SDLK_RALT,  IRR_KEY_RMENU));
	KeyMap.push_back(SKeyMap(SDLK_MENU,  IRR_KEY_MENU));

	KeyMap.push_back(SKeyMap(SDLK_PLUS,   IRR_KEY_PLUS));
	KeyMap.push_back(SKeyMap(SDLK_COMMA,  IRR_KEY_COMMA));
	KeyMap.push_back(SKeyMap(SDLK_MINUS,  IRR_KEY_MINUS));
	KeyMap.push_back(SKeyMap(SDLK_PERIOD, IRR_KEY_PERIOD));
	KeyMap.push_back(SKeyMap(SDLK_AC_BACK, IRR_KEY_ESCAPE));

	KeyMap.push_back(SKeyMap(SDLK_EQUALS, IRR_KEY_PLUS));
	KeyMap.push_back(SKeyMap(SDLK_LEFTBRACKET, IRR_KEY_OEM_4));
	KeyMap.push_back(SKeyMap(SDLK_RIGHTBRACKET, IRR_KEY_OEM_6));
	KeyMap.push_back(SKeyMap(SDLK_BACKSLASH, IRR_KEY_OEM_5));
	KeyMap.push_back(SKeyMap(SDLK_SEMICOLON, IRR_KEY_OEM_1));
	KeyMap.push_back(SKeyMap(SDLK_SLASH, IRR_KEY_OEM_2));
	KeyMap.push_back(SKeyMap(SDLK_MODE, IRR_KEY_BUTTON_MODE));

	// some special keys missing

	KeyMap.sort();

	ScanCodeMap[SDL_SCANCODE_A] = IRR_KEY_A;
	ScanCodeMap[SDL_SCANCODE_B] = IRR_KEY_B;
	ScanCodeMap[SDL_SCANCODE_C] = IRR_KEY_C;
	ScanCodeMap[SDL_SCANCODE_D] = IRR_KEY_D;
	ScanCodeMap[SDL_SCANCODE_E] = IRR_KEY_E;
	ScanCodeMap[SDL_SCANCODE_F] = IRR_KEY_F;
	ScanCodeMap[SDL_SCANCODE_G] = IRR_KEY_G;
	ScanCodeMap[SDL_SCANCODE_H] = IRR_KEY_H;
	ScanCodeMap[SDL_SCANCODE_I] = IRR_KEY_I;
	ScanCodeMap[SDL_SCANCODE_J] = IRR_KEY_J;
	ScanCodeMap[SDL_SCANCODE_K] = IRR_KEY_K;
	ScanCodeMap[SDL_SCANCODE_L] = IRR_KEY_L;
	ScanCodeMap[SDL_SCANCODE_M] = IRR_KEY_M;
	ScanCodeMap[SDL_SCANCODE_N] = IRR_KEY_N;
	ScanCodeMap[SDL_SCANCODE_O] = IRR_KEY_O;
	ScanCodeMap[SDL_SCANCODE_P] = IRR_KEY_P;
	ScanCodeMap[SDL_SCANCODE_Q] = IRR_KEY_Q;
	ScanCodeMap[SDL_SCANCODE_R] = IRR_KEY_R;
	ScanCodeMap[SDL_SCANCODE_S] = IRR_KEY_S;
	ScanCodeMap[SDL_SCANCODE_T] = IRR_KEY_T;
	ScanCodeMap[SDL_SCANCODE_U] = IRR_KEY_U;
	ScanCodeMap[SDL_SCANCODE_V] = IRR_KEY_V;
	ScanCodeMap[SDL_SCANCODE_W] = IRR_KEY_W;
	ScanCodeMap[SDL_SCANCODE_X] = IRR_KEY_X;
	ScanCodeMap[SDL_SCANCODE_Y] = IRR_KEY_Y;
	ScanCodeMap[SDL_SCANCODE_Z] = IRR_KEY_Z;
	ScanCodeMap[SDL_SCANCODE_1] = IRR_KEY_1;
	ScanCodeMap[SDL_SCANCODE_2] = IRR_KEY_2;
	ScanCodeMap[SDL_SCANCODE_3] = IRR_KEY_3;
	ScanCodeMap[SDL_SCANCODE_4] = IRR_KEY_4;
	ScanCodeMap[SDL_SCANCODE_5] = IRR_KEY_5;
	ScanCodeMap[SDL_SCANCODE_6] = IRR_KEY_6;
	ScanCodeMap[SDL_SCANCODE_7] = IRR_KEY_7;
	ScanCodeMap[SDL_SCANCODE_8] = IRR_KEY_8;
	ScanCodeMap[SDL_SCANCODE_9] = IRR_KEY_9;
	ScanCodeMap[SDL_SCANCODE_0] = IRR_KEY_0;
	ScanCodeMap[SDL_SCANCODE_RETURN] = IRR_KEY_RETURN;
	ScanCodeMap[SDL_SCANCODE_ESCAPE] = IRR_KEY_ESCAPE;
	ScanCodeMap[SDL_SCANCODE_BACKSPACE] = IRR_KEY_BACK;
	ScanCodeMap[SDL_SCANCODE_TAB] = IRR_KEY_TAB;
	ScanCodeMap[SDL_SCANCODE_SPACE] = IRR_KEY_SPACE;
	ScanCodeMap[SDL_SCANCODE_MINUS] = IRR_KEY_MINUS;
	ScanCodeMap[SDL_SCANCODE_EQUALS] = IRR_KEY_PLUS;
	ScanCodeMap[SDL_SCANCODE_LEFTBRACKET] = IRR_KEY_OEM_4;
	ScanCodeMap[SDL_SCANCODE_RIGHTBRACKET] = IRR_KEY_OEM_6;
	ScanCodeMap[SDL_SCANCODE_BACKSLASH] = IRR_KEY_OEM_5;
	ScanCodeMap[SDL_SCANCODE_SEMICOLON] = IRR_KEY_OEM_1;
	ScanCodeMap[SDL_SCANCODE_APOSTROPHE] = IRR_KEY_OEM_7;
	ScanCodeMap[SDL_SCANCODE_GRAVE] = IRR_KEY_OEM_3;
	ScanCodeMap[SDL_SCANCODE_COMMA] = IRR_KEY_COMMA;
	ScanCodeMap[SDL_SCANCODE_PERIOD] = IRR_KEY_PERIOD;
	ScanCodeMap[SDL_SCANCODE_SLASH] = IRR_KEY_OEM_2;
	ScanCodeMap[SDL_SCANCODE_CAPSLOCK] = IRR_KEY_CAPITAL;
	ScanCodeMap[SDL_SCANCODE_F1] = IRR_KEY_F1;
	ScanCodeMap[SDL_SCANCODE_F2] = IRR_KEY_F2;
	ScanCodeMap[SDL_SCANCODE_F3] = IRR_KEY_F3;
	ScanCodeMap[SDL_SCANCODE_F4] = IRR_KEY_F4;
	ScanCodeMap[SDL_SCANCODE_F5] = IRR_KEY_F5;
	ScanCodeMap[SDL_SCANCODE_F6] = IRR_KEY_F6;
	ScanCodeMap[SDL_SCANCODE_F7] = IRR_KEY_F7;
	ScanCodeMap[SDL_SCANCODE_F8] = IRR_KEY_F8;
	ScanCodeMap[SDL_SCANCODE_F9] = IRR_KEY_F9;
	ScanCodeMap[SDL_SCANCODE_F10] = IRR_KEY_F10;
	ScanCodeMap[SDL_SCANCODE_F11] = IRR_KEY_F11;
	ScanCodeMap[SDL_SCANCODE_F12] = IRR_KEY_F12;
	ScanCodeMap[SDL_SCANCODE_PRINTSCREEN] = IRR_KEY_PRINT;
	ScanCodeMap[SDL_SCANCODE_SCROLLLOCK] = IRR_KEY_SCROLL;
	ScanCodeMap[SDL_SCANCODE_PAUSE] = IRR_KEY_PAUSE;
	ScanCodeMap[SDL_SCANCODE_INSERT] = IRR_KEY_INSERT;
	ScanCodeMap[SDL_SCANCODE_HOME] = IRR_KEY_HOME;
	ScanCodeMap[SDL_SCANCODE_PAGEUP] = IRR_KEY_PRIOR;
	ScanCodeMap[SDL_SCANCODE_DELETE] = IRR_KEY_DELETE;
	ScanCodeMap[SDL_SCANCODE_END] = IRR_KEY_END;
	ScanCodeMap[SDL_SCANCODE_PAGEDOWN] = IRR_KEY_NEXT;
	ScanCodeMap[SDL_SCANCODE_RIGHT] = IRR_KEY_RIGHT;
	ScanCodeMap[SDL_SCANCODE_LEFT] = IRR_KEY_LEFT;
	ScanCodeMap[SDL_SCANCODE_DOWN] = IRR_KEY_DOWN;
	ScanCodeMap[SDL_SCANCODE_UP] = IRR_KEY_UP;
	ScanCodeMap[SDL_SCANCODE_NUMLOCKCLEAR] = IRR_KEY_NUMLOCK;
	ScanCodeMap[SDL_SCANCODE_KP_DIVIDE] = IRR_KEY_DIVIDE;
	ScanCodeMap[SDL_SCANCODE_KP_MULTIPLY] = IRR_KEY_MULTIPLY;
	ScanCodeMap[SDL_SCANCODE_KP_MINUS] = IRR_KEY_MINUS;
	ScanCodeMap[SDL_SCANCODE_KP_PLUS] = IRR_KEY_PLUS;
	ScanCodeMap[SDL_SCANCODE_SEPARATOR] = IRR_KEY_SEPARATOR;
	ScanCodeMap[SDL_SCANCODE_KP_ENTER] = IRR_KEY_RETURN;
	ScanCodeMap[SDL_SCANCODE_KP_1] = IRR_KEY_NUMPAD1;
	ScanCodeMap[SDL_SCANCODE_KP_2] = IRR_KEY_NUMPAD2;
	ScanCodeMap[SDL_SCANCODE_KP_3] = IRR_KEY_NUMPAD3;
	ScanCodeMap[SDL_SCANCODE_KP_4] = IRR_KEY_NUMPAD4;
	ScanCodeMap[SDL_SCANCODE_KP_5] = IRR_KEY_NUMPAD5;
	ScanCodeMap[SDL_SCANCODE_KP_6] = IRR_KEY_NUMPAD6;
	ScanCodeMap[SDL_SCANCODE_KP_7] = IRR_KEY_NUMPAD7;
	ScanCodeMap[SDL_SCANCODE_KP_8] = IRR_KEY_NUMPAD8;
	ScanCodeMap[SDL_SCANCODE_KP_9] = IRR_KEY_NUMPAD9;
	ScanCodeMap[SDL_SCANCODE_KP_0] = IRR_KEY_NUMPAD0;
	ScanCodeMap[SDL_SCANCODE_LCTRL] = IRR_KEY_LCONTROL;
	ScanCodeMap[SDL_SCANCODE_LSHIFT] = IRR_KEY_LSHIFT;
	ScanCodeMap[SDL_SCANCODE_LALT] = IRR_KEY_LMENU;
	ScanCodeMap[SDL_SCANCODE_LGUI] = IRR_KEY_LWIN;
	ScanCodeMap[SDL_SCANCODE_RCTRL] = IRR_KEY_RCONTROL;
	ScanCodeMap[SDL_SCANCODE_RSHIFT] = IRR_KEY_RSHIFT;
	ScanCodeMap[SDL_SCANCODE_RALT] = IRR_KEY_RMENU;
	ScanCodeMap[SDL_SCANCODE_RGUI] = IRR_KEY_RWIN;
	ScanCodeMap[SDL_SCANCODE_APPLICATION] = IRR_KEY_APPS;
	ScanCodeMap[SDL_SCANCODE_MODE] = IRR_KEY_BUTTON_MODE;
	ScanCodeMap[SDL_SCANCODE_MENU] = IRR_KEY_MENU;
}


bool CIrrDeviceSDL::supportsTouchDevice() const
{
	return SDL_GetNumTouchDevices() > 0;
}


bool CIrrDeviceSDL::hasOnScreenKeyboard() const
{
	return SDL_HasScreenKeyboardSupport() == SDL_TRUE;
}


extern "C" int Android_getMovedHeight();
s32 CIrrDeviceSDL::getMovedHeight() const
{
#if defined(IOS_STK)
	return SDL_GetMovedHeightByScreenKeyboard() * getNativeScaleY();
#elif defined(ANDROID)
	return Android_getMovedHeight();
#else
	return 0;
#endif
}


extern "C" int Android_getKeyboardHeight();
u32 CIrrDeviceSDL::getOnScreenKeyboardHeight() const
{
#if defined(IOS_STK)
	return SDL_GetScreenKeyboardHeight() * getNativeScaleY();
#elif defined(ANDROID)
	return Android_getKeyboardHeight();
#else
	return 0;
#endif
}


f32 CIrrDeviceSDL::getNativeScaleX() const
{
	if (VideoDriver)
		return (f32)VideoDriver->getScreenSize().Width / (f32)Width;
	return 1.0f;
}


f32 CIrrDeviceSDL::getNativeScaleY() const
{
	if (VideoDriver)
		return (f32)VideoDriver->getScreenSize().Height / (f32)Height;
	return 1.0f;
}


s32 CIrrDeviceSDL::getTopPadding()
{
#ifdef ANDROID
	if (Android_disablePadding() != 0)
		return 0;
#endif
	return TopPadding * getNativeScaleY();
}


s32 CIrrDeviceSDL::getBottomPadding()
{
#ifdef ANDROID
	if (Android_disablePadding() != 0)
		return 0;
#endif
	return BottomPadding * getNativeScaleY();
}


s32 CIrrDeviceSDL::getLeftPadding()
{
#ifdef ANDROID
	if (Android_disablePadding() != 0)
		return 0;
#if SDL_VERSION_ATLEAST(2, 0, 9)
	if ((InitialOrientation == SDL_ORIENTATION_LANDSCAPE ||
		InitialOrientation == SDL_ORIENTATION_LANDSCAPE_FLIPPED) &&
		SDL_GetDisplayOrientation(0) != InitialOrientation)
		return RightPadding;
#endif
	return LeftPadding;
#else
	return LeftPadding * getNativeScaleX();
#endif
}


s32 CIrrDeviceSDL::getRightPadding()
{
#ifdef ANDROID
#if SDL_VERSION_ATLEAST(2, 0, 9)
	if (Android_disablePadding() != 0)
		return 0;
	if ((InitialOrientation == SDL_ORIENTATION_LANDSCAPE ||
		InitialOrientation == SDL_ORIENTATION_LANDSCAPE_FLIPPED) &&
		SDL_GetDisplayOrientation(0) != InitialOrientation)
		return LeftPadding;
#endif
	return RightPadding;
#else
	return RightPadding * getNativeScaleX();
#endif
}


void CIrrDeviceSDL::createGUIAndVulkanScene()
{
	#ifdef _IRR_COMPILE_WITH_GUI_
	// create gui environment
	GUIEnvironment = gui::createGUIEnvironment(FileSystem, VideoDriver, Operator);
	#endif

	// create Scene manager
	SceneManager = new GE::GEVulkanSceneManager(VideoDriver, FileSystem, CursorControl, GUIEnvironment);

	setEventReceiver(UserReceiver);
}


const core::dimension2du& CIrrDeviceSDL::getRealScreenSize() const
{
	static core::dimension2du ss;
	if (VideoDriver)
		return VideoDriver->getScreenSize();
	return ss;
}


} // end namespace irr

#endif // _IRR_COMPILE_WITH_SDL_DEVICE_

