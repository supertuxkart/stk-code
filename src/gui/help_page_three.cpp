//  $Id: help_menu.cpp 812 2006-10-07 11:43:57Z hiker $
//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2006 Joerg Henrichs
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

#include "help_page_three.hpp"
#include "widget_manager.hpp"
#include "race_manager.hpp"
#include "menu_manager.hpp"
#include "user_config.hpp"
#include "player.hpp"
#include "collectable_manager.hpp"
#include "material.hpp"
#include "translation.hpp"
#include "unlock_manager.hpp"

enum WidgetTokens
{
    WTOK_TITLE,

    WTOK_TXT1,
    WTOK_TXT2,
    WTOK_TXT3,
    WTOK_TXT4,

    WTOK_SECOND_PAGE,
    WTOK_QUIT
};

HelpPageThree::HelpPageThree()
{
    widget_manager->switchOrder();
    widget_manager->addTitleWgt( WTOK_TITLE, 100, 8, _("Game modes"));

    widget_manager->addEmptyWgt( WidgetManager::WGT_NONE, 100, 2 );

    if(!unlock_manager->isLocked("grandprix"))
    {
        widget_manager->addTextWgt( WTOK_TXT1, 100, 15,
            _("Grand Prix: Win victory points \n\
by racing through a group of tracks,  \n\
most points wins (shortest time in case of a tie!)          "));
        widget_manager->setWgtTextSize( WTOK_TXT1, WGT_FNT_SML );
        widget_manager->addEmptyWgt( WidgetManager::WGT_NONE, 100, 1 );
    }

    widget_manager->addTextWgt( WTOK_TXT2, 100, 15,
        _("Quick race: Pick a single track, shortest time wins.         "));
        widget_manager->setWgtTextSize( WTOK_TXT2, WGT_FNT_SML );
    widget_manager->addEmptyWgt( WidgetManager::WGT_NONE, 100, 1 );

    widget_manager->addTextWgt( WTOK_TXT3, 100, 15,
        _("Time Trial: As quick race, but no items.                       "));
        widget_manager->setWgtTextSize( WTOK_TXT3, WGT_FNT_SML );
    widget_manager->addEmptyWgt( WidgetManager::WGT_NONE, 100, 1 );

    if(!unlock_manager->isLocked("followleader"))
    {
        widget_manager->addTextWgt( WTOK_TXT4, 100, 15,
            _("Follow the leader: run for second place, as first ahead leader, \n\
or last in race track if none ahead, will be disqualified\n\
every time the counter hits zero"));
        widget_manager->setWgtTextSize( WTOK_TXT4, WGT_FNT_SML );
        widget_manager->addEmptyWgt( WidgetManager::WGT_NONE, 100, 13 );
    }

    /*Buttons at the bottom*/
    widget_manager->addTextButtonWgt( WTOK_SECOND_PAGE, 25, 7,
        _("Previous help screen"));
    widget_manager->setWgtTextSize( WTOK_SECOND_PAGE, WGT_FNT_SML );

    widget_manager->addTextButtonWgt( WTOK_QUIT, 40, 7,
        _("Go back to the menu"));
    widget_manager->setWgtTextSize( WTOK_QUIT, WGT_FNT_SML );

    widget_manager->layout( WGT_AREA_TOP );
}   // HelpMenu

//-----------------------------------------------------------------------------
HelpPageThree::~HelpPageThree()
{
    widget_manager->reset();
}   // ~HelpMenu

//-----------------------------------------------------------------------------
void HelpPageThree::select()
{
    switch ( widget_manager->getSelectedWgt() )
    {
        case WTOK_SECOND_PAGE:
            menu_manager->popMenu();
            menu_manager->pushMenu(MENUID_HELP2);
            break;

        case WTOK_QUIT:
            menu_manager->popMenu();
            break;
    }
}   // select

/* EOF */
