//  $Id: Driver.h,v 1.31 2004/08/24 18:17:50 grumbel Exp $
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
#include "joystick.h"
#include "KartProperties.h"

class Shadow;

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
  /** The interpolated position of the kart, this might differ a bit
      from the real position, but it is used to both give a smoother
      movement and to better visualize turns and such, use it for
      everything that needs to be visual (smoke, skidmarks, kart
      placement), but don't use it for physics, thats what curr_pos is
      for */
  sgCoord visi_pos;

  /** Used to save the last position of the kart, which is then
      interpolated with the new one to form a smooth movement */
  sgCoord last_relax_pos;

  sgCoord  reset_pos ;
  sgCoord  curr_pos  ;
  sgCoord  last_pos  ;

  sgCoord  velocity  ;
public:  
  
  /* start - New Physics */
  sgVec3   force ;

  float   steer_angle ;

  float throttle;
  float brake;
protected:
  /** Complete model, including shadow */
  ssgBranch    *comp_model;

  /** Kart model alone, without the shadow */
  ssgTransform *model ;

  /** The Karts shadow */
  Shadow* shadow;

  int history_index ;
public:
  float wheelie_angle ;
  int position ;
  int lap      ;

  int collided ;
  int crashed  ;
  int rescue   ;
  float zipper_time_left ;

  sgVec3 surface_avoidance_vector ;
  sgVec3 curr_normal ;
  bool   on_ground ; 
  int    firsttime ;
  float getIsectData     ( sgVec3 start, sgVec3 end ) ;
  float collectIsectData ( sgVec3 start, sgVec3 end ) ;

  int    track_hint ;
  sgVec2 last_track_coords ;
  sgVec2 curr_track_coords ;

  /** Height of the terrain at the current position of the kart, used
      for shadow calculation */
  float height_of_terrain;

public:
  KartProperties kart_properties;

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
    return &velocity ;
  }

  void setVelocity ( sgCoord *vel )
  {
    sgCopyCoord ( & velocity, vel ) ;
  }

  sgCoord *getVisiCoord ()
  {
    return & visi_pos ;
  }

  sgCoord *getCoord ()
  {
    return & curr_pos ;
  }

  void setCoord ( sgCoord *pos )
  {
    sgCopyCoord ( & curr_pos, pos ) ;
  }

  /** Reposition the model in the scene */
  virtual void placeModel ();

  void updateVisiPos(float delta);
  void coreUpdate (float delta) ;
  void physicsUpdate (float delta) ;
  bool is_on_ground() { return on_ground; }

  virtual void doObjectInteractions () ;
  virtual void doLapCounting        () ;
  virtual void doZipperProcessing   () ;
  virtual void doCollisionAnalysis  ( float delta, float hot ) ;
  virtual void update               (float delta) ;
} ;

#endif

/* EOF */
