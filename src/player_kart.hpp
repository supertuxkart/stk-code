//  $Id: player_kart.hpp,v 1.8 2005/09/30 16:52:27 joh Exp $
//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2004-2005 Steve Baker <sjbaker1@airmail.net>
//  Copyright (C) 2006 Joerg Henrichs, Steve Baker
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


#ifndef HEADER_PLAYERKART_H
#define HEADER_PLAYERKART_H

#include "kart.hpp"

class Player;

/** PlayerKart manages control events from the player and moves
    them to the Kart */
class PlayerKart : public Kart {
 private:
  Player *player;
  float  penaltyTime;
  bool   joystickWasMoved;

  void handleKeyboard(float dt);
 public:
  PlayerKart(const KartProperties *kart_properties,
	     int position, Player *_player) :
    Kart(kart_properties, position), player(_player), 
    penaltyTime(0.0), joystickWasMoved(false)        {}

  int     earlyStartPenalty () {return penaltyTime>0; }
  Player* getPlayer         () {return player;        }
  void    update            (float);
  void    incomingJoystick  (const KartControl &ctrl);
  void    action            (int key);
  void    forceCrash        ();
  void    handleZipper      ();
  void    collectedHerring  (Herring* herring);
};

#endif

/* EOF */
