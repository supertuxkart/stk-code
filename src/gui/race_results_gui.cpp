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
#include <SDL/SDL.h>

#include "race_results_gui.hpp"
#include "widget_manager.hpp"
#include "kart_properties.hpp"
#include "world.hpp"
#include "menu_manager.hpp"
#include "race_manager.hpp"
#include "translation.hpp"

enum WidgetTokens {
    WTOK_TITLE,
    WTOK_EMPTY0,
    WTOK_HIGHSCORES,
    WTOK_RESULTS,
    WTOK_EMPTY1,
    WTOK_CONTINUE,
    WTOK_RESTART_RACE,
    WTOK_SETUP_NEW_RACE,
    WTOK_FIRST_RESULT
};

RaceResultsGUI::RaceResultsGUI()
{
    const bool SHOW_RECT = true;
    const bool SHOW_TEXT = true;
    widget_manager->set_initial_rect_state(SHOW_RECT, WGT_AREA_ALL, WGT_TRANS_BLACK);
    widget_manager->set_initial_text_state(SHOW_TEXT, "", WGT_FNT_MED );
    widget_manager->add_wgt(WTOK_TITLE, 60, 7);
    widget_manager->set_wgt_text(WTOK_TITLE, _("Result"));
    widget_manager->break_line();

    widget_manager->add_wgt(WTOK_EMPTY0, 60, 5);
    widget_manager->hide_wgt_rect(WTOK_EMPTY0);
    widget_manager->hide_wgt_text(WTOK_EMPTY0);
    widget_manager->break_line();

    widget_manager->add_wgt(WTOK_RESULTS, 50, 7);
    widget_manager->set_wgt_text(WTOK_RESULTS, _("Race results"));
#if 0
    widget_manager->add_wgt(WTOK_HIGHSCORES, 50, 7);
    widget_manager->set_wgt_text(WTOK_HIGHSCORES, _("Highscores"));
#endif
    widget_manager->break_line();


    const unsigned int MAX_STR_LEN = 60;
    const unsigned int NUM_KARTS = world->getNumKarts();

    int*  order = new int [NUM_KARTS];
    m_score = new char[NUM_KARTS * MAX_STR_LEN];
    unsigned int max_name_len = 1;

    const Highscores *hs = world->getHighscores();
    int num_scores = hs->getNumberEntries();
    m_highscores = new char[num_scores * MAX_STR_LEN];

    for(unsigned int i=0; i < NUM_KARTS; i++)
    {
        Kart *k = world->getKart(i);
        order[k->getPosition()-1] = i;
        const std::string& s = k->getName();
        unsigned int l = s.size();
        if(l>max_name_len) max_name_len = l;
    }   // for i

    for(unsigned int i = 0; i < NUM_KARTS; ++i)
    {
        const Kart *KART = world->getKart(order[i]);
        const std::string& KART_NAME = KART->getName();
        char sTime[20];
        if(i==NUM_KARTS-1 && world->m_race_setup.m_mode!=RaceSetup::RM_TIME_TRIAL)
        {
            sprintf(sTime,"          ");
        }
        else
        {
            const float T      = KART->getFinishTime();
            TimeToString(T, sTime);
        }
        //This shows position + driver name + time + points earned + total points
        if(world->m_race_setup.m_mode==RaceSetup::RM_GRAND_PRIX)
        {
            sprintf((char*)(m_score + MAX_STR_LEN * i), "%d. %s %s +%d %d",
                KART->getPosition(), KART_NAME.c_str(), sTime,
                race_manager->getPositionScore(i+1),
                race_manager->getKartScore(order[i]));
        }
        else
            {
            sprintf((char*)(m_score + MAX_STR_LEN * i), "%d. %s %s",
                KART->getPosition(), KART_NAME.c_str(), sTime);
        }

        widget_manager->add_wgt(WTOK_FIRST_RESULT + i, 50, 7);
        widget_manager->set_wgt_text(WTOK_FIRST_RESULT + i,
            (char*)(m_score + MAX_STR_LEN * i));
        widget_manager->break_line();
    }

    delete[] order;

    for(int i=0; i<num_scores; i++)
    {

        //FIXME:Add highscore table
/*        std::string kart_name, name;
        float T;
        hs->getEntry(i, kart_name, name, &T);
        const int   MINS   = (int) floor ( T / 60.0 ) ;
        const int   SECS   = (int) floor ( T - (float) ( 60 * MINS ) ) ;
        const int   TENTHS = (int) floor ( 10.0f * (T - (float)(SECS + 60*MINS)));
        sprintf((char*)(m_highscores + MAX_STR_LEN * i), 
                "%s: %3d:%02d.%01d", name.c_str(), MINS, SECS, TENTHS);
        widget_manager->add_wgt(WTOK_FIRST_HIGHSCORE + i, 40, 7);
        widget_manager->set_wgt_text(WTOK_FIRST_HIGHSCORE + i,
            (char*)(m_highscores+MAX_STR_LEN*i));*/

    }
    widget_manager->add_wgt(WTOK_EMPTY1, 60, 5);
    widget_manager->hide_wgt_rect(WTOK_EMPTY1);
    widget_manager->hide_wgt_text(WTOK_EMPTY1);
    widget_manager->break_line();

    widget_manager->set_initial_activation_state(true);
    widget_manager->add_wgt( WTOK_CONTINUE, 60, 7);
    if(world->m_race_setup.m_mode==RaceSetup::RM_GRAND_PRIX)
    {
        widget_manager->set_wgt_text( WTOK_CONTINUE, _("Continue Grand Prix"));
    }
    else
    {
        widget_manager->set_wgt_text( WTOK_CONTINUE, _("Back to the main menu"));
    }
    widget_manager->break_line();

    widget_manager->add_wgt( WTOK_RESTART_RACE, 60, 7);
    widget_manager->set_wgt_text( WTOK_RESTART_RACE, _("Race in this track again"));
    widget_manager->break_line();

    if(world->m_race_setup.m_mode==RaceSetup::RM_QUICK_RACE)
    {
        widget_manager->add_wgt( WTOK_SETUP_NEW_RACE, 60, 7);
        widget_manager->set_wgt_text( WTOK_SETUP_NEW_RACE, _("Setup New Race"));
    }

    widget_manager->layout(WGT_AREA_ALL);
}  // RaceResultsGUI

//-----------------------------------------------------------------------------
RaceResultsGUI::~RaceResultsGUI()
{
    widget_manager->reset();
    //widgetSet -> delete_widget(m_menu_id) ;
    delete[] m_score;
    delete[] m_highscores;
}   // ~RaceResultsGUI

//-----------------------------------------------------------------------------
void RaceResultsGUI::select()
{
    switch( widget_manager->get_selected_wgt() )
    {
    case WTOK_CONTINUE:
        world->unpause();
        race_manager->next();
        break;
    case WTOK_RESTART_RACE:
        world->unpause();
        menu_manager->popMenu();
        // TODO: Maybe let this go through the race_manager for
        // more flexibility.
        world->restartRace();
        break;
    case WTOK_SETUP_NEW_RACE:
        world->unpause();
        race_manager->exit_race();
        menu_manager->pushMenu(MENUID_DIFFICULTY);
        break;

    default:
        break;
    }
}   // select
//-----------------------------------------------------------------------------
void
RaceResultsGUI::handle(GameAction ga, int value)
{
  // Attempts to close the menu are silently discarded
  // since they do not make sense at this point.
  if (ga == GA_LEAVE)
   return;
  else
    BaseGUI::handle(ga, value);
}

/* EOF */
