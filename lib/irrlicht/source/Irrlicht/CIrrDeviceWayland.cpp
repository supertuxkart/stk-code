//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2016-2017 Dawid Gan
//
//  This program is free software; you can redistribute it and/or
//  modify it under the terms of the GNU General Public License
//  as published by the Free Software Foundation; either version 3
//  of the License, or (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

#include "CIrrDeviceWayland.h"

#ifdef _IRR_COMPILE_WITH_WAYLAND_DEVICE_

#include <cstdio>
#include <cstdlib>
#include <string>
#include <sys/mman.h>
#include <sys/utsname.h>
#include <time.h>

#if defined _IRR_COMPILE_WITH_JOYSTICK_EVENTS_
#include <fcntl.h>
#include <unistd.h>

// linux/joystick.h includes linux/input.h, which #defines values for various
// KEY_FOO keys. These override the irr::KEY_FOO equivalents, which stops key
// handling from working. As a workaround, defining _INPUT_H stops linux/input.h
// from being included; it doesn't actually seem to be necessary except to pull
// in sys/ioctl.h.
#define _INPUT_H
#include <sys/ioctl.h> // Would normally be included in linux/input.h
#include <linux/joystick.h>
#undef _INPUT_H

#endif // _IRR_COMPILE_WITH_JOYSTICK_EVENTS_

#include "CColorConverter.h"
#include "CContextEGL.h"
#include "COSOperator.h"
#include "CTimer.h"
#include "CVideoModeList.h"
#include "IEventReceiver.h"
#include "IGUIEnvironment.h"
#include "IGUISpriteBank.h"
#include "irrString.h"
#include "ISceneManager.h"
#include "Keycodes.h"
#include "os.h"
#include "SIrrCreationParameters.h"

namespace irr
{
    namespace video
    {
        extern bool useCoreContext;
        IVideoDriver* createOpenGLDriver(const SIrrlichtCreationParameters& params,
                io::IFileSystem* io, CIrrDeviceWayland* device);
    }
}

namespace irr
{

class WaylandCallbacks
{
public:
    static const wl_pointer_listener pointer_listener;
    static const wl_seat_listener seat_listener;
    static const wl_keyboard_listener keyboard_listener;
    static const wl_output_listener output_listener;
    static const wl_shell_surface_listener shell_surface_listener;
    static const wl_registry_listener registry_listener;

    static void pointer_enter(void* data, wl_pointer* pointer, uint32_t serial,
                              wl_surface* surface, wl_fixed_t sx, wl_fixed_t sy)
    {
        CIrrDeviceWayland* device = static_cast<CIrrDeviceWayland*>(data);

        device->m_enter_serial = serial;
        device->updateCursor();
    }

    static void pointer_leave(void* data, wl_pointer* pointer, uint32_t serial,
                              wl_surface* surface)
    {
    }

    static void pointer_motion(void* data, wl_pointer* pointer, uint32_t time,
                               wl_fixed_t sx, wl_fixed_t sy)
    {
        CIrrDeviceWayland* device = static_cast<CIrrDeviceWayland*>(data);

        device->getCursorControl()->setPosition(wl_fixed_to_int(sx),
                                                wl_fixed_to_int(sy));

        SEvent irrevent;
        irrevent.EventType = irr::EET_MOUSE_INPUT_EVENT;
        irrevent.MouseInput.Event = irr::EMIE_MOUSE_MOVED;
        irrevent.MouseInput.X = device->getCursorControl()->getPosition().X;
        irrevent.MouseInput.Y = device->getCursorControl()->getPosition().Y;
        irrevent.MouseInput.Control = device->m_xkb_ctrl_pressed;
        irrevent.MouseInput.Shift = device->m_xkb_shift_pressed;
        irrevent.MouseInput.ButtonStates = device->m_mouse_button_states;

        device->signalEvent(irrevent);
    }

