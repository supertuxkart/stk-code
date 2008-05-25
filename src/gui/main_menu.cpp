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
#include "user_config.hpp"
#include "unlock_manager.hpp"

enum WidgetTokens
{
    WTOK_SINGLE,
    WTOK_MULTI,
    WTOK_CHALLENGES,
    WTOK_OPTIONS,
    WTOK_QUIT,
    WTOK_HELP,
    WTOK_CREDITS,
    WTOK_WARNING
};

MainMenu::MainMenu()
{
    widget_manager->switchOrder();

    const int WIDTH=30;
    widget_manager->addTextButtonWgt( WTOK_SINGLE, WIDTH, 7, _("Single Player") );
    widget_manager->addTextButtonWgt( WTOK_MULTI, WIDTH, 7, _("Multiplayer") );

    std::vector<const Challenge*> all_challenges=unlock_manager->getActiveChallenges();
    if(all_challenges.size()>0)
    {
        widget_manager->addTextButtonWgt( WTOK_CHALLENGES, WIDTH, 7, _("Challenges") );
    }

    widget_manager->addTextButtonWgt( WTOK_OPTIONS, WIDTH, 7, _("Options") );
    widget_manager->addTextButtonWgt( WTOK_QUIT, WIDTH, 7, _("Quit") );

    widget_manager->addEmptyWgt( WidgetManager::WGT_NONE, WIDTH, 7 );

    widget_manager->addTextButtonWgt( WTOK_HELP, WIDTH, 7, _("Help") );
    widget_manager->setWgtTextSize( WTOK_HELP, WGT_FNT_SML );

    widget_manager->addTextButtonWgt( WTOK_CREDITS, WIDTH, 7, _("Credits") );
    widget_manager->setWgtTextSize( WTOK_CREDITS, WGT_FNT_SML );

    if(user_config->getWarning()!="")
    {
        widget_manager->addTextWgt( WTOK_WARNING, 80, 8, user_config->getWarning().c_str() );
        widget_manager->setWgtTextSize( WTOK_WARNING, WGT_FNT_SML );
        widget_manager->hideWgtRect(WTOK_WARNING);
    }

    widget_manager->layout(WGT_AREA_ALL);
}

//-----------------------------------------------------------------------------
MainMenu::~MainMenu()
{
    widget_manager->reset();
    user_config->resetWarning();
}

//-----------------------------------------------------------------------------
void MainMenu::select()
{
    switch ( widget_manager->getSelectedWgt() )
    {
    case WTOK_SINGLE:
        race_manager->setNumPlayers(1);
        menu_manager->pushMenu(MENUID_GAMEMODE);
        break;
    case WTOK_MULTI:
        menu_manager->pushMenu(MENUID_NUMPLAYERS);
        break;
    case WTOK_CHALLENGES:
        menu_manager->pushMenu(MENUID_CHALLENGES);
        break;
    case WTOK_OPTIONS:
        menu_manager->pushMenu(MENUID_OPTIONS);
        break;

    case WTOK_QUIT:
        menu_manager->pushMenu(MENUID_EXITGAME);
        break;
    case WTOK_HELP:
        menu_manager->pushMenu(MENUID_HELP1);
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
        if(!value) break;
        menu_manager->pushMenu(MENUID_EXITGAME);
        break;

    default:
        BaseGUI::handle(ga, value);
        break;
    }
}

/* EOF */
