//  $Id: PlayerDriver.cxx,v 1.8 2004/07/31 23:46:18 grumbel Exp $
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

#include "tuxkart.h"

int check_hint = 0 ;

void PlayerKartDriver::update ()
{
  KartDriver::update () ;
}


void PlayerKartDriver::incomingJoystick  ( JoyInfo *j )
{
  /*
    Delta-t is faked on slow machines for most activities - but
    for the player's joystick, it has to be the true delta-t.
  */
  
  mkjoo = true;

  float true_delta_t = fclock -> getDeltaTime () ;

  if ( j -> hits & 0x04 )  /* C == Fire */
  {
    if ( collectable == COLLECT_NOTHING )
      sound -> playSfx ( SOUND_BEEP ) ;

    useAttachment () ;
  }

  if ( on_ground )
  {
    if ( ( j -> buttons & 0x20 ) &&
         velocity.xyz[1] >= MIN_WHEELIE_VELOCITY )  /* D == Wheelie */
    {
      if ( wheelie_angle < WHEELIE_PITCH )
        wheelie_angle += WHEELIE_PITCH_RATE * true_delta_t ;
      else
        wheelie_angle = WHEELIE_PITCH ;
    }
    else
    if ( wheelie_angle > 0.0f )
    {
      wheelie_angle -= PITCH_RESTORE_RATE ;

      if ( wheelie_angle <= 0.0f )
        wheelie_angle = 0.0f ;
    }
 
    if ( j -> hits & 0x10 )  /* R == Jump */
      velocity.xyz[2] += JUMP_IMPULSE ;

    if ( j -> hits & 0x08 )  /* D == Unused */
    {
      sound -> playSfx ( SOUND_BEEP ) ;
      rescue = TRUE ;
    }

    if ( j -> buttons & 2 ) {  /* B == Active Braking */
      //velocity.xyz[1] -= MAX_BRAKING * true_delta_t ;
      brake = MAX_THROTTLE;
      throttle = 0.0f;
    } else {
      brake = 0.0f;
    }
      
    if ( ( j -> buttons & 1 ) 
    /* && velocity.xyz[1] < MAX_NATURAL_VELOCITY *( 1.0f + wheelie_angle/90.0f ) */ ) 
    { /* A == Accellerate */
      // trottle add forward force
      // velocity.xyz[1] += MAX_ACCELLERATION * true_delta_t ;
      throttle = MAX_THROTTLE;
    } else if (throttle > 0) {
    	throttle -= MAX_THROTTLE * true_delta_t;
    } else
    	throttle = 0.0f;
    
    /*
    else
    if ( velocity.xyz[1] > MAX_DECELLERATION * true_delta_t )
      velocity.xyz[1] -= MAX_DECELLERATION * true_delta_t ;
    else
    if ( velocity.xyz[1] < -MAX_DECELLERATION * true_delta_t )
      velocity.xyz[1] += MAX_DECELLERATION * true_delta_t ;
    else
      velocity.xyz[1] = 0.0f ;
     */

    if ( wheelie_angle <= 0.0f )
    {
      /*
      if ( velocity.xyz[1] >= 0.0f )
        velocity.hpr[0] = -MAX_TURN_RATE * sqrt( velocity.xyz[1])* j->data[0];
      else
        velocity.hpr[0] =  MAX_TURN_RATE * sqrt(-velocity.xyz[1])* j->data[0];
      */
      
      steer_angle = -TURN_SPEED * j->data[0];
      if ( steer_angle > MAX_WHEEL_TURN)
        steer_angle = MAX_WHEEL_TURN;
      if ( steer_angle < -MAX_WHEEL_TURN)
        steer_angle = -MAX_WHEEL_TURN;	
    }
    else
      velocity.hpr[0] = 0.0f ;

    //float s = SKID_RATE * velocity.hpr[0] * velocity.xyz[1] ;
    //velocity.xyz[0] = (s >= 0) ? (s*s) : -(s*s) ;
    }

    //velocity.xyz[2] -= GRAVITY * true_delta_t ;
    force[2] = -GRAVITY * KART_MASS;
}

#ifdef HAVE_LIBSDL
void PlayerKartDriver::incomingKeystroke ( const SDL_keysym& key )
{
  /* CTRL-R ...infinite ammo cheat. */
  if ( key.sym == SDLK_r && key.mod & KMOD_CTRL)
  {
    switch ( rand () % 5 )
    {
      case 0 : collectable = COLLECT_SPARK ;
        num_collectables = 1000000 ;
        break ;
      case 1 : collectable = COLLECT_MISSILE ;
        num_collectables = 1000000 ;
        break ;
      case 2 : collectable = COLLECT_HOMING_MISSILE ;
        num_collectables = 1000000 ;
        break ;
      case 3 : collectable = COLLECT_ZIPPER ;
        num_collectables = 1000000 ;
        break ;
      case 4 : collectable = COLLECT_MAGNET ;
        num_collectables = 1000000 ;
        break ;
    }
  }
}

#else
void PlayerKartDriver::incomingKeystroke ( int k )
{
  switch ( k )
    {
      /* CTRL-R ...infinite ammo cheat. */
    case  0x12: switch ( rand () % 5 )
      {
      case 0 : collectable = COLLECT_SPARK ;
        num_collectables = 1000000 ;
        break ;
      case 1 : collectable = COLLECT_MISSILE ;
        num_collectables = 1000000 ;
        break ;
      case 2 : collectable = COLLECT_HOMING_MISSILE ;
        num_collectables = 1000000 ;
        break ;
      case 3 : collectable = COLLECT_ZIPPER ;
        num_collectables = 1000000 ;
        break ;
      case 4 : collectable = COLLECT_MAGNET ;
        num_collectables = 1000000 ;
        break ;
      }
      break ;
    }
}
#endif

/* EOF */
