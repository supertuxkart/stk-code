//  $Id: Shadow.cxx,v 1.1 2004/08/08 03:14:17 grumbel Exp $
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

#include "material.h"
#include "Shadow.h"

Shadow::Shadow ( const std::string& name, float x1, float x2, float y1, float y2 )
{
  ssgVertexArray   *va = new ssgVertexArray   () ; sgVec3 v ;
  ssgNormalArray   *na = new ssgNormalArray   () ; sgVec3 n ;
  ssgColourArray   *ca = new ssgColourArray   () ; sgVec4 c ;
  ssgTexCoordArray *ta = new ssgTexCoordArray () ; sgVec2 t ;

  sgSetVec4 ( c, 0.0f, 0.0f, 0.0f, 1.0f ) ; ca->add(c) ;
  sgSetVec3 ( n, 0.0f, 0.0f, 1.0f ) ; na->add(n) ;
 
  sgSetVec3 ( v, x1, y1, 0.10 ) ; va->add(v) ;
  sgSetVec3 ( v, x2, y1, 0.10 ) ; va->add(v) ;
  sgSetVec3 ( v, x1, y2, 0.10 ) ; va->add(v) ;
  sgSetVec3 ( v, x2, y2, 0.10 ) ; va->add(v) ;
 
  sgSetVec2 ( t, 0.0, 0.0 ) ; ta->add(t) ;
  sgSetVec2 ( t, 1.0, 0.0 ) ; ta->add(t) ;
  sgSetVec2 ( t, 0.0, 1.0 ) ; ta->add(t) ;
  sgSetVec2 ( t, 1.0, 1.0 ) ; ta->add(t) ;
 
  sh = new ssgBranch ;
  sh -> clrTraversalMaskBits ( SSGTRAV_ISECT|SSGTRAV_HOT ) ;
 
  sh -> setName ( "Shadow" ) ;
 
  ssgVtxTable *gs = new ssgVtxTable ( GL_TRIANGLE_STRIP, va, na, ta, ca ) ;
 
  gs -> clrTraversalMaskBits ( SSGTRAV_ISECT|SSGTRAV_HOT ) ;

  // FIXME: necessary since getMaterial modifies the argument
  char* name_c = strdup(name.c_str());
  gs -> setState ( getMaterial ( name_c ) -> getState () ) ;
  free(name_c);

  sh -> addKid ( gs ) ;
  sh -> ref () ; /* Make sure it doesn't get deleted by mistake */
}

/* EOF */
