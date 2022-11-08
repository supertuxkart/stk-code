// Copyright (C) 2002-2012 Nikolaus Gebhardt
// This file is part of the "Irrlicht Engine".
// For conditions of distribution and use, see copyright notice in irrlicht.h
// This device code is based on the original SDL device implementation
// contributed by Shane Parker (sirshane).

#ifndef __C_IRR_DEVICE_SDL_H_INCLUDED__
#define __C_IRR_DEVICE_SDL_H_INCLUDED__

#include "IrrCompileConfig.h"

#ifdef _IRR_COMPILE_WITH_SDL_DEVICE_

#include "IrrlichtDevice.h"
#include "CIrrDeviceStub.h"
#include "IImagePresenter.h"
#include "ICursorControl.h"

#include <SDL.h>
#include <SDL_syswm.h>
#include <map>
#include <set>

namespace irr
{
class MoltenVK;

	class CIrrDeviceSDL : public CIrrDeviceStub, video::IImagePresenter
	{
	public:
		//! constructor
		CIrrDeviceSDL(const SIrrlichtCreationParameters& param);

		//! destructor
		virtual ~CIrrDeviceSDL();

		//! runs the device. Returns false if device wants to be deleted
		virtual bool run();

		//! pause execution temporarily
		virtual void yield();

		//! pause execution for a specified time
		virtual void sleep(u32 timeMs, bool pauseTimer);

		//! sets the caption of the window
		virtual void setWindowCaption(const wchar_t* text);

		//! sets the class of the window
		virtual void setWindowClass(const char* text) {}

		//! returns if window is active. if not, nothing need to be drawn
		virtual bool isWindowActive() const;

		//! returns if window has focus.
		bool isWindowFocused() const;

		//! returns if window is minimized.
		bool isWindowMinimized() const;

		//! returns color format of the window.
		video::ECOLOR_FORMAT getColorFormat() const;

		//! presents a surface in the client area
		virtual bool present(video::IImage* surface, void* windowId=0, core::rect<s32>* src=0);

		//! notifies the device that it should close itself
		virtual void closeDevice();

		//! \return Returns a pointer to a list with all video modes supported
		video::IVideoModeList* getVideoModeList();

		//! Sets if the window should be resizable in windowed mode.
		virtual void setResizable(bool resize=false);

		virtual bool isResizable() const;

		//! Minimizes the window.
		virtual void minimizeWindow();

		//! Maximizes the window.
		virtual void maximizeWindow();

		//! Restores the window size.
		virtual void restoreWindow();
		
		//! Move window to requested position
		virtual bool moveWindow(int x, int y);

		//! Get current window position.
		virtual bool getWindowPosition(int* x, int* y);

		//! Get DPI of current display.
		virtual bool getDisplayDPI(float* ddpi, float* hdpi, float* vdpi);

		//! Activate any joysticks, and generate events for them.
		virtual bool activateJoysticks(core::array<SJoystickInfo> & joystickInfo);

		//! Set the current Gamma Value for the Display
		virtual bool setGammaRamp( f32 red, f32 green, f32 blue, f32 brightness, f32 contrast );

		//! Get the current Gamma Value for the Display
		virtual bool getGammaRamp( f32 &red, f32 &green, f32 &blue, f32 &brightness, f32 &contrast );

		virtual void setWindowMinimumSize(u32 width, u32 height);

		virtual bool supportsTouchDevice() const;

		//! Get the device type
		virtual E_DEVICE_TYPE getType() const
		{
			return EIDT_SDL;
		}

		SDL_Window* getWindow() const { return Window; }

#ifdef IOS_STK
		const SDL_SysWMinfo& getWMInfo() const { return Info; }
#endif

		virtual s32 getTopPadding();

		virtual s32 getBottomPadding();

		virtual s32 getLeftPadding();

		virtual s32 getRightPadding();

		virtual f32 getNativeScaleX() const;
		virtual f32 getNativeScaleY() const;

		virtual bool hasOnScreenKeyboard() const;

		virtual u32 getOnScreenKeyboardHeight() const;

		virtual s32 getMovedHeight() const;

		virtual bool activateAccelerometer(float updateInterval);
		virtual bool deactivateAccelerometer();
		virtual bool isAccelerometerActive();
		virtual bool isAccelerometerAvailable();
		virtual bool activateGyroscope(float updateInterval);
		virtual bool deactivateGyroscope();
		virtual bool isGyroscopeActive();
		virtual bool isGyroscopeAvailable();

