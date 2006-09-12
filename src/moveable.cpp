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

#include "world.hpp"
#include "player_kart.hpp"
#include "material_manager.hpp"
#include "material.hpp"
#include "config.hpp"
#include "history.hpp"

Moveable::Moveable (bool bHasHistory) {
  shadow                = 0;
  firsttime             = TRUE ;
  model                 = new ssgTransform();
  model     ->ref();

  sgZeroVec3 ( reset_pos.xyz ) ; sgZeroVec3 ( reset_pos.hpr ) ;
  
  reset ();
  if(bHasHistory) {
    historyVelocity = new sgCoord[history->GetSize()];
    historyPosition = new sgCoord[history->GetSize()];
  } else {
    historyVelocity = NULL;
    historyPosition = NULL;
  }
}   // Moveable

// -----------------------------------------------------------------------------
Moveable::~Moveable() {
  if(historyVelocity) {
    delete historyVelocity;
    delete historyPosition;
  }
  // FIXME what about model?
}   // ~Moveable

// -----------------------------------------------------------------------------
void Moveable::reset () {
  on_ground        = TRUE;
  collided         = FALSE;
  crashed          = FALSE;
  wheelie_angle    = 0.0f;
  materialHOT      = NULL;
  normalHOT        = NULL;

  sgZeroVec3 ( velocity.xyz ) ;
  sgZeroVec3 ( velocity.hpr ) ;
  sgCopyCoord ( &curr_pos, &reset_pos ) ;
  sgZeroVec3 ( abs_velocity ) ;
  
}   // reset

// -----------------------------------------------------------------------------
void Moveable::update (float dt) {
  if(historyVelocity) {
    if(config->replayHistory) {
      sgCoord tmp;
      sgCopyCoord(&tmp, &(historyVelocity[history->GetCurrentIndex()]));
      printf("velocity=%f,%f,%f,%f,%f,%f\n",
	     velocity.xyz[0],velocity.xyz[1],velocity.xyz[2],
	     velocity.hpr[0],velocity.hpr[1],velocity.hpr[2]);
      printf("tmp     =%f,%f,%f,%f,%f,%f\n",
	     tmp.xyz[0],tmp.xyz[1],tmp.xyz[2],
	     tmp.hpr[0],tmp.hpr[1],tmp.hpr[2]);

#undef IGNORE_Z_IN_HISTORY
#ifdef IGNORE_Z_IN_HISTORY
      float dummy=velocity.xyz[2];
      sgCopyCoord(&velocity, &tmp);
      velocity.xyz[2]=dummy;
#else
      sgCopyCoord(&velocity, &tmp);
#endif
    } else {
      sgCopyCoord(&(historyVelocity[history->GetCurrentIndex()]), &velocity);
    }
  }   // if historyVelocity

  sgMat4 result; 
  updatePosition(dt,result);

  sgVec3 start ; sgCopyVec3      (start, curr_pos.xyz      );
  sgVec3 end   ; sgCopyVec3      (end  , result[3]         );
  
  float  hot   = collectIsectData(start, end               );

  sgCopyVec3 (result[3], end) ;

  sgVec3 prev_pos;
  sgCopyVec3(prev_pos, curr_pos.xyz);
  sgSetCoord (&curr_pos, result);
  sgSubVec3  (abs_velocity, curr_pos.xyz, prev_pos);

  if(historyPosition) {
    if(config->replayHistory) {
      sgCoord tmp;
      sgCopyCoord(&tmp, &(historyPosition[history->GetCurrentIndex()]));
      printf("curr_pos=%f,%f,%f,%f,%f,%f\n",
	     curr_pos.xyz[0],curr_pos.xyz[1],curr_pos.xyz[2],
	     curr_pos.hpr[0],curr_pos.hpr[1],curr_pos.hpr[2]);
      printf("tmp     =%f,%f,%f,%f,%f,%f --> %d\n",
	     tmp.xyz[0],tmp.xyz[1],tmp.xyz[2],
	     tmp.hpr[0],tmp.hpr[1],tmp.hpr[2],
	     history->GetCurrentIndex());

#ifdef IGNORE_Z_IN_HISTORY
      float dummy=curr_pos.xyz[2];
      sgCopyCoord(&curr_pos, &tmp);
      curr_pos.xyz[2]=dummy;
#else
      sgCopyCoord(&curr_pos, &tmp);
#endif
    } else {
      sgCopyCoord(&(historyPosition[history->GetCurrentIndex()]), &curr_pos);
    }
  }   // if historyPosition
  float hat = curr_pos.xyz[2]-hot;
   
  on_ground = ( hat <= 0.01 );

  doCollisionAnalysis(dt, hot);

  placeModel () ;

  firsttime = FALSE ;
}   // update

