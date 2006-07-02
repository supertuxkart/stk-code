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
#include "constants.hpp"
#include "world.hpp"
#include "auto_kart.hpp"

AutoKart::AutoKart(const KartProperties *kart_properties, int position) :
    Kart(kart_properties, position)
{
    time_since_last_shoot = 0.0f;
    future_hint = 0;
    next_curve_hint = -1;
    next_straight_hint = -1;
    on_curve = false;
    handle_curve = false;

    srand((unsigned)time(0));
}

//TODO: if the AI is crashing constantly, make it move backwards in a straight line, then move forward while turning.
//TODO: rotation should be dependant on how much each kart can rotate, so we don't have crazy cars.
//TODO: change_steering amount and accel by difficulties with world->raceSetup.difficulty (RD_EASY, RD_MEDIUM, RD_HARD)
//TODO: turn into constants all the stuff called from world or kart that are used constantly
//TODO: make the AI steer differently based on the difficulty
void AutoKart::update (float delta)
{
    if( world->getPhase() == World::START_PHASE )
    {
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

    const int STEPS = calc_steps();
    check_crashes(STEPS);

    const float DIST_FROM_CENTER = curr_track_coords[0] > 0.0f ?
        curr_track_coords[0] : -curr_track_coords[0];
    const size_t DRIVELINE_SIZE = world->track->driveline.size();
    const size_t NEXT_HINT = trackHint + 1 >= DRIVELINE_SIZE ? 0 : trackHint + 1;

    //If the kart is outside of the track steer back to it
    const float KART_LENGTH = 1.5f;
    if(DIST_FROM_CENTER - KART_LENGTH * 0.5f > world->track->getWidth()[trackHint])
    {
        controls.lr = steer_to_point(world->track->driveline[NEXT_HINT]);
    }
    //If there is a tight curve ahead, try to steer like a racing curve
    else if(handle_tight_curves()) controls.lr = steer_for_tight_curve();
    //If it seems like the kart will crash with a curve, try to remain
    //in the same direction of the track
    else if(crashes.curve) controls.lr = steer_to_angle(NEXT_HINT, 0.0f);
    //If we are going to crash against a kart, avoid it
    else if(crashes.kart != -1)
    {
        const float PERCENTAGE = curr_track_coords[0] /
            world->track->getWidth()[trackHint];

        if(curr_track_coords[0] > world->getKart(crashes.kart)->
           getDistanceToCenter())
        {
            if(PERCENTAGE < 0.25f) controls.lr = steer_to_angle(NEXT_HINT, -90.0f);
        }
        else
        {
            if(PERCENTAGE > -0.25f) controls.lr = steer_to_angle(NEXT_HINT, 90.0f);
        }
/*
        controls.lr = curr_track_coords[0] > world->getKart(crashes.kart)->
            getDistanceToCenter() ? steer_to_angle(NEXT_HINT, -90.0f) :
            steer_to_angle(NEXT_HINT, 90.0f);*/
    }
    //Steer to the fartest point in a straight line without crashing
    else
        controls.lr = steer_to_point(
            world->track->driveline[ find_non_crashing_hint() ] );


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
    controls.wheelie = do_wheelie(STEPS);


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

  return true;
}


float AutoKart::steer_to_angle (const size_t& NEXT_HINT, const float& ANGLE)
{
    //Desired angle minus current angle equals how many angles to turn
    float steer_angle = world->track->angle[NEXT_HINT] - curr_pos.hpr[0];
    remove_angle_excess(steer_angle);

    steer_angle += ANGLE;
    remove_angle_excess(steer_angle);

    //Traslate the angle to control steering
    steer_angle /= getMaxSteerAngle();
    if(steer_angle > 1.0f) return 1.0f;
    else if (steer_angle < -1.0f) return -1.0f;

    return steer_angle;
}


float AutoKart::steer_to_point(const sgVec2 POINT)
{
      const SGfloat ADJACENT_LINE = POINT[0] - curr_pos.xyz[0];
      const SGfloat OPPOSITE_LINE = POINT[1] - curr_pos.xyz[1];
      SGfloat theta = sgATan(OPPOSITE_LINE/ADJACENT_LINE);

      //The real value depends on the side of the track that the kart is
      theta += ADJACENT_LINE < 0.0f ? 90.0f : -90.0f;

      //FIXME: change 'rot' for steer_angle
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
                    if(kart_distance < KART_LENGTH*0.5f)
                        if(velocity.xyz[1] > world->getKart(j)->
                           getVelocity()->xyz[1] * 0.75f) crashes.kart = j;
                }
                else
                  if(kart_distance < KART_LENGTH)
                  {
                      if(velocity.xyz[1] > world->getKart(j)->
                         getVelocity()->xyz[1]) crashes.kart = j;
                  }
            }

        future_hint = world->track->spatialToTrack (step_track_coord,
                            step_coord, future_hint);

        distance = step_track_coord[0] > 0.0f ? step_track_coord[0]
                                              : -step_track_coord[0];

        check_width = world->track->getWidth()[future_hint] >
                      world->track->getWidth()[trackHint]
                    ? world->track->getWidth()[trackHint]
                    : world->track->getWidth()[future_hint];

        if (distance + KART_LENGTH * 0.5f > check_width)
        {
            crashes.curve = true;
            break;
        }
    }
}


