//  $Id: AutoDriver.cxx,v 1.3 2004/08/09 15:24:01 grumbel Exp $
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

#include "KartDriver.h"

inline float sgnsq ( float x ) { return ( x < 0 ) ? -(x * x) : (x * x) ; }

void AutoKartDriver::update ()
{
  /* Steering algorithm */

  /* If moving left-to-right and on the left - or right to left
     and on the right - do nothing. */

  sgVec2 track_velocity ;
  sgSubVec2 ( track_velocity, curr_track_coords, last_track_coords ) ;

  if ( ( track_velocity [ 0 ] < 0.0f && curr_track_coords [ 0 ] > 0.0f ) ||
       ( track_velocity [ 0 ] > 0.0f && curr_track_coords [ 0 ] < 0.0f ) )
    velocity.hpr[0] = sgnsq(curr_track_coords[0])*3.0f ;
  else
    velocity.hpr[0] = sgnsq(curr_track_coords[0])*12.0f ;

  /* Slow down if we get too far ahead of the player... */

  if ( position < kart[0]->getPosition () &&
       velocity.xyz[1] > MIN_HANDICAP_VELOCITY )
    velocity.xyz[1] -= MAX_BRAKING * delta_t * 0.1f ;
  else
  /* Speed up if we get too far behind the player... */
  if ( position > kart[0]->getPosition () &&
       velocity.xyz[1] < MAX_HANDICAP_VELOCITY )
    velocity.xyz[1] += MAX_ACCELLERATION * delta_t * 1.1f ;
  else
    velocity.xyz[1] += MAX_ACCELLERATION * delta_t ;

  velocity.xyz[2] -= GRAVITY * delta_t ;

  if ( wheelie_angle > 0.0f )
  {
    wheelie_angle -= PITCH_RESTORE_RATE ;
 
    if ( wheelie_angle <= 0.0f )
      wheelie_angle = 0.0f ;
  }

  if ( collectable != COLLECT_NOTHING )
  {
    time_since_last_shoot += delta_t ;

    if ( time_since_last_shoot > 10.0f )
    {
      useAttachment () ;
      time_since_last_shoot = 0.0f ;
    }
  }

  KartDriver::update () ;
}


void NetworkKartDriver::update ()
{
  KartDriver::update () ;
}


