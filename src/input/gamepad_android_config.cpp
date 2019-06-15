//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2010-2015 SuperTuxKart-Team
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


#include "input/gamepad_android_config.hpp"
#include "utils/string_utils.hpp"
#include "utils/translation.hpp"

#include <SKeyMap.h>

using namespace irr;

GamepadAndroidConfig::GamepadAndroidConfig()
{
    setDefaultBinds();
}

// ----------------------------------------------------------------------------
/** Saves the configuration to a file. It writes the name for a gamepad
 *  config, saves the device specific parameters, and calls
 *  DeviceConfig::save() to save the rest.
 *  \param stream The stream to save to.
 */
void GamepadAndroidConfig::save(std::ofstream& stream)
{
    stream << "<gamepad_android ";
    DeviceConfig::save(stream);
    stream << "</gamepad_android>\n\n";
}  // save

// ----------------------------------------------------------------------------

irr::core::stringw GamepadAndroidConfig::getBindingAsString(const PlayerAction action) const
{
    const Binding &b = getBinding(action);
    int id = b.getId();
    
    irr::core::stringw button_name;
    
    switch (id)
    {
    case IRR_KEY_BUTTON_LEFT:
        button_name = _C("input_key", "Left");
        break;
    case IRR_KEY_BUTTON_RIGHT:
        button_name = _C("input_key", "Right");
        break;
    case IRR_KEY_BUTTON_UP:
        button_name = _C("input_key", "Up");
        break;
    case IRR_KEY_BUTTON_DOWN:
        button_name = _C("input_key", "Down");
        break;
    case IRR_KEY_BUTTON_A:
        button_name = "A";
        break;
    case IRR_KEY_BUTTON_B:
        button_name = "B";
        break;
    case IRR_KEY_BUTTON_C:
        button_name = "C";
        break;
    case IRR_KEY_BUTTON_X:
        button_name = "X";
        break;
    case IRR_KEY_BUTTON_Y:
        button_name = "Y";
        break;
    case IRR_KEY_BUTTON_Z:
        button_name = "Z";
        break;
    case IRR_KEY_BUTTON_L1:
        button_name = "L1";
        break;
    case IRR_KEY_BUTTON_R1:
        button_name = "R1";
        break;
    case IRR_KEY_BUTTON_L2:
        button_name = "L2";
        break;
    case IRR_KEY_BUTTON_R2:
        button_name = "R2";
        break;
    case IRR_KEY_BUTTON_THUMBL:
        button_name = _C("input_key", "Thumb Left");
        break;
    case IRR_KEY_BUTTON_THUMBR:
        button_name = _C("input_key", "Thumb Right");
        break;
    case IRR_KEY_BUTTON_START:
        button_name = _C("input_key", "Start");
        break;
    case IRR_KEY_BUTTON_SELECT:
        button_name = _C("input_key", "Select");
        break;
    case IRR_KEY_BUTTON_MODE:
        button_name = _C("input_key", "Mode");
        break;
    default:
        button_name = DeviceConfig::getBindingAsString(action);
        break;
    }
    
    return button_name;
}

// ----------------------------------------------------------------------------

void GamepadAndroidConfig::setDefaultBinds()
{
    setBinding(PA_NITRO,       Input::IT_KEYBOARD, IRR_KEY_BUTTON_X);
    setBinding(PA_ACCEL,       Input::IT_KEYBOARD, IRR_KEY_BUTTON_UP);
    setBinding(PA_BRAKE,       Input::IT_KEYBOARD, IRR_KEY_BUTTON_DOWN);
    setBinding(PA_STEER_LEFT,  Input::IT_KEYBOARD, IRR_KEY_BUTTON_LEFT);
    setBinding(PA_STEER_RIGHT, Input::IT_KEYBOARD, IRR_KEY_BUTTON_RIGHT);
    setBinding(PA_DRIFT,       Input::IT_KEYBOARD, IRR_KEY_BUTTON_Y);
    setBinding(PA_RESCUE,      Input::IT_KEYBOARD, IRR_KEY_BUTTON_L1);
    setBinding(PA_FIRE,        Input::IT_KEYBOARD, IRR_KEY_BUTTON_A);
    setBinding(PA_LOOK_BACK,   Input::IT_KEYBOARD, IRR_KEY_BUTTON_R1);
    setBinding(PA_PAUSE_RACE,  Input::IT_KEYBOARD, IRR_KEY_BUTTON_B);

    setBinding(PA_MENU_UP,     Input::IT_KEYBOARD, IRR_KEY_BUTTON_UP);
    setBinding(PA_MENU_DOWN,   Input::IT_KEYBOARD, IRR_KEY_BUTTON_DOWN);
    setBinding(PA_MENU_LEFT,   Input::IT_KEYBOARD, IRR_KEY_BUTTON_LEFT);
    setBinding(PA_MENU_RIGHT,  Input::IT_KEYBOARD, IRR_KEY_BUTTON_RIGHT);
    setBinding(PA_MENU_SELECT, Input::IT_KEYBOARD, IRR_KEY_BUTTON_A);
    setBinding(PA_MENU_CANCEL, Input::IT_KEYBOARD, IRR_KEY_BUTTON_B);
}

//------------------------------------------------------------------------------

