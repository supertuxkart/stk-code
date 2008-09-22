//  $Id$
//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2006 SuperTuxKart-Team
//
//  This program is free software; you can redistribute it and/or
//  modify it under the terms of the GNU General Public License
//  as published by the Free Software Foundation; either version 3
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

#include <SDL/SDL.h>

#include "widget_manager.hpp"
#include "user_config.hpp"
#include "race_menu.hpp"

#include "menu_manager.hpp"
#include "race_manager.hpp"
#include "translation.hpp"

enum WidgetTokens
{
    WTOK_PAUSE,
    WTOK_RETURN_RACE,
    WTOK_OPTIONS,
    WTOK_HELP,
    WTOK_RESTART_RACE,
    WTOK_SETUP_NEW_RACE,
    WTOK_QUIT,
};

RaceMenu::RaceMenu()
{
    widget_manager->switchOrder();

    widget_manager->addTitleWgt( WTOK_PAUSE, 50, 7, _("Paused") );

    widget_manager->addTextButtonWgt( WTOK_RETURN_RACE, 50, 7, _("Return To Race"));
    widget_manager->addTextButtonWgt( WTOK_OPTIONS, 50, 7, _("Options") );
    widget_manager->addTextButtonWgt( WTOK_HELP, 50, 7, _("Help") );
    widget_manager->addTextButtonWgt( WTOK_RESTART_RACE, 50, 7, _("Restart Race") );

    if(race_manager->getMinorMode()==RaceManager::MINOR_MODE_QUICK_RACE)
    {
        widget_manager->addTextButtonWgt( WTOK_SETUP_NEW_RACE, 50, 7,
            _("Setup New Race") );
    }

    widget_manager->addTextButtonWgt( WTOK_QUIT, 50, 7, _("Exit Race") );

    widget_manager->layout(WGT_AREA_ALL);
    if(user_config->m_fullscreen) SDL_ShowCursor(SDL_ENABLE);
}

//-----------------------------------------------------------------------------
RaceMenu::~RaceMenu()
{
    widget_manager->reset();
}


//-----------------------------------------------------------------------------
void RaceMenu::select()
{
    int clicked_token = widget_manager->getSelectedWgt();

    switch (clicked_token)
    {
    case WTOK_RETURN_RACE:
        RaceManager::getWorld()->unpause();
        menu_manager->popMenu();
        if(user_config->m_fullscreen) SDL_ShowCursor(SDL_DISABLE);
        break;

    case WTOK_SETUP_NEW_RACE:
        RaceManager::getWorld()->unpause();
        race_manager->exit_race();
        menu_manager->pushMenu(MENUID_CHARSEL_P1);
        break;

    case WTOK_RESTART_RACE:
        menu_manager->popMenu();
        if(user_config->m_fullscreen) SDL_ShowCursor(SDL_DISABLE);
        RaceManager::getWorld()->restartRace();
        break;

    case WTOK_OPTIONS:
        menu_manager->pushMenu(MENUID_OPTIONS);
        break;

    case WTOK_HELP:
        menu_manager->pushMenu(MENUID_HELP1);
        break;

    case WTOK_QUIT:
        RaceManager::getWorld()->unpause();
        race_manager->exit_race();
        break;

    default:
        break;
    }
}

//-----------------------------------------------------------------------------
void RaceMenu::handle(GameAction ga, int value)
{
    switch ( ga )
    {
    case GA_LEAVE:
        if (value)
            break;
		
        RaceManager::getWorld()->unpause();
        menu_manager->popMenu();
        break;

    default:
        BaseGUI::handle(ga, value);
        break;
    }
}


