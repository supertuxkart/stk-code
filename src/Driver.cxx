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

#include <typeinfo>
#include <iostream>
#include "tuxkart.h"
#include "material.h"
#include "utils.h"
#include "sound.h"
#include "Shadow.h"
#include "Driver.h"
#include "KartDriver.h"
#include "KartProperties.h"
#include "Track.h"
#include "World.h"
#include "constants.h"

#define sgn(x) ((x<0)?-1:((x>0)?1:0)) 	/* macro to return the sign of a number */
#define max(m,n) ((m)>(n) ? (m) : (n))	/* macro to return highest number */
#define min(m,n) ((m)<(n) ? (m) : (n))	/* macro to return lowest number */

   static inline void relaxation(float& target, float& prev, float rate)
   {
   //This function rotates to the closest! so no +180 or -180º rotations!
   
      if (target * prev < 0.0f)//if one is positive and the other is negative
      {
         float rotation_direction = target - prev;
         if (rotation_direction > 180.0f)//rotate counter-clockwise (prev=neg)
         {                                 
            float distance_prev, distance_target, rotation;
            distance_prev = (180.0f + prev);
            distance_target =  (180.0f - target);
            rotation = prev - (distance_prev * rate + distance_target * rate);
            if (rotation < -180.0f)//The visible rotation doesn't jumps the gap
               target = rotation;
            else//It does jumps the -180 & 180 gap
               target = 360 + rotation;
         }
         else if (rotation_direction < -180.0f )//Rotate clockwise (prev=pos)
         {
            float distance_prev, distance_target, rotation;
            distance_prev = (180.0f - prev);
            distance_target =  (180.0f + target);
            rotation = prev + (distance_prev * rate + distance_target * rate);
            if (rotation < 180.0f)//The visible rotation doesn't jumps the gap
               target = rotation;
            else//It does jumps the -180 & 180 gap
               target = -360 + rotation;
         }
         else //The given target & prev don't jump the gap
            target = (prev) + rate * ((target) - (prev));
      }
      else
         target = (prev) + (rate) * ((target) - (prev));
        
      prev = (target);
   }


   Driver::Driver (World* _world, const KartProperties* kart_properties)
    : world(_world)
   {
   // hack for now (const_cast should be removed later)
      this->kart_properties = const_cast<KartProperties*> (kart_properties);
      firsttime = TRUE ;
   
      comp_model = new ssgBranch;
      comp_model->ref();
      model = new ssgTransform ;
      model->ref();
   
      comp_model->addKid(model);
   
      shadow = 0;
    
   /* New Physics */
      sgZeroVec3 (force);
      steer_angle = throttle = brake = 0.0f;
   
      sgZeroVec3 ( reset_pos.xyz ) ; sgZeroVec3 ( reset_pos.hpr ) ;
      reset () ;
   }

   Driver::~Driver()
   {
      ssgDeRefDelete(shadow);
      ssgDeRefDelete(model);
      ssgDeRefDelete(comp_model);
   }

   void
   Driver::reset ()
   {
      race_lap = -1 ;
      race_position = 9 ;
      rescue = FALSE ;
      on_ground = true ;
      zipper_time_left = 0.0f ;
      collided = crashed = FALSE ;
      history_index = 0 ;
      wheelie_angle = 0.0f ;
   
      sgZeroCoord ( &velocity ) ;
      sgZeroVec3 ( ground_normal ) ;
      sgCopyCoord ( &last_pos, &reset_pos ) ;
      sgCopyCoord ( &position, &reset_pos ) ;
      sgCopyCoord ( &visi_pos, &reset_pos ) ;
      sgCopyCoord ( &last_relax_pos, &reset_pos ) ;
   
      track_hint = world ->track -> absSpatialToTrack ( last_track_coords,
                                                 last_pos.xyz ) ;
      track_hint = world ->track -> absSpatialToTrack ( curr_track_coords,
                                                 position.xyz ) ;
   
   }

   void
   Driver::update (float delta)
   {
      physicsUpdate (delta);
      coreUpdate (delta) ;
   
      doObjectInteractions () ;
      updateVisiPos(delta);
   }

   void
   Driver::updateVisiPos(float delta)
   {
      sgCopyCoord ( &visi_pos, &position ) ;
   
      visi_pos.hpr[1] += wheelie_angle ;
   
   #if 0
   if (use_fake_drift)
    {
      // Rotate the kart a bit to get a feeling for drifting even if
      // it isn't there in reality, not 100% sure if this is a good
      // idea, but its worth a try
      visi_pos.hpr[0] += steer_angle*10.0f;
    }
   #endif
   
      visi_pos.xyz[2] += fabs( sin ( wheelie_angle * SG_DEGREES_TO_RADIANS )) * 0.3f ;
   
      relaxation(visi_pos.hpr[0], last_relax_pos.hpr[0], 25.0f * delta);
      relaxation(visi_pos.hpr[1], last_relax_pos.hpr[1], 25.0f * delta);
      relaxation(visi_pos.hpr[2], last_relax_pos.hpr[2], 25.0f * delta);
   }

   void
   Driver::placeModel ()
   {
      if ( model != NULL )
      {
         sgCoord relax_pos;
         sgCopyCoord ( &relax_pos, &visi_pos ) ;
      
         model -> setTransform ( & relax_pos ) ;
      
         sgMat4 res;
         sgVec3 hpr;
      
         hpr_from_normal(hpr, ground_normal);
      
         sgMat4 rot;
         sgMat4 rot2;
      
         relax_pos.xyz[2] = height_of_terrain;
      
         sgMakeTransMat4(res, relax_pos.xyz);
         sgMakeRotMat4(rot, hpr[0], hpr[1], hpr[2]);
         sgMakeRotMat4(rot2, relax_pos.hpr[0]-hpr[0], 0, 0);
      //sgMakeRotMat4(rot2, , 0, 0);
      
         sgMat4 res2;
         sgMultMat4(res2, res, rot);
         sgMultMat4(res, res2, rot2);
      
         if (shadow)
            shadow -> setTransform(res);
      }
   }

   static inline float _lateralForce (const KartProperties *properties,
                     float cornering, float sideslip)
   {
      return ( max(-properties->max_grip,
             min(properties->max_grip, cornering * sideslip))
             * properties->mass * 9.82 / 2 );
   }

   void
   Driver::physicsUpdate (float delta)
   {
      sgVec2 resistance;
      sgVec2 traction;
      sgVec2 lateral_f;
      sgVec2 lateral_r;
   
      float wheel_rot_angle;
      float sideslip;
      float torque;
      float kart_angular_acc;
      float kart_angular_vel = 2*M_PI * velocity.hpr[0] / 360.0f;
   
      unsigned int count;
   
      const float wheelbase = 1.2;
   
        // gravity
      force[2] = -GRAVITY * kart_properties->mass;
   
      sgZeroVec2 (resistance);
      sgZeroVec2 (traction);
      sgZeroVec2 (lateral_f);
      sgZeroVec2 (lateral_r);
   
   // rotation angle of wheels
      sideslip = atan2 (velocity.xyz[0], velocity.xyz[1]);
      wheel_rot_angle = atan2 (kart_angular_vel * wheelbase/2,
                           velocity.xyz[1]);
   
   /*----- Lateral Forces -----*/
      lateral_f[0] = _lateralForce(kart_properties, kart_properties->corn_f,
                           sideslip + wheel_rot_angle - steer_angle);
      lateral_r[0] = _lateralForce(kart_properties, kart_properties->corn_r,
                           sideslip - wheel_rot_angle);
   
   // calculate traction
      traction[0] = 0.0f;
      traction[1] = 10 * (throttle - brake*sgn(velocity.xyz[1]));
   
   // apply air friction and system friction
      resistance[0] -= velocity.xyz[0] * fabs (velocity.xyz[0]) *
          kart_properties->air_friction;
      resistance[1] -= velocity.xyz[1] * fabs (velocity.xyz[1]) *
          kart_properties->air_friction;
      resistance[0] -= 10 * kart_properties->system_friction * velocity.xyz[0];
      resistance[1] -= kart_properties->system_friction * velocity.xyz[1];
   
   // sum forces
      force[0] += traction[0] + cos(steer_angle)*lateral_f[0] + lateral_r[1] + resistance[0];
      force[1] += traction[1] + sin(steer_angle)*lateral_f[1] + lateral_r[1] + resistance[1];
   
   // torque - rotation force on kart body
      torque = (lateral_f[0] * wheelbase/2) - (lateral_r[0] * wheelbase/2);
   
      kart_angular_acc = torque / kart_properties->inertia;
   	
   // velocity
      for (count = 0; count < 3; count++)
         velocity.xyz[count] += (force[count] / kart_properties->mass) * delta;
   
      kart_angular_vel += kart_angular_acc * delta;
      velocity.hpr[0] = kart_angular_vel * 360.0f / (2*M_PI);
   
   // clear forces
      sgZeroVec3 (force);
   }

   void
   Driver::coreUpdate (float delta)
   {
      sgCoord scaled_velocity ;
   
      doZipperProcessing () ;
   
      sgCopyCoord ( &last_pos        , &position         ) ;
      sgCopyVec2  ( last_track_coords, curr_track_coords ) ;
   
   /* Scale velocities to current time step. */
      sgScaleVec3 ( scaled_velocity.xyz, velocity.xyz, delta ) ;
      sgScaleVec3 ( scaled_velocity.hpr, velocity.hpr, delta ) ;
   
      sgMat4 mat    ;
      sgMat4 result ;
      sgMat4 mdelta  ;
   
   /* Form new matrix */
      sgMakeCoordMat4 ( mdelta, & scaled_velocity ) ;
      sgMakeCoordMat4 ( mat  , & position        ) ;
      sgMultMat4      ( result, mat, mdelta ) ;
      sgVec3 start ; sgCopyVec3 ( start, position.xyz ) ;
      sgVec3 end   ; sgCopyVec3 ( end  , result[3]    ) ;
   
      float hot = collectIsectData ( start, end ) ;
      height_of_terrain = hot;
   
      sgCopyVec3 ( result[3], end ) ;
   
      sgSetCoord ( &position, result  ) ;
      float hat = position.xyz[2]-hot ;
   
      on_ground = ( hat <= 0.01 ) ;
   
      doCollisionAnalysis ( delta, hot ) ;
   
      //The next lines before firsttime = FALSE ensure no dot jumping
      int possible_hint = world ->track -> spatialToTrack ( curr_track_coords,
                                                            position.xyz,
                                                            track_hint ) ;
      if (((possible_hint + 1 <= (int)track_hint + 2) &&
            (possible_hint - 1 >= (int)track_hint - 2)) || 
               ((possible_hint == (int)world->track->driveline.size() - 1) && 
                  (track_hint == 0)) || ((possible_hint == 0) &&
                     (track_hint == world->track->driveline.size() - 1 )))
         track_hint = possible_hint;
      
   
      firsttime = FALSE ;
      doLapCounting () ;
   }

   void Driver::doObjectInteractions () { /* Empty by Default. */ 
   } 
   void Driver::doLapCounting        () { /* Empty by Default. */ 
   } 
   void Driver::doZipperProcessing   () { /* Empty by Default. */ 
   } 
   void Driver::doCollisionAnalysis  ( float delta, float hot ) { 
      (void)delta; (void)hot; }

