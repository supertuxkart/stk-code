// $Id: Player.h,v 1.4 2004/10/11 13:40:07 jamesgregory Exp $
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

#ifndef TUXKART_PLAYER_H
#define TUXKART_PLAYER_H

#include <string>

enum KartControl { KC_LEFT,
                   KC_RIGHT,
                   KC_UP,
                   KC_DOWN,
                   KC_WHEELIE,
                   KC_JUMP,
                   KC_RESCUE,
                   KC_FIRE };

/*class for managing player name and control configuration*/
class Player
{
public:
    std::string name;
    bool useJoy;    //player is using a joystick
    int keys[8]; //keyboard keymap and joystick button map
    int buttons[8];

    Player();
    Player(const std::string &name);
    void setName(const std::string &name);
    void setKeys(bool joystick,
                 int left,
                 int right,
                 int up,
                 int down,
                 int wheelie,
                 int jump,
                 int rescue,
                 int fire);
    void setKey(KartControl action, int key);
    void setButton(KartControl action, int button);
};

#endif

/*EOF*/
