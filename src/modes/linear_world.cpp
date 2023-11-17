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

#include "modes/linear_world.hpp"

#include "achievements/achievements_manager.hpp"
#include "config/player_manager.hpp"
#include "audio/music_manager.hpp"
#include "audio/sfx_base.hpp"
#include "audio/sfx_manager.hpp"
#include "config/user_config.hpp"
#include "karts/abstract_kart.hpp"
#include "karts/cannon_animation.hpp"
#include "karts/controller/controller.hpp"
#include "karts/ghost_kart.hpp"
#include "karts/kart_properties.hpp"
#include "graphics/material.hpp"
#include "guiengine/modaldialog.hpp"
#include "physics/physics.hpp"
#include "network/network_config.hpp"
#include "network/network_player_profile.hpp"
#include "network/network_string.hpp"
#include "network/protocols/game_events_protocol.hpp"
#include "network/protocols/server_lobby.hpp"
#include "network/server_config.hpp"
#include "network/stk_host.hpp"
#include "network/stk_peer.hpp"
#include "race/history.hpp"
#include "states_screens/race_gui_base.hpp"
#include "tracks/check_manager.hpp"
#include "tracks/check_structure.hpp"
#include "tracks/drive_graph.hpp"
#include "tracks/drive_node.hpp"
#include "tracks/track_sector.hpp"
#include "tracks/track.hpp"
#include "utils/constants.hpp"
#include "utils/string_utils.hpp"
#include "utils/translation.hpp"

#include <climits>
#include <iostream>

//-----------------------------------------------------------------------------
/** Constructs the linear world. Note that here no functions can be called
 *  that use World::getWorld(), since it is not yet defined.
 */
LinearWorld::LinearWorld() : WorldWithRank()
{
    m_last_lap_sfx         = SFXManager::get()->createSoundSource("last_lap_fanfare");
    m_last_lap_sfx_played  = false;
    m_last_lap_sfx_playing = false;
    m_fastest_lap_ticks    = INT_MAX;
    m_valid_reference_time = false;
    m_live_time_difference = 0.0f;
    m_fastest_lap_kart_name = "";
    m_check_structure_compatible = false;
}   // LinearWorld

// ----------------------------------------------------------------------------
/** Actually initialises the world, i.e. creates all data structures to
 *  for all karts etc. In init functions can be called that use
 *  World::getWorld().
 */
void LinearWorld::init()
{
    WorldWithRank::init();

    assert(!Track::getCurrentTrack()->isArena());
    assert(!Track::getCurrentTrack()->isSoccer());

    m_last_lap_sfx_played           = false;
    m_last_lap_sfx_playing          = false;

    m_fastest_lap_kart_name         = "";

    // The values are initialised in reset()
    m_kart_info.resize(m_karts.size());
}   // init

//-----------------------------------------------------------------------------
/** The destructor frees al data structures.
 */
LinearWorld::~LinearWorld()
{
    m_last_lap_sfx->deleteSFX();
}   // ~LinearWorld

//-----------------------------------------------------------------------------
/** Called before a race is started (or restarted). It resets the data
 *  structures that keep track of position and distance long track of
 *  all karts.
 */
void LinearWorld::reset(bool restart)
{
    WorldWithRank::reset(restart);
    m_finish_timeout = std::numeric_limits<float>::max();
    m_last_lap_sfx_played  = false;
    m_last_lap_sfx_playing = false;
    m_fastest_lap_ticks    = INT_MAX;

    const unsigned int kart_amount = (unsigned int) m_karts.size();
    for(unsigned int i=0; i<kart_amount; i++)
    {
        m_kart_info[i].reset();
    }   // next kart

    // At the moment the last kart would be the one that is furthest away
    // from the start line, i.e. it would determine the amount by which
    // the track length must be extended (to avoid negative numbers in
    // estimateFinishTimeForKart()). On the other hand future game modes
    // might not follow this rule, so check the distance for all karts here:
    m_distance_increase = Track::getCurrentTrack()->getTrackLength();
    for(unsigned int i=0; i<kart_amount; i++)
    {
        m_distance_increase = std::min(m_distance_increase,
                                       getDistanceDownTrackForKart(i, false));
    }
    // Track length - minimum distance is how much the track length must
    // be increased to avoid negative values in estimateFinishTimeForKart
    // Increase this value somewhat in case that a kart drivess/slides
    // backwards a little bit at start.
    m_distance_increase = Track::getCurrentTrack()->getTrackLength() 
                        - m_distance_increase + 5.0f;

    if(m_distance_increase<0) m_distance_increase = 1.0f;  // shouldn't happen

    // First all kart infos must be updated before the kart position can be
    // recomputed, since otherwise 'new' (initialised) valued will be compared
    // with old values.
    updateRacePosition();

#ifdef DEBUG
    //FIXME: this could be defined somewhere in a central header so it can
    //       be used everywhere
#define assertExpr( ARG1, OP, ARG2 ) if (!(ARG1 OP ARG2)) \
        { \
            std::cerr << "Failed assert " << #ARG1 << #OP << #ARG2 << " @ " \
                      << __FILE__ << ":" << __LINE__ \
                      << "; values are (" << ARG1 << #OP << ARG2 << ")\n"; \
            assert(false); \
        }

    for (unsigned int i=0; i<kart_amount; i++)
    {
        for (unsigned int j=i+1; j<kart_amount; j++)
        {
            assertExpr( m_karts[i]->getPosition(), !=,
                        m_karts[j]->getPosition() );
        }
    }
#endif

}   // reset

//-----------------------------------------------------------------------------
/** General update function called once per frame. This updates the kart
 *  sectors, which are then used to determine the kart positions.
 *  \param ticks Number of physics time steps - should be 1.
 */
