//  $Id: PlayerDriver.cxx,v 1.15 2004/08/09 15:24:01 grumbel Exp $
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
#include "KartProperties.h"
#include "KartDriver.h"

int check_hint = 0 ;

void PlayerKartDriver::update ()
{
  KartDriver::update () ;
}


void PlayerKartDriver::incomingJoystick  (JoyInfo& ji)
{
  /*
    Delta-t is faked on slow machines for most activities - but
    for the player's joystick, it has to be the true delta-t.
  */

  float true_delta_t = fclock -> getDeltaTime () ;

  if ( ji.fire ) 
  {
    if ( collectable == COLLECT_NOTHING )
      sound -> playSfx ( SOUND_BEEP ) ;

    useAttachment () ;
  }

  if ( on_ground )
  {
    if ( ( ji.wheelie ) &&
         velocity.xyz[1] >= MIN_WHEELIE_VELOCITY ) 
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
 
    if ( ji.jump )
      velocity.xyz[2] += JUMP_IMPULSE ;

    if ( ji.rescue )
    {
      sound -> playSfx ( SOUND_BEEP ) ;
      rescue = TRUE ;
    }
    
    if ( ji.accel ) {
      throttle = kart_properties.max_throttle;
    } else if (throttle > 0) {
    	throttle -= kart_properties.max_throttle * true_delta_t;
    } else
    	throttle = 0.0f;

    if ( ji.brake ) {  
      if (velocity.xyz[1] > 0) {
      	brake = kart_properties.max_throttle;
      	throttle = 0.0f;
      } else {
      	brake = 0.0f;
	throttle = -kart_properties.max_throttle/2;
      }
    } else {
      brake = 0.0f;
    }
      
    if ( wheelie_angle <= 0.0f ) {      
      steer_angle = -kart_properties.turn_speed * ji.lr;
      if ( steer_angle > kart_properties.max_wheel_turn)
        steer_angle = kart_properties.max_wheel_turn;
      if ( steer_angle < -kart_properties.max_wheel_turn)
        steer_angle = -kart_properties.max_wheel_turn;	
    }
    else
      velocity.hpr[0] = 0.0f ;
  }
  
  /* Physics debugging control*/
  if ( keyState [ SDLK_1 ] ) {
       printf ("Selected Inertia - value: %f\n", kart_properties.inertia);
       selected_property = &kart_properties.inertia;
  }
  if ( keyState [ SDLK_2 ] ) {
  	printf ("Selected corner stiffness front - value: %f\n", kart_properties.corn_f);
	selected_property = &kart_properties.corn_f;
  }
  
  if ( keyState [ SDLK_3 ] ) {
  	printf ("Selected corner stiffness rear - value: %f\n", kart_properties.corn_r);
	selected_property = &kart_properties.corn_r;
  }
  
  if ( keyState [ SDLK_4 ] ) {
  	printf ("Selected maximum grip - value: %f\n", kart_properties.max_grip);
	selected_property = &kart_properties.max_grip;
  }
  
  if ( keyState [ SDLK_5 ] ) {
  	printf ("Selected mass of kart - value: %f\n", kart_properties.mass);
	selected_property = &kart_properties.mass;
  }
  
  if ( keyState [ SDLK_6 ] ) {
  	printf ("Selected wheels turn degree - value: %f\n", kart_properties.turn_speed);
	selected_property = &kart_properties.turn_speed;
  }
  
  if ( keyState [ SDLK_PLUS ] ) {
  	*selected_property += 0.1f;
  	printf ("Increased selected value to: %f\n", *selected_property);
  }
  
  if ( keyState [ SDLK_MINUS ] ) {
  	*selected_property -= 0.1f;
  	printf ("Decreased selected value to: %f\n", *selected_property);
  }  

  force[2] = -GRAVITY * kart_properties.mass;
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
