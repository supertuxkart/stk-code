//  $Id: Herring.h,v 1.2 2004/07/31 23:46:18 grumbel Exp $
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

#ifndef HEADER_HERRING_H
#define HEADER_HERRING_H
 
class Shadow
{
  ssgBranch *sh ;
 
public:
  Shadow ( float x1, float x2, float y1, float y2 ) ;
  ssgEntity *getRoot () { return sh ; }
} ;                                                                             

 
class Herring
{
  float h ;
  ssgTransform *tr ;
 
  Shadow *sh ;
 
public:
  Herring ( sgVec4 colour ) ;
  ssgTransform *getRoot () { return tr ; }
  void update () ;
} ;


 
class ActiveThingInstance
{
public:
  sgVec3 xyz ;
  ssgTransform *scs ;
 
  ssgTransform *setup ( ssgEntity *thing, sgCoord *pos )
  {
    sgCopyVec3 ( xyz, pos->xyz );
 
    scs = new ssgTransform ;
    scs -> setTransform ( pos ) ;
    scs -> addKid ( thing ) ;
 
    return scs ;
  }
 
  ssgTransform *setup ( ssgEntity *thing, sgVec3 pos )
  {
    sgCoord c ;
    sgSetVec3  ( c.hpr, 0.0f, 0.0f, 0.0f ) ;
    sgCopyVec3 ( c.xyz, pos ) ;
    return setup ( thing, &c ) ;
  }
 
  int active () { return xyz [ 2 ] > -1000000.0f ; }
 
  void getPos ( sgVec3 pos ) { sgCopyVec3 ( pos, xyz ) ; } 
  void setPos ( sgVec3 pos )
  {
    sgCopyVec3 ( xyz, pos ) ;
    scs -> setTransform ( pos ) ;
  }
 
  virtual void update () = 0 ;
} ;                                                                             

 
class HerringInstance : public ActiveThingInstance
{
public:
  Herring *her    ;
  float    time_to_return ;
  int      eaten  ;
  int      type   ;
  int      effect ;
  void update () ;
} ;


extern Herring *silver_h ;
extern Herring *gold_h   ;
extern Herring *red_h    ;
extern Herring *green_h  ;                                                     

extern int num_herring   ;                                                      

#define EFFECT_DEFAULT   0
#define EFFECT_SPEEDUP   1
#define EFFECT_ROCKET    2

#define HE_RED           0
#define HE_GREEN         1
#define HE_GOLD          2
#define HE_SILVER        3
 
#define MAX_HERRING     50

extern HerringInstance herring [ MAX_HERRING ] ;                             

#endif

/* EOF */