void LinearWorld::update(int ticks)
{
    auto sl = LobbyProtocol::get<ServerLobby>();
    if (sl && getPhase() == RACE_PHASE)
    {
        bool all_players_finished = true;
        bool has_ai = false;
        for (unsigned i = 0; i < RaceManager::get()->getNumPlayers(); i++)
        {
            auto npp =
                RaceManager::get()->getKartInfo(i).getNetworkPlayerProfile().lock();
            if (!npp)
                continue;
            if (npp)
            {
                auto peer = npp->getPeer();
                if ((peer && peer->isAIPeer()) || sl->isAIProfile(npp))
                    has_ai = true;
                else if (!getKart(i)->hasFinishedRace())
                    all_players_finished = false;
            }
        }
        if (all_players_finished && has_ai)
            m_finish_timeout = -1.0f;
    }
    if (getPhase() == RACE_PHASE &&
        m_finish_timeout != std::numeric_limits<float>::max())
    {
        m_finish_timeout -= stk_config->ticks2Time(ticks);
        if (m_finish_timeout < 0.0f)
        {
            endRaceEarly();
            m_finish_timeout = std::numeric_limits<float>::max();
        }
    }

    // Do stuff specific to this subtype of race.
    // ------------------------------------------
    updateTrackSectors();
    // Run generic parent stuff that applies to all modes.
    // It especially updates the kart positions.
    // It MUST be done after the update of the distances
    WorldWithRank::update(ticks);

    // Update all positions. This must be done after _all_ karts have
    // updated their position and laps etc, otherwise inconsistencies
    // (like two karts at same position) can occur.
    // ---------------------------------------------------------------
    WorldWithRank::updateTrack(ticks);
    updateRacePosition();

    const unsigned int kart_amount = getNumKarts();
    for (unsigned int i=0; i<kart_amount; i++)
    {
        // ---------- update rank ------
        if (m_karts[i]->hasFinishedRace() ||
            m_karts[i]->isEliminated()       ) continue;

        // Update the estimated finish time.
        // This is used by the AI
        m_kart_info[i].m_estimated_finish =
                estimateFinishTimeForKart(m_karts[i].get());
    }
    // If one player and a ghost, or two compared ghosts,
    // compute the live time difference
    if(RaceManager::get()->hasGhostKarts() && RaceManager::get()->getNumberOfKarts() == 2)
        updateLiveDifference();

#ifdef DEBUG
    // Debug output in case that the double position error occurs again.
    std::vector<int> pos_used;
    pos_used.resize(kart_amount+1, -99);
    for(unsigned int i=0; i<kart_amount; i++)
    {
        if(pos_used[m_karts[i]->getPosition()]!=-99)
        {
            for(unsigned int j=0; j<kart_amount; j++)
            {
                Log::verbose("[LinearWorld]", "kart id=%u, position=%d, finished=%d, laps=%d, "
                       "distanceDownTrack=%f overallDistance=%f %s",
                    j, m_karts[j]->getPosition(),
                    m_karts[j]->hasFinishedRace(),
                    m_kart_info[j].m_finished_laps,
                    getDistanceDownTrackForKart(m_karts[j]->getWorldKartId(), true),
                    m_kart_info[j].m_overall_distance,
                    (m_karts[j]->getPosition() == m_karts[i]->getPosition()
                     ? "<--- !!!" : "")                                      );
            }
        }
        pos_used[m_karts[i]->getPosition()]=i;
    }
#endif
}   // update

//-----------------------------------------------------------------------------
void LinearWorld::updateTrackSectors()
{
    const unsigned int kart_amount = getNumKarts();
    for(unsigned int n=0; n<kart_amount; n++)
    {
        KartInfo& kart_info = m_kart_info[n];
        AbstractKart* kart = m_karts[n].get();

        // Nothing to do for karts that are currently being
        // rescued or eliminated
        if(kart->getKartAnimation() &&
           !dynamic_cast<CannonAnimation*>(kart->getKartAnimation()))
            continue;
        // If the kart is off road, and 'flying' over a reset plane
        // don't adjust the distance of the kart, to avoid a jump
        // in the position of the kart (e.g. while falling the kart
        // might get too close to another part of the track, shortly
        // jump to position one, then on reset fall back to last)
        if ((!getTrackSector(n)->isOnRoad() &&
            (!kart->getMaterial() ||
              kart->getMaterial()->isDriveReset()))  &&
             !kart->isGhostKart())
            continue;
        getTrackSector(n)->update(kart->getFrontXYZ());
        kart_info.m_overall_distance = kart_info.m_finished_laps
                                     * Track::getCurrentTrack()->getTrackLength()
                        + getDistanceDownTrackForKart(kart->getWorldKartId(), true);
    }   // for n
}   // updateTrackSectors

//-----------------------------------------------------------------------------
/** This updates all only graphical elements.It is only called once per
*  rendered frame, not once per time step.
*  float dt Time since last rame.
*/
void LinearWorld::updateGraphics(float dt)
{
    WorldWithRank::updateGraphics(dt);
    if (m_last_lap_sfx_playing &&
        m_last_lap_sfx->getStatus() != SFXBase::SFX_PLAYING)
    {
        music_manager->resetTemporaryVolume();
        m_last_lap_sfx_playing = false;
    }

    const GUIEngine::GameState gamestate = StateManager::get()->getGameState();
    
    if (gamestate == GUIEngine::GAME && 
        !GUIEngine::ModalDialog::isADialogActive())
    {
        const unsigned int kart_amount = getNumKarts();
        for (unsigned int i = 0; i<kart_amount; i++)
        {
            // ---------- update rank ------
            if (!m_karts[i]->hasFinishedRace() &&
                !m_karts[i]->isEliminated())
            {
                checkForWrongDirection(i, dt);
            }
        }   // for i <kart_amount
    }

}   // updateGraphics

// ----------------------------------------------------------------------------
/** This calculate the time difference between the second kart in the
 *  race and the first kart in the race (who must be a ghost)
 */
