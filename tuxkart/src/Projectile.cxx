//  $Id: Projectile.cxx,v 1.11 2004/09/24 15:45:02 matzebraun Exp $
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

#include <plib/sg.h>
#include "tuxkart.h"
#include "constants.h"
#include "Projectile.h"
#include "KartDriver.h"
#include "Explosion.h"
#include "World.h"

Projectile::Projectile(World* _world, KartDriver* _owner, int _type)
  : world(_world), owner(_owner), type(_type)
{
  sgZeroCoord(&velocity);
  
  // TODO load model, handle more types
  switch(type) {
    case COLLECT_MISSILE:
    {
      sgMat4 mat;
      sgMakeRotMat4(mat, owner->getCoord()->hpr);
      sgVec3 kart_unit = { 0, 1, 0 };
      sgXformPnt3(kart_unit, mat);
      sgScaleVec3(velocity.xyz, kart_unit, MAX_PROJECTILE_VELOCITY);
      break;
    }
    default:
      break;
  }
}

Projectile::~Projectile()
{
}

void Projectile::update (float delta)
{
  (void) delta;
  // TODO
#if 0
  float ndist = SG_MAX ;
  int nearest = -1 ;

  for ( int i = 0 ; i < world->getNumKarts() ; ++i )
  {
    sgCoord *pos ;
 
    pos = world->getKart(i) -> getCoord () ;
 
    if ( type != COLLECT_NOTHING && world->getKart(i) != owner )
    {
      float d = sgDistanceSquaredVec3 ( pos->xyz, getCoord()->xyz ) ;

      if ( d < 2.0f )
      {
        world->getKart(i) -> forceCrash () ;
        position.xyz[2] += 1.2f ;
        world->explosion[0]->start(position.xyz);
        off () ;
      }
      else
      if ( d < ndist )
      {
        ndist = d ;
        nearest = i ;
      }
    }
  }

  if ( type == COLLECT_HOMING_MISSILE && nearest != -1 &&
        ndist < MAX_HOME_DIST_SQD )
  {
    sgVec3 delta ;
    sgVec3 hpr ;
    sgCoord *k = world->getKart(nearest)->getCoord() ;

    sgSubVec3 ( delta, k->xyz, position.xyz ) ;

    delta[2] = 0.0f ;
 
    sgHPRfromVec3 ( hpr, delta ) ;

    sgSubVec3 ( hpr, position.hpr ) ;

    if ( hpr[0] >  180.0f ) hpr[0] -= 360.0f ;
    if ( hpr[0] < -180.0f ) hpr[0] += 360.0f ;
    if ( hpr[1] >  180.0f ) hpr[1] -= 360.0f ;
    if ( hpr[1] < -180.0f ) hpr[1] += 360.0f ;

    if ( hpr[0] > 80.0f || hpr[0] < -80.0f )
      velocity.hpr[0] = 0.0f ;
    else
    {
      if ( hpr[0] > 3.0f )
        velocity.hpr[0] = HOMING_MISSILE_TURN_RATE ;
      else
      if ( hpr[0] < -3.0f )
        velocity.hpr[0] = -HOMING_MISSILE_TURN_RATE ;
      else
        velocity.hpr[0] = 0.0f ;

      if ( hpr[1] > 1.0f )
        velocity.hpr[1] = HOMING_MISSILE_PITCH_RATE ;
      else
      if ( hpr[1] < -1.0f )
        velocity.hpr[1] = -HOMING_MISSILE_PITCH_RATE ;
      else
        velocity.hpr[1] = 0.0f ;
    }
  }
  else
    velocity.hpr[0] = velocity.hpr[1] = 0.0f ;
#endif
}

#if 0
void Projectile::doCollisionAnalysis  ( float /* delta*/,  float /* hot */ )
{
  if ( collided || crashed )
  {
    if ( type == COLLECT_SPARK )
    {
      sgVec3 bouncevec ;
      sgVec3 direction ;

      sgNormalizeVec3 ( bouncevec, surface_avoidance_vector ) ;
      sgSubVec3 ( direction, position.xyz, last_pos.xyz ) ;
      sgReflectInPlaneVec3 ( direction, bouncevec ) ;

      sgHPRfromVec3 ( position.hpr, direction ) ;
    }
    else
    if ( type != COLLECT_NOTHING )
    {
      position.xyz[2] += 1.2f ;
      world->explosion[0]->start(position.xyz);
      off () ;
    }
  }
}
#endif

