//  SuperTuxKart - a fun racing game with go-kart
//
//  Copyright (C) 2009-2013 Marianne Gagnon
//                2014      Joerg Henrichs
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

#include "input/gamepad_device.hpp"

#include "input/gamepad_config.hpp"
#include "karts/abstract_kart.hpp"
#include "karts/controller/player_controller.hpp"

GamePadDevice::GamePadDevice(const int irrIndex, const std::string name,
                             const int axis_count, const int button_count,
                             GamepadConfig *configuration)
{
    m_type                  = DT_GAMEPAD;
    m_prev_axis_directions  = NULL;
    m_configuration         = configuration;
    m_axis_count            = axis_count;
    m_prev_axis_directions  = new Input::AxisDirection[axis_count];
    m_prev_axis_value       = new int[axis_count];
    m_axis_ok               = new bool[axis_count];
    m_button_count          = button_count;
    m_index                 = irrIndex;
    m_name                  = name;

    for (int i = 0; i < axis_count; i++)
    {
        m_prev_axis_directions[i] = Input::AD_NEUTRAL;
        m_prev_axis_value[i] = -1;
        m_axis_ok[i] = false;
    }

    for(int n=0; n<SEvent::SJoystickEvent::NUMBER_OF_BUTTONS; n++)
        m_buttonPressed[n] = false;
}   // GamePadDevice

// ----------------------------------------------------------------------------
/** Destructor for GamePadDevice.
 */
GamePadDevice::~GamePadDevice()
{
    delete[] m_prev_axis_directions;
    delete[] m_prev_axis_value;
    delete[] m_axis_ok;

    // FIXME - any need to close devices?
}   // ~GamePadDevice

// ----------------------------------------------------------------------------

bool GamePadDevice::isButtonPressed(const int i)
{
    return m_buttonPressed[i];
}   // isButtonPressed

// ----------------------------------------------------------------------------

void GamePadDevice::setButtonPressed(const int i, bool isButtonPressed)
{
    m_buttonPressed[i] = isButtonPressed;
}   // setButtonPressed

// ----------------------------------------------------------------------------

void GamePadDevice::resetAxisDirection(const int axis,
                                       Input::AxisDirection direction)
{
    // ignore this while in menus
    if (StateManager::get()->getGameState() != GUIEngine::GAME) return;

    AbstractKart* pk = getPlayer()->getKart();
    if (!pk)
    {
        Log::error("Binding", "Trying to reset axis for an unknown player.");
        return;
    }

    for(int n=0; n<PA_COUNT; n++)
    {
        Binding& bind = m_configuration->getBinding(n);
        if(bind.getType() == Input::IT_STICKMOTION &&
           bind.getId() == axis &&
           bind.getDirection()== direction &&
           pk->getController() != NULL)
        {
            ((PlayerController*)(pk->getController()))
                                                  ->action((PlayerAction)n, 0);
            return;
        }
    }

}   // resetAxisDirection

// ----------------------------------------------------------------------------

bool GamePadDevice::processAndMapInput(PlayerAction* action /* out */,
                                       Input::InputType type, const int id,
                                       InputManager::InputDriverMode mode,
                                       int* value/* inout */)
{
    if (!m_configuration->isEnabled()) return false;

    bool success = false;
    if(m_prev_axis_directions == NULL) return false; // device not open

    if (type == Input::IT_STICKMOTION)
    {
        // this gamepad doesn't even have that many axes
        if (id >= m_axis_count) return false;

        if (getPlayer())
        {
            // going to negative from positive
            if (*value < 0 && m_prev_axis_directions[id] == Input::AD_POSITIVE)
            {
                //  set positive id to 0
                resetAxisDirection(id, Input::AD_POSITIVE);
            }
            // going to positive from negative
            else if (*value > 0 &&
                     m_prev_axis_directions[id] == Input::AD_NEGATIVE)
            {
                //  set negative id to 0
                resetAxisDirection(id, Input::AD_NEGATIVE);
            }
        }

        if     (*value > 0) m_prev_axis_directions[id] = Input::AD_POSITIVE;
        else if(*value < 0) m_prev_axis_directions[id] = Input::AD_NEGATIVE;

        if (!m_axis_ok[id])
        {
            if (m_prev_axis_value[id] == -1)
            {
                // first value we get from this axis
                m_prev_axis_value[id] = *value;
            }
            else if (m_prev_axis_value[id] != *value)
            {
                // second different value we get from this axis, consider it OK
                m_axis_ok[id] = true;
            }
        }

        // check if within deadzone
        if(*value > -m_deadzone && *value < m_deadzone && getPlayer())
        {
            // Axis stands still: This is reported once for digital axes and
            // can be called multipled times for analog ones. Uses the
            // previous direction in which the id was triggered to
            // determine which one has to be brought into the released
            // state. This allows us to regard two directions of an id
            // as completely independent input variants (as if they where
            // two buttons).

            if(m_prev_axis_directions[id] == Input::AD_NEGATIVE)
            {
                // set negative id to 0
                resetAxisDirection(id, Input::AD_NEGATIVE);
            }
            else if(m_prev_axis_directions[id] == Input::AD_POSITIVE)
            {
                // set positive id to 0
                resetAxisDirection(id, Input::AD_POSITIVE);
            }
            m_prev_axis_directions[id] = Input::AD_NEUTRAL;

            return false;
        }

        // If axis did not send proper values yet, ignore it.
        if (!m_axis_ok[id]) return false;
    }


    if (mode == InputManager::INGAME)
    {
        success = m_configuration->getGameAction(type, id, value, action);
    }
    else if (abs(*value) > Input::MAX_VALUE/2)
    {
        // bindings can only be accessed in game and menu modes
        assert(mode == InputManager::MENU);
        success = m_configuration->getMenuAction(type, id, value, action);
    }

    return success;
}   // processAndMapInput

// ----------------------------------------------------------------------------
