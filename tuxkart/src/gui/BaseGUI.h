//  $Id: BaseGUI.h,v 1.16 2004/10/11 13:40:07 jamesgregory Exp $
//
//  TuxKart - a fun racing game with go-kart
//  Copyright (C) 2004 Steve Baker <sjbaker1@airmail.net>
//
//  This program is free software; you can redistribute it and/or
//  modify it under the terms of the GNU General Public License
//  as published by the Free Software Foundation; either version 2
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

#ifndef HEADER_BASEGUI_H
#define HEADER_BASEGUI_H

#include <SDL.h> //this should be "SDL.h" for portability?

class RaceSetup;

enum GUISwitch
{
GUIS_MAINMENU,
GUIS_CHARSEL,
GUIS_CHARSELP2,
GUIS_CHARSELP3,
GUIS_CHARSELP4,
GUIS_DIFFICULTYGP,
GUIS_DIFFICULTYSR,
GUIS_GAMEMODE,
GUIS_OPTIONS,
GUIS_CONFIGCONTROLS,
GUIS_CONFIGP1,
GUIS_CONFIGP2,
GUIS_CONFIGP3,
GUIS_CONFIGP4,
GUIS_TRACKSEL,
GUIS_NUMPLAYERS,
GUIS_RACE,
GUIS_RACEMENU,
GUIS_EXITRACE
};

enum MenuOption {
  MENU_SINGLE, MENU_MULTI, MENU_REPLAY, MENU_OPTIONS, MENU_QUIT,
  MENU_GP, MENU_QUICKRACE, MENU_TIMETRIAL,
  MENU_TRACK,
  MENU_EASY, MENU_MEDIUM, MENU_HARD,
  MENU_NETWORK,
  MENU_CONTROLS,
  MENU_RETURN, MENU_RESTART, MENU_EXIT,
} ;

void updateGUI();

class BaseGUI
{
public:
	BaseGUI() {}
	virtual ~BaseGUI() {}
	
	virtual void update(float dt) = 0;
	virtual void select() = 0;
	virtual void keybd(const SDL_keysym& key);
	virtual void point(int x, int y); 
	virtual void stick(int whichAxis, int value);
      virtual void joybutton(int whichJoy, int button);
	
protected:
	int menu_id;
};

#endif
