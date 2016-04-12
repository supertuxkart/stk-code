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

#include "input/keyboard_device.hpp"

#include "input/keyboard_config.hpp"
// ----------------------------------------------------------------------------
KeyboardDevice::KeyboardDevice(KeyboardConfig *configuration)
{
    m_configuration = configuration;
    m_type          = DT_KEYBOARD;
    m_name          = "Keyboard";
    m_player        = NULL;
}   // KeyboardDevice

// ----------------------------------------------------------------------------
KeyboardDevice::KeyboardDevice()
{
    m_configuration = new KeyboardConfig();
    m_type          = DT_KEYBOARD;
    m_player        = NULL;
}   // KeyboardDevice

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
bool KeyboardDevice::processAndMapInput(Input::InputType type,  const int id,
                                        InputManager::InputDriverMode mode,
                                        PlayerAction *action, int* value)
{
    assert(type==Input::IT_KEYBOARD);
    if (mode == InputManager::INGAME)
    {
        return m_configuration->getGameAction(Input::IT_KEYBOARD, id, 0,
                                              action);
    }
    else
    {
        // bindings can only be accessed in game and menu modes
        assert(mode == InputManager::MENU);
        return m_configuration->getMenuAction(Input::IT_KEYBOARD, id, 0,
                                              action);
    }
}   // processAndMapInput

// ============================================================================
