//  $Id$
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
    setNumLocalPlayers(0);
    //setLocalKartInfo(0, "tux");
}   // RaceManager

//-----------------------------------------------------------------------------
/** Destroys the race manager.
 */
RaceManager::~RaceManager()
{
}   // ~RaceManager

//-----------------------------------------------------------------------------
/** Resets the race manager. It is called by world when restarting a race.
 */
void RaceManager::reset()
{
    m_num_finished_karts   = 0;
    m_num_finished_players = 0;
}  // reset

//-----------------------------------------------------------------------------
/** Sets a player kart (local and non-local).
 *  \param player_id Id of the player.
 *  \param ki Kart info structure for this player.
 */
void RaceManager::setPlayerKart(unsigned int player_id, const RemoteKartInfo& ki)
{
    m_player_karts[player_id] = ki;
}   // setPlayerKart

// ----------------------------------------------------------------------------
/** Stores the information which local players uses which karts.
 *  \param player_id  Id of the local player for which the kart is set.
 *  \param kart Kart name this player is using.
 */
void RaceManager::setLocalKartInfo(unsigned int player_id, const std::string& kart)
{
    assert(0<=player_id && player_id <m_local_player_karts.size());

    m_local_player_karts[player_id] = RemoteKartInfo(player_id, kart,
                                                  StateManager::get()->getActivePlayerProfile(player_id)->getName(),
                                                  network_manager->getMyHostId());
}   // setLocalKartInfo

//-----------------------------------------------------------------------------
/** Returns the kart with a given GP rank (or NULL if no such kart exists).
 *  \param n Rank (0<=n<num_karts) to look for.
 */
const Kart *RaceManager::getKartWithGPRank(unsigned int n)
{
    for(unsigned int i=0; i<m_kart_status.size(); i++)
        if(m_kart_status[i].m_gp_rank == (int)n)
            return World::getWorld()->getKart(i);
    return NULL;
}   // getKLartWithGPRank

//-----------------------------------------------------------------------------
int RaceManager::getLocalPlayerGPRank(const int playerID) const
{
    const int amount = m_kart_status.size();
    for (int n=0; n<amount; n++)
    {
        if (m_kart_status[n].m_local_player_id == playerID)
        {
            return m_kart_status[n].m_gp_rank;
        }
    }
    return -1;
}   // getLocalPlayerGPRank

//-----------------------------------------------------------------------------
/** Sets the number of local players playing on this computer (including
 *  split screen).
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
/** Sets the difficulty.
 *  \param diff Difficulty.
 */
void RaceManager::setDifficulty(Difficulty diff)
{
    m_difficulty = diff;
}   // setDifficulty

//-----------------------------------------------------------------------------
/** In case of non GP mode set the track to use.
 *  \param track Pointer to the track to use.
 */
void RaceManager::setTrack(const std::string& track)
{
    m_tracks.clear();
    m_tracks.push_back(track);
}   // setTrack

//-----------------------------------------------------------------------------
/** Computes the list of random karts to be used for the AI.
*/
void RaceManager::computeRandomKartList()
{
    int n = m_num_karts - m_player_karts.size();
    if(UserConfigParams::m_verbosity>=5)
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
    m_ai_kart_list=kart_properties_manager->getRandomKartList(n, m_player_karts);

}   // computeRandomKartList

//-----------------------------------------------------------------------------
/** Starts a new race or GP (or other mode). It sets up the list of player
 *  karts, AI karts, laps etc., and then uses startNextRace to actually start
 *  the race.6
 * \pre The list of AI karts to use must be set up first. This is usually being
 *       done by a call to computeRandomKartList() from 
 *       NetworkManager::setupPlayerKartInfo, but could be done differently 
 *       (e.g. depending on user command line options to test certain AIs)
 */
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
    int init_gp_rank = 0;
    const unsigned int ai_kart_count = m_ai_kart_list.size();
    for(unsigned int i=0; i<ai_kart_count; i++)
    {
        m_kart_status.push_back(KartStatus(m_ai_kart_list[i], i, -1, -1, 
                                           init_gp_rank, KT_AI));
        init_gp_rank ++;
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
        init_gp_rank ++;
    }

    // Then start the race with the first track
    // ========================================
    m_track_number = 0;
    startNextRace();
}   // startNew

