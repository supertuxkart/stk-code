//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2004-2015 Steve Baker <sjbaker1@airmail.net>
//  Copyright (C) 2006-2015 Joerg Henrichs, Steve Baker
//
//  This program is free software; you can redistribute it and/or
//  modify it under the terms of the GNU General Public License
//  as published by the Free Software Foundation; either version 3
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

#include "karts/controller/player_controller.hpp"

#include "config/user_config.hpp"
#include "input/input_manager.hpp"
#include "items/attachment.hpp"
#include "items/item.hpp"
#include "items/powerup.hpp"
#include "karts/abstract_kart.hpp"
#include "karts/kart_properties.hpp"
#include "karts/skidding.hpp"
#include "karts/rescue_animation.hpp"
#include "modes/world.hpp"
#include "race/history.hpp"
#include "states_screens/race_gui_base.hpp"
#include "utils/constants.hpp"
#include "utils/log.hpp"
#include "utils/translation.hpp"

PlayerController::PlayerController(AbstractKart *kart)
                : Controller(kart)
{
    m_penalty_time = 0.0f;
}   // PlayerController

//-----------------------------------------------------------------------------
/** Destructor for a player kart.
 */
PlayerController::~PlayerController()
{
}   // ~PlayerController

//-----------------------------------------------------------------------------
/** Resets the player kart for a new or restarted race.
 */
void PlayerController::reset()
{
    m_steer_val_l  = 0;
    m_steer_val_r  = 0;
    m_steer_val    = 0;
    m_prev_brake   = 0;
    m_prev_accel   = 0;
    m_prev_nitro   = false;
    m_penalty_time = 0;
}   // reset

// ----------------------------------------------------------------------------
/** Resets the state of control keys. This is used after the in-game menu to
 *  avoid that any keys pressed at the time the menu is opened are still
 *  considered to be pressed.
 */
void PlayerController::resetInputState()
{
    m_steer_val_l           = 0;
    m_steer_val_r           = 0;
    m_steer_val             = 0;
    m_prev_brake            = 0;
    m_prev_accel            = 0;
    m_prev_nitro            = false;
    m_controls->reset();
}   // resetInputState

// ----------------------------------------------------------------------------
/** This function interprets a kart action and value, and set the corresponding
 *  entries in the kart control data structure. This function handles esp.
 *  cases like 'press left, press right, release right' - in this case after
 *  releasing right, the steering must switch to left again. Similarly it
 *  handles 'press left, press right, release left' (in which case still
 *  right must be selected). Similarly for braking and acceleration.
 * \param action  The action to be executed.
 * \param value   If 32768, it indicates a digital value of 'fully set'
 *                if between 1 and 32767, it indicates an analog value,
 *                and if it's 0 it indicates that the corresponding button
 *                was released.
 */
void PlayerController::action(PlayerAction action, int value)
{
    switch (action)
    {
    case PA_STEER_LEFT:
        m_steer_val_l = value;
        if (value)
        {
          m_steer_val = value;
          if(m_controls->getSkidControl()==KartControl::SC_NO_DIRECTION)
              m_controls->setSkidControl(KartControl::SC_LEFT);
        }
        else
          m_steer_val = m_steer_val_r;

        break;
    case PA_STEER_RIGHT:
        m_steer_val_r = -value;
        if (value)
        {
            m_steer_val = -value;
            if(m_controls->getSkidControl()==KartControl::SC_NO_DIRECTION)
                m_controls->setSkidControl(KartControl::SC_RIGHT);
        }
        else
          m_steer_val = m_steer_val_l;

        break;
    case PA_ACCEL:
        m_prev_accel = value;
        if (value && !(m_penalty_time > 0.0f))
        {
            m_controls->setAccel(value/32768.0f);
            m_controls->setBrake(false);
            m_controls->setNitro(m_prev_nitro);
        }
        else
        {
            m_controls->setAccel(0.0f);
            m_controls->setBrake(m_prev_brake);
            m_controls->setNitro(false);
        }
        break;
    case PA_BRAKE:
        m_prev_brake = value!=0;
        // let's consider below that to be a deadzone
        if(value > 32768/2)
        {
            m_controls->setBrake(true);
            m_controls->setAccel(0.0f);
            m_controls->setNitro(false);
        }
        else
        {
            m_controls->setBrake(false);
            m_controls->setAccel(m_prev_accel/32768.0f);
            // Nitro still depends on whether we're accelerating
            m_controls->setNitro(m_prev_nitro && m_prev_accel);
        }
        break;
    case PA_NITRO:
        // This basically keeps track whether the button still is being pressed
        m_prev_nitro = (value != 0);
        // Enable nitro only when also accelerating
        m_controls->setNitro( ((value!=0) && m_controls->getAccel()) );
        break;
    case PA_RESCUE:
        m_controls->setRescue(value!=0);
        break;
    case PA_FIRE:
        m_controls->setFire(value!=0);
        break;
    case PA_LOOK_BACK:
        m_controls->setLookBack(value!=0);
        break;
    case PA_DRIFT:
        if(value==0)
            m_controls->setSkidControl(KartControl::SC_NONE);
        else
        {
            if(m_steer_val==0)
                m_controls->setSkidControl(KartControl::SC_NO_DIRECTION);
            else
                m_controls->setSkidControl(m_steer_val<0
                                           ? KartControl::SC_RIGHT
                                           : KartControl::SC_LEFT  );
        }
        break;
    case PA_PAUSE_RACE:
        if (value != 0) StateManager::get()->escapePressed();
        break;
    default:
       break;
    }

}   // action

