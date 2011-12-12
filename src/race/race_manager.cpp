//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2006 SuperTuxKart-Team
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
#include "config/user_config.hpp"
#include "config/stk_config.hpp"
#include "input/device_manager.hpp"
#include "input/input_manager.hpp"
#include "karts/kart_properties_manager.hpp"
#include "modes/follow_the_leader.hpp"
#include "modes/profile_world.hpp"
#include "modes/standard_race.hpp"
#include "modes/world.hpp"
#include "modes/three_strikes_battle.hpp"
#include "network/network_manager.hpp"
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
    m_num_karts          = UserConfigParams::m_num_karts;
    m_difficulty         = RD_HARD;
    m_major_mode         = MAJOR_MODE_SINGLE;
    m_minor_mode         = MINOR_MODE_NORMAL_RACE;
    m_track_number       = 0;
    m_coin_target        = 0;
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
            printf("Kart '%s' is unknown and therefore ignored.\n",
                   name.c_str());
            continue;
        }
        // This doesn't work anymore, since this is called when
        // handling the command line options, at which time the
        // player (and therefore the current slot) is not defined yet.
        //if(unlock_manager->getCurrentSlot()->isLocked(name)) 
        //{
        //   printf("Kart '%s' is locked and therefore ignored.\n",
        //           name.c_str());
        //    continue;
        //}
        m_default_ai_list.push_back(name);
    }
}   // setAIKartList

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
    assert(0<=player_id && player_id <m_local_player_karts.size());
    assert(kart_properties_manager->getKart(kart) != NULL);

    m_local_player_karts[player_id] = RemoteKartInfo(player_id, kart,
                                                  StateManager::get()->getActivePlayerProfile(player_id)->getName(),
                                                  network_manager->getMyHostId());
}   // setLocalKartInfo

//-----------------------------------------------------------------------------
/** Returns a pointer to the kart which has a given GP rank.
 *  \param n The rank (1 to number of karts) to look for.
 */
const Kart *RaceManager::getKartWithGPRank(unsigned int n)
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
    const int amount = m_kart_status.size();
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
    int n = m_num_karts - m_player_karts.size();
    if(UserConfigParams::logMisc())
        std::cout << "AI karts count = " << n << " for m_num_karts=" 
                  << m_num_karts << " and m_player_karts.size()=" 
                  << m_player_karts.size() << std::endl;

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

}   // computeRandomKartList

//-----------------------------------------------------------------------------

void RaceManager::startNew()
{
    if(m_major_mode==MAJOR_MODE_GRAND_PRIX)   // GP: get tracks and laps from grand prix
    {
        m_tracks   = m_grand_prix.getTracks();
        m_num_laps = m_grand_prix.getLaps();
    }
    assert(m_player_karts.size() > 0);

    // command line parameters: negative numbers=all karts
    if(m_num_karts < 0 ) m_num_karts = stk_config->m_max_karts;
    if((size_t)m_num_karts < m_player_karts.size()) 
        m_num_karts = (int)m_player_karts.size();

    // Create the kart status data structure to keep track of scores, times, ...
    // ==========================================================================
    m_kart_status.clear();

    // Before thi
    assert((unsigned int)m_num_karts == m_ai_kart_list.size()+m_player_karts.size());
    // First add the AI karts (randomly chosen)
    // ----------------------------------------

    // GP ranks start with -1 for the leader.
    int init_gp_rank = 
        race_manager->getMinorMode()==RaceManager::MINOR_MODE_FOLLOW_LEADER 
        ? -1 
        : 0;
    const unsigned int ai_kart_count = m_ai_kart_list.size();
    for(unsigned int i=0; i<ai_kart_count; i++)
    {
        m_kart_status.push_back(KartStatus(m_ai_kart_list[i], i, -1, -1, 
                                           init_gp_rank, KT_AI));
        init_gp_rank ++;
        if(UserConfigParams::m_ftl_debug)
        {
            printf("[ftl] rank %d ai-kart %s\n", init_gp_rank, 
                   m_ai_kart_list[i].c_str());
        }
    }

    // Then the players, which start behind the AI karts
    // -------------------------------------------------
    for(int i=m_player_karts.size()-1; i>=0; i--)
    {
        KartType kt=(m_player_karts[i].getHostId()==network_manager->getMyHostId())
                   ? KT_PLAYER : KT_NETWORK_PLAYER;
        m_kart_status.push_back(KartStatus(m_player_karts[i].getKartName(), i,
                                           m_player_karts[i].getLocalPlayerId(),
                                           m_player_karts[i].getGlobalPlayerId(),
                                           init_gp_rank, kt
                                           ) );
        if(UserConfigParams::m_ftl_debug)
        {
            printf("[ftl] rank %d kart %s\n", init_gp_rank, 
                m_player_karts[i].getKartName().c_str());
        }
        init_gp_rank ++;
    }

    // Then start the race with the first track
    // ========================================
    m_track_number = 0;
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

        std::sort(m_kart_status.begin()+offset, m_kart_status.end());
        //reverse kart order if flagged in stk_config
        if (stk_config->m_gp_order)
        {
            std::reverse(m_kart_status.begin()+offset, m_kart_status.end());
        } 
    }   // not first race

    // the constructor assigns this object to the global
    // variable world. Admittedly a bit ugly, but simplifies
    // handling of objects which get created in the constructor
    // and need world to be defined.
    if     (ProfileWorld::isProfileMode())            
        World::setWorld(new ProfileWorld());
    else if(m_minor_mode==MINOR_MODE_FOLLOW_LEADER) 
        World::setWorld(new FollowTheLeaderRace());
    else if(m_minor_mode==MINOR_MODE_NORMAL_RACE || 
            m_minor_mode==MINOR_MODE_TIME_TRIAL)    
        World::setWorld(new StandardRace());
    else if(m_minor_mode==MINOR_MODE_3_STRIKES)     
        World::setWorld(new ThreeStrikesBattle());
    else
    { 
        fprintf(stderr,"Could not create given race mode\n"); 
        assert(0); 
    }
    World::getWorld()->init();
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
        if(network_manager->getMode()==NetworkManager::NW_SERVER)
            network_manager->beginReadySetGoBarrier();
        else
            network_manager->setState(NetworkManager::NS_WAIT_FOR_RACE_DATA);
        startNextRace();
    }
    else
    {
        // Back to main menu. Change the state of the state of the
        // network manager.
        if(network_manager->getMode()==NetworkManager::NW_SERVER)
            network_manager->setState(NetworkManager::NS_MAIN_MENU);
        else
            network_manager->setState(NetworkManager::NS_WAIT_FOR_AVAILABLE_CHARACTERS);
        exitRace();
    }
}   // next

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
            printf("[ftl] kart '%s' has position %d.\n",
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
            printf("[ftl] kart '%s' has position %d score %d.\n",
                World::getWorld()->getKart(kart_id)->getIdent().c_str(),
                sd->m_position, sd->m_score);
        }
    }
    
    sort_data.insertionSort(start);
    for (unsigned int i=start; i < NUM_KARTS; ++i)
    {
        //printf("setting kart %s to rank %i\n", 
        //    m_kart_status[position[i]].m_ident.c_str(), i-start);
        if(UserConfigParams::m_ftl_debug)
        {
            const Kart *kart =
                World::getWorld()->getKart(sort_data[i].m_position);
            printf("[ftl] kart '%s' has now position %d.\n",
                kart->getIdent().c_str(), 
                i-start);
        }

        m_kart_status[sort_data[i].m_position].m_gp_rank = i - start;
    }
    // printf("kart %s has rank %i\n", 0, m_kart_status[0].m_gp_rank);
}   // computeGPRanks

