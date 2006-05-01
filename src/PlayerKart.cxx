//  $Id: PlayerKart.cxx,v 1.11 2005/08/30 08:56:31 joh Exp $
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

#include "constants.h"
#include "sound.h"
#include "PlayerKart.h"
#include "Player.h"
#include "plibdrv.h"
#include "Herring.h"
#include "World.h"

// Only keys which must keep on working when still being pressed
// are handled here, not 'one time action' keys like fire, ...
void PlayerKart::doSteering() {

  if(isKeyDown(player->keys[KC_LEFT]   )) controls.lr      = -1.0f;
  if(isKeyDown(player->keys[KC_RIGHT]  )) controls.lr      =  1.0f;
  if(isKeyDown(player->keys[KC_UP]     )) controls.accel   =  1;
  if(isKeyDown(player->keys[KC_DOWN]   )) controls.brake   =  1;
  if(isKeyDown(player->keys[KC_WHEELIE])) controls.wheelie = true;

}   // doSteering

// Gets called by RaceGUI when one of the non-steering keys
// was pressed. The entries in the CONTROLS data structure
// are reset in update(dt) after handling the corresponding
// action.
void PlayerKart::action(int key) {
  switch (key) {
    case KC_FIRE:    controls.fire    = true; break;
    case KC_JUMP:    controls.jump    = true; break;
    case KC_RESCUE:  controls.rescue  = true; break;
  }   // switch key
}   // action

