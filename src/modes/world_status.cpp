//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2004-2015 SuperTuxKart-Team
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

#include "main_loop.hpp"
#include "audio/music_manager.hpp"
#include "audio/sfx_base.hpp"
#include "audio/sfx_manager.hpp"
#include "config/stk_config.hpp"
#include "config/user_config.hpp"
#include "graphics/irr_driver.hpp"
#include "guiengine/modaldialog.hpp"
#include "karts/abstract_kart.hpp"
#include "modes/world.hpp"
#include "network/network_config.hpp"
#include "network/protocols/server_lobby.hpp"
#include "network/race_event_manager.hpp"
#include "tracks/track.hpp"

#include <irrlicht.h>

//-----------------------------------------------------------------------------
WorldStatus::WorldStatus()
{
    m_clock_mode        = CLOCK_CHRONO;

    m_prestart_sound    = SFXManager::get()->createSoundSource("pre_start_race");
    m_start_sound       = SFXManager::get()->createSoundSource("start_race");
    m_track_intro_sound = SFXManager::get()->createSoundSource("track_intro");

    m_play_track_intro_sound = UserConfigParams::m_music;
    m_play_ready_set_go_sounds = true;
    m_play_racestart_sounds = true;
    m_server_is_ready       = false;

    IrrlichtDevice *device = irr_driver->getDevice();

    if (device->getTimer()->isStopped())
        device->getTimer()->start();
    m_ready_to_race = false;
}   // WorldStatus

//-----------------------------------------------------------------------------
/** Resets all status information, used when starting a new race.
 */
