//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2006-2015 SuperTuxKart-Team
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

#include "race/race_manager.hpp"

#include <iostream>
#include <algorithm>
#include <random>

#include "challenges/unlock_manager.hpp"
#include "config/player_manager.hpp"
#include "config/saved_grand_prix.hpp"
#include "config/stk_config.hpp"
#include "config/user_config.hpp"
#include "graphics/irr_driver.hpp"
#include "guiengine/message_queue.hpp"
#include "input/device_manager.hpp"
#include "input/input_manager.hpp"
#include "karts/abstract_kart.hpp"
#include "karts/controller/controller.hpp"
#include "karts/kart_properties_manager.hpp"
#include "main_loop.hpp"
#include "modes/capture_the_flag.hpp"
#include "modes/cutscene_world.hpp"
#include "modes/demo_world.hpp"
#include "modes/easter_egg_hunt.hpp"
#include "modes/follow_the_leader.hpp"
#include "modes/free_for_all.hpp"
#include "modes/overworld.hpp"
#include "modes/standard_race.hpp"
#include "modes/tutorial_world.hpp"
#include "modes/world.hpp"
#include "modes/three_strikes_battle.hpp"
#include "modes/soccer_world.hpp"
#include "modes/lap_trial.hpp"
#include "network/protocol_manager.hpp"
#include "network/network_config.hpp"
#include "network/network_string.hpp"
#include "replay/replay_play.hpp"
#include "scriptengine/property_animator.hpp"
#include "states_screens/grand_prix_cutscene.hpp"
#include "states_screens/grand_prix_lose.hpp"
#include "states_screens/grand_prix_win.hpp"
#include "states_screens/kart_selection.hpp"
#include "states_screens/main_menu_screen.hpp"
#include "states_screens/state_manager.hpp"
#include "tracks/track_manager.hpp"
#include "utils/ptr_vector.hpp"
#include "utils/stk_process.hpp"
#include "utils/string_utils.hpp"
#include "utils/translation.hpp"
#include "io/rich_presence.hpp"

#include <IrrlichtDevice.h>

#ifdef __SWITCH__
extern "C" {
  #define u64 uint64_t
  #define u32 uint32_t
  #define s64 int64_t
  #define s32 int32_t
  #define Event libnx_Event
  #include <switch/services/applet.h>
  #undef Event
  #undef u64
  #undef u32
  #undef s64
  #undef s32
}
#endif

//=============================================================================================
RaceManager* g_race_manager[PT_COUNT];
//---------------------------------------------------------------------------------------------
RaceManager* RaceManager::get()
{
    ProcessType type = STKProcess::getType();
    return g_race_manager[type];
}   // get

//---------------------------------------------------------------------------------------------
void RaceManager::create()
{
    ProcessType type = STKProcess::getType();
    g_race_manager[type] = new RaceManager();
}   // create

//---------------------------------------------------------------------------------------------
void RaceManager::destroy()
{
    ProcessType type = STKProcess::getType();
    delete g_race_manager[type];
    g_race_manager[type] = NULL;
}   // destroy

//---------------------------------------------------------------------------------------------
void RaceManager::clear()
{
    memset(g_race_manager, 0, sizeof(g_race_manager));
}   // clear

//---------------------------------------------------------------------------------------------
/** Constructs the race manager.
 */
RaceManager::RaceManager()
{
    // Several code depends on this, e.g. kart_properties
    assert(DIFFICULTY_FIRST == 0);
    m_num_karts          = UserConfigParams::m_default_num_karts;
    m_num_ghost_karts    = 0;
    m_difficulty         = DIFFICULTY_HARD;
    m_major_mode         = MAJOR_MODE_SINGLE;
    m_minor_mode         = MINOR_MODE_NORMAL_RACE;
    m_ai_superpower      = SUPERPOWER_NONE;
    m_track_number       = 0;
    m_coin_target        = 0;
    m_started_from_overworld = false;
    m_have_kart_last_position_on_overworld = false;
    m_num_local_players = 0;
    m_hit_capture_limit = 0;
    m_flag_return_ticks = stk_config->time2Ticks(20.0f);
    m_flag_deactivated_ticks = stk_config->time2Ticks(3.0f);
    m_skipped_tracks_in_gp = 0;
    m_gp_time_target = 0.0f;
    m_gp_total_laps = 0;
    setMaxGoal(0);
    setTimeTarget(0.0f);
    setReverseTrack(false);
    setRecordRace(false);
    setRaceGhostKarts(false);
    setWatchingReplay(false);
    setTrack("jungle");
    m_default_ai_list.clear();
    setNumPlayers(0);
    setSpareTireKartNum(0);
}   // RaceManager

//---------------------------------------------------------------------------------------------
/** Destructor for the race manager.
 */
RaceManager::~RaceManager()
{
}   // ~RaceManager

//---------------------------------------------------------------------------------------------
/** Resets the race manager in preparation for a new race. It sets the
 *  counter of finished karts to zero. It is called by world when 
 *  restarting a race.
 */
void RaceManager::reset()
{
    m_num_finished_karts   = 0;
    m_num_finished_players = 0;
}  // reset

// ----------------------------------------------------------------------------
/** Sets the default list of AI karts to use.
 *  \param ai_kart_list List of the identifier of the karts to use.
 */
void RaceManager::setDefaultAIKartList(const std::vector<std::string>& ai_list)
{
    for(unsigned int i=0; i<ai_list.size(); i++)
    {
        const std::string &name=ai_list[i];
        const KartProperties *kp = kart_properties_manager->getKart(name);
        if(!kp)
        {
            Log::warn("RaceManager", "Kart '%s' is unknown and therefore ignored.",
                      name.c_str());
            continue;
        }
        // This doesn't work anymore, since this is called when
        // handling the command line options, at which time the
        // player (and therefore the current slot) is not defined yet.
        //if(unlock_manager->getCurrentSlot()->isLocked(name))
        //{
        //   Log::info("RaceManager", "Kart '%s' is locked and therefore ignored.",
        //           name.c_str());
        //    continue;
        //}
        m_default_ai_list.push_back(name);
    }
}   // setDefaultAIKartList

