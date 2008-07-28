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

#ifndef TUXKART_INPUT_H
#define TUXKART_INPUT_H

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
const int IT_LAST = IT_MOUSEBUTTON;

struct Input
{
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
	 * IT_KEYBOARD: id0 is an SDLKey value.
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

//When adding any action at the beginning or at the end, remember to update
//the KA_FIRST and/or KA_LAST constants.
enum KartAction {
	KA_LEFT,
	KA_RIGHT,
	KA_ACCEL,
	KA_BRAKE,
	KA_WHEELIE,
	KA_JUMP,
	KA_RESCUE,
	KA_FIRE,
	KA_LOOK_BACK
};
const int KA_FIRST = KA_LEFT;
const int KA_LAST = KA_LOOK_BACK;
const int KC_COUNT = (KA_LAST + 1);

enum GameAction
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
	
	// The following game actions occur when in ingame mode (= within a race).

	GA_P1_LEFT,
	GA_P1_RIGHT,
	GA_P1_ACCEL,
	GA_P1_BRAKE,
	GA_P1_WHEELIE,
	GA_P1_JUMP,
	GA_P1_RESCUE,
	GA_P1_FIRE,
	GA_P1_LOOK_BACK,
	
	GA_P2_LEFT,
	GA_P2_RIGHT,
	GA_P2_ACCEL,
	GA_P2_BRAKE,
	GA_P2_WHEELIE,
	GA_P2_JUMP,
	GA_P2_RESCUE,
	GA_P2_FIRE,
	GA_P2_LOOK_BACK,
	
	GA_P3_LEFT,
	GA_P3_RIGHT,
	GA_P3_ACCEL,
	GA_P3_BRAKE,
	GA_P3_WHEELIE,
	GA_P3_JUMP,
	GA_P3_RESCUE,
	GA_P3_FIRE,
	GA_P3_LOOK_BACK,
	
	GA_P4_LEFT,
	GA_P4_RIGHT,
	GA_P4_ACCEL,
	GA_P4_BRAKE,
	GA_P4_WHEELIE,
	GA_P4_JUMP,
	GA_P4_RESCUE,
	GA_P4_FIRE,
	GA_P4_LOOK_BACK,

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

/* The range of GameAction constants that is used while in ingame (race) mode. */
const int GA_FIRST_INGAME = GA_P1_LEFT;
const int GA_LAST_INGAME = GA_DEBUG_HISTORY;

/* The range of GameAction constants which are used ingame but are considered
 * fixed and their Inputs should not be used by the players.
 */
const int GA_FIRST_INGAME_FIXED = GA_TOGGLE_FULLSCREEN;
const int GA_LAST_INGAME_FIXED = GA_DEBUG_HISTORY;

/** The range of GameAction constants that defines the mapping
	for the players kart actions. Besides that these are the actions
	whose mappings are changeable by the user (via menu & config file).
	When looking for conflicting mappings only the user changeable
	GameAction constants are regarded.
*/
const int GA_FIRST_KARTACTION = GA_P1_LEFT;
const int GA_LAST_KARTACTION = GA_P4_LOOK_BACK;

#endif
