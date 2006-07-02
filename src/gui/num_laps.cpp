//  $Id: NumLaps.cxx,v 1.1 2005/08/19 20:43:04 joh Exp $
//
//  TuxKart - a fun racing game with go-kart
//  Copyright (C) 2004 Steve Baker <sjbaker1@airmail.net>
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

NumLaps::NumLaps() {
  menu_id = widgetSet -> varray(0);

  widgetSet -> label(menu_id, "Choose number of laps", GUI_LRG, GUI_ALL, 0, 0);

  widgetSet -> start(menu_id, "One",   GUI_MED, 1, 0);
  widgetSet -> state(menu_id, "Two",   GUI_MED, 2, 0);
  widgetSet -> state(menu_id, "Three", GUI_MED, 3, 0);
  widgetSet -> state(menu_id, "Four",  GUI_MED, 4, 0);
  widgetSet -> state(menu_id, "Five",  GUI_MED, 5, 0);
  widgetSet -> space(menu_id);

  widgetSet -> layout(menu_id, 0, 0);
}

// -----------------------------------------------------------------------------
NumLaps::~NumLaps() {
  widgetSet -> delete_widget(menu_id) ;
}   // ~NumLaps

// -----------------------------------------------------------------------------
void NumLaps::update(float dt) {
  widgetSet -> timer(menu_id, dt) ;
  widgetSet -> paint(menu_id) ;
}   // update

// -----------------------------------------------------------------------------
void NumLaps::select() {
  race_manager->setNumLaps(widgetSet->token(widgetSet->click() ));
  startScreen->switchToGame();
  
}   // select