// -----------------------------------------------------------------------------
// Computes the new position and hpr of the kart after a single time step.
void Moveable::updatePosition(float dt, sgMat4 result) {
  sgCoord scaled_velocity ;
  sgMat4  delta, mat; 
  /* Scale velocities to current time step. */
  sgScaleVec3    (scaled_velocity.xyz, velocity.xyz, dt);
  sgScaleVec3    (scaled_velocity.hpr, velocity.hpr, dt);
  sgMakeCoordMat4(delta, & scaled_velocity             );
  sgMakeCoordMat4(mat  , & curr_pos                    );
  sgMultMat4     (result, mat, delta                   );
}   // updatePosition

// -----------------------------------------------------------------------------
void Moveable::WriteHistory(char* s, int kartNumber, int indx) {
  sprintf(s, "Kart %d: v=%f,%f,%f,%f,%f,%f, p=%f,%f,%f,%f,%f,%f", kartNumber,
	  historyVelocity[indx].xyz[0],
	  historyVelocity[indx].xyz[1],
	  historyVelocity[indx].xyz[2],
	  historyVelocity[indx].hpr[0],
	  historyVelocity[indx].hpr[1],
	  historyVelocity[indx].hpr[2],
	  historyPosition[indx].xyz[0],
	  historyPosition[indx].xyz[1],
	  historyPosition[indx].xyz[2],
	  historyPosition[indx].hpr[0],
	  historyPosition[indx].hpr[1],
	  historyPosition[indx].hpr[2]);
}   // WriteHistory

// -----------------------------------------------------------------------------
void Moveable::ReadHistory(char* s, int kartNumber, int indx) {
  int k;
  sscanf(s, "Kart %d: v=%f,%f,%f,%f,%f,%f, p=%f,%f,%f,%f,%f,%f", &k,
	 historyVelocity[indx].xyz+0,
	 historyVelocity[indx].xyz+1,
	 historyVelocity[indx].xyz+2,
	 historyVelocity[indx].hpr+0,
	 historyVelocity[indx].hpr+1,
	 historyVelocity[indx].hpr+2,
	 historyPosition[indx].xyz+0,
	 historyPosition[indx].xyz+1,
	 historyPosition[indx].xyz+2,
	 historyPosition[indx].hpr+0,
	 historyPosition[indx].hpr+1,
	 historyPosition[indx].hpr+2);
  if(k!=kartNumber) {
    fprintf(stderr,"WARNING: tried reading data for kart %d, found:\n",
	    kartNumber);
    fprintf(stderr,"%s\n",s);
    exit(-2);
  }
}   // ReadHistory

// -----------------------------------------------------------------------------
void Moveable::doCollisionAnalysis  ( float,float ) { /* Empty by Default. */ }

#define ISECT_STEP_SIZE         0.4f
#define COLLISION_SPHERE_RADIUS 0.6f

#define max(m,n) ((m)>(n) ? (m) : (n))	/* return highest number */