void LinearWorld::updateLiveDifference()
{
    // First check that the call requirements are verified
    assert (RaceManager::get()->hasGhostKarts() && RaceManager::get()->getNumberOfKarts() >= 2);

    AbstractKart* ghost_kart = getKart(0);

    // Get the distance at which the second kart is
    float second_kart_distance = getOverallDistance(1);

    // Check when the ghost what at this position
    float ghost_time;

    // If there are two ghost karts, the view is set to kart 0,
    // so switch roles in the comparison. Note that
    // we can't simply multiply the time by -1, as they are assymetrical.
    // When one kart don't increase its distance (rescue, etc),
    // the difference increases linearly for one and jump for the other.
    if (getKart(1)->isGhostKart())
    {
        ghost_kart = getKart(1);
        second_kart_distance = getOverallDistance(0);
    }
    ghost_time = ghost_kart->getTimeForDistance(second_kart_distance);

    if (ghost_time >= 0.0f)
        m_valid_reference_time = true;
    else
        m_valid_reference_time = false;

    float current_time = World::getWorld()->getTime();

    m_live_time_difference = current_time - ghost_time;
}

//-----------------------------------------------------------------------------
/** Is called by check structures if a kart starts a new lap.
 *  \param kart_index Index of the kart.
 */
void LinearWorld::newLap(unsigned int kart_index)
{
    KartInfo &kart_info = m_kart_info[kart_index];
    AbstractKart *kart  = m_karts[kart_index].get();

    // Reset reset-after-lap achievements
    PlayerProfile *p = PlayerManager::getCurrentPlayer();
    if (kart->getController()->canGetAchievements())
    {
        p->getAchievementsStatus()->onLapEnd();
    }

    // Only update the kart controller if a kart that has already finished
    // the race crosses the start line again. This avoids 'fastest lap'
    // messages if the end controller does a fastest lap, but especially
    // allows the end controller to switch end cameras
    if(kart->hasFinishedRace())
    {
        kart->getController()->newLap(kart_info.m_finished_laps);
        return;
    }

    const int lap_count = RaceManager::get()->getNumLaps();

    // Only increase the lap counter and set the new time if the
    // kart hasn't already finished the race (otherwise the race_gui
    // will begin another countdown).
    if(kart_info.m_finished_laps+1 <= lap_count)
    {
        assert(kart->getWorldKartId()==kart_index);
        kart_info.m_ticks_at_last_lap=getTimeTicks();
        kart_info.m_finished_laps++;
        m_kart_info[kart_index].m_overall_distance =
              m_kart_info[kart_index].m_finished_laps 
            * Track::getCurrentTrack()->getTrackLength()
            + getDistanceDownTrackForKart(kart->getWorldKartId(), true);
    }
    // Last lap message (kart_index's assert in previous block already)
    if (raceHasLaps() && kart_info.m_finished_laps+1 == lap_count)
    {
        if (lap_count > 1 && !isLiveJoinWorld() && m_race_gui)
        {
            m_race_gui->addMessage(_("Final lap!"), kart,
                               3.0f, GUIEngine::getSkin()->getColor("font::normal"), true,
                               true /* big font */, true /* outline */);
        }
        if(!m_last_lap_sfx_played && lap_count > 1)
        {
            if (UserConfigParams::m_sfx)
            {
                m_last_lap_sfx->play();
                m_last_lap_sfx_played = true;
                m_last_lap_sfx_playing = true;

                // Temporarily reduce the volume of the main music
                // So that the last lap SFX can be heard
                if(UserConfigParams::m_music &&
                    music_manager->getCurrentMusic())
                {
                    // The parameter taken by SetTemporaryVolume is a factor
                    // that gets multiplied with the master volume
                    music_manager->setTemporaryVolume(0.5f);
                }
            }
            else
            {
                m_last_lap_sfx_played = true;
                m_last_lap_sfx_playing = false;
            }
        }
    }
    else if (raceHasLaps() && kart_info.m_finished_laps > 0 &&
             kart_info.m_finished_laps+1 < lap_count && !isLiveJoinWorld() && m_race_gui)
    {
        m_race_gui->addMessage(_("Lap %i", kart_info.m_finished_laps+1), kart,
                               2.0f, GUIEngine::getSkin()->getColor("font::normal"), true,
                               true /* big font */, true /* outline */);
    }

    // The race positions must be updated here: consider the situation where
    // the first kart does not cross the finish line in its last lap, instead
    // it passes it, the kart reverses and crosses the finishing line
    // backwards. Just before crossing the finishing line the kart will be on
    // the last lap, but with a distance along the track close to zero.
    // Therefore its position will be wrong. If the race position gets updated
    // after increasing the number of laps (but before tagging the kart to have
    // finished the race) the position will be correct (since the kart now
    // has one additional lap it will be ahead of the other karts).
    // Without this call the incorrect position for this kart would remain
    // (since a kart that has finished the race does not get its position
    // changed anymore), potentially resulting in a duplicated race position
    // (since the first kart does not have position 1, no other kart can get
    // position 1, so one rank will be duplicated).
    // Similarly the situation can happen if the distance along track should
    // go back to zero before actually crossing the finishing line. While this
    // should not happen, it could potentially be caused by floating point
    // errors. In this case the call to updateRacePosition will avoid
    // duplicated race positions as well.
    updateRacePosition();

    // Race finished
    // We compute the exact moment the kart crossed the line
    // This way, even with poor framerate, we get a time significant to the ms
    if(kart_info.m_finished_laps >= RaceManager::get()->getNumLaps() && raceHasLaps())
    {
        if (kart->isGhostKart())
        {
            GhostKart* gk = dynamic_cast<GhostKart*>(kart);
            // Old replays don't store distance, so don't use the ghost method
            // Ghosts also don't store the previous positions, so the method
            // for normal karts can't be used.
            if (gk->getGhostFinishTime() > 0.0f)
                kart->finishedRace(gk->getGhostFinishTime());
            else
                kart->finishedRace(getTime());
        }
        else
        {
            float curr_distance_after_line = getDistanceDownTrackForKart(kart->getWorldKartId(),false);

            TrackSector prev_sector;
            prev_sector.update(kart->getRecentPreviousXYZ());
            float prev_distance_before_line = Track::getCurrentTrack()->getTrackLength()
                                              - prev_sector.getDistanceFromStart(false);

            float finish_proportion = 0.0f;
            // Workaround against some bugs caused by the "distance is zero on a band" issue, see #4109
            if (curr_distance_after_line + prev_distance_before_line != 0.0f)
                finish_proportion =   curr_distance_after_line
                                   / (prev_distance_before_line + curr_distance_after_line);
        
            float prev_time = kart->getRecentPreviousXYZTime();
            float finish_time = prev_time*finish_proportion + getTime()*(1.0f-finish_proportion);

            if (NetworkConfig::get()->isServer() &&
                ServerConfig::m_auto_end &&
                m_finish_timeout == std::numeric_limits<float>::max())
            {
                m_finish_timeout = finish_time * 0.25f + 15.0f;
            }
            kart->finishedRace(finish_time);
        }
    }
    int ticks_per_lap;
    if (kart_info.m_finished_laps == 1) // just completed first lap
    {
        // To avoid negative times in countdown mode
        if (getClockMode() == CLOCK_COUNTDOWN)
            ticks_per_lap = stk_config->time2Ticks(RaceManager::get()->getTimeTarget()) - getTimeTicks();
        else
            ticks_per_lap = getTimeTicks();
    }
    else //completing subsequent laps
    {
        // To avoid negative times in countdown mode
        if (getClockMode() == CLOCK_COUNTDOWN)
            ticks_per_lap = kart_info.m_lap_start_ticks - getTimeTicks();
        else
            ticks_per_lap = getTimeTicks() - kart_info.m_lap_start_ticks;
    }

    // if new fastest lap
    if(ticks_per_lap < m_fastest_lap_ticks && raceHasLaps() &&
        kart_info.m_finished_laps>0 && !isLiveJoinWorld())
    {
        m_fastest_lap_ticks = ticks_per_lap;

        std::string s = StringUtils::ticksTimeToString(ticks_per_lap);

        // Store the temporary string because clang would mess this up
        // (remove the stringw before the wchar_t* is used).
        const core::stringw &kart_name = kart->getController()->getName();
        m_fastest_lap_kart_name = kart_name;

        //I18N: as in "fastest lap: 60 seconds by Wilber"
        irr::core::stringw m_fastest_lap_message =
            _C("fastest_lap", "%s by %s", s.c_str(), kart_name);

        if (m_race_gui)
        {
            m_race_gui->addMessage(m_fastest_lap_message, NULL, 4.0f,
                video::SColor(255, 255, 255, 255), false);
            m_race_gui->addMessage(_("New fastest lap"), NULL, 4.0f,
                video::SColor(255, 255, 255, 255), false);
        }
    } // end if new fastest lap

    kart_info.m_lap_start_ticks = getTimeTicks();
    kart->getController()->newLap(kart_info.m_finished_laps);
}   // newLap

