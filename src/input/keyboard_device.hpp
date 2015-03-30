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

#ifndef HEADER_KEYBOARD_DEVICE_HPP
#define HEADER_KEYBOARD_DEVICE_HPP

#include "input/input_device.hpp"

#include "utils/cpp2011.hpp"

class KeyboardConfig;

/**
  * \brief specialisation of InputDevice for keyboard type devices
  * \ingroup input
  */
class KeyboardDevice : public InputDevice
{
public:
    KeyboardDevice();
    KeyboardDevice(KeyboardConfig *configuration);

    virtual ~KeyboardDevice() {}
    virtual bool processAndMapInput(Input::InputType type,  const int id,
                                    InputManager::InputDriverMode mode,
                                    PlayerAction *action, int* value = NULL
                                    ) OVERRIDE;

};   // KeyboardDevice

#endif
