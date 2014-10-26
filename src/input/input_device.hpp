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

#ifndef INPUT_DEVICE_HPP
#define INPUT_DEVICE_HPP

#include <string>

#include "input/device_config.hpp"
#include "input/input.hpp"
#include "input/input_manager.hpp"
#include "states_screens/state_manager.hpp"
#include "utils/no_copy.hpp"
/**
  * \brief Input device type
  * \ingroup input
  */
enum DeviceType
{
    DT_KEYBOARD,
    DT_GAMEPAD
};

/**
  * \brief base class for input devices
  * \ingroup input
  */
class InputDevice: public NoCopy
{
    friend class DeviceManager;
protected:
    DeviceType m_type;
    StateManager::ActivePlayer* m_player;
    DeviceConfig* m_configuration;

public:
     /** If device has a name; unused for keyboards since AFAIK we
      *  can't tell keyboards apart. */
    std::string m_name;

    InputDevice();
    ~InputDevice();

    void setConfiguration(DeviceConfig *config) {m_configuration = config;}
    DeviceConfig *getConfiguration() {return m_configuration;}

    DeviceType getType() const { return m_type; };

    void setPlayer(StateManager::ActivePlayer* owner);
    StateManager::ActivePlayer *getPlayer() { return m_player; }
};

#endif
