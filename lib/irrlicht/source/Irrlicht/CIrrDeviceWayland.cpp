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
#include <unistd.h>
#include <time.h>

#ifdef __FreeBSD__
#include <dev/evdev/input.h>
#else
#include <linux/input.h>
#endif

#if defined _IRR_COMPILE_WITH_JOYSTICK_EVENTS_
#include <fcntl.h>
#include <sys/ioctl.h>
#ifdef __FreeBSD__
#include <sys/joystick.h>
#else
#include <linux/joystick.h>
#endif
#endif

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
        IVideoDriver* createOGLES2Driver(const SIrrlichtCreationParameters& params,
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
    static const wl_touch_listener touch_listener;
    static const wl_output_listener output_listener;
    static const wl_shell_surface_listener shell_surface_listener;
    static const wl_registry_listener registry_listener;
    static const xdg_wm_base_listener wm_base_listener;
    static const xdg_surface_listener surface_listener;
    static const xdg_toplevel_listener toplevel_listener;

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
        CIrrDeviceWayland* device = static_cast<CIrrDeviceWayland*>(data);

        if (!device->m_decoration && !device->CreationParams.Fullscreen &&
            state == WL_POINTER_BUTTON_STATE_PRESSED &&
            device->m_xkb_alt_pressed)
        {
            if (device->m_xdg_toplevel)
            {
                xdg_toplevel_move(device->m_xdg_toplevel, device->m_seat, serial);
            }
            else if (device->m_shell_surface)
            {
                wl_shell_surface_move(device->m_shell_surface, device->m_seat, serial);
            }
            
            return;
        }
        
        SEvent irrevent;
        irrevent.EventType = irr::EET_MOUSE_INPUT_EVENT;
        irrevent.MouseInput.X = device->getCursorControl()->getPosition().X;
        irrevent.MouseInput.Y = device->getCursorControl()->getPosition().Y;
        irrevent.MouseInput.Control = device->m_xkb_ctrl_pressed;
        irrevent.MouseInput.Shift = device->m_xkb_shift_pressed;
        irrevent.MouseInput.Event = irr::EMIE_COUNT;

        switch (button)
        {
        case BTN_LEFT:
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
        case BTN_RIGHT:
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
        case BTN_MIDDLE:
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
            break;
        default:
            break;
        }

        if (irrevent.MouseInput.Event == irr::EMIE_COUNT)
            return;

        irrevent.MouseInput.ButtonStates = device->m_mouse_button_states;

        device->signalEvent(irrevent);

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
            else if (clicks == 3)
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
        CIrrDeviceWayland* device = static_cast<CIrrDeviceWayland*>(data);

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
            return;

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
            irrevent.KeyInput.Key = (EKEY_CODE)(IRR_KEY_CODES_COUNT + key);
        }

        device->signalEvent(irrevent);

        bool repeats = xkb_keymap_key_repeats(device->m_xkb_keymap, key + 8);

        if (repeats && state == WL_KEYBOARD_KEY_STATE_PRESSED)
        {
            device->m_repeat_enabled = true;
            device->m_repeat_time = os::Timer::getRealTime();
            device->m_repeat_event = irrevent;
        }
        else
        {
            device->m_repeat_enabled = false;
        }
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
        CIrrDeviceWayland* device = static_cast<CIrrDeviceWayland*>(data);

        device->m_repeat_rate = rate == 0 ? 0 : 1000 / rate;
        device->m_repeat_delay = delay;
    }
    
    static void touch_handle_down(void* data, wl_touch* touch, uint32_t serial,
                                  uint32_t time, wl_surface *surface,
                                  int32_t id, wl_fixed_t x, wl_fixed_t y)
    {
        CIrrDeviceWayland* device = static_cast<CIrrDeviceWayland*>(data);

        SEvent event;
        event.EventType = EET_TOUCH_INPUT_EVENT;
        event.TouchInput.Event = ETIE_PRESSED_DOWN;
        event.TouchInput.ID = id;
        event.TouchInput.X = wl_fixed_to_int(x);
        event.TouchInput.Y = wl_fixed_to_int(y);
        
        device->signalEvent(event);
             
        if (device->m_touches_count == 0)
        {
            pointer_motion(data, NULL, 0, x, y);
            pointer_button(data, NULL, 0, 0, BTN_LEFT, 
                           WL_POINTER_BUTTON_STATE_PRESSED);
        }

        device->m_touches_count++;
    }
    
    static void touch_handle_up(void* data, wl_touch* touch, uint32_t serial,
                                uint32_t time, int32_t id)
    {
        CIrrDeviceWayland* device = static_cast<CIrrDeviceWayland*>(data);
        
        SEvent event;
        event.EventType = EET_TOUCH_INPUT_EVENT;
        event.TouchInput.Event = ETIE_LEFT_UP;
        event.TouchInput.ID = id;
        event.TouchInput.X = 0;
        event.TouchInput.Y = 0;
        
        device->signalEvent(event);
        
        if (device->m_touches_count == 1)
        {
            pointer_button(data, NULL, 0, 0, BTN_LEFT, 
                           WL_POINTER_BUTTON_STATE_RELEASED);
        }

        device->m_touches_count--;
    }
    
    static void touch_handle_motion(void* data, wl_touch* touch, uint32_t time,
                                  int32_t id, wl_fixed_t x, wl_fixed_t y)
    {
        CIrrDeviceWayland* device = static_cast<CIrrDeviceWayland*>(data);
        
        SEvent event;
        event.EventType = EET_TOUCH_INPUT_EVENT;
        event.TouchInput.Event = ETIE_MOVED;
        event.TouchInput.ID = id;
        event.TouchInput.X = wl_fixed_to_int(x);
        event.TouchInput.Y = wl_fixed_to_int(y);
        
        device->signalEvent(event);
        
        if (device->m_touches_count == 1)
        {
            pointer_motion(data, NULL, 0, x, y);
        }
    }
    
    static void touch_handle_frame(void* data, wl_touch* touch)
    {
    }
    
    static void touch_handle_cancel(void* data, wl_touch* touch)
    {
        CIrrDeviceWayland* device = static_cast<CIrrDeviceWayland*>(data);
        
        device->m_touches_count = 0;
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
            device->m_has_hardware_keyboard = true;
            device->m_keyboard = wl_seat_get_keyboard(seat);
            wl_keyboard_add_listener(device->m_keyboard, &keyboard_listener,
                                     device);
        }
        else if (!(caps & WL_SEAT_CAPABILITY_KEYBOARD) && device->m_keyboard)
        {
            wl_keyboard_destroy(device->m_keyboard);
            device->m_keyboard = NULL;
        }
        
        if ((caps & WL_SEAT_CAPABILITY_TOUCH) && !device->m_touch)
        {
            device->m_has_touch_device = true;
            device->m_touch = wl_seat_get_touch(seat);
            wl_touch_add_listener(device->m_touch, &touch_listener,
                                  device);
        }
        else if (!(caps & WL_SEAT_CAPABILITY_TOUCH) && device->m_touch)
        {
            wl_touch_destroy(device->m_touch);
            device->m_touch = NULL;
        }
    }

    static void seat_name(void* data, wl_seat* wl_seat, const char* name)
    {
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
    
    static void xdg_wm_base_ping(void* data, xdg_wm_base* shell, 
                                 uint32_t serial)
    {
        xdg_wm_base_pong(shell, serial);
    }
    
    static void xdg_surface_configure(void* data, xdg_surface* surface,
                                      uint32_t serial)
    {
        CIrrDeviceWayland* device = static_cast<CIrrDeviceWayland*>(data);
        
        xdg_surface_ack_configure(surface, serial);
        
        device->m_surface_configured = true;
    }
    
    static void xdg_toplevel_configure(void* data, xdg_toplevel* toplevel,
                                       int32_t width, int32_t height,
                                       wl_array* states)
    {
        //void* state_p;
        
        //wl_array_for_each(state_p, states) 
        //{
        //    uint32_t state = *(uint32_t*)state_p;
        //    
        //    switch (state) 
        //    {
        //    case ZXDG_TOPLEVEL_V6_STATE_FULLSCREEN:
        //    case ZXDG_TOPLEVEL_V6_STATE_MAXIMIZED:
        //    case ZXDG_TOPLEVEL_V6_STATE_ACTIVATED:
        //    case ZXDG_TOPLEVEL_V6_STATE_RESIZING:
        //        break;
        //    default:
        //        break;
        //    }
        //}
    }
    
    static void xdg_toplevel_close(void* data, xdg_toplevel* xdg_toplevel)
    {
        CIrrDeviceWayland* device = static_cast<CIrrDeviceWayland*>(data);
        
        device->closeDevice();
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
            device->m_has_wl_shell = true;
            device->m_wl_shell_name = name;
        }
        else if (interface_str == "wl_seat")
        {
            device->m_seat = static_cast<wl_seat*>(wl_registry_bind(registry,
                                                   name, &wl_seat_interface,
                                                   version < 4 ? version : 4));
                                                   
            wl_seat_add_listener(device->m_seat, &seat_listener, device);
        }
        else if (interface_str == "wl_shm")
        {
            device->m_shm = static_cast<wl_shm*>(wl_registry_bind(registry, 
                                                   name, &wl_shm_interface, 1));
        }
        else if (interface_str == "wl_output")
        {
            device->m_output = static_cast<wl_output*>(wl_registry_bind(
                                           registry, name, &wl_output_interface,
                                           version < 2 ? version : 2));
                                           
            wl_output_add_listener(device->m_output, &output_listener, device);
        }
        else if (interface_str == "zxdg_decoration_manager_v1")
        {
            device->m_decoration_manager = 
                                    static_cast<zxdg_decoration_manager_v1*>(
                                    wl_registry_bind(registry, name, 
                                    &zxdg_decoration_manager_v1_interface, 1));
        }
        else if (interface_str == "xdg_wm_base")
        {
            device->m_has_xdg_wm_base = true;
            device->m_xdg_wm_base_name = name;
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

const wl_touch_listener WaylandCallbacks::touch_listener =
{
    WaylandCallbacks::touch_handle_down,
    WaylandCallbacks::touch_handle_up,
    WaylandCallbacks::touch_handle_motion,
    WaylandCallbacks::touch_handle_frame,
    WaylandCallbacks::touch_handle_cancel
};

const wl_seat_listener WaylandCallbacks::seat_listener =
{
    WaylandCallbacks::seat_capabilities,
    WaylandCallbacks::seat_name
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

const xdg_wm_base_listener WaylandCallbacks::wm_base_listener = 
{
    WaylandCallbacks::xdg_wm_base_ping
};

const xdg_surface_listener WaylandCallbacks::surface_listener = 
{
    WaylandCallbacks::xdg_surface_configure
};

const xdg_toplevel_listener WaylandCallbacks::toplevel_listener = 
{
    WaylandCallbacks::xdg_toplevel_configure,
    WaylandCallbacks::xdg_toplevel_close
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
    m_compositor = NULL;
    m_cursor = NULL;
    m_cursor_theme = NULL;
    m_display = NULL;
    m_egl_window = NULL;
    m_keyboard = NULL;
    m_touch = NULL;
    m_output = NULL;
    m_pointer = NULL;
    m_registry = NULL;
    m_seat = NULL;
    m_shm = NULL;
    m_cursor_surface = NULL;
    m_surface = NULL;
    m_enter_serial = 0;

    m_shell = NULL;
    m_shell_surface = NULL;
    m_has_wl_shell = false;
    m_wl_shell_name = 0;
    
    m_xdg_wm_base = NULL;
    m_xdg_surface = NULL;
    m_xdg_toplevel = NULL;
    m_has_xdg_wm_base = false;
    m_surface_configured = false;
    m_xdg_wm_base_name = 0;
    
    m_decoration_manager = NULL;
    m_decoration = NULL;

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

    m_repeat_enabled = false;
    m_repeat_time = 0;
    m_repeat_rate = 40;
    m_repeat_delay = 400;

    m_egl_context = NULL;

    m_mouse_button_states = 0;
    m_width = params.WindowSize.Width;
    m_height = params.WindowSize.Height;
    m_touches_count = 0;
    m_has_touch_device = false;
    m_has_hardware_keyboard = false;
    m_window_has_focus = false;
    m_window_minimized = false;
    
    #ifdef _DEBUG
    setDebugName("CIrrDeviceWayland");
    #endif

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
    
    bool success = initWayland();
    
    if (!success)
        return;

    createDriver();

    if (VideoDriver)
    {
        createGUIAndScene();
    }
}

CIrrDeviceWayland::~CIrrDeviceWayland()
{
    delete m_egl_context;
    
    if (m_decoration)
        zxdg_toplevel_decoration_v1_destroy(m_decoration);
        
    if (m_decoration_manager)
        zxdg_decoration_manager_v1_destroy(m_decoration_manager);
    
    if (m_keyboard)
        wl_keyboard_destroy(m_keyboard);

    if (m_pointer)
        wl_pointer_destroy(m_pointer);

    if (m_cursor_surface)
        wl_surface_destroy(m_cursor_surface);

    if (m_cursor_theme)
        wl_cursor_theme_destroy(m_cursor_theme);
        
    if (m_xdg_toplevel)
        xdg_toplevel_destroy(m_xdg_toplevel);

    if (m_xdg_surface)
        xdg_surface_destroy(m_xdg_surface);
        
    if (m_xdg_wm_base)
        xdg_wm_base_destroy(m_xdg_wm_base);
        
    if (m_shell_surface)
        wl_shell_surface_destroy(m_shell_surface);
        
    if (m_shell)
        wl_shell_destroy(m_shell);

    if (m_egl_window)
        wl_egl_window_destroy(m_egl_window);
    
    if (m_surface)
        wl_surface_destroy(m_surface);
        
    if (m_shm)
        wl_shm_destroy(m_shm);
        
    if (m_compositor)
        wl_compositor_destroy(m_compositor);

    if (m_output)
        wl_output_destroy(m_output);

    if (m_seat)
        wl_seat_destroy(m_seat);

    if (m_registry)
        wl_registry_destroy(m_registry);

    if (m_xkb_state)
        xkb_state_unref(m_xkb_state);
        
    if (m_xkb_keymap)
        xkb_keymap_unref(m_xkb_keymap);
        
    if (m_xkb_compose_state)
        xkb_compose_state_unref(m_xkb_compose_state);
        
    if (m_xkb_compose_table)
        xkb_compose_table_unref(m_xkb_compose_table);

    if (m_xkb_context)
        xkb_context_unref(m_xkb_context);

    if (m_display)
    {
        wl_display_flush(m_display);
        wl_display_disconnect(m_display);
    }

    closeJoysticks();
}

bool CIrrDeviceWayland::initWayland()
{
    m_display = wl_display_connect(NULL);
    
    if (m_display == NULL)
    {
        os::Printer::log("Couldn't open display.", ELL_ERROR);
        return false;
    }
    
    m_xkb_context = xkb_context_new(XKB_CONTEXT_NO_FLAGS);
    
    if (m_xkb_context == NULL)
    {
        os::Printer::log("Couldn't create xkb context.", ELL_ERROR);
        return false;
    }
    
    m_registry = wl_display_get_registry(m_display);
    wl_registry_add_listener(m_registry, &WaylandCallbacks::registry_listener, 
                             this);
    
    wl_display_dispatch(m_display);
    wl_display_roundtrip(m_display);
    
    if (m_compositor == NULL || m_seat == NULL || m_output == NULL)
    {
        os::Printer::log("Important protocols are not available.", ELL_ERROR);
        return false;
    }
    
    if (!m_has_wl_shell && !m_has_xdg_wm_base)
    {
        os::Printer::log("Shell protocol is not available.", ELL_ERROR);
        return false;
    }

    if (CreationParams.DriverType != video::EDT_NULL)
    {
        if (m_has_xdg_wm_base)
        {
            m_xdg_wm_base = static_cast<xdg_wm_base*>(wl_registry_bind(
                    m_registry, m_xdg_wm_base_name, &xdg_wm_base_interface, 1));
                                                 
            xdg_wm_base_add_listener(m_xdg_wm_base, 
                                     &WaylandCallbacks::wm_base_listener, this);
        }
        else if (m_has_wl_shell)
        {
            m_shell = static_cast<wl_shell*>(wl_registry_bind(m_registry, 
                                      m_wl_shell_name, &wl_shell_interface, 1));
        }
        
        bool success = createWindow();
        
        if (!success)
        {
            os::Printer::log("Couldn't create window.", ELL_ERROR);
            return false;
        }
    }
    
    return true;
}

bool CIrrDeviceWayland::initEGL()
{
    m_egl_window = wl_egl_window_create(m_surface, m_width, m_height);

    m_egl_context = new ContextManagerEGL();

    ContextEGLParams egl_params;

    if (CreationParams.DriverType == video::EDT_OGLES2)
    {
        egl_params.opengl_api = CEGL_API_OPENGL_ES;
    }
    else
    {
        egl_params.opengl_api = CEGL_API_OPENGL;
    }

    egl_params.surface_type = CEGL_SURFACE_WINDOW;
    egl_params.force_legacy_device = CreationParams.ForceLegacyDevice;
    egl_params.handle_srgb = CreationParams.HandleSRGB;
    egl_params.with_alpha_channel = CreationParams.WithAlphaChannel;
    egl_params.swap_interval = CreationParams.SwapInterval;
    egl_params.platform = CEGL_PLATFORM_WAYLAND;
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
    
    bool success = initEGL();

    if (!success)
    {
        os::Printer::log("Couldn't create OpenGL context.", ELL_ERROR);
        return false;
    }

    if (m_xdg_wm_base != NULL)
    {
        m_xdg_surface = xdg_wm_base_get_xdg_surface(m_xdg_wm_base, m_surface);
        
        xdg_surface_add_listener(m_xdg_surface, 
                                 &WaylandCallbacks::surface_listener, this);
                                     
        m_xdg_toplevel = xdg_surface_get_toplevel(m_xdg_surface);

        xdg_toplevel_add_listener(m_xdg_toplevel,
                                  &WaylandCallbacks::toplevel_listener, this);

        wl_surface_commit(m_surface);
                                    
        if (CreationParams.Fullscreen)
        {
            xdg_toplevel_set_fullscreen(m_xdg_toplevel, NULL);
        }
        
        xdg_surface_set_window_geometry(m_xdg_surface, 0, 0, m_width, m_height);
                                    
        while (!m_surface_configured)
        {
            wl_display_dispatch(m_display);
            usleep(1000);
        }
        
        if (m_decoration_manager != NULL)
        {
            m_decoration = zxdg_decoration_manager_v1_get_toplevel_decoration(
                                        m_decoration_manager, m_xdg_toplevel);
        }
                                                       
        if (m_decoration != NULL)
        {
            zxdg_toplevel_decoration_v1_set_mode(m_decoration, 
                                ZXDG_TOPLEVEL_DECORATION_V1_MODE_SERVER_SIDE);
        }
    }
    else if (m_shell != NULL)
    {
        m_shell_surface = wl_shell_get_shell_surface(m_shell, m_surface);

        wl_shell_surface_add_listener(m_shell_surface,
                                      &WaylandCallbacks::shell_surface_listener, 
                                      this);

        if (CreationParams.Fullscreen)
        {
            wl_shell_surface_set_fullscreen(m_shell_surface,
                       WL_SHELL_SURFACE_FULLSCREEN_METHOD_DEFAULT, 0, m_output);
        }
        else
        {
            wl_shell_surface_set_toplevel(m_shell_surface);
        }
    }
    else
    {
        os::Printer::log("Cannot create shell surface.", ELL_ERROR);
        return false;
    }

    wl_region* region = wl_compositor_create_region(m_compositor);
    wl_region_add(region, 0, 0, m_width, m_height);
    wl_surface_set_opaque_region(m_surface, region);
    wl_region_destroy(region);

    if (m_shm)
    {
        m_cursor_surface = wl_compositor_create_surface(m_compositor);
        m_cursor_theme = wl_cursor_theme_load(NULL, 32, m_shm);
    }

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

void CIrrDeviceWayland::createDriver()
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
        #ifdef _IRR_COMPILE_WITH_OGLES2_
        VideoDriver = video::createOGLES2Driver(CreationParams, FileSystem, this);
        #else
        os::Printer::log("No OpenGL ES 2.0 support compiled in.", ELL_ERROR);
        #endif
        break;
    case video::EDT_NULL:
        VideoDriver = video::createNullDriver(FileSystem, CreationParams.WindowSize);
        break;
    default:
        os::Printer::log("Wayland driver only supports OpenGL.", ELL_ERROR);
        break;
    }
}

void CIrrDeviceWayland::updateCursor()
{
    if (!m_pointer)
        return;

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

    if (wl_display_dispatch_pending(m_display) == -1)
    {
        closeDevice();
    }

    for (unsigned int i = 0; i < m_events.size(); i++)
    {
        postEventFromUser(m_events[i]);
    }

    m_events.clear();

    if (m_repeat_enabled && m_repeat_rate > 0)
    {
        uint32_t curr_time = os::Timer::getRealTime();

        while (curr_time - m_repeat_time > m_repeat_delay + m_repeat_rate)
        {
            postEventFromUser(m_repeat_event);
            m_repeat_time += m_repeat_rate;
        }
    }

    if (!Close)
    {
        pollJoysticks();
    }

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
void CIrrDeviceWayland::sleep(u32 timeMs, bool pauseTimer = false)
{
    const bool wasStopped = Timer ? Timer->isStopped() : true;

    struct timespec ts;
    ts.tv_sec = (time_t) (timeMs / 1000);
    ts.tv_nsec = (long) (timeMs % 1000)*  1000000;

    if (pauseTimer && !wasStopped)
    {
        Timer->stop();
    }

    nanosleep(&ts, NULL);

    if (pauseTimer && !wasStopped)
    {
        Timer->start();
    }
}

//! sets the caption of the window
void CIrrDeviceWayland::setWindowCaption(const wchar_t* text)
{
    char title[1024];
    wcstombs(title, text, sizeof(title));
    title[1023] = '\0';

    if (m_xdg_toplevel)
    {
        xdg_toplevel_set_title(m_xdg_toplevel, title);
    }
    else if (m_shell_surface)
    {
        wl_shell_surface_set_title(m_shell_surface, title);
    }
}

//! sets the class of the window
void CIrrDeviceWayland::setWindowClass(const char* text)
{
    if (m_xdg_toplevel)
    {
        xdg_toplevel_set_app_id(m_xdg_toplevel, text);
    }
    else if (m_shell_surface)
    {
        wl_shell_surface_set_class(m_shell_surface, text);
    }
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
    if (m_xdg_toplevel)
    {
        int width = resize ? 0 : m_width;
        int height = resize ? 0 : m_height;
        
        xdg_toplevel_set_min_size(m_xdg_toplevel, width, height);
        xdg_toplevel_set_max_size(m_xdg_toplevel, width, height);
    }
}

//! Return pointer to a list with all video modes supported by the gfx adapter.
video::IVideoModeList* CIrrDeviceWayland::getVideoModeList()
{
    return &VideoModeList;
}

//! Minimize window
void CIrrDeviceWayland::minimizeWindow()
{
    if (m_xdg_toplevel)
    {
        xdg_toplevel_set_minimized(m_xdg_toplevel);
    }
}

//! Maximize window
void CIrrDeviceWayland::maximizeWindow()
{
    if (m_xdg_toplevel)
    {
        xdg_toplevel_set_maximized(m_xdg_toplevel);
    }
}

//! Restore original window size
void CIrrDeviceWayland::restoreWindow()
{
    if (m_xdg_toplevel)
    {
        xdg_toplevel_unset_maximized(m_xdg_toplevel);
    }
}

//! Move window to requested position
bool CIrrDeviceWayland::moveWindow(int x, int y)
{
    return false;
}

//! Get current window position.
bool CIrrDeviceWayland::getWindowPosition(int* x, int* y)
{
    return false;
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
    m_key_map[KEY_RESERVED] = IRR_KEY_UNKNOWN;
    m_key_map[KEY_ESC] = IRR_KEY_ESCAPE;
    m_key_map[KEY_1] = IRR_KEY_1;
    m_key_map[KEY_2] = IRR_KEY_2;
    m_key_map[KEY_3] = IRR_KEY_3;
    m_key_map[KEY_4] = IRR_KEY_4;
    m_key_map[KEY_5] = IRR_KEY_5;
    m_key_map[KEY_6] = IRR_KEY_6;
    m_key_map[KEY_7] = IRR_KEY_7;
    m_key_map[KEY_8] = IRR_KEY_8;
    m_key_map[KEY_9] = IRR_KEY_9;
    m_key_map[KEY_0] = IRR_KEY_0;
    m_key_map[KEY_MINUS] = IRR_KEY_MINUS;
    m_key_map[KEY_EQUAL] = IRR_KEY_PLUS;
    m_key_map[KEY_BACKSPACE] = IRR_KEY_BACK;
    m_key_map[KEY_TAB] = IRR_KEY_TAB;
    m_key_map[KEY_Q] = IRR_KEY_Q;
    m_key_map[KEY_W] = IRR_KEY_W;
    m_key_map[KEY_E] = IRR_KEY_E;
    m_key_map[KEY_R] = IRR_KEY_R;
    m_key_map[KEY_T] = IRR_KEY_T;
    m_key_map[KEY_Y] = IRR_KEY_Y;
    m_key_map[KEY_U] = IRR_KEY_U;
    m_key_map[KEY_I] = IRR_KEY_I;
    m_key_map[KEY_P] = IRR_KEY_P;
    m_key_map[KEY_O] = IRR_KEY_O;
    m_key_map[KEY_LEFTBRACE] = IRR_KEY_OEM_4;
    m_key_map[KEY_RIGHTBRACE] = IRR_KEY_OEM_6;
    m_key_map[KEY_ENTER] = IRR_KEY_RETURN;
    m_key_map[KEY_LEFTCTRL] = IRR_KEY_LCONTROL;
    m_key_map[KEY_A] = IRR_KEY_A;
    m_key_map[KEY_S] = IRR_KEY_S; 
    m_key_map[KEY_D] = IRR_KEY_D;
    m_key_map[KEY_F] = IRR_KEY_F;
    m_key_map[KEY_G] = IRR_KEY_G;
    m_key_map[KEY_H] = IRR_KEY_H;
    m_key_map[KEY_J] = IRR_KEY_J;
    m_key_map[KEY_K] = IRR_KEY_K;
    m_key_map[KEY_L] = IRR_KEY_L;
    m_key_map[KEY_SEMICOLON] = IRR_KEY_OEM_1;
    m_key_map[KEY_APOSTROPHE] = IRR_KEY_OEM_7;
    m_key_map[KEY_GRAVE] = IRR_KEY_OEM_3;
    m_key_map[KEY_LEFTSHIFT] = IRR_KEY_LSHIFT;
    m_key_map[KEY_BACKSLASH] = IRR_KEY_OEM_5;
    m_key_map[KEY_Z] = IRR_KEY_Z;
    m_key_map[KEY_X] = IRR_KEY_X;
    m_key_map[KEY_C] = IRR_KEY_C;
    m_key_map[KEY_V] = IRR_KEY_V;
    m_key_map[KEY_B] = IRR_KEY_B;
    m_key_map[KEY_N] = IRR_KEY_N;
    m_key_map[KEY_M] = IRR_KEY_M;
    m_key_map[KEY_COMMA] = IRR_KEY_COMMA;
    m_key_map[KEY_DOT] = IRR_KEY_PERIOD;
    m_key_map[KEY_SLASH] = IRR_KEY_OEM_2; 
    m_key_map[KEY_RIGHTSHIFT] = IRR_KEY_RSHIFT;
    m_key_map[KEY_KPASTERISK] = IRR_KEY_MULTIPLY;
    m_key_map[KEY_LEFTALT] = IRR_KEY_LMENU;
    m_key_map[KEY_SPACE] = IRR_KEY_SPACE;
    m_key_map[KEY_CAPSLOCK] = IRR_KEY_CAPITAL;
    m_key_map[KEY_F1] = IRR_KEY_F1;
    m_key_map[KEY_F2] = IRR_KEY_F2;
    m_key_map[KEY_F3] = IRR_KEY_F3;
    m_key_map[KEY_F4] = IRR_KEY_F4;
    m_key_map[KEY_F5] = IRR_KEY_F5;
    m_key_map[KEY_F6] = IRR_KEY_F6;
    m_key_map[KEY_F7] = IRR_KEY_F7;
    m_key_map[KEY_F8] = IRR_KEY_F8;
    m_key_map[KEY_F9] = IRR_KEY_F9;
    m_key_map[KEY_F10] = IRR_KEY_F10;
    m_key_map[KEY_NUMLOCK] = IRR_KEY_NUMLOCK;
    m_key_map[KEY_SCROLLLOCK] = IRR_KEY_SCROLL;
    m_key_map[KEY_KP7] = IRR_KEY_NUMPAD7;
    m_key_map[KEY_KP8] = IRR_KEY_NUMPAD8;
    m_key_map[KEY_KP9] = IRR_KEY_NUMPAD9;
    m_key_map[KEY_KPMINUS] = IRR_KEY_SUBTRACT;
    m_key_map[KEY_KP4] = IRR_KEY_NUMPAD4;
    m_key_map[KEY_KP5] = IRR_KEY_NUMPAD5;
    m_key_map[KEY_KP6] = IRR_KEY_NUMPAD6;
    m_key_map[KEY_KPPLUS] = IRR_KEY_ADD;
    m_key_map[KEY_KP1] = IRR_KEY_NUMPAD1;
    m_key_map[KEY_KP2] = IRR_KEY_NUMPAD2;
    m_key_map[KEY_KP3] = IRR_KEY_NUMPAD3;
    m_key_map[KEY_KP0] = IRR_KEY_NUMPAD0;
    m_key_map[KEY_KPDOT] = IRR_KEY_SEPARATOR;
    m_key_map[KEY_ZENKAKUHANKAKU] = IRR_KEY_UNKNOWN;
    m_key_map[KEY_102ND] = IRR_KEY_OEM_102;
    m_key_map[KEY_F11] = IRR_KEY_F11;
    m_key_map[KEY_F12] = IRR_KEY_F12;
    m_key_map[KEY_RO] = IRR_KEY_UNKNOWN;
    m_key_map[KEY_KATAKANA] = IRR_KEY_UNKNOWN;
    m_key_map[KEY_HIRAGANA] = IRR_KEY_UNKNOWN;
    m_key_map[KEY_HENKAN] = IRR_KEY_UNKNOWN;
    m_key_map[KEY_KATAKANAHIRAGANA] = IRR_KEY_UNKNOWN;
    m_key_map[KEY_MUHENKAN] = IRR_KEY_UNKNOWN;
    m_key_map[KEY_KPJPCOMMA] = IRR_KEY_SEPARATOR;
    m_key_map[KEY_KPENTER] = IRR_KEY_RETURN;
    m_key_map[KEY_RIGHTCTRL] = IRR_KEY_RCONTROL;
    m_key_map[KEY_KPSLASH] = IRR_KEY_DIVIDE;
    m_key_map[KEY_SYSRQ] = IRR_KEY_UNKNOWN;
    m_key_map[KEY_RIGHTALT] = IRR_KEY_RMENU;
    m_key_map[KEY_LINEFEED] = IRR_KEY_UNKNOWN;
    m_key_map[KEY_HOME] = IRR_KEY_HOME;
    m_key_map[KEY_UP] = IRR_KEY_UP;
    m_key_map[KEY_PAGEUP] = IRR_KEY_PRIOR;
    m_key_map[KEY_LEFT] = IRR_KEY_LEFT;
    m_key_map[KEY_RIGHT] = IRR_KEY_RIGHT;
    m_key_map[KEY_END] = IRR_KEY_END;
    m_key_map[KEY_DOWN] = IRR_KEY_DOWN;
    m_key_map[KEY_PAGEDOWN] = IRR_KEY_NEXT;
    m_key_map[KEY_INSERT] = IRR_KEY_INSERT;
    m_key_map[KEY_DELETE] = IRR_KEY_DELETE;
    m_key_map[KEY_MACRO] = IRR_KEY_UNKNOWN;
    m_key_map[KEY_MUTE] = IRR_KEY_VOLUME_MUTE;
    m_key_map[KEY_VOLUMEDOWN] = IRR_KEY_VOLUME_DOWN;
    m_key_map[KEY_VOLUMEUP] = IRR_KEY_VOLUME_UP;
    m_key_map[KEY_POWER] = IRR_KEY_UNKNOWN;
    m_key_map[KEY_KPEQUAL] = IRR_KEY_RETURN;
    m_key_map[KEY_KPPLUSMINUS] = IRR_KEY_PLUS;
    m_key_map[KEY_PAUSE] = IRR_KEY_PAUSE;
    m_key_map[KEY_SCALE] = IRR_KEY_UNKNOWN;
    m_key_map[KEY_KPCOMMA] = IRR_KEY_COMMA;
    m_key_map[KEY_HANGEUL] = IRR_KEY_UNKNOWN;
    m_key_map[KEY_HANJA] = IRR_KEY_UNKNOWN;
    m_key_map[KEY_YEN] = IRR_KEY_UNKNOWN;
    m_key_map[KEY_LEFTMETA] = IRR_KEY_LWIN;
    m_key_map[KEY_RIGHTMETA] = IRR_KEY_RWIN;
    m_key_map[KEY_COMPOSE] = IRR_KEY_MENU;
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
