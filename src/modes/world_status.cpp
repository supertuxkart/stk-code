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

#include "modes/world_status.hpp"

#include "audio/sfx_manager.hpp"
#include "audio/sfx_base.hpp"
#include "config/stk_config.hpp"
#include "guiengine/modaldialog.hpp"
#include "network/network_manager.hpp"
#include "states_screens/dialogs/race_over_dialog.hpp"

//-----------------------------------------------------------------------------
WorldStatus::WorldStatus()
{
    m_clock_mode            = CLOCK_CHRONO;
    m_time            = 0.0f;
    m_auxiliary_timer = 0.0f;
    m_phase           = SETUP_PHASE;
    m_previous_phase  = SETUP_PHASE;  // initialise it just in case
    m_phase           = SETUP_PHASE;
    
    // FIXME - is it a really good idea to reload and delete the sound every race??
    m_prestart_sound  = sfx_manager->createSoundSource("prestart");
    m_start_sound     = sfx_manager->createSoundSource("start");
}   // WorldStatus

//-----------------------------------------------------------------------------
/** Resets all status information, used when starting a new race.
 */
void WorldStatus::reset()
{
    m_time            = 0.0f;
    m_auxiliary_timer = 0.0f;
    m_phase           = READY_PHASE; // FIXME - unsure
    m_previous_phase  = SETUP_PHASE;
}   // reset

//-----------------------------------------------------------------------------
/** Destructor of WorldStatus.
 */
WorldStatus::~WorldStatus()
{
    sfx_manager->deleteSFX(m_prestart_sound);
    sfx_manager->deleteSFX(m_start_sound);
}   // ~WorldStatus

//-----------------------------------------------------------------------------
/** Sets the clock mode and the initial time of the world clock.
 *  \param mode The new clock mode.
 *  \param initial_time The new initial time for the world clock.
 */
void WorldStatus::setClockMode(const ClockType mode, const float initial_time)
{
    m_clock_mode = mode;
    m_time = initial_time;
}   // setClockMode

//-----------------------------------------------------------------------------
/** Called when the race is finished, but it still leaves some time
 *  for an end of race animation, and potentially let some more AI karts
 *  finish the race.
 */
void WorldStatus::enterRaceOverState()
{
    // Don't
    if(    m_phase == DELAY_FINISH_PHASE
        || m_phase == RESULT_DISPLAY_PHASE
        || m_phase == FINISH_PHASE          ) return;
    
    m_phase = DELAY_FINISH_PHASE;
    m_auxiliary_timer = 0.0f;
    
}   // enterRaceOverState

//-----------------------------------------------------------------------------
/** Called when it's really over (delay over if any). This function must be
 *  called after all stats were updated from the different modes!
 */
void WorldStatus::terminateRace()
{
    pause(RESULT_DISPLAY_PHASE);
    if(network_manager->getMode()==NetworkManager::NW_SERVER)
        network_manager->sendRaceResults();
}   // terminateRace

//-----------------------------------------------------------------------------
/** Updates all status information, called once per frame.
 *  \param dt Duration of time step.
 */
void WorldStatus::update(const float dt)
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
        case RACE_PHASE:
            // Nothing to do for race phase, switch to delay finish phase
            // happens when 
            break;
        case DELAY_FINISH_PHASE :
        {
            m_auxiliary_timer += dt;
            
            // Change to next phase if delay is over
            if(m_auxiliary_timer > stk_config->m_delay_finish_time)
            {
                m_phase           = RESULT_DISPLAY_PHASE;
                new RaceOverDialog(0.6f, 0.9f);
            }
            break;
        }
        case RESULT_DISPLAY_PHASE : 
            if(((RaceOverDialog*)GUIEngine::ModalDialog::getCurrent())->menuIsFinished())
            {
                terminateRace();
                m_phase = FINISH_PHASE;
            }
            break;
        case FINISH_PHASE:
            // Nothing to do here.
            break;
        default: break;
    }
    
    switch(m_clock_mode)
    {
        case CLOCK_CHRONO:
            m_time += dt;
            break;
        case CLOCK_COUNTDOWN:
            // stop countdown when race is over
            if (m_phase == RESULT_DISPLAY_PHASE || m_phase == FINISH_PHASE)
            {
                m_time = 0.0f;
                break;
            }
            
            m_time -= dt;
            
            if(m_time <= 0.0)
            {
                // event
                countdownReachedZero();
            }
                
            break;
        default: break;
    }
}   // update

//-----------------------------------------------------------------------------
/** Sets the time for the clock.
 *  \param time New time to set.
 */
void WorldStatus::setTime(const float time)
{
    m_time = time;
}   // setTime

//-----------------------------------------------------------------------------
/** Pauses the game and switches to the specified phase.
 *  \param phase Phase to switch to.
 */
void WorldStatus::pause(Phase phase)
{
    assert(m_previous_phase==SETUP_PHASE);
    m_previous_phase = m_phase;
    m_phase          = phase;
}   // pause

//-----------------------------------------------------------------------------
/** Switches back from a pause state to the previous state.
 */
void WorldStatus::unpause()
{
    m_phase          = m_previous_phase;
    // Set m_previous_phase so that we can use an assert
    // in pause to detect incorrect pause/unpause sequences.
    m_previous_phase = SETUP_PHASE;
}
