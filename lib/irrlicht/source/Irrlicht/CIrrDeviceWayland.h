#ifndef CIRRDEVICEWAYLAND_H
#define CIRRDEVICEWAYLAND_H

#include "IrrCompileConfig.h"

#ifdef _IRR_COMPILE_WITH_WAYLAND

#include "CIrrDeviceStub.h"
#include "IrrlichtDevice.h"
#include "IImagePresenter.h"
#include "ICursorControl.h"
#include "os.h"

#include <wayland-client.h>
#include <wayland-egl.h>
#include <wayland-cursor.h>
#include <xkbcommon/xkbcommon.h>
#include <xkbcommon/xkbcommon-compose.h>

#include <GL/gl.h>

#include <vector>
#define KeySym s32

class ContextManagerEGL;

// Note : only supporting shell interface

namespace irr
{

	class CIrrDeviceWayland : public CIrrDeviceStub, public video::IImagePresenter
	{
	public:

		//! constructor
		CIrrDeviceWayland(const SIrrlichtCreationParameters& param);

		//! destructor
		virtual ~CIrrDeviceWayland();

		//! runs the device. Returns false if device wants to be deleted
		virtual bool run();

		//! Cause the device to temporarily pause execution and let other processes to run
		// This should bring down processor usage without major performance loss for Irrlicht
		virtual void yield();

		//! Pause execution and let other processes to run for a specified amount of time.
		virtual void sleep(u32 timeMs, bool pauseTimer);

		//! sets the caption of the window
		virtual void setWindowCaption(const wchar_t* text);

		//! returns if window is active. if not, nothing need to be drawn
		virtual bool isWindowActive() const;

		//! returns if window has focus.
		virtual bool isWindowFocused() const;

		//! returns if window is minimized.
		virtual bool isWindowMinimized() const;

		//! returns color format of the window.
		virtual video::ECOLOR_FORMAT getColorFormat() const;

		//! presents a surface in the client area
		virtual bool present(video::IImage* surface, void* windowId=0, core::rect<s32>* src=0 );

		//! notifies the device that it should close itself
		virtual void closeDevice();

		//! \return Returns a pointer to a list with all video modes
		//! supported by the gfx adapter.
		video::IVideoModeList* getVideoModeList();

		//! Sets if the window should be resizable in windowed mode.
		virtual void setResizable(bool resize=false);

		//! Minimizes the window.
		virtual void minimizeWindow();

		//! Maximizes the window.
		virtual void maximizeWindow();

		//! Restores the window size.
		virtual void restoreWindow();

		//! Activate any joysticks, and generate events for them.
		virtual bool activateJoysticks(core::array<SJoystickInfo> & joystickInfo);

		//! Set the current Gamma Value for the Display
		virtual bool setGammaRamp( f32 red, f32 green, f32 blue, f32 brightness, f32 contrast );

		//! Get the current Gamma Value for the Display
		virtual bool getGammaRamp( f32 &red, f32 &green, f32 &blue, f32 &brightness, f32 &contrast );

		//! gets text from the clipboard
		//! \return Returns 0 if no string is in there.
		virtual const c8* getTextFromClipboard() const;

		//! copies text to the clipboard
		//! This sets the clipboard selection and _not_ the primary selection which you have on X on the middle mouse button.
		virtual void copyToClipboard(const c8* text) const;

		//! Remove all messages pending in the system message loop
		virtual void clearSystemMessages();

		//! Get the device type
		virtual E_DEVICE_TYPE getType() const
		{
				return EIDT_WAYLAND;
		}

	private:

		//! create the driver
		void createDriver();

		void initEGL();

		bool createWindow();

		void createKeyMap();

		void pollJoysticks();

		//! Implementation of the linux cursor control
		class CCursorControl : public gui::ICursorControl
		{
		public:

			CCursorControl(CIrrDeviceWayland* dev, bool null);

			~CCursorControl();

			//! Changes the visible state of the mouse cursor.
			virtual void setVisible(bool visible)
			{
				if (visible==IsVisible)
					return;
					
				IsVisible = visible;
				Device->updateCursor();
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
				//TODO
				CursorPos.X = x;
				CursorPos.Y = y;
			}

			//! Returns the current position of the mouse cursor.
			virtual const core::position2d<s32>& getPosition()
			{
				return CursorPos;
			}
			
			virtual core::position2d<f32> getRelativePosition()
			{
				if (!UseReferenceRect)
				{
					return core::position2d<f32>(CursorPos.X / (f32)Device->Width,
						CursorPos.Y / (f32)Device->Height);
				}

				return core::position2d<f32>(CursorPos.X / (f32)ReferenceRect.getWidth(),
						CursorPos.Y / (f32)ReferenceRect.getHeight());
			}

