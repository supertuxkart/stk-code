//  $Id: material.cpp,v 1.5 2005/08/23 19:55:28 joh Exp $
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

#include "loader.hpp"
#include "material.hpp"
#include "material_manager.hpp"

#define UCLAMP   1
#define VCLAMP   2

int clearSpheremap ( ssgEntity * )
{
  glDisable   ( GL_TEXTURE_GEN_S ) ;
  glDisable   ( GL_TEXTURE_GEN_T ) ;
  return true ;
}


int setSpheremap ( ssgEntity * )
{
  glTexGeni   ( GL_S, GL_TEXTURE_GEN_MODE, GL_SPHERE_MAP ) ;
  glTexGeni   ( GL_T, GL_TEXTURE_GEN_MODE, GL_SPHERE_MAP ) ;
  glEnable    ( GL_TEXTURE_GEN_S ) ;
  glEnable    ( GL_TEXTURE_GEN_T ) ;
  return true ;
}


bool Material::parseBool ( char **p )
{
  /* Skip leading spaces */

  while ( **p <= ' ' && **p != '\0' ) (*p)++ ;

  bool res = ( ( **p == 'Y' ) || ( **p == 'y' ) ) ;

  while ( **p > ' ' && **p != '\0' ) (*p)++ ;

  return res ;
}


int Material::parseInt ( char **p )
{
  /* Skip leading spaces */

  while ( **p <= ' ' && **p != '\0' ) (*p)++ ;

  return strtol ( *p, p, 0 ) ;
}



float Material::parseFloat ( char **p )
{
  /* Skip leading spaces */

  while ( **p <= ' ' && **p != '\0' ) (*p)++ ;

  return strtod ( *p, p ) ;
}



Material::Material ()
{
  texname = new char [ 1 ] ;
  texname [ 0 ] = '\0' ;
  predraw  = postdraw = NULL ;

  init    () ;
  install () ;
}


Material::Material ( char *fname, char *description )
{
  texname = new char [ strlen ( fname ) + 1 ] ;
  strcpy ( texname, fname ) ;
  predraw  = postdraw = NULL ;

  init () ;

  clamp_tex    = parseBool  ( & description ) ? UCLAMP : 0 ;
  clamp_tex   += parseBool  ( & description ) ? VCLAMP : 0 ;

  transparency = parseBool  ( & description ) ;
  alpha_ref    = parseFloat ( & description ) ;
  lighting     = parseBool  ( & description ) ;
  spheremap    = parseBool  ( & description ) ;
  friction     = parseFloat ( & description ) ;
  ignore       = parseBool  ( & description ) ;
  zipper       = parseBool  ( & description ) ;
  resetter     = parseBool  ( & description ) ;
  collideable  = parseBool  ( & description ) ;

  install () ;
}

Material::~Material()
{
  ssgDeRefDelete(state);
  delete[] texname;
}

void Material::init ()
{
  index = material_manager -> addEntity ( this ) ;
  clamp_tex    = 0     ;
  transparency = false ;
  alpha_ref    = 0.1   ;
  lighting     = true  ;
  spheremap    = false ;
  friction     = 1.0   ;
  ignore       = false ;
  zipper       = false ;
  resetter     = false ;
  collideable  = true  ;
}


void Material::applyToLeaf ( ssgLeaf *l )
{
  if ( predraw  ) l -> setCallback ( SSG_CALLBACK_PREDRAW , predraw  ) ;
  if ( postdraw ) l -> setCallback ( SSG_CALLBACK_POSTDRAW, postdraw ) ;
}


void Material::install ()
{
  if ( isSphereMap () )
  {
    predraw  =   setSpheremap ;
    postdraw = clearSpheremap ;
  }

  state = new ssgSimpleState ;
  
  state -> ref () ;
  state -> setExternalPropertyIndex ( index ) ;
  if ( texname != NULL && texname [ 0 ] != '\0' )
  {
    char fn [ 1024 ] ;
    sprintf ( fn, "images/%s", texname ) ;
    state -> setTexture ( loader->getPath(fn).c_str(), !(clamp_tex & UCLAMP),
		                                       !(clamp_tex & VCLAMP) );
    state -> enable  ( GL_TEXTURE_2D ) ;
  }
  else
    state -> disable ( GL_TEXTURE_2D ) ;

  if ( lighting )
    state -> enable  ( GL_LIGHTING ) ;
  else
    state -> disable ( GL_LIGHTING ) ;

  state -> setShadeModel ( GL_SMOOTH ) ;
  state -> enable        ( GL_COLOR_MATERIAL ) ;
  state -> enable        ( GL_CULL_FACE      ) ;
  state -> setColourMaterial ( GL_AMBIENT_AND_DIFFUSE ) ;
  state -> setMaterial   ( GL_EMISSION, 0, 0, 0, 1 ) ;
  state -> setMaterial   ( GL_SPECULAR, 0, 0, 0, 1 ) ;
  state -> setShininess  ( 0 ) ;

  if ( transparency )
  {
    state -> setTranslucent () ;
    state -> enable         ( GL_ALPHA_TEST ) ;
    state -> setAlphaClamp  ( alpha_ref ) ;
    state -> enable         ( GL_BLEND ) ;
  }
  else
  {
    state -> setOpaque () ;
    state -> disable   ( GL_BLEND ) ;
  }
}