//---------------------------------------------------------------------------------------------
/** \brief Sets a player kart (local and non-local).
 *  \param player_id  Id of the player.
 *  \param ki         Kart info structure for this player.
 */
void RaceManager::setPlayerKart(unsigned int player_id, const RemoteKartInfo& ki)
{
    m_player_karts[player_id] = ki;
}   // setPlayerKart

// ----------------------------------------------------------------------------
void RaceManager::setPlayerKart(unsigned int player_id,
                                 const std::string &kart_name)
{
    const PlayerProfile* profile =
                    StateManager::get()->getActivePlayerProfile(player_id);
    RemoteKartInfo rki(player_id, kart_name, profile->getName(), 0, false);
    m_player_karts[player_id] = rki;
}   // setPlayerKart

//---------------------------------------------------------------------------------------------
/** Sets additional information for a player to indicate which soccer team it
 *  belongs to.
*/
void RaceManager::setKartTeam(unsigned int player_id, KartTeam team)
{
    assert(player_id < m_player_karts.size());

    m_player_karts[player_id].setKartTeam(team);
}   // setKartTeam

//---------------------------------------------------------------------------------------------
/** Sets the handicap for a player.
 */
void RaceManager::setPlayerHandicap(unsigned int player_id, HandicapLevel handicap)
{
    assert(player_id < m_player_karts.size());

    m_player_karts[player_id].setHandicap(handicap);
}   // setPlayerHandicap

//---------------------------------------------------------------------------------------------
/** Returns a pointer to the kart which has a given GP rank.
 *  \param n The rank (1 to number of karts) to look for.
 */
const AbstractKart *RaceManager::getKartWithGPRank(unsigned int n)
{
    for(unsigned int i=0; i<m_kart_status.size(); i++)
        if(m_kart_status[i].m_gp_rank == (int)n)
            return World::getWorld()->getKart(i);
    return NULL;
}   // getKLartWithGPRank

//---------------------------------------------------------------------------------------------
/** Returns the GP rank (between 1 and number of karts) of a local player.
 *  \param player_id Local id of the player.
 */
int RaceManager::getLocalPlayerGPRank(const int player_id) const
{
    const int amount = (int)m_kart_status.size();
    for (int n=0; n<amount; n++)
    {
        if (m_kart_status[n].m_local_player_id == player_id)
        {
            return m_kart_status[n].m_gp_rank;
        }
    }
    return -1;
}   // getLocalPlayerGPRank

//---------------------------------------------------------------------------------------------
/** Sets the number of players and optional the number of local players.
 *  \param num Number of players.
 *  \param local_players Number of local players, only used from networking.
 */
void RaceManager::setNumPlayers(int players, int local_players)
{
    // Clear all previous game info from network (like country code atm)
    // The rest info need to be store for overworld, see #3980
    for (RemoteKartInfo& rki : m_player_karts)
        rki.setCountryCode("");
    m_player_karts.resize(players);
    if(local_players>-1)
        m_num_local_players = local_players;
    else
        m_num_local_players = players;
}   // setNumPlayers

// ----------------------------------------------------------------------------
/** Converst the difficulty given as a string into a Difficult enum. Defaults
 *  to HARD.
 *  \param difficulty The difficulty as string.
 */
RaceManager::Difficulty 
                  RaceManager::convertDifficulty(const std::string &difficulty)
{
    if (difficulty == "novice")
        return DIFFICULTY_EASY;
    else if (difficulty == "intermediate")
        return DIFFICULTY_MEDIUM;
    else if (difficulty == "expert")
        return DIFFICULTY_HARD;
    else if (difficulty == "best")
        return DIFFICULTY_BEST;
    else
        return DIFFICULTY_HARD;
}   // convertDifficulty

//---------------------------------------------------------------------------------------------
/** Sets the difficulty to use.
 *  \param diff The difficulty to use.
 */
void RaceManager::setDifficulty(Difficulty diff)
{
    m_difficulty = diff;
}   // setDifficulty

//---------------------------------------------------------------------------------------------
/** Sets a single track to be used in the next race.
 *  \param track The identifier of the track to use.
 */
void RaceManager::setTrack(const std::string& track)
{
    m_tracks.clear();
    m_tracks.push_back(track);

    m_coin_target = 0;
}   // setTrack

//---------------------------------------------------------------------------------------------
/** \brief Computes the list of random karts to be used for the AI.
 *  If a command line option specifies karts, they will be used first
 */
void RaceManager::computeRandomKartList()
{
    int n = m_num_karts - (int)m_player_karts.size();
    if(UserConfigParams::logMisc())
        Log::info("RaceManager", "AI karts count = %d for m_num_karts = %d and "
            "m_player_karts.size() = %d", n, m_num_karts, m_player_karts.size());

    // If less kart selected than there are player karts, adjust the number of
    // karts to the minimum
    if(n<0)
    {
        m_num_karts -= n;
        n = 0;
    }

    m_ai_kart_list.clear();

    //Use the command line options AI list.
    unsigned int m = std::min( (unsigned) m_num_karts,  (unsigned)m_default_ai_list.size());

    for(unsigned int i=0; i<m; i++)
    {
        m_ai_kart_list.push_back(m_default_ai_list[i]);
        n--;
    }

    if(n>0)
        kart_properties_manager->getRandomKartList(n, &m_player_karts,
                                                   &m_ai_kart_list   );

    if (m_ai_kart_override != "")
    {
        for (unsigned int n = 0; n < m_ai_kart_list.size(); n++)
        {
            m_ai_kart_list[n] = m_ai_kart_override;
        }
    }

}   // computeRandomKartList