#define ISECT_STEP_SIZE         0.4f
#define COLLISION_SPHERE_RADIUS 0.6f

   float Driver::collectIsectData ( sgVec3 start, sgVec3 end )
   {
      sgVec3 vel ;
   
      collided = crashed = FALSE ;  /* Initial assumption */
   
      sgSubVec3 ( vel, end, start ) ;
   
      float speed = sgLengthVec3 ( vel ) ;
   
   /*
    At higher speeds, we must test frequently so we can't
    pass through something thin by mistake.
   
    At very high speeds, this is getting costly...so beware!
   */
   
      int nsteps = (int) ceil ( speed / ISECT_STEP_SIZE );
   
      if ( nsteps == 0 ) nsteps = 1 ;
   
      if ( nsteps > 100 )
      {
         std::cout << "WARNING: Kart: " << this << std::endl
            << "WARNING: Speed too high for collision detect!" << std::endl
            << "WARNING: Nsteps=" << nsteps << " Speed=" << speed << std::endl;
         nsteps = 100 ;
      }
   
      sgScaleVec3 ( vel, vel, 1.0f / (float) nsteps ) ;
   
      sgVec3 pos1, pos2 ;
   
      sgCopyVec3 ( pos1, start ) ;
   
      float hot = 0.0f ;
   
      for ( int i = 0 ; i < nsteps ; i++ )
      {
         sgAddVec3 ( pos2, pos1, vel ) ;    
         hot = getIsectData ( pos1, pos2 ) ;
         sgCopyVec3 ( pos1, pos2 ) ;    
      }
   
      sgCopyVec3 ( end, pos2 ) ;    
      return hot ;
   }



   float Driver::getIsectData ( sgVec3 start, sgVec3 end )
   {
      ssgHit *results ;
      int num_hits ;
   
      sgSphere sphere ;
      sgMat4   invmat ;
   
   /*
    It's necessary to lift the center of the bounding sphere
    somewhat so that Player can stand on a slope.
   */
   
      sphere.setRadius ( COLLISION_SPHERE_RADIUS ) ;
      sphere.setCenter ( 0.0f, 0.0f, COLLISION_SPHERE_RADIUS + 0.3 ) ;
   
   /* Do a bounding-sphere test for Player. */
   
      sgMakeIdentMat4 ( invmat ) ;
      invmat[3][0] = -end[0] ;
      invmat[3][1] = -end[1] ;
      invmat[3][2] = -end[2] ;
   
      if ( firsttime )
         num_hits = 0 ;
      else
         num_hits = ssgIsect ( world->trackBranch, &sphere, invmat, &results ) ;
   
      sgSetVec3 ( surface_avoidance_vector, 0.0f, 0.0f, 0.0f ) ;
   
      int i ;
   
   /* Look at all polygons near to Player */
   
      for ( i = 0 ; i < num_hits ; i++ )
      {
         ssgHit *h = &results [ i ] ;
      
         if ( getMaterial ( h->leaf ) -> isIgnore () )
            continue ;
      
         float dist = sgDistToPlaneVec3 ( h->plane, sphere.getCenter() ) ;
      
      /*
      This is a nasty kludge to stop a weird interaction
      between collision detection and height-of-terrain
      that causes Player to get 'stuck' on some polygons
      corners. This should be fixed more carefully.
      
      Surfaces that are this close to horizontal
      are handled by the height-of-terrain code anyway.
      */
      
         if ( h -> plane[2] > 0.4 )
            continue ;
      
         if ( dist > 0 && dist < sphere.getRadius() )
         {
            dist = sphere.getRadius() - dist ;
            sgVec3 nrm ;
            sgCopyVec3  ( nrm, h->plane ) ;
            sgScaleVec3 ( nrm, nrm, dist ) ;
         
            sgAddVec3 ( surface_avoidance_vector, nrm ) ;
         
            sgVec3 tmp ;
            sgCopyVec3 ( tmp, sphere.getCenter() ) ;
            sgAddVec3 ( tmp, nrm ) ;
            sphere.setCenter ( tmp ) ;
         
            collided = TRUE ;
         
            if ( getMaterial ( h->leaf ) -> isZipper    () ) collided = FALSE ;
            if ( getMaterial ( h->leaf ) -> isCrashable () ) crashed  = TRUE  ;
            if ( getMaterial ( h->leaf ) -> isReset     () ) rescue   = TRUE  ;
         }
      }
   
   /* Look for the nearest polygon *beneath* Player (assuming avoidance) */
   
      sgAddVec3 ( end, surface_avoidance_vector ) ;
   
      float hot ;        /* H.O.T == Height Of Terrain */
      sgVec3 HOTvec ;
   
      invmat[3][0] = - end [0] ;
      invmat[3][1] = - end [1] ;
      invmat[3][2] = 0.0 ;
   
      float top = COLLISION_SPHERE_RADIUS +
         (( start[2] > end[2] ) ? start[2] : end[2] ) ;
   
      sgSetVec3 ( HOTvec, 0.0f, 0.0f, top ) ;
   
      num_hits = ssgHOT ( world->trackBranch, HOTvec, invmat, &results ) ;
   
      hot = -1000000.0f ;
   
      int need_rescue = FALSE ;
   
      for ( i = 0 ; i < num_hits ; i++ )
      {
         ssgHit *h = &results [ i ] ;
      
         if ( getMaterial ( h->leaf ) -> isIgnore () )
            continue ;
      
         float hgt = - h->plane[3] / h->plane[2] ;
      
         if ( hgt >= hot )
         {
            hot = hgt ;
            sgCopyVec3 ( ground_normal, h->plane ) ;
         
            need_rescue = getMaterial ( h->leaf ) -> isReset  () ;
         
            if ( getMaterial ( h->leaf ) -> isZipper () )
            {
               if ( this == world->getPlayerKart(0) )
                  sound->playSfx ( SOUND_WEE ) ;
            
               wheelie_angle = 45.0f ;
               zipper_time_left = ZIPPER_TIME ;
            }
         
         // toying with diffrent frictions for materials
            kart_properties->system_friction = getMaterial ( h->leaf ) -> getFriction ();
         
         }
      }
   
      if ( end [ 2 ] < hot )
      {
         end [ 2 ] = hot ;
      
         if ( need_rescue )
            rescue = TRUE ;
      }
   
      return hot ;
   }

/* EOF */

