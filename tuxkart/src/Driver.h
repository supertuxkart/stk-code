//  $Id: Driver.h,v 1.15 2004/08/07 03:38:37 jamesgregory Exp $
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
#include "tuxkart.h"
#include "Track.h"
#include "joystick.h"

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
#define MAX_THROTTLE 100
#define ENGINE_POWER 60 	/* Newton */
#define MAX_WHEEL_TURN (M_PI/2)	/* Radians */
#define TURN_SPEED (M_PI)	/* Radians per Second */
#define KART_MASS 90		/* Kilograms */
#define KART_INERTIA 13
#define AIR_FRICTION 0.8257
#define SYSTEM_FRICTION 4.8
#define CORN_F -7.2		/* Cornering stiffness - front */
#define CORN_R -5.0		/* Cornering stiffness - rear */
#define MAX_GRIP 4.0		/* maximum wheel force */
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

class Driver
{
protected:
  float delta_t ;

  sgCoord  history [ HISTORY_FRAMES ] ;

  sgCoord relax_pos;
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
  
  // debug physics
  float mass;
  float inertia;
  float corn_f;
  float corn_r;
  float max_grip;
  float turn_speed;
  
  /* End - New Physics */

  ssgTransform *model ;
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

public:

  Driver ( ssgTransform *m )
  {
    delta_t = 0.01 ;

    firsttime = TRUE ;
    model = m ;
    
    /* New Physics */
    sgZeroVec3 (acceleration);
    sgZeroVec3 (force);
    steer_angle = throttle = brake = 0.0f;
    
    // debug physics
    mass = KART_MASS;
    inertia = KART_INERTIA;
    corn_r = CORN_R;
    corn_f = CORN_F;
    max_grip = MAX_GRIP;
    turn_speed = TURN_SPEED;    
    /* End New Physics */

    sgZeroVec3 ( reset_pos.xyz ) ; sgZeroVec3 ( reset_pos.hpr ) ;
    reset () ;
  }

  float getDistanceDownTrack () { return curr_track_coords[1] ; }
  int  getLap      ()        { return lap      ; }
  int  getPosition ()        { return position ; }
  void setPosition ( int p ) { position = p    ; }
  float getSteerAngle() const { return steer_angle; }

  void reset ()
  {
    lap = 0 ;
    position = 9 ;
    rescue = FALSE ;
    on_ground = TRUE ;
    zipper_time_left = 0.0f ;
    collided = crashed = FALSE ;
    history_index = 0 ;
    wheelie_angle = 0.0f ;

    sgZeroVec3 ( velocity.xyz ) ;
    sgZeroVec3 ( velocity.hpr ) ;
    sgCopyCoord ( &last_pos, &reset_pos ) ;
    sgCopyCoord ( &curr_pos, &reset_pos ) ;
    sgCopyCoord ( &relax_pos, &reset_pos ) ;
    sgCopyCoord ( &last_relax_pos, &reset_pos ) ;

    for ( int i = 0 ; i < HISTORY_FRAMES ; i++ )
      sgCopyCoord ( &(history[i]), &reset_pos ) ;

    track_hint = curr_track -> absSpatialToTrack ( last_track_coords,
                                                   last_pos.xyz ) ;
    track_hint = curr_track -> absSpatialToTrack ( curr_track_coords,
                                                   curr_pos.xyz ) ;

    update () ;
  }

  void setReset ( sgCoord *pos )
  {
    sgCopyCoord ( & reset_pos, pos ) ;
  }

  ssgTransform *getModel () { return model ; }

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


class KartDriver : public Driver
{
protected:
  int num_collectables ;
  int grid_position ;
  int collectable ;
  float attachment_time_left ;
  int   attachment_type ;
  int num_herring_gobbled ;
  ssgSelector *attachment ;
public:

  KartDriver ( int _position, ssgTransform *m ) ;

