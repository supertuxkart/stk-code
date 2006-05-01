//  $Id: Difficulty.cxx,v 1.3 2005/08/19 20:50:14 joh Exp $
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

#include "Difficulty.h"
#include "RaceManager.h"
#include "WidgetSet.h"

Difficulty::Difficulty() {
  menu_id = widgetSet -> vstack(0);

  widgetSet -> label(menu_id, "Choose a Difficulty", GUI_LRG, GUI_ALL, 0, 0);
  
  int va = widgetSet -> varray(menu_id);
  widgetSet -> start(va, "Easy", GUI_MED, MENU_EASY, 0);
  widgetSet -> state(va, "Medium", GUI_MED, MENU_MEDIUM, 0);
  widgetSet -> state(va, "Hard", GUI_MED, MENU_HARD, 0);
  
  if (0) {
    if (std::find(guiStack.begin(), guiStack.end(), GUIS_DIFFICULTYSR) != 
	guiStack.end()) {
      widgetSet -> state(menu_id, "Number of Laps", GUI_SML, 0, 0);
      widgetSet -> state(menu_id, "Reverse Track", GUI_SML, 0, 0);
#ifdef SSG_BACKFACE_COLLISIONS_SUPPORTED
      widgetSet -> state(menu_id, "Mirror Track", GUI_SML, 0, 0);
#endif
    }   // if not guiStack.end()
  }   // if (0)

  widgetSet -> layout(menu_id, 0, 0);
}   // Difficulty

// -----------------------------------------------------------------------------
Difficulty::~Difficulty() {
	widgetSet -> delete_widget(menu_id) ;
}   // ~Difficulty
	
// -----------------------------------------------------------------------------
void Difficulty::update(float dt) {
	
	widgetSet -> timer(menu_id, dt) ;
	widgetSet -> paint(menu_id) ;
}   // update

// -----------------------------------------------------------------------------
void Difficulty::select() {
  switch ( widgetSet -> token (widgetSet -> click()) ) {
    case MENU_EASY:   race_manager->setDifficulty(RD_EASY);
	  	      guiStack.push_back(GUIS_CHARSEL);
		      break;
    case MENU_MEDIUM: race_manager->setDifficulty(RD_MEDIUM);
	  	      guiStack.push_back(GUIS_CHARSEL);
		      break;
    case MENU_HARD:   race_manager->setDifficulty(RD_HARD);
                      guiStack.push_back(GUIS_CHARSEL);
		      break;
    default: break;
  }   // switch
}   // select


