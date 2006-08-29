//  $Id: player_kart.cpp,v 1.11 2005/08/30 08:56:31 joh Exp $
//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2004-2005 Steve Baker <sjbaker1@airmail.net>
//  Copyright (C) 2006 Joerg Henrichs, Steve Baker
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

#include "constants.hpp"
#include "sound.hpp"
#include "player_kart.hpp"
#include "player.hpp"
#include "plibdrv.hpp"
#include "herring.hpp"
#include "world.hpp"
#include "config.hpp"

// -----------------------------------------------------------------------------
void PlayerKart::incomingJoystick  (const KartControl &ctrl) {
  //Steering keys(hold)
  controls.lr      = -ctrl.data[0];
  joystickWasMoved = fabsf(controls.lr)>0.01;
  controls.accel   = -ctrl.data[1];
  controls.brake   = player->getButton(KC_BRAKE)   & ctrl.buttons;
  controls.wheelie = player->getButton(KC_WHEELIE) & ctrl.buttons;

  //One time press keys; these are cleared each frame so we don't have to
  if (player->getButton(KC_RESCUE) & ctrl.presses) controls.rescue = true;
  if (player->getButton(KC_FIRE)   & ctrl.presses) controls.fire   = true;
  if (player->getButton(KC_JUMP)   & ctrl.presses) controls.jump   = true;
}   // incomingJoystick

// -----------------------------------------------------------------------------
// Only keys which must keep on working when still being pressed
// are handled here, not 'one time action' keys like fire, ...
void PlayerKart::handleKeyboard(float dt) {

  if(!config->newKeyboardStyle) {
    controls.lr = isKeyDown(player->getKey(KC_LEFT))  ?  1.0f 
                : isKeyDown(player->getKey(KC_RIGHT)) ? -1.0f : 0.0f;
  } else {
    float steerChange = dt/getTimeFullSteer();  // amount the steering is changed
    if       (isKeyDown(player->getKey(KC_LEFT)))  { controls.lr += steerChange;
    } else if(isKeyDown(player->getKey(KC_RIGHT))) { controls.lr -= steerChange; 
    } else {   // no key is pressed
      if(controls.lr>0.0f) {
	controls.lr -= steerChange;
	if(controls.lr<0.0f) controls.lr=0.0f;
      } else {   // controls.lr<=0.0f;
	controls.lr += steerChange;
	if(controls.lr>0.0f) controls.lr=0.0f;
      }   // if controls.lr<=0.0f
    }   // no key is pressed
  }   // not old steering
    // clamp control value to be within  [-1,1]
  controls.lr = std::min(1.0f, std::max(-1.0f, controls.lr));

  if(isKeyDown(player->getKey(KC_ACCEL)))   controls.accel   =  1.0f;
  if(isKeyDown(player->getKey(KC_BRAKE)))   controls.brake   =  true;
  if(isKeyDown(player->getKey(KC_WHEELIE))) controls.wheelie =  true;

}   // handleKeyboard

// -----------------------------------------------------------------------------
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
void PlayerKart::update(float dt) {
  // Joystick values takes precedence over keyboard, only
  // do the keyboard handling if joystick wasn't moved.
  if(!joystickWasMoved) handleKeyboard(dt);

  if(world->getPhase()==World::START_PHASE) {
    if(controls.lr!=0.0 || controls.accel!=0.0 || controls.brake!=false ||
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

    if ( controls.rescue ) {
      sound -> playSfx ( SOUND_BEEP ) ;
      rescue = TRUE ;
      controls.rescue=false;
    }
  }

  Kart::update(dt);
  controls.accel   = 0.0f;
  controls.brake   = false;
  controls.wheelie = false;
  controls.jump    = 0.0f;
}   // update

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