//-----------------------------------------------------------------------------
/** Returns the distance the kart has travelled along the track since
 *  crossing the start line..
 *  \param kart_id Index of the kart.
 */
float LinearWorld::getDistanceDownTrackForKart(const int kart_id, bool account_for_checklines) const
{
    return getTrackSector(kart_id)->getDistanceFromStart(account_for_checklines);
}   // getDistanceDownTrackForKart

//-----------------------------------------------------------------------------
/** Gets the distance of the kart from the center of the driveline. Positive
 *  is to the right of the center, negative values to the left.
 *  \param kart_id Index of kart.
 */
float LinearWorld::getDistanceToCenterForKart(const int kart_id) const
{
    return getTrackSector(kart_id)->getDistanceToCenter();
}   // getDistanceToCenterForKart

//-----------------------------------------------------------------------------
int LinearWorld::getLapForKart(const int kart_id) const
{
    assert(kart_id < (int)m_kart_info.size());
    return  m_kart_info[kart_id].m_finished_laps;
}   // getLapForKart

//-----------------------------------------------------------------------------
/** Returns the estimated finishing time.
 *  \param kart_id Id of the kart.
 */
float LinearWorld::getEstimatedFinishTime(const int kart_id) const
{
    assert(kart_id < (int)m_kart_info.size());
    return m_kart_info[kart_id].m_estimated_finish;
}   // getEstimatedFinishTime

//-----------------------------------------------------------------------------
int LinearWorld::getTicksAtLapForKart(const int kart_id) const
{
    assert(kart_id < (int)m_kart_info.size());
    return m_kart_info[kart_id].m_ticks_at_last_lap;
}   // getTicksAtLapForKart

