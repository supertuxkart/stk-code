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
    WTOK_EMPTY,
    WTOK_BACK
};

GameMode::GameMode()
{
    const bool SHOW_RECT = true;
    const bool SHOW_TEXT = true;
    widget_manager->setInitialRectState(SHOW_RECT, WGT_AREA_ALL, WGT_TRANS_BLACK);
    widget_manager->setInitialTextState(SHOW_TEXT, "", WGT_FNT_MED,
        WGT_FONT_GUI, WGT_WHITE );

    widget_manager->insertColumn();
    widget_manager->addWgt(WTOK_TITLE, 50, 7);
    widget_manager->setWgtText( WTOK_TITLE, _("Choose a Race Mode"));
    widget_manager->setWgtTextSize( WTOK_TITLE, WGT_FNT_LRG );

    widget_manager->setInitialActivationState(true);
    widget_manager->addWgt(WTOK_GP, 50, 7);
    if(unlock_manager->isLocked("grandprix"))
    {
        widget_manager->setWgtText( WTOK_GP, "???");
        widget_manager->deactivateWgt(WTOK_GP);
    }
    else
    {
        widget_manager->setWgtText( WTOK_GP, _("Grand Prix"));
    }

    widget_manager->addWgt(WTOK_QUICKRACE, 50, 7);
    widget_manager->setWgtText( WTOK_QUICKRACE, _("Quick Race"));

    if( race_manager->getNumPlayers() == 1 )
    {
        widget_manager->addWgt(WTOK_TIMETRIAL, 50, 7);
        widget_manager->setWgtText( WTOK_TIMETRIAL, _("Time Trial"));
    }
    widget_manager->addWgt(WTOK_FOLLOW_LEADER, 50, 7);
    if(unlock_manager->isLocked("followleader"))
    {
        widget_manager->setWgtText( WTOK_FOLLOW_LEADER, "???");
        widget_manager->deactivateWgt(WTOK_FOLLOW_LEADER);
    }
    else
    {
        widget_manager->setWgtText( WTOK_FOLLOW_LEADER, _("Follow the Leader"));
    }
    widget_manager->addWgt(WTOK_EMPTY, 50, 7);
    widget_manager->hideWgtRect( WTOK_EMPTY );
    widget_manager->hideWgtText( WTOK_EMPTY );
    widget_manager->deactivateWgt( WTOK_EMPTY );

    widget_manager->addWgt(WTOK_BACK, 50, 7);
    widget_manager->setWgtText( WTOK_BACK, _("Press <ESC> to go back"));
    widget_manager->setWgtTextSize( WTOK_BACK, WGT_FNT_SML );

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
        menu_manager->pushMenu(MENUID_GRANDPRIXSELECT);
        break;
    case WTOK_QUICKRACE:
        race_manager->setRaceMode(RaceManager::RM_QUICK_RACE);
        menu_manager->pushMenu(MENUID_CHARSEL_P1);
        break;
    case WTOK_FOLLOW_LEADER:
        race_manager->setRaceMode(RaceManager::RM_FOLLOW_LEADER);
        menu_manager->pushMenu(MENUID_CHARSEL_P1);
        break;
    case WTOK_TIMETRIAL:
        race_manager->setRaceMode(RaceManager::RM_TIME_TRIAL);
        menu_manager->pushMenu(MENUID_CHARSEL_P1);
        break;
    case WTOK_BACK:
        menu_manager->popMenu();
        break;
    default: break;
    }
}



