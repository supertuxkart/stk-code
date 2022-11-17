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
#include "guiengine/engine.hpp"
#include "guiengine/modaldialog.hpp"
#include "karts/abstract_kart.hpp"
#include "modes/world.hpp"
#include "network/network_config.hpp"
#include "network/protocols/client_lobby.hpp"
#include "network/rewind_manager.hpp"
#include "network/race_event_manager.hpp"
#include "tracks/track.hpp"
#include "utils/stk_process.hpp"

#include <IrrlichtDevice.h>

//-----------------------------------------------------------------------------
WorldStatus::WorldStatus() : m_process_type(STKProcess::getType()), m_started_at(StkTime::getMonoTimeMs())
{
    if (m_process_type == PT_MAIN)
        main_loop->setFrameBeforeLoadingWorld();
    m_clock_mode        = CLOCK_CHRONO;
    m_phase             = SETUP_PHASE;

    m_prestart_sound    = SFXManager::get()->createSoundSource("pre_start_race");
    m_start_sound       = SFXManager::get()->createSoundSource("start_race");
    m_track_intro_sound = SFXManager::get()->createSoundSource("track_intro");
    m_time            = 0.0f;
    m_time_ticks      = 0;
    m_auxiliary_ticks = 0;
    m_count_up_ticks  = 0;

    m_play_track_intro_sound = UserConfigParams::m_music;
    m_play_ready_set_go_sounds = true;
    m_play_racestart_sounds = true;
    m_live_join_world = false;

    if (m_process_type == PT_MAIN)
    {
        IrrlichtDevice *device = irr_driver->getDevice();

        if (device->getTimer()->isStopped())
            device->getTimer()->start();
    }
}   // WorldStatus

//-----------------------------------------------------------------------------
/** Resets all status information, used when starting a new race.
 */
void WorldStatus::reset(bool restart)
{
    m_started_at      = StkTime::getMonoTimeMs();
    m_time            = 0.0f;
    m_time_ticks      = 0;
    m_auxiliary_ticks = 0;
    m_count_up_ticks  = 0;
    m_start_music_ticks = -1;
    // how long to display the 'music' message
    m_race_ticks =
        stk_config->time2Ticks(stk_config->m_music_credit_time);
    m_live_join_ticks = -1;
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

    if (m_process_type == PT_MAIN)
    {
        IrrlichtDevice *device = irr_driver->getDevice();

        if (device->getTimer()->isStopped())
            device->getTimer()->start();

        // Set the right music
        Track::getCurrentTrack()->startMusic();
    }
}   // reset

//-----------------------------------------------------------------------------
/** Destructor of WorldStatus.
 */
