//  $Id: Difficulty.cxx,v 1.2 2004/08/06 00:37:41 jamesgregory Exp $
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

#include "Difficulty.h"
#include "tuxkart.h"
#include "WidgetSet.h"

Difficulty::Difficulty()
{
	menu_id = widgetSet -> varray(0);
	widgetSet -> start(menu_id, "Easy",  GUI_SML, MENU_EASY, 0);
	widgetSet -> state(menu_id, "Medium",  GUI_SML, MENU_MEDIUM, 0);
	widgetSet -> state(menu_id, "Hard",  GUI_SML, MENU_HARD, 0);
	widgetSet -> state(menu_id, "If not GP, Num Laps",  GUI_SML, 0, 0);
	widgetSet -> state(menu_id, "If not GP, Reverse Track",  GUI_SML, 0, 0);
	#ifdef SSG_BACKFACE_COLLISIONS_SUPPORTED
	widgetSet -> state(menu_id, "If not GP, Mirror Track",  GUI_SML, 0, 0);
	#endif
	widgetSet -> space(menu_id);
	widgetSet -> space(menu_id);
	
	widgetSet -> layout(menu_id, 0, -1);
}

Difficulty::~Difficulty()
{
	widgetSet -> delete_widget(menu_id) ;
}
	
void Difficulty::update(float dt)
{
	
	widgetSet -> timer(menu_id, dt) ;
	widgetSet -> paint(menu_id) ;
}

void Difficulty::select()
{
	switch ( widgetSet -> token (widgetSet -> click()) )
	{
	case MENU_EASY:	guiSwitch = GUIS_TRACKSEL;	break;
	case MENU_MEDIUM:	guiSwitch = GUIS_TRACKSEL;	break;
	case MENU_HARD:	guiSwitch = GUIS_TRACKSEL;	break;
	default: break;
	}
}

void Difficulty::keybd(const SDL_keysym& key)
{
	switch ( key.sym )
	{
	case SDLK_LEFT:    
	case SDLK_RIGHT:    
	case SDLK_UP:    
	case SDLK_DOWN:
		widgetSet -> pulse(widgetSet -> cursor(menu_id, key.sym), 1.2f);
		break;
		
	case SDLK_RETURN: select(); break;
	
	case SDLK_ESCAPE:
		guiSwitch = GUIS_MAINMENU;
		
	default: break;
	}
}

void Difficulty::point(int x, int y)
{
	widgetSet -> pulse(widgetSet -> point(menu_id, x, y), 1.2f);
}

void Difficulty::stick(int x, int y)
{
	widgetSet -> pulse(widgetSet -> stick(menu_id, x, y), 1.2f);
}

