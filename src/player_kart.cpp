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
#include "gui/menu_manager.hpp"
#include "gui/race_gui.hpp"
#include "translation.hpp"

void PlayerKart::action(KartActions action, int value, bool isKeyboard)
{
    if(isKeyboard) m_action_keys_values[action]=value;
    switch (action)
    {
    case KC_LEFT:
        // Special case: when releasing a key on the keyboard, take the
        // current value of the opposite steering direction. If the opposite
        // key is not pressed, this value is zero, otherwise it will 
        // pick up the previous direction (e.g. consider: press right,
        // press left, release left --> will steer right;  or:
        // press left, press right, release left --> will steer right)
        if(isKeyboard && value==0)
        {
            m_steer_val = m_action_keys_values[KC_RIGHT];
        }
        else
        {
            m_steer_val = -value;
        }
        break;
    case KC_RIGHT:
        if(isKeyboard && value==0)
        {
            m_steer_val = -m_action_keys_values[KC_LEFT];
        }
        else
        {
            m_steer_val = value;
        }
        break;
    case KC_ACCEL:
        m_accel_val = value;
        break;
    case KC_BRAKE:
        if (value)
            m_accel_val = 0;
        m_controls.brake = value;
        break;
    case KC_WHEELIE:
        m_controls.wheelie = value;
        break;
    case KC_RESCUE:
        m_controls.rescue = value;
        break;
    case KC_FIRE:
        m_controls.fire = value;
        break;
    case KC_JUMP:
#ifdef ENABLE_JUMPING
        m_controls.jump = value;
#endif
        break;
    }
}

//-----------------------------------------------------------------------------
void PlayerKart::smoothSteer(float dt, bool left, bool right)
{
    const float STEER_CHANGE = dt/getTimeFullSteer();  // amount the steering is changed
    if       (left)
    {
        m_controls.lr += STEER_CHANGE;
    }
    else if(right)
    {
        m_controls.lr -= STEER_CHANGE;
    }
    else
    {   // no key is pressed
        if(m_controls.lr>0.0f)
        {
            m_controls.lr -= STEER_CHANGE;
            if(m_controls.lr<0.0f) m_controls.lr=0.0f;
        }
        else
        {   // m_controls.lr<=0.0f;
            m_controls.lr += STEER_CHANGE;
            if(m_controls.lr>0.0f) m_controls.lr=0.0f;
        }   // if m_controls.lr<=0.0f
    }   // no key is pressed

    m_controls.lr = std::min(1.0f, std::max(-1.0f, m_controls.lr));

}   // smoothSteer

//-----------------------------------------------------------------------------
void PlayerKart::update(float dt)
{
    smoothSteer(dt, m_steer_val == -1, m_steer_val == 1);

    m_controls.accel = m_accel_val;

    if(world->getPhase()==World::START_PHASE)
    {
        if(m_controls.accel!=0.0 || m_controls.brake!=false ||
           m_controls.fire|m_controls.wheelie|m_controls.jump)
        {
            //JH Some sound here?
            m_penalty_time=1.0;
            // A warning gets displayed in RaceGUI
        }
        placeModel();
        return;
    }
    if(m_penalty_time>0.0)
    {
        m_penalty_time-=dt;
        return;
    }

    if ( m_controls.fire && !m_rescue)
    {
        if (m_collectable.getType()==COLLECT_NOTHING) sound_manager->playSfx(SOUND_BEEP);
        // use() needs to be called even if there currently is no collecteable
        // since use() tests for switching a magnet on/off.
        m_collectable.use() ;
        m_controls.fire = false;
    }
    if ( m_on_ground  &&  m_controls.rescue )
    {
        sound_manager -> playSfx ( SOUND_BEEP ) ;
        forceRescue();
        m_controls.rescue=false;
    }

    Kart::update(dt);
}   // update

//-----------------------------------------------------------------------------
void PlayerKart::forceCrash()
{
    Kart::forceCrash();
    sound_manager->playSfx( SOUND_CRASH );
}

//-----------------------------------------------------------------------------
void PlayerKart::handleZipper()
{
    Kart::handleZipper();
    sound_manager->playSfx ( SOUND_WEE );
}

//-----------------------------------------------------------------------------
void PlayerKart::collectedHerring(Herring* herring)
{
    Kart::collectedHerring(herring);
    sound_manager->playSfx ( ( herring->getType()==HE_GREEN ) ? SOUND_UGH:SOUND_GRAB);
}

//-----------------------------------------------------------------------------
void PlayerKart::reset()
{
    m_steer_val = 0;
    m_accel_val = 0;
    m_controls.accel = 0.0;
    m_controls.brake =false;
    m_controls.fire = false;
    m_controls.wheelie = false;
    m_controls.jump = false;
    m_penalty_time = 0;
    for(int i=KC_LEFT; i<=KC_FIRE; i++)
    {
        m_action_keys_values[i]=false;
    }
    Kart::reset();
}
//-----------------------------------------------------------------------------
/** This function is called by world to add any messages to the race gui. This
 *  can't be done (in some cases) in the update() function, since update can be
 *  called several times per frame, resulting in messages being displayed more 
 *  than once.
 **/
void PlayerKart::addMessages()
{
    RaceGUI* m=menu_manager->getRaceMenu();
    // This can happen if the option menu is called, since the
    // racegui gets deleted
    if(!m) return;

    // 1) check if the player is going in the wrong direction
    // ------------------------------------------------------
    if(world->m_race_setup.m_difficulty==RD_EASY)
    {
        float angle_diff = getCoord()->hpr[0] - world->m_track->m_angle[getSector()];
        if(angle_diff > 180.0f) angle_diff -= 360.0f;
        else if (angle_diff < -180.0f) angle_diff += 360.0f;
        // Display a warning message if the kart is going back way (unless
        // the kart has already finished the race).
        if ((angle_diff > 120.0f || angle_diff < -120.0f)   &&
            getVelocity () -> xyz [ 1 ] > 0.0  && !raceIsFinished() )
        {
            m->addMessage(_("WRONG WAY!"), this, -1.0f, 60);
        }  // if angle is too big
    }  // if difficulty easy

    // 2) Check if a shortcut is currently be taken
    // --------------------------------------------

    if(world->m_race_setup.m_difficulty != RD_EASY      && 
       m_shortcut_type                  == SC_OUTSIDE_TRACK)
    {
        m->addMessage(_("Invalid short-cut!"), this, -1.0f, 60);
    }

}   // addMessages
/* EOF */
