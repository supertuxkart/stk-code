//  $Id$
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

#include <plib/ssg.h>
#include "preprocessor.hpp"
#include "material_manager.hpp"
#include "material.hpp"

void preProcessObj ( ssgEntity *n, bool mirror )
{
  if ( n == NULL ) return ;

  n -> dirtyBSphere () ;

  if ( n -> isAKindOf ( ssgTypeLeaf() ) )
  {
    if ( mirror )
      for ( int i = 0 ; i < ((ssgLeaf *)n) -> getNumVertices () ; i++ )
        ((ssgLeaf *)n) -> getVertex ( i ) [ 0 ] *= -1.0f ;

    material_manager->getMaterial ( (ssgLeaf *) n ) -> applyToLeaf ( (ssgLeaf *) n ) ;
    return ;
  }

  if ( mirror && n -> isAKindOf ( ssgTypeTransform () ) )
  {
    sgMat4 xform ;

    ((ssgTransform *)n) -> getTransform ( xform ) ;
    xform [ 0 ][ 0 ] *= -1.0f ;
    xform [ 1 ][ 0 ] *= -1.0f ;
    xform [ 2 ][ 0 ] *= -1.0f ;
    xform [ 3 ][ 0 ] *= -1.0f ;
    ((ssgTransform *)n) -> setTransform ( xform ) ;
  }

  ssgBranch *b = (ssgBranch *) n ;

  for ( int i = 0 ; i < b -> getNumKids () ; i++ )
    preProcessObj ( b -> getKid ( i ), mirror ) ;
}
