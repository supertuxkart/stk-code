//  $Id: Difficulty.cxx,v 1.3 2005/08/19 20:50:14 joh Exp $
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

enum WidgetTokens {
  WTOK_HARD,
  WTOK_MEDIUM,
  WTOK_EASY,
  WTOK_BACK
};

Difficulty::Difficulty() {
  menu_id = widgetSet -> vstack(0);

  widgetSet -> label(menu_id, "Choose your skill level", GUI_LRG, GUI_ALL, 0, 0);
  
  int va = widgetSet -> varray(menu_id);
  widgetSet -> space(menu_id);
  widgetSet -> space(menu_id);
  widgetSet -> state(va, "Racer",  GUI_MED, WTOK_HARD, 0);
  widgetSet -> state(va, "Driver", GUI_MED, WTOK_MEDIUM, 0);
  widgetSet -> start(va, "Novice", GUI_MED, WTOK_EASY, 0);
  widgetSet -> space(menu_id);
  widgetSet -> state(menu_id, "Press <ESC> to go back", GUI_SML, WTOK_BACK, 0);

  widgetSet -> layout(menu_id, 0, 0);
}   // Difficulty

// -----------------------------------------------------------------------------
Difficulty::~Difficulty() {
	widgetSet -> delete_widget(menu_id) ;
}   // ~Difficulty
	
// -----------------------------------------------------------------------------
void Difficulty::select() {
  switch ( widgetSet -> token (widgetSet -> click()) ) {
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


