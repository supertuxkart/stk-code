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

NumLaps::NumLaps() {
  menu_id = widgetSet -> varray(0);

  widgetSet -> space(menu_id);
  widgetSet -> space(menu_id);
  widgetSet -> space(menu_id);
  widgetSet -> label(menu_id, "Choose number of laps",  GUI_LRG   );

  widgetSet -> start(menu_id, "One",                    GUI_MED,  1);
  widgetSet -> state(menu_id, "Two",                    GUI_MED,  2);
  widgetSet -> state(menu_id, "Three",                  GUI_MED,  3);
  widgetSet -> state(menu_id, "Four",                   GUI_MED,  4);
  widgetSet -> state(menu_id, "Five",                   GUI_MED,  5);
  widgetSet -> space(menu_id);
  widgetSet -> state(menu_id, "Press <ESC> to go back", GUI_SML,  6);
  widgetSet -> space(menu_id);

  widgetSet -> layout(menu_id, 0, 0);
}

// -----------------------------------------------------------------------------
NumLaps::~NumLaps() {
  widgetSet -> delete_widget(menu_id) ;
}   // ~NumLaps

// -----------------------------------------------------------------------------
void NumLaps::select() {
  int n = widgetSet->token(widgetSet->click() );
  if(n==6) {
    menu_manager->popMenu();
  } else {
    race_manager->setNumLaps(n);
    startScreen->switchToGame();
  }
}   // select