    static void pointer_button(void* data, wl_pointer* wl_pointer,
                               uint32_t serial, uint32_t time, uint32_t button,
                               uint32_t state)
    {
        CIrrDeviceWayland* device = static_cast<CIrrDeviceWayland* >(data);

        SEvent irrevent;
        irrevent.EventType = irr::EET_MOUSE_INPUT_EVENT;
        irrevent.MouseInput.X = device->getCursorControl()->getPosition().X;
        irrevent.MouseInput.Y = device->getCursorControl()->getPosition().Y;
        irrevent.MouseInput.Control = device->m_xkb_ctrl_pressed;
        irrevent.MouseInput.Shift = device->m_xkb_shift_pressed;
        irrevent.MouseInput.Event = irr::EMIE_COUNT;

        switch (button)
        {
        case 272:
            if (state == WL_POINTER_BUTTON_STATE_PRESSED)
            {
                irrevent.MouseInput.Event = irr::EMIE_LMOUSE_PRESSED_DOWN;
                device->m_mouse_button_states |= irr::EMBSM_LEFT;
            }
            else if (state == WL_POINTER_BUTTON_STATE_RELEASED)
            {
                irrevent.MouseInput.Event = irr::EMIE_LMOUSE_LEFT_UP;
                device->m_mouse_button_states &= ~(irr::EMBSM_LEFT);
            }
            break;
        case 273:
            if (state == WL_POINTER_BUTTON_STATE_PRESSED)
            {
                irrevent.MouseInput.Event = irr::EMIE_RMOUSE_PRESSED_DOWN;
                device->m_mouse_button_states |= irr::EMBSM_RIGHT;
            }
            else if (state == WL_POINTER_BUTTON_STATE_RELEASED)
            {
                irrevent.MouseInput.Event = irr::EMIE_RMOUSE_LEFT_UP;
                device->m_mouse_button_states &= ~(irr::EMBSM_RIGHT);
            }
            break;
        case 274:
            if (state == WL_POINTER_BUTTON_STATE_PRESSED)
            {
                irrevent.MouseInput.Event = irr::EMIE_MMOUSE_PRESSED_DOWN;
                device->m_mouse_button_states |= irr::EMBSM_MIDDLE;
            }
            else if (state == WL_POINTER_BUTTON_STATE_RELEASED)
            {
                irrevent.MouseInput.Event = irr::EMIE_MMOUSE_LEFT_UP;
                device->m_mouse_button_states &= ~(irr::EMBSM_MIDDLE);
            }
        default:
            break;
        }

        if (irrevent.MouseInput.Event == irr::EMIE_COUNT)
            return;

        irrevent.MouseInput.ButtonStates = device->m_mouse_button_states;

        device->signalEvent(irrevent);

        // It's not exactly true for middle/right button, but keep
        // consistency with x11 device
        if (irrevent.MouseInput.Event >= EMIE_LMOUSE_PRESSED_DOWN &&
            irrevent.MouseInput.Event <= EMIE_MMOUSE_PRESSED_DOWN)
        {
            u32 clicks = device->checkSuccessiveClicks(
                                                irrevent.MouseInput.X,
                                                irrevent.MouseInput.Y,
                                                irrevent.MouseInput.Event);
            if (clicks == 2)
            {
                irrevent.MouseInput.Event =
                        (EMOUSE_INPUT_EVENT)(EMIE_LMOUSE_DOUBLE_CLICK +
                        irrevent.MouseInput.Event-EMIE_LMOUSE_PRESSED_DOWN);

                device->signalEvent(irrevent);
            }
            else if ( clicks == 3 )
            {
                irrevent.MouseInput.Event =
                        (EMOUSE_INPUT_EVENT)(EMIE_LMOUSE_TRIPLE_CLICK +
                        irrevent.MouseInput.Event-EMIE_LMOUSE_PRESSED_DOWN);

                device->signalEvent(irrevent);
            }
        }
    }

    static void pointer_axis(void* data, wl_pointer* wl_pointer, uint32_t time,
                             uint32_t axis, wl_fixed_t value)
    {
        CIrrDeviceWayland* device = static_cast<CIrrDeviceWayland* >(data);

        if (axis == WL_POINTER_AXIS_VERTICAL_SCROLL)
        {
            SEvent irrevent;
            irrevent.EventType = irr::EET_MOUSE_INPUT_EVENT;
            irrevent.MouseInput.X = device->getCursorControl()->getPosition().X;
            irrevent.MouseInput.Y = device->getCursorControl()->getPosition().Y;
            irrevent.MouseInput.Control = device->m_xkb_ctrl_pressed;
            irrevent.MouseInput.Shift = device->m_xkb_shift_pressed;
            irrevent.MouseInput.ButtonStates = device->m_mouse_button_states;
            irrevent.MouseInput.Event = EMIE_MOUSE_WHEEL;
            irrevent.MouseInput.Wheel = wl_fixed_to_double(value) / -10.0f;

            device->signalEvent(irrevent);
        }
    }

    static void keyboard_keymap(void* data, wl_keyboard* keyboard,
                                uint32_t format, int fd, uint32_t size)
    {
        CIrrDeviceWayland* device = static_cast<CIrrDeviceWayland*>(data);

        if (!device)
        {
            close(fd);
            return;
        }

        if (format != WL_KEYBOARD_KEYMAP_FORMAT_XKB_V1)
        {
            close(fd);
            return;
        }

        char* map_str = static_cast<char*>(mmap(NULL, size, PROT_READ,
                                                MAP_SHARED, fd, 0));

        if (map_str == MAP_FAILED)
        {
            close(fd);
            return;
        }

        device->m_xkb_keymap = xkb_keymap_new_from_string(
                                                   device->m_xkb_context,
                                                   map_str,
                                                   XKB_KEYMAP_FORMAT_TEXT_V1,
                                                   XKB_KEYMAP_COMPILE_NO_FLAGS);
        munmap(map_str, size);
        close(fd);

        if (!device->m_xkb_keymap)
        {
            return;
        }

        device->m_xkb_state = xkb_state_new(device->m_xkb_keymap);

        if (!device->m_xkb_state)
        {
            xkb_keymap_unref(device->m_xkb_keymap);
            device->m_xkb_keymap = NULL;
            return;
        }

        device->m_xkb_alt_mask =
            1 << xkb_keymap_mod_get_index(device->m_xkb_keymap, "Mod1");
        device->m_xkb_ctrl_mask =
            1 << xkb_keymap_mod_get_index(device->m_xkb_keymap, "Control");
        device->m_xkb_shift_mask =
            1 << xkb_keymap_mod_get_index(device->m_xkb_keymap, "Shift");

        std::string locale = "C";

        if (getenv("LC_ALL"))
        {
            locale = getenv("LC_ALL");
        }
        else if (getenv("LC_CTYPE"))
        {
            locale = getenv("LC_CTYPE");
        }
        else if (getenv("LANG"))
        {
            locale = getenv("LANG");
        }

        device->m_xkb_compose_table = xkb_compose_table_new_from_locale(
                                                  device->m_xkb_context,
                                                  locale.c_str(),
                                                  XKB_COMPOSE_COMPILE_NO_FLAGS);

        if (!device->m_xkb_compose_table)
            return;

        device->m_xkb_compose_state = xkb_compose_state_new(
                                                device->m_xkb_compose_table,
                                                XKB_COMPOSE_STATE_NO_FLAGS);

        if (!device->m_xkb_compose_state)
        {
            xkb_compose_table_unref(device->m_xkb_compose_table);
            device->m_xkb_compose_table = NULL;
        }
    }

