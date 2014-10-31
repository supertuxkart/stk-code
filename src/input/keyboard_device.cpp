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

bool KeyboardDevice::processAndMapInput(PlayerAction* action /* out */,
                                        Input::InputType type,
                                        const int id,
                                        InputManager::InputDriverMode mode)
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
