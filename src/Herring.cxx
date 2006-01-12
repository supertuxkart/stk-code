//  $Id$
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

#include <cassert>
#include "tuxkart.h"
#include "material.h"
#include "Shadow.h"
#include "Herring.h"
#include "World.h"

Herring::Herring ( sgVec3 colour )
  : rotation(0)
{
  // create a cube
  ssgVertexArray   *va = new ssgVertexArray   () ; sgVec3 v ;
  ssgNormalArray   *na = new ssgNormalArray   () ; sgVec3 n ;
  ssgColourArray   *ca = new ssgColourArray   () ; sgVec4 c ;
  ssgTexCoordArray *ta = new ssgTexCoordArray () ; sgVec2 t ;

  sgSetVec3(v, -0.5, 0.0, 0.0 ) ; va->add(v) ;
  sgSetVec3(v,  0.5, 0.0, 0.0 ) ; va->add(v) ;
  sgSetVec3(v, -0.5, 0.0, 0.5 ) ; va->add(v) ;
  sgSetVec3(v,  0.5, 0.0, 0.5 ) ; va->add(v) ;
  sgSetVec3(v, -0.5, 0.0, 0.0 ) ; va->add(v) ;
  sgSetVec3(v,  0.5, 0.0, 0.0 ) ; va->add(v) ;

  sgSetVec3(n,  0.0f,  1.0f,  0.0f ) ; na->add(n) ;

  sgCopyVec3 ( c, colour ) ; c[ 3 ] = 1.0f ; ca->add(c) ;
 
  sgSetVec2(t, 0.0, 0.0 ) ; ta->add(t) ;
  sgSetVec2(t, 1.0, 0.0 ) ; ta->add(t) ;
  sgSetVec2(t, 0.0, 1.0 ) ; ta->add(t) ;
  sgSetVec2(t, 1.0, 1.0 ) ; ta->add(t) ;
  sgSetVec2(t, 0.0, 0.0 ) ; ta->add(t) ;
  sgSetVec2(t, 1.0, 0.0 ) ; ta->add(t) ;
 
  ssgVtxTable* cube = new ssgVtxTable ( GL_TRIANGLE_STRIP, va, na, ta, ca ) ;
  cube -> setState ( getMaterial ( "herring.png" ) -> getState () ) ;

  shadow = createShadow ( "fuzzy.png", -0.5, 0.5, -0.25, 0.25 ) ;
#if 0
  shadow->ref();
#endif
 
  transform = new ssgTransform () ;
  transform->ref();
  transform -> addKid ( shadow ) ;
  transform -> addKid ( cube ) ;
}

Herring::Herring ( ssgEntity* model )
  : rotation(0)
{
  //turn off collision detection. 
  model -> clrTraversalMaskBits ( SSGTRAV_ISECT|SSGTRAV_HOT ) ; 

  shadow = createShadow ( "fuzzy.png", -0.5, 0.5, -0.25, 0.25 ) ;
#if 0
  shadow -> ref ();
#endif

  transform = new ssgTransform () ;
  transform -> ref () ;
  transform -> addKid ( shadow ) ;
  transform -> addKid ( model ) ;
}

Herring::~Herring ()
{
#if 0
  ssgDeRefDelete(shadow);
#endif
  ssgDeRefDelete(transform);
}

void Herring::update ()
{
  sgCoord c = { { 0.0, 0.0, 0.0 }, { 0.0, 0.0, 0.0 } } ;

  c . hpr [ 0 ] = rotation ;
  transform -> setTransform ( &c ) ;

  rotation += 5.0f ;
}

void HerringInstance::update ()
{
#if 0
  if ( ! eaten || her == NULL ) return ;
#endif
  if(!eaten) return;

  assert(her != NULL);

  float t = time_to_return - world -> clock;
  if ( t > 0 )
  {
    sgVec3 hell ;
    sgCopyVec3 ( hell, xyz ) ;

    if ( t > 1.0f )
      hell [ 2 ] = -1000000.0f ;
    else
      hell [ 2 ] = -t / 2.0f ;

    scs -> setTransform ( hell ) ;
  }
  else
  {
    eaten = FALSE ;
    scs -> setTransform ( xyz ) ;
  }
}

/* EOF */