//---------------------------------------------------------------------------------------------
/** \brief Starts a new race or GP (or other mode).
 *  It sets up the list of player karts, AI karts, GP tracks if relevant
 *  etc.
 *  \pre The list of AI karts to use must be set up first. This is
 *       usually being done by a call to computeRandomKartList() from
 *       NetworkManager::setupPlayerKartInfo, but could be done differently
 *       (e.g. depending on user command line options to test certain AIs)
 *  \param from_overworld True if the race/GP is started from overworld
 *         (used to return to overworld at end of race/GP).
 */
void RaceManager::startNew(bool from_overworld)
{
    m_num_ghost_karts = 0;
    if (m_has_ghost_karts)
        m_num_ghost_karts = ReplayPlay::get()->getNumGhostKart();

    m_started_from_overworld = from_overworld;
    if (m_started_from_overworld) m_continue_saved_gp = false;
    m_saved_gp = NULL; // There will be checks for this being NULL done later

    if (m_major_mode==MAJOR_MODE_GRAND_PRIX)
    {
        // GP: get tracks, laps and reverse info from grand prix
        m_tracks        = m_grand_prix.getTrackNames();
        m_num_laps      = m_grand_prix.getLaps();
        m_reverse_track = m_grand_prix.getReverse();

        if (!NetworkConfig::get()->isNetworking())
        {
            // We look if Player 1 has a saved version of this GP.
            m_saved_gp = SavedGrandPrix::getSavedGP(
                                         StateManager::get()
                                         ->getActivePlayerProfile(0)
                                         ->getUniqueID(),
                                         m_grand_prix.getId(),
                                         m_minor_mode,
                                         (unsigned int)m_player_karts.size());
    
            // Saved GP only in offline mode
            if (m_continue_saved_gp)
            {
                if (m_saved_gp == NULL)
                {
                    Log::error("Race Manager", "Can not continue Grand Prix '%s'"
                                               "because it could not be loaded",
                                               m_grand_prix.getId().c_str());
                    m_continue_saved_gp = false; // simple and working
                }
                else
                {
                    setNumKarts(m_saved_gp->getTotalKarts());
                    setupPlayerKartInfo();
                    m_grand_prix.changeReverse((GrandPrixData::GPReverseType)
                                                m_saved_gp->getReverseType());
                    m_reverse_track = m_grand_prix.getReverse();
                    m_skipped_tracks_in_gp = m_saved_gp->getSkippedTracks();
                    Log::info("RaceManager","%d",isLapTrialMode());
                    if (isLapTrialMode())
                    {
                        m_gp_time_target = m_saved_gp->getTimeTarget();
                        m_gp_total_laps = m_saved_gp->getPlayerTotalLaps();
                    }
                }   // if m_saved_gp==NULL
            }   // if m_continue_saved_gp
        }   // if !network_world
    }   // if grand prix

    // command line parameters: negative numbers=all karts
    if(m_num_karts < 0 ) m_num_karts = stk_config->m_max_karts;
    if((size_t)m_num_karts < m_player_karts.size())
        m_num_karts = (int)m_player_karts.size();

    // Create the kart status data structure to keep track of scores, times, ...
    // ==========================================================================
    m_kart_status.clear();
    if (m_num_ghost_karts > 0)
        m_num_karts += m_num_ghost_karts;

    Log::verbose("RaceManager", "Nb of karts=%u, ghost karts:%u ai:%lu players:%lu\n",
        (unsigned int) m_num_karts, m_num_ghost_karts, m_ai_kart_list.size(), m_player_karts.size());
    std::set<std::string> used_karts;
    for (auto& kart : m_ai_kart_list)
        used_karts.insert(kart);
    for (auto& kart : m_player_karts)
        used_karts.insert(kart.getKartName());
    kart_properties_manager->onDemandLoadKartTextures(used_karts);

    assert((unsigned int)m_num_karts == m_num_ghost_karts+m_ai_kart_list.size()+m_player_karts.size());

    // First add the ghost karts (if any)
    // ----------------------------------------
    // GP ranks start with -1 for the leader.
    int init_gp_rank = getMinorMode()==MINOR_MODE_FOLLOW_LEADER ? -1 : 0;
    if (m_num_ghost_karts > 0)
    {
        for(unsigned int i = 0; i < m_num_ghost_karts; i++)
        {
            m_kart_status.push_back(KartStatus(ReplayPlay::get()->getGhostKartName(i),
                i, -1, -1, init_gp_rank, KT_GHOST, HANDICAP_NONE));
            init_gp_rank ++;
        }
    }

    // Then add the AI karts (randomly chosen)
    // ----------------------------------------
    const unsigned int ai_kart_count = (unsigned int)m_ai_kart_list.size();
    for(unsigned int i = 0; i < ai_kart_count; i++)
    {
        m_kart_status.push_back(KartStatus(m_ai_kart_list[i], i, -1, -1,
            init_gp_rank, KT_AI, HANDICAP_NONE));
        init_gp_rank ++;
        if(UserConfigParams::m_ftl_debug)
        {
            Log::debug("RaceManager", "[ftl] rank %d ai-kart %s", init_gp_rank,
                   m_ai_kart_list[i].c_str());
        }
    }

    // Finally add the players, which start behind the AI karts
    // -----------------------------------------------------
    for(unsigned int i = 0; i < m_player_karts.size(); i++)
    {
        KartType kt= m_player_karts[i].isNetworkPlayer() ? KT_NETWORK_PLAYER 
                                                         : KT_PLAYER;
        m_kart_status.push_back(KartStatus(m_player_karts[i].getKartName(), i,
                                           m_player_karts[i].getLocalPlayerId(),
                                           m_player_karts[i].getGlobalPlayerId(),
                                           init_gp_rank, kt,
                                           m_player_karts[i].getHandicap()));
        if(UserConfigParams::m_ftl_debug)
        {
            Log::debug("RaceManager", "[ftl] rank %d kart %s", init_gp_rank,
                m_player_karts[i].getKartName().c_str());
        }
        init_gp_rank ++;
    }

    m_track_number = 0;
    if (m_major_mode == MAJOR_MODE_GRAND_PRIX)
    {
        if (m_continue_saved_gp)
        {
            int next_track = m_saved_gp->getNextTrack();
            if (next_track < (int)m_tracks.size())
                m_track_number = next_track;
            m_saved_gp->loadKarts(m_kart_status);
            // Update the kart GP ranks
            // This is useful, e.g., when continuing a saved GP, see #2776
            computeGPRanks();
        }
        else 
        {
            while (m_saved_gp != NULL)
            {
                m_saved_gp->remove();
                m_saved_gp = SavedGrandPrix::getSavedGP(
                                             StateManager::get()
                                             ->getActivePlayerProfile(0)
                                             ->getUniqueID(),
                                             m_grand_prix.getId(),
                                             m_minor_mode,
                                             (unsigned int)m_player_karts.size());
            }   // while m_saved_gp
        }   // if m_continue_saved_gp
    }   // if grand prix

    startNextRace();
}   // startNew