// -----------------------------------------------------------------------------
float Moveable::collectIsectData ( sgVec3 start, sgVec3 end ) {
  sgVec3 vel ;

  collided = crashed = FALSE ;  /* Initial assumption */

  sgSubVec3 ( vel, end, start ) ;

  float speed = sgLengthVec3 ( vel ) ;

  /*
    At higher speeds, we must test frequently so we can't
    pass through something thin by mistake.

    At very high speeds, this is getting costly...so beware!
  */

  int nsteps = (int) ceil ( speed / ISECT_STEP_SIZE ) ;

  if ( nsteps == 0 ) nsteps = 1 ;

  if ( nsteps > 100 ) {
    fprintf ( stderr, "WARNING: Speed too high for collision detect!\n" ) ;
    fprintf ( stderr, "WARNING: Nsteps=%d, Speed=%f!\n", nsteps,speed ) ;
    fprintf(stderr, "moveable %p, vel=%f,%f,%f\n",this, vel[0],vel[1],vel[2]);
    nsteps = 100 ;
  }

  sgScaleVec3 ( vel, vel, 1.0f / (float) nsteps ) ;

  sgVec3 pos1, pos2 ;

  sgCopyVec3 ( pos1, start ) ;

  float hot = -9999.0 ;

  for ( int i = 0 ; i < nsteps ; i++ ) {
    sgAddVec3 ( pos2, pos1, vel ) ;    
    float hot1 = getIsectData ( pos1, pos2 ) ;
    hot = max(hot, hot1);
    sgCopyVec3 ( pos1, pos2 ) ;    
    if(collided) break;
  }

  sgCopyVec3 ( end, pos2 ) ;    
  return hot ;
}   // collectIsectData

// -----------------------------------------------------------------------------

float Moveable::getIsectData ( sgVec3 start, sgVec3 end ) {
  int      num_hits;

  sgSphere sphere;

  /*
    It's necessary to lift the center of the bounding sphere
    somewhat so that Player can stand on a slope.
  */

  sphere.setRadius ( COLLISION_SPHERE_RADIUS ) ;
  sphere.setCenter ( 0.0f, 0.0f, COLLISION_SPHERE_RADIUS + 0.3 ) ;

  /* Do a bounding-sphere test for Player. */
  sgSetVec3 ( surface_avoidance_vector, 0.0f, 0.0f, 0.0f );

  // new collision  algorithm
  AllHits a;
  sphere.setCenter ( end[0],end[1],end[2]+ COLLISION_SPHERE_RADIUS + 0.3 ) ;
  num_hits = world->Collision(&sphere, &a);
  for(AllHits::iterator i=a.begin(); i!=a.end(); i++) {
    if ( (*i)->plane[2]>0.4 ) continue;
    float dist = sphere.getRadius()-(*i)->dist;
    sgVec3 nrm ;
    sgCopyVec3  ( nrm, (*i)->plane ) ;
    sgScaleVec3 ( nrm, nrm, dist ) ;
    
    sgAddVec3 ( surface_avoidance_vector, nrm ) ;
      
    sgVec3 tmp ;
    sgCopyVec3 ( tmp, sphere.getCenter() ) ;
    sgAddVec3 ( tmp, nrm ) ;
    sphere.setCenter ( tmp ) ;
    
    collided = TRUE ;
    Material* m = material_manager->getMaterial( (*i)->leaf);
    if (m->isZipper   () ) collided = FALSE ;
    if (m->isCrashable() ) crashed  = TRUE  ;
    if (m->isReset    () ) OutsideTrack(1);
  }   // for i in a

  sgAddVec3(end, surface_avoidance_vector);

  // H.O.T == Height Of Terrain 
  // ==========================
  float top = COLLISION_SPHERE_RADIUS + max(start[2],end[2]);
  sgVec3 dstart; sgCopyVec3(dstart, end);
  sgVec3 dummy; sgCopyVec3(dummy, end);
  dummy[2]=top;
  ssgLeaf* leaf;
  float hot = world->GetHOT(dummy, dummy, &leaf, &normalHOT);
  if(leaf) {
    materialHOT = material_manager->getMaterial(leaf);
    // Only rescue the kart if it (nearly) touches the reset-material,
    // not only when it is above it. The condition for touching
    // a material is coarser then for the on_ground condition
    // (which tests for <0.01) - since the kart might have been falling
    // for quite some time, it might be really fast, so I guess a somewhat
    // coarser test is better for that case.
    if(materialHOT->isReset() &&
       fabs(top-COLLISION_SPHERE_RADIUS - hot)<0.2) OutsideTrack(1);
    if(materialHOT->isZipper()) handleZipper();
  } else {
    OutsideTrack(0);
  }
  
  if (end[2] < hot ) {
    end[2] = hot ;
  }   // end[2]<hot
  return hot ;
}   // getIsectData

