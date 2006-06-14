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
#include "MenuManager.h"

enum WidgetTokens {
  WTOK_CONTINUE,
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
      order[k->getFinishPosition()-1] = i;
      unsigned int l = strlen(k->getKartProperties()->getName());
      if(l>maxNameLen) maxNameLen = l;
    }   // for i
    
    for(unsigned int i = 0; i < numKarts; ++i)
    {
      Kart *kart = world->getKart(order[i]);
      const char* kartName = kart->getKartProperties()->getName();
      char sTime[20];
      if(i==numKarts-1) {
	sprintf(sTime,"          ");
      } else {
	sprintf(sTime,"%3d:%02d''%02d", kart->getFinishMins(),
		kart->getFinishSecs(), kart->getFinishTenths());
      }
      //This shows position + driver name + time + points earned + total points
      sprintf((char*)(score + MAX_STR_LEN * i), "%d. %s %s +%d %d",
	      kart->getFinishPosition(), kartName, sTime,
	      race_manager->getPositionScore(i+1),
	      race_manager->getKartScore(order[i]));
      widgetSet -> label(menu_id, (char*)(score + MAX_STR_LEN * i), 
			 GUI_MED, GUI_LFT, 0, 0);
    }

    delete order;
    widgetSet -> space(menu_id);

    int va = widgetSet -> varray(menu_id);
    widgetSet -> start(va, "Continue",  GUI_MED, WTOK_CONTINUE, 0);
    
    widgetSet -> layout(menu_id, 0, 1);
}

RaceResultsGUI::~RaceResultsGUI()
{
    widgetSet -> delete_widget(menu_id) ;
    delete score;
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
        case WTOK_CONTINUE:
            menu_manager->pushMenu(MENUID_NEXTRACE);
            widgetSet->tgl_paused();
            break;
        default:
            break;
    }
}
/* EOF */
