//  $Id: GameMode.cxx,v 1.2 2005/05/27 10:25:52 joh Exp $
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

#include "game_mode.hpp"
#include "widget_set.hpp"
#include "race_manager.hpp"
#include "menu_manager.hpp"

enum WidgetTokens {
  WTOK_GP,
  WTOK_QUICKRACE,
  WTOK_TIMETRIAL,
  WTOK_BACK
};

GameMode::GameMode() {
  menu_id = widgetSet -> vstack(0);
  
  widgetSet -> label(menu_id, "Choose a Race Mode", GUI_LRG);
  
  int va = widgetSet -> varray(menu_id);
  widgetSet -> space(menu_id);
  widgetSet -> start(va, "Grand Prix",  GUI_MED, WTOK_GP);
  widgetSet -> state(va, "Quick Race",  GUI_MED, WTOK_QUICKRACE);
  
  if (race_manager->getNumPlayers() == 1)
    widgetSet -> state(va, "Time Trial",  GUI_MED, WTOK_TIMETRIAL);
  
  widgetSet -> space(menu_id);
  widgetSet -> state(menu_id,"Press <ESC> to go back", GUI_SML, WTOK_BACK);
  widgetSet -> space(menu_id);
  widgetSet -> layout(menu_id, 0, 0);
}

GameMode::~GameMode() {
	widgetSet -> delete_widget(menu_id) ;
}
	
void GameMode::select()
{
	switch ( widgetSet -> token (widgetSet -> click()) )
	{
	case WTOK_GP:
		 race_manager->setRaceMode(RaceSetup::RM_GRAND_PRIX);
		 menu_manager->pushMenu(MENUID_DIFFICULTY);
		 break;
	case WTOK_QUICKRACE:
		 race_manager->setRaceMode(RaceSetup::RM_QUICK_RACE);
		 menu_manager->pushMenu(MENUID_DIFFICULTY);
		 break;
	case WTOK_TIMETRIAL:
		 race_manager->setRaceMode(RaceSetup::RM_TIME_TRIAL);
		 menu_manager->pushMenu(MENUID_CHARSEL_P1); //difficulty makes no sense here
		 break;
	case WTOK_BACK:
                 menu_manager->popMenu();
		 break;
	default: break;
	}
}



