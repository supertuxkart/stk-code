//  $Id: Moveable.h,v 1.6 2005/09/30 16:57:32 joh Exp $
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

#ifndef HEADER_MOVEABLE_H
#define HEADER_MOVEABLE_H

#include <plib/ssg.h>

/* Limits of Kart performance */

#define MAX_VELOCITY            (200.0f * KILOMETERS_PER_HOUR )
#define MAX_NATURAL_VELOCITY    ( 60.0f * KILOMETERS_PER_HOUR )
#define MAX_PARACHUTE_VELOCITY  ( 40.0f * KILOMETERS_PER_HOUR )
#define MAX_ANVIL_VELOCITY      ( 10.0f * KILOMETERS_PER_HOUR )
#define MAX_REVERSE_VELOCITY    ( -5.0f * KILOMETERS_PER_HOUR )
#define MIN_HANDICAP_VELOCITY   ( 50.0f * KILOMETERS_PER_HOUR )
#define MAX_HANDICAP_VELOCITY   ( 70.0f * KILOMETERS_PER_HOUR )
#define TRAFFIC_VELOCITY        ( 20.0f * KILOMETERS_PER_HOUR )

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

#define MAX_HERRING_EATEN    20


class Moveable {
protected:
  sgCoord       reset_pos;      /* Where to start in case of a reset           */
  sgCoord       curr_pos;       /* current position                            */
  sgCoord       velocity;       /* current velocity                            */
  sgVec3        abs_velocity;   /* world coordinates' velocity vector        */
//FIXME:Is the last_relax_pos variable used anywhere?
  sgCoord       last_relax_pos; /* Used to save the last position of the kart, 
				   which is then interpolated with the new one
				   to form a smooth movement                   */
  ssgTransform* model;
  ssgTransform* shadow;
  int           collided;
  int           crashed;
  sgVec3        surface_avoidance_vector ;
  int           firsttime ;
  float         wheelie_angle ;
  int           on_ground ; 

  float collectIsectData ( sgVec3 start, sgVec3 end ) ;
  sgCoord*      historyVelocity;
  sgCoord*      historyPosition;

public:
  
  /* start - New Physics */
  

  Moveable (bool bHasHistory=false);
  virtual ~Moveable();

  void          setReset     (sgCoord* pos)  {sgCopyCoord( &reset_pos, pos ); }
  ssgTransform* getModel     ()              {return model ;                  }
  int           isOnGround   ()              {return on_ground;               }
  sgCoord*      getVelocity  ()              {return & velocity;              }
  sgCoord*      getCoord     ()              {return &curr_pos;               }
  void          setCoord     (sgCoord* pos)  {sgCopyCoord ( &curr_pos,pos);   }
  virtual void  placeModel   ()              {model->setTransform(&curr_pos); }
  virtual void  handleZipper ()              {};
  virtual void  reset        ();
  virtual void  update       (float dt) ;
  virtual void  doCollisionAnalysis(float dt, float hot);

  // Gets called when no high of terrain can be determined (isReset=0), or 
  // there is a 'reset' material under the moveable --> karts need to be 
  // rescued, missiles should explode.
  virtual void  OutsideTrack (int isReset) {}

  float         getIsectData (sgVec3 start, sgVec3 end );
  void          WriteHistory (char* s, int kartNumber, int indx);
  void          ReadHistory  (char* s, int kartNumber, int indx);
};   // class Moveable

#endif