  void addAttachment ( ssgEntity *e )
  {
    if ( attachment == NULL )
    {
      attachment = new ssgSelector () ;
      getModel() -> addKid ( attachment ) ;
    }

    attachment -> addKid ( e ) ;
    attachment -> select ( 0 ) ;
  }

  void attach ( int which, float time )
  {
    attachment -> selectStep ( which ) ;
    attachment_time_left = time ;
    attachment_type = which ;
  }

  int getAttachment      () { return  attachment_type ; }
  int getCollectable     () { return     collectable  ; }
  int getNumCollectables () { return num_collectables ; }
  int getNumHerring      () { return num_herring_gobbled ; }

  virtual void useAttachment        () ;
  virtual void forceCrash           () ;
  virtual void doObjectInteractions () ;
  virtual void doLapCounting        () ;
  virtual void doZipperProcessing   () ;
  virtual void doCollisionAnalysis  ( float hot ) ;
  virtual void update               () ;
} ;




class NetworkKartDriver : public KartDriver
{
public:
  NetworkKartDriver ( int _pos, ssgTransform *m ) : KartDriver ( _pos, m )
  {
  }

  virtual void update () ;
} ;



class TrafficDriver : public KartDriver
{
public:
  TrafficDriver ( sgVec3 _pos, ssgTransform *m ) : KartDriver ( 0, m )
  {
    sgCopyVec3 ( reset_pos.xyz, _pos ) ;
    reset () ;
  }

  virtual void doObjectInteractions () ;
  virtual void doLapCounting        () ;
  virtual void doZipperProcessing   () ;
  virtual void update () ;
} ;


class AutoKartDriver : public KartDriver
{
  float time_since_last_shoot ;
public:
  AutoKartDriver ( int _pos, ssgTransform *m ) : KartDriver ( _pos, m )
  {
    time_since_last_shoot = 0.0f ;
  }

  virtual void update () ;
} ;


class PlayerKartDriver : public KartDriver
{  
protected:
  float    tscale ;
  float    rscale ;
  
  // physics debugging
  float *selected_property;

public:
  PlayerKartDriver ( int _pos, ssgTransform *m ) : KartDriver ( _pos, m )
  {
    tscale = 10.0 ;
    rscale =  3.0 ;
  }

  virtual void update () ;

  void incomingKeystroke ( const SDL_keysym& ) ;
  void incomingJoystick  ( JoyInfo& ji ) ;
} ;


class Projectile : public Driver
{
  KartDriver *owner ;
  int type ;

  void setType ( int _type )
  {
    type = _type ;

    switch ( type )
    {
      case COLLECT_NOTHING :
      case COLLECT_ZIPPER  :
      case COLLECT_MAGNET  :
        ((ssgSelector *)(getModel () -> getKid ( 0 ))) -> select ( 0 ) ;
        break ;

      case COLLECT_SPARK :
        ((ssgSelector *)(getModel () -> getKid ( 0 ))) -> selectStep ( 0 ) ;
        break ;

      case COLLECT_MISSILE :
        ((ssgSelector *)(getModel () -> getKid ( 0 ))) -> selectStep ( 1 ) ;
        break ;

      case COLLECT_HOMING_MISSILE :
        ((ssgSelector *)(getModel () -> getKid ( 0 ))) -> selectStep ( 2 ) ;
        break ;
    }
  }

public:
  Projectile ( ssgTransform *m ) : Driver ( m )
  {
    type = COLLECT_NOTHING ;
  }

  void off ()
  {
    setType ( COLLECT_NOTHING ) ;
  }

  void fire ( KartDriver *who, int _type )
  {
    owner = who ;
    setCoord ( who->getCoord() ) ;
    setType  ( _type ) ;
  }


  virtual void doObjectInteractions () ;
  virtual void doLapCounting        () ;
  virtual void doZipperProcessing   () ;
  virtual void doCollisionAnalysis  ( float hot ) ;
  virtual void update               () ;

} ;

#endif

/* EOF */
