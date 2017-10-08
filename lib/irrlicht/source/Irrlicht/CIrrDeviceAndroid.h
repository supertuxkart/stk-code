// Copyright (C) 2002-2011 Nikolaus Gebhardt
// Copyright (C) 2016-2017 Dawid Gan
// This file is part of the "Irrlicht Engine".
// For conditions of distribution and use, see copyright notice in irrlicht.h

#ifndef __C_IRR_DEVICE_ANDROID_H_INCLUDED__
#define __C_IRR_DEVICE_ANDROID_H_INCLUDED__

#include "IrrCompileConfig.h"

#ifdef _IRR_COMPILE_WITH_ANDROID_DEVICE_

#include <android/window.h>
#include <android/sensor.h>
#include <android_native_app_glue.h>
#include "CIrrDeviceStub.h"
#include "IrrlichtDevice.h"
#include "IImagePresenter.h"
#include "ICursorControl.h"

#include <map>


namespace irr
{

	class CIrrDeviceAndroid : public CIrrDeviceStub, video::IImagePresenter
	{
	public:
		//! constructor
		CIrrDeviceAndroid(const SIrrlichtCreationParameters& param);

		//! destructor
		virtual ~CIrrDeviceAndroid();

		virtual bool run();
		virtual void yield();
		virtual void sleep(u32 timeMs, bool pauseTimer=false);
		virtual void setWindowCaption(const wchar_t* text);
		virtual void setWindowClass(const char* text) {}
		virtual bool present(video::IImage* surface, void* windowId, core::rect<s32>* srcClip);
		virtual bool isWindowActive() const;
		virtual bool isWindowFocused() const;
		virtual bool isWindowMinimized() const;
		virtual void closeDevice();
		virtual void setResizable( bool resize=false );
		virtual void minimizeWindow();
		virtual void maximizeWindow();
		virtual void restoreWindow();
		virtual bool moveWindow(int x, int y);
		virtual bool getWindowPosition(int* x, int* y);
		virtual E_DEVICE_TYPE getType() const;
		virtual bool activateAccelerometer(float updateInterval);
		virtual bool deactivateAccelerometer();
		virtual bool isAccelerometerActive();
		virtual bool isAccelerometerAvailable();
		virtual bool activateGyroscope(float updateInterval);
		virtual bool deactivateGyroscope();
		virtual bool isGyroscopeActive();
		virtual bool isGyroscopeAvailable();
		
		class CCursorControl : public gui::ICursorControl
		{
		public:

			CCursorControl() : CursorPos(core::position2d<s32>(0, 0)) {}
			virtual void setVisible(bool visible) {}
			virtual bool isVisible() const {return false;}
			virtual void setPosition(const core::position2d<f32> &pos)
			{
				setPosition(pos.X, pos.Y);
			}
			virtual void setPosition(f32 x, f32 y) 
			{
				CursorPos.X = x;
				CursorPos.Y = y;
			}
			virtual void setPosition(const core::position2d<s32> &pos)
			{
				setPosition(pos.X, pos.Y);
			}
			virtual void setPosition(s32 x, s32 y)
			{
				CursorPos.X = x;
				CursorPos.Y = y;
			}
			virtual const core::position2d<s32>& getPosition() 
			{
				return CursorPos;
			}
			virtual core::position2d<f32> getRelativePosition()
			{
				return core::position2d<f32>(0, 0);
			}
			virtual void setReferenceRect(core::rect<s32>* rect=0) {}
		private:
			core::position2d<s32> CursorPos;
		};
		
		static void onCreate();

	private:
		android_app* Android;
		ASensorManager* SensorManager;
		ASensorEventQueue* SensorEventQueue;
		const ASensor* Accelerometer;
		const ASensor* Gyroscope;

		static bool IsPaused;
		static bool IsFocused;
		static bool IsStarted;
		
		bool IsMousePressed;

		video::SExposedVideoData ExposedVideoData;

		std::map<int, EKEY_CODE> KeyMap;
		
		void createDriver();
		void createKeyMap();
		void createVideoModeList();
		void getKeyChar(SEvent& event);
		video::SExposedVideoData& getExposedVideoData();
		
		static void handleAndroidCommand(android_app* app, int32_t cmd);
		static s32 handleInput(android_app* app, AInputEvent* event);
	};

} // end namespace irr

#endif // _IRR_COMPILE_WITH_ANDROID_DEVICE_
#endif // __C_IRR_DEVICE_ANDROID_H_INCLUDED__
