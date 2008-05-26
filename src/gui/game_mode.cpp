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

#include "game_mode.hpp"
#include "widget_manager.hpp"
#include "race_manager.hpp"
#include "material_manager.hpp"
#include "menu_manager.hpp"
#include "translation.hpp"
#include "user_config.hpp"
#include "unlock_manager.hpp"

enum WidgetTokens
{
    WTOK_TITLE,

    WTOK_GP,
    WTOK_QUICKRACE,
    WTOK_TIMETRIAL,
    WTOK_FOLLOW_LEADER,

    WTOK_HELP,
    WTOK_QUIT
};

GameMode::GameMode()
{
    widget_manager->switchOrder();

    widget_manager->addTextButtonWgt( WTOK_GP, 60, 7, _("Grand Prix"));

    if(unlock_manager->isLocked("grandprix"))
    {
        widget_manager->hideWgtText( WTOK_GP );
        widget_manager->deactivateWgt( WTOK_GP );

        widget_manager->setWgtColor( WTOK_GP, WGT_WHITE);
        widget_manager->setWgtTexture( WTOK_GP, "gui_lock.rgb", false );
        widget_manager->showWgtTexture( WTOK_GP );
    }


    widget_manager->addTextButtonWgt(WTOK_QUICKRACE, 60, 7, _("Quick Race"));

    if( race_manager->getNumPlayers() == 1 )
    {
        widget_manager->addTextButtonWgt(WTOK_TIMETRIAL, 60, 7, _("Time Trial"));
    }


    widget_manager->addTextButtonWgt( WTOK_FOLLOW_LEADER, 60, 7,
        _("Follow the Leader"));

    if(unlock_manager->isLocked("followleader"))
    {
        widget_manager->hideWgtText( WTOK_FOLLOW_LEADER );

//        widget_manager->deactivateWgt( WTOK_FOLLOW_LEADER );

        widget_manager->setWgtColor( WTOK_FOLLOW_LEADER, WGT_GRAY);
        widget_manager->setWgtTexture( WTOK_FOLLOW_LEADER, "gui_lock.rgb", false );
        widget_manager->showWgtTexture( WTOK_FOLLOW_LEADER );
    }

    widget_manager->addEmptyWgt( WidgetManager::WGT_NONE, 1, 7);

    widget_manager->addTextButtonWgt( WTOK_HELP, 60, 7, _("Game mode help"));
    widget_manager->setWgtTextSize( WTOK_HELP, WGT_FNT_SML );

    widget_manager->addEmptyWgt( WidgetManager::WGT_NONE, 1, 7);

    widget_manager->addTextButtonWgt(WTOK_QUIT, 60, 7, _("Press <ESC> to go back"));
    widget_manager->setWgtTextSize( WTOK_QUIT, WGT_FNT_SML );

    widget_manager->layout(WGT_AREA_ALL);
}

//-----------------------------------------------------------------------------
GameMode::~GameMode()
{
    widget_manager->reset();
}

//-----------------------------------------------------------------------------
void GameMode::select()
{
    switch ( widget_manager->getSelectedWgt() )
    {
    case WTOK_GP:
        race_manager->setRaceMode(RaceManager::RM_GRAND_PRIX);
        menu_manager->pushMenu(MENUID_CHARSEL_P1);
        break;
    case WTOK_QUICKRACE:
        race_manager->setRaceMode(RaceManager::RM_QUICK_RACE);
        menu_manager->pushMenu(MENUID_CHARSEL_P1);
        break;
    case WTOK_FOLLOW_LEADER:
        if(unlock_manager->isLocked("followleader"))
        {
            widget_manager->showWgtText( WTOK_FOLLOW_LEADER );
            widget_manager->setWgtTextColor( WTOK_FOLLOW_LEADER, WGT_TRANS_GRAY);
            widget_manager->setWgtColor( WTOK_FOLLOW_LEADER, WGT_TRANS_GRAY);
        }
        else
        {
            race_manager->setRaceMode(RaceManager::RM_FOLLOW_LEADER);
            menu_manager->pushMenu(MENUID_CHARSEL_P1);
        }
        break;
    case WTOK_TIMETRIAL:
        race_manager->setRaceMode(RaceManager::RM_TIME_TRIAL);
        menu_manager->pushMenu(MENUID_CHARSEL_P1);
        break;
    case WTOK_HELP:
        menu_manager->pushMenu(MENUID_HELP3);
        break;
    case WTOK_QUIT:
        menu_manager->popMenu();
        break;
    default: break;
    }
}



