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

#include "race_manager.hpp"
#include "num_players.hpp"
#include "widget_manager.hpp"
#include "menu_manager.hpp"
#include "translation.hpp"
#include "network/network_manager.hpp"

enum WidgetTokens
{
    WTOK_PLAYER_2 = 2,
    WTOK_PLAYER_3,
    WTOK_PLAYER_4,

    WTOK_QUIT
};

NumPlayers::NumPlayers()
{
    widget_manager->switchOrder();
    widget_manager->addTextButtonWgt( WTOK_PLAYER_2, 35, 7, _("Two Players") );
    widget_manager->addTextButtonWgt( WTOK_PLAYER_3, 35, 7, _("Three Players") );
    widget_manager->addTextButtonWgt( WTOK_PLAYER_4, 35, 7, _("Four Players") );

    widget_manager->addEmptyWgt( WidgetManager::WGT_NONE, 35, 7 );

    widget_manager->addTextButtonWgt( WTOK_QUIT, 35, 7,
        _("Press <ESC> to go back") );
    widget_manager->setWgtTextSize( WTOK_QUIT, WGT_FNT_SML );

    widget_manager->layout(WGT_AREA_ALL);
}

// -----------------------------------------------------------------------------
NumPlayers::~NumPlayers()
{
    widget_manager->reset() ;
}

// -----------------------------------------------------------------------------
void NumPlayers::select()
{
    switch ( widget_manager->getSelectedWgt() )
    {
    case WTOK_PLAYER_2:
    case WTOK_PLAYER_3:
    case WTOK_PLAYER_4:
        race_manager->setNumLocalPlayers(widget_manager->getSelectedWgt());
        if(network_manager->getMode()==NetworkManager::NW_CLIENT)
            menu_manager->pushMenu(MENUID_CHARSEL_P1);
        else
            menu_manager->pushMenu(MENUID_GAMEMODE);
        break;
    case WTOK_QUIT:
        menu_manager->popMenu();
        break;
    default:
        break;
    }
}



