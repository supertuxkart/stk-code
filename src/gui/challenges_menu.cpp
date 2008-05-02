//  $Id: challenges_menu.cpp 1305 2007-11-26 14:28:15Z hiker $
//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2008 Joerg Henrichs
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

#include "gui/challenges_menu.hpp"
#include "widget_manager.hpp"
#include "menu_manager.hpp"
#include "unlock_manager.hpp"
#include "translation.hpp"

enum WidgetTokens
{
    WTOK_TITLE,
    WTOK_BACK,
    WTOK_DESCRIPTION,
    WTOK_CHALLENGES
};

ChallengesMenu::ChallengesMenu()
{
    widget_manager->addTitleWgt( WTOK_TITLE, 60, 10, _("Active Challenges"));
    widget_manager->breakLine();

    m_all_challenges=unlock_manager->getActiveChallenges();
    for(int i=0; i<(int)m_all_challenges.size(); i++)
    {
        widget_manager->addTextButtonWgt(WTOK_CHALLENGES+i, 60, 10,
            _(m_all_challenges[i]->getName().c_str()) );
        widget_manager->breakLine();
    }

    widget_manager->addTextButtonWgt( WTOK_DESCRIPTION, 60, 30, "");
    widget_manager->breakLine();

    widget_manager->addTextButtonWgt(WTOK_BACK, 50, 7,
        _("Go back to the main menu"));

    widget_manager->layout(WGT_AREA_ALL);
}   // ChallengesMenu

//-----------------------------------------------------------------------------
ChallengesMenu::~ChallengesMenu()
{
    widget_manager->reset();
}   // ~ChallengesMenu

//-----------------------------------------------------------------------------
void ChallengesMenu::update(float dt)
{
    const int challenge= widget_manager->getSelectedWgt() - WTOK_CHALLENGES;
    if(challenge>=0 && challenge<(int)m_all_challenges.size())
    {
        widget_manager->setWgtText(WTOK_DESCRIPTION,
            m_all_challenges[challenge]->getChallengeDescription());
    }
    widget_manager->update(dt);
}   // update

//-----------------------------------------------------------------------------
void ChallengesMenu::select()
{
    if(widget_manager->getSelectedWgt()==WTOK_BACK)
    {
        menu_manager->popMenu();
        return;
    }
    int n=widget_manager->getSelectedWgt()-WTOK_CHALLENGES;
    if(n>=0 && n<(int)m_all_challenges.size())
    {
        m_all_challenges[n]->setRace();
        menu_manager->pushMenu(MENUID_START_RACE_FEEDBACK);
    }
}   // select