WorldStatus::~WorldStatus()
{
    m_prestart_sound->deleteSFX();
    m_start_sound->deleteSFX();
    m_track_intro_sound->deleteSFX();

    if (m_process_type == PT_MAIN)
    {
        IrrlichtDevice *device = irr_driver->getDevice();

        if (device->getTimer()->isStopped())
            device->getTimer()->start();
    }
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
        if (!World::getWorld()->getKart(i)->isEliminated())
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
    switch (m_phase.load())
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

            if (m_process_type == PT_MAIN && Weather::getInstance())
            {
                Weather::getInstance()->playSound();
            }

            return;   // Do not increase time
        case TRACK_INTRO_PHASE:
            m_auxiliary_ticks++;

            if (UserConfigParams::m_artist_debug_mode &&
                !NetworkConfig::get()->isNetworking() &&
                RaceManager::get()->getNumberOfKarts() -
                RaceManager::get()->getNumSpareTireKarts() == 1 &&
                RaceManager::get()->getTrackName() != "tutorial")
            {
                m_auxiliary_ticks += 6;
            }

            if (!m_play_track_intro_sound)
            {
                startEngines();
            }

            // Wait before ready phase
            if (m_auxiliary_ticks < stk_config->time2Ticks(3.0f))
                return;

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
                if (!m_live_join_world)
                {
                    auto lobby = LobbyProtocol::get<LobbyProtocol>();
                    assert(lobby);
                    lobby->finishedLoadingWorld();
                }
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
            if (m_live_join_world)
            {
                m_auxiliary_ticks++;
                // Add 3 seconds delay before telling server finish loading
                // world, so previous (if any) disconnected player has left
                // fully
                if (m_auxiliary_ticks == stk_config->time2Ticks(3.0f))
                {
                    auto cl = LobbyProtocol::get<ClientLobby>();
                    assert(cl);
                    cl->finishedLoadingWorld();
#ifndef MOBILE_STK
                    static bool helper_msg_shown = false;
                    if (!helper_msg_shown && cl->isSpectator())
                    {
                        helper_msg_shown = true;
                        cl->addSpectateHelperMessage();
                    }
#endif
                }
                return;
            }
            return;   // Don't increase time
        }
        case SERVER_READY_PHASE:
        {
            auto lobby = LobbyProtocol::get<LobbyProtocol>();
            if (lobby && lobby->isRacing())
            {
                if (m_play_ready_set_go_sounds)
                    m_prestart_sound->play();
                m_phase = READY_PHASE;
            }
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
                RaceManager::get()->getNumberOfKarts() -
                RaceManager::get()->getNumSpareTireKarts() == 1 &&
                RaceManager::get()->getTrackName() != "tutorial")
            {
                m_auxiliary_ticks += 6;
            }

            return;   // Do not increase time
        case SET_PHASE:
            if (m_auxiliary_ticks > 2*stk_config->getPhysicsFPS())
            {
                // set phase is over, go to the next one
                m_phase = GO_PHASE;
                if (m_play_ready_set_go_sounds)
                {
                    m_start_sound->play();
                }

                // event
                onGo();
                // In artist debug mode, when without opponents,
                // skip the ready/set/go counter faster
                m_start_music_ticks =
                    UserConfigParams::m_artist_debug_mode &&
                    !NetworkConfig::get()->isNetworking()     &&
                    RaceManager::get()->getNumberOfKarts() -
                    RaceManager::get()->getNumSpareTireKarts() == 1 &&
                    RaceManager::get()->getTrackName() != "tutorial" ?
                    stk_config->time2Ticks(0.2f) :
                    stk_config->time2Ticks(1.0f);
            }

            m_auxiliary_ticks++;

            // In artist debug mode, when without opponents, 
            // skip the ready/set/go counter faster
            if (UserConfigParams::m_artist_debug_mode &&
                !NetworkConfig::get()->isNetworking() &&
                RaceManager::get()->getNumberOfKarts() -
                RaceManager::get()->getNumSpareTireKarts() == 1 &&
                RaceManager::get()->getTrackName() != "tutorial")
            {
                m_auxiliary_ticks += 6;
            }

            return;   // Do not increase time
        case GO_PHASE:
        {
            if (m_start_music_ticks != -1 &&
                m_count_up_ticks >= m_start_music_ticks)
            {
                m_start_music_ticks = -1;
                if (music_manager->getCurrentMusic() &&
                    !music_manager->getCurrentMusic()->isPlaying())
                {
                    music_manager->startMusic();
                }
                // no graphics mode goes race phase now
                if (GUIEngine::isNoGraphics())
                {
                    m_race_ticks = -1;
                    m_phase = RACE_PHASE;
                }
                else
                    m_phase = MUSIC_PHASE;
            }
            break;   // Now the world time starts
        }
        case MUSIC_PHASE:
        {
            // Start the music here when starting fast
            if (UserConfigParams::m_race_now)
            {
                music_manager->startMusic();
                UserConfigParams::m_race_now = false;
            }
            if (m_race_ticks != -1 && m_count_up_ticks >= m_race_ticks)
            {
                m_race_ticks = -1;
                m_phase = RACE_PHASE;
            }
            break;
        }
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
        default: break;
    }

    IrrlichtDevice *device = irr_driver->getDevice();
    
    switch (m_clock_mode)
    {
        case CLOCK_CHRONO:
            if (m_process_type == PT_CHILD || !device->getTimer()->isStopped())
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
                // For rescue animation playing (if any) in result screen
                if (m_process_type == PT_CHILD || !device->getTimer()->isStopped())
                    m_count_up_ticks++;
                break;
            }

            if (m_process_type == PT_CHILD || !device->getTimer()->isStopped())
            {
                m_time_ticks--;
                m_time = stk_config->ticks2Time(m_time_ticks);
                m_count_up_ticks++;
            }

            if (m_time_ticks <= 0)
            {
                // event
                countdownReachedZero();
            }

            break;
        default: break;
    }   // switch m_phase

}   // update

//-----------------------------------------------------------------------------
/** Sets the time for the clock.
 *  \param time New time to set.
 */
void WorldStatus::setTime(const float time)
{
    int new_time_ticks = stk_config->time2Ticks(time);
    m_time_ticks       = new_time_ticks;
    m_time             = stk_config->ticks2Time(new_time_ticks);
}   // setTime

//-----------------------------------------------------------------------------
/** Sets a new time for the world time, measured in ticks.
 *  \param ticks New time in ticks to set.
 */
void WorldStatus::setTicks(int ticks)
{
    m_time_ticks = ticks;
    m_time = stk_config->ticks2Time(ticks);
}   // setTicks

//-----------------------------------------------------------------------------
/** Sets a new time for the world time (used by rewind), measured in ticks.
 *  \param ticks New time in ticks to set (always count upwards).
 */
void WorldStatus::setTicksForRewind(int ticks)
{
    m_count_up_ticks = ticks;
    if (RaceManager::get()->hasTimeTarget())
    {
        m_time_ticks = stk_config->time2Ticks(RaceManager::get()->getTimeTarget()) -
            m_count_up_ticks;
    }
    else
        m_time_ticks = ticks;
    m_time = stk_config->ticks2Time(m_time_ticks);
}   // setTicksForRewind

//-----------------------------------------------------------------------------
/** Pauses the game and switches to the specified phase.
 *  \param phase Phase to switch to.
 */
void WorldStatus::pause(Phase phase)
{
    assert(m_previous_phase == UNDEFINED_PHASE);

    m_previous_phase = m_phase;
    m_phase          = phase;

    if (m_process_type != PT_MAIN)
        return;

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

    if (m_process_type != PT_MAIN)
        return;

    IrrlichtDevice *device = irr_driver->getDevice();

    if (device->getTimer()->isStopped() &&
        !NetworkConfig::get()->isNetworking())
        device->getTimer()->start();
}   // unpause

//-----------------------------------------------------------------------------
/** Base on the network timer set current world count up ticks to tick_now.
 */
void WorldStatus::endLiveJoinWorld(int ticks_now)
{
    m_live_join_ticks = ticks_now;
    m_live_join_world = false;
    m_auxiliary_ticks = 0;
    m_phase = MUSIC_PHASE;
    m_race_ticks = m_live_join_ticks + stk_config->time2Ticks(
        stk_config->m_music_credit_time);
    onGo();
    startEngines();
    music_manager->startMusic();
    setTicksForRewind(m_live_join_ticks);
}   // endLiveJoinWorld
