//  $Id: Explosion.cxx,v 1.4 2004/08/11 00:36:19 grumbel Exp $
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

#include "tuxkart.h"
#include "Explosion.h"
#include "World.h"

static sgVec3 hell = { -1000000, -1000000, -1000000 } ;

static ssgSelector *find_selector ( ssgBranch *b )
{
  if ( b == NULL )
    return NULL ;

  if ( ! b -> isAKindOf ( ssgTypeBranch () ) )
    return NULL ;

  if ( b -> isAKindOf ( ssgTypeSelector () ) )
    return (ssgSelector *) b ;

  for ( int i = 0 ; i < b -> getNumKids() ; i++ )
  {
    ssgSelector *res = find_selector ( (ssgBranch *)(b ->getKid(i)) ) ;

    if ( res != NULL )
      return res ;
  }

  return NULL ;
}
 

Explosion::Explosion ( ssgBranch *b )
{
  ssgSelector *e = find_selector ( b ) ;
  ssgCutout *cut ;

  if ( e == NULL )
  {
    fprintf ( stderr, "Explode.ac doesn't have an 'explosion' object.\n" ) ;
    exit ( 1 ) ;
  }

  step = -1 ;

  dcs = new ssgTransform ;
  cut = new ssgCutout ;
  seq = (ssgSelector *) e ;

  World::current()->scene -> addKid ( dcs ) ;
  dcs   -> addKid ( cut ) ;
  cut   -> addKid ( seq ) ;

  dcs -> setTransform ( hell ) ;
  seq -> select ( 0 ) ;
}



void Explosion::update ()
{
  if ( step < 0 )
  {
    dcs -> setTransform ( hell ) ;
    return ;
  }

  seq -> selectStep ( step ) ;

  if ( ++step >= 16 )
    step = -1 ;
}


