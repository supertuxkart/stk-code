//  $Id: AutoDriver.cxx,v 1.5 2004/08/14 12:53:29 grumbel Exp $
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

#include "World.h"
#include "KartDriver.h"
#include "AutoDriver.h"

inline float sgnsq ( float x ) { return ( x < 0 ) ? -(x * x) : (x * x) ; }

void AutoDriver::update ()
{
  assert(kart);

  /* Steering algorithm */

  /* If moving left-to-right and on the left - or right to left
     and on the right - do nothing. */

  sgVec2 track_velocity ;
  sgSubVec2 ( track_velocity, kart->curr_track_coords, kart->last_track_coords ) ;

  if ( ( track_velocity [ 0 ] < 0.0f && kart->curr_track_coords [ 0 ] > 0.0f ) ||
       ( track_velocity [ 0 ] > 0.0f && kart->curr_track_coords [ 0 ] < 0.0f ) )
    kart->velocity.hpr[0] = sgnsq(kart->curr_track_coords[0])*3.0f ;
  else
    kart->velocity.hpr[0] = sgnsq(kart->curr_track_coords[0])*12.0f ;

  /* Slow down if we get too far ahead of the player... */

  if ( kart->position < World::current()->kart[0]->getPosition () &&
       kart->velocity.xyz[1] > MIN_HANDICAP_VELOCITY )
    kart->velocity.xyz[1] -= MAX_BRAKING * kart->delta_t * 0.1f ;
  else
  /* Speed up if we get too far behind the player... */
  if ( kart->position > World::current()->kart[0]->getPosition () &&
       kart->velocity.xyz[1] < MAX_HANDICAP_VELOCITY )
    kart->velocity.xyz[1] += MAX_ACCELLERATION * kart->delta_t * 1.1f ;
  else
    kart->velocity.xyz[1] += MAX_ACCELLERATION * kart->delta_t ;

  kart->velocity.xyz[2] -= GRAVITY * kart->delta_t ;

  if ( kart->wheelie_angle > 0.0f )
  {
    kart->wheelie_angle -= PITCH_RESTORE_RATE ;
 
    if ( kart->wheelie_angle <= 0.0f )
      kart->wheelie_angle = 0.0f ;
  }

  if ( kart->collectable != COLLECT_NOTHING )
  {
    time_since_last_shoot += kart->delta_t ;

    if ( time_since_last_shoot > 10.0f )
    {
      kart->useAttachment () ;
      time_since_last_shoot = 0.0f ;
    }
  }
}


void NetworkDriver::update ()
{
}


