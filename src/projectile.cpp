//  $Id$
//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2004-2005 Steve Baker <sjbaker1@airmail.net>
//  Copyright (C) 2006 Joerg Henrichs, Steve Baker
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

#include "constants.hpp"
#include "projectile.hpp"
#include "world.hpp"
#include "kart.hpp"
#include "projectile_manager.hpp"
#include "sound_manager.hpp"

Projectile::Projectile(Kart *kart, int collectable) : Moveable(false) {
  init(kart, collectable);  
  getModel()->clrTraversalMaskBits(SSGTRAV_ISECT|SSGTRAV_HOT);
}   // Projectile

// -----------------------------------------------------------------------------
void Projectile::init(Kart *kart, int collectable_) {
  owner              = kart;
  type               = collectable_;
  hasHitSomething    = false;
  nLastRadarBeep     = -1;
  speed              = collectable_manager->getSpeed(type);
  ssgTransform *m    = getModel();
  m->addKid(collectable_manager->getModel(type));
  setCoord(kart->getCoord());
  world->addToScene(m);
}
// -----------------------------------------------------------------------------
Projectile::~Projectile() {
}   // ~Projectile
// -----------------------------------------------------------------------------
void Projectile::update (float dt) {
  // we don't even do any physics here - just set the
  // velocity, and ignore everything else for projectiles.
  velocity.xyz[1] = speed;
  sgCopyCoord ( &last_pos, &curr_pos);
  Moveable::update(dt);
  doObjectInteractions();
}   // update

// -----------------------------------------------------------------------------
// Returns true if this missile has hit something,
// otherwise false.
void Projectile::doObjectInteractions () {
  float ndist = SG_MAX ;
  int nearest = -1 ;

  for ( int i = 0 ; i < world->getNumKarts() ; i++ ) {
    sgCoord *pos ;
 
    Kart *kart = world -> getKart(i);
    pos        = kart  -> getCoord();
 
    if ( type != COLLECT_NOTHING && kart != owner ) {
      float d = sgDistanceSquaredVec3 ( pos->xyz, getCoord()->xyz ) ;

      if ( d < 2.0f ) {
	explode();
        kart -> forceCrash () ;
	return;
      } else if ( d < ndist ) {
        ndist = d ;
        nearest = i ;
      }  // if !d<2.0f
    }   // if type!=NOTHING &&kart!=owner
  }  // for i<getNumKarts
  if ( type == COLLECT_HOMING_MISSILE && nearest != -1 &&
        ndist < MAX_HOME_DIST_SQD                          ) {
    sgVec3 delta;
    sgVec3 hpr;
    Kart *kart=world->getKart(nearest);
    if(nLastRadarBeep!=nearest && kart->isPlayerKart()) {
      sound_manager->playSfx(SOUND_MISSILE_LOCK);
      nLastRadarBeep=nearest;
    }
    sgCoord *k = kart->getCoord() ;

    sgSubVec3 ( delta, k->xyz, curr_pos.xyz ) ;

    delta[2] = 0.0f ;

    sgHPRfromVec3 ( hpr, delta ) ;

    sgSubVec3 ( hpr, curr_pos.hpr ) ;

    if ( hpr[0] >  180.0f ) hpr[0] -= 360.0f ;
    if ( hpr[0] < -180.0f ) hpr[0] += 360.0f ;
    if ( hpr[1] >  180.0f ) hpr[1] -= 360.0f ;
    if ( hpr[1] < -180.0f ) hpr[1] += 360.0f ;

    if ( hpr[0] > 80.0f || hpr[0] < -80.0f )
      velocity.hpr[0] = 0.0f ;
    else {
      if      ( hpr[0] >  3.0f ) velocity.hpr[0] =  HOMING_MISSILE_TURN_RATE ;
      else if ( hpr[0] < -3.0f ) velocity.hpr[0] = -HOMING_MISSILE_TURN_RATE ;
      else                       velocity.hpr[0] =  0.0f ;

      if      ( hpr[2] > 1.0f  ) velocity.hpr[1] = -HOMING_MISSILE_PITCH_RATE ;
      else if ( hpr[2] < -1.0f ) velocity.hpr[1] = HOMING_MISSILE_PITCH_RATE ;
      else                       velocity.hpr[1] = 0.0f ;
    }
  } else  // type!=HOMING||nearest==-1||ndist>MAX_HOME_DIST_SQD
    velocity.hpr[0] = velocity.hpr[1] = 0.0f ;
}

// -----------------------------------------------------------------------------
void Projectile::doCollisionAnalysis  ( float dt, float hot )
{
  if ( collided || crashed ) {
    if ( type == COLLECT_SPARK ) {
      sgVec3 bouncevec ;
      sgVec3 direction ;

      sgNormalizeVec3 ( bouncevec, surface_avoidance_vector ) ;
      sgSubVec3 ( direction, curr_pos.xyz, last_pos.xyz ) ;
      sgReflectInPlaneVec3 ( direction, bouncevec ) ;

      sgHPRfromVec3 ( curr_pos.hpr, direction ) ;
    } else if ( type != COLLECT_NOTHING ) {
      explode();
    }
  }   // if collided||crashed
}   // doCollisionAnalysis

// -----------------------------------------------------------------------------
void Projectile::explode() {
  hasHitSomething=true;
  curr_pos.xyz[2] += 1.2f ;
  // Notify the projectile manager that this rocket has hit something.
  // The manager will create the appropriate explosion object, and
  // place this projectile into a list so that it can be reused later,
  // without the additional cost of creating the object again
  projectile_manager->explode();
  
  // Now remove this projectile from the graph:
  ssgTransform *m = getModel();
  m->removeAllKids();
  world->removeFromScene(m);
}   // explode

/* EOF */