//-----------------------------------------------------------------------------
void LinearWorld::getKartsDisplayInfo(
                           std::vector<RaceGUIBase::KartIconDisplayInfo> *info)
{
    int laps_of_leader  = -1;
    int ticks_of_leader = INT_MAX;
    // Find the best time for the lap. We can't simply use
    // the time of the kart at position 1, since the kart
    // might have been overtaken by now
    const unsigned int kart_amount = getNumKarts();
    for(unsigned int i = 0; i < kart_amount ; i++)
    {
        RaceGUIBase::KartIconDisplayInfo& rank_info = (*info)[i];
        AbstractKart* kart = m_karts[i].get();

        // reset color
        rank_info.m_color = video::SColor(255, 255, 255, 255);
        rank_info.lap = -1;

        if(kart->isEliminated()) continue;
        const int lap_ticks = getTicksAtLapForKart(kart->getWorldKartId());
        const int current_lap  = getLapForKart( kart->getWorldKartId() );
        rank_info.lap = current_lap;

        if(current_lap > laps_of_leader)
        {
            // more laps than current leader --> new leader and
            // new time computation
            laps_of_leader = current_lap;
            ticks_of_leader = lap_ticks;
        } else if(current_lap == laps_of_leader)
        {
            // Same number of laps as leader: use fastest time
            ticks_of_leader=std::min(ticks_of_leader,lap_ticks);
        }
    }

    // we now know the best time of the lap. fill the remaining bits of info
    for(unsigned int i = 0; i < kart_amount ; i++)
    {
        RaceGUIBase::KartIconDisplayInfo& rank_info = (*info)[i];
        KartInfo& kart_info = m_kart_info[i];
        AbstractKart* kart = m_karts[i].get();

        const int position = kart->getPosition();

        // Don't compare times when crossing the start line first
        if(laps_of_leader>0                                                &&
           (getTimeTicks() - getTicksAtLapForKart(kart->getWorldKartId())  <
            stk_config->time2Ticks(8)                                      ||
            rank_info.lap != laps_of_leader)                               &&
            raceHasLaps())
        {  // Display for 5 seconds
            std::string str;
            if(position == 1)
            {
                str = " " + StringUtils::ticksTimeToString(
                                 getTicksAtLapForKart(kart->getWorldKartId()) );
            }
            else
            {
                int ticks_behind;
                ticks_behind = (kart_info.m_finished_laps==laps_of_leader
                                ? getTicksAtLapForKart(kart->getWorldKartId())
                                : getTimeTicks())
                           - ticks_of_leader;
                str = "+" + StringUtils::ticksTimeToString(ticks_behind);
            }
            rank_info.m_text = irr::core::stringw(str.c_str());
        }
        else if (kart->hasFinishedRace())
        {
            rank_info.m_text = kart->getController()->getName();
            if (RaceManager::get()->getKartGlobalPlayerId(i) > -1)
            {
                const core::stringw& flag = StringUtils::getCountryFlag(
                    RaceManager::get()->getKartInfo(i).getCountryCode());
                if (!flag.empty())
                {
                    rank_info.m_text += L" ";
                    rank_info.m_text += flag;
                }
            }
        }
        else
        {
            rank_info.m_text = "";
        }

        int numLaps = RaceManager::get()->getNumLaps();

        if(kart_info.m_finished_laps>=numLaps)
        {  // kart is finished, display in green
            rank_info.m_color.setGreen(0);
            rank_info.m_color.setBlue(0);
        }
        else if(kart_info.m_finished_laps>=0 && numLaps>1)
        {
            int col = (int)(255*(1.0f-(float)kart_info.m_finished_laps
                                    /((float)numLaps-1.0f)        ));
            rank_info.m_color.setBlue(col);
            rank_info.m_color.setGreen(col);
        }
    }   // next kart


}   // getKartsDisplayInfo

//-----------------------------------------------------------------------------
/** Estimate the arrival time of any karts that haven't arrived yet by using
 *  their average speed up to now and the distance still to race. This
 *  approach guarantees that the order of the karts won't change anymore
 *  (karts ahead will have covered more distance and have therefore a higher
 *  average speed and therefore finish the race earlier than karts further
 *  behind), so the position doesn't have to be updated to get the correct
 *  scoring.
 *  As so often the devil is in the details: a kart that hasn't crossed the
 *  starting line has a negative distance (when it is crossing the start line
 *  its distance becomes 0), which can result in a negative average speed
 *  (and therefore incorrect estimates). This is partly taken care of by
 *  adding m_distance_increase to the distance covered by a kart. The value
 *  of m_distance_increase is a bit more than the distance the last kart
 *  has from the start line at start time. This guarantees that the distance
 *  used in computing the average speed is positive in most cases. Only
 *  exception is if a kart is driving backwards on purpose. While this
 *  shouldn't happen (the AI doesn't do it, and if it's a player the game
 *  won't finish so the time estimation won't be called and so the incorrect
 *  result won't be displayed), this is still taken care of: if the average
 *  speed is negative, the estimated arrival time of the kart is set to
 *  99:00 plus kart position. This means that even in this case all karts
 *  will have a different arrival time.
 *  \pre The position of the karts are set according to the distance they
 *       have covered.
 *  \param kart The kart for which to estimate the finishing times.
 */
float LinearWorld::estimateFinishTimeForKart(AbstractKart* kart)
{
    const KartInfo &kart_info = m_kart_info[kart->getWorldKartId()];

    float full_distance = RaceManager::get()->getNumLaps()
                        * Track::getCurrentTrack()->getTrackLength();

    // For ghost karts, use the replay data rather than estimating
    if (kart->isGhostKart())
    {
        GhostKart* gk = dynamic_cast<GhostKart*>(kart);
        // Old replays don't store distance, so don't use the ghost method
        // They'll return a negative time here
        if (gk->getGhostFinishTime() > 0.0f)
            return gk->getGhostFinishTime();
    }

    if(full_distance == 0)
        full_distance = 1.0f;   // For 0 lap races avoid warning below

#ifdef DEBUG
    if(kart_info.m_overall_distance > full_distance)
    {
        Log::debug("[LinearWorld]", "Full distance < distance covered for kart '%s':",
               kart->getIdent().c_str());
        Log::debug("[LinearWorld]", "%f < %f", full_distance, kart_info.m_overall_distance);
    }
#endif
    // Avoid potential problems (floating point issues, coding bug?) if a
    // kart has driven more than the full distance, but not finished:
    // Return the current time plus initial position to spread arrival
    // times a bit. This code should generally not be used at all, it's
    // just here to avoid invalid finishing times.
    if(kart_info.m_overall_distance > full_distance)
        return getTime() + kart->getInitialPosition();

    // Finish time is the time needed for the whole race with
    // the computed average speed computed. The distance is always positive
    // due to the way m_distance_increase was computed, so average speed
    // is also always positive.
    float average_speed = getTime()==0
                        ? 1.0f
                        : (m_distance_increase + kart_info.m_overall_distance)
                          / getTime();

    // Avoid NAN or invalid results when average_speed is very low
    // or negative (which can happen if a kart drives backwards and
    // m_overall distance becomes smaller than -m_distance_increase).
    // In this case set the time to 59 minutes, offset by kart
    // position (to spread arrival times for all karts that arrive
    // even later). This works for up to 60 karts (otherwise the
    // time displayed would overflow to 00:yy).
    if(average_speed<0.01f)
        return 59*60.0f + kart->getPosition();

    float est_time = getTime() + (full_distance - kart_info.m_overall_distance)
                                 / average_speed;

    // Avoid times > 59:00 - in this case use kart position to spread
    // arrival time so that each kart has a unique value. The pre-condition
    // guarantees that this works correctly (higher position -> less distance
    // covered -> later arrival time).
    if(est_time>59*60.0f)
        return 59*60.0f + kart->getPosition();

    return est_time;
}   // estimateFinishTimeForKart

