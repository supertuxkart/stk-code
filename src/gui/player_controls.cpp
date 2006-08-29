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

#include <plib/pw.h>
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
void PlayerControls::keybd(int key) {
  if (grabInput) {
    //    printf("Setting %d: %lx: from %d to %d\n",
    //	   player_index, config->player[player_index],config->player[player_index].keys[editAction],key);
    config->player[player_index].setKey(editAction, key);
    grabInput = false;
    changeKeyLabel(grab_id, editAction);
  } else
    BaseGUI::keybd(key);
}   // keybd

// -----------------------------------------------------------------------------
void PlayerControls::point(int x, int y) {
    if (!grabInput)
        widgetSet -> pulse(widgetSet -> point(menu_id, x, y), 1.2f);
}   // point

// -----------------------------------------------------------------------------
void PlayerControls::stick(const int &whichAxis, const float &value) {
  if (!grabInput)
    widgetSet -> pulse(widgetSet -> stick(menu_id, whichAxis, (int)value), 1.2f);
}   // stick

// -----------------------------------------------------------------------------
void PlayerControls::joybuttons(int whichJoy, int hold, int presses,
                                int releases) {
  if(grabInput)
  {
      if (editAction != KC_LEFT && editAction != KC_RIGHT &&
          editAction != KC_ACCEL && whichJoy == player_index && presses)
      {
          config->player[player_index].setButton(editAction, presses);
          grabInput = false;
          changeKeyLabel(grab_id, editAction);
      }
  }
  else
  {
      BaseGUI::joybuttons(whichJoy, hold, presses, releases);
  }
}   // joybuttons

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
  KeyNames[control] = config->GetKeyAsString(player_index, control);
}   // setKeyInfoString
