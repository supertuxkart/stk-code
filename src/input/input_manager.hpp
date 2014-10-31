//  SuperTuxKart - a fun racing game with go-kart
//
//  Copyright (C) 2004-2006 Steve Baker <sjbaker1@airmail.net>
//  Copyright (C) 2006-2013 SuperTuxKart-Team
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

#ifndef HEADER_INPUT_MANAGER_HPP
#define HEADER_INPUT_MANAGER_HPP

#include <string>
#include <vector>
#include <set>

#include "guiengine/event_handler.hpp"
#include "input/input.hpp"
#include "utils/no_copy.hpp"

class DeviceManager;

/**
  * \brief Class to handle input.
  * \ingroup input
  */
class InputManager: public NoCopy
{
public:
    enum InputDriverMode
    {
        MENU = 0,
        INGAME,
        INPUT_SENSE_KEYBOARD,
        INPUT_SENSE_GAMEPAD,
        BOOTSTRAP
    };

    // to put a delay before a new gamepad axis move is considered in menu
    bool m_timer_in_use;
    float m_timer;

private:

    DeviceManager  *m_device_manager;
    std::set<int>   m_sensed_input_high_gamepad;
    std::set<int>   m_sensed_input_high_kbd;
    std::set<int>   m_sensed_input_zero_gamepad;

    InputDriverMode  m_mode;

    /** When at true, only the master player can play with menus */
    bool m_master_player_only;

    /* Helper values to store and track the relative mouse movements. If these
    * values exceed the deadzone value the input is reported to the game. This
    * makes the mouse behave like an analog axis on a gamepad/joystick.
    */
    int    m_mouse_val_x, m_mouse_val_y;

    void   dispatchInput(Input::InputType, int deviceID, int btnID,
                         Input::AxisDirection direction, int value);
    void   handleStaticAction(int id0, int value);
    void   inputSensing(Input::InputType type, int deviceID, int btnID,
                        Input::AxisDirection axisDirection,  int value);
public:
           InputManager();
          ~InputManager();
    // void   initGamePadDevices();

    //void   input();
    GUIEngine::EventPropagation   input(const irr::SEvent& event);

    DeviceManager* getDeviceManager() { return m_device_manager; }

    void   setMode(InputDriverMode);
    bool   isInMode(InputDriverMode);
    InputDriverMode getMode() { return m_mode; }

    /** When this mode is enabled, only the master player will be able to play
     *  with menus (only works in 'assign' mode) */
    void   setMasterPlayerOnly(bool enabled);

    /** Returns whether only the master player should be allowed to perform
     *  changes in menus. */
    bool    masterPlayerOnly() const;

    void   update(float dt);

    /** Returns the ID of the player that plays with the keyboard,
     *  or -1 if none. */
    int    getPlayerKeyboardID() const;
};

extern InputManager *input_manager;

#endif
