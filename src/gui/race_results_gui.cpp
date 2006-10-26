//  $Id$
//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2006 Eduardo Hernandez Munoz
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

#include "race_results_gui.hpp"
#include "widget_set.hpp"
#include "kart_properties.hpp"
#include "world.hpp"
#include "menu_manager.hpp"
#include "race_manager.hpp"

enum WidgetTokens {
  WTOK_CONTINUE,
  WTOK_RESTART_RACE,
  WTOK_SETUP_NEW_RACE,
};

RaceResultsGUI::RaceResultsGUI()
{
    menu_id = widgetSet -> vstack(0);
    const unsigned int MAX_STR_LEN = 60;
    widgetSet -> label(menu_id, "Race results", GUI_LRG, GUI_ALL, 0, 0);

    unsigned int numKarts = world->getNumKarts();
    int*  order;
    score = new char[numKarts * MAX_STR_LEN];
    order = new int [numKarts              ];
    unsigned int maxNameLen = 1;
    for(unsigned int i=0; i<numKarts; i++) {
      Kart *k = world->getKart(i);
      order[k->getPosition()-1] = i;
      const std::string& s = k->getName();
      unsigned int l = s.size();
      if(l>maxNameLen) maxNameLen = l;
    }   // for i
    
    for(unsigned int i = 0; i < numKarts; ++i)
    {
      Kart *kart = world->getKart(order[i]);
      const std::string& kartName = kart->getName();
      char sTime[20];
      if(i==numKarts-1) {
	sprintf(sTime,"          ");
      } else {
	float t      = kart->getFinishTime();
	int   mins   = (int) floor ( t / 60.0 ) ;
	int   secs   = (int) floor ( t - (float) ( 60 * mins ) ) ;
	int   tenths = (int) floor ( 10.0f * (t - (float)(secs + 60*mins)));
	sprintf(sTime,"%3d:%02d.%01d", mins, secs, tenths);
      }
      //This shows position + driver name + time + points earned + total points
      sprintf((char*)(score + MAX_STR_LEN * i), "%d. %s %s +%d %d",
              kart->getPosition(), kartName.c_str(), sTime,
              race_manager->getPositionScore(i+1),
              race_manager->getKartScore(order[i]));
      widgetSet -> label(menu_id, (char*)(score + MAX_STR_LEN * i), 
			 GUI_MED, GUI_LFT, 0, 0);
    }

    delete[] order;
    widgetSet -> space(menu_id);

    int va = widgetSet -> varray(menu_id);
    widgetSet -> start(va, "Continue",  GUI_MED, WTOK_CONTINUE);
    widgetSet -> start(va, "Restart Race",  GUI_MED, WTOK_RESTART_RACE);
    if(world->raceSetup.mode==RaceSetup::RM_QUICK_RACE) {
      widgetSet -> start(va, "Setup New Race",  GUI_MED, WTOK_SETUP_NEW_RACE);
    }
    
    widgetSet -> layout(menu_id, 0, 1);
}

RaceResultsGUI::~RaceResultsGUI()
{
    widgetSet -> delete_widget(menu_id) ;
    delete[] score;
}

void RaceResultsGUI::select()
{
    switch( widgetSet->token( widgetSet->click() ) )
    {
        case WTOK_CONTINUE:
            widgetSet->tgl_paused();
            race_manager->next();
            break;
        case WTOK_RESTART_RACE:
            widgetSet->tgl_paused();
            menu_manager->popMenu();
            // TODO: Maybe let this go through the race_manager for
            // more flexibility.
            world->restartRace();
            break;
        case WTOK_SETUP_NEW_RACE:
            widgetSet->tgl_paused();
            race_manager->exit_race();
            menu_manager->pushMenu(MENUID_DIFFICULTY);
            break;

        default:
            break;
    }
}
/* EOF */
