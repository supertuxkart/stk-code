//  $Id: PlayerDriver.cxx,v 1.12 2004/08/03 15:12:22 straver Exp $
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
#include "sound.h"
#include "Driver.h"

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
    
    if ( j -> buttons & 1 ) { /* A == Accellerate */
      throttle = MAX_THROTTLE;
    } else if (throttle > 0) {
    	throttle -= MAX_THROTTLE * true_delta_t;
    } else
    	throttle = 0.0f;

    if ( j -> buttons & 2 ) {  /* B == Active Braking */
      if (velocity.xyz[1] > 0) {
      	brake = MAX_THROTTLE;
      	throttle = 0.0f;
      } else {
      	brake = 0.0f;
	throttle = -MAX_THROTTLE/2;
      }
    } else {
      brake = 0.0f;
    }
      
    if ( wheelie_angle <= 0.0f ) {      
      steer_angle = -turn_speed * j->data[0];
      if ( steer_angle > MAX_WHEEL_TURN)
        steer_angle = MAX_WHEEL_TURN;
      if ( steer_angle < -MAX_WHEEL_TURN)
        steer_angle = -MAX_WHEEL_TURN;	
    }
    else
      velocity.hpr[0] = 0.0f ;
  }
  
  /* Physics debugging control*/
  if (j -> hits & 0x40) { // 1
  	printf ("Selected Inertia - value: %f\n", inertia);
	selected_property = &inertia;
  }
  if (j -> hits & 0x80) { // 2
  	printf ("Selected corner stiffness front - value: %f\n", corn_f);
	selected_property = &corn_f;
  }
  if (j -> hits & 0x100) { // 3
  	printf ("Selected corner stiffness rear - value: %f\n", corn_r);
	selected_property = &corn_r;
  }
  if (j -> hits & 0x200) { // 4
  	printf ("Selected maximum grip - value: %f\n", max_grip);
	selected_property = &max_grip;
  }
  if (j -> hits & 0x400) { // 5
  	printf ("Selected mass of kart - value: %f\n", mass);
	selected_property = &mass;
  }
  if (j -> hits & 0x800) { // 6
  	printf ("Selected wheels turn degree - value: %f\n", turn_speed);
	selected_property = &turn_speed;
  }
  if (j -> hits & 0x1000) { // +
  	*selected_property += 0.1f;
  	printf ("Increased selected value to: %f\n", *selected_property);
  }
  if (j -> hits & 0x2000) { // -
  	*selected_property -= 0.1f;
  	printf ("Decreased selected value to: %f\n", *selected_property);
  }
  

  force[2] = -GRAVITY * KART_MASS;
}

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

/* EOF */
