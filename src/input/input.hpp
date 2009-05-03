//  $Id: input.hpp 694 2006-08-29 07:42:36Z hiker $
//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2007-2008 Robert Schuster <robertschuster@fsfe.org>
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

#ifndef HEADER_INPUT_HPP
#define HEADER_INPUT_HPP

#include <string>

const int DEADZONE_MOUSE       =  150;
const int DEADZONE_MOUSE_SENSE =  200;
const int DEADZONE_JOYSTICK    = 2000;
const int MULTIPLIER_MOUSE     =  750;

struct Input
{
    enum AxisDirection {
        AD_NEGATIVE,
        AD_POSITIVE,
        AD_NEUTRAL
    };

    enum InputType {
        IT_NONE = 0,
        IT_KEYBOARD,
        IT_STICKMOTION,
        IT_STICKBUTTON,
        IT_STICKHAT,
        IT_MOUSEMOTION,
        IT_MOUSEBUTTON
    };
    static const int IT_LAST = IT_MOUSEBUTTON;

    InputType type;
    int id0;
    int id1;
    int id2;

	// Esoteric C++ feature alarm: structs are classes where the fields and
	// methods are public by default. I just needed some convenient constructors
	// for this struct.
	Input()
		: type(IT_NONE), id0(0), id1(0), id2(0)
	{
		// Nothing to do.
	}
	
	/** Creates an Input instance which represents an arbitrary way of getting
	 * game input using a type specifier and 3 integers.
	 *
	 * Meaning of the 3 integers for each InputType:
	 * IT_NONE: This means nothing. In certain cases this is regarded as an
	 * unset binding.
	 * IT_KEYBOARD: id0 is a irrLicht value.
	 * IT_STICKMOTION: id0 - stick index, id1 - axis index, id2 - axis direction
	 * (negative, positive). You can assume that axis 0 is the X-Axis where the
     * negative direction is to the left and that axis 1 is the Y-Axis with the
	 * negative direction being upwards.
	 * IT_STICKBUTTON: id0 - stick index, id1 - button index. Button 0 and 1 are
	 * usually reached most easily.
	 * IT_STICKHAT: This is not yet implemented.
	 * IT_MOUSEMOTION: id0 - axis index (0 -> X, 1 -> Y). Mouse wheel is
	 * represented as buttons!
	 * IT_MOUSEBUTTON: id0 - button number (1 -> left, 2 -> middle, 3 -> right,
	 * ...) 
	 *
	 * Note: For joystick bindings that are actice in the menu the joystick's
	 * index should be zero. The binding will react to all joysticks connected
	 * to the system.
	 */
	Input(InputType ntype, int nid0 , int nid1 = 0, int nid2= 0) 
		: type(ntype), id0(nid0), id1(nid1), id2(nid2)
	{
		// Nothing to do.
	}
};

enum PlayerAction
{
    PA_FIRST = -1,
    
	PA_LEFT = 0,
	PA_RIGHT,
	PA_ACCEL,
	PA_BRAKE,
	PA_NITRO,
	PA_DRIFT,
	PA_RESCUE,
	PA_FIRE,
	PA_LOOK_BACK,
    
    PA_COUNT
};

static std::string KartActionStrings[PA_COUNT] = {std::string("left"), 
                                                  std::string("right"),
                                                  std::string("accel"),
                                                  std::string("brake"),
                                                  std::string("nitro"),
                                                  std::string("drift"),
                                                  std::string("rescue"),
                                                  std::string("fire"),
                                                  std::string("lookBack")};

enum StaticAction
{
	// Below this are synthetic game actions which are never triggered through
	// an input device.
	GA_NULL,				// Nothing dummy entry.
	GA_SENSE_CANCEL,		// Input sensing canceled.
	GA_SENSE_COMPLETE,		// Input sensing successfully finished.
	
	// Below this point are the game actions which can happen while in menu
	// mode.
		
	GA_ENTER,				// Enter menu, acknowledge, ...
	GA_LEAVE,				// Leave a menu.
	
	GA_CLEAR_MAPPING,		// Clear an input mapping.
	
	GA_INC_SCROLL_SPEED,
	GA_INC_SCROLL_SPEED_FAST,
	GA_DEC_SCROLL_SPEED,
	GA_DEC_SCROLL_SPEED_FAST,
	
	GA_CURSOR_UP,
	GA_CURSOR_DOWN,
	GA_CURSOR_LEFT,
	GA_CURSOR_RIGHT,
	

	GA_TOGGLE_FULLSCREEN,	// Switch between windowed/fullscreen mode
	GA_LEAVE_RACE,			// Switch from race to menu.
	
	GA_DEBUG_ADD_MISSILE,
	GA_DEBUG_ADD_BOWLING,
	GA_DEBUG_ADD_HOMING,
	GA_DEBUG_TOGGLE_FPS,
	GA_DEBUG_TOGGLE_WIREFRAME,
	GA_DEBUG_HISTORY
	
};



/* Some constants to make future changes more easier to handle. If you use
 * any of the GameAction constants to mark the beginning or end of a range
 * or denote a count then something is wrong with your code. ;)
 */

/** Generally the first GameAction constant. Unlikely to change. */
const int GA_FIRST = GA_NULL;

/** A usefull value for array allocations. Should always be to the
 * last constant + 1.
 */
const int GA_COUNT = (GA_DEBUG_HISTORY + 1);

/* The range of GameAction constants that is used while in menu mode. */
const int GA_FIRST_MENU = GA_ENTER;
const int GA_LAST_MENU = GA_CURSOR_RIGHT;


/* The range of GameAction constants which are used ingame but are considered
 * fixed and their Inputs should not be used by the players.
 */
const int GA_FIRST_INGAME_FIXED = GA_TOGGLE_FULLSCREEN;
const int GA_LAST_INGAME_FIXED = GA_DEBUG_HISTORY;


#endif
