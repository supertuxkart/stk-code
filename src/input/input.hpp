//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2007-2015 Robert Schuster <robertschuster@fsfe.org>
//  Copyright (C) 2012-2015 SuperTuxKart-Team
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

#ifndef HEADER_INPUT_HPP
#define HEADER_INPUT_HPP

/**
  * \defgroup input
  * Contains classes for input management (keyboard and gamepad)
  */

#include <string>
#include <irrString.h>

/**
  * \ingroup input
  */
struct Input
{
    static const int MAX_VALUE = 32768;

    static const int HAT_H_ID = 100;
    static const int HAT_V_ID = 101;

    enum AxisDirection
    {
        AD_NEGATIVE,
        AD_POSITIVE,
        AD_NEUTRAL
    };

    enum AxisRange
    {
        AR_HALF,
        AR_FULL
    };

    enum InputType
    {
        IT_NONE = 0,
        IT_KEYBOARD,
        IT_STICKMOTION,
        IT_STICKBUTTON,
        //IT_STICKHAT,
        IT_MOUSEMOTION,
        IT_MOUSEBUTTON
    };
    static const int IT_LAST = IT_MOUSEBUTTON;

    InputType m_type;
    int       m_device_id;
    int       m_button_id; // or axis ID for gamepads axes
    int       m_axis_direction;
    int       m_axis_range;
    wchar_t   m_character;

    Input()
        : m_type(IT_NONE), m_device_id(0), m_button_id(0),
        m_axis_direction(0), m_axis_range(Input::AR_FULL), m_character(0)
    {
        // Nothing to do.
    }

    /** Creates an Input instance which represents an arbitrary way of getting
     * game input using a type specifier and 3 integers.
     *
     * Meaning of the 3 integers for each InputType:
     * IT_NONE: This means nothing. In certain cases this is regarded as an
     * unset binding.
     * IT_KEYBOARD: id0 is a irrLicht value.
     * IT_STICKMOTION: id0 - stick index, id1 - axis index, id2 - axis direction
     * (negative, positive). You can assume that axis 0 is the X-Axis where the
     * negative direction is to the left and that axis 1 is the Y-Axis with the
     * negative direction being upwards.
     * IT_STICKBUTTON: id0 - stick index, id1 - button index. Button 0 and 1 are
     * usually reached most easily.
     * IT_STICKHAT: This is not yet implemented.
     * IT_MOUSEMOTION: id0 - axis index (0 -> X, 1 -> Y). Mouse wheel is
     * represented as buttons!
     * IT_MOUSEBUTTON: id0 - button number (1 -> left, 2 -> middle, 3 -> right,
     * ...)
     *
     * Note: For joystick bindings that are actice in the menu the joystick's
     * index should be zero. The binding will react to all joysticks connected
     * to the system.
     */
    Input(InputType ntype, int deviceID , int btnID = 0, int axisDirection= 0)
        : m_type(ntype), m_device_id(deviceID), m_button_id(btnID),
          m_axis_direction(axisDirection), m_axis_range(Input::AR_FULL)
    {
        // Nothing to do.
    }

};   // struct Input

/**
  * \brief types of input events / what actions the players can do
  * \ingroup input
  */
enum PlayerAction
{
    PA_BEFORE_FIRST = -1,

    PA_STEER_LEFT = 0,
    PA_STEER_RIGHT,
    PA_ACCEL,
    PA_BRAKE,
    PA_NITRO,
    PA_DRIFT,
    PA_RESCUE,
    PA_FIRE,
    PA_LOOK_BACK,
    PA_PAUSE_RACE,

    PA_MENU_UP,
    PA_MENU_DOWN,
    PA_MENU_LEFT,
    PA_MENU_RIGHT,
    PA_MENU_SELECT,
    PA_MENU_CANCEL,

    PA_COUNT
};

const PlayerAction PA_FIRST_GAME_ACTION = PA_STEER_LEFT;
const PlayerAction PA_LAST_GAME_ACTION = PA_PAUSE_RACE;
const PlayerAction PA_FIRST_MENU_ACTION = PA_MENU_UP;
const PlayerAction PA_LAST_MENU_ACTION = PA_MENU_CANCEL;

/**
  * \brief  human-readable strings for each PlayerAction
  * \ingroup input
  */
static std::string KartActionStrings[PA_COUNT] = {std::string("steerLeft"),
                                                  std::string("steerRight"),
                                                  std::string("accel"),
                                                  std::string("brake"),
                                                  std::string("nitro"),
                                                  std::string("drift"),
                                                  std::string("rescue"),
                                                  std::string("fire"),
                                                  std::string("lookBack"),
                                                  std::string("pauserace"),
                                                  std::string("menuUp"),
                                                  std::string("menuDown"),
                                                  std::string("menuLeft"),
                                                  std::string("menuRight"),
                                                  std::string("menuSelect"),
                                                  std::string("menuCancel")
                                                  };

#endif
