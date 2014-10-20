//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2006-2013 SuperTuxKart-Team
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

#include "challenges/unlock_manager.hpp"
#include "config/player_manager.hpp"
#include "config/saved_grand_prix.hpp"
#include "config/stk_config.hpp"
#include "config/user_config.hpp"
#include "graphics/irr_driver.hpp"
#include "input/device_manager.hpp"
#include "input/input_manager.hpp"
#include "karts/abstract_kart.hpp"
#include "karts/controller/controller.hpp"
#include "karts/kart_properties_manager.hpp"
#include "modes/cutscene_world.hpp"
#include "modes/demo_world.hpp"
#include "modes/easter_egg_hunt.hpp"
#include "modes/follow_the_leader.hpp"
#include "modes/overworld.hpp"
#include "modes/profile_world.hpp"
#include "modes/standard_race.hpp"
#include "modes/tutorial_world.hpp"
#include "modes/world.hpp"
#include "modes/three_strikes_battle.hpp"
#include "modes/soccer_world.hpp"
#include "network/protocol_manager.hpp"
#include "network/network_world.hpp"
#include "network/protocols/start_game_protocol.hpp"
#include "states_screens/grand_prix_cutscene.hpp"
#include "states_screens/grand_prix_lose.hpp"
#include "states_screens/grand_prix_win.hpp"
#include "states_screens/kart_selection.hpp"
#include "states_screens/main_menu_screen.hpp"
#include "states_screens/state_manager.hpp"
#include "tracks/track_manager.hpp"
#include "utils/ptr_vector.hpp"

RaceManager* race_manager= NULL;

/** Constructs the race manager.
 */
RaceManager::RaceManager()
{
    // Several code depends on this, e.g. kart_properties
    assert(DIFFICULTY_FIRST == 0);
    m_num_karts          = UserConfigParams::m_num_karts;
    m_difficulty         = DIFFICULTY_HARD;
    m_major_mode         = MAJOR_MODE_SINGLE;
    m_minor_mode         = MINOR_MODE_NORMAL_RACE;
    m_ai_superpower      = SUPERPOWER_NONE;
    m_track_number       = 0;
    m_coin_target        = 0;
    m_started_from_overworld = false;
    m_have_kart_last_position_on_overworld = false;
    setReverseTrack(false);
    setTrack("jungle");
    m_default_ai_list.clear();
    setNumLocalPlayers(0);
}   // RaceManager

//-----------------------------------------------------------------------------
/** Destructor for the race manager.
 */
RaceManager::~RaceManager()
{
}   // ~RaceManager

//-----------------------------------------------------------------------------
/** Resets the race manager in preparation for a new race. It sets the
 *  counter of finished karts to zero.
 */
void RaceManager::reset()
{
    m_num_finished_karts   = 0;
    m_num_finished_players = 0;
}  // reset

//-----------------------------------------------------------------------------
/** \brief Sets a player kart (local and non-local).
 *  \param player_id  Id of the player.
 *  \param ki         Kart info structure for this player.
 */
void RaceManager::setPlayerKart(unsigned int player_id, const RemoteKartInfo& ki)
{
    m_player_karts[player_id] = ki;
}   // setPlayerKart

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

// ----------------------------------------------------------------------------
/** Sets information about a kart used by a local player (i.e. on this
 *  computer).
 *  \param player_id  Id of this player.
 *  \param kart The kart this player uses.
 */
void RaceManager::setLocalKartInfo(unsigned int player_id,
                                   const std::string& kart)
{
    assert(kart.size() > 0);
    assert(player_id <m_local_player_karts.size());
    assert(kart_properties_manager->getKart(kart) != NULL);

    const PlayerProfile* profile = StateManager::get()->getActivePlayerProfile(player_id);
    m_local_player_karts[player_id] = RemoteKartInfo(player_id, kart,
                                                  profile->getName(),
                                                    0, false);
}   // setLocalKartInfo

//-----------------------------------------------------------------------------
/** Sets additional information for a player to indicate which soccer team it belong to
*/
void RaceManager::setLocalKartSoccerTeam(unsigned int player_id, SoccerTeam team)
{
    assert(player_id <m_local_player_karts.size());

    m_local_player_karts[player_id].setSoccerTeam(team);
}

//-----------------------------------------------------------------------------
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

//-----------------------------------------------------------------------------
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

//-----------------------------------------------------------------------------
/** Sets the number of local players, i.e. the number of players on this
 *  computer.
 *  \param n Number of local players.
 */
void RaceManager::setNumLocalPlayers(unsigned int n)
{
    m_local_player_karts.resize(n);
}   // setNumLocalPlayers

//-----------------------------------------------------------------------------
/** Sets the number of players.
 *  \param num Number of players.
 */
