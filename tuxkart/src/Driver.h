//  $Id: Driver.h,v 1.21 2004/08/09 15:24:01 grumbel Exp $
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

#ifndef HEADER_DRIVER_H
#define HEADER_DRIVER_H

#include <plib/ssg.h>
#include <plib/sg.h>
#include "tuxkart.h"
#include "Track.h"
#include "joystick.h"
#include "KartProperties.h"

class Shadow;

#define HISTORY_FRAMES 128

#define COLLECT_NOTHING         0
#define COLLECT_SPARK           1
#define COLLECT_MISSILE         2
#define COLLECT_HOMING_MISSILE  3
#define COLLECT_ZIPPER          4
#define COLLECT_MAGNET          5

#define ATTACH_PARACHUTE        0
#define ATTACH_MAGNET           1
#define ATTACH_MAGNET_BZZT      2
#define ATTACH_ANVIL            3
#define ATTACH_TINYTUX          4
#define ATTACH_NOTHING          999

/* Limits of Kart performance */

#define MAX_VELOCITY            (200.0f * KILOMETERS_PER_HOUR )
#define MAX_PROJECTILE_VELOCITY (200.0f * KILOMETERS_PER_HOUR )
#define MAX_HOMING_PROJECTILE_VELOCITY (105.0f * KILOMETERS_PER_HOUR )
#define MAX_NATURAL_VELOCITY    ( 60.0f * KILOMETERS_PER_HOUR )
#define MAX_PARACHUTE_VELOCITY  ( 40.0f * KILOMETERS_PER_HOUR )
#define MAX_ANVIL_VELOCITY      ( 10.0f * KILOMETERS_PER_HOUR )
#define MAX_REVERSE_VELOCITY    ( -5.0f * KILOMETERS_PER_HOUR )
#define MIN_HANDICAP_VELOCITY   ( 50.0f * KILOMETERS_PER_HOUR )
#define MAX_HANDICAP_VELOCITY   ( 70.0f * KILOMETERS_PER_HOUR )
#define TRAFFIC_VELOCITY        ( 20.0f * KILOMETERS_PER_HOUR )

/* Start - New Physics Constants */
#define SYSTEM_FRICTION 4.8
/* End - New Physics Constants */

#define MAX_ACCELLERATION       ( MAX_NATURAL_VELOCITY * 0.3f )
#define MAX_BRAKING             ( MAX_NATURAL_VELOCITY * 1.0f )
#define MAX_DECELLERATION       ( MAX_NATURAL_VELOCITY * 0.4f )

#define MAX_TURN_RATE             22.5f  /* Degrees per second. */
#define HOMING_MISSILE_TURN_RATE  (MAX_TURN_RATE*6.0f)
#define HOMING_MISSILE_PITCH_RATE (MAX_TURN_RATE/2.0f)
#define SKID_RATE                  0.0007f

#define MAGNET_RANGE         30.0f
#define MAGNET_RANGE_SQD     (MAGNET_RANGE * MAGNET_RANGE)
#define MAGNET_MIN_RANGE     4.0f
#define MAGNET_MIN_RANGE_SQD (MAGNET_MIN_RANGE * MAGNET_MIN_RANGE)

#define JUMP_IMPULSE         (0.3*GRAVITY)

#define CRASH_PITCH          -45.0f
#define WHEELIE_PITCH         45.0f
#define WHEELIE_PITCH_RATE    60.0f
#define PITCH_RESTORE_RATE    90.0f

#define MIN_WHEELIE_VELOCITY (MAX_NATURAL_VELOCITY * 0.9f)
#define MIN_CRASH_VELOCITY   (MAX_NATURAL_VELOCITY * 0.2f)
#define MIN_COLLIDE_VELOCITY (MAX_NATURAL_VELOCITY * 0.1f)
#define COLLIDE_BRAKING_RATE (MAX_NATURAL_VELOCITY * 1.0f)

#define ZIPPER_TIME          1.5f  /* Seconds */
#define ZIPPER_VELOCITY      (100.0f * KILOMETERS_PER_HOUR )

#define MAX_HERRING_EATEN    20

class KartDriver;

class Driver
{
protected:
  float delta_t ;

  sgCoord  history [ HISTORY_FRAMES ] ;
  
  /** Used to save the last position of the kart, which is then
      interpolated with the new one to form a smooth movement */
  sgCoord last_relax_pos;

  sgCoord  reset_pos ;
  sgCoord  curr_pos  ;
  sgCoord  last_pos  ;

  sgCoord  curr_vel  ;
  sgCoord  velocity  ;
  
  /* start - New Physics */
  sgVec3   acceleration ;
  sgVec3   force ;
  float   steer_angle ;
  
  float throttle;
  float brake;
  
  /** Complete model, including shadow */
  ssgBranch    *comp_model;

  /** Kart model alone, without the shadow */
  ssgTransform *model ;

  /** The Karts shadow */
  Shadow* shadow;

  int history_index ;
  float wheelie_angle ;

  int position ;
  int lap      ;

  int collided ;
  int crashed  ;
  int rescue   ;
  float zipper_time_left ;

  sgVec3 surface_avoidance_vector ;
  sgVec3 curr_normal ;
  int    on_ground ; 
  int    firsttime ;
  float getIsectData     ( sgVec3 start, sgVec3 end ) ;
  float collectIsectData ( sgVec3 start, sgVec3 end ) ;

  int    track_hint ;
  sgVec2 last_track_coords ;
  sgVec2 curr_track_coords ;

  /** Height of the terrain at the current position of the kart, used
      for shadow calculation */
  float height_of_terrain;

  KartProperties kart_properties;
public:

  Driver ( );

  virtual ~Driver() {}

  float getDistanceDownTrack () { return curr_track_coords[1] ; }
  int  getLap      ()        { return lap      ; }
  int  getPosition ()        { return position ; }
  void setPosition ( int p ) { position = p    ; }
  float getSteerAngle() const { return steer_angle; }

  void reset ();
  void setReset ( sgCoord *pos )
  {
    sgCopyCoord ( & reset_pos, pos ) ;
  }

  ssgTransform *getModel () { return model ; }
  ssgEntity *getRoot () { return comp_model ; }

  KartProperties& getKartProperties()
  {
    return kart_properties;
  }

  sgCoord *getVelocity ()
  {
    sgCopyCoord ( & curr_vel, & velocity ) ;
    return & curr_vel ;
  }

  void setVelocity ( sgCoord *vel )
  {
    sgCopyCoord ( & velocity, vel ) ;
    update () ;
  }

  sgCoord *getCoord ()
  {
    return & curr_pos ;
  }

  void setCoord ( sgCoord *pos )
  {
    sgCopyCoord ( & curr_pos, pos ) ;
    update () ;
  }

  virtual void placeModel ();

  sgCoord *getHistory ( int delay )
  {
    int q = history_index - delay ;

    if ( q < 0 )
      q += HISTORY_FRAMES ;

    return & history [ q ] ;
  }

  void coreUpdate () ;
  void physicsUpdate () ;

  virtual void doObjectInteractions () ;
  virtual void doLapCounting        () ;
  virtual void doZipperProcessing   () ;
  virtual void doCollisionAnalysis  ( float hot ) ;
  virtual void update               () ;
} ;

#endif

/* EOF */
