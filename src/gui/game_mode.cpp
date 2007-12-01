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
    WTOK_BACK
};

GameMode::GameMode()
{
    widget_manager->add_wgt(WTOK_TITLE, 50, 7);
    widget_manager->show_wgt_rect( WTOK_TITLE );
    widget_manager->set_wgt_text( WTOK_TITLE, _("Choose a Race Mode"));
    widget_manager->set_wgt_text_size( WTOK_TITLE, WGT_FNT_LRG );
    widget_manager->show_wgt_text( WTOK_TITLE );
    widget_manager->break_line();

    widget_manager->add_wgt(WTOK_GP, 50, 7);
    widget_manager->show_wgt_rect( WTOK_GP );
    widget_manager->set_wgt_text( WTOK_GP, _("Grand Prix"));
    widget_manager->set_wgt_text_size( WTOK_GP, WGT_FNT_MED );
    widget_manager->show_wgt_text( WTOK_GP );
    widget_manager->activate_wgt( WTOK_GP );
    widget_manager->break_line();

    widget_manager->add_wgt(WTOK_QUICKRACE, 50, 7);
    widget_manager->show_wgt_rect( WTOK_QUICKRACE );
    widget_manager->set_wgt_text( WTOK_QUICKRACE, _("Quick Race"));
    widget_manager->set_wgt_text_size( WTOK_QUICKRACE, WGT_FNT_MED );
    widget_manager->show_wgt_text( WTOK_QUICKRACE );
    widget_manager->activate_wgt( WTOK_QUICKRACE );
    widget_manager->break_line();

    if( race_manager->getNumPlayers() == 1 )
    {
        widget_manager->add_wgt(WTOK_TIMETRIAL, 50, 7);
        widget_manager->show_wgt_rect( WTOK_TIMETRIAL );
        widget_manager->set_wgt_text( WTOK_TIMETRIAL, _("Time Trial"));
        widget_manager->set_wgt_text_size( WTOK_TIMETRIAL, WGT_FNT_MED );
        widget_manager->show_wgt_text( WTOK_TIMETRIAL );
        widget_manager->activate_wgt( WTOK_TIMETRIAL );
        widget_manager->break_line();
    }

    widget_manager->add_wgt(WidgetManager::WGT_NONE, 50, 7);
    widget_manager->break_line();

    widget_manager->add_wgt(WTOK_BACK, 50, 7);
    widget_manager->show_wgt_rect( WTOK_BACK );
    widget_manager->set_wgt_text( WTOK_BACK, _("Press <ESC> to go back"));
    widget_manager->set_wgt_text_size( WTOK_BACK, WGT_FNT_SML );
    widget_manager->show_wgt_text( WTOK_BACK );
    widget_manager->activate_wgt( WTOK_BACK );

    widget_manager->layout(WGT_AREA_ALL);
/*    m_menu_id = widgetSet -> vstack(0);

    widgetSet -> label(m_menu_id, _("Choose a Race Mode"), GUI_LRG);

    const int VA = widgetSet -> varray(m_menu_id);
    widgetSet -> space(m_menu_id);
    widgetSet -> start(VA, _("Grand Prix"),  GUI_MED, WTOK_GP);
    widgetSet -> state(VA, _("Quick Race"),  GUI_MED, WTOK_QUICKRACE);

    if (race_manager->getNumPlayers() == 1)
        widgetSet -> state(VA, _("Time Trial"),  GUI_MED, WTOK_TIMETRIAL);

    widgetSet -> space(VA);
    widgetSet -> state(VA,_("Press <ESC> to go back"), GUI_SML, WTOK_BACK);
    widgetSet -> space(VA);
    widgetSet -> layout(m_menu_id, 0, 0);*/
}

//-----------------------------------------------------------------------------
GameMode::~GameMode()
{
    widget_manager->reset();
}

//-----------------------------------------------------------------------------
void GameMode::select()
{
//    switch ( widgetSet -> get_token (widgetSet -> click()) )
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



