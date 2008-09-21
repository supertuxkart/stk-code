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

#include "race_manager.hpp"

#include <iostream>

#include "track_manager.hpp"
#include "kart_properties_manager.hpp"
#include "unlock_manager.hpp"
#include "gui/menu_manager.hpp"
#include "modes/world.hpp"
#include "scene.hpp"
#include "user_config.hpp"
#include "stk_config.hpp"
#include "network/network_manager.hpp"
#include "modes/standard_race.hpp"
#include "modes/follow_the_leader.hpp"

RaceManager* race_manager= NULL;

//-----------------------------------------------------------------------------
World* world = NULL;
World* RaceManager::getWorld()
{
    return world;
}
/** Call to set the world, or call setWorld(NULL) to delete the current world.
 */
void RaceManager::setWorld(World* world_arg)
{
    if(world != NULL) delete world;
    world = world_arg;
}
Track* RaceManager::getTrack()
{
    return world->getTrack();
}
Kart* RaceManager::getPlayerKart(const unsigned int n)
{
    return world->getPlayerKart(n);
}
Kart* RaceManager::getKart(const unsigned int n)
{
    return world->getKart(n);
}
//-----------------------------------------------------------------------------

/** Constructs the race manager.
 */
RaceManager::RaceManager()
{
    m_num_karts          = user_config->m_karts;
    m_difficulty         = RD_HARD;
    m_major_mode         = RM_SINGLE;
    m_minor_mode         = RM_QUICK_RACE;
    m_track_number       = 0;
    m_active_race        = false;
    m_score_for_position = stk_config->m_scores;
    m_coin_target        = 0;
    setTrack("race");
    setNumLocalPlayers(1);
    setLocalKartInfo(0, "tuxkart");
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
    assert(0<=player_id && player_id <m_local_kart_info.size());

    m_local_kart_info[player_id]=RemoteKartInfo(player_id, kart,
                                                user_config->m_player[player_id].getName(),
                                                network_manager->getMyHostId());
}   // setLocalKartInfo

//-----------------------------------------------------------------------------
/** Sets the number of local players playing on this computer (including
 *  split screen).
 *  \param n Number of local players.
 */
void RaceManager::setNumLocalPlayers(unsigned int n)
{
    m_local_kart_info.resize(n);
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
    if(diff==RD_SKIDDING)
    {
        m_difficulty = RD_HARD;
        user_config->m_skidding = true;
    }
    else
    {
        m_difficulty = diff;
        user_config->m_skidding = false;
    }
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
    // If less kart selected than there are player karts, adjust the number of
    // karts to the minimum
    if(n<0)
    {
        m_num_karts -= n;
        n = 0;
    }
    m_random_kart_list=kart_properties_manager->getRandomKartList(n,
                                                                  m_player_karts);

}   // computeRandomKartList

//-----------------------------------------------------------------------------
/** Starts a new race or GP (or other mode). It sets up the list of player
 *  karts, AI karts, laps etc., and then uses startNextRace to actually start
 *  the race.
 */
