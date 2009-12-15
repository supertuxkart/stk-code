//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2004 SuperTuxKart-Team
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

#include "modes/clock.hpp"

#include "audio/sfx_manager.hpp"
#include "audio/sfx_base.hpp"
#include "config/stk_config.hpp"
#include "network/network_manager.hpp"
#include "states_screens/dialogs/race_over_dialog.hpp"

//-----------------------------------------------------------------------------
TimedRace::TimedRace()
{
    m_mode            = CHRONO;
    m_time            = 0.0f;
    m_auxiliary_timer = 0.0f;
    m_phase           = SETUP_PHASE;
    m_previous_phase  = SETUP_PHASE;  // initialise it just in case
    m_phase           = SETUP_PHASE;
    
    // FIXME - is it a really good idea to reload and delete the sound every race??
    m_prestart_sound  = sfx_manager->newSFX(SFXManager::SOUND_PRESTART);
    m_start_sound     = sfx_manager->newSFX(SFXManager::SOUND_START);
}   // TimedRace

//-----------------------------------------------------------------------------
void TimedRace::reset()
{
    m_time = 0.0f;
    m_auxiliary_timer = 0.0f;
    m_phase = READY_PHASE; // FIXME - unsure
    m_previous_phase      = SETUP_PHASE;
}   // reset

//-----------------------------------------------------------------------------
TimedRace::~TimedRace()
{
    sfx_manager->deleteSFX(m_prestart_sound);
    sfx_manager->deleteSFX(m_start_sound);
}   // ~TimedRace

//-----------------------------------------------------------------------------
void TimedRace::setClockMode(const ClockType mode, const float initial_time)
{
    m_mode = mode;
    m_time = initial_time;
}   // setClockMode

//-----------------------------------------------------------------------------
void TimedRace::enterRaceOverState(const bool delay)
{
    if(m_phase == DELAY_FINISH_PHASE || m_phase == FINISH_PHASE) return; // we already know
    
    if(delay)
    {
        m_phase = DELAY_FINISH_PHASE;
        m_auxiliary_timer = 0.0f;
    }
    else
        m_phase = FINISH_PHASE;
    
    if(network_manager->getMode()==NetworkManager::NW_SERVER)
        network_manager->sendRaceResults();
}   // enterRaceOverState

//-----------------------------------------------------------------------------
void TimedRace::update(const float dt)
{
    switch(m_phase)
    {
        // Note: setup phase must be a separate phase, since the race_manager
        // checks the phase when updating the camera: in the very first time
        // step dt is large (it includes loading time), so the camera might
        // tilt way too much. A separate setup phase for the first frame
        // simplifies this handling
        case SETUP_PHASE:
            m_auxiliary_timer = 0.0f;  
            m_phase = READY_PHASE;
            m_prestart_sound->play();
            return;               // loading time, don't play sound yet
        case READY_PHASE:
            if(m_auxiliary_timer>1.0)
            {
                m_phase=SET_PHASE;   
                m_prestart_sound->play();
            }
            m_auxiliary_timer += dt;
            return;
        case SET_PHASE  :
            if(m_auxiliary_timer>2.0) 
            {
                // set phase is over, go to the next one
                m_phase=GO_PHASE;
                
                m_start_sound->play();
                
                // event
                onGo();
            }
            m_auxiliary_timer += dt;
            return;
        case GO_PHASE  :
            if(m_auxiliary_timer>3.0)    // how long to display the 'go' message  
                m_phase=MUSIC_PHASE;    
            m_auxiliary_timer += dt;
            break;
        case MUSIC_PHASE:
            // how long to display the 'music' message
            if(m_auxiliary_timer>stk_config->m_music_credit_time)
                m_phase=RACE_PHASE;  
            m_auxiliary_timer += dt;
            break;
        case DELAY_FINISH_PHASE :
        {
            m_auxiliary_timer += dt;
            
            // Nothing more to do if delay time is not over yet
            if(m_auxiliary_timer < stk_config->m_delay_finish_time) break;
            
            m_phase = FINISH_PHASE;
            
            new RaceOverDialog(0.6f, 0.9f);
            
            break;            
        }
        case FINISH_PHASE:
            // event
            terminateRace();
            return;
        default: break;  // default for RACE_PHASE, LIMBO_PHASE
    }
    
    switch(m_mode)
    {
        case CHRONO:
            m_time += dt;
            break;
        case COUNTDOWN:
            m_time -= dt;
            
            if(m_time <= 0.0)
            {
                // event
                countdownReachedZero();
            }
                
                break;
        default: break;
    }
}
//-----------------------------------------------------------------------------
void TimedRace::setTime(const float time)
{
    m_time = time;
}
//-----------------------------------------------------------------------------
void TimedRace::pause()
{
    m_previous_phase = m_phase;
    m_phase = LIMBO_PHASE;
}
//-----------------------------------------------------------------------------
void TimedRace::unpause()
{
    m_phase = m_previous_phase;
}

