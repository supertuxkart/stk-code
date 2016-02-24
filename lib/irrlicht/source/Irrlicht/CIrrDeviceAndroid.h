// Copyright (C) 2002-2011 Nikolaus Gebhardt
// This file is part of the "Irrlicht Engine".
// For conditions of distribution and use, see copyright notice in irrlicht.h

#ifndef __C_IRR_DEVICE_ANDROID_H_INCLUDED__
#define __C_IRR_DEVICE_ANDROID_H_INCLUDED__

#include "IrrCompileConfig.h"

#ifdef _IRR_COMPILE_WITH_ANDROID_DEVICE_

#include <android/sensor.h>
#include <android_native_app_glue.h>
#include "CIrrDeviceStub.h"
#include "IrrlichtDevice.h"
#include "IImagePresenter.h"
#include "ICursorControl.h"

namespace irr
{

	class CIrrDeviceAndroid : public CIrrDeviceStub, video::IImagePresenter
	{
	public:

		//! constructor
		CIrrDeviceAndroid(const SIrrlichtCreationParameters& param);

		//! destructor
		virtual ~CIrrDeviceAndroid();

		virtual bool run( void );
		virtual void yield( void );
		virtual void sleep( u32 timeMs, bool pauseTimer=false );
		virtual void setWindowCaption(const wchar_t* text);
		virtual bool present(video::IImage* surface, void* windowId, core::rect<s32>* srcClip)	;
		virtual bool isWindowActive( void ) const;
		virtual bool isWindowFocused( void ) const;
		virtual bool isWindowMinimized( void ) const;
		virtual void closeDevice( void );
		virtual void setResizable( bool resize=false );
		virtual void minimizeWindow( void );
		virtual void maximizeWindow( void );
		virtual void restoreWindow( void );
		virtual E_DEVICE_TYPE getType( void ) const;

		//! Implementation of the linux cursor control
		class CCursorControl : public gui::ICursorControl
		{
		public:

			CCursorControl(CIrrDeviceAndroid* dev)
				: Device(dev), IsVisible(true)
			{
			}

			//! Changes the visible state of the mouse cursor.
			virtual void setVisible(bool visible)
			{
				IsVisible = visible;
				//if ( visible )
				//	SDL_ShowCursor( SDL_ENABLE );
				//else
				//	SDL_ShowCursor( SDL_DISABLE );
			}

			//! Returns if the cursor is currently visible.
			virtual bool isVisible() const
			{
				return IsVisible;
			}

			//! Sets the new position of the cursor.
			virtual void setPosition(const core::position2d<f32> &pos)
			{
				setPosition(pos.X, pos.Y);
			}

			//! Sets the new position of the cursor.
			virtual void setPosition(f32 x, f32 y)
			{
				setPosition((s32)(x*Device->DeviceWidth), (s32)(y*Device->DeviceHeight));
			}

			//! Sets the new position of the cursor.
			virtual void setPosition(const core::position2d<s32> &pos)
			{
				setPosition(pos.X, pos.Y);
			}

			//! Sets the new position of the cursor.
			virtual void setPosition(s32 x, s32 y)
			{
				//SDL_WarpMouse( x, y );
			}

			//! Returns the current position of the mouse cursor.
			virtual const core::position2d<s32>& getPosition()
			{
				updateCursorPos();
				return CursorPos;
			}

			//! Returns the current position of the mouse cursor.
			virtual core::position2d<f32> getRelativePosition()
			{
				updateCursorPos();
				return core::position2d<f32>(CursorPos.X / (f32)Device->DeviceWidth,
					CursorPos.Y / (f32)Device->DeviceHeight);
			}

			virtual void setReferenceRect(core::rect<s32>* rect=0)
			{
			}

		private:

			void updateCursorPos()
			{
				CursorPos.X = Device->MouseX;
				CursorPos.Y = Device->MouseY;

				if (CursorPos.X < 0)
					CursorPos.X = 0;
				if (CursorPos.X > (s32)Device->DeviceWidth)
					CursorPos.X = Device->DeviceWidth;
				if (CursorPos.Y < 0)
					CursorPos.Y = 0;
				if (CursorPos.Y > (s32)Device->DeviceHeight)
					CursorPos.Y = Device->DeviceHeight;
			}

			CIrrDeviceAndroid* Device;
			core::position2d<s32> CursorPos;
			bool IsVisible;
		};

        static android_app *getAndroidApp() { return Android; }
	private:
	
		static android_app *Android;
		ASensorManager *SensorManager;
		ASensorEventQueue *SensorEventQueue;
		bool Animating;
		bool IsReady;
		bool IsClosing;
		u32 DeviceWidth, DeviceHeight;
		u32 MouseX, MouseY;
		void createDriver( void );
		static void handleAndroidCommand( struct android_app* app, s32 cmd );
		static s32 handleInput( struct android_app* app, AInputEvent* event );
	};

} // end namespace irr

#endif // _IRR_COMPILE_WITH_ANDROID_DEVICE_
#endif // __C_IRR_DEVICE_ANDROID_H_INCLUDED__
