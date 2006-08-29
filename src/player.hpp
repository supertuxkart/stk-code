// $Id: player.hpp,v 1.2 2005/07/13 17:18:53 joh Exp $
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

#ifndef TUXKART_PLAYER_H
#define TUXKART_PLAYER_H

#include <string>

// Some part (e.g. gui/PlayerControls) depend on KC_LEFT being the first
// entry, and KC_FIRE being the last - so any action added should be
// added in between those two values.
enum KartActions { KC_LEFT,
		   KC_RIGHT,
		   KC_ACCEL,
		   KC_BRAKE,
		   KC_WHEELIE,
		   KC_JUMP,
		   KC_RESCUE,
		   KC_FIRE };

extern char *sKartAction2String[KC_FIRE+1];
/*class for managing player name and control configuration*/
class Player
{
private:
    std::string name;
    bool useJoy;    //player is using a joystick
    int keys[8]; //keyboard keymap and joystick button map
    int buttons[8];

public:
    Player(){}
    Player(const std::string &name_):name(name_){}
    void setName(const std::string &name_){name = name_;}
    void setKey(KartActions action, int key){keys[action]=key;}
    void setButton(KartActions action, int button){buttons[action]=button;}
    void setUseJoystick(bool set){useJoy= set;}
    
    const char* getName() {return name.c_str();}
	int getKey(KartActions action){return keys[action];}
    int getButton(KartActions action){return buttons[action];}
    bool IsUsingJoystick() {return useJoy; }
};

#endif

/*EOF*/
