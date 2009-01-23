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

#include "gui/main_menu.hpp"

#include <SDL/SDL.h>

#include "race_manager.hpp"
#include "user_config.hpp"
#include "challenges/unlock_manager.hpp"
#include "gui/menu_manager.hpp"
#include "gui/widget_manager.hpp"
#include "network/network_manager.hpp"
#include "utils/translation.hpp"

enum WidgetTokens
{
    WTOK_SINGLE,
    WTOK_MULTI,
    WTOK_NETWORK,
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

    // Only allow challenges if not networking for now!
    std::vector<const Challenge*> all_challenges=unlock_manager->getActiveChallenges();
    const bool challenges_active = all_challenges.size()>0 && network_manager->getMode()==NetworkManager::NW_NONE;
    
    const int WIDTH=30;
    widget_manager->addTextButtonWgt( WTOK_SINGLE, WIDTH, 7, _("Single Player") );
    widget_manager->addTextButtonWgt( WTOK_MULTI, WIDTH, 7, _("Splitscreen") );

    // Only display the networking entry when not already connected (and networking is enabled)
    if(network_manager->getMode()==NetworkManager::NW_NONE && stk_config->m_enable_networking)
        widget_manager->addTextButtonWgt( WTOK_NETWORK, WIDTH, 7, _("Networking") );

    if(challenges_active)
        widget_manager->addTextButtonWgt( WTOK_CHALLENGES, WIDTH, 7, _("Challenges") );

    widget_manager->addTextButtonWgt( WTOK_OPTIONS, WIDTH, 7, _("Options") );
    widget_manager->addTextButtonWgt( WTOK_QUIT, WIDTH, 7, _("Quit") );

    if(user_config->getWarning()!="")
    {
        widget_manager->addTextWgt( WTOK_WARNING, 80, 6, user_config->getWarning().c_str() );
        widget_manager->setWgtTextSize( WTOK_WARNING, WGT_FNT_SML );
        widget_manager->hideWgtRect(WTOK_WARNING);
    }
    else
        widget_manager->addEmptyWgt( WidgetManager::WGT_NONE, WIDTH, 6 );

    widget_manager->addTextButtonWgt( WTOK_HELP, WIDTH, 7, _("Help") );
    widget_manager->setWgtTextSize( WTOK_HELP, WGT_FNT_SML );

    widget_manager->addTextButtonWgt( WTOK_CREDITS, WIDTH, 7, _("Credits") );
    widget_manager->setWgtTextSize( WTOK_CREDITS, WGT_FNT_SML );
    
    widget_manager->addEmptyWgt( WidgetManager::WGT_NONE, WIDTH, 15 );

    widget_manager->activateWgt(WTOK_SINGLE);
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
        race_manager->setNumLocalPlayers(1);
		race_manager->setNumPlayers(1); // reset since this is used to determine the minimum number of players
        // The clients do not do any  mode selection, they go immediately
        // to the character selection screen.
        if(network_manager->getMode()==NetworkManager::NW_CLIENT)
        {
            menu_manager->pushMenu(MENUID_CHARSEL_P1);
        }
        else
        {
            menu_manager->pushMenu(MENUID_GAMEMODE);
        }
        break;
    case WTOK_MULTI:
        menu_manager->pushMenu(MENUID_NUMPLAYERS);
        break;
    case WTOK_NETWORK:
        menu_manager->pushMenu(MENUID_NETWORK_GUI);
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
