//  $Id: RaceResultsGUI.cxx,v 1.1 2005/05/25 21:47:54 joh Exp $
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

#include <iostream>
#include "RaceResultsGUI.h"
#include "RaceManager.h"
#include "WidgetSet.h"
#include "KartProperties.h"
#include "World.h"

RaceResultsGUI::RaceResultsGUI()
{
	menu_id = widgetSet -> vstack(0);
    const unsigned int MAX_STR_LEN = 60;
    widgetSet -> label(menu_id, "Race results", GUI_LRG, GUI_ALL, 0, 0);

	static char score[MAX_STR_LEN][6];
	for(unsigned int i = 0; i < world->raceSetup.getNumKarts(); ++i)
    {
    //This shows position + driver name + time + points earned + total points
        sprintf((char*)(score + MAX_STR_LEN * i), "%d. %s %d:%d''%d +%d %d",
            world->getKart(i)->getFinishPosition(),
            world->getKart(i)->getKartProperties()->name.c_str(),
            world->getKart(i)->getFinishMins(),
            world->getKart(i)->getFinishSecs(),
            world->getKart(i)->getFinishTenths(),
            world->getKart(i)->getFinishPosition() > 4 ? 0 :
                4 - world->getKart(i)->getFinishPosition(),
            race_manager->getKartScore(i));

        widgetSet -> label(menu_id, (char*)(score + MAX_STR_LEN * i), GUI_MED, GUI_LFT, 0, 0);
    }
	widgetSet -> space(menu_id);

    int va = widgetSet -> varray(menu_id);
	widgetSet -> start(va, "Continue",  GUI_MED, MENU_CONTINUE, 0);

	widgetSet -> layout(menu_id, 0, 1);
}

RaceResultsGUI::~RaceResultsGUI()
{
	widgetSet -> delete_widget(menu_id) ;
}

void RaceResultsGUI::update(float dt)
{
	widgetSet -> timer(menu_id, dt) ;
	widgetSet -> paint(menu_id) ;
}

void RaceResultsGUI::select()
{
    switch( widgetSet->token( widgetSet->click() ) )
    {
        case MENU_CONTINUE:
            guiStack.push_back(GUIS_NEXTRACE);
            break;
        default:
            break;
    }
}
/* EOF */
