//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2013-2015 SuperTuxKart-Team
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

#ifdef ENABLE_WIIUSE

#include "input/wiimote.hpp"

#include "config/user_config.hpp"
#include "input/gamepad_device.hpp"
#include "input/device_manager.hpp"
#include "utils/string_utils.hpp"

#include "wiiuse.h"



Wiimote::Wiimote(wiimote_t* wiimote_handle, int wiimote_id,
                  GamepadConfig* gamepad_config)
{
    m_wiimote_handle    = wiimote_handle;
    m_wiimote_id        = wiimote_id;
    resetIrrEvent();

    m_connected = true;

    // Create the corresponding gamepad device
    irr::core::stringc gamepad_name = irr::core::stringc("Wiimote ") +
                                     StringUtils::toString(wiimote_id).c_str();

    gamepad_config->setPlugged();
    // Determine number of bits in the bit mask of all buttons.
    int button_count = (int)( log((float)WIIMOTE_BUTTON_ALL) / log(2.0f))+1;

    m_gamepad_device = new GamePadDevice(getIrrId(),
                                         gamepad_name.c_str(),
                                         /*num axes*/ 1,
                                         button_count,
                                         gamepad_config );
    DeviceManager* device_manager = input_manager->getDeviceManager();
    device_manager->addGamepad(m_gamepad_device);

}   // Wiimote

// ----------------------------------------------------------------------------
Wiimote::~Wiimote()
{
}   // ~Wiimote

// -----------------------------------------------------------------------------
void Wiimote::resetIrrEvent()
{
    irr::SEvent &event = m_irr_event.getData();
    event.EventType = irr::EET_JOYSTICK_INPUT_EVENT;
    for(int i=0 ; i < irr::SEvent::SJoystickEvent::NUMBER_OF_AXES ; i++)
        event.JoystickEvent.Axis[i] = 0;
    event.JoystickEvent.Joystick = getIrrId();
    event.JoystickEvent.POV = 65535;
    event.JoystickEvent.ButtonStates = 0;
}   // resetIrrEvent

// -----------------------------------------------------------------------------
/** Called from the update thread: takes the wiimote state and
 */
void Wiimote::update()
{
    float normalized_angle = -(m_wiimote_handle->accel.y-128)
                           /  UserConfigParams::m_wiimote_raw_max;

    if(normalized_angle<-1.0f)
        normalized_angle = -1.0f;
    else if(normalized_angle>1.0f)
        normalized_angle = 1.0f;

    // Shape the curve that determines steering depending on wiimote angle.
    // The wiimote value is already normalized to be in [-1,1]. Now use a
    // weighted linear combination to compute the steering value used in game.
    float w1 = UserConfigParams::m_wiimote_weight_linear;
    float w2 = UserConfigParams::m_wiimote_weight_square;
    float wa = UserConfigParams::m_wiimote_weight_asin;
    float ws = UserConfigParams::m_wiimote_weight_sin;

    const float sign = normalized_angle >= 0.0f ? 1.0f : -1.0f;
    const float normalized_angle_2 = w1 * normalized_angle
                                   + w2 * sign*normalized_angle*normalized_angle
                                   + wa * asin(normalized_angle)*(2.0f/M_PI)
                                   + ws * sin(normalized_angle*(M_PI/2.0f));

    if(UserConfigParams::m_wiimote_debug)
    {
        Log::verbose("wiimote", "raw %d normal %f result %f",
                     m_wiimote_handle->accel.y,
                     normalized_angle,
                     normalized_angle_2);
    }

    const float JOYSTICK_ABS_MAX_ANGLE = 32766.0f;

    const float angle = normalized_angle_2 * JOYSTICK_ABS_MAX_ANGLE;

    m_irr_event.lock();
    {

        irr::SEvent::SJoystickEvent &ev = m_irr_event.getData().JoystickEvent;
        ev.Axis[SEvent::SJoystickEvent::AXIS_X] =
                 (irr::s16)(irr::core::clamp(angle, -JOYSTICK_ABS_MAX_ANGLE,
                                                    +JOYSTICK_ABS_MAX_ANGLE));
        // --------------------- Wiimote buttons --------------------
        // Copy the wiimote button structure, but mask out the non-button
        // bits (4 bits of the button structure are actually bits for the
        // accelerator).
        ev.ButtonStates = m_wiimote_handle->btns & WIIMOTE_BUTTON_ALL;
    }
    m_irr_event.unlock();

#ifdef DEBUG
    if(UserConfigParams::m_wiimote_debug)
        printDebugInfo();
#endif

}   // update

// ----------------------------------------------------------------------------
/** Prints debug information to Log::verbose about accelerometer and button
 *  states.
 */
void Wiimote::printDebugInfo() const
{
    struct WiimoteAction
    {
        int         wiimote_action_id;
        const char* wiimote_action_name;
    };   // struct WiimoteAction

    static WiimoteAction wiimote_actions[] = {
        {WIIMOTE_BUTTON_LEFT,   "WIIMOTE_BUTTON_LEFT" },
        {WIIMOTE_BUTTON_RIGHT,  "WIIMOTE_BUTTON_RIGHT"},
        {WIIMOTE_BUTTON_UP,     "WIIMOTE_BUTTON_UP"   },
        {WIIMOTE_BUTTON_DOWN,   "WIIMOTE_BUTTON_DOWN" },
        {WIIMOTE_BUTTON_A,      "WIIMOTE_BUTTON_A"    },
        {WIIMOTE_BUTTON_B,      "WIIMOTE_BUTTON_B"    },
        {WIIMOTE_BUTTON_PLUS,   "WIIMOTE_BUTTON_PLUS" },
        {WIIMOTE_BUTTON_MINUS,  "WIIMOTE_BUTTON_MINUS"},
        {WIIMOTE_BUTTON_ONE,    "WIIMOTE_BUTTON_ONE"  },
        {WIIMOTE_BUTTON_TWO,    "WIIMOTE_BUTTON_TWO"  },
        {WIIMOTE_BUTTON_HOME,   "WIIMOTE_BUTTON_HOME" },
    };   // wiimote_actions

    const unsigned int count = sizeof(wiimote_actions)/sizeof(WiimoteAction);
    for(unsigned int i=0 ; i<count; i++)
    {
        if(IS_PRESSED(m_wiimote_handle, wiimote_actions[i].wiimote_action_id))
        {
            Log::verbose("wiimote", "%d: pressed button %s -> button id: %d",
                m_wiimote_id, wiimote_actions[i].wiimote_action_name,
                i);
        }
    }   // for i < count
}   // printDebugInfo

// ----------------------------------------------------------------------------
/** Thread-safe reading of the last updated event
 */
irr::SEvent Wiimote::getIrrEvent()
{
    return m_irr_event.getAtomic();
}   // getIrrEvent

#endif // ENABLE_WIIUSE