    static void keyboard_enter(void* data, wl_keyboard* keyboard,
                               uint32_t serial, wl_surface* surface,
                               wl_array* keys)
    {
    }

    static void keyboard_leave(void* data, wl_keyboard* keyboard,
                               uint32_t serial, wl_surface* surface)
    {
    }

    static void keyboard_key(void* data, wl_keyboard* keyboard, uint32_t serial,
                             uint32_t time, uint32_t key, uint32_t state)
    {
        CIrrDeviceWayland* device = static_cast<CIrrDeviceWayland*>(data);

        if (!device->m_xkb_state)
            return;

        wchar_t key_char = 0;

        if (state == WL_KEYBOARD_KEY_STATE_PRESSED)
        {
            xkb_keysym_t sym = XKB_KEY_NoSymbol;

            const xkb_keysym_t* syms;
            uint32_t num_syms = xkb_state_key_get_syms(device->m_xkb_state,
                                                       key + 8, &syms);

            if (num_syms == 1)
                sym = syms[0];

            if (sym != XKB_KEY_NoSymbol && device->m_xkb_compose_state)
            {
                xkb_compose_feed_result result = xkb_compose_state_feed(
                                              device->m_xkb_compose_state, sym);

                if (result == XKB_COMPOSE_FEED_ACCEPTED)
                {
                    xkb_compose_status status = xkb_compose_state_get_status(
                                                   device->m_xkb_compose_state);
                    switch (status)
                    {
                    case XKB_COMPOSE_COMPOSING:
                    case XKB_COMPOSE_CANCELLED:
                        sym = XKB_KEY_NoSymbol;
                        break;
                    case XKB_COMPOSE_COMPOSED:
                        sym = xkb_compose_state_get_one_sym(
                                                   device->m_xkb_compose_state);
                        break;
                    default:
                        break;
                    }
                }
            }

            if (sym != XKB_KEY_NoSymbol)
            {
                key_char = xkb_keysym_to_utf32(sym);
            }
        }

        SEvent irrevent;
        irrevent.EventType = irr::EET_KEY_INPUT_EVENT;
        irrevent.KeyInput.Control = device->m_xkb_ctrl_pressed;
        irrevent.KeyInput.Shift = device->m_xkb_shift_pressed;
        irrevent.KeyInput.PressedDown = (state == WL_KEYBOARD_KEY_STATE_PRESSED);
        irrevent.KeyInput.Char = key_char;
        irrevent.KeyInput.Key = device->m_key_map[key];

        if (irrevent.KeyInput.Key == 0 && key > 0)
        {
            irrevent.KeyInput.Key = (EKEY_CODE)(KEY_KEY_CODES_COUNT + key);
        }

        device->signalEvent(irrevent);
    }

    static void keyboard_modifiers(void* data, wl_keyboard* keyboard,
                                   uint32_t serial, uint32_t mods_depressed,
                                   uint32_t mods_latched, uint32_t mods_locked,
                                   uint32_t group)
    {
        CIrrDeviceWayland* device = static_cast<CIrrDeviceWayland*>(data);

        if (!device->m_xkb_keymap)
            return;

        xkb_state_update_mask(device->m_xkb_state, mods_depressed, mods_latched,
                              mods_locked, 0, 0, group);
        xkb_state_component state_component = (xkb_state_component)(
                            XKB_STATE_MODS_DEPRESSED | XKB_STATE_MODS_LATCHED);

        xkb_mod_mask_t mods = xkb_state_serialize_mods(device->m_xkb_state,
                                                       state_component);

        device->m_xkb_alt_pressed = (mods & device->m_xkb_alt_mask) != 0;
        device->m_xkb_ctrl_pressed = (mods & device->m_xkb_ctrl_mask) != 0;
        device->m_xkb_shift_pressed = (mods & device->m_xkb_shift_mask) != 0;
    }

    static void keyboard_repeat_info(void* data, wl_keyboard* keyboard,
                                     int32_t rate, int32_t delay)
    {
    }

    static void seat_capabilities(void* data, wl_seat* seat, uint32_t caps)
    {
        CIrrDeviceWayland* device = static_cast<CIrrDeviceWayland*>(data);

        if ((caps & WL_SEAT_CAPABILITY_POINTER) && !device->m_pointer)
        {
            device->m_pointer = wl_seat_get_pointer(seat);
            wl_pointer_add_listener(device->m_pointer, &pointer_listener, device);
        }
        else if (!(caps & WL_SEAT_CAPABILITY_POINTER) && device->m_pointer)
        {
            wl_pointer_destroy(device->m_pointer);
            device->m_pointer = NULL;
        }

        if ((caps & WL_SEAT_CAPABILITY_KEYBOARD) && !device->m_keyboard)
        {
            device->m_keyboard = wl_seat_get_keyboard(seat);
            wl_keyboard_add_listener(device->m_keyboard, &keyboard_listener,
                                     device);
        }
        else if (!(caps & WL_SEAT_CAPABILITY_KEYBOARD) && device->m_keyboard)
        {
            wl_keyboard_destroy(device->m_keyboard);
            device->m_keyboard = NULL;
        }
    }