// ------------------------------------------------------------------------
/** Returns the number of rescue positions on a given track, which in
 *  linear races is just the number of driveline quads.
  */
unsigned int LinearWorld::getNumberOfRescuePositions() const
{
    return DriveGraph::get()->getNumNodes();
}   // getNumberOfRescuePositions

// ------------------------------------------------------------------------
unsigned int LinearWorld::getRescuePositionIndex(AbstractKart *kart)
{
    const unsigned int kart_id = kart->getWorldKartId();

    getTrackSector(kart_id)->rescue();
    // Setting XYZ for the kart is important since otherwise the kart
    // will not detect the right material again when doing the next
    // raycast to detect where it is driving on (--> potential rescue loop)
    int index = getTrackSector(kart_id)->getCurrentGraphNode();

    // Do not rescue to an ignored quad, find another (non-ignored) quad
    if (Graph::get()->getQuad(index)->isIgnored())
    {
        Vec3 pos = kart->getFrontXYZ();
        int sector = Graph::get()->findOutOfRoadSector(pos);
        return sector;
    }

    return index;
}   // getRescuePositionIndex

// ------------------------------------------------------------------------
btTransform LinearWorld::getRescueTransform(unsigned int index) const
{
    const Vec3 &xyz = DriveGraph::get()->getNode(index)->getCenter();
    const Vec3 &normal = DriveGraph::get()->getNode(index)->getNormal();
    btTransform pos;
    pos.setOrigin(xyz);

    // First rotate into the quad's plane (q1), then rotate so that the kart points in the
    // right direction (q2).
    btQuaternion q1 = shortestArcQuat(Vec3(0, 1, 0), normal);
    // First apply the heading change, than the 'parallelisation' to the plane
    btQuaternion q2(btVector3(0,1,0), Track::getCurrentTrack()->getAngle(index));
    pos.setRotation(q1*q2);
    return pos;
}   // getRescueTransform

//-----------------------------------------------------------------------------
/** Find the position (rank) of every kart. ATM it uses a stable O(n^2)
 *  algorithm by counting for each kart how many other karts are ahead of
 *  it.
 */
void LinearWorld::updateRacePosition()
{
    // Mostly for debugging:
    beginSetKartPositions();
    const unsigned int kart_amount = (unsigned int) m_karts.size();

#ifdef DEBUG
    bool rank_changed = false;
#endif

    // NOTE: if you do any changes to this loop, the next loop (see
    // DEBUG_KART_RANK below) needs to have the same changes applied
    // so that debug output is still correct!!!!!!!!!!!
    for (unsigned int i=0; i<kart_amount; i++)
    {
        AbstractKart* kart = m_karts[i].get();
        // Karts that are either eliminated or have finished the
        // race already have their (final) position assigned. If
        // these karts would get their rank updated, it could happen
        // that a kart that finished first will be overtaken after
        // crossing the finishing line and become second!
        if(kart->isEliminated() || kart->hasFinishedRace())
        {
            // This is only necessary to support debugging inconsistencies
            // in kart position parameters.
            setKartPosition(i, kart->getPosition());
            continue;
        }
        KartInfo& kart_info = m_kart_info[i];

        int p = 1 ;

        const unsigned int my_id = kart->getWorldKartId();
        const float my_distance  = m_kart_info[my_id].m_overall_distance;

        // Count karts ahead of the current kart, i.e. kart that are
        // already finished or have covered a larger overall distance.
        for (unsigned int j = 0 ; j < kart_amount ; j++)
        {
            // don't compare a kart with itself and ignore eliminated karts
            if(j == my_id || m_karts[j]->isEliminated())
                continue;

            // If the other kart has:
            // - finished the race (but this kart hasn't)
            // - or is ahead
            // - or has the same distance (very unlikely) but started earlier
            // it is ahead --> increase position
            if((!kart->hasFinishedRace() && m_karts[j]->hasFinishedRace()) ||
                m_kart_info[j].m_overall_distance > my_distance            ||
               (m_kart_info[j].m_overall_distance == my_distance &&
                m_karts[j]->getInitialPosition()<kart->getInitialPosition() ) )
            {
                p++;
            }

        } //next kart

#ifndef DEBUG
        setKartPosition(i, p);
#else
        rank_changed |= kart->getPosition()!=p;
        if (!setKartPosition(i,p))
        {
            Log::error("[LinearWorld]", "Same rank used twice!!");

            Log::debug("[LinearWorld]", "Info used to decide ranking :");
            for (unsigned int d=0; d<kart_amount; d++)
            {
                Log::debug("[LinearWorld]", "Kart %s has finished (%d), is at lap (%u),"
                            "is at distance (%u), is eliminated(%d)",
                            m_karts[d]->getIdent().c_str(),
                            m_karts[d]->hasFinishedRace(),
                            getLapForKart(d),
                            m_kart_info[d].m_overall_distance,
                            m_karts[d]->isEliminated());
            }

            Log::debug("[LinearWorld]", "Who has each ranking so far :");
            for (unsigned int d=0; d<i; d++)
            {
                Log::debug("[LinearWorld]", "%s has rank %d", m_karts[d]->getIdent().c_str(),
                            m_karts[d]->getPosition());
            }

            Log::debug("[LinearWorld]", "    --> And %s is being set at rank %d",
                        kart->getIdent().c_str(), p);
            history->Save();
            assert(false);
        }
#endif

        // Switch on faster music if not already done so, if the
        // first kart is doing its last lap.
        if(!m_faster_music_active                                  &&
            p == 1                                                 &&
            kart_info.m_finished_laps == RaceManager::get()->getNumLaps() - 1 &&
            useFastMusicNearEnd()                                       )
        {
            music_manager->switchToFastMusic();
            m_faster_music_active=true;
        }
    }   // for i<kart_amount

    // Define this to get a detailled analyses each time a race position
    // changes.
#ifdef DEBUG
#undef DEBUG_KART_RANK
#ifdef DEBUG_KART_RANK
    if(rank_changed)
    {
        Log::debug("[LinearWorld]", "Counting laps at %u seconds.", getTime());
        for (unsigned int i=0; i<kart_amount; i++)
        {
            AbstractKart* kart = m_karts[i].get();
            Log::debug("[LinearWorld]", "counting karts ahead of %s (laps %u,"
                        " progress %u, finished %d, eliminated %d, initial position %u.",
                        kart->getIdent().c_str(),
                        m_kart_info[i].m_race_lap,
                        m_kart_info[i].m_overall_distance,
                        kart->hasFinishedRace(),
                        kart->isEliminated(),
                        kart->getInitialPosition());
            // Karts that are either eliminated or have finished the
            // race already have their (final) position assigned. If
            // these karts would get their rank updated, it could happen
            // that a kart that finished first will be overtaken after
            // crossing the finishing line and become second!
            if(kart->isEliminated() || kart->hasFinishedRace()) continue;
            KartInfo& kart_info = m_kart_info[i];
            int p = 1 ;
            const int my_id         = kart->getWorldKartId();
            const float my_distance = m_kart_info[my_id].m_overall_distance;

            for (unsigned int j = 0 ; j < kart_amount ; j++)
            {
                if(j == my_id) continue;
                if(m_karts[j]->isEliminated())
                {
                    Log::debug("[LinearWorld]", " %u: %s because it is eliminated.",
                                p, m_karts[j]->getIdent().c_str());
                    continue;
                }
                if(!kart->hasFinishedRace() && m_karts[j]->hasFinishedRace())
                {
                    p++;
                    Log::debug("[LinearWorld]", " %u: %s because it has finished the race.",
                                p, m_karts[j]->getIdent().c_str());
                    continue;
                }
                if(m_kart_info[j].m_overall_distance > my_distance)
                {
                    p++;
                    Log::debug("[LinearWorld]", " %u: %s because it is ahead %u.",
                                p, m_karts[j]->getIdent().c_str(),
                                m_kart_info[j].m_overall_distance);
                    continue;
                }
                if(m_kart_info[j].m_overall_distance == my_distance &&
                   m_karts[j]->getInitialPosition()<kart->getInitialPosition())
                {
                    p++;
                    Log::debug("[LinearWorld]"," %u: %s has same distance, but started ahead %d",
                                p, m_karts[j]->getIdent().c_str(),
                                m_karts[j]->getInitialPosition());
                }
            }   // next kart j
        }   // for i<kart_amount
        Log::debug("LinearWorld]", "-------------------------------------------");
    }   // if rank_changed
#endif
#endif

    endSetKartPositions();
}   // updateRacePosition

