//  $Id$
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

#include "PlayerControls.h"
#include "tuxkart.h"
#include "WidgetSet.h"
#include "Config.h"

#include <string>

PlayerControls::PlayerControls(int whichPlayer):
        config_index(whichPlayer),
        grabInput(false)
{
    menu_id = widgetSet -> vstack(0);
    char output[60];
    sprintf(output, "Choose your controls, %s", config.player[config_index].name.c_str());
    widgetSet -> label(menu_id, output, GUI_LRG, GUI_ALL, 0, 0);

    int ha = widgetSet -> harray(menu_id);
    int change_id = widgetSet -> varray(ha);
    
    addKeyLabel(change_id, KC_LEFT, true);
    addKeyLabel(change_id, KC_RIGHT, false);
    addKeyLabel(change_id, KC_UP, false);
    addKeyLabel(change_id, KC_DOWN, false);
    addKeyLabel(change_id, KC_WHEELIE, false);
    addKeyLabel(change_id, KC_JUMP, false);
    addKeyLabel(change_id, KC_RESCUE, false);
    addKeyLabel(change_id, KC_FIRE, false);

    int label_id = widgetSet -> varray(ha);
    widgetSet -> label(label_id, "Left",  GUI_MED, GUI_ALL, 0, 0);
    widgetSet -> label(label_id, "Right",  GUI_MED, GUI_ALL, 0, 0);
    widgetSet -> label(label_id, "Accelerate",  GUI_MED, GUI_ALL, 0, 0);
    widgetSet -> label(label_id, "Brake",  GUI_MED, GUI_ALL, 0, 0);
    widgetSet -> label(label_id, "Pull wheelie",  GUI_MED, GUI_ALL, 0, 0);
    widgetSet -> label(label_id, "Jump",  GUI_MED, GUI_ALL, 0, 0);
    widgetSet -> label(label_id, "Call for rescue",  GUI_MED, GUI_ALL, 0, 0);
    widgetSet -> label(label_id, "Fire",  GUI_MED, GUI_ALL, 0, 0);

    widgetSet -> layout(menu_id, 0, 0);
}

PlayerControls::~PlayerControls()
{
    widgetSet -> delete_widget(menu_id) ;
}

void PlayerControls::update(float dt)
{

    widgetSet -> timer(menu_id, dt) ;
    widgetSet -> paint(menu_id) ;
}

void PlayerControls::select()
{
    if (grabInput)
        return;

    grab_id = widgetSet -> click();
    int menuChoice = widgetSet -> token (grab_id);

    if ( menuChoice == MENU_RETURN)
        guiStack.pop_back();
    else
    {
        editKey = static_cast<KartControl>(menuChoice);
        grabInput = true;
    }
}

void PlayerControls::keybd(const SDL_keysym& key)
{
    if (grabInput)
    {
        config.player[config_index].keys[editKey] = key.sym;
        grabInput = false;
        changeKeyLabel(grab_id, editKey);
    }
    else
        BaseGUI::keybd(key);
}

void PlayerControls::point(int x, int y)
{
    if (!grabInput)
        widgetSet -> pulse(widgetSet -> point(menu_id, x, y), 1.2f);
}

void PlayerControls::stick(int whichAxis, int value)
{
    if (!grabInput)
        widgetSet -> pulse(widgetSet -> stick(menu_id, whichAxis, value), 1.2f);
}

void PlayerControls::joybutton(int whichJoy, int button)
{
    if (grabInput && editKey != KC_LEFT && editKey != KC_RIGHT && whichJoy == config_index)
    {
        config.player[config_index].buttons[editKey] = button;
        grabInput = false;
        changeKeyLabel(grab_id, editKey);
    }
    else
        BaseGUI::joybutton(whichJoy, button);
}

void PlayerControls::addKeyLabel(int change_id, KartControl control, bool start)
{
    std::string currentKeys = getKeyInfoString(control);
    
    if (start)
        widgetSet -> start(change_id, currentKeys.c_str(), GUI_MED, control, 0);
    else
        widgetSet -> state(change_id, currentKeys.c_str(), GUI_MED, control, 0);
}

void PlayerControls::changeKeyLabel(int grab_id, KartControl control)
{
    std::string currentKeys = getKeyInfoString(control);
    
    widgetSet -> set_label(grab_id, currentKeys.c_str());
}

std::string PlayerControls::getKeyInfoString(KartControl control)
{
    std::string ret = SDL_GetKeyName(static_cast<SDLKey>(config.player[config_index].keys[control]));
    if (config.player[config_index].useJoy)
    {
        char joyInfo[60];
        if (control == KC_LEFT)
            sprintf(joyInfo, " or joystick left");
        else if (control == KC_RIGHT)
            sprintf(joyInfo, " or joystick right");
        else
            sprintf(joyInfo, " or joy button %d", config.player[config_index].buttons[control]);
        ret += joyInfo;
    }
    
    return ret;
}