int AutoKart::find_non_crashing_hint()
{
    //FIXME:The tuxkart is about 1.5f long and 1.0f wide, so I'm using
    //these values for now, it won't work correctly on big or small karts.
    const float KART_LENGTH = 1.5f;
    int count = 0;

    const size_t DRIVELINE_SIZE = world->track->driveline.size();
    while(1)
    {
        int next_hint = trackHint + 1 >= DRIVELINE_SIZE ? 0 : trackHint + 1;
        int target_hint = next_hint + count;
        int hint = next_hint;

        sgVec2 direction;
        sgSubVec2(direction, world->track->driveline[target_hint], curr_pos.xyz);

        int steps = int(sgLengthVec2(direction) / KART_LENGTH);
        if(steps < 1) steps = 1;

        sgVec2 step_coord, step_track_coord;
        SGfloat distance;

        sgNormalizeVec2(direction);

        for(int i = 2; i < steps; ++i)
        {
            sgAddScaledVec2(step_coord, curr_pos.xyz, direction, KART_LENGTH * i);
            world->track->spatialToTrack (step_track_coord, step_coord, hint);

            distance = step_track_coord[0] > 0.0f ? step_track_coord[0]
                                                  : -step_track_coord[0];

            if (distance + KART_LENGTH * 0.25f > world->track->getWidth()[trackHint])
            {

                target_hint = target_hint - 1 < 0 ? DRIVELINE_SIZE - 1 : target_hint - 1;
                return target_hint;
                break;
            }
        }
        ++count;
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
    next_curve_hint = -1;
    next_straight_hint = -1;
    on_curve = false;
    handle_curve = false;

    Kart::reset();
}


inline void AutoKart::remove_angle_excess(float &angle)
{
    if (angle > 180.0f) angle -= 360.0f;
    else if (angle < -180.0f ) angle += 360.0f;
}


int AutoKart::calc_steps()
{
    //FIXME:The tuxkart is about 1.5f long and 1.0f wide, so I'm using
    //these values for now, it won't work correctly on big or small karts.
    const float KART_LENGTH = 1.5f;
    const int MIN_STEPS = 2;
    const int MAX_STEPS = int(world->track->getWidth()[trackHint] * 2.0f) + MIN_STEPS;

    int steps = int(velocity.xyz[1] / KART_LENGTH);
    if(steps < MIN_STEPS) steps = MIN_STEPS;
    else if(steps > MAX_STEPS) steps = MAX_STEPS;

    return steps;
}


bool AutoKart::handle_tight_curves()
{
  /*if(!crashes.curve)*/
  {
    if(hint_is_behind(next_straight_hint))
    {
        handle_curve = false;
        on_curve = false;
    }

    if(!handle_curve)
    {
        next_curve_hint = find_curve( find_check_hint() );
        if(next_curve_hint != -1) setup_curve_handling();
    }
    else
    {
        if(hint_is_behind(next_curve_hint)) on_curve = true;
        return true;
    }
  } //if(!crashes.curve)

  return false;
}


bool AutoKart::hint_is_behind(const int& HINT)
{
    const int DRIVELINE_SIZE = world->track->driveline.size();
    int pos = DRIVELINE_SIZE - int(trackHint) + HINT;
    if(pos > DRIVELINE_SIZE) pos -= DRIVELINE_SIZE;
    if(pos > DRIVELINE_SIZE * 0.5f) return true;

    return false;
}


int AutoKart::find_curve(const int& HINT)
{
    const int DRIVELINE_SIZE = world->track->driveline.size();
    float total_dist = 0.0f;
    size_t next_hint;

    for(int i = HINT; total_dist < velocity.xyz[1]; i = next_hint)
    {
        next_hint = i + 1 >= DRIVELINE_SIZE ? 0 : i + 1;
        float dist = sgDistanceVec2(world->track->driveline[i], world->track->driveline[next_hint]);
        total_dist += dist;
        float ang_diff = (world->track->angle[next_hint] - world->track->angle[i]) * dist;
        if(ang_diff < 0.0f) ang_diff = -ang_diff;

        if(ang_diff > getMaxSteerAngle()) return next_hint;
    }

    return -1;
}


int AutoKart::find_check_hint()
{
   //Find where to start checking for curves
    const int DRIVELINE_SIZE = world->track->driveline.size();
    float total_dist = 0.0f;
    size_t next_hint = trackHint;

    for(int i = trackHint; total_dist < velocity.xyz[1]; i = next_hint)
    {
        next_hint = i + 1 >= DRIVELINE_SIZE ? 0 : i + 1;
        total_dist += sgDistanceVec2(world->track->driveline[i],world->track->driveline[next_hint]);
    }

    return next_hint;
}


void AutoKart::setup_curve_handling()
{
  size_t next_hint;
  const int DRIVELINE_SIZE = world->track->driveline.size();

  float total_ang_diff = 0.0f, total_dist = 0.0f;
  float dist, ang_diff, pos_ang_diff;

  //Find the angle of the curve
  int i;
  for(i = next_curve_hint; total_dist < velocity.xyz[1]; i = next_hint)
  {
      next_hint = i + 1 >= DRIVELINE_SIZE ? 0 : i + 1;

      dist = sgDistanceVec2(world->track->driveline[i],
          world->track->driveline[next_hint]);
      total_dist += dist;

      ang_diff = world->track->angle[next_hint] - world->track->angle[i];
      remove_angle_excess(ang_diff);

      pos_ang_diff = ang_diff > 0.0f ? ang_diff : -ang_diff;
      next_straight_hint = i;
      if(pos_ang_diff < getMaxSteerAngle()) break;
      total_ang_diff += ang_diff;
  }
  next_straight_hint = i;

  curve_direction = total_ang_diff > 0.0f ? LEFT : RIGHT;

  if(total_ang_diff < 0.0f) total_ang_diff = -total_ang_diff;
  //If the curve is has more than 90.0f degrees
  if(total_ang_diff > 90.0f)
  {
      total_ang_diff = total_ang_diff / total_dist;

      //FIXME: getSteerAngle should be replaced by how much the kart can steer
      if(total_ang_diff < getMaxSteerAngle()) handle_curve = false;
      else handle_curve = true;
  }
  else handle_curve = false;

}


float AutoKart::steer_for_tight_curve()
{
      const float PERCENTAGE = curr_track_coords[0] /
          world->track->getWidth()[trackHint];
      const size_t NEXT_HINT = trackHint + 1 >=
          world->track->driveline.size() ? 0 : trackHint + 1;

      if(!on_curve)
      {
        if(curve_direction == LEFT)
        {
            if(PERCENTAGE < 0.5f)
               return steer_to_angle(NEXT_HINT, -22.5f);
        }
        else if(PERCENTAGE > -0.5f)
               return steer_to_angle(NEXT_HINT, 22.5f);
      }
      else
      {
        if(curve_direction == RIGHT)
        {
            if(PERCENTAGE < 0.333f)
                return steer_to_angle(NEXT_HINT, -90.0f);
        }
        else if(PERCENTAGE > -0.333f)
               return steer_to_angle(NEXT_HINT, 90.0f);
      }

      return steer_to_angle(NEXT_HINT, 0.0f);
}

