// $Id$
//
//  SuperTuxKart - a fun racing game with go-kart
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

#include "Player.h"


Player::Player()
{
}


Player::Player(const std::string &name)
{
  setName(name);
}


void
Player::setName(const std::string &name)
{
  Player::name=name;
}

//FIXMEJAMES: never used???
void
Player::setKeys(bool joystick,
                     int left,
                     int right,
                     int up,
                     int down,
                     int wheelie,
                     int jump,
                     int rescue,
                     int fire)
{
	if (joystick) {
		buttons[KC_LEFT]    = left;
		buttons[KC_RIGHT]   = right;
		buttons[KC_UP]      = up;
		buttons[KC_DOWN]    = down;
		buttons[KC_WHEELIE] = wheelie;
		buttons[KC_JUMP]    = jump;
		buttons[KC_FIRE]    = fire;
	}
	else {
		keys[KC_LEFT]    = left;
		keys[KC_RIGHT]   = right;
		keys[KC_UP]      = up;
		keys[KC_DOWN]    = down;
		keys[KC_WHEELIE] = wheelie;
		keys[KC_JUMP]    = jump;
		keys[KC_RESCUE]  = rescue;
		keys[KC_FIRE]    = fire;
  }
}

void Player::setKey(KartControl action, int key)
{
  keys[action]=key;
}

void Player::setButton(KartControl action, int button)
{
  buttons[action]=button;
}

/*EOF*/