void RaceManager::setNumPlayers(int num)
{
    m_player_karts.resize(num);
}   // setNumPlayers

//-----------------------------------------------------------------------------
/** Sets the difficulty to use.
 *  \param diff The difficulty to use.
 */
void RaceManager::setDifficulty(Difficulty diff)
{
    m_difficulty = diff;
}   // setDifficulty

//-----------------------------------------------------------------------------
/** Sets a single track to be used in the next race.
 *  \param track The identifier of the track to use.
 */
void RaceManager::setTrack(const std::string& track)
{
    m_tracks.clear();
    m_tracks.push_back(track);

    m_coin_target = 0;
}   // setTrack

//-----------------------------------------------------------------------------
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
    unsigned int m = std::min( (unsigned) n,  (unsigned)m_default_ai_list.size());

    for(unsigned int i=0; i<m; i++)
    {
        m_ai_kart_list.push_back(m_default_ai_list[i]);
        n--;
    }

    if(n>0)
        kart_properties_manager->getRandomKartList(n, m_player_karts,
                                                   &m_ai_kart_list   );

    if (m_ai_kart_override != "")
    {
        for (unsigned int n = 0; n < m_ai_kart_list.size(); n++)
        {
            m_ai_kart_list[n] = m_ai_kart_override;
        }
    }

}   // computeRandomKartList

//-----------------------------------------------------------------------------

void RaceManager::startNew(bool from_overworld)
{
    m_started_from_overworld = from_overworld;
    m_saved_gp = NULL; // There will be checks for this being NULL done later

    if (m_major_mode==MAJOR_MODE_GRAND_PRIX)
    {
        // GP: get tracks, laps and reverse info from grand prix
        m_tracks        = m_grand_prix.getTrackNames();
        m_num_laps      = m_grand_prix.getLaps();
        m_reverse_track = m_grand_prix.getReverse();

        // We look if Player 1 has a saved version of this GP.
        m_saved_gp = SavedGrandPrix::getSavedGP(
                                     StateManager::get()
                                     ->getActivePlayerProfile(0)
                                     ->getUniqueID(),
                                     m_grand_prix.getId(),
                                     m_player_karts.size());

        // Saved GP only in offline mode
        if (m_continue_saved_gp && !NetworkWorld::getInstance<NetworkWorld>()->isRunning())
        {
            if (m_saved_gp == NULL)
            {
                Log::error("Race Manager", "Can not continue Grand Prix '%s'"
                                           "because it could not exist",
                                           m_grand_prix.getId().c_str());
                m_continue_saved_gp = false; // simple and working
            }
            else
            {
                setNumKarts(m_saved_gp->getTotalKarts());
                setupPlayerKartInfo();
                m_grand_prix.changeReverse((GrandPrixData::GPReverseType)m_saved_gp->getReverseType());
                m_reverse_track = m_grand_prix.getReverse();
            }
        }
    }

    // command line parameters: negative numbers=all karts
    if(m_num_karts < 0 ) m_num_karts = stk_config->m_max_karts;
    if((size_t)m_num_karts < m_player_karts.size())
        m_num_karts = (int)m_player_karts.size();

    // Create the kart status data structure to keep track of scores, times, ...
    // ==========================================================================
    m_kart_status.clear();
    Log::verbose("RaceManager", "Nb of karts=%u, ai:%lu players:%lu\n", (unsigned int)m_num_karts, m_ai_kart_list.size(), m_player_karts.size());

    assert((unsigned int)m_num_karts == m_ai_kart_list.size()+m_player_karts.size());
    // First add the AI karts (randomly chosen)
    // ----------------------------------------

    // GP ranks start with -1 for the leader.
    int init_gp_rank =
        race_manager->getMinorMode()==RaceManager::MINOR_MODE_FOLLOW_LEADER
        ? -1
        : 0;
    const unsigned int ai_kart_count = m_ai_kart_list.size();
    for(unsigned int i = 0; i < ai_kart_count; i++)
    {
        m_kart_status.push_back(KartStatus(m_ai_kart_list[i], i, -1, -1,
                                           init_gp_rank, KT_AI));
        init_gp_rank ++;
        if(UserConfigParams::m_ftl_debug)
        {
            Log::debug("RaceManager", "[ftl] rank %d ai-kart %s", init_gp_rank,
                   m_ai_kart_list[i].c_str());
        }
    }

    // Then the players, which start behind the AI karts
    // -------------------------------------------------
    for(unsigned int i = 0; i < m_player_karts.size(); i++)
    {
        KartType kt= m_player_karts[i].isNetworkPlayer() ? KT_NETWORK_PLAYER : KT_PLAYER;
        m_kart_status.push_back(KartStatus(m_player_karts[i].getKartName(), i,
                                           m_player_karts[i].getLocalPlayerId(),
                                           m_player_karts[i].getGlobalPlayerId(),
                                           init_gp_rank, kt
                                           ) );
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
            m_track_number  = m_saved_gp->getNextTrack();
            m_saved_gp->loadKarts(m_kart_status);
        }
        else if (m_saved_gp != NULL)
        {
            m_saved_gp->remove();
            m_saved_gp = NULL;
        }
    }

    startNextRace();
}   // startNew