// -----------------------------------------------------------------------------
#define OLDUPDATE
void PlayerKart::update(float dt) {
  doSteering();
  if(world->getPhase()==World::START_PHASE) {
    if(controls.lr!=0.0 || controls.accel!=0.0 || controls.brake!=0.0 ||
       controls.fire|controls.wheelie|controls.jump) {
      //JH Some sound here?
      penaltyTime=1.0;
      // A warning gets displayed in RaceGUI
    }
    placeModel();
    return;
  }
  if(penaltyTime>0.0) {
    penaltyTime-=dt;
    return;
  }
#ifdef OLDUPDATE
 if ( controls.fire ) {
    if (collectable.getType()==COLLECT_NOTHING) sound->playSfx(SOUND_BEEP);
    // use() needs to be called even if there currently is no collecteable
    // since use() tests for switching a magnet on/off.
    collectable.use() ;
    controls.fire = false;
  }
  if ( on_ground ) {
    if ( controls.wheelie && velocity.xyz[1] >= MIN_WHEELIE_VELOCITY ) {
      if ( wheelie_angle < WHEELIE_PITCH )
        wheelie_angle += WHEELIE_PITCH_RATE * dt ;
      else
        wheelie_angle = WHEELIE_PITCH ;
    } else if ( wheelie_angle > 0.0f ) {
      wheelie_angle -= PITCH_RESTORE_RATE * dt;
      if ( wheelie_angle <= 0.0f ) wheelie_angle = 0.0f ;
    }

    if (controls.jump) {
      if(on_ground) velocity.xyz[2] += JUMP_IMPULSE ;
      controls.jump=false;
    }

    if ( controls.rescue ) {
      sound -> playSfx ( SOUND_BEEP ) ;
      rescue = TRUE ;
      controls.rescue=false;
    }

    if ( controls.brake && on_ground )  /* B == Active Braking */
      velocity.xyz[1] -= MAX_BRAKING * dt ;
    else
    if ( controls.accel &&
         velocity.xyz[1] < MAX_NATURAL_VELOCITY *
                       ( 1.0f + wheelie_angle/90.0f ) )  /* A == Accellerate */
      velocity.xyz[1] += MAX_ACCELLERATION * dt ;
    else
    if ( velocity.xyz[1] > MAX_DECELLERATION * dt )
      velocity.xyz[1] -= MAX_DECELLERATION * dt ;
    else
    if ( velocity.xyz[1] < -MAX_DECELLERATION * dt )
      velocity.xyz[1] += MAX_DECELLERATION * dt ;
    else
      velocity.xyz[1] = 0.0f ;

    if ( wheelie_angle <= 0.0f )
    {
      if ( velocity.xyz[1] >= 0.0f )
        velocity.hpr[0] = -MAX_TURN_RATE * sqrt( velocity.xyz[1])*controls.lr;
      else
        velocity.hpr[0] =  MAX_TURN_RATE * sqrt(-velocity.xyz[1])*controls.lr;
    }
    else
      velocity.hpr[0] = 0.0f ;

    float s = SKID_RATE * velocity.hpr[0] * velocity.xyz[1] ;
    velocity.xyz[0] = (s >= 0) ? (s*s) : -(s*s) ;
  }

  velocity.xyz[2] -= GRAVITY * dt ;

#else
  if ( controls.fire ) {
    if ( getCollectable() == COLLECT_NOTHING )
      beginPowerslide ();
    //sound -> playSfx ( SOUND_BEEP ) ;

    collectable.use();
    controls.fire = false;
  } else endPowerslide ();

  if ( ( controls.wheelie ) &&
        getVelocity()->xyz[1] >= MIN_WHEELIE_VELOCITY ) {
    if ( wheelie_angle < WHEELIE_PITCH )
      wheelie_angle += WHEELIE_PITCH_RATE * dt ;
    else
      wheelie_angle = WHEELIE_PITCH ;
  } else if ( wheelie_angle > 0.0f ) {
    wheelie_angle -= PITCH_RESTORE_RATE * dt;
    if ( wheelie_angle <= 0.0f ) wheelie_angle = 0.0f ;
  }

  if (controls.jump) {
    if (on_ground) getVelocity()->xyz[2] += JUMP_IMPULSE ;
    controls.jump=false;
  }

  if ( controls.rescue ) {
    sound -> playSfx ( SOUND_BEEP ) ;
    rescue = TRUE ;
    controls.rescue=false;
  }

  if ((controls.accel) && (on_ground)) {
    throttle = kart_properties->max_throttle;
  } else if (throttle > 0) {
    throttle -= kart_properties->max_throttle * dt;
  } else throttle = 0.0f;

  if ((controls.brake) && (on_ground)) {
    if (getVelocity()->xyz[1] > 0) {
      brake = kart_properties->max_throttle/2;
      throttle = 0.0f;
    } else {
      brake = 0.0f;
      throttle = -kart_properties->max_throttle/2;
    }
  } else {
    brake = 0.0f;
  }   // if !controls.brake || !on_ground

  if ((wheelie_angle <= 0.0f) && (on_ground)) {
    steer_angle = -kart_properties->turn_speed * controls.lr;

    if ( steer_angle > kart_properties->max_wheel_turn)
      steer_angle = kart_properties->max_wheel_turn;
    if ( steer_angle < -kart_properties->max_wheel_turn)
      steer_angle = -kart_properties->max_wheel_turn;
  } else
    getVelocity()->hpr[0] = 0.0f ;
#endif
  controls.lr      = 0;
  controls.accel   = 0;
  controls.brake   = 0;
  controls.wheelie = false;
  Kart::update(dt);
}   // update

// -----------------------------------------------------------------------------
void PlayerKart::incomingJoystick  (const KartControl &ctrl) {
  //Steering keys(hold)
  controls.lr = ctrl.data[0] ;
  controls.accel = player->buttons[KC_UP] & ctrl.buttons;
  controls.brake = player->buttons[KC_DOWN] & ctrl.buttons;
  controls.wheelie = player->buttons[KC_WHEELIE] & ctrl.buttons;

  //One time press keys; these are cleared each frame so we don't have to
  if (player->buttons[KC_RESCUE] & ctrl.presses) controls.rescue = true;
  if (player->buttons[KC_FIRE] & ctrl.presses) controls.fire = true;
  if (player->buttons[KC_JUMP] & ctrl.presses) controls.jump = true;
}   // incomingJoystick

// -----------------------------------------------------------------------------
void PlayerKart::forceCrash() {
  Kart::forceCrash();
  sound->playSfx( SOUND_BONK );
}

// -----------------------------------------------------------------------------
void PlayerKart::handleZipper() {
  Kart::handleZipper();
  sound->playSfx ( SOUND_WEE );
}

// -----------------------------------------------------------------------------
void PlayerKart::collectedHerring(Herring* herring) {
    Kart::collectedHerring(herring);
    sound->playSfx ( ( herring->getType()==HE_GREEN ) ? SOUND_UGH:SOUND_BURP);
}
/* EOF */
