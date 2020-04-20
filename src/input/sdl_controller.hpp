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

#include <SDL.h>
#include <IEventReceiver.h>
#include "utils/types.hpp"

class GamePadDevice;

class SDLController
{
private:
    SDL_GameController* m_game_controller;

    SDL_Joystick* m_joystick;

    GamePadDevice* m_gamepad;

    int m_buttons;

    int m_axes;

    int m_hats;

    SDL_JoystickID m_id;

    irr::SEvent m_irr_event;
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
    bool handleAxis(const SDL_Event& event)
    {
        if (event.jaxis.axis > m_axes)
            return false;
        m_irr_event.JoystickEvent.Axis[event.jaxis.axis] = event.jaxis.value;
        uint32_t value = 1 << event.jaxis.axis;
        m_irr_event.JoystickEvent.AxisChanged = value;
        return true;
    }   // handleAxis
    // ------------------------------------------------------------------------
    bool handleHat(const SDL_Event& event)
    {
        if (event.jhat.hat > m_hats)
            return false;
        uint32_t value = 0;
        // Up, right, down and left (4 buttons)
        switch (event.jhat.value)
        {
        case SDL_HAT_UP:
            value = 1;
            break;
        case SDL_HAT_RIGHTUP:
            value = 1 | (1 << 1);
            break;
        case SDL_HAT_RIGHT:
            value = 1 << 1;
            break;
        case SDL_HAT_RIGHTDOWN:
            value = (1 << 1) | (1 << 2);
            break;
        case SDL_HAT_DOWN:
            value = 1 << 2;
            break;
        case SDL_HAT_LEFTDOWN:
            value = (1 << 2) | (1 << 3);
            break;
        case SDL_HAT_LEFT:
            value = 1 << 3;
            break;
        case SDL_HAT_LEFTUP:
            value = (1 << 3) | 1;
            break;
        case SDL_HAT_CENTERED:
        default:
            value = 0;
            break;
        }
        int hat_start = m_buttons - (m_hats * 4);
        unsigned hat_mask = (unsigned)((1 << hat_start) - 1);
        m_irr_event.JoystickEvent.ButtonStates &= hat_mask;
        value <<= hat_start;
        value <<= (m_hats - 1) * 4;
        m_irr_event.JoystickEvent.ButtonStates |= value;
        m_irr_event.JoystickEvent.AxisChanged = 0;
        return true;
    }   // handleHat
    // ------------------------------------------------------------------------
    bool handleButton(const SDL_Event& event)
    {
        if (event.jbutton.button > m_buttons)
            return false;
        bool pressed = event.jbutton.state == SDL_PRESSED;
        uint32_t value = 1 << event.jbutton.button;
        if (pressed)
            m_irr_event.JoystickEvent.ButtonStates |= value;
        else
            m_irr_event.JoystickEvent.ButtonStates &= (uint32_t)~value;
        m_irr_event.JoystickEvent.AxisChanged = 0;
        return true;
    }   // handleButton
};

#endif

#endif