//-----------------------------------------------------------------------------
/** Handles steering for a player kart.
 */
void PlayerController::steer(float dt, int steer_val)
{
    // Get the old value, compute the new steering value,
    // and set it at the end of this function
    float steer = m_controls->getSteer();
    if(stk_config->m_disable_steer_while_unskid &&
        m_controls->getSkidControl()==KartControl::SC_NONE &&
       m_kart->getSkidding()->getVisualSkidRotation()!=0)
    {
        steer = 0;
    }

    // Amount the steering is changed for digital devices.
    // If the steering is 'back to straight', a different steering
    // change speed is used.
    const float STEER_CHANGE = ( (steer_val<=0 && steer<0) ||
                                 (steer_val>=0 && steer>0)   )
                     ? dt/m_kart->getKartProperties()->getTurnTimeResetSteer()
                     : dt/m_kart->getTimeFullSteer(fabsf(steer));
    if (steer_val < 0)
    {
        // If we got analog values do not cumulate.
        if (steer_val > -32767)
            steer = -steer_val/32767.0f;
        else
            steer += STEER_CHANGE;
    }
    else if(steer_val > 0)
    {
        // If we got analog values do not cumulate.
        if (steer_val < 32767)
            steer = -steer_val/32767.0f;
        else
            steer -= STEER_CHANGE;
    }
    else
    {   // no key is pressed
        if(steer>0.0f)
        {
            steer -= STEER_CHANGE;
            if(steer<0.0f) steer=0.0f;
        }
        else
        {   // steer<=0.0f;
            steer += STEER_CHANGE;
            if(steer>0.0f) steer=0.0f;
        }   // if steer<=0.0f
    }   // no key is pressed

    m_controls->setSteer(std::min(1.0f, std::max(-1.0f, steer)) );

}   // steer

//-----------------------------------------------------------------------------
/** Callback when the skidding bonus is triggered. The player controller
 *  resets the current steering to 0, which makes the kart easier to control.
 */
void PlayerController::skidBonusTriggered()
{
    m_controls->setSteer(0);
}   // skidBonusTriggered

//-----------------------------------------------------------------------------
/** Updates the player kart, called once each timestep.
 */
void PlayerController::update(float dt)
{
    // Don't do steering if it's replay. In position only replay it doesn't
    // matter, but if it's physics replay the gradual steering causes
    // incorrect results, since the stored values are already adjusted.
    if (!history->replayHistory())
        steer(dt, m_steer_val);

    if (World::getWorld()->getPhase() == World::GOAL_PHASE)
    {
        m_controls->setBrake(false);
        m_controls->setAccel(0.0f);
        return;
    }

    if (World::getWorld()->isStartPhase())
    {
        if (m_controls->getAccel() || m_controls->getBrake()||
            m_controls->getFire()  || m_controls->getNitro())
        {
            // Only give penalty time in SET_PHASE.
            // Penalty time check makes sure it doesn't get rendered on every
            // update.
            if (m_penalty_time == 0.0 &&
                World::getWorld()->getPhase() == WorldStatus::SET_PHASE)
            {
                displayPenaltyWarning();
                m_penalty_time = stk_config->m_penalty_time;
            }   // if penalty_time = 0

            m_controls->setBrake(false);
            m_controls->setAccel(0.0f);
        }   // if key pressed

        return;
    }   // if isStartPhase

    if (m_penalty_time>0.0)
    {
        m_penalty_time-=dt;
        return;
    }

    // Only accept rescue if there is no kart animation is already playing
    // (e.g. if an explosion happens, wait till the explosion is over before
    // starting any other animation).
    if ( m_controls->getRescue() && !m_kart->getKartAnimation() )
    {
        new RescueAnimation(m_kart);
        m_controls->setRescue(false);
    }
}   // update

//-----------------------------------------------------------------------------
/** Called when a kart hits or uses a zipper.
 */
void PlayerController::handleZipper(bool play_sound)
{
    m_kart->showZipperFire();
}   // handleZipper
