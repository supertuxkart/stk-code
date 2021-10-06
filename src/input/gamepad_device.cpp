//  SuperTuxKart - a fun racing game with go-kart
//
//  Copyright (C) 2009-2015 Marianne Gagnon
//            (C) 2014-2015 Joerg Henrichs
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
#include "karts/controller/local_player_controller.hpp"

/** Constructor for GamePadDevice from a connected gamepad for which no
 *  configuration existed (defaults will be used)
 *  \param irrIndex Index of stick as given by irrLicht.
 */

GamePadDevice::GamePadDevice(const int irr_index, const std::string &name,
                             const int axis_count, const int button_count,
                             GamepadConfig *configuration)
{
    m_type                  = DT_GAMEPAD;
    m_configuration         = configuration;
    GamepadConfig *config = static_cast<GamepadConfig*>(m_configuration);
    if(m_configuration->getNumberOfButtons()<button_count)
    {
        config->setNumberOfButtons(button_count);
    }

    m_prev_axis_directions.resize(axis_count);
    m_irr_index             = irr_index;
    m_name                  = name;

    for (int i = 0; i < axis_count; i++)
        m_prev_axis_directions[i] = Input::AD_NEUTRAL;

    m_button_pressed.resize(button_count);
    for(int n=0; n<button_count; n++)
        m_button_pressed[n] = false;
}   // GamePadDevice

// ----------------------------------------------------------------------------
/** Destructor for GamePadDevice.
 */
GamePadDevice::~GamePadDevice()
{
}   // ~GamePadDevice

// ----------------------------------------------------------------------------
/** Returns if the specified value is larger than the deadzone. */
bool GamePadDevice::moved(int value) const
{
    int dz = static_cast<GamepadConfig*>(m_configuration)->getDeadzone();
    return abs(value) > dz;
}   // moved

// ----------------------------------------------------------------------------
/** Returns the number of buttons of this gamepad. */
int GamePadDevice::getNumberOfButtons() const
{
    return m_configuration->getNumberOfButtons();
}   // getNumberOfButtons

// ----------------------------------------------------------------------------
bool GamePadDevice::isButtonPressed(const int i)
{
    if (i < (int)m_button_pressed.size())
        return m_button_pressed[i];
    else
        return false;
}   // isButtonPressed

// ----------------------------------------------------------------------------

void GamePadDevice::setButtonPressed(const int i, bool isButtonPressed)
{
    if (i < (int)m_button_pressed.size())
        m_button_pressed[i] = isButtonPressed;
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

    for(int n=PA_BEFORE_FIRST+1; n<PA_COUNT; n++)
    {
        const Binding& bind = m_configuration->getBinding(n);
        if(bind.getType() == Input::IT_STICKMOTION &&
           bind.getId() == axis &&
           bind.getDirection()== direction &&
           pk->getController() != NULL)
        {
            ((LocalPlayerController*)(pk->getController()))
                                                  ->action((PlayerAction)n, 0);
            return;
        }
    }

}   // resetAxisDirection

// ----------------------------------------------------------------------------
/** Invoked when this device it used. Verifies if the key/button that was
 *  pressed is associated with a binding. If yes, sets action and returns
 *  true; otherwise returns false. It can also modify the value used.
 *  \param type Type of input (e.g. IT_STICKMOTION, ...).
 *  \param id   ID of the key that was pressed or of the axis that was
 *              triggered (depending on the value of the 'type' parameter).
 *  \param mode Used to determine whether to map menu actions or
 *              game actions
 * \param[out] action  The action associated to this input (only check
 *                     this value if method returned true)
 * \param[in,out] value The value associated with this type (typically
 *                      how far a gamepad axis is moved).
 *
 * \return Whether the pressed key/button is bound with an action
 */

bool GamePadDevice::processAndMapInput(Input::InputType type, const int id,
                                       InputManager::InputDriverMode mode,
                                       PlayerAction* action /* out */,
                                       int* value           /* inout */ )
{
    if (!m_configuration->isEnabled()) return false;

    // A digital input value is 32767 or -32768 (which then triggers 
    // time-full-steer to be used to adjust actual steering values.
    // To prevent this delay for analog gamesticks, make sure that
    // 32767/-32768 are never used.
    if(m_configuration->isAnalog(type, id))
    {
        if(*value==32767)
            *value = 32766;
        else if(*value==-32768)
            *value = -32767;
    }

    // Desensitizing means to map an input in the range x in [0,1] to
    // x^2. This results in changes close to 0 to have less impact
    // (less sensitive).
    if(m_configuration->desensitize())
    {
        // x/32767 is in [-1,1], (x/32767)^2 is in [0,1]. Take care of the
        // sign and map this back to [0,32767] by multiplying by 32767.
        // Which all in all results in:
        *value = int( (*value / 32767.0f) * fabsf(float(*value)) );
    }

    bool success = false;
    if (type == Input::IT_STICKMOTION &&
        m_prev_axis_directions.size() == 0) return false; // device not open

    if (type == Input::IT_STICKMOTION)
    {
        // this gamepad doesn't even have that many axes
        if (id >= m_configuration->getNumberOfAxes()) return false;

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

        int dz = static_cast<GamepadConfig*>(m_configuration)->getDeadzone();
        // check if within deadzone
        if(*value > -dz && *value < dz && getPlayer())
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
bool GamePadDevice::useForceFeedback() const
{
    return static_cast<GamepadConfig*>(m_configuration)->useForceFeedback();
}   // useForceFeedback

// ----------------------------------------------------------------------------
int GamePadDevice::getAutoCenterStrength() const
{
    return static_cast<GamepadConfig*>(m_configuration)->getAutoCenterStrength();
}   // shouldAutoCenter
