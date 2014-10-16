//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2004-2013 SuperTuxKart-Team
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

#include "audio/music_manager.hpp"
#include "audio/sfx_base.hpp"
#include "audio/sfx_manager.hpp"
#include "config/stk_config.hpp"
#include "config/user_config.hpp"
#include "graphics/irr_driver.hpp"
#include "guiengine/modaldialog.hpp"
#include "karts/abstract_kart.hpp"
#include "modes/world.hpp"
#include "tracks/track.hpp"

#include <irrlicht.h>

//-----------------------------------------------------------------------------
WorldStatus::WorldStatus()
{
    m_clock_mode        = CLOCK_CHRONO;

    m_prestart_sound    = SFXManager::get()->createSoundSource("pre_start_race");
    m_start_sound       = SFXManager::get()->createSoundSource("start_race");
    m_track_intro_sound = SFXManager::get()->createSoundSource("track_intro");

    m_play_racestart_sounds = true;

    IrrlichtDevice *device = irr_driver->getDevice();

    if (device->getTimer()->isStopped())
        device->getTimer()->start();
}   // WorldStatus

//-----------------------------------------------------------------------------
/** Resets all status information, used when starting a new race.
 */
void WorldStatus::reset()
{
    m_time            = 0.0f;
    m_auxiliary_timer = 0.0f;
    // Using SETUP_PHASE will play the track into sfx first, and has no
    // other side effects.
    m_phase           = UserConfigParams::m_race_now ? RACE_PHASE : SETUP_PHASE;
    m_previous_phase  = UNDEFINED_PHASE;
    // Just in case that the game is reset during the intro phase
    m_track_intro_sound->stop();

    IrrlichtDevice *device = irr_driver->getDevice();

    if (device->getTimer()->isStopped()) 
        device->getTimer()->start();
}   // reset

//-----------------------------------------------------------------------------
/** Destructor of WorldStatus.
 */
WorldStatus::~WorldStatus()
{
    m_prestart_sound->deleteSFX();
    m_start_sound->deleteSFX();
    m_track_intro_sound->deleteSFX();
    IrrlichtDevice *device = irr_driver->getDevice();

    if (device->getTimer()->isStopped())  
        device->getTimer()->start();
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
    // Don't enter race over if it's already race over
    if (m_phase == DELAY_FINISH_PHASE || m_phase == RESULT_DISPLAY_PHASE ||
        m_phase == FINISH_PHASE)
        return;

    m_phase = DELAY_FINISH_PHASE;
    m_auxiliary_timer = 0.0f;
}   // enterRaceOverState

//-----------------------------------------------------------------------------
/** Called when it's really over (delay over if any). This function must be
 *  called after all stats were updated from the different modes!
 */
void WorldStatus::terminateRace()
{
}   // terminateRace

//-----------------------------------------------------------------------------
/** Updates all status information, called once per frame.
 *  \param dt Duration of time step.
 */
