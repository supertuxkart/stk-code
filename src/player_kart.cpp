//  $Id$
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
#include "sound_manager.hpp"
#include "player_kart.hpp"
#include "player.hpp"
#include "sdldrv.hpp"
#include "herring.hpp"
#include "world.hpp"
#include "config.hpp"

// -----------------------------------------------------------------------------
void PlayerKart::action(KartActions action, int value)
{
  switch (action)
  {
    case KC_LEFT:
      steerVal = -value;
      break;
    case KC_RIGHT:
      steerVal = value;
      break;
    case KC_ACCEL:
      accelVal = value;
      break;
    case KC_BRAKE:
      if (value)
        accelVal = 0;
      controls.brake = value;
      break;
    case KC_WHEELIE:
      controls.wheelie = value;
      break;
    case KC_RESCUE:
      controls.rescue = value;
      break;
    case KC_FIRE:
      controls.fire = value;
      break;
    case KC_JUMP:
      controls.jump = value;
      break;
   }
}

void PlayerKart::smoothSteer(float dt, bool left, bool right)
{
    float steerChange = dt/getTimeFullSteer();  // amount the steering is changed
    if       (left)  { controls.lr += steerChange;
    } else if(right) { controls.lr -= steerChange; 
    } else {   // no key is pressed
      if(controls.lr>0.0f) {
	controls.lr -= steerChange;
	if(controls.lr<0.0f) controls.lr=0.0f;
      } else {   // controls.lr<=0.0f;
	controls.lr += steerChange;
	if(controls.lr>0.0f) controls.lr=0.0f;
      }   // if controls.lr<=0.0f
    }   // no key is pressed

  controls.lr = std::min(1.0f, std::max(-1.0f, controls.lr));

}   // smoothSteer

// -----------------------------------------------------------------------------
void PlayerKart::update(float dt) {
  smoothSteer(dt, steerVal == -1, steerVal == 1);

  controls.accel = accelVal;

  if(world->getPhase()==World::START_PHASE) {
    if(controls.accel!=0.0 || controls.brake!=false ||
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
    if (collectable.getType()==COLLECT_NOTHING) sound_manager->playSfx(SOUND_BEEP);
    // use() needs to be called even if there currently is no collecteable
    // since use() tests for switching a magnet on/off.
    collectable.use() ;
    controls.fire = false;
  }
  if ( on_ground  &&  controls.rescue ) {
    sound_manager -> playSfx ( SOUND_BEEP ) ;
    rescue = TRUE ;
    controls.rescue=false;
  }

  Kart::update(dt);
}   // update

// -----------------------------------------------------------------------------
void PlayerKart::forceCrash() {
  Kart::forceCrash();
  sound_manager->playSfx( SOUND_CRASH );
}

// -----------------------------------------------------------------------------
void PlayerKart::handleZipper() {
  Kart::handleZipper();
  sound_manager->playSfx ( SOUND_WEE );
}

// -----------------------------------------------------------------------------
void PlayerKart::collectedHerring(Herring* herring) {
    Kart::collectedHerring(herring);
    sound_manager->playSfx ( ( herring->getType()==HE_GREEN ) ? SOUND_UGH:SOUND_GRAB);
}

void PlayerKart::reset()
{
    steerVal = 0;
    accelVal = 0;
    controls.accel = 0.0;
    controls.brake =false;
    controls.fire = false;
    controls.wheelie = false;
    controls.jump = false;
    penaltyTime = 0;

    Kart::reset();
}

/* EOF */