//-----------------------------------------------------------------------------
/** \brief Starts the next (or first) race.
 *  It sorts the kart status data structure
 *  according to the number of points, and then creates the world().
 */
void RaceManager::startNextRace()
{
    // Uncomment to debug audio leaks
    // sfx_manager->dump();

    stk_config->getAllScores(&m_score_for_position, m_num_karts);
    IrrlichtDevice* device = irr_driver->getDevice();
    GUIEngine::renderLoading();
    device->getVideoDriver()->endScene();
    device->getVideoDriver()->beginScene(true, true, video::SColor(255,100,101,140));


    m_num_finished_karts   = 0;
    m_num_finished_players = 0;

    // if subsequent race, sort kart status structure
    // ==============================================
    if (m_track_number > 0)
    {
        // In follow the leader mode do not change the first kart,
        // since it's always the leader.
        int offset = (m_minor_mode==MINOR_MODE_FOLLOW_LEADER) ? 1 : 0;

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
    else if(m_minor_mode==MINOR_MODE_TUTORIAL)
        World::setWorld(new TutorialWorld());
    else if(m_minor_mode==MINOR_MODE_3_STRIKES)
        World::setWorld(new ThreeStrikesBattle());
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

    // A second constructor phase is necessary in order to be able to
    // call functions which are overwritten (otherwise polymorphism
    // will fail and the results will be incorrect . Also in init() functions
    // can be called that use World::getWorld().
    World::getWorld()->init();

    // Now initialise all values that need to be reset from race to race
    // Calling this here reduces code duplication in init and restartRace()
    // functions.
    World::getWorld()->reset();

    irr_driver->onLoadWorld();

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

    StartGameProtocol* protocol = static_cast<StartGameProtocol*>(
            ProtocolManager::getInstance()->getProtocol(PROTOCOL_START_GAME));
    if (protocol)
        protocol->ready();
}   // startNextRace

//-----------------------------------------------------------------------------
void RaceManager::next()
{
    World::deleteWorld();
    m_num_finished_karts   = 0;
    m_num_finished_players = 0;
    m_track_number++;
    if(m_track_number<(int)m_tracks.size())
    {
        if(m_major_mode==MAJOR_MODE_GRAND_PRIX && !NetworkWorld::getInstance()->isRunning())
        {
            // Saving GP state
            // If Player 1 has already saved a GP, we adapt it
            if(m_saved_gp != NULL)
            {
                m_saved_gp->setKarts(m_kart_status);
                m_saved_gp->setNextTrack(m_track_number);
                user_config->saveConfig();
            }
            else
            {
                saveGP();
            }
        }
        startNextRace();
    }
    else
    {
        exitRace();
    }
}   // next

//-----------------------------------------------------------------------------
void RaceManager::saveGP()
{
    UserConfigParams::m_saved_grand_prix_list.push_back(
        new SavedGrandPrix(
            StateManager::get()->getActivePlayerProfile(0)
                               ->getUniqueID(),
            m_grand_prix.getId(),
            m_difficulty,
            (int)m_player_karts.size(),
            m_track_number,
            m_grand_prix.getReverseType(),
            m_kart_status
        )
    );
    user_config->saveConfig();
}

//-----------------------------------------------------------------------------

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
    int start=(race_manager->getMinorMode()==RaceManager::MINOR_MODE_FOLLOW_LEADER);
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
            Log::debug("Race Manager","[ftl] kart '%s' has position %d score %d.",
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

//-----------------------------------------------------------------------------

void RaceManager::exitRace(bool delete_world)
{
    // Only display the grand prix result screen if all tracks
    // were finished, and not when a race is aborted.
    if (m_major_mode==MAJOR_MODE_GRAND_PRIX && m_track_number==(int)m_tracks.size())
    {
        PlayerManager::getCurrentPlayer()->grandPrixFinished();
        if(m_major_mode==MAJOR_MODE_GRAND_PRIX&& !NetworkWorld::getInstance()->isRunning())
        {
            if(m_saved_gp != NULL)
                m_saved_gp->remove();
        }
        StateManager::get()->resetAndGoToScreen( MainMenuScreen::getInstance() );

        bool someHumanPlayerWon = false;
        const unsigned int kart_status_count = (unsigned int)m_kart_status.size();

        const int loserThreshold = 3;

        std::string winners[3];
        std::vector<std::string> humanLosers; // because we don't care about AIs that lost
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
                winners[rank] = m_kart_status[i].m_ident;
                if (m_kart_status[i].m_kart_type == KT_PLAYER ||
                    m_kart_status[i].m_kart_type == KT_NETWORK_PLAYER)
                {
                    someHumanPlayerWon = true;
                }
            }
            else if (rank >= loserThreshold)
            {
                if (m_kart_status[i].m_kart_type == KT_PLAYER ||
                    m_kart_status[i].m_kart_type == KT_NETWORK_PLAYER)
                {
                    humanLosers.push_back(m_kart_status[i].m_ident);
                }
            }
        }

        if (delete_world) World::deleteWorld();
        delete_world = false;

        StateManager::get()->enterGameState();
        race_manager->setMinorMode(RaceManager::MINOR_MODE_CUTSCENE);
        race_manager->setNumKarts(0);
        race_manager->setNumPlayers(0);
        race_manager->setNumLocalPlayers(0);

        if (someHumanPlayerWon)
        {
            race_manager->startSingleRace("gpwin", 999, false);
            GrandPrixWin* scene = GrandPrixWin::getInstance();
            scene->push();
            scene->setKarts(winners);
        }
        else
        {
            race_manager->startSingleRace("gplose", 999, false);
            GrandPrixLose* scene = GrandPrixLose::getInstance();
            scene->push();

            if (humanLosers.size() >= 1)
            {
                scene->setKarts(humanLosers);
            }
            else
            {
                Log::error("RaceManager", "There are no winners and no losers."
                           "This should have never happend\n");
                std::vector<std::string> karts;
                karts.push_back(UserConfigParams::m_default_kart);
                scene->setKarts(karts);
            }
        }
    }

    if (delete_world) World::deleteWorld();

    m_saved_gp = NULL;
    m_track_number = 0;
}   // exitRace

