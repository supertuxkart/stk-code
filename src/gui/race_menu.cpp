//  $Id$
//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2006 SuperTuxKart-Team
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

#include <SDL/SDL.h>

#include "widget_manager.hpp"
#include "user_config.hpp"
#include "race_menu.hpp"
#include "world.hpp"

#include "menu_manager.hpp"
#include "race_manager.hpp"
#include "sound_manager.hpp"
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
    const bool SHOW_RECT = true;
    const bool SHOW_TEXT = true;
    widget_manager->setInitialRectState(SHOW_RECT, WGT_AREA_ALL, WGT_TRANS_BLACK);
    widget_manager->setInitialTextState(SHOW_TEXT, "", WGT_FNT_MED,
        WGT_FONT_GUI, WGT_WHITE );

    widget_manager->addWgt(WTOK_PAUSE, 30, 7);
    widget_manager->setWgtText(WTOK_PAUSE, _("Paused"));
    widget_manager->breakLine();

    widget_manager->setInitialActivationState(true);
    widget_manager->addWgt(WTOK_RETURN_RACE, 30, 7);
    widget_manager->setWgtText(WTOK_RETURN_RACE, _("Return To Race"));
    widget_manager->breakLine();

    widget_manager->addWgt(WTOK_OPTIONS, 30, 7);
    widget_manager->setWgtText(WTOK_OPTIONS, _("Options"));
    widget_manager->breakLine();

    widget_manager->addWgt(WTOK_HELP, 30, 7);
    widget_manager->setWgtText(WTOK_HELP, _("Help"));
    widget_manager->breakLine();

    widget_manager->addWgt(WTOK_RESTART_RACE, 30, 7);
    widget_manager->setWgtText(WTOK_RESTART_RACE, _("Restart Race"));
    widget_manager->breakLine();

    if(race_manager->getRaceMode()==RaceManager::RM_QUICK_RACE)
    {
        widget_manager->addWgt(WTOK_SETUP_NEW_RACE, 30, 7);
        widget_manager->setWgtText(WTOK_SETUP_NEW_RACE, _("Setup New Race"));
        widget_manager->breakLine();
    }

    widget_manager->addWgt(WTOK_QUIT, 30, 7);
    widget_manager->setWgtText(WTOK_QUIT, _("Exit Race"));

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
        world->unpause();
        menu_manager->popMenu();
        if(user_config->m_fullscreen) SDL_ShowCursor(SDL_DISABLE);
        break;

    case WTOK_SETUP_NEW_RACE:
        world->unpause();
        race_manager->exit_race();
        menu_manager->pushMenu(MENUID_CHARSEL_P1);
        break;

    case WTOK_RESTART_RACE:
        world->unpause();
        menu_manager->popMenu();
        if(user_config->m_fullscreen) SDL_ShowCursor(SDL_DISABLE);
        world->restartRace();
        break;

    case WTOK_OPTIONS:
        menu_manager->pushMenu(MENUID_OPTIONS);
        break;

    case WTOK_HELP:
        menu_manager->pushMenu(MENUID_HELP1);
        break;

    case WTOK_QUIT:
        world->unpause();
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
		
        world->unpause();
        menu_manager->popMenu();
        break;

    default:
        BaseGUI::handle(ga, value);
        break;
    }
}


