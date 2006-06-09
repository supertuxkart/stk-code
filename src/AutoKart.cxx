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
#include <cstdlib>
#include <cmath>
#include <cstdio>
#include <iostream>
#include "constants.h"
#include "World.h"
#include "AutoKart.h"

AutoKart::AutoKart(const KartProperties *kart_properties, int position) :
    Kart(kart_properties, position)
{
    time_since_last_shoot = 0.0f;
    future_hint = 0;

    lane_change = false;
    start_lane_pos = 0.0f;
    target_lane_pos = 0.0f;
}

//TODO: if the AI is crashing constantly, make it move backwards in a straight line, then move forward while turning.
//TODO: rotation should be dependant on how much each kart can rotate, so we don't have crazy cars.
//TODO: change_steering amount and accel by difficulties with world->raceSetup.difficulty (RD_EASY, RD_MEDIUM, RD_HARD)
//TODO: add delay to starting the race and to turning, depending on the difficulty.
//TODO: only change lanes when the kart in front of the AI is traveling at less speed than the AI
void AutoKart::update (float delta) {

  if (world->getPhase()==World::START_PHASE) {
    placeModel();
    return;
  }

  //FIXME:The tuxkart is about 1.5f long and 1.0f wide, so I'm using
  //these values for now, it won't work correctly on big or small karts.
  const float KART_LENGTH = 1.5f;
  const int MIN_STEPS = 2;
  const int MAX_STEPS = int((world->track->getWidth()[trackHint] * 2.0f) ) + MIN_STEPS;

  int steps = int(velocity.xyz[1] / KART_LENGTH);

  if(steps < MIN_STEPS) steps = MIN_STEPS;
  else if(steps > MAX_STEPS) steps = MAX_STEPS;

  CrashTypes crashes;
  check_crashes(crashes, steps);

//The next block of code sorta works but the lane change doesn't works properly yet
//#define DETECT_TIGHT_CURVES
#ifdef DETECT_TIGHT_CURVES
    float total_dist = 0.0f, angle_diff = 0.0f;
    size_t next_hint;
    for(int i = future_hint; total_dist < world->track->getWidth()[trackHint] * 8;)
    {
        next_hint = i + 1 >= world->track->driveline.size() ? 0 : i + 1;
        float dist = sgDistanceVec2(world->track->driveline[i],world->track->driveline[next_hint]);
        total_dist += dist;
        angle_diff -= world->track->angle[i] * dist;
        i = next_hint;
    }

    angle_diff = angle_diff / total_dist;

    int direction = angle_diff > 0.0f ? 1 : -1;

    if( angle_diff * direction > getMaxSteerAngle() * world->track->getWidth()[trackHint]){
        std::cout << "TIGHT curve!" << std::endl;
        float first = curr_track_coords[0] > 0.0f ?  curr_track_coords[0]
                                                 : -curr_track_coords[0];
        if(first + KART_LENGTH < world->track->getWidth()[trackHint] && !crashes.curve)
        {
            lane_change = true;
            start_lane_pos = curr_track_coords[0];
            target_lane_pos = world->track->getWidth()[trackHint] - start_lane_pos - 0.75f;
        }
    }
#endif

if(lane_change)
{
    if(!crashes.curve)
    crashes.kart = true;

    if(curr_track_coords[0] - start_lane_pos > target_lane_pos - 1.0f)
    lane_change = false;
}

controls.lr = 0.0f;
size_t next = trackHint + 1 >= world->track->driveline.size() ? 0 : trackHint + 1;
if(crashes.kart) controls.lr = change_lane(next);
    else
if(crashes.curve && wheelie_angle <= 0.0f) {

    float first = curr_track_coords[0] > 0.0f ?  curr_track_coords[0]
                                             : -curr_track_coords[0];
    if(first + KART_LENGTH > world->track->getWidth()[trackHint])
      controls.lr = find_steer_to_point(world->track->driveline[next]);
    else
    {
      controls.lr = find_steer_to_paralel(next);
    }
  }

bool brake = false;
//At the moment the AI brakes too much
//#define BRAKE
#ifdef BRAKE
float time = (velocity.xyz[1]/ -guess_accel(-1.0f));

//Braking distance is in openGL units.
float braking_distance = velocity.xyz[1] * time - (-guess_accel(-1.0f) / 2) * time * time;
if(crashes.curve && braking_distance > crashes.curve) brake = true;
#endif
if(!brake)
{

    controls.accel = 1.0f;
    controls.brake = false;
}
else
{

    controls.accel = 0.0f;
    controls.brake = true;
}

//TODO: this won't work for now, but when the wheelie code is moved to the
//Kart class it will.
  if(world->raceSetup.difficulty != RD_EASY && !crashes.curve)
    controls.wheelie = do_wheelie(steps);

  if ( collectable.getType() != COLLECT_NOTHING ) {
    time_since_last_shoot += delta ;

    if ( time_since_last_shoot > 10.0f ) {
      collectable.use() ;
      time_since_last_shoot = 0.0f ;
    }
  }   // if COLLECT_NOTHING
  Kart::update(delta);
}   // update

