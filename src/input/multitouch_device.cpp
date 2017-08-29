//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2013-2015 SuperTuxKart-Team
//
//  This program is free software; you can redistribute it and/or
//  modify it under the terms of the GNU General Public License
//  as published by the Free Software Foundation; either version 3
//  of the License: or (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

#include <cassert>
#include <algorithm>

#include "config/user_config.hpp"
#include "input/multitouch_device.hpp"
#include "karts/abstract_kart.hpp"
#include "karts/controller/controller.hpp"
#include "graphics/irr_driver.hpp"
#include "guiengine/modaldialog.hpp"
#include "guiengine/screen_keyboard.hpp"

// ----------------------------------------------------------------------------
/** The multitouch device constructor
 */
MultitouchDevice::MultitouchDevice()
{
    m_configuration = NULL;
    m_type          = DT_MULTITOUCH;
    m_name          = "Multitouch";
    m_player        = NULL;
    m_accelerometer_active = false;
#ifdef ANDROID
    m_android_device = dynamic_cast<CIrrDeviceAndroid*>(
                                                    irr_driver->getDevice());
    assert(m_android_device != NULL);
#endif

    reset();
    updateConfigParams();
}   // MultitouchDevice

// ----------------------------------------------------------------------------
/** The multitouch device destructor
 */
MultitouchDevice::~MultitouchDevice()
{
    clearButtons();
}

// ----------------------------------------------------------------------------
/** Returns a number of fingers that are currently in use
 */
unsigned int MultitouchDevice::getActiveTouchesCount()
{
    unsigned int count = 0;

    for (MultitouchEvent event : m_events)
    {
        if (event.touched)
            count++;
    }

    return count;
} // getActiveTouchesCount

// ----------------------------------------------------------------------------
/** Creates a button of specified type and position. The button is then updated
 *  when touch event occurs and proper action is sent to player controller.
 *  Note that it just determines the screen area that is considered as button
 *  and it doesn't draw the GUI element on a screen.
 *  \param type The button type that determines its behaviour.
 *  \param x Vertical position of the button.
 *  \param y Horizontal position of the button.
 *  \param width Width of the button.
 *  \param height Height of the button.
 */
void MultitouchDevice::addButton(MultitouchButtonType type, int x, int y,
                                 int width, int height)
{
    assert(width > 0 && height > 0);

    MultitouchButton* button = new MultitouchButton();
    button->type = type;
    button->event_id = 0;
    button->pressed = false;
    button->x = x;
    button->y = y;
    button->width = width;
    button->height = height;
    button->axis_x = 0.0f;
    button->axis_y = 0.0f;

    switch (button->type)
    {
    case MultitouchButtonType::BUTTON_FIRE:
        button->action = PA_FIRE;
        break;
    case MultitouchButtonType::BUTTON_NITRO:
        button->action = PA_NITRO;
        break;
    case MultitouchButtonType::BUTTON_SKIDDING:
        button->action = PA_DRIFT;
        break;
    case MultitouchButtonType::BUTTON_LOOK_BACKWARDS:
        button->action = PA_LOOK_BACK;
        break;
    case MultitouchButtonType::BUTTON_RESCUE:
        button->action = PA_RESCUE;
        break;
    case MultitouchButtonType::BUTTON_ESCAPE:
        button->action = PA_PAUSE_RACE;
        break;
    case MultitouchButtonType::BUTTON_UP:
        button->action = PA_ACCEL;
        break;
    case MultitouchButtonType::BUTTON_DOWN:
        button->action = PA_BRAKE;
        break;
    case MultitouchButtonType::BUTTON_LEFT:
        button->action = PA_STEER_LEFT;
        break;
    case MultitouchButtonType::BUTTON_RIGHT:
        button->action = PA_STEER_RIGHT;
        break;
    default:
        button->action = PA_BEFORE_FIRST;
        break;
    }

    m_buttons.push_back(button);

#ifdef ANDROID
    if (button->type == MultitouchButtonType::BUTTON_STEERING)
    {
        if (UserConfigParams::m_multitouch_accelerometer > 0 &&
            !m_android_device->isAccelerometerActive())
        {
            m_android_device->activateAccelerometer(1.0f / 30);

            if (m_android_device->isAccelerometerActive())
            {
                m_accelerometer_active = true;
            }
        }
    }
#endif
} // addButton

// ----------------------------------------------------------------------------
/** Deletes all previously created buttons
 */
void MultitouchDevice::clearButtons()
{
#ifdef ANDROID
    if (m_accelerometer_active == true &&
        m_android_device->isAccelerometerActive())
    {
        m_android_device->deactivateAccelerometer();
        m_accelerometer_active = false;
    }
#endif

    for (MultitouchButton* button : m_buttons)
    {
        delete button;
    }

    m_buttons.clear();
} // clearButtons

// ----------------------------------------------------------------------------
/** Sets all buttons and events to default state
 */
void MultitouchDevice::reset()
{
    for (MultitouchButton* button : m_buttons)
    {
        button->pressed = false;
        button->event_id = 0;
        button->axis_x = 0.0f;
        button->axis_y = 0.0f;
    }

    for (MultitouchEvent& event : m_events)
    {
        event.id = 0;
        event.touched = false;
        event.x = 0;
        event.y = 0;
    }
} // reset

// ----------------------------------------------------------------------------
/** The function that is executed when touch event occurs. It updates the
 *  buttons state when it's needed.
 *  \param event_id The id of touch event that should be processed.
 */
