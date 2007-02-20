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

#include "num_laps.hpp"
#include "race_manager.hpp"
#include "start_screen.hpp"
#include "widget_set.hpp"
#include "menu_manager.hpp"
#include "translation.hpp"

NumLaps::NumLaps()
{
    m_menu_id = widgetSet -> varray(0);

    widgetSet -> space(m_menu_id);
    widgetSet -> space(m_menu_id);
    widgetSet -> space(m_menu_id);
    widgetSet -> label(m_menu_id, _("Choose number of laps"),  GUI_LRG   );

    widgetSet -> start(m_menu_id, _("One"),                    GUI_MED,  1);
    widgetSet -> state(m_menu_id, _("Two"),                    GUI_MED,  2);
    widgetSet -> state(m_menu_id, _("Four"),                   GUI_MED,  4);
    widgetSet -> state(m_menu_id, _("Five"),                   GUI_MED,  5);
    widgetSet -> state(m_menu_id, _("Six"),                    GUI_MED,  6);
    widgetSet -> state(m_menu_id, _("Eight"),                  GUI_MED,  8);
    widgetSet -> space(m_menu_id);
    widgetSet -> state(m_menu_id, _("Press <ESC> to go back"), GUI_SML, -1);
    widgetSet -> space(m_menu_id);

    widgetSet -> layout(m_menu_id, 0, 0);
}

// -----------------------------------------------------------------------------
NumLaps::~NumLaps()
{
    widgetSet -> delete_widget(m_menu_id) ;
}   // ~NumLaps

// -----------------------------------------------------------------------------
void NumLaps::select()
{
    const int N = widgetSet->token(widgetSet->click() );
    if(N==-1)
    {
        menu_manager->popMenu();
    }
    else
    {
        race_manager->setNumLaps(N);
        startScreen->switchToGame();
    }
}   // select