			virtual void setReferenceRect(core::rect<s32>* rect=0)
			{
				if (rect)
				{
					ReferenceRect = *rect;
					UseReferenceRect = true;

					// prevent division through zero and uneven sizes

					if (!ReferenceRect.getHeight() || ReferenceRect.getHeight()%2)
						ReferenceRect.LowerRightCorner.Y += 1;

					if (!ReferenceRect.getWidth() || ReferenceRect.getWidth()%2)
						ReferenceRect.LowerRightCorner.X += 1;
				}
				else
					UseReferenceRect = false;
			}

			//! Sets the active cursor icon
			virtual void setActiveIcon(gui::ECURSOR_ICON iconId) {};

			//! Gets the currently active icon
			virtual gui::ECURSOR_ICON getActiveIcon() const
			{
				return ActiveIcon;
			}

			//! Add a custom sprite as cursor icon.
			virtual gui::ECURSOR_ICON addIcon(const gui::SCursorSprite& icon) 
			{
				return gui::ECI_NORMAL;
			}

			//! replace the given cursor icon.
			virtual void changeIcon(gui::ECURSOR_ICON iconId, const gui::SCursorSprite& icon) {}

			//! Return a system-specific size which is supported for cursors. Larger icons will fail, smaller icons might work.
			virtual core::dimension2di getSupportedIconSize() const
			{
				return core::dimension2di(0, 0);
			}
		private:

			CIrrDeviceWayland* Device;
			core::position2d<s32> CursorPos;
			core::rect<s32> ReferenceRect;
			bool IsVisible;
			bool Null;
			bool UseReferenceRect;
			gui::ECURSOR_ICON ActiveIcon;
			u32 ActiveIconStartTime;
		};

		friend class CCursorControl;

		friend class COpenGLDriver;

		friend class WaylandCallbacks;
		std::vector<SEvent> events;
		std::vector<core::dimension2du> Modes;
		core::dimension2du CurrentModes;

		ContextManagerEGL* EglContext;
		
	public:
		static bool isWaylandDeviceWorking();
		void signalEvent(const SEvent&);
		void addMode(const core::dimension2du &mode) { Modes.push_back(mode); }
		void setCurrentMode(const core::dimension2du &mode) { CurrentModes = mode; }

		wl_display *display;
		wl_registry *registry;
		wl_compositor *compositor;
		wl_seat *seat;
		wl_pointer *pointer;
		wl_keyboard *keyboard;
		wl_output *output;
		
		wl_shm* shm;
		wl_cursor_theme* cursor_theme;
		wl_cursor* default_cursor;
		wl_surface* cursor_surface;
		
		xkb_context *xkbctx;
		xkb_keymap *keymap;
		xkb_state *state;
		xkb_mod_mask_t control_mask;
		xkb_mod_mask_t alt_mask;
		xkb_mod_mask_t shift_mask;
		uint32_t modifiers;
		struct xkb_compose_table *compose_table;
		struct xkb_compose_state *compose_state;

		wl_shell *shell;
		wl_surface *surface;
		wl_shell_surface *shell_surface;
		wl_egl_window *egl_window;
		
		u32 ButtonStates;
		uint32_t enter_serial;
		
		ContextManagerEGL* getEGLContext() {return EglContext;}
		void swapBuffers();
		void updateCursor();
		
	private:
//		XVisualInfo* visual;
		mutable core::stringc Clipboard;

		u32 Width, Height;
		bool WindowHasFocus;
		bool WindowMinimized;
		bool UseXVidMode;
		bool UseXRandR;
		bool UseGLXWindow;
		bool ExternalWindow;
		int AutorepeatSupport;
public:
		struct SKeyMap
		{
			SKeyMap() {}
			SKeyMap(s32 x11, s32 win32)
				: X11Key(x11), Win32Key(win32)
			{
			}

			KeySym X11Key;
			s32 Win32Key;

			bool operator<(const SKeyMap& o) const
			{
				return X11Key<o.X11Key;
			}
		};

		core::array<SKeyMap> KeyMap;
private:
#if defined(_IRR_COMPILE_WITH_JOYSTICK_EVENTS_)
		struct JoystickInfo
		{
			int	fd;
			int	axes;
			int	buttons;

			SEvent persistentData;

			JoystickInfo() : fd(-1), axes(0), buttons(0) { }
		};
		core::array<JoystickInfo> ActiveJoysticks;
#endif
	};


} // end namespace irr

#endif

#endif // CIRRDEVICEWAYLAND_H