void MultitouchDevice::updateDeviceState(unsigned int event_id)
{
    assert(event_id < m_events.size());

    MultitouchButton* pressed_button = NULL;
    
    for (MultitouchButton* button : m_buttons)
    {
        if (button->pressed && button->event_id == event_id) 
        {
            pressed_button = button;
            break;
        }
    }

    for (MultitouchButton* button : m_buttons)
    {
        if (pressed_button != NULL && button != pressed_button)
            continue;
            
        bool update_controls = false;
        bool prev_button_state = button->pressed;
        MultitouchEvent event = m_events[event_id];

        if (pressed_button != NULL ||
            (event.x >= button->x && event.x <= button->x + button->width &&
            event.y >= button->y && event.y <= button->y + button->height))
        {
            button->pressed = event.touched;
            button->event_id = event_id;

            if (button->type == MultitouchButtonType::BUTTON_STEERING)
            {
                float prev_axis_x = button->axis_x;
                float prev_axis_y = button->axis_y;
                
                if (button->pressed == true)
                {
                    updateButtonAxes(button,
                         (float)(event.x - button->x) / (button->width/2) - 1,
                         (float)(event.y - button->y) / (button->height/2) - 1);
                }
                else
                {
                    updateButtonAxes(button, 0.0f, 0.0f);
                }

                if (prev_axis_x != button->axis_x ||
                    prev_axis_y != button->axis_y)
                {
                    update_controls = true;
                }
            }
        }

        if (prev_button_state != button->pressed)
        {
            update_controls = true;
        }

        if (update_controls)
        {
            handleControls(button);
        }

    }
} // updateDeviceState

// ----------------------------------------------------------------------------
/** Updates config parameters i.e. when they are modified in options
 */
void MultitouchDevice::updateConfigParams()
{
    m_deadzone_center = UserConfigParams::m_multitouch_deadzone_center;
    m_deadzone_center = std::min(std::max(m_deadzone_center, 0.0f), 0.5f);

    m_deadzone_edge = UserConfigParams::m_multitouch_deadzone_edge;
    m_deadzone_edge = std::min(std::max(m_deadzone_edge, 0.0f), 0.5f);
} // updateConfigParams

// ----------------------------------------------------------------------------
/** Helper function that returns a steering factor for steering button.
 *  \param value The axis value from 0 to 1.
 */
float MultitouchDevice::getSteeringFactor(float value)
{
    if (m_deadzone_edge + m_deadzone_center >= 1.0f)
        return 1.0f;

    assert(m_deadzone_edge + m_deadzone_center != 1.0f);

    return std::min((value - m_deadzone_center) / (1.0f - m_deadzone_edge -
                    m_deadzone_center), 1.0f);
}

/** Updates the button axes. It leaves X axis untouched if the accelerometer is
 *  used for turning left/right
 *  \param button A button that should be updated
 *  \param x A value from 0 to 1
 *  \param y A value from 0 to 1
 */
void MultitouchDevice::updateButtonAxes(MultitouchButton* button, float x,
                                        float y)
{
    if (m_accelerometer_active == false)
    {
        button->axis_x = x;
    }

    button->axis_y = y;
}

// ----------------------------------------------------------------------------


// ----------------------------------------------------------------------------
/** Sends proper action for player controller depending on the button type
 *  and state.
 *  \param button The button that should be handled.
 */
void MultitouchDevice::handleControls(MultitouchButton* button)
{
    if (m_player == NULL)
        return;

    // Handle multitouch events only when race is running. It avoids to process
    // it when pause dialog is active during the race. And there is no reason
    // to use it for GUI navigation.
    if (StateManager::get()->getGameState() != GUIEngine::GAME ||
        GUIEngine::ModalDialog::isADialogActive() ||
        GUIEngine::ScreenKeyboard::isActive() ||
        race_manager->isWatchingReplay())
        return;

    AbstractKart* pk = m_player->getKart();

    if (pk == NULL)
        return;

    Controller* controller = pk->getController();

    if (controller == NULL)
        return;

    if (button->type == MultitouchButtonType::BUTTON_STEERING)
    {
        if (button->axis_y < -m_deadzone_center)
        {
            float factor = getSteeringFactor(std::abs(button->axis_y));
            controller->action(PA_ACCEL, int(factor * Input::MAX_VALUE));
        }
        else if (button->axis_y > m_deadzone_center)
        {
            float factor = getSteeringFactor(std::abs(button->axis_y));
            controller->action(PA_BRAKE, int(factor * Input::MAX_VALUE));
        }
        else
        {
            controller->action(PA_BRAKE, 0);
            controller->action(PA_ACCEL, 0);
        }

        if (button->axis_x < -m_deadzone_center)
        {
            float factor = getSteeringFactor(std::abs(button->axis_x));
            controller->action(PA_STEER_LEFT, int(factor * Input::MAX_VALUE));
        }
        else if (button->axis_x > m_deadzone_center)
        {
            float factor = getSteeringFactor(std::abs(button->axis_x));
            controller->action(PA_STEER_RIGHT, int(factor * Input::MAX_VALUE));
        }
        else
        {
            controller->action(PA_STEER_LEFT, 0);
            controller->action(PA_STEER_RIGHT, 0);
        }
    }
    else
    {
        if (button->action != PA_BEFORE_FIRST)
        {
            int value = button->pressed ? Input::MAX_VALUE : 0;
            controller->action(button->action, value);
        }
    }
} // handleControls

// ----------------------------------------------------------------------------
