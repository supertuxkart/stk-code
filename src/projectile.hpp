//  $Id: Projectile.h,v 1.7 2005/08/19 20:51:56 joh Exp $
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

#ifndef HEADER_PROJECTILE_H
#define HEADER_PROJECTILE_H

#include "moveable.hpp"

class Kart;

class Projectile : public Moveable {
  sgCoord     last_pos;
  const Kart* owner;
  float       speed;    // Speed of the projectile
  int         type ;
  bool        hasHitSomething;

public:

  Projectile               (Kart* kart_, int type);
  virtual ~Projectile();
  void init                (Kart* kart_, int type);
  void update              (float);
  void doCollisionAnalysis (float dt, float hot);
  void doObjectInteractions();
  void explode             ();
  bool hasHit              ()            {return hasHitSomething;            }
  void reset               ()            {Moveable::reset();
                                          sgCopyCoord(&last_pos,&reset_pos );}
  void OutsideTrack        (int isReset) {explode();                         }

} ;



#endif
