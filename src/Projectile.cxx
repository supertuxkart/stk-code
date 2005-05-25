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

#include <iostream>
#include <plib/sg.h>
#include <plib/ssg.h>
#include "tuxkart.h"
#include "constants.h"
#include "Projectile.h"
#include "KartDriver.h"
#include "Explosion.h"
#include "World.h"
#include "material.h"

Projectile::Projectile(World* _world, KartDriver* _owner, int _type)
  : world(_world), owner(_owner), type(_type)
{
  current_lifetime = 0.0f;
  exploded = false;
  sgZeroCoord(&velocity);
  setCoord (owner->getCoord());

  model = new ssgTransform;
  
  switch(type) {
    case COLLECT_SPARK:
    {
      velocity.xyz[1] = MAX_SPARK_VELOCITY;
	  model ->addKid (world->projectile_spark);
 	  collision_sphere_radius = SPARK_COLLISION_SPHERE_RADIUS;
	  lifetime_limit_secs = SPARK_LIFETIME;
      break;
    }
    case COLLECT_MISSILE:
    {
      /*sgMat4 mat;
      sgMakeRotMat4(mat, owner->getCoord()->hpr);
      sgVec3 kart_unit = { 0, 1, 0 };
      sgXformPnt3(kart_unit, mat);
      sgScaleVec3(velocity.xyz, kart_unit, MAX_PROJECTILE_VELOCITY);*/
	  velocity.xyz[1] = MAX_PROJECTILE_VELOCITY;
	  model ->addKid (world->projectile_missle);
 	  collision_sphere_radius = MISSILE_COLLISION_SPHERE_RADIUS;
	  lifetime_limit_secs = MISSILE_LIFETIME;
      break;
    }
	case COLLECT_HOMING_MISSILE:
    {
	  velocity.xyz[1] = MAX_HOMING_PROJECTILE_VELOCITY;
	  model ->addKid (world->projectile_flamemissle);
	  collision_sphere_radius = HOMING_MISSILE_COLLISION_SPHERE_RADIUS;
	  lifetime_limit_secs = HOMING_MISSILE_LIFETIME;
      break;
    }
    default:
      break;
  }
  world->scene->addKid (model) ;
}

Projectile::~Projectile()
{
world->scene->removeKid(world->scene->searchForKid(model)) ;
/*No need to delete model; even tought it was allocated with new since
acording with the PLIB docs in this case it will be deleted 
automagically*/
}