bool AutoKart::do_wheelie ( const int &STEPS )
{
    //FIXME:The tuxkart is about 1.5f long and 1.0f wide, so I'm using
    //these values for now, it won't work optimally on big or small karts.
    const float KART_LENGTH = 1.5f;

    sgVec2 vel_normal, step_coord, step_track_coord;
    float distance;

    sgNormalizeVec2(vel_normal, abs_velocity);

  //FIXME: instead of using 1.35, it should find out how much time it will
  //pass to stop doing the wheelie completely from the current state.
    const int WHEELIE_STEPS = int((velocity.xyz[1] * 1.35f)/ KART_LENGTH );
    for(int i = WHEELIE_STEPS; i > STEPS - 1; --i)
    {
      sgAddScaledVec2(step_coord, curr_pos.xyz, vel_normal, KART_LENGTH * i);
      world->track->spatialToTrack(step_track_coord, step_coord, future_hint);
      distance = step_track_coord[0] > 0.0f ?  step_track_coord[0]
                                            : -step_track_coord[0];

      if (distance > world->track->getWidth()[trackHint]) return false;
    }

/*  Coz: I feel a bit uncomfortable deleting this until the wheelie is moved
 *  to the Kart class.

    if (do_wheelie) {
      if (wheelie_angle < WHEELIE_PITCH)
        wheelie_angle += WHEELIE_PITCH_RATE * delta;
      else wheelie_angle = WHEELIE_PITCH;
    }
    else if ( wheelie_angle > 0.0f ) {
      wheelie_angle -= PITCH_RESTORE_RATE * delta;
      if ( wheelie_angle <= 0.0f ) wheelie_angle = 0.0f ;
    }
  }
  else if ( wheelie_angle > 0.0f ) {
    wheelie_angle -= PITCH_RESTORE_RATE * delta;
    if ( wheelie_angle <= 0.0f ) wheelie_angle = 0.0f ;
  }*/

  return true;
}

SGfloat AutoKart::change_lane ( const size_t &NEXT_HINT )
{
    sgLineSegment3 path;
    sgCopyVec2(path.a, world->track->driveline[trackHint]);
    sgCopyVec2(path.b, world->track->driveline[NEXT_HINT]);

    const SGfloat CURR_DIST = sgDistanceVec2(curr_pos.xyz, world->track->driveline[trackHint]);
    const SGfloat NEXT_DIST = sgDistanceVec2(curr_pos.xyz, world->track->driveline[NEXT_HINT]) - CURR_DIST;
    const SGfloat PERC = (sgDistToLineSegmentVec3 ( path, curr_pos.xyz ) -
        CURR_DIST) / NEXT_DIST * 0.01f;

    const SGfloat RESULT = world->track->angle[NEXT_HINT] * PERC +
        world->track->angle[NEXT_HINT] * (1.0f - PERC);

    SGfloat steer_angle = RESULT - curr_pos.hpr[0];

    if (steer_angle < -180.0){ steer_angle += 180; steer_angle = -steer_angle;}
    else if (steer_angle > 180.0){ steer_angle -= 180; steer_angle = -steer_angle;}

    const float LANE_ANGLE = curr_track_coords[0] - start_lane_pos <
        target_lane_pos / 2 ? -90.0f : 90.0f;
    steer_angle = (LANE_ANGLE + steer_angle) / 2;

    steer_angle /= getMaxSteerAngle();

    if(steer_angle > 1.0f) return 1.0f;
    else if (steer_angle < -1.0f) return -1.0f;

    return steer_angle;
}

SGfloat AutoKart::find_steer_to_paralel (const size_t &NEXT_HINT)
{
    sgLineSegment3 path;
    sgCopyVec2(path.a, world->track->driveline[trackHint]);
    sgCopyVec2(path.b, world->track->driveline[NEXT_HINT]);

    const SGfloat CURR_DIST = sgDistanceVec2(curr_pos.xyz, world->track->driveline[trackHint]);
    const SGfloat NEXT_DIST = sgDistanceVec2(curr_pos.xyz, world->track->driveline[NEXT_HINT]) - CURR_DIST;
    const SGfloat PERC = (sgDistToLineSegmentVec3 ( path, curr_pos.xyz ) -
        CURR_DIST) / NEXT_DIST * 0.01f;

    const SGfloat RESULT = world->track->angle[NEXT_HINT] * PERC +
        world->track->angle[NEXT_HINT] * (1.0f - PERC);

    SGfloat steer_angle = RESULT - curr_pos.hpr[0];

    if (steer_angle < -180.0){ steer_angle += 180; steer_angle = -steer_angle;}
    else if (steer_angle > 180.0){ steer_angle -= 180; steer_angle = -steer_angle;}

    steer_angle /= getMaxSteerAngle();

    if(steer_angle > 1.0f) return 1.0f;
    else if (steer_angle < -1.0f) return -1.0f;

    return steer_angle;
}

