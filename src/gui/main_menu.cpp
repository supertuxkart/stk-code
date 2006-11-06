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

#include "main_menu.hpp"
#include "widget_set.hpp"
#include "race_manager.hpp"
#include "menu_manager.hpp"

enum WidgetTokens {
    WTOK_SINGLE,
    WTOK_MULTI,
    WTOK_OPTIONS,
    WTOK_REPLAY,
    WTOK_QUIT,
    WTOK_HELP,
    WTOK_CREDITS,
};

MainMenu::MainMenu()
{
    m_menu_id = widgetSet -> varray(0);
    widgetSet -> space(m_menu_id);
    widgetSet -> space(m_menu_id);
    widgetSet -> start(m_menu_id, "Single Player", GUI_MED, WTOK_SINGLE,  0);
    widgetSet -> state(m_menu_id, "Multiplayer",   GUI_MED, WTOK_MULTI,   0);
    widgetSet -> state(m_menu_id, "Options",       GUI_MED, WTOK_OPTIONS, 0);
    widgetSet -> state(m_menu_id, "Quit",          GUI_MED, WTOK_QUIT,    0);
    widgetSet -> space(m_menu_id);
    widgetSet -> state(m_menu_id, "Help",          GUI_SML, WTOK_HELP,    0);
    widgetSet -> state(m_menu_id, "Credits",       GUI_SML, WTOK_CREDITS, 0);
    widgetSet -> space(m_menu_id);

    widgetSet -> layout(m_menu_id, 0, 0);
}

//-----------------------------------------------------------------------------
MainMenu::~MainMenu()
{
    widgetSet -> delete_widget(m_menu_id) ;
}

//-----------------------------------------------------------------------------
void MainMenu::select()
{
    switch ( widgetSet -> token (widgetSet -> click()) )
    {
    case WTOK_SINGLE:
        race_manager->setNumPlayers(1);
        menu_manager->pushMenu(MENUID_GAMEMODE);
        break;
    case WTOK_MULTI:
        menu_manager->pushMenu(MENUID_NUMPLAYERS);
        break;

    case WTOK_REPLAY:
        break;

    case WTOK_OPTIONS:
        menu_manager->pushMenu(MENUID_OPTIONS);
        break;

    case WTOK_QUIT:
        menu_manager->pushMenu(MENUID_EXITGAME);
        break;
    case WTOK_HELP:
        menu_manager->pushMenu(MENUID_HELP);
        break;
    case WTOK_CREDITS:
        menu_manager->pushMenu(MENUID_CREDITS);
        break;
    }
}

//-----------------------------------------------------------------------------
void MainMenu::inputKeyboard(int key, int pressed)
{
    switch ( key )
    {
    case SDLK_ESCAPE:   //ESC
        if(!pressed)
            break;
        menu_manager->pushMenu(MENUID_EXITGAME);
        break;

    default:
        BaseGUI::inputKeyboard(key, pressed);
        break;
    }
}

/* EOF */