//---------------------------------------------------------------------------------------------
/** \brief Starts the next (or first) race.
 *  It sorts the kart status data structure
 *  according to the number of points, and then creates the world().
 */
void RaceManager::startNextRace()
{
#ifdef __SWITCH__
    // Throttles GPU while boosting CPU
    appletSetCpuBoostMode(ApmCpuBoostMode_FastLoad);
#endif  
    ProcessType type = STKProcess::getType();
    main_loop->renderGUI(0);
    // Uncomment to debug audio leaks
    // sfx_manager->dump();

    if (type == PT_MAIN)
    {
        IrrlichtDevice* device = irr_driver->getDevice();
        device->getVideoDriver()->beginScene(true, true,
                                            video::SColor(255,100,101,140));
        GUIEngine::clearLoadingTips();
        GUIEngine::renderLoading(true/*clearIcons*/, false/*launching*/, false/*update_tips*/);
        device->getVideoDriver()->endScene();
        GUIEngine::flushRenderLoading(false/*launching*/);
    }

    m_num_finished_karts   = 0;
    m_num_finished_players = 0;
    // In follow the leader mode do not change the first kart,
    // since it's always the leader.
    int offset = (m_minor_mode==MINOR_MODE_FOLLOW_LEADER) ? 1 : 0;

    // if subsequent race, sort kart status structure
    // ==============================================
    if (m_track_number > 0)
    {
        // Keep players at the end if needed
        int player_last_offset = 0;
        if (UserConfigParams::m_gp_player_last)
        {
            // Doing this is enough to keep player karts at
            // the end because of the simple reason that they
            // are at the end when getting added. Keep them out
            // of the later sorting and they will stay there.
            player_last_offset = (int)m_player_karts.size();
        }

        std::sort(m_kart_status.begin()+offset,
                  m_kart_status.end() - player_last_offset);
        // reverse kart order if flagged in user's config
        if (UserConfigParams::m_gp_most_points_first)
        {
            std::reverse(m_kart_status.begin()+offset,
                         m_kart_status.end() - player_last_offset);
        }
    }   // not first race
    else
    {
        const bool random_pos_available = !NetworkConfig::get()->isNetworking() &&
            !m_has_ghost_karts &&
            (RaceManager::get()->getMinorMode() == RaceManager::MINOR_MODE_NORMAL_RACE
            || RaceManager::get()->getMinorMode() == RaceManager::MINOR_MODE_TIME_TRIAL
            || RaceManager::get()->getMinorMode() == RaceManager::MINOR_MODE_FOLLOW_LEADER);

        if (UserConfigParams::m_random_player_pos)
        {
            if (random_pos_available)
            {
                unsigned seed = std::chrono::system_clock::now().time_since_epoch().count();
                std::shuffle(m_kart_status.begin() + offset, m_kart_status.end(),
                    std::default_random_engine(seed));
            }
        }
    }

    // set boosted AI status for AI karts
    int boosted_ai_count = std::min<int>((int)m_ai_kart_list.size(),
                                         ((int)(m_kart_status.size())-2)/4 + 1);
    if (boosted_ai_count > 4) boosted_ai_count = 4;
    int ai_count = (int)m_ai_kart_list.size();

    for (unsigned int i=0;i<m_kart_status.size();i++)
    {
        if (m_kart_status[i].m_kart_type == KT_AI)
        {
            if (boosted_ai_count > 0 &&
                (UserConfigParams::m_gp_most_points_first ||
                ai_count == boosted_ai_count))
            {
                m_kart_status[i].m_boosted_ai = true;
                boosted_ai_count--;
            }
            else
            {
                m_kart_status[i].m_boosted_ai = false;
            }
            ai_count--;
        }
    }

    main_loop->renderGUI(100);

    // the constructor assigns this object to the global
    // variable world. Admittedly a bit ugly, but simplifies
    // handling of objects which get created in the constructor
    // and need world to be defined.
    if(DemoWorld::isDemoMode())
        World::setWorld(new DemoWorld());
    else if(ProfileWorld::isProfileMode())
        World::setWorld(new ProfileWorld());
    else if(m_minor_mode==MINOR_MODE_FOLLOW_LEADER)
        World::setWorld(new FollowTheLeaderRace());
    else if(m_minor_mode==MINOR_MODE_NORMAL_RACE ||
            m_minor_mode==MINOR_MODE_TIME_TRIAL)
        World::setWorld(new StandardRace());
    else if(m_minor_mode==MINOR_MODE_LAP_TRIAL)
    {
        World::setWorld(new LapTrial());
        if (m_major_mode == MAJOR_MODE_GRAND_PRIX)
            RaceManager::get()->setTimeTarget(m_gp_time_target);
    }
    else if(m_minor_mode==MINOR_MODE_TUTORIAL)
        World::setWorld(new TutorialWorld());
    else if (isBattleMode())
    {
        if (m_minor_mode == MINOR_MODE_3_STRIKES)
            World::setWorld(new ThreeStrikesBattle());
        else if (m_minor_mode == MINOR_MODE_FREE_FOR_ALL)
            World::setWorld(new FreeForAll());
        else if (m_minor_mode == MINOR_MODE_CAPTURE_THE_FLAG)
            World::setWorld(new CaptureTheFlag());
    }
    else if(m_minor_mode==MINOR_MODE_SOCCER)
        World::setWorld(new SoccerWorld());
    else if(m_minor_mode==MINOR_MODE_OVERWORLD)
        World::setWorld(new OverWorld());
    else if(m_minor_mode==MINOR_MODE_CUTSCENE)
        World::setWorld(new CutsceneWorld());
    else if(m_minor_mode==MINOR_MODE_EASTER_EGG)
        World::setWorld(new EasterEggHunt());
    else
    {
        Log::error("RaceManager", "Could not create given race mode.");
        assert(0);
    }
    main_loop->renderGUI(200);

    // A second constructor phase is necessary in order to be able to
    // call functions which are overwritten (otherwise polymorphism
    // will fail and the results will be incorrect). Also in init() functions
    // can be called that use World::getWorld().
    World::getWorld()->init();
    main_loop->renderGUI(8000);
    // Now initialise all values that need to be reset from race to race
    // Calling this here reduces code duplication in init and restartRace()
    // functions.
    World::getWorld()->reset();

    if (NetworkConfig::get()->isNetworking())
    {
        for (unsigned i = 0; i < getNumPlayers(); i++)
        {
            // Eliminate all reserved players in the begining
            const RemoteKartInfo& rki = getKartInfo(i);
            if (rki.isReserved())
            {
                AbstractKart* k = World::getWorld()->getKart(i);
                World::getWorld()->eliminateKart(i,
                    false/*notify_of_elimination*/);
                k->setPosition(
                    World::getWorld()->getCurrentNumKarts() + 1);
                k->finishedRace(World::getWorld()->getTime(),
                    true/*from_server*/);
            }
        }
    }

    if (type == PT_MAIN)
        irr_driver->onLoadWorld();
    main_loop->renderGUI(8100);

    // Save the current score and set last time to zero. This is necessary
    // if someone presses esc after finishing a gp, and selects restart:
    // The race is rerun, and the points and scores get reset ... but if
    // a kart hasn't finished the race at this stage, last_score and time
    // would be undefined.
    for(int i=0; i<m_num_karts; i++)
    {
        m_kart_status[i].m_last_score = m_kart_status[i].m_score;
        m_kart_status[i].m_last_time  = 0;
    }
    main_loop->renderGUI(8200);
#ifdef __SWITCH__
    appletSetCpuBoostMode(ApmCpuBoostMode_Normal);
#endif

    RichPresenceNS::RichPresence::get()->update(true);
}   // startNextRace

