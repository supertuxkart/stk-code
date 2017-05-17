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

#ifdef _IRR_COMPILE_WITH_WAYLAND

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

#define MOD_SHIFT_MASK   0x01
#define MOD_ALT_MASK     0x02
#define MOD_CONTROL_MASK 0x04

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
        irrevent.MouseInput.Control = (device->m_xkb_modifiers & MOD_CONTROL_MASK) != 0;
        irrevent.MouseInput.Shift = (device->m_xkb_modifiers & MOD_SHIFT_MASK) != 0;
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
        irrevent.MouseInput.Control = (device->m_xkb_modifiers & MOD_CONTROL_MASK) != 0;
        irrevent.MouseInput.Shift = (device->m_xkb_modifiers & MOD_SHIFT_MASK) != 0;
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
            irrevent.MouseInput.Control = (device->m_xkb_modifiers & MOD_CONTROL_MASK) != 0;
            irrevent.MouseInput.Shift = (device->m_xkb_modifiers & MOD_SHIFT_MASK) != 0;
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

        device->m_xkb_control_mask =
            1 << xkb_keymap_mod_get_index(device->m_xkb_keymap, "Control");
        device->m_xkb_alt_mask =
            1 << xkb_keymap_mod_get_index(device->m_xkb_keymap, "Mod1");
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
                             uint32_t time, uint32_t key, uint32_t state_w)
    {
        CIrrDeviceWayland* device = static_cast<CIrrDeviceWayland*>(data);

        if (!device->m_xkb_state)
            return;

        wl_keyboard_key_state state = (wl_keyboard_key_state)state_w;
        uint32_t code = key + 8;
        xkb_keysym_t sym = XKB_KEY_NoSymbol;

        const xkb_keysym_t* syms;
        uint32_t num_syms = xkb_state_key_get_syms(device->m_xkb_state, code,
                                                   &syms);

        if (num_syms == 1)
            sym = syms[0];

        bool ignore = false;

        if (sym != XKB_KEY_NoSymbol && device->m_xkb_compose_state)
        {
            xkb_compose_feed_result result;
            result = xkb_compose_state_feed(device->m_xkb_compose_state, sym);

            if (result == XKB_COMPOSE_FEED_ACCEPTED)
            {
                xkb_compose_status status;
                status = xkb_compose_state_get_status(
                                                   device->m_xkb_compose_state);

                if (status == XKB_COMPOSE_COMPOSING ||
                    status == XKB_COMPOSE_CANCELLED)
                {
                    ignore = true;
                }

                if (status == XKB_COMPOSE_COMPOSED)
                {
                    sym = xkb_compose_state_get_one_sym(
                                                   device->m_xkb_compose_state);
                }
            }
        }

        if (ignore == true)
            return;

        SEvent irrevent;
        irrevent.EventType = irr::EET_KEY_INPUT_EVENT;
        irrevent.KeyInput.PressedDown = (state == WL_KEYBOARD_KEY_STATE_PRESSED);
        irrevent.KeyInput.Char = xkb_keysym_to_utf32(sym);
        irrevent.KeyInput.Control = (device->m_xkb_modifiers &
                                                        MOD_CONTROL_MASK) != 0;
        irrevent.KeyInput.Shift = (device->m_xkb_modifiers &
                                                        MOD_SHIFT_MASK) != 0;
        irrevent.KeyInput.Key = device->m_key_map[sym];

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
        xkb_mod_mask_t mask = xkb_state_serialize_mods(device->m_xkb_state,
                                                       state_component);

        device->m_xkb_modifiers = 0;

        if (mask & device->m_xkb_control_mask)
        {
            device->m_xkb_modifiers |= MOD_CONTROL_MASK;
        }

        if (mask & device->m_xkb_alt_mask)
        {
            device->m_xkb_modifiers |= MOD_ALT_MASK;
        }

        if (mask & device->m_xkb_shift_mask)
        {
            device->m_xkb_modifiers |= MOD_SHIFT_MASK;
        }
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
    m_xkb_control_mask = 0;
    m_xkb_alt_mask = 0;
    m_xkb_shift_mask = 0;
    m_xkb_modifiers = 0;

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
    m_key_map[XKB_KEY_NoSymbol] = KEY_UNKNOWN;
    m_key_map[XKB_KEY_BackSpace] = KEY_BACK;
    m_key_map[XKB_KEY_Tab] = KEY_TAB;
    m_key_map[XKB_KEY_ISO_Left_Tab] = KEY_TAB;
//  m_key_map[XK_Linefeed] = 0; // ???
    m_key_map[XKB_KEY_Clear] = KEY_CLEAR;
    m_key_map[XKB_KEY_Return] = KEY_RETURN;
    m_key_map[XKB_KEY_Pause] = KEY_PAUSE;
    m_key_map[XKB_KEY_Scroll_Lock] = KEY_SCROLL;
//  m_key_map[XK_Sys_Req] = 0; // ???
    m_key_map[XKB_KEY_Escape] = KEY_ESCAPE;
    m_key_map[XKB_KEY_Insert] = KEY_INSERT;
    m_key_map[XKB_KEY_Delete] = KEY_DELETE;
    m_key_map[XKB_KEY_Home] = KEY_HOME;
    m_key_map[XKB_KEY_Left] = KEY_LEFT;
    m_key_map[XKB_KEY_Up] = KEY_UP;
    m_key_map[XKB_KEY_Right] = KEY_RIGHT;
    m_key_map[XKB_KEY_Down] = KEY_DOWN;
    m_key_map[XKB_KEY_Prior] = KEY_PRIOR;
    m_key_map[XKB_KEY_Page_Up] = KEY_PRIOR;
    m_key_map[XKB_KEY_Next] = KEY_NEXT;
    m_key_map[XKB_KEY_Page_Down] = KEY_NEXT;
    m_key_map[XKB_KEY_End] = KEY_END;
    m_key_map[XKB_KEY_Begin] = KEY_HOME;
    m_key_map[XKB_KEY_Num_Lock] = KEY_NUMLOCK;
    m_key_map[XKB_KEY_space] = KEY_SPACE;
    m_key_map[XKB_KEY_KP_Tab] = KEY_TAB;
    m_key_map[XKB_KEY_KP_Enter] = KEY_RETURN;
    m_key_map[XKB_KEY_KP_F1] = KEY_F1;
    m_key_map[XKB_KEY_KP_F2] = KEY_F2;
    m_key_map[XKB_KEY_KP_F3] = KEY_F3;
    m_key_map[XKB_KEY_KP_F4] = KEY_F4;
    m_key_map[XKB_KEY_KP_Home] = KEY_HOME;
    m_key_map[XKB_KEY_KP_Left] = KEY_LEFT;
    m_key_map[XKB_KEY_KP_Up] = KEY_UP;
    m_key_map[XKB_KEY_KP_Right] = KEY_RIGHT;
    m_key_map[XKB_KEY_KP_Down] = KEY_DOWN;
    m_key_map[XKB_KEY_Print] = KEY_PRINT;
    m_key_map[XKB_KEY_KP_Prior] = KEY_PRIOR;
    m_key_map[XKB_KEY_KP_Page_Up] = KEY_PRIOR;
    m_key_map[XKB_KEY_KP_Next] = KEY_NEXT;
    m_key_map[XKB_KEY_KP_Page_Down] = KEY_NEXT;
    m_key_map[XKB_KEY_KP_End] = KEY_END;
    m_key_map[XKB_KEY_KP_Begin] = KEY_HOME;
    m_key_map[XKB_KEY_KP_Insert] = KEY_INSERT;
    m_key_map[XKB_KEY_KP_Delete] = KEY_DELETE;
//  m_key_map[XK_KP_Equal] = 0; // ???
    m_key_map[XKB_KEY_KP_Multiply] = KEY_MULTIPLY;
    m_key_map[XKB_KEY_KP_Add] = KEY_ADD;
    m_key_map[XKB_KEY_KP_Separator] = KEY_SEPARATOR;
    m_key_map[XKB_KEY_KP_Subtract] = KEY_SUBTRACT;
    m_key_map[XKB_KEY_KP_Decimal] = KEY_DECIMAL;
    m_key_map[XKB_KEY_KP_Divide] = KEY_DIVIDE;
    m_key_map[XKB_KEY_KP_0] = KEY_NUMPAD0;
    m_key_map[XKB_KEY_KP_1] = KEY_NUMPAD1;
    m_key_map[XKB_KEY_KP_2] = KEY_NUMPAD2;
    m_key_map[XKB_KEY_KP_3] = KEY_NUMPAD3;
    m_key_map[XKB_KEY_KP_4] = KEY_NUMPAD4;
    m_key_map[XKB_KEY_KP_5] = KEY_NUMPAD5;
    m_key_map[XKB_KEY_KP_6] = KEY_NUMPAD6;
    m_key_map[XKB_KEY_KP_7] = KEY_NUMPAD7;
    m_key_map[XKB_KEY_KP_8] = KEY_NUMPAD8;
    m_key_map[XKB_KEY_KP_9] = KEY_NUMPAD9;
    m_key_map[XKB_KEY_F1] = KEY_F1;
    m_key_map[XKB_KEY_F2] = KEY_F2;
    m_key_map[XKB_KEY_F3] = KEY_F3;
    m_key_map[XKB_KEY_F4] = KEY_F4;
    m_key_map[XKB_KEY_F5] = KEY_F5;
    m_key_map[XKB_KEY_F6] = KEY_F6;
    m_key_map[XKB_KEY_F7] = KEY_F7;
    m_key_map[XKB_KEY_F8] = KEY_F8;
    m_key_map[XKB_KEY_F9] = KEY_F9;
    m_key_map[XKB_KEY_F10] = KEY_F10;
    m_key_map[XKB_KEY_F11] = KEY_F11;
    m_key_map[XKB_KEY_F12] = KEY_F12;
    m_key_map[XKB_KEY_Shift_L] = KEY_LSHIFT;
    m_key_map[XKB_KEY_Shift_R] = KEY_RSHIFT;
    m_key_map[XKB_KEY_Control_L] = KEY_LCONTROL;
    m_key_map[XKB_KEY_Control_R] = KEY_RCONTROL;
    m_key_map[XKB_KEY_Caps_Lock] = KEY_CAPITAL;
    m_key_map[XKB_KEY_Shift_Lock] = KEY_CAPITAL;
    m_key_map[XKB_KEY_Meta_L] = KEY_LWIN;
    m_key_map[XKB_KEY_Meta_R] = KEY_RWIN;
    m_key_map[XKB_KEY_Alt_L] = KEY_LMENU;
    m_key_map[XKB_KEY_Alt_R] = KEY_RMENU;
    m_key_map[XKB_KEY_ISO_Level3_Shift] = KEY_RMENU;
    m_key_map[XKB_KEY_Menu] = KEY_MENU;
    m_key_map[XKB_KEY_space] = KEY_SPACE;
//  m_key_map[XKB_key_ex] = 0; //?
//  m_key_map[XK_quotedbl] = 0; //?
//  m_key_map[XK_section] = 0; //?
    m_key_map[XKB_KEY_numbersign] = KEY_OEM_2;
//  m_key_map[XK_dollar] = 0; //?
//  m_key_map[XK_percent] = 0; //?
//  m_key_map[XK_ampersand] = 0; //?
    m_key_map[XKB_KEY_apostrophe] = KEY_OEM_7;
//  m_key_map[XK_parenleft] = 0; //?
//  m_key_map[XK_parenright] = 0; //?
//  m_key_map[XK_asterisk] = 0; //?
    m_key_map[XKB_KEY_plus] = KEY_PLUS; //?
    m_key_map[XKB_KEY_comma] = KEY_COMMA; //?
    m_key_map[XKB_KEY_minus] = KEY_MINUS; //?
    m_key_map[XKB_KEY_period] = KEY_PERIOD; //?
    m_key_map[XKB_KEY_slash] = KEY_OEM_2; //?
    m_key_map[XKB_KEY_0] = KEY_KEY_0;
    m_key_map[XKB_KEY_1] = KEY_KEY_1;
    m_key_map[XKB_KEY_2] = KEY_KEY_2;
    m_key_map[XKB_KEY_3] = KEY_KEY_3;
    m_key_map[XKB_KEY_4] = KEY_KEY_4;
    m_key_map[XKB_KEY_5] = KEY_KEY_5;
    m_key_map[XKB_KEY_6] = KEY_KEY_6;
    m_key_map[XKB_KEY_7] = KEY_KEY_7;
    m_key_map[XKB_KEY_8] = KEY_KEY_8;
    m_key_map[XKB_KEY_9] = KEY_KEY_9;
//  m_key_map[XK_colon] = 0; //?
    m_key_map[XKB_KEY_semicolon] = KEY_OEM_1;
    m_key_map[XKB_KEY_less] = KEY_OEM_102;
    m_key_map[XKB_KEY_equal] = KEY_PLUS;
//  m_key_map[XK_greater] = 0; //?
//  m_key_map[XK_question] = 0; //?
    m_key_map[XKB_KEY_at] = KEY_KEY_2; //?
//  m_key_map[XK_mu] = 0; //?
//  m_key_map[XK_EuroSign] = 0; //?
    m_key_map[XKB_KEY_A] = KEY_KEY_A;
    m_key_map[XKB_KEY_B] = KEY_KEY_B;
    m_key_map[XKB_KEY_C] = KEY_KEY_C;
    m_key_map[XKB_KEY_D] = KEY_KEY_D;
    m_key_map[XKB_KEY_E] = KEY_KEY_E;
    m_key_map[XKB_KEY_F] = KEY_KEY_F;
    m_key_map[XKB_KEY_G] = KEY_KEY_G;
    m_key_map[XKB_KEY_H] = KEY_KEY_H;
    m_key_map[XKB_KEY_I] = KEY_KEY_I;
    m_key_map[XKB_KEY_J] = KEY_KEY_J;
    m_key_map[XKB_KEY_K] = KEY_KEY_K;
    m_key_map[XKB_KEY_L] = KEY_KEY_L;
    m_key_map[XKB_KEY_M] = KEY_KEY_M;
    m_key_map[XKB_KEY_N] = KEY_KEY_N;
    m_key_map[XKB_KEY_O] = KEY_KEY_O;
    m_key_map[XKB_KEY_P] = KEY_KEY_P;
    m_key_map[XKB_KEY_Q] = KEY_KEY_Q;
    m_key_map[XKB_KEY_R] = KEY_KEY_R;
    m_key_map[XKB_KEY_S] = KEY_KEY_S;
    m_key_map[XKB_KEY_T] = KEY_KEY_T;
    m_key_map[XKB_KEY_U] = KEY_KEY_U;
    m_key_map[XKB_KEY_V] = KEY_KEY_V;
    m_key_map[XKB_KEY_W] = KEY_KEY_W;
    m_key_map[XKB_KEY_X] = KEY_KEY_X;
    m_key_map[XKB_KEY_Y] = KEY_KEY_Y;
    m_key_map[XKB_KEY_Z] = KEY_KEY_Z;
    m_key_map[XKB_KEY_bracketleft] = KEY_OEM_4;
    m_key_map[XKB_KEY_backslash] = KEY_OEM_5;
    m_key_map[XKB_KEY_bracketright] = KEY_OEM_6;
    m_key_map[XKB_KEY_asciicircum] = KEY_OEM_5;
//  m_key_map[XK_degree] = 0; //?
    m_key_map[XKB_KEY_underscore] = KEY_MINUS; //?
    m_key_map[XKB_KEY_grave] = KEY_OEM_3;
    m_key_map[XKB_KEY_acute] = KEY_OEM_6;
    m_key_map[XKB_KEY_a] = KEY_KEY_A;
    m_key_map[XKB_KEY_b] = KEY_KEY_B;
    m_key_map[XKB_KEY_c] = KEY_KEY_C;
    m_key_map[XKB_KEY_d] = KEY_KEY_D;
    m_key_map[XKB_KEY_e] = KEY_KEY_E;
    m_key_map[XKB_KEY_f] = KEY_KEY_F;
    m_key_map[XKB_KEY_g] = KEY_KEY_G;
    m_key_map[XKB_KEY_h] = KEY_KEY_H;
    m_key_map[XKB_KEY_i] = KEY_KEY_I;
    m_key_map[XKB_KEY_j] = KEY_KEY_J;
    m_key_map[XKB_KEY_k] = KEY_KEY_K;
    m_key_map[XKB_KEY_l] = KEY_KEY_L;
    m_key_map[XKB_KEY_m] = KEY_KEY_M;
    m_key_map[XKB_KEY_n] = KEY_KEY_N;
    m_key_map[XKB_KEY_o] = KEY_KEY_O;
    m_key_map[XKB_KEY_p] = KEY_KEY_P;
    m_key_map[XKB_KEY_q] = KEY_KEY_Q;
    m_key_map[XKB_KEY_r] = KEY_KEY_R;
    m_key_map[XKB_KEY_s] = KEY_KEY_S;
    m_key_map[XKB_KEY_t] = KEY_KEY_T;
    m_key_map[XKB_KEY_u] = KEY_KEY_U;
    m_key_map[XKB_KEY_v] = KEY_KEY_V;
    m_key_map[XKB_KEY_w] = KEY_KEY_W;
    m_key_map[XKB_KEY_x] = KEY_KEY_X;
    m_key_map[XKB_KEY_y] = KEY_KEY_Y;
    m_key_map[XKB_KEY_z] = KEY_KEY_Z;
    m_key_map[XKB_KEY_ssharp] = KEY_OEM_4;
    m_key_map[XKB_KEY_adiaeresis] = KEY_OEM_7;
    m_key_map[XKB_KEY_odiaeresis] = KEY_OEM_3;
    m_key_map[XKB_KEY_udiaeresis] = KEY_OEM_1;
    m_key_map[XKB_KEY_Super_L] = KEY_LWIN;
    m_key_map[XKB_KEY_Super_R] = KEY_RWIN;
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
