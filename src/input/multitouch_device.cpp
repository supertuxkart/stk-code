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

#include <IrrlichtDevice.h>

// ----------------------------------------------------------------------------
/** The multitouch device constructor
 */
MultitouchDevice::MultitouchDevice()
{
    m_configuration = NULL;
    m_type          = DT_MULTITOUCH;
    m_name          = "Multitouch";
    m_player        = NULL;
    m_controller    = NULL;
    m_irrlicht_device = irr_driver->getDevice();

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
 *  \param callback Pointer to a function that is executed on button event.
 */
void MultitouchDevice::addButton(MultitouchButtonType type, int x, int y,
                                 int width, int height, 
                                 void (*callback)(unsigned int, bool))
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
    button->id = m_buttons.size();
    button->callback = callback;

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
} // addButton

// ----------------------------------------------------------------------------
/** Deletes all previously created buttons
 */
void MultitouchDevice::clearButtons()
{
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

    m_orientation = 0.0f;
    m_gyro_time = 0;
    m_affected_linked_buttons.clear();
} // reset

// ----------------------------------------------------------------------------
/** Activates accelerometer
 */
void MultitouchDevice::activateAccelerometer()
{
    if (!m_irrlicht_device->isAccelerometerActive())
    {
        m_irrlicht_device->activateAccelerometer(1.0f / 30);
    }
}

// ----------------------------------------------------------------------------
/** Deativates accelerometer
 */
void MultitouchDevice::deactivateAccelerometer()
{
    if (m_irrlicht_device->isAccelerometerActive())
    {
        m_irrlicht_device->deactivateAccelerometer();
    }
}

// ----------------------------------------------------------------------------
/** Get accelerometer state
 *  \return true if accelerometer is active
 */
bool MultitouchDevice::isAccelerometerActive()
{
    return m_irrlicht_device->isAccelerometerActive();
}

// ----------------------------------------------------------------------------
/** Activates gyroscope
 */
void MultitouchDevice::activateGyroscope()
{
    if (!m_irrlicht_device->isGyroscopeActive())
    {
        // Assume 60 FPS, some phones can do 90 and 120 FPS but we won't handle 
        // them now
        m_irrlicht_device->activateGyroscope(1.0f / 60);
    }
}

// ----------------------------------------------------------------------------
/** Deativates gyroscope
 */
void MultitouchDevice::deactivateGyroscope()
{
    if (m_irrlicht_device->isGyroscopeActive())
    {
        m_irrlicht_device->deactivateGyroscope();
    }
}

// ----------------------------------------------------------------------------
/** Get gyroscope state
 *  \return true if gyroscope is active
 */
bool MultitouchDevice::isGyroscopeActive()
{
    return m_irrlicht_device->isGyroscopeActive();
}

// ----------------------------------------------------------------------------
/** The function that is executed when touch event occurs. It updates the
 *  buttons state when it's needed.
 *  \param event_id The id of touch event that should be processed.
 */