//-----------------------------------------------------------------------------

void RaceManager::exitRace()
{
    // Only display the grand prix result screen if all tracks 
    // were finished, and not when a race is aborted.
    if (m_major_mode==MAJOR_MODE_GRAND_PRIX && m_track_number==(int)m_tracks.size()) 
    {
        unlock_manager->getCurrentSlot()->grandPrixFinished();
        
        StateManager::get()->resetAndGoToScreen( MainMenuScreen::getInstance() );
        

        bool someHumanPlayerWon = false;
        const unsigned int kartStatusCount = m_kart_status.size();
        
        const int loserThreshold = 3;
        
        std::string winners[3];
        std::vector<std::string> humanLosers; // because we don't care about AIs that lost
        for (unsigned int i=0; i < kartStatusCount; ++i)
        {
            if(UserConfigParams::logMisc())
            {
                std::cout << m_kart_status[i].m_ident << " has GP final rank "
                          << m_kart_status[i].m_gp_rank << std::endl;
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
        
        if (someHumanPlayerWon)
        {
            StateManager::get()->pushScreen( GrandPrixWin::getInstance()  );
            GrandPrixWin::getInstance()->setKarts(winners);
        }
        else
        {
            StateManager::get()->pushScreen( GrandPrixLose::getInstance()  );
            
            if (humanLosers.size() >= 1)
            {
                GrandPrixLose::getInstance()->setKarts( humanLosers );
            }
            else
            {
                std::cerr << "RaceManager::exitRace() : what's going on?? no winners and no losers??\n";
                std::vector<std::string> karts;
                karts.push_back(UserConfigParams::m_default_kart);
                GrandPrixLose::getInstance()->setKarts( karts );
            }
        }
    }

    World::deleteWorld();
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
void RaceManager::kartFinishedRace(const Kart *kart, float time)
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
    if(kart->getController()->isPlayerController()) m_num_finished_players++;
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
    World::getWorld()->restartRace();
}   // rerunRace

//-----------------------------------------------------------------------------

void RaceManager::startGP(const GrandPrixData* gp)
{
    assert(gp != NULL);

    StateManager::get()->enterGameState();
    setGrandPrix(*gp);
    setCoinTarget( 0 ); // Might still be set from a previous challenge
    network_manager->setupPlayerKartInfo();
    
    setMajorMode(RaceManager::MAJOR_MODE_GRAND_PRIX);
    startNew();
}

//-----------------------------------------------------------------------------

void RaceManager::startSingleRace(const std::string trackIdent, const int num_laps)
{
    StateManager::get()->enterGameState();
    setTrack(trackIdent.c_str());
    
    if (num_laps != -1) setNumLaps( num_laps );
    
    setMajorMode(RaceManager::MAJOR_MODE_SINGLE);
    
    setCoinTarget( 0 ); // Might still be set from a previous challenge
    network_manager->setupPlayerKartInfo();
    
    startNew();
}

/* EOF */