//-----------------------------------------------------------------------------
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
    m_kart_status[id].m_score        += m_score_for_position[pos-1];
    m_kart_status[id].m_overall_time += time;
    m_kart_status[id].m_last_time     = time;
    m_num_finished_karts ++;
    if(kart->getController()->isPlayerController() ||
        kart->getController()->isNetworkController())
        m_num_finished_players++;
}   // kartFinishedRace

//-----------------------------------------------------------------------------

void RaceManager::rerunRace()
{
    // Subtract last score from all karts:
    for(int i=0; i<m_num_karts; i++)
    {
        m_kart_status[i].m_score         = m_kart_status[i].m_last_score;
        m_kart_status[i].m_overall_time -= m_kart_status[i].m_last_time;
    }
    World::getWorld()->reset();
}   // rerunRace

//-----------------------------------------------------------------------------

void RaceManager::startGP(const GrandPrixData &gp, bool from_overworld,
                          bool continue_saved_gp)
{
    StateManager::get()->enterGameState();
    setGrandPrix(gp);
    race_manager->setupPlayerKartInfo();
    m_continue_saved_gp = continue_saved_gp;

    setMajorMode(RaceManager::MAJOR_MODE_GRAND_PRIX);
    startNew(from_overworld);
}

//-----------------------------------------------------------------------------
/** \brief Higher-level method to start a GP without having to care about
 *  the exact startup sequence.
 * \param trackIdent Internal name of the track to race on
 * \param num_laps   Number of laps to race, or -1 if number of laps is
 *        not relevant in current mode
 */
void RaceManager::startSingleRace(const std::string &track_ident,
                                  const int num_laps,
                                  bool from_overworld)
{
    StateManager::get()->enterGameState();
    setTrack(track_ident.c_str());

    if (num_laps != -1) setNumLaps( num_laps );

    setMajorMode(RaceManager::MAJOR_MODE_SINGLE);

    setCoinTarget( 0 ); // Might still be set from a previous challenge
    if (!NetworkWorld::getInstance<NetworkWorld>()->isRunning()) // if not in a network world
        race_manager->setupPlayerKartInfo(); // do this setup player kart

    startNew(from_overworld);
}

//-----------------------------------------------------------------------------
/** Receive and store the information from sendKartsInformation()
*/
void RaceManager::setupPlayerKartInfo()
{

    std::vector<RemoteKartInfo> kart_info;

    // Get the local kart info
    for(unsigned int i=0; i<getNumLocalPlayers(); i++)
        kart_info.push_back(getLocalKartInfo(i));

    // Now sort by (hostid, playerid)
    std::sort(kart_info.begin(), kart_info.end());

    // Set the player kart information
    setNumPlayers((int)kart_info.size());

    // Set the global player ID for each player
    for(unsigned int i=0; i<kart_info.size(); i++)
    {
        kart_info[i].setGlobalPlayerId(i);
        setPlayerKart(i, kart_info[i]);
    }

    computeRandomKartList();
}   // setupPlayerKartInfo

/* EOF */
