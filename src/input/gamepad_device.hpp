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

#ifndef HEADER_GAMEPAD_DEVICE_HPP
#define HEADER_GAMEPAD_DEVICE_HPP

#include "input/input_device.hpp"

class GamepadConfig;
/**
  * \brief specialisation of Inputdevice for gamepad type devices
  * \ingroup input
  */
class GamePadDevice : public InputDevice
{
    void resetAxisDirection(const int axis, Input::AxisDirection direction);
    bool m_buttonPressed[SEvent::SJoystickEvent::NUMBER_OF_BUTTONS];

public:
    Input::AxisDirection *m_prev_axis_directions;

    /** used to determine if an axis is valid; an axis is considered valid
      * when at least 2 different values are read from this axis (if an axis
      * keeps on sending the exact same value continuously, chances are that
      * it's not meant by the user - for instance some gamepads have hats or
      * analog switches that get reported as axis, we even had a report that
      * on linux some hard disks may be reported as gamepads with
      * uninteresting axis values)
      */
    int                  *m_prev_axis_value;
    /** \see m_prev_axis_value */
    bool                 *m_axis_ok;

    int                   m_deadzone;
    int                   m_index;
    int                   m_axis_count;
    int                   m_button_count;

    /** Constructor for GamePadDevice from a connected gamepad for which no
      * configuration existed (defaults will be used)
      *  \param irrIndex Index of stick as given by irrLicht.
      */
    GamePadDevice(const int irrIndex, const std::string name,
                  const int axis_number,
                  const int btnAmount, GamepadConfig *configuration);
    virtual ~GamePadDevice();

    bool isButtonPressed(const int i);
    void setButtonPressed(const int i, bool isButtonPressed);

    /**
     * Invoked when this device it used. Verifies if the key/button that
     * was pressed is associated with a binding. If yes, sets action and
     * returns true; otherwise returns false.
     *
     * \param      id      ID of the key that was pressed or of the axis
     *                     that was triggered (depending on
     *                     the value of the 'type' parameter)
     * \param      mode    Used to determine whether to map menu actions or
     *                     game actions
     * \param[out] action  The action associated to this input (only check
     *                     this value if method returned true)
     *
     * \return Whether the pressed key/button is bound with an action
     */
    bool processAndMapInput(PlayerAction* action, Input::InputType type,
                            const int id, 
                            InputManager::InputDriverMode mode, int* value);

};


#endif
