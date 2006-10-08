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

#include <SDL/SDL.h>

#include "player_controls.hpp"
#include "widget_set.hpp"
#include "config.hpp"
#include "menu_manager.hpp"

#include <string>

char *sKartAction2String[KC_FIRE+1] = {"Left", "Right", "Accelerate", 
				       "Brake","Wheelie", "Jump", 
				       "Rescue", "Fire"};


PlayerControls::PlayerControls(int whichPlayer): player_index(whichPlayer),
						 grabInput(false)          {
  menu_id = widgetSet -> vstack(0);
  sprintf(Heading, "Choose your controls, %s", config->player[player_index].getName());
  widgetSet -> label(menu_id, Heading, GUI_LRG, GUI_ALL, 0, 0);

  int ha        = widgetSet->harray(menu_id);
  int change_id = widgetSet->varray(ha);
  int label_id  = widgetSet->varray(ha);

  for(int i=KC_LEFT; i<=KC_FIRE; i++) {
    //FIXME: this is temporal, just while the jumping is disabled.
    if(i==KC_JUMP) continue;
    addKeyLabel(change_id, (KartActions)i,    i==0 );
    widgetSet->label(label_id, sKartAction2String[i]);
  }

  widgetSet->state(menu_id,"Press <ESC> to go back", GUI_SML, -1);
  widgetSet -> layout(menu_id, 0, 0);
}   // PlayerControls

// -----------------------------------------------------------------------------
PlayerControls::~PlayerControls() {
  widgetSet -> delete_widget(menu_id) ;
}   // ~PlayerControls

// -----------------------------------------------------------------------------
void PlayerControls::select() {
  if (grabInput) return;

  grab_id        = widgetSet -> click();
  int menuChoice = widgetSet -> token (grab_id);
  if(menuChoice==-1) {
    menu_manager->popMenu();
    return;
  }

  editAction   = static_cast<KartActions>(menuChoice);
  grabInput = true;
  widgetSet->set_label(grab_id,"Press key");
}   // select

// -----------------------------------------------------------------------------
void PlayerControls::input(InputType type, int id0, int id1, int id2, int value)
{
  if (grabInput && value)
  {
    grabInput = false;

    // Do not accept pressing ESC as input.
    if (type != IT_KEYBOARD && id0 != SDLK_ESCAPE)
      config->player[player_index].setInput(editAction, type, id0, id1, id2);

    changeKeyLabel(grab_id, editAction);
  }
  else
    BaseGUI::input(type, id0, id1, id2, value);
}

// -----------------------------------------------------------------------------
void PlayerControls::addKeyLabel(int change_id, KartActions control, bool start)
{

  setKeyInfoString(control);

  if (start)
    widgetSet -> start(change_id, KeyNames[control].c_str(), GUI_MED, control);
  else
    widgetSet -> state(change_id, KeyNames[control].c_str(), GUI_MED, control);
}   // addKeyLabel

// -----------------------------------------------------------------------------
void PlayerControls::changeKeyLabel(int grab_id, KartActions control) {
  setKeyInfoString(control);
  widgetSet -> set_label(grab_id, KeyNames[control].c_str());
  //  widgetSet -> layout(menu_id, 0, 0);
}   // changeKeyLabel

// -----------------------------------------------------------------------------
void PlayerControls::setKeyInfoString(KartActions control) {
  KeyNames[control] = config->getInputAsString(player_index, control);
}   // setKeyInfoString
