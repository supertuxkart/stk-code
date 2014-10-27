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

#ifndef HEADER_INPUT_DEVICE_HPP
#define HEADER_INPUT_DEVICE_HPP


#include "input/device_config.hpp"
#include "input/input.hpp"
#include "input/input_manager.hpp"
#include "states_screens/state_manager.hpp"
#include "utils/no_copy.hpp"

#include <string>

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
protected:
    /** Device type (keyboard, gamepad). */
    DeviceType m_type;

    /** Which player is using this device. */
    StateManager::ActivePlayer* m_player;

    /** The configuration for this device. */
    DeviceConfig* m_configuration;

    /** If device has a name; unused for keyboards since AFAIK we
     *  can't tell keyboards apart. */
    std::string m_name;

public:

             InputDevice();
    virtual ~InputDevice();
        bool processAndMapInput(PlayerAction* action, Input::InputType type,  int id, InputManager::InputDriverMode mode);

    bool processAndMapInput(PlayerAction* action, Input::InputType type, const int id,
                            int* value, InputManager::InputDriverMode mode);

#ifdef NOTYET
    virtual bool processAndMapInput(PlayerAction *action,
                                    Input::InputType type, 

                                    const int id,
                                    int* value,
                                    InputManager::InputDriverMode mode,
                                    PlayerAction* action) = 0;
#endif

    // ------------------------------------------------------------------------
    /** Sets which players uses this device; or pass NULL to say no player 
     *  uses it. */
    void setPlayer(StateManager::ActivePlayer* owner) { m_player = owner; }

    // ------------------------------------------------------------------------
    /** Sets the configuration to be used by this input device. */
    void setConfiguration(DeviceConfig *config) {m_configuration = config;}

    // ------------------------------------------------------------------------
    /** Returns the configuration for this device. */
    DeviceConfig *getConfiguration() {return m_configuration;}

    // ------------------------------------------------------------------------
    /** Returns the type of this device. */
    DeviceType getType() const { return m_type; };

    // ------------------------------------------------------------------------
    /** Returns the player using this device. */
    StateManager::ActivePlayer *getPlayer() { return m_player; }
    // ------------------------------------------------------------------------
    /** Returns the name of this device. */
    const std::string& getName() const { return m_name; }
};   // class InputDevice

#endif
