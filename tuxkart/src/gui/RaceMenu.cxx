//  $Id: RaceMenu.cxx,v 1.11 2004/09/24 15:45:02 matzebraun Exp $
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

#include "RaceMenu.h"
#include "tuxkart.h"
#include "World.h"
#include "StartScreen.h"
#include "ScreenManager.h"
#include "WidgetSet.h"

RaceMenu::RaceMenu()
{
	menu_id = widgetSet -> vstack(0);
        widgetSet -> label(menu_id, "Paused", GUI_LRG, GUI_ALL, 0, 0);
	int va = widgetSet -> varray(menu_id);
	widgetSet -> start(va, "Return To Race",  GUI_MED, MENU_RETURN, 0);
	widgetSet -> state(va, "Restart Race",  GUI_MED, MENU_RESTART, 0);
	widgetSet -> state(va, "Exit Race",  GUI_MED, MENU_EXIT, 0);
	
	widgetSet -> layout(menu_id, 0, 0);
}

RaceMenu::~RaceMenu()
{
	widgetSet -> delete_widget(menu_id) ;
}
	
void RaceMenu::update(float dt)
{
	widgetSet -> timer(menu_id, dt) ;
	widgetSet -> blank();
	widgetSet -> paint(menu_id) ;
}

void RaceMenu::select()
{
	widgetSet -> tgl_paused();
	
	switch ( widgetSet -> token (widgetSet -> click()) )
	{
	case MENU_RETURN:	
                guiStack.pop_back(); 
                break;

	case MENU_RESTART:
                guiStack.pop_back(); 
                world->restartRace();
                break;

	case MENU_EXIT:	
                guiStack.push_back(GUIS_EXITRACE); 
                break;
                
	default:
                break;
	}
}

void RaceMenu::keybd(const SDL_keysym& key)
{
	switch ( key.sym )
	{
	case SDLK_ESCAPE:
		widgetSet -> tgl_paused();
		guiStack.pop_back();
		break;
		
	default:
		BaseGUI::keybd(key);
		break;
	}
}


