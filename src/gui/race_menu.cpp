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

#include "race_menu.hpp"
#include "config.hpp"
#include "world.hpp"
#include "widget_set.hpp"

#include "menu_manager.hpp"
#include "race_manager.hpp"


enum WidgetTokens {
    WTOK_RETURN_RACE,
    WTOK_OPTIONS,
    WTOK_HELP,
    WTOK_RESTART_RACE,
    WTOK_SETUP_NEW_RACE,
    WTOK_EXIT_RACE,
};

RaceMenu::RaceMenu()
{
    m_menu_id = widgetSet -> vstack(0);
    widgetSet -> label(m_menu_id, "Paused", GUI_LRG, GUI_ALL, 0, 0);

    const int VA = widgetSet -> varray(m_menu_id);
    widgetSet -> start(VA, "Return To Race",  GUI_MED, WTOK_RETURN_RACE);
    widgetSet -> state(VA, "Options",         GUI_MED, WTOK_OPTIONS);
    widgetSet -> state(VA, "Help",            GUI_MED, WTOK_HELP);
    widgetSet -> state(VA, "Restart Race",    GUI_MED, WTOK_RESTART_RACE);

    if(world->m_race_setup.m_mode==RaceSetup::RM_QUICK_RACE)
    {
        widgetSet->state(VA, "Setup New Race",  GUI_MED, WTOK_SETUP_NEW_RACE);
    }

    widgetSet -> state(VA, "Exit Race",       GUI_MED, WTOK_EXIT_RACE);

    widgetSet -> layout(m_menu_id, 0, 0);

    if(config->m_fullscreen) SDL_ShowCursor(SDL_ENABLE);
}

//-----------------------------------------------------------------------------
RaceMenu::~RaceMenu()
{
    widgetSet -> delete_widget(m_menu_id) ;

    if(config->m_fullscreen) SDL_ShowCursor(SDL_DISABLE);
}

//-----------------------------------------------------------------------------
void RaceMenu::update(float dt)
{
    widgetSet -> timer(m_menu_id, dt) ;
    // widgetSet -> blank();
    widgetSet -> paint(m_menu_id) ;
}

//-----------------------------------------------------------------------------
void RaceMenu::select()
{
    int clicked_token = widgetSet->token(widgetSet->click());
    if(clicked_token != WTOK_OPTIONS && clicked_token != WTOK_HELP)
        widgetSet -> tgl_paused();

    switch (clicked_token)
    {
    case WTOK_RETURN_RACE:
        menu_manager->popMenu();
        break;

    case WTOK_SETUP_NEW_RACE:
        race_manager->exit_race();
        menu_manager->pushMenu(MENUID_DIFFICULTY);
        break;

    case WTOK_RESTART_RACE:
        menu_manager->popMenu();
        world->restartRace();
        break;

    case WTOK_OPTIONS:
        menu_manager->pushMenu(MENUID_OPTIONS);
        break;

    case WTOK_HELP:
        menu_manager->pushMenu(MENUID_HELP);
        break;

    case WTOK_EXIT_RACE:
        race_manager->exit_race();
        break;

    default:
        break;
    }
}

//-----------------------------------------------------------------------------
void RaceMenu::inputKeyboard(int key, int pressed)
{
    switch ( key )
    {
    case SDLK_ESCAPE: //ESC
        if(!pressed)
            break;
        widgetSet -> tgl_paused();
        menu_manager->popMenu();
        break;

    default:
        BaseGUI::inputKeyboard(key, pressed);
        break;
    }
}