void RaceManager::startNew()
{
    if(m_major_mode==RM_GRAND_PRIX)   // GP: get tracks and laps from grand prix
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

    // First add the AI karts (randomly chosen)
    // ----------------------------------------
    for(unsigned int i=0; i<m_random_kart_list.size(); i++)
        m_kart_status.push_back(KartStatus(m_random_kart_list[i], i, -1, -1, KT_AI));

    // Then the players, which start behind the AI karts
    // -------------------------------------------------
    for(int i=m_player_karts.size()-1; i>=0; i--)
    {
        KartType kt=(m_player_karts[i].getHostId()==network_manager->getMyHostId())
                   ? KT_PLAYER : KT_NETWORK_PLAYER;
        m_kart_status.push_back(KartStatus(m_player_karts[i].getKartName(), i,
                                           m_player_karts[i].getLocalPlayerId(),
                                           m_player_karts[i].getGlobalPlayerId(),
                                           kt
                                           ) );
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
    m_num_finished_karts   = 0;
    m_num_finished_players = 0;

    // if subsequent race, sort kart status structure
    // ==============================================
    if (m_track_number > 0)
    {  
        // In follow the leader mode do not change the first kart, 
        // since it's always the leader.
        int offset = (m_minor_mode==RM_FOLLOW_LEADER) ? 1 : 0;

        std::sort(m_kart_status.begin()+offset, m_kart_status.end());
        //reverse kart order if flagged in stk_config
        if (stk_config->m_grid_order)
        {
            std::reverse(m_kart_status.begin()+offset, m_kart_status.end());
        } 
    }   // not first race

    // the constructor assigns this object to the global
    // variable world. Admittedly a bit ugly, but simplifies
    // handling of objects which get created in the constructor
    // and need world to be defined.
    
    /*
     RM_GRAND_PRIX, RM_SINGLE, 
     RM_QUICK_RACE, RM_TIME_TRIAL, RM_FOLLOW_LEADER
     FIXME
     */
    
    if(m_minor_mode==RM_FOLLOW_LEADER) new FollowTheLeaderRace();
    else new StandardRace();

    m_active_race = true;
}   // startNextRace

//-----------------------------------------------------------------------------
/** If there are more races to do, it starts the next race, otherwise it
 *  calls exit_race to finish the race.
 */
void RaceManager::next()
{
    m_num_finished_karts   = 0;
    m_num_finished_players = 0;
    m_track_number++;
    if(m_track_number<(int)m_tracks.size())
    {
        scene->clear();
        startNextRace();
    }
    else
    {
        exit_race();
    }
}   // next

//-----------------------------------------------------------------------------
/** In GP displays the GP result screen, and then deletes the world.
 */
void RaceManager::exit_race()
{
    // Only display the grand prix result screen if all tracks 
    // were finished, and not when a race is aborted.
    if(m_major_mode==RM_GRAND_PRIX && m_track_number==(int)m_tracks.size()) 
    {
        unlock_manager->grandPrixFinished();
        menu_manager->switchToGrandPrixEnding();
    }
    else
    {
        menu_manager->switchToMainMenu();
    }
    scene->clear();
    delete world;
    world          = 0;
    m_track_number = 0;
    m_active_race  = false;    
}   // exit_Race

//-----------------------------------------------------------------------------
/** A kart has finished the race at the specified time (which can be 
 *  different from RaceManager::getWorld()->getClock() in case of setting extrapolated arrival 
 *  times).
 *  \param kart The kart that finished the race.
 *  \param time Time at which the kart finished the race.
 */
void RaceManager::RaceFinished(const Kart *kart, float time)
{
    unsigned i;
    for(i=0; i<m_kart_status.size(); i++)
    {
        if(kart->getIdent()==m_kart_status[i].m_ident) break;
    }   // for i
    if(i>=m_kart_status.size())
    {
        fprintf(stderr, "Kart '%s' not found. Ignored.\n",kart->getName().c_str());
        return;
    }

    // In follow the leader mode, kart 0 does not get any points,
    // so the position of each kart is actually one better --> decrease pos
    int pos = kart->getPosition();
    if(m_minor_mode==RM_FOLLOW_LEADER) 
    {
        pos--;
        // If the position is negative (i.e. follow leader and kart on 
        // position 0) set the score of this kart to the lowest possible
        // score, since the kart is ahead of the leader
        if(pos<=0) pos=stk_config->m_max_karts;
    }

    m_kart_status[i].m_score        += m_score_for_position[pos-1];
    m_kart_status[i].m_last_score    = m_score_for_position[pos-1];
    m_kart_status[i].m_overall_time += time;
    m_kart_status[i].m_last_time     = time;
    m_num_finished_karts ++;
    if(kart->isPlayerKart()) m_num_finished_players++;
}   // raceFinished

//-----------------------------------------------------------------------------
/** Reruns the last race. This is called after a race is finished, and it will
 *  adjust the number of points and the overall time before restarting the race.
 */
void RaceManager::rerunRace()
{
    // Subtract last score from all karts:
    for(int i=0; i<m_num_karts; i++)
    {
        m_kart_status[i].m_score        -= m_kart_status[i].m_last_score;
        m_kart_status[i].m_overall_time -= m_kart_status[i].m_last_time;
    }
    world->restartRace();
}   // rerunRace

/* EOF */
