//  $Id: input.hpp 694 2006-08-29 07:42:36Z hiker $
//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2007-2008 Robert Schuster <robertschuster@fsfe.org>
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

/** \defgroup input */

#include <string>
#include <irrString.h>

const int DEADZONE_MOUSE       =  150;
const int DEADZONE_MOUSE_SENSE =  200;
const int DEADZONE_JOYSTICK    = 2000;
const int MULTIPLIER_MOUSE     =  750;

/**
  * \ingroup input
  */
struct Input
{
    static const int MAX_VALUE = 32768;
    
    enum AxisDirection
    {
        AD_NEGATIVE,
        AD_POSITIVE,
        AD_NEUTRAL
    };

    enum InputType
    {
        IT_NONE = 0,
        IT_KEYBOARD,
        IT_STICKMOTION,
        IT_STICKBUTTON,
        IT_STICKHAT,
        IT_MOUSEMOTION,
        IT_MOUSEBUTTON
    };
    static const int IT_LAST = IT_MOUSEBUTTON;

    InputType type;
    int deviceID;
    int btnID; // or axis ID for gamepads axes
    int axisDirection;

    Input()
        : type(IT_NONE), deviceID(0), btnID(0), axisDirection(0)
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
        : type(ntype), deviceID(deviceID), btnID(btnID), axisDirection(axisDirection)
    {
        // Nothing to do.
    }
    
    static irr::core::stringw getInputAsString(const Input::InputType type, const int id, const Input::AxisDirection dir=AD_NEUTRAL);

};

/**
  * \brief types of input events / what actions the players can do
  * \ingroup input
  */
enum PlayerAction
{
    PA_FIRST = -1,
    
    PA_LEFT = 0,
    PA_RIGHT,
    PA_ACCEL,
    PA_BRAKE,
    PA_NITRO,
    PA_DRIFT,
    PA_RESCUE,
    PA_FIRE,
    PA_LOOK_BACK,
    
    PA_COUNT
};

/**
  * \brief  human-readable strings for each PlayerAction
  * \ingroup input
  */
static std::string KartActionStrings[PA_COUNT] = {std::string("left"), 
                                                  std::string("right"),
                                                  std::string("accel"),
                                                  std::string("brake"),
                                                  std::string("nitro"),
                                                  std::string("drift"),
                                                  std::string("rescue"),
                                                  std::string("fire"),
                                                  std::string("lookBack")};

#endif
