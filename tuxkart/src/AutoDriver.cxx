//  $Id: AutoDriver.cxx,v 1.20 2004/12/12 01:25:02 cosmosninja Exp $
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

#include "constants.h"
#include "World.h"
#include "KartDriver.h"
#include "AutoDriver.h"

#include <iostream>
#include "Track.h"
#include <math.h>

   inline float sgnsq ( float x ) { 
      return ( x < 0 ) ? -(x * x) : (x * x) ; }

   void AutoDriver::update (float delta)
   {
      assert(kart);
   
   /* OLD Steering algorithm. Uncomment next paragraph and comment the new
      steering algorithm to use it. If moving left-to-right and on the left 
      - or right to left and on the right - do nothing. */
   
   /*
      sgVec2 track_velocity ;
      sgSubVec2 ( track_velocity, kart->curr_track_coords, kart->last_track_coords ) ;
   
      if ( ( track_velocity [ 0 ] < 0.0f && kart->curr_track_coords [ 0 ] > 0.0f ) ||
         ( track_velocity [ 0 ] > 0.0f && kart->curr_track_coords [ 0 ] < 0.0f ) )
         kart->getVelocity()->hpr[0] = sgnsq(kart->curr_track_coords[0])*3.0f ;
      else
         kart->getVelocity()->hpr[0] = sgnsq(kart->curr_track_coords[0])*12.0f ;
   
   */
   
   
   //If you want to try the new steering algorithm with the player kart check
   //PlayerDriver.cxx in the update function.
   
   /*New steering algorithm. We find out which is the next dot that the AI
   should follow, then we calculate rotation needed to go in a straight
   line towards the nest dot. If the rotation we plan to do(we can't do it
   at once since it will not look smooth) is bigger than the amount we
   calculated, we rotate the rotation we planned. Otherwise, it rotates just
   what we need.*/
   
   //1. Get which is the next dot that the AI should follow
      size_t next ;
   
      next = (kart->track_hint + 1 >= world->track->driveline.size()) ? 0 :
         kart->track_hint + 1;
            
   //2. Calculate the rotation we need using trigonometry, we get the sides
   //of a right triangle where the 2 points that define the hypotenuse are
   //the next dot and the current kart position. The angle adyacent to the
   //kart position in the triangle is what we look for.
   
      SGfloat adjacent_line, opposite_line, theta;
      adjacent_line = world->track->driveline[next][0] - 
         kart->getCoord()->xyz[0];
      opposite_line = world->track->driveline[next][1] - 
         kart->getCoord()->xyz[1];
   
      theta = atanf(opposite_line/adjacent_line) * SG_RADIANS_TO_DEGREES;
      
      //The real value depends on the side of the track that the kart is
      if (adjacent_line < 0.0f) theta = theta + 90.0f;
      else theta = theta - 90.0f;
      
   //See in which direction we have to rotate, and does it.
      float rotation_direction = kart->getCoord()->hpr[0] - theta;
      if (rotation_direction > 180.0f)//rotate counter-clockwise(theta=neg)
         kart->getVelocity()->hpr[0] = 3.5f * kart->getVelocity()->xyz[1];
      else if (rotation_direction < -180.0f )//Rotate clockwise (prev=pos)
         kart->getVelocity()->hpr[0] = -3.5f * kart->getVelocity()->xyz[1];
      else //If it's neither, the rotation doesn't jumps the gap
      {
         kart->getVelocity()->hpr[0] = 0.3f * -rotation_direction * kart->getVelocity()->xyz[1];
      } //End of the new steering algorithm
      
   
      kart->throttle = kart->kart_properties->max_throttle;
   
      /*if (0) // use handicap
         {*/
      /* Slow down if we get too far ahead of the player... */
      /*if ( kart->getPosition() < world->getPlayerKart(0)->getPosition () &&
            kart->getVelocity()->xyz[1] > MIN_HANDICAP_VELOCITY )
         kart->getVelocity()->xyz[1] -= MAX_BRAKING * delta * 0.1f ;
      else*/
         /* Speed up if we get too far behind the player... */
         /*if ( kart->getPosition() > world->getPlayerKart(0)->getPosition () &&
               kart->getVelocity()->xyz[1] < MAX_HANDICAP_VELOCITY )
            kart->getVelocity()->xyz[1] += MAX_ACCELLERATION * delta * 1.1f ;
         else
            kart->getVelocity()->xyz[1] += MAX_ACCELLERATION * delta ;*/
         //}x
   
      //kart->getVelocity()->xyz[2] -= GRAVITY * delta ;
   
      if ( kart->wheelie_angle > 0.0f )
      {
         kart->wheelie_angle -= PITCH_RESTORE_RATE ;
      
         if ( kart->wheelie_angle <= 0.0f )
            kart->wheelie_angle = 0.0f ;
      }
   
      if ( kart->collectable != COLLECT_NOTHING )
      {
         time_since_last_shoot += delta ;
      
         if ( time_since_last_shoot > 10.0f )
         {
            kart->useAttachment () ;
            time_since_last_shoot = 0.0f ;
         }
      }
   }

