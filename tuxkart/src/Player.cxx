// $Id: Player.cxx,v 1.1 2004/09/01 02:21:24 oaf_thadres Exp $
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


void
Player::setKeys(enum ControlDevice device,
                     int left,
                     int right,
                     int up,
                     int down,
                     int wheelie,
                     int jump,
                     int rescue,
                     int fire)
{
  keys[device][KC_LEFT]    = left;
  keys[device][KC_RIGHT]   = right;
  keys[device][KC_UP]      = up;
  keys[device][KC_DOWN]    = down;
  keys[device][KC_WHEELIE] = wheelie;
  keys[device][KC_JUMP]    = jump;
  keys[device][KC_RESCUE]  = rescue;
  keys[device][KC_FIRE]    = fire;
}


/*associate a key or joystick button w/ an action*/
void
Player::setKey(enum ControlDevice device, enum KartControl action, int button)
{
  keys[device][action]=button;
}

/*EOF*/
