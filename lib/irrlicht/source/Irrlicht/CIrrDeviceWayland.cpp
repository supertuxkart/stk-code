#include "CIrrDeviceWayland.h"

#ifdef _IRR_COMPILE_WITH_WAYLAND

#include <stdio.h>
#include <stdlib.h>
#include <sys/utsname.h>
#include <time.h>
#include <string>
#include "IEventReceiver.h"
#include "ISceneManager.h"
#include "IGUIEnvironment.h"
#include "os.h"
#include "CTimer.h"
#include "irrString.h"
#include "Keycodes.h"
#include "COSOperator.h"
#include "CColorConverter.h"
#include "SIrrCreationParameters.h"
#include "IGUISpriteBank.h"
#include <sys/mman.h>
#include "CVideoModeList.h"
#include "CContextEGL.h"

#if defined _IRR_COMPILE_WITH_JOYSTICK_EVENTS_
#include <fcntl.h>
#include <unistd.h>


// linux/joystick.h includes linux/input.h, which #defines values for various KEY_FOO keys.
// These override the irr::KEY_FOO equivalents, which stops key handling from working.
// As a workaround, defining _INPUT_H stops linux/input.h from being included; it
// doesn't actually seem to be necessary except to pull in sys/ioctl.h.
#define _INPUT_H
#include <sys/ioctl.h> // Would normally be included in linux/input.h
#include <linux/joystick.h>
#undef _INPUT_H

#endif // _IRR_COMPILE_WITH_JOYSTICK_EVENTS_

#define MOD_SHIFT_MASK		0x01
#define MOD_ALT_MASK		0x02
#define MOD_CONTROL_MASK	0x04

namespace irr
{
	namespace video
	{
		extern bool useCoreContext;
		IVideoDriver* createOpenGLDriver(const SIrrlichtCreationParameters& params,
				io::IFileSystem* io, CIrrDeviceWayland* device);
	}
} // end namespace irr

namespace irr
{

class WaylandCallbacks
{
public:
	// from http://cgit.freedesktop.org/wayland/weston/tree/clients/simple-egl.c
	static void
	pointer_handle_enter(void *data, struct wl_pointer *pointer,
					 uint32_t serial, struct wl_surface *surface,
					 wl_fixed_t sx, wl_fixed_t sy)
	{
		printf("enter!\n");
		
		CIrrDeviceWayland *device = static_cast<CIrrDeviceWayland *>(data);


		if (device->default_cursor) 
		{
			wl_cursor_image* image = device->default_cursor->images[0];
			wl_buffer* buffer = wl_cursor_image_get_buffer(image);
			
			if (!buffer)
				return;
				
			wl_pointer_set_cursor(pointer, serial, device->cursor_surface,
							      image->hotspot_x, image->hotspot_y);
			wl_surface_attach(device->cursor_surface, buffer, 0, 0);
			wl_surface_damage(device->cursor_surface, 0, 0,
							  image->width, image->height);
			wl_surface_commit(device->cursor_surface);
		}
		
	}

	static void
	pointer_handle_leave(void *data, struct wl_pointer *pointer,
					 uint32_t serial, struct wl_surface *surface)
	{
	}

	static void
	pointer_handle_motion(void *data, struct wl_pointer *pointer,
						uint32_t time, wl_fixed_t sx, wl_fixed_t sy)
	{
		CIrrDeviceWayland *device = static_cast<CIrrDeviceWayland *>(data);
		device->getCursorControl()->setPosition(sx, sy);
		SEvent irrevent;
		irrevent.EventType = irr::EET_MOUSE_INPUT_EVENT;
		irrevent.MouseInput.Event = irr::EMIE_MOUSE_MOVED;
		irrevent.MouseInput.X = device->getCursorControl()->getPosition().X;
		irrevent.MouseInput.Y = device->getCursorControl()->getPosition().Y;
		irrevent.MouseInput.Control = (device->modifiers & MOD_CONTROL_MASK) != 0;
		irrevent.MouseInput.Shift = (device->modifiers & MOD_SHIFT_MASK) != 0;
		irrevent.MouseInput.ButtonStates = device->ButtonStates;

		device->signalEvent(irrevent);
	}

	static void
	pointer_handle_button(void *data, struct wl_pointer *wl_pointer,
						uint32_t serial, uint32_t time, uint32_t button,
						uint32_t state)
	{
		CIrrDeviceWayland *device = static_cast<CIrrDeviceWayland *>(data);

		SEvent irrevent;
		irrevent.EventType = irr::EET_MOUSE_INPUT_EVENT;
		irrevent.MouseInput.X = device->getCursorControl()->getPosition().X;
		irrevent.MouseInput.Y = device->getCursorControl()->getPosition().Y;
		irrevent.MouseInput.Control = (device->modifiers & MOD_CONTROL_MASK) != 0;
		irrevent.MouseInput.Shift = (device->modifiers & MOD_SHIFT_MASK) != 0;
		irrevent.MouseInput.Event = irr::EMIE_COUNT;

		switch (button)
		{
		case 272:
			if (state == WL_POINTER_BUTTON_STATE_PRESSED)
			{
				irrevent.MouseInput.Event = irr::EMIE_LMOUSE_PRESSED_DOWN;
				device->ButtonStates |= irr::EMBSM_LEFT;
			}
			else if (state == WL_POINTER_BUTTON_STATE_RELEASED)
			{
				irrevent.MouseInput.Event = irr::EMIE_LMOUSE_LEFT_UP;
				device->ButtonStates &= ~(irr::EMBSM_LEFT);
			}
			break;
		case 273:
			if (state == WL_POINTER_BUTTON_STATE_PRESSED)
			{
				irrevent.MouseInput.Event = irr::EMIE_RMOUSE_PRESSED_DOWN;
				device->ButtonStates |= irr::EMBSM_RIGHT;
			}
			else if (state == WL_POINTER_BUTTON_STATE_RELEASED)
			{
				irrevent.MouseInput.Event = irr::EMIE_RMOUSE_LEFT_UP;
				device->ButtonStates &= ~(irr::EMBSM_RIGHT);
			}
			break;
		case 274:
			if (state == WL_POINTER_BUTTON_STATE_PRESSED)
			{
				irrevent.MouseInput.Event = irr::EMIE_MMOUSE_PRESSED_DOWN;
				device->ButtonStates |= irr::EMBSM_MIDDLE;
			}
			else if (state == WL_POINTER_BUTTON_STATE_RELEASED)
			{
				irrevent.MouseInput.Event = irr::EMIE_MMOUSE_LEFT_UP;
				device->ButtonStates &= ~(irr::EMBSM_MIDDLE);
			}
		default:
			break;
		}
			
		irrevent.MouseInput.ButtonStates = device->ButtonStates;
		
		device->signalEvent(irrevent);
	}

	static void
	pointer_handle_axis(void *data, struct wl_pointer *wl_pointer,
					uint32_t time, uint32_t axis, wl_fixed_t value)
	{
	}

	static const struct wl_pointer_listener pointer_listener;

	static void
	keyboard_repeat_func(struct task *task, uint32_t events)
	{
	}