    static void output_geometry(void* data, wl_output* wl_output, int32_t x,
                                int32_t y, int32_t physical_width,
                                int32_t physical_height, int32_t subpixel,
                                const char* make, const char* model,
                                int32_t transform)

    {
    }

    static void output_done(void* data, wl_output* wl_output)
    {
    }

    static void output_scale(void* data, wl_output* wl_output, int32_t scale)
    {
    }

    static void output_mode(void* data, struct wl_output* wl_output,
                            uint32_t flags, int32_t width, int32_t height,
                            int32_t refresh)
    {
        CIrrDeviceWayland* device = static_cast<CIrrDeviceWayland*>(data);

        device->VideoModeList.addMode(core::dimension2du(width, height), 24);

        if (flags & WL_OUTPUT_MODE_CURRENT)
        {
            device->VideoModeList.setDesktop(24, core::dimension2du(width,
                                                                    height));
        }
    }

    static void shell_surface_ping(void* data, wl_shell_surface* shell_surface,
                                   uint32_t serial)
    {
        wl_shell_surface_pong(shell_surface, serial);
    }

    static void shell_surface_configure(void* data,
                                        wl_shell_surface* shell_surface,
                                        uint32_t edges, int32_t width,
                                        int32_t height)
    {
    }

    static void shell_surface_popup_done(void* data,
                                         wl_shell_surface* shell_surface)
    {
    }

    static void registry_global(void* data, wl_registry* registry,
                                uint32_t name, const char* interface,
                                uint32_t version)
    {
        CIrrDeviceWayland* device = static_cast<CIrrDeviceWayland*>(data);

        if (interface == NULL)
            return;

        std::string interface_str = interface;

        if (interface_str == "wl_compositor")
        {
            device->m_compositor = static_cast<wl_compositor*>(wl_registry_bind(
                                  registry, name, &wl_compositor_interface, 1));
        }
        else if (interface_str == "wl_shell")
        {
            device->m_shell = static_cast<wl_shell*>(wl_registry_bind(registry,
                                                 name, &wl_shell_interface, 1));
        }
        else if (interface_str == "wl_seat")
        {
            device->m_seat = static_cast<wl_seat*>(wl_registry_bind(registry,
                                                  name, &wl_seat_interface, 1));
        }
        else if (interface_str == "wl_shm")
        {
            device->m_shm = static_cast<wl_shm*>(wl_registry_bind(registry, name,
                                                         &wl_shm_interface, 1));
        }
        else if (interface_str == "wl_output")
        {
            device->m_output = static_cast<wl_output*>(wl_registry_bind(registry,
                                                name, &wl_output_interface, 2));
        }
    }

