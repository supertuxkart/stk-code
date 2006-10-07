//  $Id$
//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2006 Joerg Henrichs
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

#include "help_menu.hpp"
#include "widget_set.hpp"
#include "race_manager.hpp"
#include "menu_manager.hpp"
#include "config.hpp"
#include "player.hpp"

HelpMenu::HelpMenu(){
  menu_id = widgetSet->vstack(0);
  widgetSet->multi(menu_id, 
"Finish the race before other drivers, by driving and using\n\
powerups from the blue boxes! Bananas will slow you down,\n\
coins will let you get more powerups, gold coins are better.\n\
At high speeds you can use wheelies to go even faster, but\n\
be careful because you won't be able to steer.\n\
If you get stuck somewhere or fall too far from the road,\n\
use the rescue button to get back on track.\n\
Current keys bindings for the first player:",GUI_SML );
  int ha        = widgetSet->harray(menu_id);
  int change_id = widgetSet->varray(ha);
  int label_id  = widgetSet->varray(ha);
  for(int i=KC_LEFT; i<=KC_FIRE; i++) {
    //FIXME: this is temporal, just while the jumping is disabled.
    if(i==KC_JUMP) continue;

    // *sigh* widget set stores only pointer to strings, so
    // to make sure that all key-strings are permanent, they
    // are assigned to an array allKeys within this object.
    allKeys[i]=config->getInputAsString(0, (KartActions)i);
    widgetSet->label(change_id, allKeys[i].c_str(),    GUI_SML);
    widgetSet->label(label_id,  sKartAction2String[i], GUI_SML);
  }
  widgetSet->start(menu_id,"Press <ESC> to go back", GUI_SML, 1);
  widgetSet->layout(menu_id, 0, 0);
}   // HelpMenu

// -----------------------------------------------------------------------------
HelpMenu::~HelpMenu() {
  widgetSet -> delete_widget(menu_id) ;
}   // ~HelpMenu
	
// -----------------------------------------------------------------------------
void HelpMenu::select() {
  // must be esc, nothing else is available. So just pop this menu
  menu_manager->popMenu();
}   // select

// -----------------------------------------------------------------------------
/* EOF */
