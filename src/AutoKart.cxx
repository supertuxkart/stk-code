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
#include <ctime>
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

    srand((unsigned)time(0));
}

//TODO: if the AI is crashing constantly, make it move backwards in a straight line, then move forward while turning.
//TODO: rotation should be dependant on how much each kart can rotate, so we don't have crazy cars.
//TODO: change_steering amount and accel by difficulties with world->raceSetup.difficulty (RD_EASY, RD_MEDIUM, RD_HARD)
void AutoKart::update (float delta)
{

  if (world->getPhase()==World::START_PHASE) {
    placeModel();
    if(starting_delay <  0.0f)
    {
        switch(world->raceSetup.difficulty)
        {
            case RD_EASY:
                starting_delay = (float)rand()/RAND_MAX * 0.5f;
                break;
            case RD_MEDIUM:
                starting_delay = (float)rand()/RAND_MAX * 0.3f;
                break;
            case RD_HARD:
                starting_delay = (float)rand()/RAND_MAX * 0.15f;
                break;
        }
    }
    return;
  }

  //FIXME:The tuxkart is about 1.5f long and 1.0f wide, so I'm using
  //these values for now, it won't work correctly on big or small karts.
  const float KART_LENGTH = 1.5f;
  const int MIN_STEPS = 2;
  const int MAX_STEPS = int(world->track->getWidth()[trackHint] * 2.0f) + MIN_STEPS;

  int steps = int(velocity.xyz[1] / KART_LENGTH);

  if(steps < MIN_STEPS) steps = MIN_STEPS;
  else if(steps > MAX_STEPS) steps = MAX_STEPS;

  check_crashes(steps);

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

controls.lr = 0.0f;

const size_t NEXT = trackHint + 1 >= world->track->driveline.size() ? 0 : trackHint + 1;
float first = curr_track_coords[0] > 0.0f ?  curr_track_coords[0]
                                         : -curr_track_coords[0];
if(first + KART_LENGTH * 0.75f > world->track->getWidth()[trackHint])
    controls.lr = steer_to_point(world->track->driveline[NEXT]);
else if(crashes.curve && wheelie_angle <= 0.0f)
{
      controls.lr = steer_to_parallel(NEXT);
}
else if(crashes.kart != -1)
{
    controls.lr = curr_track_coords[0] > world->getKart(crashes.kart)->
        getDistanceToCenter() ? steer_to_side(NEXT, STEER_RIGHT) :
        steer_to_side(NEXT, STEER_LEFT);
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
if(starting_delay < 0.0f)
{
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
}
else
{
    controls.accel = 0.0f;
    controls.brake = false;
    starting_delay -= delta;
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


SGfloat AutoKart::steer_to_side (const size_t &NEXT_HINT, const STEER_SIDE &SIDE)
{
    float steer_angle = world->track->angle[NEXT_HINT] - curr_pos.hpr[0];
    remove_angle_excess(steer_angle);

    steer_angle += SIDE ? -90.0f : 90.0f;
    remove_angle_excess(steer_angle);
    steer_angle /= getMaxSteerAngle();

    if(steer_angle > 1.0f) return 1.0f;
    else if (steer_angle < -1.0f) return -1.0f;

    return steer_angle;
}


SGfloat AutoKart::steer_to_parallel (const size_t &NEXT_HINT)
{
    //Desired angle minus current angle equals how many angles to turn
    float steer_angle = world->track->angle[NEXT_HINT] - curr_pos.hpr[0];
    remove_angle_excess(steer_angle);

    //Traslate the angle to control steering
    steer_angle /= getMaxSteerAngle();
    if(steer_angle > 1.0f) return 1.0f;
    else if (steer_angle < -1.0f) return -1.0f;

    return steer_angle;
}


SGfloat AutoKart::steer_to_point(const sgVec2 POINT)
{
      const SGfloat ADJACENT_LINE = POINT[0] - curr_pos.xyz[0];
      const SGfloat OPPOSITE_LINE = POINT[1] - curr_pos.xyz[1];
      SGfloat theta = sgATan(OPPOSITE_LINE/ADJACENT_LINE);

      //The real value depends on the side of the track that the kart is
      theta += ADJACENT_LINE < 0.0f ? 90.0f : -90.0f;

      float rot = theta - getCoord()->hpr[0];
      remove_angle_excess(rot);
      rot /= getMaxSteerAngle();

      if(rot > 1.0f) return 1.0f;
      else if(rot < -1.0f) return -1.0f;

      return rot;
}


void AutoKart::check_crashes(const int &STEPS)
{
    //FIXME:The tuxkart is about 1.5f long and 1.0f wide, so I'm using
    //these values for now, it won't work optimally on big or small karts.
    const float KART_LENGTH = 1.5f;

    sgVec2 vel_normal, step_coord, step_track_coord;
    SGfloat distance, check_width, kart_distance;

    crashes.clear();

    const size_t NUM_KARTS = world->getNumKarts();
    sgNormalizeVec2(vel_normal, abs_velocity);

    for(int i = 1; i < STEPS; ++i)
    {
        sgAddScaledVec2(step_coord, curr_pos.xyz, vel_normal, KART_LENGTH * i);

        //Don't try to find a kart to dodge if we already have found one
        if(crashes.kart == -1)
            for (size_t j = 0; j < NUM_KARTS; ++j)
            {
                if(world->getKart(j) == this) continue;

                kart_distance = sgDistanceVec2(step_coord,
                    world->getKart(j)->getCoord()->xyz);

                if(i != 1)
                {
                    if(kart_distance < KART_LENGTH/2)
                        if(velocity.xyz[1] > world->getKart(j)->
                           getVelocity()->xyz[1] * 0.75f) crashes.kart = j;
                }
                else
                {
                    if(kart_distance < KART_LENGTH)
                    {
                        if(velocity.xyz[1] > world->getKart(j)->
                           getVelocity()->xyz[1]) crashes.kart = j;
                    }
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
            crashes.curve = true;
            break;
        }
    }
}


float AutoKart::guess_accel(const float throttle)
{
    const float SysResistance   = getRollResistance() * velocity.xyz[1];
    const float AirResistance   = getAirFriction() * velocity.xyz[1] * fabs(velocity.xyz[1]);
    const float force           = throttle * getMaxPower();

    const float  mass        = getMass();
    float effForce     = (force-AirResistance-SysResistance);

    const float gravity     = world->getGravity();
    const float ForceOnRearTire   = 0.5f*mass*gravity + prevAccel*mass*getHeightCOG()/getWheelBase();
    const float ForceOnFrontTire  =      mass*gravity - ForceOnRearTire;
    const float maxGrip           = (ForceOnRearTire > ForceOnFrontTire ?
      ForceOnRearTire : ForceOnFrontTire) * getTireGrip();

  // Slipping: more force than what can be supported by the back wheels
  // --> reduce the effective force acting on the kart - currently
  //     by an arbitrary value.
  while(fabs(effForce)>maxGrip) effForce *= 0.4f;

  return effForce / mass;
}

void AutoKart::reset() {
    future_hint = 0;
    starting_delay = -1.0f;

    Kart::reset();
}

inline void AutoKart::remove_angle_excess(float &angle)
{
      if (angle > 180.0f) angle = -angle + 180.0f;
      else if (angle < -180.0f ) angle = -angle - 180.0f;
}
