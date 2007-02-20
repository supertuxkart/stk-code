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
    const unsigned int MAX_STR_LEN = 60;
    widgetSet -> label(m_menu_id, _("Race results"), GUI_LRG, GUI_ALL, 0, 0);

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
            const int   MINS   = (int) floor ( T / 60.0 ) ;
            const int   SECS   = (int) floor ( T - (float) ( 60 * MINS ) ) ;
            const int   TENTHS = (int) floor ( 10.0f * (T - (float)(SECS + 60*MINS)));
            sprintf(sTime,"%3d:%02d.%01d", MINS, SECS, TENTHS);
        }
        //This shows position + driver name + time + points earned + total points
        sprintf((char*)(m_score + MAX_STR_LEN * i), "%d. %s %s +%d %d",
                KART->getPosition(), KART_NAME.c_str(), sTime,
                race_manager->getPositionScore(i+1),
                race_manager->getKartScore(order[i]));
        widgetSet -> label(m_menu_id, (char*)(m_score + MAX_STR_LEN * i),
                           GUI_MED, GUI_ALL);
    }

    delete[] order;
    widgetSet -> space(m_menu_id);

    const int VA = widgetSet -> varray(m_menu_id);
    widgetSet -> start(VA, _("Back to the main menu"),  GUI_MED, WTOK_CONTINUE);
    widgetSet -> start(VA, _("Race in this track again"),  GUI_MED, WTOK_RESTART_RACE);
    if(world->m_race_setup.m_mode==RaceSetup::RM_QUICK_RACE)
    {
        widgetSet -> start(VA, _("Setup New Race"),  GUI_MED, WTOK_SETUP_NEW_RACE);
    }

    widgetSet -> layout(m_menu_id, 0, 1);
}

//-----------------------------------------------------------------------------
RaceResultsGUI::~RaceResultsGUI()
{
    widgetSet -> delete_widget(m_menu_id) ;
    delete[] m_score;
}

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
}


//-----------------------------------------------------------------------------
void RaceResultsGUI::input(InputType type, int id0, int  id1, int id2, int value)
{
    switch (type)
    {
    case IT_KEYBOARD:
        inputKeyboard(id0, value);
        break;
    case IT_MOUSEMOTION:
        if (id0 == 1 && value)
            if (id1 == AD_NEGATIVE)
                inputKeyboard(SDLK_UP, 1);
            else
                inputKeyboard(SDLK_DOWN, 1);
        break;
    case IT_STICKMOTION:
        if(widgetSet)
            widgetSet -> pulse(widgetSet -> stick(m_menu_id, id1, id2, value), 1.2f);
        break;
    case IT_STICKBUTTON:
        if( value)
            switch (id1) // Button no
            {
            case 0:
                select();
                break;

            case 1:   //Usually here would be code to close this gui. Not only
                      //that doesn't has any real function in this gui,
                      //but also closing this gui causes bug #9157.
                break;
            }
        break;
    default:
        break;
    }

}

//-----------------------------------------------------------------------------
void RaceResultsGUI::inputKeyboard(int key, int pressed)
{
    if (!pressed)
        return;

    switch ( key )
    {
    case SDLK_LEFT:
    case SDLK_RIGHT:
    case SDLK_UP:
    case SDLK_DOWN:
        widgetSet->pulse(widgetSet->cursor(m_menu_id, key), 1.2f);
        break;
    case SDLK_SPACE:
    case SDLK_RETURN:
        select();
        break;

    case SDLK_ESCAPE: //Usually here would be code to close this gui. Not only
                      //that doesn't has any real function in this gui,
                      //but also causes bug #9157.
        break;

    default:
        break;
    }   // switch
}   // inputKeyboard

/* EOF */
