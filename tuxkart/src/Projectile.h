//  $Id: Projectile.h,v 1.2 2004/08/15 13:57:55 grumbel Exp $
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

#ifndef HEADER_PROJECTILE_H
#define HEADER_PROJECTILE_H

#include "Driver.h"

class Projectile : public Driver
{
  KartDriver *owner ;
  int type ;

  void setType ( int _type )
  {
    type = _type ;

    switch ( type )
    {
      case COLLECT_NOTHING :
      case COLLECT_ZIPPER  :
      case COLLECT_MAGNET  :
        ((ssgSelector *)(getModel () -> getKid ( 0 ))) -> select ( 0 ) ;
        break ;

      case COLLECT_SPARK :
        ((ssgSelector *)(getModel () -> getKid ( 0 ))) -> selectStep ( 0 ) ;
        break ;

      case COLLECT_MISSILE :
        ((ssgSelector *)(getModel () -> getKid ( 0 ))) -> selectStep ( 1 ) ;
        break ;

      case COLLECT_HOMING_MISSILE :
        ((ssgSelector *)(getModel () -> getKid ( 0 ))) -> selectStep ( 2 ) ;
        break ;
    }
  }

public:
  Projectile ( ) : Driver ( )
  {
    type = COLLECT_NOTHING ;
  }

  virtual ~Projectile() {}

  void off ()
  {
    setType ( COLLECT_NOTHING ) ;
  }

  void fire ( KartDriver *who, int _type );

  virtual void doObjectInteractions () ;
  virtual void doLapCounting        () ;
  virtual void doZipperProcessing   () ;
  virtual void doCollisionAnalysis  ( float delta , float hot ) ;
  virtual void update               ( float delta ) ;

} ;

#endif

/* EOF */
