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

#include "GameMode.h"
#include "WidgetSet.h"
#include "RaceManager.h"

GameMode::GameMode()
{
	menu_id = widgetSet -> vstack(0);

	widgetSet -> label(menu_id, "Choose a Race Mode", GUI_LRG, GUI_ALL, 0, 0);

	int va = widgetSet -> varray(menu_id);
	widgetSet -> start(va, "Grand Prix",  GUI_MED, MENU_GP, 0);
	widgetSet -> state(va, "Quick Race",  GUI_MED, MENU_QUICKRACE, 0);
	
	if (race_manager->getNumPlayers() == 1)
		widgetSet -> state(va, "Time Trial",  GUI_MED, MENU_TIMETRIAL, 0);
	
	widgetSet -> layout(menu_id, 0, 0);
}

GameMode::~GameMode()
{
	widgetSet -> delete_widget(menu_id) ;
}
	
void GameMode::update(float dt)
{
	
	widgetSet -> timer(menu_id, dt) ;
	widgetSet -> paint(menu_id) ;
}

void GameMode::select()
{
	switch ( widgetSet -> token (widgetSet -> click()) )
	{
	case MENU_GP:
		race_manager->setRaceMode(RaceSetup::RM_GRAND_PRIX);
		guiStack.push_back(GUIS_DIFFICULTY);
		break;
	case MENU_QUICKRACE:
		race_manager->setRaceMode(RaceSetup::RM_QUICK_RACE);
		guiStack.push_back(GUIS_DIFFICULTY);
		break;
	case MENU_TIMETRIAL:
		race_manager->setRaceMode(RaceSetup::RM_TIME_TRIAL);
		guiStack.push_back(GUIS_CHARSEL); //difficulty makes no sense here
		break;
	default: break;
	}
}



