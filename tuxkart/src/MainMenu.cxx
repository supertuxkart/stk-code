//  $Id: MainMenu.cxx,v 1.4 2004/08/05 18:33:00 jamesgregory Exp $
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

#include "MainMenu.h"
#include "tuxkart.h"
#include "WidgetSet.h"

enum MenuOption {MENU_SINGLE, MENU_MULTI, MENU_REPLAY, MENU_OPTIONS, MENU_QUIT } ;

MainMenu::MainMenu()
{
	menu_id = widgetSet -> varray(0);
	widgetSet -> start(menu_id, "Single Player",  GUI_SML, MENU_SINGLE, 0);
	widgetSet -> state(menu_id, "Multiplayer",  GUI_SML, MENU_MULTI, 0);
	widgetSet -> state(menu_id, "Watch Replay",  GUI_SML, MENU_REPLAY, 0);
	widgetSet -> state(menu_id, "Options",  GUI_SML, MENU_OPTIONS, 0);
	widgetSet -> state(menu_id, "Quit",  GUI_SML, MENU_QUIT, 0);
	
	widgetSet -> layout(menu_id, 0, 0);
}

MainMenu::~MainMenu()
{
	widgetSet -> delete_widget(menu_id) ;
}
	
void MainMenu::update(float dt)
{
	
	widgetSet -> timer(menu_id, dt) ;
	widgetSet -> paint(menu_id) ;
}

void MainMenu::select()
{
	switch ( widgetSet -> token (widgetSet -> click()) )
	{
	case MENU_SINGLE:		singlePlayer();	break;
	case MENU_MULTI: 		break;
	case MENU_REPLAY:		break;
	case MENU_OPTIONS:	break;
	case MENU_QUIT:		shutdown(); break;
	}
}

void MainMenu::cursor(SDLKey key)
{
	widgetSet -> pulse(widgetSet -> cursor(menu_id, key), 1.2f);
}

void MainMenu::point(int x, int y)
{
	widgetSet -> pulse(widgetSet -> point(menu_id, x, y), 1.2f);
}

void MainMenu::stick(int x, int y)
{
	widgetSet -> pulse(widgetSet -> stick(menu_id, x, y), 1.2f);
}

void MainMenu::singlePlayer()
{
	switchToGame () ;
}
