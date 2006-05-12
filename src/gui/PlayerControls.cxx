//  $Id: PlayerControls.cxx,v 1.3 2005/07/13 17:17:47 joh Exp $
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

#include <plib/pw.h>
#include "PlayerControls.h"
#include "WidgetSet.h"
#include "Config.h"

#include <string>

PlayerControls::PlayerControls(int whichPlayer): player_index(whichPlayer),
						 grabInput(false)          {
  menu_id = widgetSet -> vstack(0);
  sprintf(Heading, "Choose your controls, %s",
	  config->player[player_index].name.c_str());
  widgetSet -> label(menu_id, Heading, GUI_LRG, GUI_ALL, 0, 0);

  int ha = widgetSet -> harray(menu_id);
  int change_id = widgetSet -> varray(ha);

  for(int i=KC_LEFT; i<=KC_FIRE; i++) {
    addKeyLabel(change_id, (KartActions)i,    i==0 );
  }

  int label_id = widgetSet -> varray(ha);
  widgetSet -> label(label_id, "Left",             GUI_MED, GUI_ALL, 0, 0);
  widgetSet -> label(label_id, "Right",            GUI_MED, GUI_ALL, 0, 0);
  widgetSet -> label(label_id, "Accelerate",       GUI_MED, GUI_ALL, 0, 0);
  widgetSet -> label(label_id, "Brake",            GUI_MED, GUI_ALL, 0, 0);
  widgetSet -> label(label_id, "Pull wheelie",     GUI_MED, GUI_ALL, 0, 0);
  widgetSet -> label(label_id, "Jump",             GUI_MED, GUI_ALL, 0, 0);
  widgetSet -> label(label_id, "Call for rescue",  GUI_MED, GUI_ALL, 0, 0);
  widgetSet -> label(label_id, "Fire",             GUI_MED, GUI_ALL, 0, 0);

  widgetSet -> layout(menu_id, 0, 0);
}   // PlayerControls

// -----------------------------------------------------------------------------
PlayerControls::~PlayerControls() {
  widgetSet -> delete_widget(menu_id) ;
}   // ~PlayerControls

// -----------------------------------------------------------------------------
void PlayerControls::update(float dt) {

  widgetSet -> timer(menu_id, dt) ;
  widgetSet -> paint(menu_id) ;
}   // update

// -----------------------------------------------------------------------------
void PlayerControls::select() {
  if (grabInput) return;

  grab_id        = widgetSet -> click();
  int menuChoice = widgetSet -> token (grab_id);

  if ( menuChoice == MENU_RETURN) {
    config->saveConfig();
    guiStack.pop_back();
  } else {
    editAction   = static_cast<KartActions>(menuChoice);
    grabInput = true;
    widgetSet->set_label(grab_id,"Press key");
  }
}   // select

// -----------------------------------------------------------------------------
void PlayerControls::keybd(int key) {
  if (grabInput) {
    //    printf("Setting %d: %lx: from %d to %d\n",
    //	   player_index, config->player[player_index],config->player[player_index].keys[editAction],key);
    config->player[player_index].keys[editAction] = key;
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
          config->player[player_index].buttons[editAction] = presses;
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
    widgetSet -> start(change_id, KeyNames[control].c_str(), GUI_MED,
		       control, 0);
  else
    widgetSet -> state(change_id, KeyNames[control].c_str(), GUI_MED,
		       control, 0);
}   // addKeyLabel

// -----------------------------------------------------------------------------
void PlayerControls::changeKeyLabel(int grab_id, KartActions control) {
  setKeyInfoString(control);
  widgetSet -> set_label(grab_id, KeyNames[control].c_str());
  //  widgetSet -> layout(menu_id, 0, 0);
}   // changeKeyLabel

// -----------------------------------------------------------------------------
void PlayerControls::setKeyInfoString(KartActions control) {
  std::string ret;
  int key = config->player[player_index].keys[control];

  switch (key) {
    case PW_KEY_RIGHT     : ret="right"  ; break;
    case PW_KEY_LEFT      : ret="left"   ; break;
    case PW_KEY_UP        : ret="up"     ; break;
    case PW_KEY_DOWN      : ret="down"   ; break;
    case PW_KEY_F1        : ret="F1"     ; break;
    case PW_KEY_F2        : ret="F2"     ; break;
    case PW_KEY_F3        : ret="F3"     ; break;
    case PW_KEY_F4        : ret="F4"     ; break;
    case PW_KEY_F5        : ret="F5"     ; break;
    case PW_KEY_F6        : ret="F6"     ; break;
    case PW_KEY_F7        : ret="F7"     ; break;
    case PW_KEY_F8        : ret="F8"     ; break;
    case PW_KEY_F9        : ret="F9"     ; break;
    case PW_KEY_F10       : ret="F10"    ; break;
    case PW_KEY_F11       : ret="F11"    ; break;
    case PW_KEY_F12       : ret="F12"    ; break;
    case PW_KEY_PAGE_UP   : ret="pg up"  ; break;
    case PW_KEY_PAGE_DOWN : ret="pg down"; break;
    case PW_KEY_HOME      : ret="home"   ; break;
    case PW_KEY_END       : ret="end"    ; break;
    case PW_KEY_INSERT    : ret="insert" ; break;
    case 13               : ret="enter"  ; break;
    case 32               : ret="space"  ; break;
    default:                ret=" ";
                            ret[0]=key;
  }
  if (config->player[player_index].useJoy) {
    char joyInfo[60];
    if      (control == KC_LEFT)  sprintf(joyInfo, " or stick left");
    else if (control == KC_RIGHT) sprintf(joyInfo, " or stick right");
    else if (control == KC_ACCEL) sprintf(joyInfo, " or stick up");
    else
    {
        const int TARGET_BUTTON = config->player[player_index].buttons[control];
        int button_number = 0;
        while((1 << button_number != TARGET_BUTTON) && (button_number < 16))
            ++button_number;

        if(button_number < 16)
            sprintf(joyInfo, " or joybutton %d", button_number + 1 );
        else
            sprintf(joyInfo, " or unassigned" );

    }
    ret += joyInfo;
  }
  KeyNames[control] = ret;

}   // setKeyInfoString