//---------------------------------------------------------------------------------------------
/** \brief Start the next race or go back to the start screen
 * If there are more races to do, starts the next race, otherwise
 * calls exitRace to finish the race.
 */
void RaceManager::next()
{
    if (STKProcess::getType() == PT_MAIN)
        PropertyAnimator::get()->clear();
    World::deleteWorld();
    m_num_finished_karts   = 0;
    m_num_finished_players = 0;
    m_track_number++;
    if(m_track_number<(int)m_tracks.size())
    {
        if (m_major_mode == MAJOR_MODE_GRAND_PRIX &&
            !NetworkConfig::get()->isNetworking())
        {
            // Saving GP state
            saveGP();
        }
        startNextRace();
    }
    else
    {
        exitRace();
    }
}   // next

//---------------------------------------------------------------------------------------------
/** Saves the current GP to the config.
 */
void RaceManager::saveGP()
{
    // If Player 1 has already saved a GP, we adapt it
    if (m_saved_gp != NULL)
    {
        m_saved_gp->setKarts(m_kart_status);
        m_saved_gp->setNextTrack(m_track_number);
    }
    else  if(!m_grand_prix.isRandomGP())
    {
        m_saved_gp = new SavedGrandPrix(
            StateManager::get()->getActivePlayerProfile(0)->getUniqueID(),
            m_grand_prix.getId(),
            m_minor_mode,
            m_difficulty,
            (int)m_player_karts.size(),
            m_track_number,
            m_grand_prix.getReverseType(),
            m_skipped_tracks_in_gp,
            isLapTrialMode() ? m_gp_time_target : 0.0f,
            isLapTrialMode() ? m_gp_total_laps : 0,
            m_kart_status);

        // If a new GP is saved, delete any other saved data for this
        // GP at the same difficulty (even if #karts is different, otherwise
        // the user has to remember the number of AI karts, with no indication
        // on which ones are saved).
        for (unsigned int i = 0;
            i < UserConfigParams::m_saved_grand_prix_list.size();)
        {
            // Delete other save files (and random GP, which should never
            // have been saved in the first place)
            const SavedGrandPrix &sgp =
                                  UserConfigParams::m_saved_grand_prix_list[i];
            if (sgp.getGPID() == "random"                          ||
                (sgp.getGPID() == m_saved_gp->getGPID() &&
                 sgp.getDifficulty() == m_saved_gp->getDifficulty())    )
            {
                UserConfigParams::m_saved_grand_prix_list.erase(i);
            }
            else i++;
        }
        UserConfigParams::m_saved_grand_prix_list.push_back(m_saved_gp);
    }

    user_config->saveConfig();
}   // saveGP

//---------------------------------------------------------------------------------------------

/** This class is only used in computeGPRanks, but the C++ standard
 *  forbids the usage of local data type in templates, so we have to
 *  declare it outside of the function. This class is used to store
 *  and compare data for determining the GP rank.
 */