	static void
	keyboard_handle_keymap(void *data, struct wl_keyboard *keyboard,
						 uint32_t format, int fd, uint32_t size)
	{
		CIrrDeviceWayland *device = static_cast<CIrrDeviceWayland *>(data);
		
		//~ struct xkb_keymap *keymap;
		//~ struct xkb_state *state;
		//~ struct xkb_compose_table *compose_table;
		//~ struct xkb_compose_state *compose_state;
		char *map_str;
	
		if (!device) {
			close(fd);
			return;
		}
	
		if (format != WL_KEYBOARD_KEYMAP_FORMAT_XKB_V1) {
			close(fd);
			return;
		}
	
		map_str = static_cast<char *>(mmap(NULL, size, PROT_READ, MAP_SHARED, fd, 0));
		if (map_str == MAP_FAILED) {
			close(fd);
			return;
		}
	
		/* Set up XKB keymap */
		device->keymap = xkb_keymap_new_from_string(device->xkbctx,
						    map_str,
						    XKB_KEYMAP_FORMAT_TEXT_V1,
						    XKB_KEYMAP_COMPILE_NO_FLAGS);
		munmap(map_str, size);
		close(fd);
	
		if (!device->keymap) {
			fprintf(stderr, "failed to compile keymap\n");
			return;
		}
	
		/* Set up XKB state */
		device->state = xkb_state_new(device->keymap);
		if (!device->state) {
			fprintf(stderr, "failed to create XKB state\n");
			xkb_keymap_unref(device->keymap);
			return;
		}
	
		/* Look up the preferred locale, falling back to "C" as default */
		std::string locale = "C";
		
		if (getenv("LC_ALL"))
			locale = getenv("LC_ALL");
		else if (getenv("LC_CTYPE"))
			locale = getenv("LC_CTYPE");
		else if (getenv("LANG"))
			locale = getenv("LANG");
	
		/* Set up XKB compose table */
		device->compose_table =
			xkb_compose_table_new_from_locale(device->xkbctx,
							  locale.c_str(),
							  XKB_COMPOSE_COMPILE_NO_FLAGS);
		if (device->compose_table) {
			/* Set up XKB compose state */
			device->compose_state = xkb_compose_state_new(device->compose_table,
						      XKB_COMPOSE_STATE_NO_FLAGS);
			if (device->compose_state) {
				//~ xkb_compose_state_unref(device->compose_state);
				//~ xkb_compose_table_unref(device->compose_table);
				//~ device->compose_state = compose_state;
				//~ device->compose_table = compose_table;
			} else {
				fprintf(stderr, "could not create XKB compose state.  "
					"Disabiling compose.\n");
				xkb_compose_table_unref(device->compose_table);
				device->compose_table = NULL;
			}
		} else {
			fprintf(stderr, "could not create XKB compose table for locale '%s'.  "
				"Disabiling compose\n", locale.c_str());
	}
	
		//~ xkb_keymap_unref(device->keymap);
		//~ xkb_state_unref(device->state);
		//~ device->keymap = keymap;
		//~ device->state = state;
	
		device->control_mask =
			1 << xkb_keymap_mod_get_index(device->keymap, "Control");
		device->alt_mask =
			1 << xkb_keymap_mod_get_index(device->keymap, "Mod1");
		device->shift_mask =
			1 << xkb_keymap_mod_get_index(device->keymap, "Shift");
	}

	static void
	keyboard_handle_enter(void *data, struct wl_keyboard *keyboard,
						uint32_t serial, struct wl_surface *surface,
						struct wl_array *keys)
	{
	}

	static void
	keyboard_handle_leave(void *data, struct wl_keyboard *keyboard,
						uint32_t serial, struct wl_surface *surface)
	{
	}

	static void
	keyboard_handle_key(void *data, struct wl_keyboard *keyboard,
					uint32_t serial, uint32_t time, uint32_t key,
					uint32_t state_w)
	{
		CIrrDeviceWayland *device = static_cast<CIrrDeviceWayland *>(data);
		
		uint32_t code;
		enum wl_keyboard_key_state state = (wl_keyboard_key_state) state_w;
		xkb_keysym_t sym;
		uint32_t num_syms;
		const xkb_keysym_t *syms;
		SEvent irrevent;
		
		if (!device->state)
			return;
	
		code = key + 8;
		
		num_syms = xkb_state_key_get_syms(device->state, code, &syms);
	
		sym = XKB_KEY_NoSymbol;
		if (num_syms == 1)
			sym = syms[0];
			
		CIrrDeviceWayland::SKeyMap mp;
		mp.X11Key = sym;
			

	if (!device->compose_state)
		sym = sym;
	if (sym == XKB_KEY_NoSymbol)
		sym = sym;
	if (xkb_compose_state_feed(device->compose_state,
				   sym) != XKB_COMPOSE_FEED_ACCEPTED)
		sym = sym;

	switch (xkb_compose_state_get_status(device->compose_state)) {
	case XKB_COMPOSE_COMPOSING:
		return;
	case XKB_COMPOSE_COMPOSED:
		sym = xkb_compose_state_get_one_sym(device->compose_state);
	case XKB_COMPOSE_CANCELLED:
		return;
	case XKB_COMPOSE_NOTHING:
		sym = sym;
	default:
		sym = sym;
	}

	
	//~ printf("%c\n", sym);
	
		if (sym == XKB_KEY_NoSymbol)
			sym = 0;
	

	
		irrevent.EventType = irr::EET_KEY_INPUT_EVENT;
		irrevent.KeyInput.PressedDown = (state == WL_KEYBOARD_KEY_STATE_PRESSED);
		irrevent.KeyInput.Char = 	xkb_keysym_to_utf32(sym);
		irrevent.KeyInput.Control = (device->modifiers & MOD_CONTROL_MASK) != 0;
		irrevent.KeyInput.Shift = (device->modifiers & MOD_SHIFT_MASK) != 0;
	
	
	

	
	
		const s32 idx = device->KeyMap.binary_search(mp);
		if (idx != -1)
		{
			irrevent.KeyInput.Key = (EKEY_CODE)device->KeyMap[idx].Win32Key;
		}
		else
		{
			irrevent.KeyInput.Key = (EKEY_CODE)0;
		}
		if (irrevent.KeyInput.Key == 0)
		{
			// 1:1 mapping to windows-keys would require testing for keyboard type (us, ger, ...)
			// So unless we do that we will have some unknown keys here.
			if (idx == -1)
			{
	//			os::Printer::log("Could not find EKEY_CODE, using orig. X11 keycode instead", core::stringc(event.xkey.keycode).c_str(), ELL_INFORMATION);
			}
			else
			{
	//			os::Printer::log("EKEY_CODE is 0, using orig. X11 keycode instead", core::stringc(event.xkey.keycode).c_str(), ELL_INFORMATION);
			}
			// Any value is better than none, that allows at least using the keys.
			// Worst case is that some keys will be identical, still better than _all_
			// unknown keys being identical.
	//		irrevent.KeyInput.Key = (EKEY_CODE)event.xkey.keycode;
		}
	
		device->signalEvent(irrevent);
	}