//-----------------------------------------------------------------------------
/** Checks if a kart is going in the wrong direction. This is done only for
 *  player karts to display a message to the player.
 *  \param i Kart id.
 *  \param dt Time step size.
 */
void LinearWorld::checkForWrongDirection(unsigned int i, float dt)
{
    if (!m_karts[i]->getController()->isLocalPlayerController()) 
        return;

    KartInfo &ki = m_kart_info[i];
    
    const AbstractKart *kart=m_karts[i].get();
    // If the kart can go in more than one directions from the current track
    // don't do any reverse message handling, since it is likely that there
    // will be one direction in which it isn't going backwards anyway.
    int sector = getTrackSector(i)->getCurrentGraphNode();
    
    if (DriveGraph::get()->getNumberOfSuccessors(sector) > 1)
        return;

    // check if the player is going in the wrong direction
    const DriveNode* node = DriveGraph::get()->getNode(sector);
    Vec3 center_line = node->getUpperCenter() - node->getLowerCenter();
    float angle_diff = kart->getVelocity().angle(center_line);

    if (angle_diff > M_PI)
        angle_diff -= 2*M_PI;
    else if (angle_diff < -M_PI)
        angle_diff += 2*M_PI;

    // Display a warning message if the kart is going back way, i.e. if angle
    // is too big(unless the kart has already finished the race).
    if ((angle_diff > DEGREE_TO_RAD * 120.0f ||
        angle_diff < -DEGREE_TO_RAD * 120.0f) &&
        kart->getVelocityLC().getY() > 0.0f &&
        !kart->hasFinishedRace())
    {
        ki.m_wrong_way_timer += dt;
        
        if (ki.m_wrong_way_timer> 2.0f)
            ki.m_wrong_way_timer= 2.0f;
    }
    else
    {
        ki.m_wrong_way_timer -= dt;

        if (ki.m_wrong_way_timer < 0)
            ki.m_wrong_way_timer = 0;
    }
    
    if (kart->getKartAnimation())
        ki.m_wrong_way_timer = 0;
    
    if (ki.m_wrong_way_timer > 1.0f && m_race_gui)
    {
        m_race_gui->addMessage(_("WRONG WAY!"), kart,
                               /* time */ -1.0f,
                               video::SColor(255,255,255,255),
                               /*important*/ true,
                               /*big font*/  true);
    }
    
}   // checkForWrongDirection

//-----------------------------------------------------------------------------
void LinearWorld::setLastTriggeredCheckline(unsigned int kart_index, int index)
{
    if (m_kart_info.size() == 0) return;
    getTrackSector(kart_index)->setLastTriggeredCheckline(index);
}   // setLastTriggeredCheckline

//-----------------------------------------------------------------------------
std::pair<uint32_t, uint32_t> LinearWorld::getGameStartedProgress() const
{
    std::pair<uint32_t, uint32_t> progress(
        std::numeric_limits<uint32_t>::max(),
        std::numeric_limits<uint32_t>::max());
    AbstractKart* slowest_kart = NULL;
    for (unsigned i = (unsigned)m_karts.size(); i > 0; i--)
    {
        slowest_kart = getKartAtPosition(i);
        if (slowest_kart && !slowest_kart->isEliminated())
            break;
    }
    if (slowest_kart &&
        getFinishedLapsOfKart(slowest_kart->getWorldKartId()) != -1)
    {
        progress.second = (uint32_t)(
            getOverallDistance(slowest_kart->getWorldKartId()) /
            (Track::getCurrentTrack()->getTrackLength() *
            (float)RaceManager::get()->getNumLaps()) * 100.0f);
    }
    return progress;
}   // getGameStartedProgress