void MultitouchDevice::updateDeviceState(unsigned int event_id)
{
    assert(event_id < m_events.size());

    auto inside_button = [](MultitouchButton* button,
        const MultitouchEvent& event)->bool
        {
            if (button == NULL)
                return false;
            return event.x >= button->x &&
                event.x <= button->x + button->width &&
                event.y >= button->y && event.y <= button->y + button->height;
        };
    auto is_linked_button = [](MultitouchButton* button)->bool
        {
            if (button == NULL)
                return false;
            return button->type >= BUTTON_FIRE &&
                button->type <= BUTTON_LOOK_BACKWARDS;
        };

    const MultitouchEvent& event = m_events[event_id];
    std::vector<MultitouchButton*> linked_buttons;
    for (MultitouchButton* button : m_buttons)
    {
        if (button == NULL || !is_linked_button(button))
            continue;
        linked_buttons.push_back(button);
    }

    if (!event.touched)
    {
        m_affected_linked_buttons.erase(event.id);
    }
    else
    {
        for (MultitouchButton* button : linked_buttons)
        {
            if (inside_button(button, event))
            {
                auto& ret = m_affected_linked_buttons[event.id];
                if (!ret.empty() && ret.back() == button)
                    continue;
                if (ret.size() >= 2)
                {
                    if (ret[0] == button)
                        ret.resize(1);
                    else if (ret[ret.size() - 2] == button)
                        ret.resize(ret.size() - 1);
                    else
                        ret.push_back(button);
                }
                else
                    ret.push_back(button);
                break;
            }
        }
    }
    for (MultitouchButton* button : linked_buttons)
    {
        bool found = false;
        for (auto& p : m_affected_linked_buttons)
        {
            auto it = std::find(p.second.begin(), p.second.end(), button);
            if (it != p.second.end())
            {
                button->pressed = true;
                handleControls(button);
                found = true;
                break;
            }
        }
        if (!found)
        {
            button->pressed = false;
            handleControls(button);
        }
    }

    MultitouchButton* pressed_button = NULL;
    
    for (MultitouchButton* button : m_buttons)
    {
        if (is_linked_button(button))
            continue;
        if (button->pressed && button->event_id == event_id)
        {
            pressed_button = button;
            break;
        }
    }

    for (MultitouchButton* button : m_buttons)
    {
        if (is_linked_button(button))
            continue;
        if (pressed_button != NULL && button != pressed_button)
            continue;

        bool update_controls = false;
        bool prev_button_state = button->pressed;

        if (pressed_button != NULL || inside_button(button, event))
        {
            button->pressed = event.touched;
            button->event_id = event_id;

            if (button->type == MultitouchButtonType::BUTTON_STEERING)
            {
                float prev_axis_x = button->axis_x;
                
                if (button->pressed == true)
                {
                    button->axis_x = 
                        (float)(event.x - button->x) / (button->width/2) - 1;
                }
                else
                {
                    button->axis_x = 0.0f;
                }

                if (prev_axis_x != button->axis_x)
                {
                    update_controls = true;
                }
            }

            if (button->type == MultitouchButtonType::BUTTON_STEERING ||
                button->type == MultitouchButtonType::BUTTON_UP_DOWN)
            {
                float prev_axis_y = button->axis_y;
                
                if (button->pressed == true)
                {
                    button->axis_y = 
                        (float)(event.y - button->y) / (button->height/2) - 1;
                }
                else
                {
                    button->axis_y = 0.0f;
                }

                if (prev_axis_y != button->axis_y)
                {
                    update_controls = true;
                }
            }
        }

        if (prev_button_state != button->pressed)
        {
            update_controls = true;
        }
        
        if (button->type == MultitouchButtonType::BUTTON_UP_DOWN && 
            !button->pressed)
        {
            update_controls = false;
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
    m_deadzone = UserConfigParams::m_multitouch_deadzone;
    m_deadzone = std::min(std::max(m_deadzone, 0.0f), 0.5f);

    m_sensitivity_x = UserConfigParams::m_multitouch_sensitivity_x;
    m_sensitivity_x = std::min(std::max(m_sensitivity_x, 0.0f), 1.0f);
    
    m_sensitivity_y = UserConfigParams::m_multitouch_sensitivity_y;
    m_sensitivity_y = std::min(std::max(m_sensitivity_y, 0.0f), 1.0f);
} // updateConfigParams

// ----------------------------------------------------------------------------
/** Helper function that returns a steering factor for steering button.
 *  \param value The axis value from 0 to 1.
 *  \param sensitivity Value from 0 to 1.0.
 */
float MultitouchDevice::getSteeringFactor(float value, float sensitivity)
{
    if (m_deadzone >= 1.0f)
        return 0.0f;
        
    if (sensitivity >= 1.0f)
        return 1.0f;

    float factor = (value - m_deadzone) / (1.0f - m_deadzone);
    factor *= 1.0f / (1.0f - sensitivity);
    
    return std::min(factor, 1.0f);
}

// ----------------------------------------------------------------------------

void MultitouchDevice::updateAxisX(float value)
{
    if (m_controller == NULL)
        return;
        
    if (value < -m_deadzone)
    {
        float factor = getSteeringFactor(std::abs(value), m_sensitivity_x);
        m_controller->action(PA_STEER_RIGHT, 0);
        m_controller->action(PA_STEER_LEFT, int(factor * Input::MAX_VALUE));
    }
    else if (value > m_deadzone)
    {
        float factor = getSteeringFactor(std::abs(value), m_sensitivity_x);
        m_controller->action(PA_STEER_LEFT, 0);
        m_controller->action(PA_STEER_RIGHT, int(factor * Input::MAX_VALUE));
    }
    else
    {
        m_controller->action(PA_STEER_LEFT, 0);
        m_controller->action(PA_STEER_RIGHT, 0);
    }
}

// ----------------------------------------------------------------------------

void MultitouchDevice::updateAxisY(float value)
{
    if (m_controller == NULL)
        return;
        
    if (value < -m_deadzone)
    {
        float factor = getSteeringFactor(std::abs(value), m_sensitivity_y);
        m_controller->action(PA_ACCEL, int(factor * Input::MAX_VALUE));
    }
    else if (value > m_deadzone)
    {
        float factor = getSteeringFactor(std::abs(value), m_sensitivity_y);
        m_controller->action(PA_BRAKE, int(factor * Input::MAX_VALUE));
    }
    else
    {
        m_controller->action(PA_BRAKE, 0);
        m_controller->action(PA_ACCEL,
            UserConfigParams::m_multitouch_controls == 1 &&
            UserConfigParams::m_multitouch_auto_acceleration ? Input::MAX_VALUE : 0);
    }

}

// ----------------------------------------------------------------------------

/** Returns device orientation Z angle, in radians, where 0 is landscape 
 *  orientation parallel to the floor.
 */
float MultitouchDevice::getOrientation()
{
    return m_orientation;
}

// ----------------------------------------------------------------------------

/** Update device orientation from the accelerometer measurements.
 *  Accelerometer is shaky, so it adjusts the orientation angle slowly.
 *  \param x Accelerometer X axis
 *  \param y Accelerometer Y axis
 */
void MultitouchDevice::updateOrientationFromAccelerometer(float x, float y)
{
    const float ACCEL_DISCARD_THRESHOLD = 4.0f;
    const float ACCEL_MULTIPLIER = 0.05f; // Slowly adjust the angle over time, 
                                          // this prevents shaking
    const float ACCEL_CHANGE_THRESHOLD = 0.01f; // ~0.5 degrees

    // The device is flat on the table, cannot reliably determine the 
    // orientation
    if (fabsf(x) + fabsf(y) < ACCEL_DISCARD_THRESHOLD)
        return;

    float angle = atan2f(y, x);
    if (angle > (M_PI / 2.0))
    {
        angle = (M_PI / 2.0);
    }
    if (angle < -(M_PI / 2.0))
    {
        angle = -(M_PI / 2.0);
    }

    float delta = angle - m_orientation;
    delta *= ACCEL_MULTIPLIER;
    if (delta > ACCEL_CHANGE_THRESHOLD)
    {
        delta = ACCEL_CHANGE_THRESHOLD;
    }
    if (delta < -ACCEL_CHANGE_THRESHOLD)
    {
        delta = -ACCEL_CHANGE_THRESHOLD;
    }

    m_orientation += delta;

    //Log::warn("Accel", "X %03.4f Y %03.4f angle %03.4f delta %03.4f "
    //          "orientation %03.4f", x, y, angle, delta, m_orientation);
}

// ----------------------------------------------------------------------------

/** Update device orientation from the gyroscope measurements.
 *  Gyroscope is not shaky and very sensitive, but drifts over time.
 *  \param x Gyroscope Z axis
 */
void MultitouchDevice::updateOrientationFromGyroscope(float z)
{
    const float GYRO_SPEED_THRESHOLD = 0.005f;

    uint64_t now = StkTime::getMonoTimeMs();
    uint64_t delta = now - m_gyro_time;
    m_gyro_time = now;
    float timedelta = (float)delta / 1000.f;
    if (timedelta > 0.5f)
    {
        timedelta = 0.1f;
    }

    float angular_speed = -z;

    if (fabsf(angular_speed) < GYRO_SPEED_THRESHOLD)
    {
        angular_speed = 0.0f;
    }

    m_orientation += angular_speed * timedelta;
    if (m_orientation > (M_PI / 2.0))
    {
        m_orientation = (M_PI / 2.0);
    }
    if (m_orientation < -(M_PI / 2.0))
    {
        m_orientation = -(M_PI / 2.0);
    }

    //Log::warn("Gyro", "Z %03.4f angular_speed %03.4f delta %03.4f "
    //          "orientation %03.4f", z, angular_speed, 
    //          angular_speed * timedelta, m_orientation);
}

// ----------------------------------------------------------------------------

/** Sends proper action for player controller depending on the button type
 *  and state.
 *  \param button The button that should be handled.
 */
void MultitouchDevice::handleControls(MultitouchButton* button)
{
    if (!isGameRunning())
        return;

    if (button->type == MultitouchButtonType::BUTTON_ESCAPE)
    {
        StateManager::get()->escapePressed();
    }
    
    if (m_controller != NULL && !RaceManager::get()->isWatchingReplay())
    {
        if (button->type == MultitouchButtonType::BUTTON_STEERING)
        {
            updateAxisX(button->axis_x);
            updateAxisY(button->axis_y);
        }
        else if (button->type == MultitouchButtonType::BUTTON_UP_DOWN)
        {
            updateAxisY(button->axis_y);
        }
        else if (button->action != PA_BEFORE_FIRST)
        {
            int value = button->pressed ? Input::MAX_VALUE : 0;
            m_controller->action(button->action, value);
        }
    }

    if (button->callback != NULL)
    {
        button->callback(button->id, button->pressed);
    }
}

// ----------------------------------------------------------------------------

bool MultitouchDevice::isGameRunning()
{
    return StateManager::get()->getGameState() == GUIEngine::GAME &&
           !GUIEngine::ModalDialog::isADialogActive() &&
           !GUIEngine::ScreenKeyboard::isActive();
}

// ----------------------------------------------------------------------------

void MultitouchDevice::updateController()
{
    if (m_player == NULL)
    {
        m_controller = NULL;
        return;
    }

    // Handle multitouch events only when race is running. It avoids to process
    // it when pause dialog is active during the race. And there is no reason
    // to use it for GUI navigation.
    if (!isGameRunning())
    {
        m_controller = NULL;
        return;
    }

    AbstractKart* pk = m_player->getKart();

    if (pk == NULL)
    {
        m_controller = NULL;
        return;
    }

    m_controller = pk->getController();
}

// ----------------------------------------------------------------------------
