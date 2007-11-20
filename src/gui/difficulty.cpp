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

#include "difficulty.hpp"
#include "race_manager.hpp"
#include "widget_manager.hpp"
#include "menu_manager.hpp"
#include "translation.hpp"

enum WidgetTokens {
    WTOK_TITLE,

    WTOK_HARD,
    WTOK_MEDIUM,
    WTOK_EASY,

    WTOK_QUIT
};

Difficulty::Difficulty()
{
    const bool SHOW_RECT = true;
    const bool SHOW_TEXT = true;
    widget_manager->set_initial_rect_state(SHOW_RECT, WGT_AREA_ALL, WGT_TRANS_BLACK);
    widget_manager->set_initial_text_state(SHOW_TEXT, "", WGT_FNT_MED, Font::ALIGN_CENTER, Font::ALIGN_CENTER );

    widget_manager->add_wgt(WTOK_TITLE, 60, 7);
    widget_manager->show_wgt_rect(WTOK_TITLE);
    widget_manager->show_wgt_text(WTOK_TITLE);
    widget_manager->set_wgt_text(WTOK_TITLE,
        _("Choose your skill level"));
    widget_manager->break_line();

    widget_manager->set_initial_activation_state(true);
    widget_manager->add_wgt(WTOK_HARD, 60, 7);
    widget_manager->set_wgt_text(WTOK_HARD, _("Racer"));
    widget_manager->break_line();

    widget_manager->add_wgt(WTOK_MEDIUM, 60, 7);
    widget_manager->set_wgt_text(WTOK_MEDIUM, _("Driver"));
    widget_manager->break_line();

    widget_manager->add_wgt(WTOK_EASY, 60, 7);
    widget_manager->set_wgt_text(WTOK_EASY, _("Novice"));
    widget_manager->break_line();

    widget_manager->add_wgt(WTOK_QUIT, 60, 7);
    widget_manager->set_wgt_text(WTOK_QUIT, _("Press <ESC> to go back"));

    widget_manager->layout(WGT_AREA_ALL);
}   // Difficulty

//-----------------------------------------------------------------------------
Difficulty::~Difficulty()
{
    widget_manager->delete_wgts();
}   // ~Difficulty

//-----------------------------------------------------------------------------
void Difficulty::select()
{
    switch ( widget_manager->get_selected_wgt())
    {
    case WTOK_EASY:
        race_manager->setDifficulty(RD_EASY);
        menu_manager->pushMenu(MENUID_CHARSEL_P1);
        break;
    case WTOK_MEDIUM:
        race_manager->setDifficulty(RD_MEDIUM);
        menu_manager->pushMenu(MENUID_CHARSEL_P1);
        break;
    case WTOK_HARD:
        race_manager->setDifficulty(RD_HARD);
        menu_manager->pushMenu(MENUID_CHARSEL_P1);
        break;
    case WTOK_QUIT:
        menu_manager->popMenu();
        break;
    default: break;
    }   // switch
}   // select