void WorldStatus::update(const float dt)
{
    switch (m_phase)
    {
        // Note: setup phase must be a separate phase, since the race_manager
        // checks the phase when updating the camera: in the very first time
        // step dt is large (it includes loading time), so the camera might
        // tilt way too much. A separate setup phase for the first frame
        // simplifies this handling
        case SETUP_PHASE:
            m_auxiliary_timer = 0.0f;
            m_phase = TRACK_INTRO_PHASE;
            
            if (m_play_racestart_sounds)
            {
                m_track_intro_sound->play();
            }

            if (World::getWorld()->getWeather() != NULL)
            {
                 World::getWorld()->getWeather()->playSound();
            }

            return;
        case TRACK_INTRO_PHASE:
            m_auxiliary_timer += dt;

            if (UserConfigParams::m_artist_debug_mode &&
                race_manager->getNumberOfKarts() == 1 &&
                race_manager->getTrackName() != "tutorial")
            {
                m_auxiliary_timer += dt * 6;
            }

            // Work around a bug that occurred on linux once:
            // the SFXManager::get() kept on reporting that it is playing,
            // while it was not - so STK would never reach the ready
            // ... phase. Since the sound effect is about 3 seconds
            // long, we use the aux timer to force the next phase
            // after 3.5 seconds.
            if (m_track_intro_sound->getStatus() == SFXBase::SFX_PLAYING &&
                m_auxiliary_timer < 3.5f)
                return;

            // Wait before ready phase if sounds are disabled
            if (!UserConfigParams::m_sfx && m_auxiliary_timer < 3.0f)
                return;

            m_auxiliary_timer = 0.0f;

            if (m_play_racestart_sounds) 
                m_prestart_sound->play();

            m_phase = READY_PHASE;
            
            for (unsigned int i = 0; i < World::getWorld()->getNumKarts(); i++)
            {
                World::getWorld()->getKart(i)->startEngineSFX();
            }

            break;
        case READY_PHASE:
            if (m_auxiliary_timer > 1.0)
            {
                if (m_play_racestart_sounds)
                {
                    m_prestart_sound->play();
                }

                m_phase = SET_PHASE;
            }

            m_auxiliary_timer += dt;

            // In artist debug mode, when without opponents, skip the ready/set/go counter faster
            if (UserConfigParams::m_artist_debug_mode &&
                race_manager->getNumberOfKarts() == 1 &&
                race_manager->getTrackName() != "tutorial")
            {
                m_auxiliary_timer += dt*6;
            }

            return;
        case SET_PHASE:
            if (m_auxiliary_timer > 2.0)
            {
                // set phase is over, go to the next one
                m_phase = GO_PHASE;
                if (m_play_racestart_sounds)
                {
                    m_start_sound->play();
                }

                World::getWorld()->getTrack()->startMusic();

                // event
                onGo();
            }

            m_auxiliary_timer += dt;

            // In artist debug mode, when without opponents, skip the ready/set/go counter faster
            if (UserConfigParams::m_artist_debug_mode &&
                race_manager->getNumberOfKarts() == 1  &&
                race_manager->getTrackName() != "tutorial")
            {
                m_auxiliary_timer += dt*6;
            }

            return;
        case GO_PHASE  :

            if (m_auxiliary_timer>2.5f && music_manager->getCurrentMusic())
            {
                music_manager->startMusic(music_manager->getCurrentMusic());
            }

            if (m_auxiliary_timer > 3.0f)    // how long to display the 'go' message
            {
                m_phase = MUSIC_PHASE;
            }

            m_auxiliary_timer += dt;

            // In artist debug mode, when without opponents, skip the ready/set/go counter faster
            if (UserConfigParams::m_artist_debug_mode &&
                race_manager->getNumberOfKarts() == 1  &&
                race_manager->getTrackName() != "tutorial")
            {
                m_auxiliary_timer += dt*6;
            }

            break;
        case MUSIC_PHASE:
            // how long to display the 'music' message
            if (m_auxiliary_timer>stk_config->m_music_credit_time)
            {
                m_phase = RACE_PHASE;
            }

            m_auxiliary_timer += dt;
            break;
        case RACE_PHASE:
            // Nothing to do for race phase, switch to delay finish phase
            // happens when
            break;
        case DELAY_FINISH_PHASE:
        {
            m_auxiliary_timer += dt;

            // Change to next phase if delay is over
            if (m_auxiliary_timer > stk_config->m_delay_finish_time)
            {
                m_phase = RESULT_DISPLAY_PHASE;
                terminateRace();
            }

            break;
        }
        case RESULT_DISPLAY_PHASE:
        {
            break;
        }
        case FINISH_PHASE:
            // Nothing to do here.
            break;
        case GOAL_PHASE:
            // Nothing to do here as well.

        default: break;
    }

    switch (m_clock_mode)
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
    assert(m_previous_phase == UNDEFINED_PHASE);

    m_previous_phase = m_phase;
    m_phase          = phase;
    IrrlichtDevice *device = irr_driver->getDevice();

    if (!device->getTimer()->isStopped())  
        device->getTimer()->stop();
}   // pause

//-----------------------------------------------------------------------------
/** Switches back from a pause state to the previous state.
 */
void WorldStatus::unpause()
{
    m_phase          = m_previous_phase;
    // Set m_previous_phase so that we can use an assert
    // in pause to detect incorrect pause/unpause sequences.
    m_previous_phase = UNDEFINED_PHASE;
    IrrlichtDevice *device = irr_driver->getDevice();

    if (device->getTimer()->isStopped()) 
        device->getTimer()->start();
}   // unpause
