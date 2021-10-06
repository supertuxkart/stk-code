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
    void setIrrIndex(int i ) { m_irr_index = i; }
    bool useForceFeedback() const;
    int getAutoCenterStrength() const;
};   // class GamepadDevice

#endif