    static void registry_global_remove(void* data, wl_registry* registry,
                                       uint32_t name)
    {
    }
};

const wl_pointer_listener WaylandCallbacks::pointer_listener =
{
    WaylandCallbacks::pointer_enter,
    WaylandCallbacks::pointer_leave,
    WaylandCallbacks::pointer_motion,
    WaylandCallbacks::pointer_button,
    WaylandCallbacks::pointer_axis
};

const wl_keyboard_listener WaylandCallbacks::keyboard_listener =
{
    WaylandCallbacks::keyboard_keymap,
    WaylandCallbacks::keyboard_enter,
    WaylandCallbacks::keyboard_leave,
    WaylandCallbacks::keyboard_key,
    WaylandCallbacks::keyboard_modifiers,
    WaylandCallbacks::keyboard_repeat_info
};

const wl_seat_listener WaylandCallbacks::seat_listener =
{
    WaylandCallbacks::seat_capabilities
};

const wl_output_listener WaylandCallbacks::output_listener =
{
    WaylandCallbacks::output_geometry,
    WaylandCallbacks::output_mode,
    WaylandCallbacks::output_done,
    WaylandCallbacks::output_scale
};

const wl_shell_surface_listener WaylandCallbacks::shell_surface_listener =
{
    WaylandCallbacks::shell_surface_ping,
    WaylandCallbacks::shell_surface_configure,
    WaylandCallbacks::shell_surface_popup_done
};

const wl_registry_listener WaylandCallbacks::registry_listener =
{
    WaylandCallbacks::registry_global,
    WaylandCallbacks::registry_global_remove
};



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

CIrrDeviceWayland::CIrrDeviceWayland(const SIrrlichtCreationParameters& params)
    : CIrrDeviceStub(params)
{
    #ifdef _DEBUG
    setDebugName("CIrrDeviceWayland");
    #endif

    m_compositor = NULL;
    m_cursor = NULL;
    m_cursor_theme = NULL;
    m_display = NULL;
    m_egl_window = NULL;
    m_keyboard = NULL;
    m_output = NULL;
    m_pointer = NULL;
    m_registry = NULL;
    m_seat = NULL;
    m_shell = NULL;
    m_shell_surface = NULL;
    m_shm = NULL;
    m_cursor_surface = NULL;
    m_surface = NULL;
    m_enter_serial = 0;

    m_xkb_context = NULL;
    m_xkb_compose_table = NULL;
    m_xkb_compose_state = NULL;
    m_xkb_keymap = NULL;
    m_xkb_state = NULL;
    m_xkb_alt_mask = 0;
    m_xkb_ctrl_mask = 0;
    m_xkb_shift_mask = 0;
    m_xkb_alt_pressed = false;
    m_xkb_ctrl_pressed = false;
    m_xkb_shift_pressed = false;

    m_egl_context = NULL;

    m_mouse_button_states = 0;
    m_width = params.WindowSize.Width;
    m_height = params.WindowSize.Height;
    m_window_has_focus = false;
    m_window_minimized = false;

    utsname LinuxInfo;
    uname(&LinuxInfo);

    core::stringc linuxversion;
    linuxversion += LinuxInfo.sysname;
    linuxversion += " ";
    linuxversion += LinuxInfo.release;
    linuxversion += " ";
    linuxversion += LinuxInfo.version;
    linuxversion += " ";
    linuxversion += LinuxInfo.machine;

    Operator = new COSOperator(linuxversion, this);
    os::Printer::log(linuxversion.c_str(), ELL_INFORMATION);

    CursorControl = new CCursorControl(this);

    createKeyMap();

    m_display = wl_display_connect(NULL);
    m_registry = wl_display_get_registry(m_display);
    wl_registry_add_listener(m_registry, &WaylandCallbacks::registry_listener, this);
    wl_display_dispatch(m_display);

    m_xkb_context = xkb_context_new(XKB_CONTEXT_NO_FLAGS);

    if (CreationParams.DriverType != video::EDT_NULL)
    {
        if (!createWindow())
            return;
    }

    wl_seat_add_listener(m_seat, &WaylandCallbacks::seat_listener, this);
    wl_output_add_listener(m_output, &WaylandCallbacks::output_listener, this);

    createDriver();

    if (VideoDriver)
        createGUIAndScene();

    wl_display_dispatch(m_display);
}

//! destructor
CIrrDeviceWayland::~CIrrDeviceWayland()
{
    delete m_egl_context;

    if (m_keyboard)
        wl_keyboard_destroy(m_keyboard);

    if (m_pointer)
        wl_pointer_destroy(m_pointer);

    if (m_cursor_surface)
        wl_surface_destroy(m_cursor_surface);

    if (m_cursor_theme)
        wl_cursor_theme_destroy(m_cursor_theme);

    if (m_shell_surface)
        wl_shell_surface_destroy(m_shell_surface);

    wl_output_destroy(m_output);
    wl_seat_destroy(m_seat);
    wl_registry_destroy(m_registry);
    wl_display_flush(m_display);
    wl_display_disconnect(m_display);

    xkb_context_unref(m_xkb_context);

    closeJoysticks();
}

bool CIrrDeviceWayland::initEGL()
{
    m_egl_window = wl_egl_window_create(m_surface, m_width, m_height);

    m_egl_context = new ContextManagerEGL();

    ContextEGLParams egl_params;
    egl_params.opengl_api = CEGL_API_OPENGL;
    egl_params.surface_type = CEGL_SURFACE_WINDOW;
    egl_params.force_legacy_device = CreationParams.ForceLegacyDevice;
    egl_params.with_alpha_channel = CreationParams.WithAlphaChannel;
    egl_params.vsync_enabled = CreationParams.Vsync;
    egl_params.window = m_egl_window;
    egl_params.display = m_display;

    bool success = m_egl_context->init(egl_params);

    if (!success)
        return false;

    video::useCoreContext = !m_egl_context->isLegacyDevice();

    int w = 0;
    int h = 0;

    if (m_egl_context->getSurfaceDimensions(&w, &h))
    {
        m_width = w;
        m_height = h;
        CreationParams.WindowSize.Width = m_width;
        CreationParams.WindowSize.Height = m_height;
    }

    return true;
}

bool CIrrDeviceWayland::createWindow()
{
    m_surface = wl_compositor_create_surface(m_compositor);
    m_shell_surface = wl_shell_get_shell_surface(m_shell, m_surface);

    wl_shell_surface_add_listener(m_shell_surface,
                               &WaylandCallbacks::shell_surface_listener, this);

    if (CreationParams.Fullscreen)
    {
        wl_shell_surface_set_fullscreen(m_shell_surface,
                       WL_SHELL_SURFACE_FULLSCREEN_METHOD_DEFAULT, 0, m_output);
    }
    else
    {
        wl_shell_surface_set_toplevel(m_shell_surface);
    }

    wl_display_flush(m_display);

    bool success = initEGL();

    if (!success)
    {
        os::Printer::log("Couldn't create OpenGL context.", ELL_ERROR);
        return false;
    }

    wl_region* region = wl_compositor_create_region(m_compositor);
    wl_region_add(region, 0, 0, m_width, m_height);
    wl_surface_set_opaque_region(m_surface, region);
    wl_region_destroy(region);

    wl_display_flush(m_display);

    m_cursor_surface = wl_compositor_create_surface(m_compositor);
    m_cursor_theme = wl_cursor_theme_load(NULL, 32, m_shm);

    if (!m_cursor_theme)
    {
        os::Printer::log("Couldn't load cursor theme.", ELL_ERROR);
    }
    else
    {
        m_cursor = wl_cursor_theme_get_cursor(m_cursor_theme, "left_ptr");

        if (!m_cursor)
        {
            os::Printer::log("Couldn't load left pointer cursor.", ELL_ERROR);
        }
    }

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
        #ifdef _IRR_COMPILE_WITH_OPENGL_
        VideoDriver = video::createOpenGLDriver(CreationParams, FileSystem, this);
        #else
        os::Printer::log("No OpenGL support compiled in.", ELL_ERROR);
        #endif
        break;
    }
}