// ----------------------------------------------------------------------------
void LinearWorld::KartInfo::saveCompleteState(BareNetworkString* bns)
{
    bns->addUInt32(m_finished_laps);
    bns->addUInt32(m_ticks_at_last_lap);
    bns->addUInt32(m_lap_start_ticks);
    bns->addFloat(m_estimated_finish);
    bns->addFloat(m_overall_distance);
    bns->addFloat(m_wrong_way_timer);
}   // saveCompleteState

// ----------------------------------------------------------------------------
void LinearWorld::KartInfo::restoreCompleteState(const BareNetworkString& b)
{
    m_finished_laps = b.getUInt32();
    m_ticks_at_last_lap = b.getUInt32();
    m_lap_start_ticks = b.getUInt32();
    m_estimated_finish = b.getFloat();
    m_overall_distance = b.getFloat();
    m_wrong_way_timer = b.getFloat();
}   // restoreCompleteState

// ----------------------------------------------------------------------------
void LinearWorld::saveCompleteState(BareNetworkString* bns, STKPeer* peer)
{
    bns->addUInt32(m_fastest_lap_ticks);
    bns->addFloat(m_distance_increase);
    for (auto& kart : m_karts)
    {
        bns->add(kart->getXYZ());
        bns->add(kart->getRotation());
    }
    for (KartInfo& ki : m_kart_info)
        ki.saveCompleteState(bns);
    for (TrackSector* ts : m_kart_track_sector)
        ts->saveCompleteState(bns);

    CheckManager* cm = Track::getCurrentTrack()->getCheckManager();
    const uint8_t cc = (uint8_t)cm->getCheckStructureCount();
    bns->addUInt8(cc);
    for (unsigned i = 0; i < cc; i++)
        cm->getCheckStructure(i)->saveCompleteState(bns);
}   // saveCompleteState

// ----------------------------------------------------------------------------
void LinearWorld::restoreCompleteState(const BareNetworkString& b)
{
    m_fastest_lap_ticks = b.getUInt32();
    m_distance_increase = b.getFloat();
    for (auto& kart : m_karts)
    {
        btTransform t;
        Vec3 xyz = b.getVec3();
        t.setOrigin(xyz);
        t.setRotation(b.getQuat());
        kart->setTrans(t);
        kart->setXYZ(xyz);
    }
    for (KartInfo& ki : m_kart_info)
        ki.restoreCompleteState(b);
    for (TrackSector* ts : m_kart_track_sector)
        ts->restoreCompleteState(b);

    updateRacePosition();
    const unsigned cc = b.getUInt8();
    CheckManager* cm = Track::getCurrentTrack()->getCheckManager();
    if (cc != cm->getCheckStructureCount())
    {
        Log::warn("LinearWorld",
            "Server has different check structures size.");
        return;
    }
    for (unsigned i = 0; i < cc; i++)
        cm->getCheckStructure(i)->restoreCompleteState(b);
}   // restoreCompleteState

// ----------------------------------------------------------------------------
/** Called in server whenever a kart cross a check line, it send server
 *  current kart lap count, last triggered checkline and check structure status
 *  to all players in game (including spectators so that the lap count is
 *  correct)
 *  \param check_id The check structure it it triggered.
 *  \param kart_id The kart which triggered a checkline.
 */
void LinearWorld::updateCheckLinesServer(int check_id, int kart_id)
{
    if (!NetworkConfig::get()->isNetworking() ||
        NetworkConfig::get()->isClient())
        return;

    NetworkString cl(PROTOCOL_GAME_EVENTS);
    cl.setSynchronous(true);
    cl.addUInt8(GameEventsProtocol::GE_CHECK_LINE).addUInt8((uint8_t)check_id)
        .addUInt8((uint8_t)kart_id);

    int8_t finished_laps = (int8_t)m_kart_info[kart_id].m_finished_laps;
    cl.addUInt8(finished_laps);

    int8_t ltcl =
        (int8_t)m_kart_track_sector[kart_id]->getLastTriggeredCheckline();
    cl.addUInt8(ltcl);

    cl.addUInt32(m_fastest_lap_ticks);
    cl.encodeString(m_fastest_lap_kart_name);

    CheckManager* cm = Track::getCurrentTrack()->getCheckManager();
    const uint8_t cc = (uint8_t)cm->getCheckStructureCount();
    cl.addUInt8(cc);
    for (unsigned i = 0; i < cc; i++)
        cm->getCheckStructure(i)->saveIsActive(kart_id, &cl);

    STKHost::get()->sendPacketToAllPeers(&cl, true);
}   // updateCheckLinesServer

// ----------------------------------------------------------------------------
/* Synchronize with server from the above data. */
void LinearWorld::updateCheckLinesClient(const BareNetworkString& b)
{
    // Reserve for future auto checkline correction
    //int check_id = b.getUInt8();
    b.getUInt8();
    int kart_id = b.getUInt8();

    int8_t finished_laps = b.getUInt8();
    m_kart_info.at(kart_id).m_finished_laps = finished_laps;

    int8_t ltcl = b.getUInt8();
    m_kart_track_sector.at(kart_id)->setLastTriggeredCheckline(ltcl);

    m_fastest_lap_ticks = b.getUInt32();
    b.decodeStringW(&m_fastest_lap_kart_name);

    const unsigned cc = b.getUInt8();
    if (cc != Track::getCurrentTrack()->getCheckManager()->getCheckStructureCount())
        return;
    for (unsigned i = 0; i < cc; i++)
        Track::getCurrentTrack()->getCheckManager()->getCheckStructure(i)->restoreIsActive(kart_id, b);

}   // updateCheckLinesClient

// ----------------------------------------------------------------------------
void LinearWorld::handleServerCheckStructureCount(unsigned count)
{
    if (count != Track::getCurrentTrack()->getCheckManager()->getCheckStructureCount())
    {
        Log::warn("LinearWorld",
            "Server has different check structures size.");
        m_check_structure_compatible = false;
    }
    else
        m_check_structure_compatible = true;
}   // handleServerCheckStructureCount