//-----------------------------------------------------------------------------
/** Starts the next (or first) race. It sorts the kart status data structure
 *  according to the number of points, and then creates the world().
 */
void RaceManager::startNextRace()
{
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
/** If there are more races to do, it starts the next race, otherwise it
 *  calls exitRace to finish the race.
 */
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
/**
  * Sets/updates the m_gp_rank status of each KartStatus object
  */
void RaceManager::computeGPRanks()
{
    // calculate the rank of each kart
    const unsigned int NUM_KARTS = getNumberOfKarts();
    int *scores                  = new int[NUM_KARTS];
    int *position                = new int[NUM_KARTS];
    double *race_time            = new double[NUM_KARTS];
    // Ignore the first kart if it's a follow-the-leader race.
    int start=(race_manager->getMinorMode()==RaceManager::MINOR_MODE_FOLLOW_LEADER);
    for (unsigned int kart_id = start; kart_id < NUM_KARTS; ++kart_id)
    {
        position[kart_id]  = kart_id;
        scores[kart_id]    = race_manager->getKartScore(kart_id);
        race_time[kart_id] = race_manager->getOverallTime(kart_id);
    }
    
    if (race_manager->getMinorMode()==RaceManager::MINOR_MODE_FOLLOW_LEADER)
    {
        // fill values for leader
        position[0]  = -1;
        scores[0]    = -1;
        race_time[0] = -1; 
        m_kart_status[0].m_gp_rank = -1;
    }
    
    //Bubblesort
    bool sorted;
    do
    {
        sorted = true;
        for( unsigned int i = start; i < NUM_KARTS - 1; ++i )
        {
            if( scores[i] < scores[i+1] || (scores[i] == scores[i+1] 
                                            && race_time[i] > race_time[i+1]))
            {
                int tmp_score[2];
                double tmp_time;
                
                tmp_score[0] = position[i];
                tmp_score[1] = scores[i];
                tmp_time = race_time[i];
                
                position[i] = position[i+1];
                scores[i] = scores[i+1];
                race_time[i] = race_time[i+1];
                
                position[i+1] = tmp_score[0];
                scores[i+1] = tmp_score[1];
                race_time[i+1] = tmp_time;
                
                sorted = false;
            }
        }
    } while(!sorted);
    
    for (unsigned int i=start; i < NUM_KARTS; ++i)
    {
        //printf("setting kart %s to rank %i\n", race_manager->getKartName(position[i]).c_str(), i-start);
        m_kart_status[position[i]].m_gp_rank = i - start;
    }
    // printf("kart %s has rank %i\n", 0, m_kart_status[0].m_gp_rank);
    delete []scores;
    delete []position;
    delete []race_time;    
}

//-----------------------------------------------------------------------------
/** In GP displays the GP result screen, and then deletes the world.
 */
void RaceManager::exitRace()
{
    // Only display the grand prix result screen if all tracks 
    // were finished, and not when a race is aborted.
    if (m_major_mode==MAJOR_MODE_GRAND_PRIX && m_track_number==(int)m_tracks.size()) 
    {
        unlock_manager->grandPrixFinished();
        
        StateManager::get()->resetAndGoToScreen( MainMenuScreen::getInstance() );
        

        bool someHumanPlayerWon = false;
        const unsigned int kartStatusCount = m_kart_status.size();
        
        /** The rank from which players are considered losers - you are behind half the players OR
            behind the first 3 (the first condition handles cases where there are 3 karts or less).
            However this rank may not be 0, since at least one player must win, hence the std::max call. */
        const int loserThreshold = std::max(1, std::min(3, int(kartStatusCount)/2));
        
        std::string winners[3];
        std::vector<std::string> humanLosers; // because we don't care about AIs that lost
        for (unsigned int i=0; i < kartStatusCount; ++i)
        {
            if(UserConfigParams::m_verbosity >= 5)
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
/** Reruns the last race. This is called after a race is finished, and it will
 *  adjust the number of points and the overall time before restarting the race.
 */
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

/* EOF */
