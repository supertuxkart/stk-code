//  $Id: AutoKart.cxx,v 1.6 2005/08/30 08:56:31 joh Exp $
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
#include "constants.h"
#include "World.h"
#include "Track.h"
#include "AutoKart.h"

void AutoKart::update (float delta) {
  // perhaps add a bit of delay depending on the difficulty?
  // I.e. Stronger opponents would have less delay??    JH
  if (world->getPhase()==World::START_PHASE) {
    placeModel();
    return;
  }

  SGfloat difficulty;
  switch (world->raceSetup.difficulty) {
     case RD_EASY:
         difficulty = 0.90f;
         break;
     case RD_MEDIUM:
         difficulty = 0.97f;
         break;
     case RD_HARD:
         difficulty = 1.0f;
         break;
     default:
         std::cerr << "AI got the wrong difficulty!" << std::endl;
         difficulty = 1.0f;
  }

  bool steer = false;

  //Check if a length equal to the velocity in front of the kart inside the
  //track, otherwise, steer
  //FIXME:The tuxkart is about 1.5f long and 1.0f wide, so I'm using
  //these values for now, it won't work correctly on big or small karts.
  const float kart_length = 1.5f;
  int steps = (int)(velocity.xyz[1] / kart_length);
  if(steps == 0) steps = 1; //Always check at least space for one kart

  const int MAX_STEPS = int(world->track->getWidth()[trackHint] * 2.0f) + 2;
  if(steps > MAX_STEPS) steps = MAX_STEPS;
  sgVec3 future_coords;
  sgNormalizeVec3(future_coords, abs_velocity);
  future_coords[2] = 0.0f;

  sgVec3 future_coords2;
  sgVec3 future_track_coords;
  SGfloat distance;

  float check_width;
  for(int i = steps; i > -1 ; --i) {
    sgAddScaledVec3(future_coords2, curr_pos.xyz,
		    future_coords, kart_length * i);
    future_hint = world->track->spatialToTrack( future_track_coords,
						future_coords2, future_hint);

    distance = future_track_coords[0] > 0.0f ?  future_track_coords[0]
                                             : -future_track_coords[0];

    check_width = world->track->getWidth()[future_hint] >
                  world->track->getWidth()[trackHint]
                ? world->track->getWidth()[trackHint]
                : world->track->getWidth()[future_hint];

    if (distance + 0.75f > check_width) {
      steer = true;
      break;
    }
  }   // for i
  //Distance required for braking = speed * time - 1/2 * acceleration * (time * time)
  //Time required for braking = speed / acceleration(which should be neg)
  //FIXME: if the kart if crashing constantly(like with a wall) make it move
  //       backwards in a straight line, then move forward while turning.
  //FIXME: if the kart is going the wrong way make it move backwards while
  //       turning.
  //FIXME: if the kart's curr_pos are at less than 1 kart from crashing with
  //       the track, brake.
  //FIXME: if the kart's curr_pos are at less than 1/2 kart from crashing with
  //       another kart, brake.
  //FIXME: if a kart is in front at less than 1 kart, move towards the side of
  //       the track with most space.
  //FIXME: rotation should be dependant on how much each kart can rotate, so we
  //       don't have crazy cars.

  if(steer && wheelie_angle <= 0.0f) {
    controls.lr = sqrt(velocity.xyz[1]) * difficulty
                * (future_track_coords[0] > 0.0f ?  1.0f : -1.0f);
    if(controls.lr>1.0f) controls.lr=1.0f;
    else if (controls.lr<-1.0f) controls.lr=-1.0f;
  }
  else controls.lr = 0.0f;
  // JH FIXME: Since there is no max_natural_velocity anymore, we need
  //           a better way of determining when to slow down!
  // Original code:
  //if (velocity.xyz[1] < MAX_NATURAL_VELOCITY * (1.0f + wheelie_angle/90.0f) ) {
  //  throttle = 1.0f;
  //} else if ( velocity.xyz[1] > MAX_DECELLERATION * delta ) {
  //  throttle = -1.0f;
  //} else if ( velocity.xyz[1] < -MAX_DECELLERATION * delta ) {
  //  throttle = 1.0f;
  //} else {
  //  throttle = 0.0f;
  //}
  controls.accel = true;

  //Check if we should do a wheelie. It's the same as steering, only this one
  //checks the line that goes from velocity to velocity * 1.35
  //FIXME: instead of using 1.35, it should find out how much time it will
  //pass to stop doing the wheelie completely from the current state.
  if(world->raceSetup.difficulty != RD_EASY && !steer &&
     velocity.xyz[1] >= MIN_WHEELIE_VELOCITY)
  {
    bool do_wheelie = true;
    const int WHEELIE_STEPS = int((velocity.xyz[1] * 1.35f)/ kart_length );
    for(int i = WHEELIE_STEPS; i > steps - 1; --i) {
      sgAddScaledVec3(future_track_coords, curr_pos.xyz, future_coords,
              kart_length * i);
      world->track->spatialToTrack( future_track_coords, future_track_coords,
                    future_hint);
      distance = future_track_coords[0] > 0.0f ?  future_track_coords[0]
                                               : -future_track_coords[0];

      if (distance > world->track->getWidth()[trackHint]) {
        do_wheelie = false;
        break;
      }
    }   // for i

    if (do_wheelie) {
      if (wheelie_angle < WHEELIE_PITCH)
        wheelie_angle += WHEELIE_PITCH_RATE * delta;
      else wheelie_angle = WHEELIE_PITCH;
    }
    else if ( wheelie_angle > 0.0f ) {
      wheelie_angle -= PITCH_RESTORE_RATE * delta;
      if ( wheelie_angle <= 0.0f ) wheelie_angle = 0.0f ;
    }
  }   // if difficulty!=RD_EASY
  else if ( wheelie_angle > 0.0f ) {
    wheelie_angle -= PITCH_RESTORE_RATE * delta;
    if ( wheelie_angle <= 0.0f ) wheelie_angle = 0.0f ;
  }

  if ( collectable.getType() != COLLECT_NOTHING ) {
    time_since_last_shoot += delta ;

    if ( time_since_last_shoot > 10.0f ) {
      collectable.use() ;
      time_since_last_shoot = 0.0f ;
    }
  }   // if COLLECT_NOTHING
  Kart::update(delta);
}   // update


void AutoKart::reset() {
    future_hint = 0;

    Kart::reset();
}