namespace computeGPRanksData
{
    class SortData
    {
    public:
        int m_score;
        int m_position;
        float m_race_time;
        bool operator<(const SortData &a)
        {
            if (RaceManager::get()->getMinorMode()==RaceManager::MINOR_MODE_FOLLOW_LEADER)
            {
                return ( (m_score > a.m_score) ||
                (m_score == a.m_score && m_race_time > a.m_race_time) );
            }
            return ( (m_score > a.m_score) ||
                (m_score == a.m_score && m_race_time < a.m_race_time) );
        }

    };   // SortData
}   // namespace

// ----------------------------------------------------------------------------
/** Sort karts and update the m_gp_rank KartStatus member, in preparation
 *  for future calls to RaceManager::getKartGPRank or
 *  RaceManager::getKartWithGPRank
 */
void RaceManager::computeGPRanks()
{
    // calculate the rank of each kart
    const unsigned int NUM_KARTS = getNumberOfKarts();
    PtrVector<computeGPRanksData::SortData> sort_data;

    // Ignore the first kart if it's a follow-the-leader race.
    int start=(getMinorMode()==RaceManager::MINOR_MODE_FOLLOW_LEADER);
    if (start)
    {
        // fill values for leader
        computeGPRanksData::SortData *sd = new computeGPRanksData::SortData();

        sd->m_position  = -1;
        sd->m_score     = -1;
        sd->m_race_time = -1;
        sort_data.push_back(sd);
        m_kart_status[0].m_gp_rank = -1;
        if(UserConfigParams::m_ftl_debug)
        {
            Log::debug("Race Manager","[ftl] kart '%s' has position %d.",
                       World::getWorld()->getKart(0)->getIdent().c_str(),
                       sd->m_position);
        }
    }
    for (unsigned int kart_id = start; kart_id < NUM_KARTS; ++kart_id)
    {
        computeGPRanksData::SortData *sd = new computeGPRanksData::SortData();
        sd->m_position  = kart_id;
        sd->m_score     = getKartScore(kart_id);
        sd->m_race_time = getOverallTime(kart_id);
        sort_data.push_back(sd);
        if(UserConfigParams::m_ftl_debug)
        {
            Log::debug("Race Manager",
                       "[ftl] kart '%s' has position %d score %d.",
                       World::getWorld()->getKart(kart_id)->getIdent().c_str(),
                       sd->m_position, sd->m_score);
        }
    }

    sort_data.insertionSort(start);
    for (unsigned int i=start; i < NUM_KARTS; ++i)
    {
        if(UserConfigParams::m_ftl_debug)
        {
            const AbstractKart *kart =
                World::getWorld()->getKart(sort_data[i].m_position);
            Log::debug("Race Manager","[ftl] kart '%s' has now position %d.",
                kart->getIdent().c_str(),
                i-start);
        }

        m_kart_status[sort_data[i].m_position].m_gp_rank = i - start;
    }
}   // computeGPRanks

//---------------------------------------------------------------------------------------------
/** \brief Exit a race (and don't start the next one)
 * \note In GP, displays the GP result screen first
 * \param delete_world If set deletes the world.
 */
void RaceManager::exitRace(bool delete_world)
{
    // Only display the grand prix result screen if all tracks
    // were finished, and not when a race is aborted.
    MessageQueue::discardStatic();
    ProcessType type = STKProcess::getType();

    if ( m_major_mode==MAJOR_MODE_GRAND_PRIX &&
         m_track_number==(int)m_tracks.size()   )
    {
        PlayerManager::getCurrentPlayer()->grandPrixFinished();
        if (m_major_mode == MAJOR_MODE_GRAND_PRIX &&
            !NetworkConfig::get()->isNetworking())
        {
            if(m_saved_gp != NULL)
                m_saved_gp->remove();
        }
        StateManager::get()->resetAndGoToScreen( MainMenuScreen::getInstance() );

        bool some_human_player_well_ranked = false;
        bool some_human_player_won = false;
        const unsigned int kart_status_count = (unsigned int)m_kart_status.size();

        const int loserThreshold = 3;

        std::pair<std::string, float> winners[3];
        // because we don't care about AIs that lost
        std::vector<std::pair<std::string, float> > humanLosers;
        for (unsigned int i=0; i < kart_status_count; ++i)
        {
            if(UserConfigParams::logMisc())
            {
                Log::info("RaceManager", "%s has GP final rank %d",
                    m_kart_status[i].m_ident.c_str(), m_kart_status[i].m_gp_rank);
            }

            const int rank = m_kart_status[i].m_gp_rank;
            if (rank >= 0 && rank < loserThreshold)
            {
                winners[rank].first = m_kart_status[i].m_ident;
                winners[rank].second = m_kart_status[i].m_color;
                if (m_kart_status[i].m_kart_type == KT_PLAYER ||
                    m_kart_status[i].m_kart_type == KT_NETWORK_PLAYER)
                {
                    some_human_player_well_ranked = true;
                    if (rank == 0)
                        some_human_player_won = true;
                }
            }
            else if (rank >= loserThreshold)
            {
                if (m_kart_status[i].m_kart_type == KT_PLAYER ||
                    m_kart_status[i].m_kart_type == KT_NETWORK_PLAYER)
                {
                    humanLosers.emplace_back(m_kart_status[i].m_ident, m_kart_status[i].m_color);
                }
            }
        }

        if (delete_world)
        {
            if (type == PT_MAIN)
                PropertyAnimator::get()->clear();
            World::deleteWorld();
        }
        delete_world = false;

        StateManager::get()->enterGameState();
        setMinorMode(RaceManager::MINOR_MODE_CUTSCENE);
        int num_gp_karts = m_num_karts;
        setNumKarts(0);
        setNumPlayers(0);

        std::set<std::string> used_karts;
        if (some_human_player_well_ranked)
        {
            startSingleRace("gpwin", 999,
                                  raceWasStartedFromOverworld());
            GrandPrixWin* scene = GrandPrixWin::getInstance();
            scene->push();
            scene->setNumGPKarts(num_gp_karts); // This must be set before we set karts
            scene->setKarts(winners);
            scene->setPlayerWon(some_human_player_won);
            std::set<std::string> karts;
            for (auto& kart : winners)
                used_karts.insert(kart.first);
        }
        else
        {
            startSingleRace("gplose", 999,
                                  raceWasStartedFromOverworld());
            GrandPrixLose* scene = GrandPrixLose::getInstance();
            scene->push();

            if (humanLosers.size() >= 1)
            {
                scene->setKarts(humanLosers);
                for (auto& kart : humanLosers)
                    used_karts.insert(kart.first);
            }
            else
            {
                Log::error("RaceManager", "There are no winners and no losers."
                           "This should have never happened\n");
                std::vector<std::pair<std::string, float> > karts;
                karts.emplace_back(UserConfigParams::m_default_kart, 0.0f);
                used_karts.insert(UserConfigParams::m_default_kart);
                scene->setKarts(karts);
            }
        }

        kart_properties_manager->onDemandLoadKartTextures(used_karts);
    }

    if (delete_world)
    {
        if (type == PT_MAIN)
            PropertyAnimator::get()->clear();
        World::deleteWorld();
    }

    // Reload track screenshot after delete_world (track textures are unloaded)
    track_manager->onDemandLoadTrackScreenshots();
    m_saved_gp = NULL;
    m_track_number = 0;

    RichPresenceNS::RichPresence::get()->update(true);
}   // exitRace

