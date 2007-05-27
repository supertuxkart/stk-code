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
#include "widget_set.hpp"
#include "menu_manager.hpp"
#include "translation.hpp"

enum WidgetTokens {
    WTOK_HARD,
    WTOK_MEDIUM,
    WTOK_EASY,
    WTOK_BACK
};

Difficulty::Difficulty()
{
    m_menu_id = widgetSet -> vstack(0);

    widgetSet -> label(m_menu_id, _("Choose your skill level"), GUI_LRG, GUI_ALL, 0, 0);

    const int VA = widgetSet -> varray(m_menu_id);
    widgetSet -> space(m_menu_id);
    widgetSet -> space(m_menu_id);
    widgetSet -> state(VA, _("Racer"),  GUI_MED, WTOK_HARD);
    widgetSet -> state(VA, _("Driver"), GUI_MED, WTOK_MEDIUM);
    widgetSet -> start(VA, _("Novice"), GUI_MED, WTOK_EASY);
    widgetSet -> space(VA);
    widgetSet -> state(VA, _("Press <ESC> to go back"), GUI_SML, WTOK_BACK);

    widgetSet -> layout(m_menu_id, 0, 0);
}   // Difficulty

//-----------------------------------------------------------------------------
Difficulty::~Difficulty()
{
    widgetSet -> delete_widget(m_menu_id) ;
}   // ~Difficulty

//-----------------------------------------------------------------------------
void Difficulty::select()
{
    switch ( widgetSet -> get_token (widgetSet -> click()) )
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
    case WTOK_BACK:
        menu_manager->popMenu();
        break;
    default: break;
    }   // switch
}   // select


