//  $Id$
//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2006 SuperTuxKart-Team
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

#include "race_menu.hpp"
#include "world.hpp"
#include "widget_set.hpp"

#include "menu_manager.hpp"
#include "race_manager.hpp"


enum WidgetTokens {
  WTOK_RETURN_RACE,
  WTOK_OPTIONS,
  WTOK_RESTART_RACE,
  WTOK_EXIT_RACE,
};

RaceMenu::RaceMenu()
{
  menu_id = widgetSet -> vstack(0);
  widgetSet -> label(menu_id, "Paused", GUI_LRG, GUI_ALL, 0, 0);
	
  int va = widgetSet -> varray(menu_id);
  widgetSet -> start(va, "Return To Race",  GUI_MED, WTOK_RETURN_RACE, 0);
  widgetSet -> state(va, "Options",         GUI_MED, WTOK_OPTIONS, 0);
  widgetSet -> state(va, "Restart Race",    GUI_MED, WTOK_RESTART_RACE, 0);
  widgetSet -> state(va, "Exit Race",       GUI_MED, WTOK_EXIT_RACE, 0);
	
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
  int clicked_token = widgetSet->token(widgetSet->click());
  if(clicked_token != WTOK_OPTIONS)
	  widgetSet -> tgl_paused();
	
	switch (clicked_token)
	{
	case WTOK_RETURN_RACE:	
                menu_manager->popMenu(); 
                break;

	case WTOK_RESTART_RACE:
                menu_manager->popMenu(); 
                world->restartRace();
                break;

	case WTOK_OPTIONS:
                menu_manager->pushMenu(MENUID_OPTIONS);	
                break;

	case WTOK_EXIT_RACE:	
                race_manager->exit_race();
                break;
                
	default:
                break;
	}
}

void RaceMenu::keybd(int key)
{
	switch ( key )
	{
	case 27: //ESC
		widgetSet -> tgl_paused();
		menu_manager->popMenu();
		break;
		
	default:
		BaseGUI::keybd(key);
		break;
	}
}


