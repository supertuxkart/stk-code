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
#include "widget_set.hpp"
#include "kart_properties.hpp"
#include "world.hpp"
#include "menu_manager.hpp"
#include "race_manager.hpp"
#include "translation.hpp"

enum WidgetTokens {
    WTOK_CONTINUE,
    WTOK_RESTART_RACE,
    WTOK_SETUP_NEW_RACE,
};

RaceResultsGUI::RaceResultsGUI()
{
    m_menu_id = widgetSet -> vstack(0);
    widgetSet -> label(m_menu_id, _("Result"), GUI_LRG, GUI_ALL, 0, 0);
    const unsigned int MAX_STR_LEN = 60;
    widgetSet -> space(m_menu_id);

    const int HA = widgetSet->harray(m_menu_id);
    const int HIGHSCORE_TABLE = widgetSet->varray(HA);
    const int RESULT_TABLE    = widgetSet->varray(HA);
    widgetSet -> label(RESULT_TABLE,    _("Race results"),GUI_LRG,GUI_ALL,0,0);
    widgetSet -> label(HIGHSCORE_TABLE, _("Highscores"),  GUI_LRG,GUI_ALL,0,0);
    const unsigned int NUM_KARTS = world->getNumKarts();
    int*  order = new int [NUM_KARTS];
    m_score = new char[NUM_KARTS * MAX_STR_LEN];
    unsigned int max_name_len = 1;

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
        if(i==NUM_KARTS-1)
        {
            sprintf(sTime,"          ");
        }
        else
        {
            const float T      = KART->getFinishTime();
            TimeToString(T, sTime);
        }
        //This shows position + driver name + time + points earned + total points
        sprintf((char*)(m_score + MAX_STR_LEN * i), "%d. %s %s +%d %d",
                KART->getPosition(), KART_NAME.c_str(), sTime,
                race_manager->getPositionScore(i+1),
                race_manager->getKartScore(order[i]));
        widgetSet -> label(RESULT_TABLE, (char*)(m_score + MAX_STR_LEN * i),
                           GUI_MED, GUI_ALL);
    }

    delete[] order;

    const Highscores *hs = world->getHighscores();
    int num_scores = hs->getNumberEntries();
    m_highscores = new char[num_scores * MAX_STR_LEN];
    for(int i=0; i<num_scores; i++)
    {
        std::string kart_name, name;
        float T;
        hs->getEntry(i, kart_name, name, &T);
        const int   MINS   = (int) floor ( T / 60.0 ) ;
        const int   SECS   = (int) floor ( T - (float) ( 60 * MINS ) ) ;
        const int   TENTHS = (int) floor ( 10.0f * (T - (float)(SECS + 60*MINS)));
        sprintf((char*)(m_highscores + MAX_STR_LEN * i), 
                "%s: %3d:%02d.%01d", name.c_str(), MINS, SECS, TENTHS);
        widgetSet->label(HIGHSCORE_TABLE, (char*)(m_highscores+MAX_STR_LEN*i),
                         GUI_MED, GUI_ALL);
        
    }
    widgetSet -> space(m_menu_id);

    //    const int VA = widgetSet -> varray(m_menu_id);
    
    if(world->m_race_setup.m_mode==RaceSetup::RM_GRAND_PRIX)
    {
      widgetSet -> start(m_menu_id, _("Continue Grand Prix"),  GUI_MED, WTOK_CONTINUE);
    }
    else
    {
      widgetSet -> start(m_menu_id, _("Back to the main menu"),  GUI_MED, WTOK_CONTINUE);
    }
    widgetSet -> start(m_menu_id, _("Race in this track again"),  GUI_MED, WTOK_RESTART_RACE);
    if(world->m_race_setup.m_mode==RaceSetup::RM_QUICK_RACE)
    {
        widgetSet -> start(m_menu_id, _("Setup New Race"),  GUI_MED, WTOK_SETUP_NEW_RACE);
    }

    widgetSet -> layout(m_menu_id, 0, 0);
}  // RaceResultsGUI

//-----------------------------------------------------------------------------
RaceResultsGUI::~RaceResultsGUI()
{
    widgetSet -> delete_widget(m_menu_id) ;
    delete[] m_score;
    delete[] m_highscores;
}   // ~RaceResultsGUI

//-----------------------------------------------------------------------------
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
}   // select


//-----------------------------------------------------------------------------
void RaceResultsGUI::input(InputType type, int id0, int  id1, int id2, int value)
{
    if( (type==IT_STICKBUTTON && value && id1==1               ) ||
        (type==IT_MOUSEBUTTON && value && id0==SDL_BUTTON_RIGHT)    )
    {    // Usually here would be code to close this gui. Not only
         // that doesn't has any real function in this gui,
         // but also closing this gui causes bug #9157.
        widgetSet->tgl_paused();
        race_manager->next();
    }
    else
    {
        BaseGUI::input(type, id0, id1, id2, value);
    }

}   // input

//-----------------------------------------------------------------------------
void RaceResultsGUI::inputKeyboard(int key, int pressed)
{
    if (!pressed)
        return;

    if(key!=SDLK_ESCAPE)
    {
        BaseGUI::inputKeyboard(key, pressed);
    }
    else
    {   // Usually here would be code to close this gui. Not only
        // that doesn't has any real function in this gui,
        // but also causes bug #9157.
        widgetSet->tgl_paused();
        race_manager->next();
    }   // sif SDLK_ESCAPE
}   // inputKeyboard

/* EOF */
