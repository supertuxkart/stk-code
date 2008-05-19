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
#include "scene.hpp"
#include "camera.hpp"

PlayerKart::PlayerKart(const std::string& kart_name, int position, Player *player,
                       sgCoord init_pos, int player_index) :
            Kart(kart_name, position, init_pos)
{
    m_player       = player;
    m_penalty_time = 0.0f;
    m_camera       = scene->createCamera(player_index, this);
    m_camera->setMode(Camera::CM_NORMAL);
    reset();
}   // PlayerKart

//-----------------------------------------------------------------------------
void PlayerKart::reset()
{
    m_steer_val_l = 0;
    m_steer_val_r = 0;
    m_steer_val = 0;
    m_accel_val = 0;
    m_controls.accel = 0.0;
    m_controls.brake =false;
    m_controls.fire = false;
    m_controls.wheelie = false;
    m_controls.jump = false;
    m_penalty_time = 0;
    m_time_last_crash_sound = -10.0f;
    m_camera->setMode(Camera::CM_NORMAL);   // can be changed if camera was eliminated
    Kart::reset();
}   // reset

// ----------------------------------------------------------------------------
void PlayerKart::action(KartAction action, int value)
{
    switch (action)
    {
    case KA_LEFT:
        m_steer_val_l = -value;
        if (value)
          m_steer_val = -value;
        else
          m_steer_val = m_steer_val_r;

        break;
    case KA_RIGHT:
        m_steer_val_r = value;
        if (value)
          m_steer_val = value;
        else
          m_steer_val = m_steer_val_l;

        break;
    case KA_ACCEL:
        m_accel_val = value;
        break;
    case KA_BRAKE:
        if (value)
            m_accel_val = 0;
        m_controls.brake = (value!=0);  // This syntax avoid visual c++ warning (when brake=value)
        break;
    case KA_WHEELIE:
        m_controls.wheelie = (value!=0);
        break;
    case KA_RESCUE:
        m_controls.rescue = (value!=0);
        break;
    case KA_FIRE:
        m_controls.fire = (value!=0);
        break;
    case KA_LOOK_BACK:
        m_camera->setMode(value!=0 ? Camera::CM_REVERSE : Camera::CM_NORMAL);
        break;
    case KA_JUMP:
        m_controls.jump = (value!=0);
        break;
    }
}   // action

//-----------------------------------------------------------------------------
void PlayerKart::steer(float dt, int steer_val)
{
    const float STEER_CHANGE = dt/getTimeFullSteer();  // amount the steering is changed
    if (steer_val < 0)
    {
      // If we got analog values do not cumulate.
      if (steer_val > -32767)
        m_controls.lr = -steer_val/32767.0f;
      else
        m_controls.lr += STEER_CHANGE;
    }
    else if(steer_val > 0)
    {
      // If we got analog values do not cumulate.
      if (steer_val < 32767)
        m_controls.lr = -steer_val/32767.0f;
      else
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

}   // steer

//-----------------------------------------------------------------------------
void PlayerKart::update(float dt)
{
    steer(dt, m_steer_val);

    m_controls.accel = m_accel_val / 32768.0f;

    if(world->isStartPhase())
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

    if ( m_controls.fire && !isRescue())
    {
        if (m_collectable.getType()==COLLECT_NOTHING) sound_manager->playSfx(SOUND_BEEP);
        // use() needs to be called even if there currently is no collecteable
        // since use() can test if something needs to be switched on/off.
        m_collectable.use() ;
        m_controls.fire = false;
    }

    // We can't restrict rescue to fulfil isOnGround() (which would be more like
    // MK), since e.g. in the City track it is possible for the kart to end
    // up sitting on a brick wall, with all wheels in the air :((
    if ( m_controls.rescue )
    {
        sound_manager -> playSfx ( SOUND_BEEP ) ;
        forceRescue();
        m_controls.rescue=false;
    }

    Kart::update(dt);
}   // update

//-----------------------------------------------------------------------------
void PlayerKart::crashed()
{
    Kart::crashed();
    // A collision is usually reported several times, even when hitting
    // something only once. This results in a kind of 'machine gun'
    // noise by playing the crash sound over and over again. To prevent
    // this, the crash sound is only played if there was at least 0.5
    // seconds since the last time it was played (for this kart)
    if(world->getTime() - m_time_last_crash_sound > 0.5f) 
    {
        sound_manager->playSfx( SOUND_CRASH );
        m_time_last_crash_sound = world->getTime();
    }
}   // crashed

//-----------------------------------------------------------------------------
/** Checks if the kart was overtaken, and if so plays a sound
*/
void PlayerKart::setPosition(int p)
{
    if(getPosition()<p)
    {
        sound_manager->playSfx(SOUND_BEEP);
    }
    Kart::setPosition(p);
}   // setPosition

//-----------------------------------------------------------------------------
void PlayerKart::handleZipper()
{
    Kart::handleZipper();
    sound_manager->playSfx ( SOUND_WEE );
}   // handleZipper

//-----------------------------------------------------------------------------
void PlayerKart::collectedHerring(Herring* herring)
{
    Kart::collectedHerring(herring);
    sound_manager->playSfx ( ( herring->getType()==HE_GREEN ) ? SOUND_UGH:SOUND_GRAB);
}   // collectedHerring

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
    if(race_manager->getDifficulty()==RaceManager::RD_EASY)
    {
        float angle_diff = getCoord()->hpr[0] - world->m_track->m_angle[getSector()];
        if(angle_diff > 180.0f) angle_diff -= 360.0f;
        else if (angle_diff < -180.0f) angle_diff += 360.0f;
        // Display a warning message if the kart is going back way (unless
        // the kart has already finished the race).
        if ((angle_diff > 120.0f || angle_diff < -120.0f)   &&
            getVelocity().getY() > 0.0f  && !raceIsFinished() )
        {
            m->addMessage(_("WRONG WAY!"), this, -1.0f, 60);
        }  // if angle is too big
    }  // if difficulty easy

}   // addMessages
/* EOF */