void CIrrDeviceWayland::swapBuffers()
{
    wl_display_dispatch_pending(m_display);
    m_egl_context->swapBuffers();
}

void CIrrDeviceWayland::updateCursor()
{
    if (!getCursorControl()->isVisible() && CreationParams.Fullscreen)
    {
        wl_pointer_set_cursor(m_pointer, m_enter_serial, NULL, 0, 0);
    }
    else if (m_cursor)
    {
        wl_cursor_image* image = m_cursor->images[0];
        wl_buffer* buffer = wl_cursor_image_get_buffer(image);

        if (!buffer)
            return;

        wl_pointer_set_cursor(m_pointer, m_enter_serial, m_cursor_surface,
                              image->hotspot_x, image->hotspot_y);
        wl_surface_attach(m_cursor_surface, buffer, 0, 0);
        wl_surface_damage(m_cursor_surface, 0, 0, image->width, image->height);
        wl_surface_commit(m_cursor_surface);
    }
}

void CIrrDeviceWayland::signalEvent(const SEvent &event)
{
    m_events.push_back(event);
}

//! runs the device. Returns false if device wants to be deleted
bool CIrrDeviceWayland::run()
{
    os::Timer::tick();

    for (unsigned int i = 0; i < m_events.size(); i++)
    {
        postEventFromUser(m_events[i]);
    }

    m_events.clear();

    if (!Close)
        pollJoysticks();

    return !Close;
}

//! Pause the current process for the minimum time allowed only to allow other
//! processes to execute
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
    ts.tv_nsec = (long) (timeMs % 1000)*  1000000;

    if (pauseTimer && !wasStopped)
        Timer->stop();

    nanosleep(&ts, NULL);

    if (pauseTimer && !wasStopped)
        Timer->start();
}

//! sets the caption of the window
void CIrrDeviceWayland::setWindowCaption(const wchar_t* text)
{
}

//! presents a surface in the client area
bool CIrrDeviceWayland::present(video::IImage* image, void* windowId,
                                core::rect<s32>* srcRect)
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
    return (m_window_has_focus && !m_window_minimized);
}

//! returns if window has focus.
bool CIrrDeviceWayland::isWindowFocused() const
{
    return m_window_has_focus;
}

