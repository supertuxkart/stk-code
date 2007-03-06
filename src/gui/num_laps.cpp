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

NumLaps::NumLaps() : laps(3)
{
    m_menu_id = widgetSet -> varray(0);
    widgetSet -> label(m_menu_id, _("Choose number of laps"),  GUI_LRG, GUI_ALL, 0, 0 );
    
    widgetSet -> space(m_menu_id);
    
    lap_label_id = widgetSet -> label(m_menu_id, _("Laps: 3"));
    widgetSet -> space(m_menu_id);
    widgetSet -> state(m_menu_id, _("Less"), GUI_MED, 10);
    widgetSet -> start(m_menu_id, _("More"), GUI_MED, 20);
    widgetSet -> space(m_menu_id);
    widgetSet -> state(m_menu_id, _("Next"), GUI_SML, 30);
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
    const int id = widgetSet->click();
    const int n = widgetSet->token(id);
    switch (n)
    {
      case 10:
        laps = std::max(1, laps-1);
	snprintf(lap_label, MAX_MESSAGE_LENGTH, "Laps: %d", laps);
	widgetSet->set_label(lap_label_id, lap_label);
	break;
      case 20:
        laps = std::min(10, laps+1);
	snprintf(lap_label, MAX_MESSAGE_LENGTH, "Laps: %d", laps);
	widgetSet->set_label(lap_label_id, lap_label);
	break;
      case 30:
        race_manager->setNumLaps(laps);
        startScreen->switchToGame();
        break;
      case -1:
        menu_manager->popMenu();
	break;
    }
}   // select