void WorldStatus::reset()
{
    m_time            = 0.0f;
    m_auxiliary_timer = 0.0f;
    m_count_up_timer  = 0.0f;

    m_engines_started = false;
    
    // Using SETUP_PHASE will play the track into sfx first, and has no
    // other side effects.
    m_phase           = UserConfigParams::m_race_now ? MUSIC_PHASE : SETUP_PHASE;

    // Parts of the initialisation-phase are skipped so do it here
    if (UserConfigParams::m_race_now)
    {
        // Setup music and sound
        if (World::getWorld()->getWeather() != NULL)
            World::getWorld()->getWeather()->playSound();

        // Start engines
        for (unsigned int i = 0; i < World::getWorld()->getNumKarts(); i++)
            World::getWorld()->getKart(i)->startEngineSFX();
    }

    m_previous_phase  = UNDEFINED_PHASE;
    // Just in case that the game is reset during the intro phase
    m_track_intro_sound->stop();

    IrrlichtDevice *device = irr_driver->getDevice();

    if (device->getTimer()->isStopped())
        device->getTimer()->start();

    // Set the right music
    World::getWorld()->getTrack()->startMusic();
    // In case of a networked race the race can only start once
    // all protocols are up. This flag waits for that, and is
    // set by 
    m_ready_to_race = !NetworkConfig::get()->isNetworking();
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
/** Starts the kart engines.
 */
void WorldStatus::startEngines()
{
    if (m_engines_started)
        return;

    m_engines_started = true;
    for (unsigned int i = 0; i < World::getWorld()->getNumKarts(); i++)
    {
        World::getWorld()->getKart(i)->startEngineSFX();
    }
}   // startEngines

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
/** Update, called once per frame. Called early on before physics are
 *  updated.
 *  \param dt Time step.
 */
void WorldStatus::update(float dt)
{
}   // update

//-----------------------------------------------------------------------------
/** Updates the world time and clock (which might be running backwards), and
 *  all status information, called once per frame at the end of the main
 *  loop.
 *  \param dt Duration of time step.
 */
void WorldStatus::updateTime(const float dt)
{
    // In case of a networked race wait till all necessary protocols are
    // ready before progressing the timer
//    if (!m_ready_to_race) return;

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
            
            if (m_play_track_intro_sound)
            {
                m_track_intro_sound->play();
            }

            if (World::getWorld()->getWeather() != NULL)
            {
                World::getWorld()->getWeather()->playSound();
            }

            return;   // Do not increase time
        case TRACK_INTRO_PHASE:
            m_auxiliary_timer += dt;

            if (UserConfigParams::m_artist_debug_mode &&
                race_manager->getNumberOfKarts() -
                race_manager->getNumSpareTireKarts() == 1 &&
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
                return;   // Do not increase time

            // Wait before ready phase if sounds are disabled
            if (!UserConfigParams::m_sfx && m_auxiliary_timer < 3.0f)
                return;
            
            if (!m_play_track_intro_sound)
            {
                startEngines();
                if (m_auxiliary_timer < 3.0f)
                    return;   // Do not increase time
            }

            m_auxiliary_timer = 0.0f;

            if (m_play_ready_set_go_sounds)
                m_prestart_sound->play();

            // In a networked game the client needs to wait for a notification
            // from the server that ready-set-go can start now. So client will go
            // to the 'wait_for_server_phase', from which they will progress once
            // the notification is received. In all other cases (no networking or
            // server), immediately go to race start
            if (NetworkConfig::get()->isNetworking())
            {
                m_phase = WAIT_FOR_SERVER_PHASE;
            }
            else
            {
                m_phase = READY_PHASE;
                startEngines();
            }
            return;   // Don't increase time
        case WAIT_FOR_SERVER_PHASE:
            // On a client this phase waits for the server to be ready. On a
            // server or in case of non-networked game this phase is reached
            // with m_server_is_ready set to true, so it will immediately
            // start the engines and then the race
            if(!m_server_is_ready) return;

            if (NetworkConfig::get()->isNetworking() &&
                NetworkConfig::get()->isClient())
            {
                // Each client informs the server when its starting the race.
                // The server waits for the last message (i.e. from the slowest
                // client) before starting, which means the server's local time
                // will always be behind any client's time by the latency to
                // that client. This in turn should guarantee that any message
                // from the clients reach the server before the server actually
                // needs it. By running the server behind the clients the need
                // for rollbacks on the server is greatly reduced.
                //RaceEventManager::getInstance()->clientHasStarted();
            }

            m_phase = READY_PHASE;            

            // Receiving a 'startReadySetGo' message from the server triggers
            // a call to startReadySetGo() here, which will change the phase
            // (or state) of the finite state machine.
            return;   // Don't increase time
        case READY_PHASE:
            startEngines();

            if (m_auxiliary_timer > 1.0)
            {
                if (m_play_ready_set_go_sounds)
                {
                    m_prestart_sound->play();
                }

                m_phase = SET_PHASE;
            }

            m_auxiliary_timer += dt;

            // In artist debug mode, when without opponents, skip the
            // ready/set/go counter faster
            if (UserConfigParams::m_artist_debug_mode &&
                race_manager->getNumberOfKarts() -
                race_manager->getNumSpareTireKarts() == 1 &&
                race_manager->getTrackName() != "tutorial")
            {
                m_auxiliary_timer += dt*6;
            }

            return;   // Do not increase time
        case SET_PHASE:
            if (m_auxiliary_timer > 2.0)
            {
                // set phase is over, go to the next one
                m_phase = GO_PHASE;
                if (m_play_ready_set_go_sounds)
                {
                    m_start_sound->play();
                }

                // event
                onGo();
            }

            m_auxiliary_timer += dt;

            // In artist debug mode, when without opponents, 
            // skip the ready/set/go counter faster
            if (UserConfigParams::m_artist_debug_mode &&
                race_manager->getNumberOfKarts() -
                race_manager->getNumSpareTireKarts() == 1 &&
                race_manager->getTrackName() != "tutorial")
            {
                m_auxiliary_timer += dt*6;
            }

            return;   // Do not increase time
        case GO_PHASE  :

            if (m_auxiliary_timer>2.5f && music_manager->getCurrentMusic() &&
                !music_manager->getCurrentMusic()->isPlaying())
            {
                music_manager->startMusic();
            }

            if (m_auxiliary_timer > 3.0f)    // how long to display the 'go' message
            {
                m_phase = MUSIC_PHASE;
            }

            m_auxiliary_timer += dt;

            // In artist debug mode, when without opponents,
            // skip the ready/set/go counter faster
            if (UserConfigParams::m_artist_debug_mode &&
                race_manager->getNumberOfKarts() -
                race_manager->getNumSpareTireKarts() == 1 &&
                race_manager->getTrackName() != "tutorial")
            {
                m_auxiliary_timer += dt*6;
            }

            break;   // Now the world time starts
        case MUSIC_PHASE:
            // Start the music here when starting fast
            if (UserConfigParams::m_race_now)
            {
                music_manager->startMusic();
                UserConfigParams::m_race_now = false;
            }
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

    IrrlichtDevice *device = irr_driver->getDevice();

    switch (m_clock_mode)
    {
        case CLOCK_CHRONO:
            if (!device->getTimer()->isStopped())
            {
                m_time += dt;
                m_count_up_timer += dt;
            }
            break;
        case CLOCK_COUNTDOWN:
            // stop countdown when race is over
            if (m_phase == RESULT_DISPLAY_PHASE || m_phase == FINISH_PHASE)
            {
                m_time = 0.0f;
                m_count_up_timer = 0.0f;
                break;
            }

            if (!device->getTimer()->isStopped())
            {
                m_time -= dt;
                m_count_up_timer += dt;
            }

            if(m_time <= 0.0)
            {
                // event
                countdownReachedZero();
            }

            break;
        default: break;
    }   // switch m_phase
}   // update

//-----------------------------------------------------------------------------
/** Called on the client when it receives a notification from the server that
 *  all clients (and server) are ready to start the race. The server will
 *  then additionally wait for all clients to report back that they are
 *  starting, which guarantees that the server is running far enough behind
 *  clients time that at server time T all events from the clients at time
 *  T have arrived, minimising rollback impact.
 */
void WorldStatus::startReadySetGo()
{
    m_server_is_ready = true;
}   // startReadySetGo

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