//! returns if window is minimized.
bool CIrrDeviceWayland::isWindowMinimized() const
{
    return m_window_minimized;
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

//! Set the current Gamma Value for the Display
bool CIrrDeviceWayland::setGammaRamp(f32 red, f32 green, f32 blue,
                                     f32 brightness, f32 contrast)
{
    return false;
}

//! Get the current Gamma Value for the Display
bool CIrrDeviceWayland::getGammaRamp(f32 &red, f32 &green, f32 &blue,
                                     f32 &brightness, f32 &contrast)
{
    brightness = 0.0f;
    contrast = 0.0f;
    return false;
}

//! gets text from the clipboard
//! \return Returns 0 if no string is in there.
const c8* CIrrDeviceWayland::getTextFromClipboard() const
{
    return m_clipboard.c_str();
}

//! copies text to the clipboard
void CIrrDeviceWayland::copyToClipboard(const c8* text) const
{
    m_clipboard = text;
}

//! Remove all messages pending in the system message loop
void CIrrDeviceWayland::clearSystemMessages()
{
}

void CIrrDeviceWayland::createKeyMap()
{
    m_key_map[0] = KEY_UNKNOWN; //KEY_RESERVED
    m_key_map[1] = KEY_ESCAPE; //KEY_ESC
    m_key_map[2] = KEY_KEY_1; //KEY_1
    m_key_map[3] = KEY_KEY_2; //KEY_2
    m_key_map[4] = KEY_KEY_3; //KEY_3
    m_key_map[5] = KEY_KEY_4; //KEY_4
    m_key_map[6] = KEY_KEY_5; //KEY_5
    m_key_map[7] = KEY_KEY_6; //KEY_6
    m_key_map[8] = KEY_KEY_7; //KEY_7
    m_key_map[9] = KEY_KEY_8; //KEY_8
    m_key_map[10] = KEY_KEY_9; //KEY_9
    m_key_map[11] = KEY_KEY_0; //KEY_0
    m_key_map[12] = KEY_MINUS; //KEY_MINUS
    m_key_map[13] = KEY_PLUS; //KEY_EQUAL
    m_key_map[14] = KEY_BACK; //KEY_BACKSPACE
    m_key_map[15] = KEY_TAB; //KEY_TAB
    m_key_map[16] = KEY_KEY_Q; //KEY_Q
    m_key_map[17] = KEY_KEY_W; //KEY_W
    m_key_map[18] = KEY_KEY_E; //KEY_E
    m_key_map[19] = KEY_KEY_R; //KEY_R
    m_key_map[20] = KEY_KEY_T; //KEY_T
    m_key_map[21] = KEY_KEY_Y; //KEY_Y
    m_key_map[22] = KEY_KEY_U; //KEY_U
    m_key_map[23] = KEY_KEY_I; //KEY_I
    m_key_map[25] = KEY_KEY_P; //KEY_P
    m_key_map[24] = KEY_KEY_O; //KEY_O
    m_key_map[26] = KEY_OEM_4; //KEY_LEFTBRACE
    m_key_map[27] = KEY_OEM_6; //KEY_RIGHTBRACE
    m_key_map[28] = KEY_RETURN; //KEY_ENTER
    m_key_map[29] = KEY_LCONTROL; //KEY_LEFTCTRL
    m_key_map[30] = KEY_KEY_A; //KEY_A
    m_key_map[31] = KEY_KEY_S; // KEY_S
    m_key_map[32] = KEY_KEY_D; //KEY_D
    m_key_map[33] = KEY_KEY_F; //KEY_F
    m_key_map[34] = KEY_KEY_G; //KEY_G
    m_key_map[35] = KEY_KEY_H; //KEY_H
    m_key_map[36] = KEY_KEY_J; //KEY_J
    m_key_map[37] = KEY_KEY_K; //KEY_K
    m_key_map[38] = KEY_KEY_L; //KEY_L
    m_key_map[39] = KEY_OEM_1; //KEY_SEMICOLON
    m_key_map[40] = KEY_OEM_7; //KEY_APOSTROPHE
    m_key_map[41] = KEY_OEM_3; //KEY_GRAVE
    m_key_map[42] = KEY_LSHIFT; //KEY_LEFTSHIFT
    m_key_map[43] = KEY_OEM_5; //KEY_BACKSLASH
    m_key_map[44] = KEY_KEY_Z; //KEY_Z
    m_key_map[45] = KEY_KEY_X; //KEY_X
    m_key_map[46] = KEY_KEY_C; //KEY_C
    m_key_map[47] = KEY_KEY_V; //KEY_V
    m_key_map[48] = KEY_KEY_B; //KEY_B
    m_key_map[49] = KEY_KEY_N; //KEY_N
    m_key_map[50] = KEY_KEY_M; //KEY_M
    m_key_map[51] = KEY_COMMA; //KEY_COMMA
    m_key_map[52] = KEY_PERIOD; //KEY_DOT
    m_key_map[53] = KEY_OEM_2; // KEY_SLASH
    m_key_map[54] = KEY_RSHIFT; //KEY_RIGHTSHIFT
    m_key_map[55] = KEY_MULTIPLY; //KEY_KPASTERISK
    m_key_map[56] = KEY_LMENU; //KEY_LEFTALT
    m_key_map[57] = KEY_SPACE; //KEY_SPACE
    m_key_map[58] = KEY_CAPITAL; //KEY_CAPSLOCK
    m_key_map[59] = KEY_F1; //KEY_F1
    m_key_map[60] = KEY_F2; //KEY_F2
    m_key_map[61] = KEY_F3; //KEY_F3
    m_key_map[62] = KEY_F4; //KEY_F4
    m_key_map[63] = KEY_F5; //KEY_F5
    m_key_map[64] = KEY_F6; //KEY_F6
    m_key_map[65] = KEY_F7; //KEY_F7
    m_key_map[66] = KEY_F8; //KEY_F8
    m_key_map[67] = KEY_F9; //KEY_F9
    m_key_map[68] = KEY_F10; //KEY_F10
    m_key_map[69] = KEY_NUMLOCK; //KEY_NUMLOCK
    m_key_map[70] = KEY_SCROLL; //KEY_SCROLLLOCK
    m_key_map[71] = KEY_NUMPAD7; //KEY_KP7
    m_key_map[72] = KEY_NUMPAD8; //KEY_KP8
    m_key_map[73] = KEY_NUMPAD9; //KEY_KP9
    m_key_map[74] = KEY_SUBTRACT; //KEY_KPMINUS
    m_key_map[75] = KEY_NUMPAD4; //KEY_KP4
    m_key_map[76] = KEY_NUMPAD5; //KEY_KP5
    m_key_map[77] = KEY_NUMPAD6; //KEY_KP6
    m_key_map[78] = KEY_ADD; //KEY_KPPLUS
    m_key_map[79] = KEY_NUMPAD1; //KEY_KP1
    m_key_map[80] = KEY_NUMPAD2; //KEY_KP2
    m_key_map[81] = KEY_NUMPAD3; //KEY_KP3
    m_key_map[82] = KEY_NUMPAD0; //KEY_KP0
    m_key_map[83] = KEY_SEPARATOR; //KEY_KPDOT
    m_key_map[85] = KEY_UNKNOWN; //KEY_ZENKAKUHANKAKU
    m_key_map[86] = KEY_OEM_102; //KEY_102ND
    m_key_map[87] = KEY_F11; //KEY_F11
    m_key_map[88] = KEY_F12; //KEY_F12
    m_key_map[89] = KEY_UNKNOWN; //KEY_RO
    m_key_map[90] = KEY_UNKNOWN; //KEY_KATAKANA
    m_key_map[91] = KEY_UNKNOWN; //KEY_HIRAGANA
    m_key_map[92] = KEY_UNKNOWN; //KEY_HENKAN
    m_key_map[93] = KEY_UNKNOWN; //KEY_KATAKANAHIRAGANA
    m_key_map[94] = KEY_UNKNOWN; //KEY_MUHENKAN
    m_key_map[95] = KEY_SEPARATOR; //KEY_KPJPCOMMA
    m_key_map[96] = KEY_RETURN; //KEY_KPENTER
    m_key_map[97] = KEY_RCONTROL; //KEY_RIGHTCTRL
    m_key_map[98] = KEY_DIVIDE; //KEY_KPSLASH
    m_key_map[99] = KEY_UNKNOWN; //KEY_SYSRQ
    m_key_map[100] = KEY_RMENU; //KEY_RIGHTALT
    m_key_map[101] = KEY_UNKNOWN; //KEY_LINEFEED
    m_key_map[102] = KEY_HOME; //KEY_HOME
    m_key_map[103] = KEY_UP; //KEY_UP
    m_key_map[104] = KEY_PRIOR; //KEY_PAGEUP
    m_key_map[105] = KEY_LEFT; //KEY_LEFT
    m_key_map[106] = KEY_RIGHT; //KEY_RIGHT
    m_key_map[107] = KEY_END; //KEY_END
    m_key_map[108] = KEY_DOWN; //KEY_DOWN
    m_key_map[109] = KEY_NEXT; //KEY_PAGEDOWN
    m_key_map[110] = KEY_INSERT; //KEY_INSERT
    m_key_map[111] = KEY_DELETE; //KEY_DELETE
    m_key_map[112] = KEY_UNKNOWN; //KEY_MACRO
    m_key_map[113] = KEY_VOLUME_MUTE; //KEY_MUTE
    m_key_map[114] = KEY_VOLUME_DOWN; //KEY_VOLUMEDOWN
    m_key_map[115] = KEY_VOLUME_UP; //KEY_VOLUMEUP
    m_key_map[116] = KEY_UNKNOWN; //KEY_POWER
    m_key_map[117] = KEY_RETURN; //KEY_KPEQUAL
    m_key_map[118] = KEY_PLUS; //KEY_KPPLUSMINUS
    m_key_map[119] = KEY_PAUSE; //KEY_PAUSE
    m_key_map[120] = KEY_UNKNOWN; //KEY_SCALE
    m_key_map[121] = KEY_COMMA; //KEY_KPCOMMA
    m_key_map[122] = KEY_UNKNOWN; //KEY_HANGEUL
    m_key_map[123] = KEY_UNKNOWN; //KEY_HANJA
    m_key_map[124] = KEY_UNKNOWN; //KEY_YEN
    m_key_map[125] = KEY_LWIN; //KEY_LEFTMETA
    m_key_map[126] = KEY_RWIN; //KEY_RIGHTMETA
    m_key_map[127] = KEY_MENU; //KEY_COMPOSE
}

// The joystick code is mostly copied from CIrrDeviceLinux.

bool CIrrDeviceWayland::activateJoysticks(core::array<SJoystickInfo>& joystickInfo)
{
#if defined (_IRR_COMPILE_WITH_JOYSTICK_EVENTS_)

    joystickInfo.clear();

    u32 joystick;
    for (joystick = 0; joystick < 32; ++joystick)
    {
        // The joystick device could be here...
        core::stringc devName = "/dev/js";
        devName += joystick;

        JoystickInfo info;
        info.fd = open(devName.c_str(), O_RDONLY);
        if (info.fd == -1)
        {
            // ...but Ubuntu and possibly other distros
            // create the devices in /dev/input
            devName = "/dev/input/js";
            devName += joystick;
            info.fd = open(devName.c_str(), O_RDONLY);
        }

        if (info.fd == -1)
        {
            // and BSD here
            devName = "/dev/joy";
            devName += joystick;
            info.fd = open(devName.c_str(), O_RDONLY);
        }

        if (info.fd == -1)
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
        info.persistentData.JoystickEvent.Joystick = m_active_joysticks.size();

        // There's no obvious way to determine which (if any) axes represent a POV
        // hat, so we'll just set it to "not used" and forget about it.
        info.persistentData.JoystickEvent.POV = 65535;

        m_active_joysticks.push_back(info);

        SJoystickInfo returnInfo;
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
    if (m_active_joysticks.size() == 0)
        return;

    for (unsigned int i = 0; i < m_active_joysticks.size(); i++)
    {
        JoystickInfo& info = m_active_joysticks[i];

#ifdef __FreeBSD__
        struct joystick js;
        if (read(info.fd, &js, sizeof(js)) == sizeof(js))
        {
            /* should be a two-bit field*/
            info.persistentData.JoystickEvent.ButtonStates = js.b1 | (js.b2 << 1);
            info.persistentData.JoystickEvent.Axis[0] = js.x; /* X axis*/
            info.persistentData.JoystickEvent.Axis[1] = js.y; /* Y axis*/
        }
#else
        struct js_event event;
        while (sizeof(event) == read(info.fd, &event, sizeof(event)))
        {
            switch(event.type & ~JS_EVENT_INIT)
            {
            case JS_EVENT_BUTTON:
            {
                if (event.value)
                {
                    info.persistentData.JoystickEvent.ButtonStates |= (1 << event.number);
                }
                else
                {
                    info.persistentData.JoystickEvent.ButtonStates &= ~(1 << event.number);
                }
                break;
            }
            case JS_EVENT_AXIS:
            {
                if (event.number < SEvent::SJoystickEvent::NUMBER_OF_AXES)
                {
                    info.persistentData.JoystickEvent.Axis[event.number] = event.value;
                }
                break;
            }
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

void CIrrDeviceWayland::closeJoysticks()
{
#if defined(_IRR_COMPILE_WITH_JOYSTICK_EVENTS_)
    for (unsigned int i = 0; i < m_active_joysticks.size(); i++)
    {
        if (m_active_joysticks[i].fd < 0)
            continue;

        close(m_active_joysticks[i].fd);
    }
#endif
}

} // end namespace

#endif
