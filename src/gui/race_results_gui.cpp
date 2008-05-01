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
#include "highscore_manager.hpp"
#include "unlock_manager.hpp"
#include "translation.hpp"

enum WidgetTokens
{
    WTOK_TITLE,
    WTOK_HIGHSCORES,
    WTOK_RESULTS,
    WTOK_CONTINUE,
    WTOK_RESTART_RACE,
    WTOK_SETUP_NEW_RACE,
    WTOK_FIRST_HIGHSCORE,
    //Add 3 because the maximum number of highscores is 3
    WTOK_FIRST_RESULT = WTOK_FIRST_HIGHSCORE + 3
};

RaceResultsGUI::RaceResultsGUI()
{
    widget_manager->addTitleWgt( WTOK_TITLE, 60, 7, _("Result") );
    widget_manager->breakLine();

    widget_manager->addEmptyWgt( WidgetManager::WGT_NONE, 60, 5 );
    widget_manager->breakLine();

    widget_manager->addTextWgt( WTOK_RESULTS, 50, 7, _("Race results") );
    widget_manager->addTextWgt( WTOK_HIGHSCORES, 50, 7, _("Highscores") );
    widget_manager->breakLine();

    widget_manager->insertColumn();

    const unsigned int MAX_STR_LEN = 60;
    const unsigned int NUM_KARTS = race_manager->getNumKarts();

    int*  order = new int [NUM_KARTS];
    m_score = new char[NUM_KARTS * MAX_STR_LEN];
    unsigned int max_name_len = 1;

    for(unsigned int i=0; i < NUM_KARTS; i++)
    {
        Kart *k = world->getKart(i);             // Display even for eliminated karts!
        order[k->getPosition()-1] = i;
        const std::string& s = k->getName();
        unsigned int l = (unsigned int)s.size();
        if(l>max_name_len) max_name_len = l;
    }   // for i

    for(unsigned int i = 0; i < NUM_KARTS; ++i)
    {
        const Kart *KART = world->getKart(order[i]);
        const std::string& KART_NAME = KART->getName();
        char sTime[20];
        const float T      = KART->getFinishTime();
        TimeToString(T, sTime);

        //This shows position + driver name + time + points earned + total points
        if(race_manager->getRaceMode()==RaceManager::RM_GRAND_PRIX)
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

        widget_manager->addTextWgt( WTOK_FIRST_RESULT + i, 50, 7,
            (char*)(m_score + MAX_STR_LEN * i) );
    }

    delete[] order;

    widget_manager->breakLine();
    widget_manager->insertColumn();

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
        sprintf((char*)( m_highscores + MAX_STR_LEN * i ),
                "%s: %3d:%02d.%01d", name.c_str(), MINS, SECS, TENTHS);
        widget_manager->addTextWgt( WTOK_FIRST_HIGHSCORE + i, 50, 7,
            (char*)( m_highscores+MAX_STR_LEN*i ) );
    }
    widget_manager->breakLine();
    widget_manager->breakLine();

    widget_manager->addEmptyWgt( WidgetManager::WGT_NONE, 60, 5 );
    widget_manager->breakLine();

    // If a new feature was unlocked, only offer 'continue' otherwise add the 
    // full menu choices. The new feature menu returns to this menu, and will
    // then display the whole menu.
    if(unlock_manager->getUnlockedFeatures().size()>0)
    {
        widget_manager->addTextButtonWgt( WTOK_CONTINUE, 60, 7, _("Continue") );
    } else
    {
        if(race_manager->getRaceMode()==RaceManager::RM_GRAND_PRIX)
        {
            widget_manager->addTextButtonWgt( WTOK_CONTINUE, 60, 7, _("Continue Grand Prix"));
        }
        else
        {
            widget_manager->addTextButtonWgt( WTOK_CONTINUE, 60, 7, _("Back to the main menu"));
        }
        widget_manager->breakLine();

        widget_manager->addTextButtonWgt( WTOK_RESTART_RACE, 60, 7, _("Race in this track again"));
        widget_manager->breakLine();

        if(race_manager->getRaceMode()==RaceManager::RM_QUICK_RACE)
        {
            widget_manager->addTextButtonWgt( WTOK_SETUP_NEW_RACE, 60, 7, _("Setup New Race"));
        }
    }   // if !unlock_manager has something unlocked*/

    widget_manager->layout(WGT_AREA_ALL);
}  // RaceResultsGUI

//-----------------------------------------------------------------------------
RaceResultsGUI::~RaceResultsGUI()
{
    widget_manager->reset();
    delete[] m_score;
    delete[] m_highscores;
}   // ~RaceResultsGUI

//-----------------------------------------------------------------------------
void RaceResultsGUI::select()
{
    // Push the unlocked-feature menu in for now
    if(unlock_manager->getUnlockedFeatures().size()>0)
    {
        // Push the new feature menu on the stack, from where
        // control will be returned to this menu.
        menu_manager->pushMenu(MENUID_UNLOCKED_FEATURE);
        return;
    }
    switch( widget_manager->getSelectedWgt() )
    {
    case WTOK_CONTINUE:
        // Gets called when:
        // 1) something was unlocked
        // 2) a Grand Prix is run
        // 3) "back to the main menu" otherwise
        world->unpause();
        race_manager->next();
        break;
    case WTOK_RESTART_RACE:
        world->unpause();
        menu_manager->popMenu();
        race_manager->restartRace();
        break;
    case WTOK_SETUP_NEW_RACE:
        world->unpause();
        race_manager->exit_race();
        menu_manager->pushMenu(MENUID_CHARSEL_P1);
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
