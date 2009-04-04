//  $Id: input_manager.hpp 694 2006-08-29 07:42:36Z hiker $
//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2004-2006 Steve Baker <sjbaker1@airmail.net>
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

#ifndef HEADER_INPUT_MANAGER_H
#define HEADER_INPUT_MANAGER_H

#include <string>
#include <vector>
#include <irrlicht.h>

#include <SDL/SDL.h>

#include "input/input.hpp"

class DeviceManager;

/** Class to handle input. 
 */
class InputManager
{
public:
    enum InputDriverMode {
        MENU = 0,
        INGAME,
        INPUT_SENSE_PREFER_AXIS,
        INPUT_SENSE_PREFER_BUTTON,
        LOWLEVEL,
        BOOTSTRAP
    };

    // to put a delay before a new gamepad axis move is considered in menu
    bool m_timer_in_use;
    float m_timer;
    
private:

	Input          *m_sensed_input;
    DeviceManager  *m_device_manager;
    
    /** Stores the maximum sensed input values. This allows to select the
     *  axis which was pushed the furthest when sensing input. */
    int              m_max_sensed_input;
    Input::InputType m_max_sensed_type;
	//ActionMap       *m_action_map;
	//GamePadDevice      **m_stick_infos;
	InputDriverMode  m_mode;
	
	/* Helper values to store and track the relative mouse movements. If these
	* values exceed the deadzone value the input is reported to the game. This
  	* makes the mouse behave like an analog axis on a gamepad/joystick.
	*/
	int    m_mouse_val_x, m_mouse_val_y;
    
    void   input(Input::InputType, int, int, int, int);
    void   postIrrLichtMouseEvent(irr::EMOUSE_INPUT_EVENT type, const int x, const int y);
    void   handleStaticAction(int id0, int value);
    void   handlePlayerAction(PlayerAction pa, const int playerNo,  int value);
public:
	       InputManager();
          ~InputManager();
	void   initGamePadDevices();
	void   input();
	void   setMode(InputDriverMode);		
	bool   isInMode(InputDriverMode);
    
    void   update(float dt);
    
	Input &getSensedInput();
};

extern InputManager *input_manager;

#endif
