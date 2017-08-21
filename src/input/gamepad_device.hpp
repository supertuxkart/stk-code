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

#ifndef HEADER_GAMEPAD_DEVICE_HPP
#define HEADER_GAMEPAD_DEVICE_HPP

#include "input/input_device.hpp"
#include "utils/cpp2011.hpp"

#include <vector>
class GamepadConfig;

/**
  * \brief specialisation of Inputdevice for gamepad type devices
  * \ingroup input
  */
class GamePadDevice : public InputDevice
{
    void resetAxisDirection(const int axis, Input::AxisDirection direction);
    std::vector<bool> m_button_pressed;

    std::vector<Input::AxisDirection> m_prev_axis_directions;

    /** used to determine if an axis is valid; an axis is considered valid
      * when at least 2 different values are read from this axis (if an axis
      * keeps on sending the exact same value continuously, chances are that
      * it's not meant by the user - for instance some gamepads have hats or
      * analog switches that get reported as axis, we even had a report that
      * on linux some hard disks may be reported as gamepads with
      * uninteresting axis values)
      */
    std::vector<int> m_prev_axis_value;

    /** \see m_prev_axis_value */
    std::vector<bool> m_axis_ok;

    /** Irrlicht index of this gamepad. */
    int                   m_irr_index;

public:
             GamePadDevice(const int irrIndex, const std::string &name,
                           const int axis_number,
                           const int button_count,
                           GamepadConfig *configuration);
    virtual ~GamePadDevice();
    bool     isButtonPressed(const int i);
    void     setButtonPressed(const int i, bool isButtonPressed);

    virtual bool processAndMapInput(Input::InputType type,  const int id,
                                    InputManager::InputDriverMode mode,
                                    PlayerAction *action, int* value = NULL
                                    ) OVERRIDE;
    int getNumberOfButtons() const;
    bool moved(int value) const;

    // ------------------------------------------------------------------------
    /** Returns the irrlicht index of this gamepad. */
    int getIrrIndex() const { return m_irr_index; }

    // ------------------------------------------------------------------------

};   // class GamepadDevice

#endif
