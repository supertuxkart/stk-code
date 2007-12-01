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
#include "widget_manager.hpp"
#include "race_manager.hpp"
#include "menu_manager.hpp"
#include "translation.hpp"

enum WidgetTokens {
    WTOK_SINGLE,
    WTOK_MULTI,
    WTOK_OPTIONS,
    WTOK_QUIT,
    WTOK_EMPTY,
    WTOK_HELP,
    WTOK_CREDITS
};

MainMenu::MainMenu()
{
    const bool SHOW_RECT = true;
    const bool SHOW_TEXT = true;
    widget_manager->set_initial_activation_state(true);
    widget_manager->set_initial_rect_state(SHOW_RECT, WGT_AREA_ALL, WGT_TRANS_BLACK);
    widget_manager->set_initial_text_state(SHOW_TEXT, "", WGT_FNT_MED, Font::ALIGN_CENTER, Font::ALIGN_CENTER );

    widget_manager->insert_column();
    widget_manager->add_wgt(WTOK_SINGLE, 25, 7);
    widget_manager->set_wgt_text( WTOK_SINGLE, _("Single Player") );

    widget_manager->add_wgt(WTOK_MULTI, 25, 7);
    widget_manager->set_wgt_text( WTOK_MULTI, _("Multiplayer") );

    widget_manager->add_wgt(WTOK_OPTIONS, 25, 7);
    widget_manager->set_wgt_text( WTOK_OPTIONS, _("Options") );

    widget_manager->add_wgt(WTOK_QUIT, 25, 7);
    widget_manager->set_wgt_text( WTOK_QUIT, _("Quit") );

    widget_manager->add_wgt(WTOK_EMPTY, 25, 7);
    widget_manager->hide_wgt_text( WTOK_EMPTY );
    widget_manager->hide_wgt_rect( WTOK_EMPTY );
    widget_manager->deactivate_wgt( WTOK_EMPTY );

    widget_manager->add_wgt(WTOK_HELP, 25, 7);
    widget_manager->set_wgt_text( WTOK_HELP, _("Help") );
    //FIXME: if text size is not set, we get a crash when resizing the rect to the text
    widget_manager->set_wgt_text_size( WTOK_HELP, WGT_FNT_SML );

    widget_manager->add_wgt(WTOK_CREDITS, 25, 7);
    widget_manager->set_wgt_text( WTOK_CREDITS, _("Credits") );
    widget_manager->set_wgt_text_size( WTOK_CREDITS, WGT_FNT_SML );

    widget_manager->layout(WGT_AREA_ALL);
}

//-----------------------------------------------------------------------------
MainMenu::~MainMenu()
{
    widget_manager->reset();
}

//-----------------------------------------------------------------------------
void MainMenu::select()
{
    #if 0
    switch ( widgetSet -> get_token (widgetSet -> click()) )
    {
    case WTOK_SINGLE:
        race_manager->setNumPlayers(1);
        menu_manager->pushMenu(MENUID_GAMEMODE);
        break;
    case WTOK_MULTI:
        menu_manager->pushMenu(MENUID_NUMPLAYERS);
        break;

    case WTOK_REPLAY:
        //TODO
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
    #endif
    switch ( widget_manager->get_selected_wgt() )
    {
    case WTOK_SINGLE:
        race_manager->setNumPlayers(1);
        menu_manager->pushMenu(MENUID_GAMEMODE);
        break;
    case WTOK_MULTI:
        menu_manager->pushMenu(MENUID_NUMPLAYERS);
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
void MainMenu::handle(GameAction ga, int value)
{
    switch ( ga )
    {
    case GA_LEAVE:
        if(!value)
            break;
		
        menu_manager->pushMenu(MENUID_EXITGAME);
        break;

    default:
        BaseGUI::handle(ga, value);
        break;
    }
}

/* EOF */
