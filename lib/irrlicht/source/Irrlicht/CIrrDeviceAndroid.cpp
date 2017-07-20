// Copyright (C) 2002-2007 Nikolaus Gebhardt
// Copyright (C) 2007-2011 Christian Stehno
// Copyright (C) 2016-2017 Dawid Gan
// This file is part of the "Irrlicht Engine".
// For conditions of distribution and use, see copyright notice in irrlicht.h

#include "CIrrDeviceAndroid.h"

#ifdef _IRR_COMPILE_WITH_ANDROID_DEVICE_

#include <assert.h>
#include "os.h"
#include "CContextEGL.h"
#include "CFileSystem.h"
#include "COGLES2Driver.h"

namespace irr
{
	namespace video
	{
		IVideoDriver* createOGLES2Driver(const SIrrlichtCreationParameters& params,
			video::SExposedVideoData& data, io::IFileSystem* io);
	}
}

namespace irr
{

// These variables must be global. Otherwise initialization will reach infinite
// loop after creating the device second time (i.e. the NULL driver and then
// GLES2 driver). We get initialization events from Android only once.
bool CIrrDeviceAndroid::IsPaused = true;
bool CIrrDeviceAndroid::IsFocused = false;
bool CIrrDeviceAndroid::IsStarted = false;

//! constructor
CIrrDeviceAndroid::CIrrDeviceAndroid(const SIrrlichtCreationParameters& param)
	: CIrrDeviceStub(param),
	Accelerometer(0),
	Gyroscope(0),
	IsMousePressed(false)
{
	#ifdef _DEBUG
	setDebugName("CIrrDeviceAndroid");
	#endif

	Android = (android_app *)(param.PrivateData);
	assert(Android != NULL);

	Android->userData = this;
	Android->onAppCmd = handleAndroidCommand;
	Android->onInputEvent = handleInput;
	
	createKeyMap();

	CursorControl = new CCursorControl();
	
	Close = Android->destroyRequested;

	// It typically shouldn't happen, but just in case...
	if (Close)
		return;

	SensorManager = ASensorManager_getInstance();
	SensorEventQueue = ASensorManager_createEventQueue(SensorManager,
								Android->looper, LOOPER_ID_USER, NULL, NULL);

	ANativeActivity_setWindowFlags(Android->activity,
								   AWINDOW_FLAG_KEEP_SCREEN_ON |
								   AWINDOW_FLAG_FULLSCREEN, 0);

	os::Printer::log("Waiting for Android activity window to be created.", ELL_DEBUG);

	while (!IsStarted || !IsFocused || IsPaused)
	{
		s32 events = 0;
		android_poll_source* source = 0;

		s32 id = ALooper_pollAll(-1, NULL, &events, (void**)&source);

		if (id >=0 && source != NULL)
		{
			source->process(Android, source);
		}
	}
	
	assert(Android->window);
	os::Printer::log("Done", ELL_DEBUG);
	
	ExposedVideoData.OGLESAndroid.Window = Android->window;

	createVideoModeList();

	createDriver();

	if (VideoDriver)
		createGUIAndScene();
}


CIrrDeviceAndroid::~CIrrDeviceAndroid()
{
	Android->userData = NULL;
	Android->onAppCmd = NULL;
	Android->onInputEvent = NULL;
}

void CIrrDeviceAndroid::createVideoModeList()
{
	if (VideoModeList.getVideoModeCount() > 0)
		return;
		
	int width = ANativeWindow_getWidth(Android->window);
	int height = ANativeWindow_getHeight(Android->window);
	
	if (width > 0 && height > 0)
	{
		CreationParams.WindowSize.Width = width;
		CreationParams.WindowSize.Height = height;
	}

	core::dimension2d<u32> size = core::dimension2d<u32>(
									CreationParams.WindowSize.Width,
									CreationParams.WindowSize.Height);

	VideoModeList.addMode(size, 32);
	VideoModeList.setDesktop(32, size);
}

void CIrrDeviceAndroid::createDriver()
{
	// Create the driver.
	switch(CreationParams.DriverType)
	{
	case video::EDT_OGLES2:
		#ifdef _IRR_COMPILE_WITH_OGLES2_
		VideoDriver = video::createOGLES2Driver(CreationParams, ExposedVideoData, FileSystem);
		#else
		os::Printer::log("No OpenGL ES 2.0 support compiled in.", ELL_ERROR);
		#endif
		break;

	case video::EDT_NULL:
		VideoDriver = video::createNullDriver(FileSystem, CreationParams.WindowSize);
		break;

	default:
		os::Printer::log("Unable to create video driver of unknown type.", ELL_ERROR);
		break;
	}
}

bool CIrrDeviceAndroid::run()
{
	os::Timer::tick();
	
	while (!Close)
	{
		s32 Events = 0;
		android_poll_source* Source = 0;
		bool should_run = (IsStarted && IsFocused && !IsPaused);
		s32 id = ALooper_pollAll(should_run ? 0 : -1, NULL, &Events,
								 (void**)&Source);

		if (id < 0)
			break;

		if (Source)
		{
			Source->process(Android, Source);
		}

		// if a sensor has data, we'll process it now.
		if (id == LOOPER_ID_USER)
		{
			ASensorEvent event;
			while (ASensorEventQueue_getEvents(SensorEventQueue, &event, 1) > 0)
			{
				switch (event.type)
				{
				case ASENSOR_TYPE_ACCELEROMETER:
					SEvent accEvent;
					accEvent.EventType = EET_ACCELEROMETER_EVENT;
					accEvent.AccelerometerEvent.X = event.acceleration.x;
					accEvent.AccelerometerEvent.Y = event.acceleration.y;
					accEvent.AccelerometerEvent.Z = event.acceleration.z;

					postEventFromUser(accEvent);
					break;

				case ASENSOR_TYPE_GYROSCOPE:
					SEvent gyroEvent;
					gyroEvent.EventType = EET_GYROSCOPE_EVENT;
					gyroEvent.GyroscopeEvent.X = event.vector.x;
					gyroEvent.GyroscopeEvent.Y = event.vector.y;
					gyroEvent.GyroscopeEvent.Z = event.vector.z;

					postEventFromUser(gyroEvent);
					break;
				default:
					break;
				}
			}
		}
	}

	return !Close;
}

void CIrrDeviceAndroid::yield()
{
	struct timespec ts = {0,1};
	nanosleep(&ts, NULL);
}

void CIrrDeviceAndroid::sleep(u32 timeMs, bool pauseTimer)
{
	const bool wasStopped = Timer ? Timer->isStopped() : true;

	struct timespec ts;
	ts.tv_sec = (time_t) (timeMs / 1000);
	ts.tv_nsec = (long) (timeMs % 1000) * 1000000;

	if (pauseTimer && !wasStopped)
		Timer->stop();

	nanosleep(&ts, NULL);

	if (pauseTimer && !wasStopped)
		Timer->start();
}

void CIrrDeviceAndroid::setWindowCaption(const wchar_t* text)
{
}

bool CIrrDeviceAndroid::present(video::IImage* surface, void* windowId,
								core::rect<s32>* srcClip)
{
	return true;
}

bool CIrrDeviceAndroid::isWindowActive() const
{
	return (IsFocused && !IsPaused);
}

bool CIrrDeviceAndroid::isWindowFocused() const
{
	return IsFocused;
}

bool CIrrDeviceAndroid::isWindowMinimized() const
{
	return IsPaused;
}

void CIrrDeviceAndroid::closeDevice()
{
	Close = true;
}

void CIrrDeviceAndroid::setResizable(bool resize)
{
}

void CIrrDeviceAndroid::minimizeWindow()
{
}

void CIrrDeviceAndroid::maximizeWindow()
{
}

void CIrrDeviceAndroid::restoreWindow()
{
}

E_DEVICE_TYPE CIrrDeviceAndroid::getType() const
{
	return EIDT_ANDROID;
}

void CIrrDeviceAndroid::handleAndroidCommand(android_app* app, int32_t cmd)
{
	CIrrDeviceAndroid* device = (CIrrDeviceAndroid *)app->userData;
	assert(device != NULL);
	
	switch (cmd)
	{
	case APP_CMD_SAVE_STATE:
		os::Printer::log("Android command APP_CMD_SAVE_STATE", ELL_DEBUG);
		break;
	case APP_CMD_INIT_WINDOW:
		os::Printer::log("Android command APP_CMD_INIT_WINDOW", ELL_DEBUG);
		
		device->getExposedVideoData().OGLESAndroid.Window = app->window;
		
		// If the Android app is resumed, we need to re-create EGL surface
		// to allow to draw something on it again.
		if (device->VideoDriver != NULL && 
			device->CreationParams.DriverType == video::EDT_OGLES2)
		{
			video::COGLES2Driver* driver = (video::COGLES2Driver*)(device->VideoDriver);
			driver->getEGLContext()->reloadEGLSurface(app->window);
		}
		
		IsStarted = true;
		break;
	case APP_CMD_TERM_WINDOW:
		os::Printer::log("Android command APP_CMD_TERM_WINDOW", ELL_DEBUG);
		IsStarted = false;
		break;
	case APP_CMD_GAINED_FOCUS:
		os::Printer::log("Android command APP_CMD_GAINED_FOCUS", ELL_DEBUG);
		IsFocused = true;
		break;
	case APP_CMD_LOST_FOCUS:
		os::Printer::log("Android command APP_CMD_LOST_FOCUS", ELL_DEBUG);
		IsFocused = false;
		break;
	case APP_CMD_DESTROY:
		os::Printer::log("Android command APP_CMD_DESTROY", ELL_DEBUG);
		device->Close = true;
		// Make sure that state variables are set to the default state
		// when the app is destroyed
		IsPaused = true;
		IsFocused = false;
		IsStarted = false;
		break;
	case APP_CMD_PAUSE:
		os::Printer::log("Android command APP_CMD_PAUSE", ELL_DEBUG);
		IsPaused = true;
		break;
	case APP_CMD_RESUME:
		os::Printer::log("Android command APP_CMD_RESUME", ELL_DEBUG);
		IsPaused = false;
		break;
	case APP_CMD_START:
		os::Printer::log("Android command APP_CMD_START", ELL_DEBUG);
		break;
	case APP_CMD_STOP:
		os::Printer::log("Android command APP_CMD_STOP", ELL_DEBUG);
		break;
	case APP_CMD_WINDOW_RESIZED:
		os::Printer::log("Android command APP_CMD_WINDOW_RESIZED", ELL_DEBUG);
		break;
	case APP_CMD_CONFIG_CHANGED:
		os::Printer::log("Android command APP_CMD_CONFIG_CHANGED", ELL_DEBUG);
		break;
	case APP_CMD_LOW_MEMORY:
		os::Printer::log("Android command APP_CMD_LOW_MEMORY", ELL_DEBUG);
		break;
	default:
		break;
	}
	
	SEvent event;
	event.EventType = EET_SYSTEM_EVENT;
	event.SystemEvent.EventType = ESET_ANDROID_CMD;
	event.SystemEvent.AndroidCmd.Cmd = cmd;
	device->postEventFromUser(event);
}

s32 CIrrDeviceAndroid::handleInput(android_app* app, AInputEvent* androidEvent)
{
	CIrrDeviceAndroid* device = (CIrrDeviceAndroid*)app->userData;
	assert(device != NULL);
	
	s32 status = 0;
	
	switch (AInputEvent_getType(androidEvent))
	{
	case AINPUT_EVENT_TYPE_MOTION:
	{
		SEvent event;
		event.EventType = EET_TOUCH_INPUT_EVENT;

		s32 eventAction = AMotionEvent_getAction(androidEvent);

#if 0
		// Useful for debugging. We might have to pass some of those infos on at some point.
		// but preferably device independent (so iphone can use same irrlicht flags).
		int32_t flags = AMotionEvent_getFlags(androidEvent);
		os::Printer::log("flags: ", core::stringc(flags).c_str(), ELL_DEBUG);
		int32_t metaState = AMotionEvent_getMetaState(androidEvent);
		os::Printer::log("metaState: ", core::stringc(metaState).c_str(), ELL_DEBUG);
		int32_t edgeFlags = AMotionEvent_getEdgeFlags(androidEvent);
		os::Printer::log("edgeFlags: ", core::stringc(flags).c_str(), ELL_DEBUG);
#endif

		bool touchReceived = true;
		bool simulate_mouse = false;
		core::position2d<s32> mouse_pos = core::position2d<s32>(0, 0);

		switch (eventAction & AMOTION_EVENT_ACTION_MASK)
		{
		case AMOTION_EVENT_ACTION_DOWN:
		case AMOTION_EVENT_ACTION_POINTER_DOWN:
			event.TouchInput.Event = ETIE_PRESSED_DOWN;
			break;
		case AMOTION_EVENT_ACTION_MOVE:
			event.TouchInput.Event = ETIE_MOVED;
			break;
		case AMOTION_EVENT_ACTION_UP:
		case AMOTION_EVENT_ACTION_POINTER_UP:
		case AMOTION_EVENT_ACTION_CANCEL:
			event.TouchInput.Event = ETIE_LEFT_UP;
			break;
		default:
			touchReceived = false;
			break;
		}
		
		if (touchReceived)
		{
			s32 count = 1;
			s32 idx = (eventAction & AMOTION_EVENT_ACTION_POINTER_INDEX_MASK) >> 
								AMOTION_EVENT_ACTION_POINTER_INDEX_SHIFT;
			
			// Process all touches for move action.
			if (event.TouchInput.Event == ETIE_MOVED)
			{
				count = AMotionEvent_getPointerCount(androidEvent);
				idx = 0;
			}

			for (s32 i = 0; i < count; ++i)
			{
				event.TouchInput.ID = AMotionEvent_getPointerId(androidEvent, i + idx);
				event.TouchInput.X = AMotionEvent_getX(androidEvent, i + idx);
				event.TouchInput.Y = AMotionEvent_getY(androidEvent, i + idx);
				
				device->postEventFromUser(event);
				
				if (event.TouchInput.ID == 0)
				{
					simulate_mouse = true;
					mouse_pos.X = event.TouchInput.X;
					mouse_pos.Y = event.TouchInput.Y;
				}
			}

			status = 1;
		}
		
		// Simulate mouse event for first finger on multitouch device.
		// This allows to click on GUI elements.		
		if (simulate_mouse)
		{
			device->CursorControl->setPosition(mouse_pos);

			SEvent irrevent;
			bool send_event = true;
			
			switch (event.TouchInput.Event)
			{
			case ETIE_PRESSED_DOWN:
				irrevent.MouseInput.Event = EMIE_LMOUSE_PRESSED_DOWN;
				device->IsMousePressed = true;
				break;
			case ETIE_LEFT_UP:
				irrevent.MouseInput.Event = EMIE_LMOUSE_LEFT_UP;
				device->IsMousePressed = false;
				break;
			case ETIE_MOVED:
				irrevent.MouseInput.Event = EMIE_MOUSE_MOVED;
				break;
			default:
				send_event = false;
				break;
			}
			
			if (send_event)
			{
				irrevent.MouseInput.Control = false;
				irrevent.MouseInput.Shift = false;
				irrevent.MouseInput.ButtonStates = device->IsMousePressed ? 
														irr::EMBSM_LEFT : 0;
				irrevent.EventType = EET_MOUSE_INPUT_EVENT;
				irrevent.MouseInput.X = mouse_pos.X;
				irrevent.MouseInput.Y = mouse_pos.Y;

				device->postEventFromUser(irrevent);
			}
		}
		
		break;
	}
	case AINPUT_EVENT_TYPE_KEY:
	{
		bool ignore_event = false;

		SEvent event;
		event.EventType = EET_KEY_INPUT_EVENT;
		event.KeyInput.Char = 0;
		event.KeyInput.PressedDown = false;
		event.KeyInput.Key = IRR_KEY_UNKNOWN;

		int32_t keyCode = AKeyEvent_getKeyCode(androidEvent);
		int32_t keyAction = AKeyEvent_getAction(androidEvent);
		int32_t keyMetaState = AKeyEvent_getMetaState(androidEvent);
		int32_t keyRepeat = AKeyEvent_getRepeatCount(androidEvent);

		if (keyAction == AKEY_EVENT_ACTION_DOWN)
		{
			event.KeyInput.PressedDown = true;
		}
		else if (keyAction == AKEY_EVENT_ACTION_UP)
		{
			event.KeyInput.PressedDown = false;
		}
		else if (keyAction == AKEY_EVENT_ACTION_MULTIPLE)
		{
			// TODO: Multiple duplicate key events have occurred in a row,
			// or a complex string is being delivered. The repeat_count
			// property of the key event contains the number of times the
			// given key code should be executed.
			// I guess this might necessary for more complicated i18n key input,
			// but don't see yet how to handle this correctly.
		}

		event.KeyInput.Shift = (keyMetaState & AMETA_SHIFT_ON ||
								keyMetaState & AMETA_SHIFT_LEFT_ON ||
								keyMetaState & AMETA_SHIFT_RIGHT_ON);

		event.KeyInput.Control = (keyMetaState & AMETA_CTRL_ON ||
								  keyMetaState & AMETA_CTRL_LEFT_ON ||
								  keyMetaState & AMETA_CTRL_RIGHT_ON);

		event.KeyInput.SystemKeyCode = (u32)keyCode;

		if (keyCode >= 0 && (u32)keyCode < device->KeyMap.size())
		{
			event.KeyInput.Key = device->KeyMap[keyCode];
		}

		if (event.KeyInput.Key > 0)
		{
			device->getKeyChar(event);
		}

		// Handle an event when back button in pressed just like an escape key
		// and also avoid repeating the event to avoid some strange behaviour
		if (event.KeyInput.SystemKeyCode == AKEYCODE_BACK)
		{
			status = 1;
			
			if (event.KeyInput.PressedDown == false || keyRepeat > 0)
			{
				ignore_event = true;
			}
		}

		// Mark escape key event as handled by application to avoid receiving
		// AKEYCODE_BACK key event
		if (event.KeyInput.SystemKeyCode == AKEYCODE_ESCAPE)
		{
			status = 1;
		}

		if (!ignore_event)
		{
			device->postEventFromUser(event);
		}

		break;
	}
	default:
		break;
	}

	return status;
}

video::SExposedVideoData& CIrrDeviceAndroid::getExposedVideoData()
{
	return ExposedVideoData;
}

void CIrrDeviceAndroid::createKeyMap()
{
	KeyMap.set_used(223);

	KeyMap[0] = IRR_KEY_UNKNOWN; // AKEYCODE_UNKNOWN
	KeyMap[1] = IRR_KEY_LBUTTON; // AKEYCODE_SOFT_LEFT
	KeyMap[2] = IRR_KEY_RBUTTON; // AKEYCODE_SOFT_RIGHT
	KeyMap[3] = IRR_KEY_HOME; // AKEYCODE_HOME
	KeyMap[4] = IRR_KEY_ESCAPE; // AKEYCODE_BACK
	KeyMap[5] = IRR_KEY_UNKNOWN; // AKEYCODE_CALL
	KeyMap[6] = IRR_KEY_UNKNOWN; // AKEYCODE_ENDCALL
	KeyMap[7] = IRR_KEY_0; // AKEYCODE_0
	KeyMap[8] = IRR_KEY_1; // AKEYCODE_1
	KeyMap[9] = IRR_KEY_2; // AKEYCODE_2
	KeyMap[10] = IRR_KEY_3; // AKEYCODE_3
	KeyMap[11] = IRR_KEY_4; // AKEYCODE_4
	KeyMap[12] = IRR_KEY_5; // AKEYCODE_5
	KeyMap[13] = IRR_KEY_6; // AKEYCODE_6
	KeyMap[14] = IRR_KEY_7; // AKEYCODE_7
	KeyMap[15] = IRR_KEY_8; // AKEYCODE_8
	KeyMap[16] = IRR_KEY_9; // AKEYCODE_9
	KeyMap[17] = IRR_KEY_UNKNOWN; // AKEYCODE_STAR
	KeyMap[18] = IRR_KEY_UNKNOWN; // AKEYCODE_POUND
	KeyMap[19] = IRR_KEY_UP; // AKEYCODE_DPAD_UP
	KeyMap[20] = IRR_KEY_DOWN; // AKEYCODE_DPAD_DOWN
	KeyMap[21] = IRR_KEY_LEFT; // AKEYCODE_DPAD_LEFT
	KeyMap[22] = IRR_KEY_RIGHT; // AKEYCODE_DPAD_RIGHT
	KeyMap[23] = IRR_KEY_SELECT; // AKEYCODE_DPAD_CENTER
	KeyMap[24] = IRR_KEY_VOLUME_DOWN; // AKEYCODE_VOLUME_UP
	KeyMap[25] = IRR_KEY_VOLUME_UP; // AKEYCODE_VOLUME_DOWN
	KeyMap[26] = IRR_KEY_UNKNOWN; // AKEYCODE_POWER
	KeyMap[27] = IRR_KEY_UNKNOWN; // AKEYCODE_CAMERA
	KeyMap[28] = IRR_KEY_CLEAR; // AKEYCODE_CLEAR
	KeyMap[29] = IRR_KEY_A; // AKEYCODE_A
	KeyMap[30] = IRR_KEY_B; // AKEYCODE_B
	KeyMap[31] = IRR_KEY_C; // AKEYCODE_C
	KeyMap[32] = IRR_KEY_D; // AKEYCODE_D
	KeyMap[33] = IRR_KEY_E; // AKEYCODE_E
	KeyMap[34] = IRR_KEY_F; // AKEYCODE_F
	KeyMap[35] = IRR_KEY_G; // AKEYCODE_G
	KeyMap[36] = IRR_KEY_H; // AKEYCODE_H
	KeyMap[37] = IRR_KEY_I; // AKEYCODE_I
	KeyMap[38] = IRR_KEY_J; // AKEYCODE_J
	KeyMap[39] = IRR_KEY_K; // AKEYCODE_K
	KeyMap[40] = IRR_KEY_L; // AKEYCODE_L
	KeyMap[41] = IRR_KEY_M; // AKEYCODE_M
	KeyMap[42] = IRR_KEY_N; // AKEYCODE_N
	KeyMap[43] = IRR_KEY_O; // AKEYCODE_O
	KeyMap[44] = IRR_KEY_P; // AKEYCODE_P
	KeyMap[45] = IRR_KEY_Q; // AKEYCODE_Q
	KeyMap[46] = IRR_KEY_R; // AKEYCODE_R
	KeyMap[47] = IRR_KEY_S; // AKEYCODE_S
	KeyMap[48] = IRR_KEY_T; // AKEYCODE_T
	KeyMap[49] = IRR_KEY_U; // AKEYCODE_U
	KeyMap[50] = IRR_KEY_V; // AKEYCODE_V
	KeyMap[51] = IRR_KEY_W; // AKEYCODE_W
	KeyMap[52] = IRR_KEY_X; // AKEYCODE_X
	KeyMap[53] = IRR_KEY_Y; // AKEYCODE_Y
	KeyMap[54] = IRR_KEY_Z; // AKEYCODE_Z
	KeyMap[55] = IRR_KEY_COMMA; // AKEYCODE_COMMA
	KeyMap[56] = IRR_KEY_PERIOD; // AKEYCODE_PERIOD
	KeyMap[57] = IRR_KEY_MENU; // AKEYCODE_ALT_LEFT
	KeyMap[58] = IRR_KEY_MENU; // AKEYCODE_ALT_RIGHT
	KeyMap[59] = IRR_KEY_LSHIFT; // AKEYCODE_SHIFT_LEFT
	KeyMap[60] = IRR_KEY_RSHIFT; // AKEYCODE_SHIFT_RIGHT
	KeyMap[61] = IRR_KEY_TAB; // AKEYCODE_TAB
	KeyMap[62] = IRR_KEY_SPACE; // AKEYCODE_SPACE
	KeyMap[63] = IRR_KEY_UNKNOWN; // AKEYCODE_SYM
	KeyMap[64] = IRR_KEY_UNKNOWN; // AKEYCODE_EXPLORER
	KeyMap[65] = IRR_KEY_UNKNOWN; // AKEYCODE_ENVELOPE
	KeyMap[66] = IRR_KEY_RETURN; // AKEYCODE_ENTER
	KeyMap[67] = IRR_KEY_BACK; // AKEYCODE_DEL
	KeyMap[68] = IRR_KEY_OEM_3; // AKEYCODE_GRAVE
	KeyMap[69] = IRR_KEY_MINUS; // AKEYCODE_MINUS
	KeyMap[70] = IRR_KEY_UNKNOWN; // AKEYCODE_EQUALS
	KeyMap[71] = IRR_KEY_UNKNOWN; // AKEYCODE_LEFT_BRACKET
	KeyMap[72] = IRR_KEY_UNKNOWN; // AKEYCODE_RIGHT_BRACKET
	KeyMap[73] = IRR_KEY_UNKNOWN; // AKEYCODE_BACKSLASH
	KeyMap[74] = IRR_KEY_UNKNOWN; // AKEYCODE_SEMICOLON
	KeyMap[75] = IRR_KEY_UNKNOWN; // AKEYCODE_APOSTROPHE
	KeyMap[76] = IRR_KEY_UNKNOWN; // AKEYCODE_SLASH
	KeyMap[77] = IRR_KEY_UNKNOWN; // AKEYCODE_AT
	KeyMap[78] = IRR_KEY_UNKNOWN; // AKEYCODE_NUM
	KeyMap[79] = IRR_KEY_UNKNOWN; // AKEYCODE_HEADSETHOOK
	KeyMap[80] = IRR_KEY_UNKNOWN; // AKEYCODE_FOCUS (*Camera* focus)
	KeyMap[81] = IRR_KEY_PLUS; // AKEYCODE_PLUS
	KeyMap[82] = IRR_KEY_MENU; // AKEYCODE_MENU
	KeyMap[83] = IRR_KEY_UNKNOWN; // AKEYCODE_NOTIFICATION
	KeyMap[84] = IRR_KEY_UNKNOWN; // AKEYCODE_SEARCH
	KeyMap[85] = IRR_KEY_MEDIA_PLAY_PAUSE; // AKEYCODE_MEDIA_PLAY_PAUSE
	KeyMap[86] = IRR_KEY_MEDIA_STOP; // AKEYCODE_MEDIA_STOP
	KeyMap[87] = IRR_KEY_MEDIA_NEXT_TRACK; // AKEYCODE_MEDIA_NEXT
	KeyMap[88] = IRR_KEY_MEDIA_PREV_TRACK; // AKEYCODE_MEDIA_PREVIOUS
	KeyMap[89] = IRR_KEY_UNKNOWN; // AKEYCODE_MEDIA_REWIND
	KeyMap[90] = IRR_KEY_UNKNOWN; // AKEYCODE_MEDIA_FAST_FORWARD
	KeyMap[91] = IRR_KEY_VOLUME_MUTE; // AKEYCODE_MUTE
	KeyMap[92] = IRR_KEY_PRIOR; // AKEYCODE_PAGE_UP
	KeyMap[93] = IRR_KEY_NEXT; // AKEYCODE_PAGE_DOWN
	KeyMap[94] = IRR_KEY_UNKNOWN; // AKEYCODE_PICTSYMBOLS
	KeyMap[95] = IRR_KEY_UNKNOWN; // AKEYCODE_SWITCH_CHARSET

	// following look like controller inputs
	KeyMap[96] = IRR_KEY_UNKNOWN; // AKEYCODE_BUTTON_A
	KeyMap[97] = IRR_KEY_UNKNOWN; // AKEYCODE_BUTTON_B
	KeyMap[98] = IRR_KEY_UNKNOWN; // AKEYCODE_BUTTON_C
	KeyMap[99] = IRR_KEY_UNKNOWN; // AKEYCODE_BUTTON_X
	KeyMap[100] = IRR_KEY_UNKNOWN; // AKEYCODE_BUTTON_Y
	KeyMap[101] = IRR_KEY_UNKNOWN; // AKEYCODE_BUTTON_Z
	KeyMap[102] = IRR_KEY_UNKNOWN; // AKEYCODE_BUTTON_L1
	KeyMap[103] = IRR_KEY_UNKNOWN; // AKEYCODE_BUTTON_R1
	KeyMap[104] = IRR_KEY_UNKNOWN; // AKEYCODE_BUTTON_L2
	KeyMap[105] = IRR_KEY_UNKNOWN; // AKEYCODE_BUTTON_R2
	KeyMap[106] = IRR_KEY_UNKNOWN; // AKEYCODE_BUTTON_THUMBL
	KeyMap[107] = IRR_KEY_UNKNOWN; // AKEYCODE_BUTTON_THUMBR
	KeyMap[108] = IRR_KEY_UNKNOWN; // AKEYCODE_BUTTON_START
	KeyMap[109] = IRR_KEY_UNKNOWN; // AKEYCODE_BUTTON_SELECT
	KeyMap[110] = IRR_KEY_UNKNOWN; // AKEYCODE_BUTTON_MODE

	KeyMap[111] = IRR_KEY_ESCAPE; // AKEYCODE_ESCAPE
	KeyMap[112] = IRR_KEY_DELETE; // AKEYCODE_FORWARD_DEL
	KeyMap[113] = IRR_KEY_CONTROL; // AKEYCODE_CTRL_LEFT
	KeyMap[114] = IRR_KEY_CONTROL; // AKEYCODE_CTRL_RIGHT
	KeyMap[115] = IRR_KEY_CAPITAL; // AKEYCODE_CAPS_LOCK
	KeyMap[116] = IRR_KEY_SCROLL; // AKEYCODE_SCROLL_LOCK
	KeyMap[117] = IRR_KEY_UNKNOWN; // AKEYCODE_META_LEFT
	KeyMap[118] = IRR_KEY_UNKNOWN; // AKEYCODE_META_RIGHT
	KeyMap[119] = IRR_KEY_UNKNOWN; // AKEYCODE_FUNCTION
	KeyMap[120] = IRR_KEY_SNAPSHOT; // AKEYCODE_SYSRQ
	KeyMap[121] = IRR_KEY_PAUSE; // AKEYCODE_BREAK
	KeyMap[122] = IRR_KEY_HOME; // AKEYCODE_MOVE_HOME
	KeyMap[123] = IRR_KEY_END; // AKEYCODE_MOVE_END
	KeyMap[124] = IRR_KEY_INSERT; // AKEYCODE_INSERT
	KeyMap[125] = IRR_KEY_UNKNOWN; // AKEYCODE_FORWARD
	KeyMap[126] = IRR_KEY_PLAY; // AKEYCODE_MEDIA_PLAY
	KeyMap[127] = IRR_KEY_MEDIA_PLAY_PAUSE; // AKEYCODE_MEDIA_PAUSE
	KeyMap[128] = IRR_KEY_UNKNOWN; // AKEYCODE_MEDIA_CLOSE
	KeyMap[129] = IRR_KEY_UNKNOWN; // AKEYCODE_MEDIA_EJECT
	KeyMap[130] = IRR_KEY_UNKNOWN; // AKEYCODE_MEDIA_RECORD
	KeyMap[131] = IRR_KEY_F1; // AKEYCODE_F1
	KeyMap[132] = IRR_KEY_F2; // AKEYCODE_F2
	KeyMap[133] = IRR_KEY_F3; // AKEYCODE_F3
	KeyMap[134] = IRR_KEY_F4; // AKEYCODE_F4
	KeyMap[135] = IRR_KEY_F5; // AKEYCODE_F5
	KeyMap[136] = IRR_KEY_F6; // AKEYCODE_F6
	KeyMap[137] = IRR_KEY_F7; // AKEYCODE_F7
	KeyMap[138] = IRR_KEY_F8; // AKEYCODE_F8
	KeyMap[139] = IRR_KEY_F9; // AKEYCODE_F9
	KeyMap[140] = IRR_KEY_F10; // AKEYCODE_F10
	KeyMap[141] = IRR_KEY_F11; // AKEYCODE_F11
	KeyMap[142] = IRR_KEY_F12; // AKEYCODE_F12
	KeyMap[143] = IRR_KEY_NUMLOCK; // AKEYCODE_NUM_LOCK
	KeyMap[144] = IRR_KEY_NUMPAD0; // AKEYCODE_NUMPAD_0
	KeyMap[145] = IRR_KEY_NUMPAD1; // AKEYCODE_NUMPAD_1
	KeyMap[146] = IRR_KEY_NUMPAD2; // AKEYCODE_NUMPAD_2
	KeyMap[147] = IRR_KEY_NUMPAD3; // AKEYCODE_NUMPAD_3
	KeyMap[148] = IRR_KEY_NUMPAD4; // AKEYCODE_NUMPAD_4
	KeyMap[149] = IRR_KEY_NUMPAD5; // AKEYCODE_NUMPAD_5
	KeyMap[150] = IRR_KEY_NUMPAD6; // AKEYCODE_NUMPAD_6
	KeyMap[151] = IRR_KEY_NUMPAD7; // AKEYCODE_NUMPAD_7
	KeyMap[152] = IRR_KEY_NUMPAD8; // AKEYCODE_NUMPAD_8
	KeyMap[153] = IRR_KEY_NUMPAD9; // AKEYCODE_NUMPAD_9
	KeyMap[154] = IRR_KEY_DIVIDE; // AKEYCODE_NUMPAD_DIVIDE
	KeyMap[155] = IRR_KEY_MULTIPLY; // AKEYCODE_NUMPAD_MULTIPLY
	KeyMap[156] = IRR_KEY_SUBTRACT; // AKEYCODE_NUMPAD_SUBTRACT
	KeyMap[157] = IRR_KEY_ADD; // AKEYCODE_NUMPAD_ADD
	KeyMap[158] = IRR_KEY_UNKNOWN; // AKEYCODE_NUMPAD_DOT
	KeyMap[159] = IRR_KEY_COMMA; // AKEYCODE_NUMPAD_COMMA
	KeyMap[160] = IRR_KEY_RETURN; // AKEYCODE_NUMPAD_ENTER
	KeyMap[161] = IRR_KEY_UNKNOWN; // AKEYCODE_NUMPAD_EQUALS
	KeyMap[162] = IRR_KEY_UNKNOWN; // AKEYCODE_NUMPAD_LEFT_PAREN
	KeyMap[163] = IRR_KEY_UNKNOWN; // AKEYCODE_NUMPAD_RIGHT_PAREN
	KeyMap[164] = IRR_KEY_VOLUME_MUTE; // AKEYCODE_VOLUME_MUTE
	KeyMap[165] = IRR_KEY_UNKNOWN; // AKEYCODE_INFO
	KeyMap[166] = IRR_KEY_UNKNOWN; // AKEYCODE_CHANNEL_UP
	KeyMap[167] = IRR_KEY_UNKNOWN; // AKEYCODE_CHANNEL_DOWN
	KeyMap[168] = IRR_KEY_ZOOM; // AKEYCODE_ZOOM_IN
	KeyMap[169] = IRR_KEY_UNKNOWN; // AKEYCODE_ZOOM_OUT
	KeyMap[170] = IRR_KEY_UNKNOWN; // AKEYCODE_TV
	KeyMap[171] = IRR_KEY_UNKNOWN; // AKEYCODE_WINDOW
	KeyMap[172] = IRR_KEY_UNKNOWN; // AKEYCODE_GUIDE
	KeyMap[173] = IRR_KEY_UNKNOWN; // AKEYCODE_DVR
	KeyMap[174] = IRR_KEY_UNKNOWN; // AKEYCODE_BOOKMARK
	KeyMap[175] = IRR_KEY_UNKNOWN; // AKEYCODE_CAPTIONS
	KeyMap[176] = IRR_KEY_UNKNOWN; // AKEYCODE_SETTINGS
	KeyMap[177] = IRR_KEY_UNKNOWN; // AKEYCODE_TV_POWER
	KeyMap[178] = IRR_KEY_UNKNOWN; // AKEYCODE_TV_INPUT
	KeyMap[179] = IRR_KEY_UNKNOWN; // AKEYCODE_STB_POWER
	KeyMap[180] = IRR_KEY_UNKNOWN; // AKEYCODE_STB_INPUT
	KeyMap[181] = IRR_KEY_UNKNOWN; // AKEYCODE_AVR_POWER
	KeyMap[182] = IRR_KEY_UNKNOWN; // AKEYCODE_AVR_INPUT
	KeyMap[183] = IRR_KEY_UNKNOWN; // AKEYCODE_PROG_RED
	KeyMap[184] = IRR_KEY_UNKNOWN; // AKEYCODE_PROG_GREEN
	KeyMap[185] = IRR_KEY_UNKNOWN; // AKEYCODE_PROG_YELLOW
	KeyMap[186] = IRR_KEY_UNKNOWN; // AKEYCODE_PROG_BLUE
	KeyMap[187] = IRR_KEY_UNKNOWN; // AKEYCODE_APP_SWITCH
	KeyMap[188] = IRR_KEY_UNKNOWN; // AKEYCODE_BUTTON_1
	KeyMap[189] = IRR_KEY_UNKNOWN; // AKEYCODE_BUTTON_2
	KeyMap[190] = IRR_KEY_UNKNOWN; // AKEYCODE_BUTTON_3
	KeyMap[191] = IRR_KEY_UNKNOWN; // AKEYCODE_BUTTON_4
	KeyMap[192] = IRR_KEY_UNKNOWN; // AKEYCODE_BUTTON_5
	KeyMap[193] = IRR_KEY_UNKNOWN; // AKEYCODE_BUTTON_6
	KeyMap[194] = IRR_KEY_UNKNOWN; // AKEYCODE_BUTTON_7
	KeyMap[195] = IRR_KEY_UNKNOWN; // AKEYCODE_BUTTON_8
	KeyMap[196] = IRR_KEY_UNKNOWN; // AKEYCODE_BUTTON_9
	KeyMap[197] = IRR_KEY_UNKNOWN; // AKEYCODE_BUTTON_10
	KeyMap[198] = IRR_KEY_UNKNOWN; // AKEYCODE_BUTTON_11
	KeyMap[199] = IRR_KEY_UNKNOWN; // AKEYCODE_BUTTON_12
	KeyMap[200] = IRR_KEY_UNKNOWN; // AKEYCODE_BUTTON_13
	KeyMap[201] = IRR_KEY_UNKNOWN; // AKEYCODE_BUTTON_14
	KeyMap[202] = IRR_KEY_UNKNOWN; // AKEYCODE_BUTTON_15
	KeyMap[203] = IRR_KEY_UNKNOWN; // AKEYCODE_BUTTON_16
	KeyMap[204] = IRR_KEY_UNKNOWN; // AKEYCODE_LANGUAGE_SWITCH
	KeyMap[205] = IRR_KEY_UNKNOWN; // AKEYCODE_MANNER_MODE
	KeyMap[206] = IRR_KEY_UNKNOWN; // AKEYCODE_3D_MODE
	KeyMap[207] = IRR_KEY_UNKNOWN; // AKEYCODE_CONTACTS
	KeyMap[208] = IRR_KEY_UNKNOWN; // AKEYCODE_CALENDAR
	KeyMap[209] = IRR_KEY_UNKNOWN; // AKEYCODE_MUSIC
	KeyMap[210] = IRR_KEY_UNKNOWN; // AKEYCODE_CALCULATOR
	KeyMap[211] = IRR_KEY_UNKNOWN; // AKEYCODE_ZENKAKU_HANKAKU
	KeyMap[212] = IRR_KEY_UNKNOWN; // AKEYCODE_EISU
	KeyMap[213] = IRR_KEY_UNKNOWN; // AKEYCODE_MUHENKAN
	KeyMap[214] = IRR_KEY_UNKNOWN; // AKEYCODE_HENKAN
	KeyMap[215] = IRR_KEY_UNKNOWN; // AKEYCODE_KATAKANA_HIRAGANA
	KeyMap[216] = IRR_KEY_UNKNOWN; // AKEYCODE_YEN
	KeyMap[217] = IRR_KEY_UNKNOWN; // AKEYCODE_RO
	KeyMap[218] = IRR_KEY_UNKNOWN; // AKEYCODE_KANA
	KeyMap[219] = IRR_KEY_UNKNOWN; // AKEYCODE_ASSIST
	KeyMap[220] = IRR_KEY_UNKNOWN; // AKEYCODE_BRIGHTNESS_DOWN
	KeyMap[221] = IRR_KEY_UNKNOWN; // AKEYCODE_BRIGHTNESS_UP ,
	KeyMap[222] = IRR_KEY_UNKNOWN; // AKEYCODE_MEDIA_AUDIO_TRACK
}

void CIrrDeviceAndroid::getKeyChar(SEvent& event)
{
	// Handle ASCII chars

	event.KeyInput.Char = 0;

	// A-Z
	if (event.KeyInput.SystemKeyCode > 28 && event.KeyInput.SystemKeyCode < 55)
	{
		if (event.KeyInput.Shift)
		{
			event.KeyInput.Char = event.KeyInput.SystemKeyCode + 36;
		}
		else
		{
			event.KeyInput.Char = event.KeyInput.SystemKeyCode + 68;
		}
	}

	// 0-9
	else if (event.KeyInput.SystemKeyCode > 6 && event.KeyInput.SystemKeyCode < 17)
	{
		event.KeyInput.Char = event.KeyInput.SystemKeyCode + 41;
	}

	else if (event.KeyInput.SystemKeyCode == AKEYCODE_BACK)
	{
		event.KeyInput.Char =  8;
	}
	else if (event.KeyInput.SystemKeyCode == AKEYCODE_DEL)
	{
		event.KeyInput.Char =  8;
	}
	else if (event.KeyInput.SystemKeyCode == AKEYCODE_TAB)
	{
		event.KeyInput.Char =  9;
	}
	else if (event.KeyInput.SystemKeyCode == AKEYCODE_ENTER)
	{
		event.KeyInput.Char =  13;
	}
	else if (event.KeyInput.SystemKeyCode == AKEYCODE_SPACE)
	{
		event.KeyInput.Char =  32;
	}
	else if (event.KeyInput.SystemKeyCode == AKEYCODE_COMMA)
	{
		event.KeyInput.Char =  44;
	}
	else if (event.KeyInput.SystemKeyCode == AKEYCODE_PERIOD)
	{
		event.KeyInput.Char =  46;
	}
}

bool CIrrDeviceAndroid::activateAccelerometer(float updateInterval)
{
	if (!isAccelerometerAvailable())
		return false;

	ASensorEventQueue_enableSensor(SensorEventQueue, Accelerometer);
	ASensorEventQueue_setEventRate(SensorEventQueue, Accelerometer,
					(int32_t)(updateInterval*1000.f*1000.f)); // in microseconds

	os::Printer::log("Activated accelerometer", ELL_DEBUG);
	return true;
}

bool CIrrDeviceAndroid::deactivateAccelerometer()
{
	if (!Accelerometer)
		return false;

	ASensorEventQueue_disableSensor(SensorEventQueue, Accelerometer);
	Accelerometer = 0;
	os::Printer::log("Deactivated accelerometer", ELL_DEBUG);
	return true;
}

bool CIrrDeviceAndroid::isAccelerometerActive()
{
	return (Accelerometer != NULL);
}

bool CIrrDeviceAndroid::isAccelerometerAvailable()
{
	if (!Accelerometer)
	{
		Accelerometer = ASensorManager_getDefaultSensor(SensorManager,
													ASENSOR_TYPE_ACCELEROMETER);
	}

	return (Accelerometer != NULL);
}

bool CIrrDeviceAndroid::activateGyroscope(float updateInterval)
{
	if (!isGyroscopeAvailable())
		return false;

	ASensorEventQueue_enableSensor(SensorEventQueue, Gyroscope);
	ASensorEventQueue_setEventRate(SensorEventQueue, Gyroscope,
					(int32_t)(updateInterval*1000.f*1000.f)); // in microseconds

	os::Printer::log("Activated gyroscope", ELL_DEBUG);
	return true;
}

bool CIrrDeviceAndroid::deactivateGyroscope()
{
	if (!Gyroscope)
		return false;

	ASensorEventQueue_disableSensor(SensorEventQueue, Gyroscope);
	Gyroscope = 0;
	os::Printer::log("Deactivated gyroscope", ELL_DEBUG);
	return true;
}

bool CIrrDeviceAndroid::isGyroscopeActive()
{
	return (Gyroscope != NULL);
}

bool CIrrDeviceAndroid::isGyroscopeAvailable()
{
	if (!Gyroscope)
	{
		Gyroscope = ASensorManager_getDefaultSensor(SensorManager,
													ASENSOR_TYPE_GYROSCOPE);
	}

	return (Gyroscope != NULL);
}


} // end namespace irr

#endif


