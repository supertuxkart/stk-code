// $Id: Player.h,v 1.1 2004/09/01 02:21:24 oaf_thadres Exp $
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
enum ControlDevice { CD_KEYBOARD, CD_JOYSTICK };

/*class for managing player name and control configuration*/
class Player
{
  public:
    std::string name;
    bool useJoy;    //player is using a joystick
    int joystick;   //which joystick device belongs to player
    int keys[2][8]; //keyboard keymap and joystick button map

    Player();
    Player(const std::string &name);
    void setName(const std::string &name);
    void setKeys(enum ControlDevice device,
                 int left,
                 int right,
                 int up,
                 int down,
                 int wheelie,
                 int jump,
                 int rescue,
                 int fire);
    void setKey(enum ControlDevice device, enum KartControl action, int button);
};

#endif

/*EOF*/