	static void
	keyboard_handle_modifiers(void *data, struct wl_keyboard *keyboard,
					uint32_t serial, uint32_t mods_depressed,
					uint32_t mods_latched, uint32_t mods_locked,
					uint32_t group)
	{
		CIrrDeviceWayland *device = static_cast<CIrrDeviceWayland *>(data);

		xkb_mod_mask_t mask;
	
		/* If we're not using a keymap, then we don't handle PC-style modifiers */
		if (!device->keymap)
			return;
	
		xkb_state_update_mask(device->state, mods_depressed, mods_latched,
				      mods_locked, 0, 0, group);
		xkb_state_component state_component = (xkb_state_component)(
							XKB_STATE_MODS_DEPRESSED | XKB_STATE_MODS_LATCHED);
		mask = xkb_state_serialize_mods(device->state,
						state_component);
						
		device->modifiers = 0;
		if (mask & device->control_mask)
			device->modifiers |= MOD_CONTROL_MASK;
		if (mask & device->alt_mask)
			device->modifiers |= MOD_ALT_MASK;
		if (mask & device->shift_mask)
			device->modifiers |= MOD_SHIFT_MASK;
	}

	static void
	set_repeat_info(struct input *input, int32_t rate, int32_t delay)
	{
		
	}

	static void
	keyboard_handle_repeat_info(void *data, struct wl_keyboard *keyboard,
						int32_t rate, int32_t delay)
	{
	}

	static const struct wl_keyboard_listener keyboard_listener;

	static void
	seat_handle_capabilities(void *data, struct wl_seat *seat,
				uint32_t caps)
	{
		CIrrDeviceWayland *dev = static_cast<CIrrDeviceWayland *>(data);

		if ((caps & WL_SEAT_CAPABILITY_POINTER) && !dev->pointer) {
			dev->pointer = wl_seat_get_pointer(seat);
			wl_pointer_add_listener(dev->pointer, &pointer_listener, data);
		} else if (!(caps & WL_SEAT_CAPABILITY_POINTER) && dev->pointer) {
			wl_pointer_destroy(dev->pointer);
			dev->pointer = NULL;
		}

		if ((caps & WL_SEAT_CAPABILITY_KEYBOARD) && !dev->keyboard) {
			dev->keyboard = wl_seat_get_keyboard(seat);
			wl_keyboard_add_listener(dev->keyboard, &keyboard_listener, dev);
		} else if (!(caps & WL_SEAT_CAPABILITY_KEYBOARD) && dev->keyboard) {
			wl_keyboard_destroy(dev->keyboard);
			dev->keyboard = NULL;
		}
	}

	static const struct wl_seat_listener seat_listener;

	static void
	display_handle_geometry(void *data,
				struct wl_output *wl_output,
				int x, int y,
				int physical_width,
				int physical_height,
				int subpixel,
				const char *make,
				const char *model,
				int transform)
	{
		printf("output model is %s\n", model);
	}

	static void
	display_handle_done(void *data,
					 struct wl_output *wl_output)
	{
		printf("done\n");
	}

	static void
	display_handle_scale(void *data,
					 struct wl_output *wl_output,
					 int32_t scale)
	{
	}

	static void
	display_handle_mode(void *data,
					struct wl_output *wl_output,
					uint32_t flags,
					int width,
					int height,
					int refresh)
	{
		CIrrDeviceWayland *dev = static_cast<CIrrDeviceWayland *>(data);
		dev->VideoModeList.addMode(core::dimension2du(width, height), 24);
		if (flags & WL_OUTPUT_MODE_CURRENT) {
			dev->VideoModeList.setDesktop(24, core::dimension2du(width, height));
		}
	}
	
	static void
	handle_ping(void *data, struct wl_shell_surface *shell_surface,
	            uint32_t serial)
	{
		printf("ping");
	    wl_shell_surface_pong(shell_surface, serial);
	}
	
	static void
	handle_configure(void *data, struct wl_shell_surface *shell_surface,
	                 uint32_t edges, int32_t width, int32_t height)
	{
	}
	
	static void
	handle_popup_done(void *data, struct wl_shell_surface *shell_surface)
	{
	}

	static const struct wl_output_listener output_listener;
	static const struct wl_shell_surface_listener shell_surface_listener;

	static void registry_add (void *data, struct wl_registry *registry, uint32_t name, const char *interface, uint32_t version) {
		CIrrDeviceWayland *dev = static_cast<CIrrDeviceWayland *>(data);
		if (!strcmp(interface,"wl_compositor")) {
			printf("binding compositor\n");
			dev->compositor = static_cast<wl_compositor *>(wl_registry_bind (registry, name, &wl_compositor_interface, 1));
		}
		else if (!strcmp(interface,"wl_shell")) {
			printf("binding shell\n");
			dev->shell = static_cast<wl_shell *>(wl_registry_bind (registry, name, &wl_shell_interface, 1));
		}
		else if (strcmp(interface, "wl_seat") == 0) {
			printf("binding seat\n");
			dev->seat = static_cast<wl_seat *>(wl_registry_bind(registry, name, &wl_seat_interface, 1));
		}
		else if (strcmp(interface, "wl_shm") == 0) {
			dev->shm = static_cast<wl_shm*>(wl_registry_bind(registry, name, &wl_shm_interface, 1));
			dev->cursor_theme = wl_cursor_theme_load(NULL, 32, dev->shm);
			if (!dev->cursor_theme) {
				printf("unable to load default theme\n");
				return;
			}
			dev->default_cursor =
				wl_cursor_theme_get_cursor(dev->cursor_theme, "left_ptr");
			if (!dev->default_cursor) {
				printf("unable to load default left pointer\n");
			}
		}
		else if (strcmp(interface, "wl_output") == 0)
		{
			printf("binding output\n");
			dev->output = static_cast<wl_output *>(wl_registry_bind(dev->registry, name, &wl_output_interface, 2));
		}
	}

	static void registry_remove (void *data, struct wl_registry *registry, uint32_t name) {
		printf ("registry_remove_object\n");
	}

