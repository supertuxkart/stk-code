//  $Id: PlayerControls.cxx,v 1.4 2004/08/29 19:50:45 oaf_thadres Exp $
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
		widgetSet -> start(change_id, SDL_GetKeyName((SDLKey)config.player[config_index].keys[CD_KEYBOARD][KC_LEFT]),    GUI_MED, KC_LEFT,    0);
		widgetSet -> state(change_id, SDL_GetKeyName((SDLKey)config.player[config_index].keys[CD_KEYBOARD][KC_RIGHT]),   GUI_MED, KC_RIGHT,   0);
		widgetSet -> state(change_id, SDL_GetKeyName((SDLKey)config.player[config_index].keys[CD_KEYBOARD][KC_UP]),      GUI_MED, KC_UP,      0);
		widgetSet -> state(change_id, SDL_GetKeyName((SDLKey)config.player[config_index].keys[CD_KEYBOARD][KC_DOWN]),    GUI_MED, KC_DOWN,    0);
		widgetSet -> state(change_id, SDL_GetKeyName((SDLKey)config.player[config_index].keys[CD_KEYBOARD][KC_WHEELIE]), GUI_MED, KC_WHEELIE, 0);
		widgetSet -> state(change_id, SDL_GetKeyName((SDLKey)config.player[config_index].keys[CD_KEYBOARD][KC_JUMP]),    GUI_MED, KC_JUMP,    0);
		widgetSet -> state(change_id, SDL_GetKeyName((SDLKey)config.player[config_index].keys[CD_KEYBOARD][KC_RESCUE]),  GUI_MED, KC_RESCUE,  0);
		widgetSet -> state(change_id, SDL_GetKeyName((SDLKey)config.player[config_index].keys[CD_KEYBOARD][KC_FIRE]),    GUI_MED, KC_FIRE,    0);
	
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
		config.player[config_index].keys[CD_KEYBOARD][editKey] = key.sym;
		grabInput = false;
		widgetSet -> set_label(grab_id, SDL_GetKeyName((SDLKey)config.player[config_index].keys[CD_KEYBOARD][editKey]));
	}
	else
	{
		switch ( key.sym )
		{
		case SDLK_LEFT:
		case SDLK_RIGHT:
		case SDLK_UP:
		case SDLK_DOWN:
			widgetSet -> pulse(widgetSet -> cursor(menu_id, key.sym), 1.2f);
			break;
			
		case SDLK_RETURN: select(); break;
		
		case SDLK_ESCAPE:
			guiStack.pop_back();
			
		default: break;
		}
	}
}

void PlayerControls::point(int x, int y)
{
	if (!grabInput)
		widgetSet -> pulse(widgetSet -> point(menu_id, x, y), 1.2f);
}

void PlayerControls::stick(int x, int y)
{
	if (!grabInput)
		widgetSet -> pulse(widgetSet -> stick(menu_id, x, y), 1.2f);
}




