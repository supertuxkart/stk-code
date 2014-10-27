//  SuperTuxKart - a fun racing game with go-kart
//
//  Copyright (C) 2009-2013 Marianne Gagnon
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

#include "input/device_config.hpp"

#include "guiengine/abstract_state_manager.hpp"
#include "input/input.hpp"
#include "input/input_device.hpp"
#include "karts/abstract_kart.hpp"
#include "karts/controller/player_controller.hpp"
#include "modes/world.hpp"
#include "race/race_manager.hpp"
#include "states_screens/state_manager.hpp"


InputDevice::InputDevice()
{
    m_player        = NULL;
    m_configuration = NULL;
}   // InputDevice

// ----------------------------------------------------------------------------
InputDevice::~InputDevice()
{
}   // ~InputDevice

