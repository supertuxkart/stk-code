//  $Id: BaseGUI.h,v 1.2 2004/08/06 00:37:41 jamesgregory Exp $
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

#include <SDL.h>

enum GUISwitch
{
GUIS_CURRENT,
GUIS_MAINMENU,
GUIS_CHARSEL,
GUIS_DIFFICULTY,
GUIS_GAMEMODE,
GUIS_OPTIONS,
GUIS_TRACKSEL,
GUIS_RACE
};

enum MenuOption {MENU_SINGLE, MENU_MULTI, MENU_REPLAY, MENU_OPTIONS, MENU_QUIT,
MENU_QUICKRACE,
MENU_TRACK,
MENU_EASY, MENU_MEDIUM, MENU_HARD

} ;

void updateGUI();

class BaseGUI
{
public:
	BaseGUI() {}
	virtual ~BaseGUI() {}
	
	virtual void update(float dt) = 0;
	virtual void select() = 0;
	virtual void keybd(const SDL_keysym& key) = 0;
	virtual void point(int x, int y) = 0; 
	virtual void stick(int x, int y) = 0;
};

#endif
