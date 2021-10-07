//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2020 SuperTuxKart-Team
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

#ifndef HEADER_SDL_CONTROLLER_HPP
#define HEADER_SDL_CONTROLLER_HPP

#ifndef SERVER_ONLY

#include <SDL_events.h>
#include <SDL_gamecontroller.h>
#include <SDL_joystick.h>
#include <IEventReceiver.h>
#include <bitset>
#include "utils/types.hpp"

#include <SDL_version.h>
#include <SDL_haptic.h>

class GamePadDevice;

class SDLController
{
private:
    SDL_GameController* m_game_controller;

    SDL_Joystick* m_joystick;

    GamePadDevice* m_gamepad;

    SDL_Haptic* m_haptic;
    int m_auto_center;

    int m_buttons;

    int m_axes;

    int m_hats;

    SDL_JoystickID m_id;

    irr::SEvent m_irr_event;

    int16_t m_prev_axes[irr::SEvent::SJoystickEvent::NUMBER_OF_AXES];

    uint64_t m_last_power_level_time;
#ifdef ANDROID
    void handleDirectScanCode(const SDL_Event& event);
#endif

    void updateAutoCenter(int state);
public:
    // ------------------------------------------------------------------------
    SDLController(int device_id);
    // ------------------------------------------------------------------------
    ~SDLController();
    // ------------------------------------------------------------------------
    const irr::SEvent& getEvent() const                 { return m_irr_event; }
    // ------------------------------------------------------------------------
    SDL_JoystickID getInstanceID() const                       { return m_id; }
    // ------------------------------------------------------------------------
    void handleAxisInputSense(const SDL_Event& event);
    // ------------------------------------------------------------------------
    bool handleAxis(const SDL_Event& event)
    {
        int axis_idx = event.jaxis.axis;
        if (axis_idx > m_axes)
            return false;
        m_irr_event.JoystickEvent.Axis[axis_idx] = event.jaxis.value;
        m_prev_axes[axis_idx] = event.jaxis.value;
        uint32_t value = 1 << axis_idx;
        m_irr_event.JoystickEvent.AxisChanged = value;
        return true;
    }   // handleAxis
    // ------------------------------------------------------------------------
    bool handleHat(const SDL_Event& event)
    {
        if (event.jhat.hat > m_hats)
            return false;
        std::bitset<4> new_hat_status;
        // Up, right, down and left (4 buttons)
        switch (event.jhat.value)
        {
        case SDL_HAT_UP:
            new_hat_status[0] = true;
            break;
        case SDL_HAT_RIGHTUP:
            new_hat_status[0] = true;
            new_hat_status[1] = true;
            break;
        case SDL_HAT_RIGHT:
            new_hat_status[1] = true;
            break;
        case SDL_HAT_RIGHTDOWN:
            new_hat_status[1] = true;
            new_hat_status[2] = true;
            break;
        case SDL_HAT_DOWN:
            new_hat_status[2] = true;
            break;
        case SDL_HAT_LEFTDOWN:
            new_hat_status[2] = true;
            new_hat_status[3] = true;
            break;
        case SDL_HAT_LEFT:
            new_hat_status[3] = true;
            break;
        case SDL_HAT_LEFTUP:
            new_hat_status[3] = true;
            new_hat_status[0] = true;
            break;
        case SDL_HAT_CENTERED:
        default:
            break;
        }
        int hat_start = m_buttons - (m_hats * 4) + (event.jhat.hat * 4);
        std::bitset<irr::SEvent::SJoystickEvent::NUMBER_OF_BUTTONS> states
            (m_irr_event.JoystickEvent.ButtonStates);
        for (unsigned i = 0; i < 4; i++)
        {
            int hat_button_id = i + hat_start;
            states[hat_button_id] = new_hat_status[i];
        }
        m_irr_event.JoystickEvent.ButtonStates = (irr::u32)states.to_ulong();
        m_irr_event.JoystickEvent.AxisChanged = 0;
        return true;
    }   // handleHat
    // ------------------------------------------------------------------------
    bool handleButton(const SDL_Event& event)
    {
        if (event.jbutton.button > m_buttons)
        {
#ifdef ANDROID
            handleDirectScanCode(event);
#endif
            return false;
        }
        bool pressed = event.jbutton.state == SDL_PRESSED;
        std::bitset<irr::SEvent::SJoystickEvent::NUMBER_OF_BUTTONS> states
            (m_irr_event.JoystickEvent.ButtonStates);
        states[event.jbutton.button] = pressed;
        m_irr_event.JoystickEvent.ButtonStates = (irr::u32)states.to_ulong();
        m_irr_event.JoystickEvent.AxisChanged = 0;
        return true;
    }   // handleButton
    // ------------------------------------------------------------------------
    SDL_GameController* getGameController() const { return m_game_controller; }
    // ------------------------------------------------------------------------
    void checkPowerLevel();
    // ------------------------------------------------------------------------
    void doRumble(float strength_low, float strength_high, uint32_t duration_ms);
    GamePadDevice* getGamePadDevice() const { return m_gamepad; }
};

#endif

#endif
