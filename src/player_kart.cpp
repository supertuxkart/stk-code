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

void PlayerKart::action(KartActions action, int value)
{
    switch (action)
    {
    case KC_LEFT:
        m_steer_val = -value;
        break;
    case KC_RIGHT:
        m_steer_val = value;
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
        m_rescue = true ;
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

    Kart::reset();
}

/* EOF */