void Projectile::update (float delta)
{
/*1. Calculate new projectile position and place the model*/
sgCoord scaled_velocity = velocity;
sgScaleVec3(scaled_velocity.xyz, delta);
sgScaleVec3(scaled_velocity.hpr, delta);

sgMat4 mdelta;
sgMat4 mat;
sgMat4 result;

last_pos = position;

sgMakeCoordMat4 ( mdelta, & scaled_velocity ) ;
sgMakeCoordMat4 ( mat  , & position ) ;
sgMultMat4      ( result, mat, mdelta ) ;

sgSetCoord ( &position, result  ) ;

model -> setTransform ( &position ) ;

/*2. Calculate if the projectile crashes with the track or if it
lifetime is up*/
float hot = collectIsectData ( last_pos.xyz, position.xyz) ;
updateLifetime(delta);

// TODO
//#if 0
/*3. Calculate if the projectile crashes with a kart*/
  float ndist = SG_MAX ;
  int nearest = -1 ;
  
  for ( int i = 0 ; i < world->getNumKarts() ; ++i )
  {
    sgCoord *pos ;
 
    pos = world->getKart(i) -> getCoord () ;
 
    if ( world->getKart(i) != owner )
    {
      float d = sgDistanceSquaredVec3 ( pos->xyz, getCoord()->xyz ) ;

      if ( d < 2.0f )
      {
        world->getKart(i) -> forceCrash () ;
		collided = true;
        //world->explosions[0]->start(position.xyz);
        //off () ;
      }
      else
      if ( d < ndist )
      {
        ndist = d ;
        nearest = i ;
      }
    }
  }

/*5. Homming missile tracking stuff*/
  if ( type == COLLECT_HOMING_MISSILE && nearest != -1 &&
        ndist < MAX_HOME_DIST_SQD )
  {
    sgVec3 vec_delta ;
    sgVec3 hpr ;
    sgCoord *k = world->getKart(nearest)->getCoord() ;

    sgSubVec3 ( vec_delta, k->xyz, position.xyz ) ;

    vec_delta[2] = 0.0f ;
 
    sgHPRfromVec3 ( hpr, vec_delta ) ;

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
//#endif

/*6.*/
doCollisionAnalysis(delta,hot);
}
//#if 0
float Projectile::collectIsectData ( sgVec3 start, sgVec3 end)
{
   sgVec3 vel;

   collided = FALSE ;  /* Initial assumption */

   sgSubVec3 ( vel, end, start ) ;

   float speed = sgLengthVec3 ( vel ) ;

   /*
    At higher speeds, we must test frequently so we can't
    pass through something thin by mistake.

    At very high speeds, this is getting costly...so beware!
   */

   int nsteps = (int) ceil ( speed / PROJECTILE_ISECT_STEP_SIZE );

   if ( nsteps == 0 ) nsteps = 1 ;

   if ( nsteps > 100 )
   {
      std::cout << "WARNING: Projectile: " << this << std::endl
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
 //sgCopyVec3 ( end, pos2 ) ;
 return hot ;
}

float Projectile::getIsectData ( sgVec3 start, sgVec3 end )
{
   ssgHit *results ;
   int num_hits ;
   
   sgSphere sphere ;
   sgMat4   invmat ;
   
   /*
    It's necessary to lift the center of the bounding sphere
    somewhat so that Player can stand on a slope.
   */
   
   sphere.setRadius ( collision_sphere_radius ) ;
   
   /*If we don't elevate the center a little we have problems with
     slopes, this happens to karts too.*/
   if ( type == COLLECT_SPARK )   
      sphere.setCenter ( 0.0f, 0.0f, collision_sphere_radius + 0.3 ) ;
   else
      sphere.setCenter ( 0.0f, 0.0f, collision_sphere_radius) ;
   
   /* Do a bounding-sphere test. */
   
      sgMakeIdentMat4 ( invmat ) ;
      invmat[3][0] = -end[0] ;
      invmat[3][1] = -end[1] ;
      invmat[3][2] = -end[2] ;
   
      num_hits = ssgIsect ( world->trackBranch, &sphere, invmat, &results ) ;
   
      if(num_hits)  
	 
      sgSetVec3 ( surface_avoidance_vector, 0.0f, 0.0f, 0.0f ) ;
   
      int i ;
   
   /* Look at all polygons near to projectile */
   
      for (i = 0 ; i < num_hits ; i++ )
      {
         ssgHit *h = &results [ i ] ;
      
 	     if ( getMaterial ( h->leaf ) -> isIgnore () )
             continue ;
      
         float dist = sgDistToPlaneVec3 ( h->plane, sphere.getCenter() ) ;
      
      /*
      This is a nasty kludge to stop a weird interaction
      between collision detection and height-of-terrain
      that causes the projectile to get 'stuck' on some polygons
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
         }
      }
   
   /* Look for the nearest polygon *beneath* Player (assuming avoidance) */
   
      sgAddVec3 ( end, surface_avoidance_vector ) ;
   
      float hot ;        /* H.O.T == Height Of Terrain */
      sgVec3 HOTvec ;
   
      invmat[3][0] = - end [0] ;
      invmat[3][1] = - end [1] ;
      invmat[3][2] = 0.0 ;
   
      float top = collision_sphere_radius +
         (( start[2] > end[2] ) ? start[2] : end[2] ) ;
   
      sgSetVec3 ( HOTvec, 0.0f, 0.0f, top ) ;
   
      num_hits = ssgHOT ( world->trackBranch, HOTvec, invmat, &results ) ;
   
      hot = -1000000.0f ;
   
      for ( i = 0 ; i < num_hits ; i++ )
      {
         ssgHit *h = &results [ i ] ;
      
         if ( getMaterial ( h->leaf ) -> isIgnore () )
            continue ;
			
		 float hgt = - h->plane[3] / h->plane[2] ;
      
         if ( hgt >= hot )
         {
            hot = hgt ;
         }
       }
   
   if ( end [ 2 ] < hot )
   {
      end [ 2 ] = hot ;
   
   }
   return hot ;
}
//#endif

//#if 0
void Projectile::doCollisionAnalysis  ( float  delta,  float  hot  )
{

  if ( collided )
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
		exploded = true;
      //position.xyz[2] += 1.2f ;
      //world->explosion[0]->start(position.xyz);
      //off () ;
    }
  }

  //Now, the response to the height of terrain collision
	if (position.xyz[2] - hot <= 0.01 )
	{
		position.xyz[2] = hot ;
	    /*if ( type == COLLECT_SPARK)
		{
			position.xyz[2] = hot ;
		}
		else if ( type != COLLECT_NOTHING )
        {
			exploded = true;
	    }*/
	}
		
}
//#endif

void Projectile::updateLifetime(float delta)
{
	current_lifetime += delta ;

	if(lifetime_limit_secs < current_lifetime)
	{
	    exploded = true;
        //world->explosions[0]->start(position.xyz);
    	//off () ;
	} 
}
