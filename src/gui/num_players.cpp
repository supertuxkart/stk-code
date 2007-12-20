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

#include "race_manager.hpp"
#include "num_players.hpp"
#include "widget_manager.hpp"
#include "menu_manager.hpp"
#include "translation.hpp"

enum WidgetTokens {
    WTOK_PLAYER_2 = 2,
    WTOK_PLAYER_3,
    WTOK_PLAYER_4,
    WTOK_BACK
};

NumPlayers::NumPlayers()
{
    widget_manager->addWgt(WTOK_PLAYER_2, 35, 7);
    widget_manager->showWgtRect( WTOK_PLAYER_2 );
    widget_manager->setWgtText( WTOK_PLAYER_2, _("Two Players") );
    widget_manager->setWgtTextSize( WTOK_PLAYER_2, WGT_FNT_MED );
    widget_manager->showWgtText( WTOK_PLAYER_2 );
    widget_manager->activateWgt( WTOK_PLAYER_2 );
    widget_manager->breakLine();

    widget_manager->addWgt(WTOK_PLAYER_3, 35, 7);
    widget_manager->showWgtRect( WTOK_PLAYER_3 );
    widget_manager->setWgtText( WTOK_PLAYER_3, _("Three Players") );
    widget_manager->setWgtTextSize( WTOK_PLAYER_3, WGT_FNT_MED );
    widget_manager->showWgtText( WTOK_PLAYER_3 );
    widget_manager->activateWgt( WTOK_PLAYER_3);
    widget_manager->breakLine();

    widget_manager->addWgt(WTOK_PLAYER_4, 35, 7);
    widget_manager->showWgtRect( WTOK_PLAYER_4 );
    widget_manager->setWgtText( WTOK_PLAYER_4, _("Four Players") );
    widget_manager->setWgtTextSize( WTOK_PLAYER_4, WGT_FNT_MED );
    widget_manager->showWgtText( WTOK_PLAYER_4 );
    widget_manager->activateWgt( WTOK_PLAYER_4 );
    widget_manager->breakLine();

    widget_manager->addWgt(WidgetManager::WGT_NONE, 35, 7);
    widget_manager->breakLine();

    widget_manager->addWgt(WTOK_BACK, 35, 7);
    widget_manager->showWgtRect( WTOK_BACK );
    widget_manager->setWgtText( WTOK_BACK, _("Press <ESC> to go back") );
    widget_manager->setWgtTextSize( WTOK_BACK, WGT_FNT_SML );
    widget_manager->showWgtText( WTOK_BACK );
    widget_manager->activateWgt( WTOK_BACK );

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
        race_manager->setNumPlayers(widget_manager->getSelectedWgt());
        menu_manager->pushMenu(MENUID_GAMEMODE);
        break;
    case WTOK_BACK:
        menu_manager->popMenu();
        break;
    default:
        break;
    }
}



