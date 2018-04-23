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
#include "network/protocols/client_lobby.hpp"
#include "network/rewind_manager.hpp"
#include "network/race_event_manager.hpp"
#include "tracks/track.hpp"

#include <irrlicht.h>

//-----------------------------------------------------------------------------
WorldStatus::WorldStatus()
{
    main_loop->setFrameBeforeLoadingWorld();
    m_clock_mode        = CLOCK_CHRONO;

    m_prestart_sound    = SFXManager::get()->createSoundSource("pre_start_race");
    m_start_sound       = SFXManager::get()->createSoundSource("start_race");
    m_track_intro_sound = SFXManager::get()->createSoundSource("track_intro");

    m_play_track_intro_sound = UserConfigParams::m_music;
    m_play_ready_set_go_sounds = true;
    m_play_racestart_sounds = true;
    m_server_is_ready.store(false);

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
    m_adjust_time_by  = 0.0f;
    m_time_ticks      = 0;
    m_auxiliary_ticks = 0;
    m_count_up_ticks  = 0;

    m_engines_started = false;
    
    // Using SETUP_PHASE will play the track into sfx first, and has no
    // other side effects.
    m_phase           = UserConfigParams::m_race_now ? MUSIC_PHASE : SETUP_PHASE;

    // Parts of the initialisation-phase are skipped so do it here
    if (UserConfigParams::m_race_now)
    {
        // Setup music and sound
        if (Weather::getInstance())
            Weather::getInstance()->playSound();

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
    Track::getCurrentTrack()->startMusic();
    // In case of a networked race the race can only start once
    // all protocols are up. This flag is used to wait for
    // a confirmation before starting the actual race.
    m_server_is_ready.store(false);
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
    m_time_ticks = stk_config->time2Ticks(initial_time);
    m_time       = stk_config->ticks2Time(m_time_ticks);
}   // setClockMode

//-----------------------------------------------------------------------------
/** Called when the race is finished, but it still leaves some time
 *  for an end of race animation, and potentially let some more AI karts
 *  finish the race.
 */
void WorldStatus::enterRaceOverState()
{
    // Waiting for server result info
    auto cl = LobbyProtocol::get<ClientLobby>();
    if (cl && !cl->receivedServerResult())
        return;

    // Don't enter race over if it's already race over
    if (m_phase == DELAY_FINISH_PHASE || m_phase == RESULT_DISPLAY_PHASE ||
        m_phase == FINISH_PHASE)
        return;

    m_phase = DELAY_FINISH_PHASE;
    m_auxiliary_ticks = 0;
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
void WorldStatus::update(int ticks)
{
}   // update

//-----------------------------------------------------------------------------
/** Updates the world time and clock (which might be running backwards), and
 *  all status information, called once per frame at the end of the main
 *  loop.
 *  \param ticks Number of ticks (physics time steps) - should be 1.
 */
void WorldStatus::updateTime(int ticks)
{
    switch (m_phase)
    {
        // Note: setup phase must be a separate phase, since the race_manager
        // checks the phase when updating the camera: in the very first time
        // step dt is large (it includes loading time), so the camera might
        // tilt way too much. A separate setup phase for the first frame
        // simplifies this handling
        case SETUP_PHASE:
            m_auxiliary_ticks= 0;
            m_phase = TRACK_INTRO_PHASE;
            
            if (m_play_track_intro_sound)
            {
                m_track_intro_sound->play();
            }

            if (Weather::getInstance())
            {
                Weather::getInstance()->playSound();
            }

            return;   // Do not increase time
        case TRACK_INTRO_PHASE:
            m_auxiliary_ticks++;

            if (UserConfigParams::m_artist_debug_mode &&
                !NetworkConfig::get()->isNetworking() &&
                race_manager->getNumberOfKarts() -
                race_manager->getNumSpareTireKarts() == 1 &&
                race_manager->getTrackName() != "tutorial")
            {
                m_auxiliary_ticks += 6;
            }

            // Work around a bug that occurred on linux once:
            // the SFXManager::get() kept on reporting that it is playing,
            // while it was not - so STK would never reach the ready
            // ... phase. Since the sound effect is about 3 seconds
            // long, we use the aux timer to force the next phase
            // after 3.5 seconds.
            if (m_track_intro_sound->getStatus() == SFXBase::SFX_PLAYING &&
                m_auxiliary_ticks < stk_config->time2Ticks(3.5f)           )
                return;   // Do not increase time

            // Wait before ready phase if sounds are disabled
            if (!UserConfigParams::m_sfx &&
                m_auxiliary_ticks < stk_config->time2Ticks(3.0f))
                return;

            if (!m_play_track_intro_sound)
            {
                startEngines();
                if (m_auxiliary_ticks < stk_config->time2Ticks(3.0f))
                    return;   // Do not increase time
            }

            m_auxiliary_ticks = 0;

            // In a networked game the client needs to wait for a notification
            // from the server that all clients and the server are ready to 
            // start the game. The server will actually wait for all clients
            // to confirm that they have started the race before starting
            // itself. In a normal race, this phase is skipped and the race
            // starts immediately.
            if (NetworkConfig::get()->isNetworking())
            {
                m_phase = WAIT_FOR_SERVER_PHASE;
                // In networked races, inform the start game protocol that
                // the world has been setup
                auto lobby = LobbyProtocol::get<LobbyProtocol>();
                assert(lobby);
                lobby->finishedLoadingWorld();
            }
            else
            {
                if (m_play_ready_set_go_sounds)
                    m_prestart_sound->play();
                m_phase = READY_PHASE;
            }
            return;   // Don't increase time
        case WAIT_FOR_SERVER_PHASE:
        {
            // Wait for all players to finish loading world
            auto lobby = LobbyProtocol::get<LobbyProtocol>();
            assert(lobby);
            if (!lobby->allPlayersReady())
                return;
            // This stage is only reached in case of a networked game.
            // The server waits for a confirmation from
            // each client that they have started (to guarantee that the
            // server is running with a local time behind all clients).
            if (m_play_ready_set_go_sounds)
                m_prestart_sound->play();

            if (NetworkConfig::get()->isServer() &&
                m_server_is_ready.load() == false) return;

            m_phase = READY_PHASE;
            auto cl = LobbyProtocol::get<ClientLobby>();
            if (cl)
                cl->startingRaceNow();
            return;   // Don't increase time
        }
        case READY_PHASE:
            startEngines();
            // One second
            if (m_auxiliary_ticks > stk_config->getPhysicsFPS())
            {
                if (m_play_ready_set_go_sounds)
                {
                    m_prestart_sound->play();
                }

                m_phase = SET_PHASE;
            }

            m_auxiliary_ticks++;

            // In artist debug mode, when without opponents, skip the
            // ready/set/go counter faster
            if (UserConfigParams::m_artist_debug_mode     &&
                !NetworkConfig::get()->isNetworking()     &&
                race_manager->getNumberOfKarts() -
                race_manager->getNumSpareTireKarts() == 1 &&
                race_manager->getTrackName() != "tutorial")
            {
                m_auxiliary_ticks += 6;
            }

            return;   // Do not increase time
        case SET_PHASE:
            if (m_auxiliary_ticks > 2*stk_config->getPhysicsFPS())
            {
                // set phase is over, go to the next one
                m_phase = GO_PHASE;
                // Save one initial state on a client, in case that an event
                // is received from a client (trieggering a rollback) before
                // a state from the server has been received.
                if (NetworkConfig::get()->isNetworking() &&  
                    NetworkConfig::get()->isClient()        )
                {
                    RewindManager::get()->saveLocalState();
                    // FIXME TODO: save state in rewind queue!
                }
                if (m_play_ready_set_go_sounds)
                {
                    m_start_sound->play();
                }

                // event
                onGo();
            }

            m_auxiliary_ticks++;

            // In artist debug mode, when without opponents, 
            // skip the ready/set/go counter faster
            if (UserConfigParams::m_artist_debug_mode &&
                !NetworkConfig::get()->isNetworking() &&
                race_manager->getNumberOfKarts() -
                race_manager->getNumSpareTireKarts() == 1 &&
                race_manager->getTrackName() != "tutorial")
            {
                m_auxiliary_ticks += 6;
            }

            return;   // Do not increase time
        case GO_PHASE  :
            // 2.5 seconds
            if (m_auxiliary_ticks>stk_config->time2Ticks(2.5f) && 
                music_manager->getCurrentMusic() &&
                !music_manager->getCurrentMusic()->isPlaying())
            {
                music_manager->startMusic();
            }
            // how long to display the 'go' message
            if (m_auxiliary_ticks > 3 * stk_config->getPhysicsFPS())
            {
                m_phase = MUSIC_PHASE;
            }

            m_auxiliary_ticks++;

            // In artist debug mode, when without opponents,
            // skip the ready/set/go counter faster
            if (UserConfigParams::m_artist_debug_mode &&
                race_manager->getNumberOfKarts() -
                race_manager->getNumSpareTireKarts() == 1 &&
                race_manager->getTrackName() != "tutorial")
            {
                m_auxiliary_ticks += 6;
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
            if (m_auxiliary_ticks >  
                stk_config->time2Ticks(stk_config->m_music_credit_time) )
            {
                m_phase = RACE_PHASE;
            }

            m_auxiliary_ticks++;
            break;
        case RACE_PHASE:
            // Nothing to do for race phase, switch to delay finish phase
            // happens when
            break;
        case DELAY_FINISH_PHASE:
        {
            m_auxiliary_ticks++;

            // Change to next phase if delay is over
            if (m_auxiliary_ticks >
                stk_config->time2Ticks(stk_config->m_delay_finish_time))
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
        case IN_GAME_MENU_PHASE:
            // Nothing to do here.
            break;
        case GOAL_PHASE:
            // Nothing to do here as well.
            break;

        default: break;
    }

    IrrlichtDevice *device = irr_driver->getDevice();
    
    switch (m_clock_mode)
    {
        case CLOCK_CHRONO:
            if (!device->getTimer()->isStopped())
            {
                m_time_ticks++;
                m_time  = stk_config->ticks2Time(m_time_ticks);
                m_count_up_ticks++;
            }
            break;
        case CLOCK_COUNTDOWN:
            // stop countdown when race is over
            if (m_phase == RESULT_DISPLAY_PHASE || m_phase == FINISH_PHASE)
            {
                m_time_ticks = 0;
                m_time = 0.0f;
                m_count_up_ticks = 0;
                break;
            }

            if (!device->getTimer()->isStopped())
            {
                m_time_ticks--;
                m_time = stk_config->ticks2Time(m_time_ticks);
                m_count_up_ticks++;
            }

            if(m_time_ticks <= 0.0)
            {
                // event
                countdownReachedZero();
            }

            break;
        default: break;
    }   // switch m_phase
}   // update

//-----------------------------------------------------------------------------
/** If the server requests that a client's time should be adjusted,
 *  smoothly change the clock speed of this client to go a bit faster
 *  or slower till the overall adjustment was reached.
 *  \param dt Original time step size.
 */
float WorldStatus::adjustDT(float dt)
{
    // If request, adjust world time to go ahead (adjust>0) or
    // slow down (<0). This is done in 5% of dt steps so that the
    // user will not notice this.
    const float FRACTION = 0.10f;   // fraction of dt to be adjusted
    float time_adjust;
    if (m_adjust_time_by >= 0)   // make it run faster
    {
        time_adjust = dt * FRACTION;
        if (time_adjust > m_adjust_time_by) time_adjust = m_adjust_time_by;
        if (m_adjust_time_by > 0)
            Log::verbose("info", "At %f %f adjusting time by %f dt %f to dt %f for %f",
                World::getWorld()->getTime(), StkTime::getRealTime(),
                time_adjust, dt, dt - time_adjust, m_adjust_time_by);
    }
    else   // m_adjust_time negative, i.e. will go slower
    {
        time_adjust = -dt * FRACTION;
        if (time_adjust < m_adjust_time_by) time_adjust = m_adjust_time_by;
        Log::verbose("info", "At %f %f adjusting time by %f dt %f to dt %f for %f",
            World::getWorld()->getTime(), StkTime::getRealTime(),
            time_adjust, dt, dt - time_adjust, m_adjust_time_by);
    }
    m_adjust_time_by -= time_adjust;
    dt -= time_adjust;
    return dt;
}  // adjustDT

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
    m_server_is_ready.store(true);
}   // startReadySetGo

//-----------------------------------------------------------------------------
/** Sets the time for the clock.
 *  \param time New time to set.
 */
void WorldStatus::setTime(const float time)
{
    int new_time_ticks = stk_config->time2Ticks(time);
    m_count_up_ticks  += (new_time_ticks - m_time_ticks);
    m_time_ticks       = new_time_ticks;
    m_time             = stk_config->ticks2Time(new_time_ticks);
}   // setTime

//-----------------------------------------------------------------------------
/** Sets a new time for the world time, measured in ticks.
 *  \param ticks New time in ticks to set.
 */
void WorldStatus::setTicks(int ticks)
{
    m_count_up_ticks += ticks - m_time_ticks;
    m_time_ticks = ticks;
    m_time = stk_config->ticks2Time(ticks);
}   // setTicks

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

    if (!device->getTimer()->isStopped() &&
        !NetworkConfig::get()->isNetworking())
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

    if (device->getTimer()->isStopped() &&
        !NetworkConfig::get()->isNetworking())
        device->getTimer()->start();
}   // unpause
