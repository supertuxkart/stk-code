//  SuperTuxKart - a fun racing game with go-kart
//
//  Copyright (C) 2009-2015 Marianne Gagnon
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


#include "input/input.hpp"
#include "input/input_manager.hpp"
#include "states_screens/state_manager.hpp"
#include "utils/no_copy.hpp"

#include <string>

class DeviceConfig;

/**
  * \brief Input device type
  * \ingroup input
  */
enum DeviceType
{
    DT_KEYBOARD,
    DT_GAMEPAD,
    DT_MULTITOUCH
};

/**
  * \brief base class for input devices
  * \ingroup input
  */
class InputDevice: public NoCopy
{
protected:
    /** For SDL controller it's set false when it's unplugged. */
    bool m_connected;

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
    virtual bool processAndMapInput(Input::InputType type,  const int id,
                                    InputManager::InputDriverMode mode,
                                    PlayerAction *action, int* value = NULL
                                    ) = 0;

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
    // ------------------------------------------------------------------------
    void setConnected(bool val) { m_connected = val; }
    // ------------------------------------------------------------------------
    bool isConnected() const { return m_connected; }
};   // class InputDevice

#endif