		virtual void resetPaused() { clearAllTouchIds(); }
		void handleNewSize(u32 width, u32 height);

		//! Implementation of the linux cursor control
		class CCursorControl : public gui::ICursorControl
		{
		public:

			CCursorControl(CIrrDeviceSDL* dev)
				: Device(dev), IsVisible(true)
			{
			}

			//! Changes the visible state of the mouse cursor.
			virtual void setVisible(bool visible)
			{
				IsVisible = visible;
				if ( visible )
					SDL_ShowCursor( SDL_ENABLE );
				else
					SDL_ShowCursor( SDL_DISABLE );
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
				setPosition((s32)(x*Device->Width), (s32)(y*Device->Height));
			}

			//! Sets the new position of the cursor.
			virtual void setPosition(const core::position2d<s32> &pos)
			{
				setPosition(pos.X, pos.Y);
			}

			//! Sets the new position of the cursor.
			virtual void setPosition(s32 x, s32 y)
			{
				SDL_WarpMouseGlobal( x, y );
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
				return core::position2d<f32>(CursorPos.X / (f32)Device->Width,
					CursorPos.Y / (f32)Device->Height);
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
				if (CursorPos.X > (s32)Device->Width)
					CursorPos.X = Device->Width;
				if (CursorPos.Y < 0)
					CursorPos.Y = 0;
				if (CursorPos.Y > (s32)Device->Height)
					CursorPos.Y = Device->Height;
			}

			CIrrDeviceSDL* Device;
			core::position2d<s32> CursorPos;
			bool IsVisible;
		};

	private:

		//! create the driver
		void createDriver();

		bool createWindow();

		void createKeyMap();

		SDL_Window* Window;
		SDL_GLContext Context;

		s32 MouseX, MouseY;
		u32 MouseButtonStates;

		u32 Width, Height;
		std::map<SDL_FingerID, size_t> TouchIDMap;

		//! Get a unique touch id per touch, create one if it's a new touch
		size_t getTouchId(SDL_FingerID touch)
		{
			auto it = TouchIDMap.find(touch);
			if (it == TouchIDMap.end())
			{
				std::set<size_t> ids;
				for (auto& p : TouchIDMap)
					ids.insert(p.second);
				size_t cur_id = 0;
				while (true)
				{
					if (ids.find(cur_id) == ids.end())
						break;
					 cur_id++;
				}
				TouchIDMap[touch] = cur_id;
				return cur_id;
			}
			return it->second;
		}

		//! Remove a unique touch id, free it for future usage
		void removeTouchId(SDL_FingerID touch)
		{
			TouchIDMap.erase(touch);
		}

		//! Clear all unique touch ids, used when the app out focused
		void clearAllTouchIds()
		{
			TouchIDMap.clear();
		}

		f32 TopPadding;
		f32 BottomPadding;
		f32 LeftPadding;
		f32 RightPadding;
		int InitialOrientation;

		bool WindowHasFocus;
		bool WindowMinimized;
		bool Resizable;

		s32 AccelerometerIndex;
		s32 AccelerometerInstance;
		s32 GyroscopeIndex;
		s32 GyroscopeInstance;

		f32 NativeScaleX, NativeScaleY;

		struct SKeyMap
		{
			SKeyMap() {}
			SKeyMap(s32 x11, s32 win32)
				: SDLKey(x11), Win32Key(win32)
			{
			}

			s32 SDLKey;
			s32 Win32Key;

			bool operator<(const SKeyMap& o) const
			{
				return SDLKey<o.SDLKey;
			}
		};

		core::array<SKeyMap> KeyMap;
		std::map<SDL_Scancode, irr::EKEY_CODE> ScanCodeMap;
		SDL_SysWMinfo Info;
		void tryCreateOpenGLContext(u32 flags);
#ifdef DLOPEN_MOLTENVK
		MoltenVK* m_moltenvk;
#endif
		void createGUIAndVulkanScene();
		void updateNativeScale(u32* saving_width = NULL, u32* saving_height = NULL);
	};

} // end namespace irr

#endif // _IRR_COMPILE_WITH_SDL_DEVICE_
#endif // __C_IRR_DEVICE_SDL_H_INCLUDED__

