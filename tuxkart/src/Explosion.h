//  $Id: Explosion.h,v 1.5 2004/09/24 15:45:02 matzebraun Exp $
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

#ifndef HEADER_EXPLOSION_H
#define HEADER_EXPLOSION_H

#include <plib/ssg.h>
#include "tuxkart.h"
#include "sound.h"

class World;

class Explosion
{
  World* world;
  ssgTransform *dcs ;
  ssgSelector  *seq ;

  int step ;

public:

  Explosion ( World* world, ssgBranch *b ) ;

  void update () ;
  void start  ( sgVec3 where )
  {
    sound -> playSfx ( SOUND_EXPLOSION ) ;
    dcs -> setTransform ( where ) ;
    step = 0 ;
  }

  int  inUse  () { return (step >= 0) ; }
} ;

#endif

/* EOF */
