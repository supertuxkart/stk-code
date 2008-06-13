//  $Id$
//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2004-2005 Steve Baker <sjbaker1@airmail.net>
//  Copyright (C) 2006 SuperTuxKart-Team, Steve Baker
//
//  This program is free software; you can redistribute it and/or
//  modify it under the terms of the GNU General Public License
//  as published by the Free Software Foundation; either version 3
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

#include "material_manager.hpp"
#include "material.hpp"
#include "shadow.hpp"

ssgTransform* createShadow( const std::string& name,
                            float x1, float x2, float y1, float y2 )
{
    ssgVertexArray   *va = new ssgVertexArray   () ; sgVec3 v ;
    ssgNormalArray   *na = new ssgNormalArray   () ; sgVec3 n ;
    ssgColourArray   *ca = new ssgColourArray   () ; sgVec4 c ;
    ssgTexCoordArray *ta = new ssgTexCoordArray () ; sgVec2 t ;

    sgSetVec4 ( c, 0.0f, 0.0f, 0.0f, 1.0f ) ; ca->add(c) ;
    sgSetVec3 ( n, 0.0f, 0.0f, 1.0f ) ; na->add(n) ;

    sgSetVec3 ( v, x1, y1, 0.05f ) ; va->add(v) ;
    sgSetVec3 ( v, x2, y1, 0.05f ) ; va->add(v) ;
    sgSetVec3 ( v, x1, y2, 0.05f ) ; va->add(v) ;
    sgSetVec3 ( v, x2, y2, 0.05f ) ; va->add(v) ;

    sgSetVec2 ( t, 0.0f, 0.0f ) ; ta->add(t) ;
    sgSetVec2 ( t, 1.0f, 0.0f ) ; ta->add(t) ;
    sgSetVec2 ( t, 0.0f, 1.0f ) ; ta->add(t) ;
    sgSetVec2 ( t, 1.0f, 1.0f ) ; ta->add(t) ;

    ssgTransform* result = new ssgTransform ;
    result -> clrTraversalMaskBits ( SSGTRAV_ISECT|SSGTRAV_HOT ) ;

    result -> setName ( "Shadow" ) ;

    ssgVtxTable *gs = new ssgVtxTable ( GL_TRIANGLE_STRIP, va, na, ta, ca ) ;

    gs -> clrTraversalMaskBits ( SSGTRAV_ISECT|SSGTRAV_HOT ) ;
    gs -> setState ( material_manager->getMaterial ( name.c_str() ) -> getState () ) ;

    result -> addKid ( gs ) ;

    return result;
}

/* EOF */