//---------------------------------------------------------------------------------------------
/** A kart has finished the race at the specified time (which can be
 *  different from World::getWorld()->getClock() in case of setting
 *  extrapolated arrival times). This function is only called from
 *  kart::finishedRace()
 *  \param kart The kart that finished the race.
 *  \param time Time at which the kart finished the race.
 */
void RaceManager::kartFinishedRace(const AbstractKart *kart, float time)
{
    unsigned int id = kart->getWorldKartId();
    int pos = kart->getPosition();

    assert(pos-1 >= 0);
    assert(pos-1 < (int)m_kart_status.size());

    m_kart_status[id].m_last_score    = m_kart_status[id].m_score;

    // In follow the leader mode, the winner is actually the kart with
    // position 2, so adjust the points (#points for leader do not matter)
    WorldWithRank *wwr = dynamic_cast<WorldWithRank*>(World::getWorld());
    if (wwr)
        m_kart_status[id].m_score += wwr->getScoreForPosition(pos);
    else
    {
        Log::error("RaceManager",
                   "World with scores that is not a WorldWithRank??");
    }

    m_kart_status[id].m_overall_time += time;
    m_kart_status[id].m_last_time     = time;
    m_num_finished_karts ++;
    if(kart->getController()->isPlayerController())
        m_num_finished_players++;
}   // kartFinishedRace

//---------------------------------------------------------------------------------------------
/** \brief Rerun the same race again
 * This is called after a race is finished, and it will adjust
 * the number of points and the overall time before restarting the race.
 */
void RaceManager::rerunRace()
{
    // Subtract last score from all karts:
    for(int i=0; i<m_num_karts; i++)
    {
        m_kart_status[i].m_score         = m_kart_status[i].m_last_score;
        m_kart_status[i].m_overall_time -= m_kart_status[i].m_last_time;
    }
    World::getWorld()->reset(true /* restart */);
}   // rerunRace

//---------------------------------------------------------------------------------------------
/** \brief Higher-level method to start a GP without having to care about
 *  the exact startup sequence
 */
void RaceManager::startGP(const GrandPrixData &gp, bool from_overworld,
                          bool continue_saved_gp)
{
    StateManager::get()->enterGameState();
    setGrandPrix(gp);
    setupPlayerKartInfo();
    m_continue_saved_gp = continue_saved_gp;
    if (!continue_saved_gp)
        m_skipped_tracks_in_gp = 0;

    setMajorMode(RaceManager::MAJOR_MODE_GRAND_PRIX);
    startNew(from_overworld);
}

//---------------------------------------------------------------------------------------------
/** \brief Higher-level method to start a GP without having to care about
 *  the exact startup sequence.
 * \param trackIdent Internal name of the track to race on
 * \param num_laps   Number of laps to race, or -1 if number of laps is
 *        not relevant in current mode
 * \param from_overworld If it was started from the Story Mode overworld
 */
void RaceManager::startSingleRace(const std::string &track_ident,
                                  const int num_laps,
                                  bool from_overworld)
{
    assert(!m_watching_replay);
    StateManager::get()->enterGameState();

    // In networking, make sure that the tracks screen is shown. This will
    // allow for a 'randomly pick track' animation to be shown while
    // world is loaded.
    // Disable until render gui during loading is bug free
    /*if (NetworkConfig::get()->isNetworking() &&
        NetworkConfig::get()->isClient()        )
    {
        // TODO: The enterGameState() call above deleted all GUIs, which
        // means even if the tracks screen is shown, it need to be recreated.
        // And we have to make sure that it is recreated as network version.
        TracksScreen *ts = TracksScreen::getInstance();
        if (GUIEngine::getCurrentScreen() != ts)
        {
            ts->setNetworkTracks();
            ts->push();
        }
    }*/


    setTrack(track_ident);

    if (num_laps != -1) setNumLaps( num_laps );

    setMajorMode(RaceManager::MAJOR_MODE_SINGLE);

    setCoinTarget( 0 ); // Might still be set from a previous challenge

    // if not in a network world, setup player karts
    if (!NetworkConfig::get()->isNetworking())
        setupPlayerKartInfo(); // do this setup player kart

    startNew(from_overworld);
}   // startSingleRace

//---------------------------------------------------------------------------------------------
/** Fills up the remaining kart slots with AI karts.
 */
void RaceManager::setupPlayerKartInfo()
{
    computeRandomKartList();
}   // setupPlayerKartInfo

