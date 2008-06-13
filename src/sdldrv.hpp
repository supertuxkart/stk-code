//  $Id: sdldrv.hpp 694 2006-08-29 07:42:36Z hiker $
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

#ifndef HEADER_SDLDRV_H
#define HEADER_SDLDRV_H

#include <string>
#include <vector>

#include <SDL/SDL.h>

#include "input.hpp"

class ActionMap;

enum MouseState { INITIAL, MOVED, RESET_NEEDED }; 

enum InputDriverMode {
	MENU = 0,
	INGAME,
	INPUT_SENSE,
	LOWLEVEL,
	BOOTSTRAP
};

class SDLDriver
{
	class StickInfo {
	public:
		SDL_Joystick *sdlJoystick;

		std::string *id;
				
		int deadzone;
		
		int index;
		
		AxisDirection *prevAxisDirections;

		StickInfo(int);
				
		~StickInfo();
	};

	Input *sensedInput;
	ActionMap *actionMap;
	
	SDL_Surface *mainSurface;
	long flags;

	StickInfo **stickInfos;
		
	InputDriverMode mode;
	
	/* Helper values to store and track the relative mouse movements. If these
	* values exceed the deadzone value the input is reported to the game. This
  	* makes the mouse behave like an analog axis on a gamepad/joystick.
	*/
	int mouseValX;
	int mouseValY;
	
	void showPointer();
	
	void hidePointer();
	
	void input(InputType, int, int, int, int);
	
public:
	SDLDriver();
	~SDLDriver();
			
	void initStickInfos();

	void toggleFullscreen(bool resetTextures=1);
		
	void setVideoMode(bool resetTextures=1);

	void input();

	void setMode(InputDriverMode);
		
	bool isInMode(InputDriverMode);

	Input &getSensedInput();
};

extern SDLDriver *inputDriver;

#endif
