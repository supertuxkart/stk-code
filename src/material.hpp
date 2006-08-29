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

#ifndef HEADER_MATERIAL_H
#define HEADER_MATERIAL_H

#include <plib/ssg.h>

class Material
{
  ssgSimpleState *state ;
  ssgCallback predraw ;
  ssgCallback postdraw ;

  int   index ;

  char *texname      ;

  bool  collideable  ;
  bool  zipper       ;
  bool  resetter     ;
  bool  ignore       ;

  int   clamp_tex    ;
  bool  lighting     ;
  bool  spheremap    ;
  bool  transparency ;
  float alpha_ref    ;
  float friction     ;

  bool  parseBool  ( char **p ) ;
  int   parseInt   ( char **p ) ;
  float parseFloat ( char **p ) ;

  void init    () ;
  void install () ;

public:

  Material () ;
  Material ( char *fname, char *description ) ;

  ~Material ();

  int matches ( char *tx ) ;

  bool isIgnore    () { return ignore      ; }
  bool isZipper    () { return zipper      ; }
  bool isSphereMap () { return spheremap   ; }
  bool isCrashable () { return collideable ; }
  bool isReset     () { return resetter    ; }
  float getFriction() { return friction    ; }

  void applyToLeaf ( ssgLeaf *l ) ;

  ssgSimpleState *getState () { return state ; }
  void      apply    () { state -> apply ()  ; }

  char *getTexFname  () { return texname     ; }

} ;


#endif

/* EOF */