//---------------------------------------------------------------------------------------------
/** \brief Function to start the race with only ghost kart(s) and watch.
 * \param trackIdent Internal name of the track to race on
 * \param num_laps   Number of laps to race, or -1 if number of laps is
 *        not relevant in current mode
 */
void RaceManager::startWatchingReplay(const std::string &track_ident,
                                      const int num_laps)
{
    assert(m_watching_replay && m_has_ghost_karts && !m_is_recording_race);
    StateManager::get()->enterGameState();
    setTrack(track_ident);
    setNumLaps(num_laps);
    setMajorMode(RaceManager::MAJOR_MODE_SINGLE);
    setCoinTarget(0);
    m_num_karts = ReplayPlay::get()->getNumGhostKart();
    m_kart_status.clear();

    Log::verbose("RaceManager", "%u ghost kart(s) for watching replay only\n",
        (unsigned int)m_num_karts);

    int init_gp_rank = 0;

    for(int i = 0; i < m_num_karts; i++)
    {
        m_kart_status.push_back(KartStatus(ReplayPlay::get()->getGhostKartName(i),
            i, -1, -1, init_gp_rank, KT_GHOST, HANDICAP_NONE));
        init_gp_rank ++;
    }

    m_track_number = 0;
    startNextRace();
}   // startWatchingReplay

//---------------------------------------------------------------------------------------------
void RaceManager::configGrandPrixResultFromNetwork(NetworkString& ns)
{
    setMajorMode(MAJOR_MODE_GRAND_PRIX);
    class NetworkGrandPrixData : public GrandPrixData
    {
    public:
        NetworkGrandPrixData() : GrandPrixData()
            { setGroup(GrandPrixData::GP_STANDARD); }
        virtual std::vector<std::string>
            getTrackNames(const bool includeLocked=false) const
            { return m_tracks; }
        virtual unsigned int
            getNumberOfTracks(const bool includeLocked=false) const
            { return (unsigned int)m_tracks.size(); }
        void addNetworkTrack(const std::string& t) { m_tracks.push_back(t); }
    };

    NetworkGrandPrixData ngpd;
    unsigned int track_size = ns.getUInt8();
    unsigned int all_track_size = ns.getUInt8();
    assert(all_track_size > 0);
    m_track_number = all_track_size -1;
    for (unsigned i = 0; i < all_track_size; i++)
    {
        std::string t;
        ns.decodeString(&t);
        ngpd.addNetworkTrack(t);
    }
    while (all_track_size < track_size)
    {
        ngpd.addNetworkTrack("");
        all_track_size++;
    }

    m_tracks = ngpd.getTrackNames();
    // For result screen we only need current lap and reserve
    m_num_laps.resize(track_size, m_num_laps[0]);
    m_reverse_track.resize(track_size, m_reverse_track[0]);

    m_grand_prix = ngpd;
    unsigned int player_size = ns.getUInt8();
    assert(player_size == m_kart_status.size());
    for (unsigned i = 0; i < player_size; i++)
    {
        int last_score = ns.getUInt32();
        int cur_score = ns.getUInt32();
        float overall_time = ns.getFloat();
        m_kart_status[i].m_last_score = last_score;
        m_kart_status[i].m_score = cur_score;
        m_kart_status[i].m_overall_time = overall_time;
        m_kart_status[i].m_gp_rank = i;
    }
}   // configGrandPrixResultFromNetwork

//---------------------------------------------------------------------------------------------
void RaceManager::clearNetworkGrandPrixResult()
{
    if (m_major_mode != MAJOR_MODE_GRAND_PRIX)
        return;
    setMajorMode(MAJOR_MODE_SINGLE);
    m_grand_prix = GrandPrixData();
    m_track_number = 0;
    m_tracks.clear();
    m_num_laps.clear();
    m_reverse_track.clear();

}   // clearNetworkGrandPrixResult

//---------------------------------------------------------------------------------------------
/** Returns a (translated) name of a minor race mode.
 *  \param mode Minor race mode.
 */
const core::stringw RaceManager::getNameOf(const MinorRaceModeType mode)
{
    switch (mode)
    {
        //I18N: Game mode
        case MINOR_MODE_NORMAL_RACE:    return _("Normal Race");
        //I18N: Game mode
        case MINOR_MODE_TIME_TRIAL:     return _("Time Trial");
        //I18N: Game mode
        case MINOR_MODE_FOLLOW_LEADER:  return _("Follow the Leader");
        //I18N: Game mode
        case MINOR_MODE_LAP_TRIAL:      return _("Lap Trial");
        //I18N: Game mode
        case MINOR_MODE_3_STRIKES:      return _("3 Strikes Battle");
        //I18N: Game mode
        case MINOR_MODE_FREE_FOR_ALL:   return _("Free-For-All");
        //I18N: Game mode
        case MINOR_MODE_CAPTURE_THE_FLAG: return _("Capture The Flag");
        //I18N: Game mode
        case MINOR_MODE_EASTER_EGG:     return _("Egg Hunt");
        //I18N: Game mode
        case MINOR_MODE_SOCCER:         return _("Soccer");
        default: return L"";
    }
}   // getNameOf

//---------------------------------------------------------------------------------------------
/** Returns the specified difficulty as a string. */
core::stringw RaceManager::getDifficultyName(Difficulty diff) const
{
    switch (diff)
    {
        case RaceManager::DIFFICULTY_EASY:   return _("Novice");   break;
        case RaceManager::DIFFICULTY_MEDIUM: return _("Intermediate"); break;
        case RaceManager::DIFFICULTY_HARD:   return _("Expert");   break;
        case RaceManager::DIFFICULTY_BEST:   return _("SuperTux");   break;
        default:    Log::error("RaceManager", "Difficulty level '%u' is unknown.", diff);
                    // Uncomment to generate a crash and backtrace, if the cause of the
                    // incorrect difficulty level is unknown (i.e. not online servers' settings)
                    // assert(false);
    }
    return "";
}   // getDifficultyName
