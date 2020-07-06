// Copyright (C) 2002-2012 Nikolaus Gebhardt
// This file is part of the "Irrlicht Engine".
// For conditions of distribution and use, see copyright notice in irrlicht.h
// This device code is based on the original SDL device implementation
// contributed by Shane Parker (sirshane).

#ifndef __C_IRR_DEVICE_SERVER_H_INCLUDED__
#define __C_IRR_DEVICE_SERVER_H_INCLUDED__

#include "IrrCompileConfig.h"

#ifdef SERVER_ONLY

#include "IrrlichtDevice.h"
#include "CIrrDeviceStub.h"

namespace irr
{

	class CIrrDeviceServer : public CIrrDeviceStub
	{
	public:
		//! constructor
		CIrrDeviceServer(const SIrrlichtCreationParameters& param);

		//! runs the device. Returns false if device wants to be deleted
		virtual bool run() { return !Close; }

		//! pause execution temporarily
		virtual void yield() {}

		//! pause execution for a specified time
		virtual void sleep(u32 timeMs, bool pauseTimer) {}

		//! sets the caption of the window
		virtual void setWindowCaption(const wchar_t* text) {}

		//! sets the class of the window
		virtual void setWindowClass(const char* text) {}

		//! returns if window is active. if not, nothing need to be drawn
		virtual bool isWindowActive() const { return true; }

		//! returns if window has focus.
		bool isWindowFocused() const { return true; }

		//! returns if window is minimized.
		bool isWindowMinimized() const { return false; }

		//! notifies the device that it should close itself
		virtual void closeDevice() { Close = true; }

		//! Sets if the window should be resizable in windowed mode.
		virtual void setResizable(bool resize=false) {}

		//! Minimizes the window.
		virtual void minimizeWindow() {}

		//! Maximizes the window.
		virtual void maximizeWindow() {}

		//! Restores the window size.
		virtual void restoreWindow() {}

		//! Move window to requested position
		virtual bool moveWindow(int x, int y) { return true; }

		//! Get current window position.
		virtual bool getWindowPosition(int* x, int* y) { return true; }

		//! Get the device type
		virtual E_DEVICE_TYPE getType() const { return EIDT_SERVER; }
	};

} // end namespace irr

#endif // _IRR_COMPILE_WITH_SDL_DEVICE_
#endif // __C_IRR_DEVICE_SDL_H_INCLUDED__

