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

enum WidgetTokens {
    WTOK_TITLE,
    WTOK_GP,
    WTOK_QUICKRACE,
    WTOK_TIMETRIAL,
    WTOK_EMPTY,
    WTOK_BACK
};

GameMode::GameMode()
{
    const bool SHOW_RECT = true;
    const bool SHOW_TEXT = true;
    widget_manager->set_initial_rect_state(SHOW_RECT, WGT_AREA_ALL, WGT_TRANS_BLACK);
    widget_manager->set_initial_text_state(SHOW_TEXT, "", WGT_FNT_MED );

    widget_manager->insert_column();
    widget_manager->add_wgt(WTOK_TITLE, 50, 7);
    widget_manager->set_wgt_text( WTOK_TITLE, _("Choose a Race Mode"));
    widget_manager->set_wgt_text_size( WTOK_TITLE, WGT_FNT_LRG );

    widget_manager->set_initial_activation_state(true);
    widget_manager->add_wgt(WTOK_GP, 50, 7);
    widget_manager->set_wgt_text( WTOK_GP, _("Grand Prix"));

    widget_manager->add_wgt(WTOK_QUICKRACE, 50, 7);
    widget_manager->set_wgt_text( WTOK_QUICKRACE, _("Quick Race"));

    if( race_manager->getNumPlayers() == 1 )
    {
        widget_manager->add_wgt(WTOK_TIMETRIAL, 50, 7);
        widget_manager->set_wgt_text( WTOK_TIMETRIAL, _("Time Trial"));
    }

    widget_manager->add_wgt(WTOK_EMPTY, 50, 7);
    widget_manager->hide_wgt_rect( WTOK_EMPTY );
    widget_manager->hide_wgt_text( WTOK_EMPTY );
    widget_manager->deactivate_wgt( WTOK_EMPTY );

    widget_manager->add_wgt(WTOK_BACK, 50, 7);
    widget_manager->set_wgt_text( WTOK_BACK, _("Press <ESC> to go back"));
    widget_manager->set_wgt_text_size( WTOK_BACK, WGT_FNT_SML );

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
    switch ( widget_manager->get_selected_wgt() )
    {
    case WTOK_GP:
        race_manager->setRaceMode(RaceSetup::RM_GRAND_PRIX);
        menu_manager->pushMenu(MENUID_GRANDPRIXSELECT);
        break;
    case WTOK_QUICKRACE:
        race_manager->setRaceMode(RaceSetup::RM_QUICK_RACE);
        menu_manager->pushMenu(MENUID_DIFFICULTY);
        break;
    case WTOK_TIMETRIAL:
        race_manager->setRaceMode(RaceSetup::RM_TIME_TRIAL);
        menu_manager->pushMenu(MENUID_CHARSEL_P1); //difficulty makes no sense here
        break;
    case WTOK_BACK:
        menu_manager->popMenu();
        break;
    default: break;
    }
}