	static const wl_registry_listener registry_listener;
};

const struct wl_pointer_listener WaylandCallbacks::pointer_listener = {
	WaylandCallbacks::pointer_handle_enter,
	WaylandCallbacks::pointer_handle_leave,
	WaylandCallbacks::pointer_handle_motion,
	WaylandCallbacks::pointer_handle_button,
	WaylandCallbacks::pointer_handle_axis,
};

const struct wl_keyboard_listener WaylandCallbacks::keyboard_listener = {
	WaylandCallbacks::keyboard_handle_keymap,
	WaylandCallbacks::keyboard_handle_enter,
	WaylandCallbacks::keyboard_handle_leave,
	WaylandCallbacks::keyboard_handle_key,
	WaylandCallbacks::keyboard_handle_modifiers,
	WaylandCallbacks::keyboard_handle_repeat_info
};

const struct wl_seat_listener WaylandCallbacks::seat_listener = {
	WaylandCallbacks::seat_handle_capabilities,
};

const struct wl_output_listener WaylandCallbacks::output_listener = {
	WaylandCallbacks::display_handle_geometry,
	WaylandCallbacks::display_handle_mode,
	WaylandCallbacks::display_handle_done,
	WaylandCallbacks::display_handle_scale
};

const struct wl_shell_surface_listener WaylandCallbacks::shell_surface_listener = 
{
    WaylandCallbacks::handle_ping,
    WaylandCallbacks::handle_configure,
    WaylandCallbacks::handle_popup_done
};

const wl_registry_listener WaylandCallbacks::registry_listener = {
	WaylandCallbacks::registry_add,
	WaylandCallbacks::registry_remove,
};



//const char* wmDeleteWindow = "WM_DELETE_WINDOW";

bool CIrrDeviceWayland::isWaylandDeviceWorking()
{
	bool is_working = false;
	
	wl_display* display = wl_display_connect(NULL);

	if (display != NULL) 
	{
		is_working = true;
		wl_display_disconnect(display);
	}

    return is_working;
}

//! constructor
CIrrDeviceWayland::CIrrDeviceWayland(const SIrrlichtCreationParameters& param)
	: CIrrDeviceStub(param),
	Width(param.WindowSize.Width), Height(param.WindowSize.Height),
	WindowHasFocus(false), WindowMinimized(false),
	UseXVidMode(false), UseXRandR(false), UseGLXWindow(false),
	ExternalWindow(false), AutorepeatSupport(0)
{
	#ifdef _DEBUG
	setDebugName("CIrrDeviceLinux");
	#endif
	
	EglContext = NULL;

	// print version, distribution etc.
	// thx to LynxLuna for pointing me to the uname function
	core::stringc linuxversion;
	struct utsname LinuxInfo;
	uname(&LinuxInfo);

	linuxversion += LinuxInfo.sysname;
	linuxversion += " ";
	linuxversion += LinuxInfo.release;
	linuxversion += " ";
	linuxversion += LinuxInfo.version;
	linuxversion += " ";
	linuxversion += LinuxInfo.machine;

	Operator = new COSOperator(linuxversion, this);
	os::Printer::log(linuxversion.c_str(), ELL_INFORMATION);

	// Retrieve wayland infos
	display = wl_display_connect(NULL);
	registry = wl_display_get_registry(display);
	wl_registry_add_listener(registry, &WaylandCallbacks::registry_listener, this);
	wl_display_dispatch(display);

	xkbctx = xkb_context_new(XKB_CONTEXT_NO_FLAGS);

	pointer = 0;
	keyboard = 0;
	ButtonStates = 0;

	wl_seat_add_listener(seat, &WaylandCallbacks::seat_listener, this);
	wl_output_add_listener(output, &WaylandCallbacks::output_listener, this);
	
	shell_surface = NULL;
	cursor_surface = NULL;
	cursor_theme = NULL;

	// create keymap
	createKeyMap();

	// create window
	if (CreationParams.DriverType != video::EDT_NULL)
	{
		// create the window, only if we do not use the null device
		if (!createWindow())
			return;
			
		cursor_surface = wl_compositor_create_surface(compositor);
	}

	// create cursor control
	CursorControl = new CCursorControl(this, CreationParams.DriverType == video::EDT_NULL);

	// create driver
	createDriver();

	if (VideoDriver)
		createGUIAndScene();

	// Sync everything wayland before leaving
	wl_display_dispatch(display);
}


//! destructor
CIrrDeviceWayland::~CIrrDeviceWayland()
{
	printf("destroy dev\n");
	
	delete EglContext;
	
	wl_output_destroy(output);
	wl_keyboard_destroy(keyboard);
	wl_pointer_destroy(pointer);
	wl_seat_destroy(seat);
	
	if (cursor_surface)
		wl_surface_destroy(cursor_surface);
		
	if (cursor_theme)
		wl_cursor_theme_destroy(cursor_theme);
	
	if (shell_surface)
		wl_shell_surface_destroy(shell_surface);
		
	wl_registry_destroy(registry);
	wl_display_flush(display);
	wl_display_disconnect(display);
	xkb_context_unref(xkbctx);
	

#if defined(_IRR_COMPILE_WITH_JOYSTICK_EVENTS_)
	for (u32 joystick = 0; joystick < ActiveJoysticks.size(); ++joystick)
	{
		if (ActiveJoysticks[joystick].fd >= 0)
		{
			close(ActiveJoysticks[joystick].fd);
		}
	}
#endif
}

bool CIrrDeviceWayland::restoreResolution()
{
	if (!CreationParams.Fullscreen)
		return true;
	return true;
}


bool CIrrDeviceWayland::changeResolution()
{
	if (!CreationParams.Fullscreen)
		return true;

	getVideoModeList();

	return CreationParams.Fullscreen;
}


void CIrrDeviceWayland::initEGL()
{
	egl_window = wl_egl_window_create(surface, Width, Height);

	EglContext = new ContextManagerEGL();
		
	ContextEGLParams egl_params;
	egl_params.opengl_api = CEGL_API_OPENGL;
	egl_params.surface_type = CEGL_SURFACE_WINDOW;
	egl_params.force_legacy_device = CreationParams.ForceLegacyDevice;
	egl_params.with_alpha_channel = CreationParams.WithAlphaChannel;
	egl_params.vsync_enabled = CreationParams.Vsync;
	egl_params.window = egl_window;
	egl_params.display = display;
	
	EglContext->init(egl_params);
	video::useCoreContext = !EglContext->isLegacyDevice();
}

bool CIrrDeviceWayland::createWindow()
{
	surface = wl_compositor_create_surface(compositor);
	shell_surface = wl_shell_get_shell_surface(shell, surface);
	
	wl_shell_surface_add_listener(shell_surface,
								  &WaylandCallbacks::shell_surface_listener, this);
	
	if (CreationParams.Fullscreen)
	{
		wl_shell_surface_set_fullscreen(shell_surface,
										WL_SHELL_SURFACE_FULLSCREEN_METHOD_DEFAULT,
										0, output);
	}
	else
	{   
		wl_shell_surface_set_toplevel(shell_surface);
	}

	wl_display_flush(display);

	initEGL();

	CreationParams.WindowSize.Width = Width;
	CreationParams.WindowSize.Height = Height;

	return true;
}


//! create the driver
void CIrrDeviceWayland::createDriver()
{
	switch(CreationParams.DriverType)
	{
	default:
		os::Printer::log("Wayland driver only supports OpenGL.", ELL_ERROR);
		break;
	case video::EDT_OPENGL:
//		#ifdef _IRR_COMPILE_WITH_OPENGL_
//		if (Context)
			VideoDriver = video::createOpenGLDriver(CreationParams, FileSystem, this);
/*		#else
		os::Printer::log("No OpenGL support compiled in.", ELL_ERROR);
		#endif*/
		break;
	}
}


//! runs the device. Returns false if device wants to be deleted
bool CIrrDeviceWayland::run()
{
	os::Timer::tick();

//	printf("vents size is %d\n", events.size());
	for (unsigned i = 0; i < events.size(); i++)
	{
		postEventFromUser(events[i]);

/*		if (irrevent.MouseInput.Event != irr::EMIE_COUNT)
		{
						printf("posteventfromuser\n");
			bool v = postEventFromUser(irrevent);
			printf("v is %d\n", v);

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
		}*/
	}
	events.clear();


	if (!Close)
		pollJoysticks();

	return !Close;
}


//! Pause the current process for the minimum time allowed only to allow other processes to execute
void CIrrDeviceWayland::yield()
{
	struct timespec ts = {0,1};
	nanosleep(&ts, NULL);
}


//! Pause execution and let other processes to run for a specified amount of time.
void CIrrDeviceWayland::sleep(u32 timeMs, bool pauseTimer=false)
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

CIrrDeviceWayland::CCursorControl::CCursorControl(CIrrDeviceWayland* dev, bool null)
	: Device(dev)
	, IsVisible(true), Null(null), UseReferenceRect(false)
	, ActiveIcon(gui::ECI_NORMAL), ActiveIconStartTime(0)
{
}

CIrrDeviceWayland::CCursorControl::~CCursorControl()
{
	// Do not clearCursors here as the display is already closed
	// TODO (cutealien): droping cursorcontrol earlier might work, not sure about reason why that's done in stub currently.
}

//! Sets the new position of the cursor.
void CIrrDeviceWayland::CCursorControl::setPosition(s32 x, s32 y)
{
	CursorPos = core::position2di(x / 256, y / 256);
}

//! Returns the current position of the mouse cursor.
const core::position2d<s32>& CIrrDeviceWayland::CCursorControl::getPosition()
{
	return CursorPos;
}

core::position2d<f32> CIrrDeviceWayland::CCursorControl::getRelativePosition()
{}

//! Sets the active cursor icon
void CIrrDeviceWayland::CCursorControl::setActiveIcon(gui::ECURSOR_ICON iconId)
{

}

void CIrrDeviceWayland::signalEvent(const SEvent &event)
{
	events.push_back(event);
}

//! Add a custom sprite as cursor icon.
gui::ECURSOR_ICON CIrrDeviceWayland::CCursorControl::addIcon(const gui::SCursorSprite& icon)
{
	return gui::ECI_NORMAL;
}

//! replace the given cursor icon.
void CIrrDeviceWayland::CCursorControl::changeIcon(gui::ECURSOR_ICON iconId, const gui::SCursorSprite& icon)
{
}

irr::core::dimension2di CIrrDeviceWayland::CCursorControl::getSupportedIconSize() const
{
	// this returns the closest match that is smaller or same size, so we just pass a value which should be large enough for cursors
	unsigned int width=0, height=0;
	return core::dimension2di(width, height);
}

//! sets the caption of the window
void CIrrDeviceWayland::setWindowCaption(const wchar_t* text)
{
}


//! presents a surface in the client area
bool CIrrDeviceWayland::present(video::IImage* image, void* windowId, core::rect<s32>* srcRect)
{
	return true;
}


//! notifies the device that it should close itself
void CIrrDeviceWayland::closeDevice()
{
	Close = true;
}


//! returns if window is active. if not, nothing need to be drawn
bool CIrrDeviceWayland::isWindowActive() const
{
	return (WindowHasFocus && !WindowMinimized);
}


//! returns if window has focus.
bool CIrrDeviceWayland::isWindowFocused() const
{
	return WindowHasFocus;
}


//! returns if window is minimized.
bool CIrrDeviceWayland::isWindowMinimized() const
{
	return WindowMinimized;
}


//! returns color format of the window.
video::ECOLOR_FORMAT CIrrDeviceWayland::getColorFormat() const
{
		return video::ECF_R8G8B8;
}


//! Sets if the window should be resizable in windowed mode.
void CIrrDeviceWayland::setResizable(bool resize)
{
}


//! Return pointer to a list with all video modes supported by the gfx adapter.
video::IVideoModeList* CIrrDeviceWayland::getVideoModeList()
{
	printf("retrieve\n");
	return &VideoModeList;
}


//! Minimize window
void CIrrDeviceWayland::minimizeWindow()
{

}


//! Maximize window
void CIrrDeviceWayland::maximizeWindow()
{

}


//! Restore original window size
void CIrrDeviceWayland::restoreWindow()
{

}


void CIrrDeviceWayland::createKeyMap()
{
	KeyMap.reallocate(190);
	KeyMap.push_back(SKeyMap(XKB_KEY_BackSpace, KEY_BACK));
	KeyMap.push_back(SKeyMap(XKB_KEY_Tab, KEY_TAB));
	KeyMap.push_back(SKeyMap(XKB_KEY_ISO_Left_Tab, KEY_TAB));
//	KeyMap.push_back(SKeyMap(XK_Linefeed, 0)); // ???
	KeyMap.push_back(SKeyMap(XKB_KEY_Clear, KEY_CLEAR));
	KeyMap.push_back(SKeyMap(XKB_KEY_Return, KEY_RETURN));
	KeyMap.push_back(SKeyMap(XKB_KEY_Pause, KEY_PAUSE));
	KeyMap.push_back(SKeyMap(XKB_KEY_Scroll_Lock, KEY_SCROLL));
//	KeyMap.push_back(SKeyMap(XK_Sys_Req, 0)); // ???
	KeyMap.push_back(SKeyMap(XKB_KEY_Escape, KEY_ESCAPE));
	KeyMap.push_back(SKeyMap(XKB_KEY_Insert, KEY_INSERT));
	KeyMap.push_back(SKeyMap(XKB_KEY_Delete, KEY_DELETE));
	KeyMap.push_back(SKeyMap(XKB_KEY_Home, KEY_HOME));
	KeyMap.push_back(SKeyMap(XKB_KEY_Left, KEY_LEFT));
	KeyMap.push_back(SKeyMap(XKB_KEY_Up, KEY_UP));
	KeyMap.push_back(SKeyMap(XKB_KEY_Right, KEY_RIGHT));
	KeyMap.push_back(SKeyMap(XKB_KEY_Down, KEY_DOWN));
	KeyMap.push_back(SKeyMap(XKB_KEY_Prior, KEY_PRIOR));
	KeyMap.push_back(SKeyMap(XKB_KEY_Page_Up, KEY_PRIOR));
	KeyMap.push_back(SKeyMap(XKB_KEY_Next, KEY_NEXT));
	KeyMap.push_back(SKeyMap(XKB_KEY_Page_Down, KEY_NEXT));
	KeyMap.push_back(SKeyMap(XKB_KEY_End, KEY_END));
	KeyMap.push_back(SKeyMap(XKB_KEY_Begin, KEY_HOME));
	KeyMap.push_back(SKeyMap(XKB_KEY_Num_Lock, KEY_NUMLOCK));
	KeyMap.push_back(SKeyMap(XKB_KEY_space, KEY_SPACE));
	KeyMap.push_back(SKeyMap(XKB_KEY_KP_Tab, KEY_TAB));
	KeyMap.push_back(SKeyMap(XKB_KEY_KP_Enter, KEY_RETURN));
	KeyMap.push_back(SKeyMap(XKB_KEY_KP_F1, KEY_F1));
	KeyMap.push_back(SKeyMap(XKB_KEY_KP_F2, KEY_F2));
	KeyMap.push_back(SKeyMap(XKB_KEY_KP_F3, KEY_F3));
	KeyMap.push_back(SKeyMap(XKB_KEY_KP_F4, KEY_F4));
	KeyMap.push_back(SKeyMap(XKB_KEY_KP_Home, KEY_HOME));
	KeyMap.push_back(SKeyMap(XKB_KEY_KP_Left, KEY_LEFT));
	KeyMap.push_back(SKeyMap(XKB_KEY_KP_Up, KEY_UP));
	KeyMap.push_back(SKeyMap(XKB_KEY_KP_Right, KEY_RIGHT));
	KeyMap.push_back(SKeyMap(XKB_KEY_KP_Down, KEY_DOWN));
	KeyMap.push_back(SKeyMap(XKB_KEY_Print, KEY_PRINT));
	KeyMap.push_back(SKeyMap(XKB_KEY_KP_Prior, KEY_PRIOR));
	KeyMap.push_back(SKeyMap(XKB_KEY_KP_Page_Up, KEY_PRIOR));
	KeyMap.push_back(SKeyMap(XKB_KEY_KP_Next, KEY_NEXT));
	KeyMap.push_back(SKeyMap(XKB_KEY_KP_Page_Down, KEY_NEXT));
	KeyMap.push_back(SKeyMap(XKB_KEY_KP_End, KEY_END));
	KeyMap.push_back(SKeyMap(XKB_KEY_KP_Begin, KEY_HOME));
	KeyMap.push_back(SKeyMap(XKB_KEY_KP_Insert, KEY_INSERT));
	KeyMap.push_back(SKeyMap(XKB_KEY_KP_Delete, KEY_DELETE));
//	KeyMap.push_back(SKeyMap(XK_KP_Equal, 0)); // ???
	KeyMap.push_back(SKeyMap(XKB_KEY_KP_Multiply, KEY_MULTIPLY));
	KeyMap.push_back(SKeyMap(XKB_KEY_KP_Add, KEY_ADD));
	KeyMap.push_back(SKeyMap(XKB_KEY_KP_Separator, KEY_SEPARATOR));
	KeyMap.push_back(SKeyMap(XKB_KEY_KP_Subtract, KEY_SUBTRACT));
	KeyMap.push_back(SKeyMap(XKB_KEY_KP_Decimal, KEY_DECIMAL));
	KeyMap.push_back(SKeyMap(XKB_KEY_KP_Divide, KEY_DIVIDE));
	KeyMap.push_back(SKeyMap(XKB_KEY_KP_0, KEY_NUMPAD0));
	KeyMap.push_back(SKeyMap(XKB_KEY_KP_1, KEY_NUMPAD1));
	KeyMap.push_back(SKeyMap(XKB_KEY_KP_2, KEY_NUMPAD2));
	KeyMap.push_back(SKeyMap(XKB_KEY_KP_3, KEY_NUMPAD3));
	KeyMap.push_back(SKeyMap(XKB_KEY_KP_4, KEY_NUMPAD4));
	KeyMap.push_back(SKeyMap(XKB_KEY_KP_5, KEY_NUMPAD5));
	KeyMap.push_back(SKeyMap(XKB_KEY_KP_6, KEY_NUMPAD6));
	KeyMap.push_back(SKeyMap(XKB_KEY_KP_7, KEY_NUMPAD7));
	KeyMap.push_back(SKeyMap(XKB_KEY_KP_8, KEY_NUMPAD8));
	KeyMap.push_back(SKeyMap(XKB_KEY_KP_9, KEY_NUMPAD9));
	KeyMap.push_back(SKeyMap(XKB_KEY_F1, KEY_F1));
	KeyMap.push_back(SKeyMap(XKB_KEY_F2, KEY_F2));
	KeyMap.push_back(SKeyMap(XKB_KEY_F3, KEY_F3));
	KeyMap.push_back(SKeyMap(XKB_KEY_F4, KEY_F4));
	KeyMap.push_back(SKeyMap(XKB_KEY_F5, KEY_F5));
	KeyMap.push_back(SKeyMap(XKB_KEY_F6, KEY_F6));
	KeyMap.push_back(SKeyMap(XKB_KEY_F7, KEY_F7));
	KeyMap.push_back(SKeyMap(XKB_KEY_F8, KEY_F8));
	KeyMap.push_back(SKeyMap(XKB_KEY_F9, KEY_F9));
	KeyMap.push_back(SKeyMap(XKB_KEY_F10, KEY_F10));
	KeyMap.push_back(SKeyMap(XKB_KEY_F11, KEY_F11));
	KeyMap.push_back(SKeyMap(XKB_KEY_F12, KEY_F12));
	KeyMap.push_back(SKeyMap(XKB_KEY_Shift_L, KEY_LSHIFT));
	KeyMap.push_back(SKeyMap(XKB_KEY_Shift_R, KEY_RSHIFT));
	KeyMap.push_back(SKeyMap(XKB_KEY_Control_L, KEY_LCONTROL));
	KeyMap.push_back(SKeyMap(XKB_KEY_Control_R, KEY_RCONTROL));
	KeyMap.push_back(SKeyMap(XKB_KEY_Caps_Lock, KEY_CAPITAL));
	KeyMap.push_back(SKeyMap(XKB_KEY_Shift_Lock, KEY_CAPITAL));
	KeyMap.push_back(SKeyMap(XKB_KEY_Meta_L, KEY_LWIN));
	KeyMap.push_back(SKeyMap(XKB_KEY_Meta_R, KEY_RWIN));
	KeyMap.push_back(SKeyMap(XKB_KEY_Alt_L, KEY_LMENU));
	KeyMap.push_back(SKeyMap(XKB_KEY_Alt_R, KEY_RMENU));
	KeyMap.push_back(SKeyMap(XKB_KEY_ISO_Level3_Shift, KEY_RMENU));
	KeyMap.push_back(SKeyMap(XKB_KEY_Menu, KEY_MENU));
	KeyMap.push_back(SKeyMap(XKB_KEY_space, KEY_SPACE));
//	KeyMap.push_back(SKeyMap(XKB_key_ex, 0)); //?
//	KeyMap.push_back(SKeyMap(XK_quotedbl, 0)); //?
//	KeyMap.push_back(SKeyMap(XK_section, 0)); //?
	KeyMap.push_back(SKeyMap(XKB_KEY_numbersign, KEY_OEM_2));
//	KeyMap.push_back(SKeyMap(XK_dollar, 0)); //?
//	KeyMap.push_back(SKeyMap(XK_percent, 0)); //?
//	KeyMap.push_back(SKeyMap(XK_ampersand, 0)); //?
	KeyMap.push_back(SKeyMap(XKB_KEY_apostrophe, KEY_OEM_7));
//	KeyMap.push_back(SKeyMap(XK_parenleft, 0)); //?
//	KeyMap.push_back(SKeyMap(XK_parenright, 0)); //?
//	KeyMap.push_back(SKeyMap(XK_asterisk, 0)); //?
	KeyMap.push_back(SKeyMap(XKB_KEY_plus, KEY_PLUS)); //?
	KeyMap.push_back(SKeyMap(XKB_KEY_comma, KEY_COMMA)); //?
	KeyMap.push_back(SKeyMap(XKB_KEY_minus, KEY_MINUS)); //?
	KeyMap.push_back(SKeyMap(XKB_KEY_period, KEY_PERIOD)); //?
	KeyMap.push_back(SKeyMap(XKB_KEY_slash, KEY_OEM_2)); //?
	KeyMap.push_back(SKeyMap(XKB_KEY_0, KEY_KEY_0));
	KeyMap.push_back(SKeyMap(XKB_KEY_1, KEY_KEY_1));
	KeyMap.push_back(SKeyMap(XKB_KEY_2, KEY_KEY_2));
	KeyMap.push_back(SKeyMap(XKB_KEY_3, KEY_KEY_3));
	KeyMap.push_back(SKeyMap(XKB_KEY_4, KEY_KEY_4));
	KeyMap.push_back(SKeyMap(XKB_KEY_5, KEY_KEY_5));
	KeyMap.push_back(SKeyMap(XKB_KEY_6, KEY_KEY_6));
	KeyMap.push_back(SKeyMap(XKB_KEY_7, KEY_KEY_7));
	KeyMap.push_back(SKeyMap(XKB_KEY_8, KEY_KEY_8));
	KeyMap.push_back(SKeyMap(XKB_KEY_9, KEY_KEY_9));
//	KeyMap.push_back(SKeyMap(XK_colon, 0)); //?
	KeyMap.push_back(SKeyMap(XKB_KEY_semicolon, KEY_OEM_1));
	KeyMap.push_back(SKeyMap(XKB_KEY_less, KEY_OEM_102));
	KeyMap.push_back(SKeyMap(XKB_KEY_equal, KEY_PLUS));
//	KeyMap.push_back(SKeyMap(XK_greater, 0)); //?
//	KeyMap.push_back(SKeyMap(XK_question, 0)); //?
	KeyMap.push_back(SKeyMap(XKB_KEY_at, KEY_KEY_2)); //?
//	KeyMap.push_back(SKeyMap(XK_mu, 0)); //?
//	KeyMap.push_back(SKeyMap(XK_EuroSign, 0)); //?
	KeyMap.push_back(SKeyMap(XKB_KEY_A, KEY_KEY_A));
	KeyMap.push_back(SKeyMap(XKB_KEY_B, KEY_KEY_B));
	KeyMap.push_back(SKeyMap(XKB_KEY_C, KEY_KEY_C));
	KeyMap.push_back(SKeyMap(XKB_KEY_D, KEY_KEY_D));
	KeyMap.push_back(SKeyMap(XKB_KEY_E, KEY_KEY_E));
	KeyMap.push_back(SKeyMap(XKB_KEY_F, KEY_KEY_F));
	KeyMap.push_back(SKeyMap(XKB_KEY_G, KEY_KEY_G));
	KeyMap.push_back(SKeyMap(XKB_KEY_H, KEY_KEY_H));
	KeyMap.push_back(SKeyMap(XKB_KEY_I, KEY_KEY_I));
	KeyMap.push_back(SKeyMap(XKB_KEY_J, KEY_KEY_J));
	KeyMap.push_back(SKeyMap(XKB_KEY_K, KEY_KEY_K));
	KeyMap.push_back(SKeyMap(XKB_KEY_L, KEY_KEY_L));
	KeyMap.push_back(SKeyMap(XKB_KEY_M, KEY_KEY_M));
	KeyMap.push_back(SKeyMap(XKB_KEY_N, KEY_KEY_N));
	KeyMap.push_back(SKeyMap(XKB_KEY_O, KEY_KEY_O));
	KeyMap.push_back(SKeyMap(XKB_KEY_P, KEY_KEY_P));
	KeyMap.push_back(SKeyMap(XKB_KEY_Q, KEY_KEY_Q));
	KeyMap.push_back(SKeyMap(XKB_KEY_R, KEY_KEY_R));
	KeyMap.push_back(SKeyMap(XKB_KEY_S, KEY_KEY_S));
	KeyMap.push_back(SKeyMap(XKB_KEY_T, KEY_KEY_T));
	KeyMap.push_back(SKeyMap(XKB_KEY_U, KEY_KEY_U));
	KeyMap.push_back(SKeyMap(XKB_KEY_V, KEY_KEY_V));
	KeyMap.push_back(SKeyMap(XKB_KEY_W, KEY_KEY_W));
	KeyMap.push_back(SKeyMap(XKB_KEY_X, KEY_KEY_X));
	KeyMap.push_back(SKeyMap(XKB_KEY_Y, KEY_KEY_Y));
	KeyMap.push_back(SKeyMap(XKB_KEY_Z, KEY_KEY_Z));
	KeyMap.push_back(SKeyMap(XKB_KEY_bracketleft, KEY_OEM_4));
	KeyMap.push_back(SKeyMap(XKB_KEY_backslash, KEY_OEM_5));
	KeyMap.push_back(SKeyMap(XKB_KEY_bracketright, KEY_OEM_6));
	KeyMap.push_back(SKeyMap(XKB_KEY_asciicircum, KEY_OEM_5));
//	KeyMap.push_back(SKeyMap(XK_degree, 0)); //?
	KeyMap.push_back(SKeyMap(XKB_KEY_underscore, KEY_MINUS)); //?
	KeyMap.push_back(SKeyMap(XKB_KEY_grave, KEY_OEM_3));
	KeyMap.push_back(SKeyMap(XKB_KEY_acute, KEY_OEM_6));
	KeyMap.push_back(SKeyMap(XKB_KEY_a, KEY_KEY_A));
	KeyMap.push_back(SKeyMap(XKB_KEY_b, KEY_KEY_B));
	KeyMap.push_back(SKeyMap(XKB_KEY_c, KEY_KEY_C));
	KeyMap.push_back(SKeyMap(XKB_KEY_c, KEY_KEY_D));
	KeyMap.push_back(SKeyMap(XKB_KEY_e, KEY_KEY_E));
	KeyMap.push_back(SKeyMap(XKB_KEY_f, KEY_KEY_F));
	KeyMap.push_back(SKeyMap(XKB_KEY_g, KEY_KEY_G));
	KeyMap.push_back(SKeyMap(XKB_KEY_h, KEY_KEY_H));
	KeyMap.push_back(SKeyMap(XKB_KEY_i, KEY_KEY_I));
	KeyMap.push_back(SKeyMap(XKB_KEY_j, KEY_KEY_J));
	KeyMap.push_back(SKeyMap(XKB_KEY_k, KEY_KEY_K));
	KeyMap.push_back(SKeyMap(XKB_KEY_l, KEY_KEY_L));
	KeyMap.push_back(SKeyMap(XKB_KEY_m, KEY_KEY_M));
	KeyMap.push_back(SKeyMap(XKB_KEY_n, KEY_KEY_N));
	KeyMap.push_back(SKeyMap(XKB_KEY_o, KEY_KEY_O));
	KeyMap.push_back(SKeyMap(XKB_KEY_p, KEY_KEY_P));
	KeyMap.push_back(SKeyMap(XKB_KEY_q, KEY_KEY_Q));
	KeyMap.push_back(SKeyMap(XKB_KEY_r, KEY_KEY_R));
	KeyMap.push_back(SKeyMap(XKB_KEY_s, KEY_KEY_S));
	KeyMap.push_back(SKeyMap(XKB_KEY_t, KEY_KEY_T));
	KeyMap.push_back(SKeyMap(XKB_KEY_u, KEY_KEY_U));
	KeyMap.push_back(SKeyMap(XKB_KEY_v, KEY_KEY_V));
	KeyMap.push_back(SKeyMap(XKB_KEY_w, KEY_KEY_W));
	KeyMap.push_back(SKeyMap(XKB_KEY_x, KEY_KEY_X));
	KeyMap.push_back(SKeyMap(XKB_KEY_y, KEY_KEY_Y));
	KeyMap.push_back(SKeyMap(XKB_KEY_z, KEY_KEY_Z));
	KeyMap.push_back(SKeyMap(XKB_KEY_ssharp, KEY_OEM_4));
	KeyMap.push_back(SKeyMap(XKB_KEY_adiaeresis, KEY_OEM_7));
	KeyMap.push_back(SKeyMap(XKB_KEY_odiaeresis, KEY_OEM_3));
	KeyMap.push_back(SKeyMap(XKB_KEY_udiaeresis, KEY_OEM_1));
	KeyMap.push_back(SKeyMap(XKB_KEY_Super_L, KEY_LWIN));
	KeyMap.push_back(SKeyMap(XKB_KEY_Super_R, KEY_RWIN));

	KeyMap.sort();


}

bool CIrrDeviceWayland::activateJoysticks(core::array<SJoystickInfo> & joystickInfo)
{
#if defined (_IRR_COMPILE_WITH_JOYSTICK_EVENTS_)

	joystickInfo.clear();

	u32 joystick;
	for (joystick = 0; joystick < 32; ++joystick)
	{
		// The joystick device could be here...
		core::stringc devName = "/dev/js";
		devName += joystick;

		SJoystickInfo returnInfo;
		JoystickInfo info;

		info.fd = open(devName.c_str(), O_RDONLY);
		if (-1 == info.fd)
		{
			// ...but Ubuntu and possibly other distros
			// create the devices in /dev/input
			devName = "/dev/input/js";
			devName += joystick;
			info.fd = open(devName.c_str(), O_RDONLY);
			if (-1 == info.fd)
			{
				// and BSD here
				devName = "/dev/joy";
				devName += joystick;
				info.fd = open(devName.c_str(), O_RDONLY);
			}
		}

		if (-1 == info.fd)
			continue;

#ifdef __FreeBSD__
		info.axes=2;
		info.buttons=2;
#else
		ioctl( info.fd, JSIOCGAXES, &(info.axes) );
		ioctl( info.fd, JSIOCGBUTTONS, &(info.buttons) );
		fcntl( info.fd, F_SETFL, O_NONBLOCK );
#endif

		(void)memset(&info.persistentData, 0, sizeof(info.persistentData));
		info.persistentData.EventType = irr::EET_JOYSTICK_INPUT_EVENT;
		info.persistentData.JoystickEvent.Joystick = ActiveJoysticks.size();

		// There's no obvious way to determine which (if any) axes represent a POV
		// hat, so we'll just set it to "not used" and forget about it.
		info.persistentData.JoystickEvent.POV = 65535;

		ActiveJoysticks.push_back(info);

		returnInfo.HasGenericName = false;
		returnInfo.Joystick = joystick;
		returnInfo.PovHat = SJoystickInfo::POV_HAT_UNKNOWN;
		returnInfo.Axes = info.axes;
		returnInfo.Buttons = info.buttons;

#ifndef __FreeBSD__
		char name[80];
		ioctl( info.fd, JSIOCGNAME(80), name);
		returnInfo.Name = name;
#endif

		joystickInfo.push_back(returnInfo);
	}

	for (joystick = 0; joystick < joystickInfo.size(); ++joystick)
	{
		char logString[256];
		(void)sprintf(logString, "Found joystick %u, %u axes, %u buttons '%s'",
			joystick, joystickInfo[joystick].Axes,
			joystickInfo[joystick].Buttons, joystickInfo[joystick].Name.c_str());
		os::Printer::log(logString, ELL_INFORMATION);
	}

	return true;
#else
	return false;
#endif // _IRR_COMPILE_WITH_JOYSTICK_EVENTS_
}


void CIrrDeviceWayland::pollJoysticks()
{
#if defined (_IRR_COMPILE_WITH_JOYSTICK_EVENTS_)
	if (0 == ActiveJoysticks.size())
		return;

	for (u32 j= 0; j< ActiveJoysticks.size(); ++j)
	{
		JoystickInfo & info =  ActiveJoysticks[j];

#ifdef __FreeBSD__
		struct joystick js;
		if (read(info.fd, &js, sizeof(js)) == sizeof(js))
		{
			info.persistentData.JoystickEvent.ButtonStates = js.b1 | (js.b2 << 1); /* should be a two-bit field */
			info.persistentData.JoystickEvent.Axis[0] = js.x; /* X axis */
			info.persistentData.JoystickEvent.Axis[1] = js.y; /* Y axis */
		}
#else
		struct js_event event;
		while (sizeof(event) == read(info.fd, &event, sizeof(event)))
		{
			switch(event.type & ~JS_EVENT_INIT)
			{
			case JS_EVENT_BUTTON:
				if (event.value)
						info.persistentData.JoystickEvent.ButtonStates |= (1 << event.number);
				else
						info.persistentData.JoystickEvent.ButtonStates &= ~(1 << event.number);
				break;

			case JS_EVENT_AXIS:
				if (event.number < SEvent::SJoystickEvent::NUMBER_OF_AXES)
					info.persistentData.JoystickEvent.Axis[event.number] = event.value;
				break;

			default:
				break;
			}
		}
#endif

		// Send an irrlicht joystick event once per ::run() even if no new data were received.
		(void)postEventFromUser(info.persistentData);
	}
#endif // _IRR_COMPILE_WITH_JOYSTICK_EVENTS_
}


//! Set the current Gamma Value for the Display
bool CIrrDeviceWayland::setGammaRamp( f32 red, f32 green, f32 blue, f32 brightness, f32 contrast )
{
	return false;
}


//! Get the current Gamma Value for the Display
bool CIrrDeviceWayland::getGammaRamp( f32 &red, f32 &green, f32 &blue, f32 &brightness, f32 &contrast )
{
	brightness = 0.f;
	contrast = 0.f;
	return false;
}


//! gets text from the clipboard
//! \return Returns 0 if no string is in there.
const c8* CIrrDeviceWayland::getTextFromClipboard() const
{
	printf("get text from clipboard: %s\n", Clipboard.c_str());
	return Clipboard.c_str();

}

//! copies text to the clipboard
void CIrrDeviceWayland::copyToClipboard(const c8* text) const
{
	printf("copy to clipboard: %s\n", text);
	Clipboard = text;
}


//! Remove all messages pending in the system message loop
void CIrrDeviceWayland::clearSystemMessages()
{
}

void CIrrDeviceWayland::initXAtoms()
{
}

} // end namespace

#endif