SGfloat AutoKart::find_steer_to_point(const sgVec2 POINT)
{
      SGfloat adjacent_line = POINT[0] - curr_pos.xyz[0];
      SGfloat opposite_line = POINT[1] - curr_pos.xyz[1];
      SGfloat theta = atanf(opposite_line/adjacent_line) * SG_RADIANS_TO_DEGREES;

      //The real value depends on the side of the track that the kart is
      theta += adjacent_line < 0.0f ? 90.0f : -90.0f;

      float rot = theta - getCoord()->hpr[0];

      if (rot > 180.0f) rot = -rot + 180.0f;
      else if (rot < -180.0f ) rot = -rot - 180.0f;

      rot /= getMaxSteerAngle();
      if(rot > 1.0f) return 1.0f;
      else if(rot < -1.0f) return -1.0f;

      return rot;
}

void AutoKart::check_crashes(AutoKart::CrashTypes &crashes, const int &STEPS)
{
    //FIXME:The tuxkart is about 1.5f long and 1.0f wide, so I'm using
    //these values for now, it won't work optimally on big or small karts.
    const float KART_LENGTH = 1.5f;

    sgVec2 vel_normal, step_coord, step_track_coord;
    SGfloat distance, check_width, kart_distance;

    const size_t NUM_KARTS = world->getNumKarts();
    sgNormalizeVec2(vel_normal, abs_velocity);

    for(int i = 1; i < STEPS; ++i)
    {
        sgAddScaledVec2(step_coord, curr_pos.xyz, vel_normal, KART_LENGTH * i);

        //Don't try to find a kart to dodge if we already have found one
        if(!crashes.kart)
            for (size_t j = 0; j < NUM_KARTS; ++j)
            {
                if(world->getKart(j) == this) continue;

                kart_distance = sgDistanceVec2(step_coord,
                    world->getKart(j)->getCoord()->xyz);

                if(kart_distance < KART_LENGTH/2)
                {
                    if(kart_distance > 1.5f) kart_distance = 1.5f;
                    crashes.kart = kart_distance;
                }
            }

        future_hint = world->track->spatialToTrack( step_track_coord,
                            step_coord, future_hint);

        distance = step_track_coord[0] > 0.0f ? step_track_coord[0]
                                              : -step_track_coord[0];

        check_width = world->track->getWidth()[future_hint] >
                      world->track->getWidth()[trackHint]
                    ? world->track->getWidth()[trackHint]
                    : world->track->getWidth()[future_hint];

        if (distance + KART_LENGTH/2 > check_width)
        {
            crashes.curve = sgDistanceVec2(curr_pos.xyz, step_coord);
            break;
        }
    }
}

float AutoKart::guess_accel(const float throttle)
{
  float  rollResist  = getRollResistance();
  float  airFriction = getAirFriction();   // includes attachmetn.AirFrictAdjust

  float SysResistance   = rollResist*velocity.xyz[1];
  float AirResistance   = airFriction*velocity.xyz[1]*fabs(velocity.xyz[1]);
  float force           = throttle * getMaxPower();

  float  mass        = getMass();          // includes attachment.WeightAdjust
  float effForce     = (force-AirResistance-SysResistance);

  float  gravity     = world->getGravity();
  float  wheelBase   = getWheelBase();
  float ForceOnRearTire   = 0.5f*mass*gravity + prevAccel*mass*getHeightCOG()/wheelBase;
  float ForceOnFrontTire  =      mass*gravity - ForceOnRearTire;
#define max(m,n) ((m)>(n) ? (m) : (n))
  float maxGrip           = max(ForceOnRearTire,ForceOnFrontTire)*getTireGrip();
  // Slipping: more force than what can be supported by the back wheels
  // --> reduce the effective force acting on the kart - currently
  //     by an arbitrary value.
  while(fabs(effForce)>maxGrip) {
    effForce *= 0.6f;
    skidding  = true;
  }   // while effForce>maxGrip

  float accel       = effForce / mass;

  return accel;
}

void AutoKart::reset() {
    future_hint = 0;

    Kart::reset();
}
