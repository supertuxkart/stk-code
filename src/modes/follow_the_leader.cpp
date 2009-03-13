//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2004 SuperTuxKart-Team
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

#include "user_config.hpp"
#include "audio/sound_manager.hpp"
#include "challenges/unlock_manager.hpp"
#include "gui/race_gui.hpp"
#include "items/powerup_manager.hpp"
#include "modes/follow_the_leader.hpp"
#include "tracks/track.hpp"
#include "utils/translation.hpp"

//-----------------------------------------------------------------------------
FollowTheLeaderRace::FollowTheLeaderRace() : LinearWorld()
{
    m_leader_intervals = stk_config->m_leader_intervals;
    LinearWorld::init();
    m_use_highscores   = false;  // disable high scores
	TimedRace::setClockMode(COUNTDOWN, m_leader_intervals[0]);
}

//-----------------------------------------------------------------------------
FollowTheLeaderRace::~FollowTheLeaderRace()
{
}

#if 0
#pragma mark -
#pragma mark clock events
#endif
//-----------------------------------------------------------------------------
void FollowTheLeaderRace::countdownReachedZero()
{
    if(m_leader_intervals.size()>1)
        m_leader_intervals.erase(m_leader_intervals.begin());
    TimedRace::setTime(m_leader_intervals[0]);
    int kart_number;
    // If the leader kart is not the first kart, remove the first
    // kart, otherwise remove the last kart.
    int position_to_remove = m_kart[0]->getPosition()==1 
        ? getCurrentNumKarts() : 1;
    const int kart_amount = m_kart.size();
    for (kart_number=0; kart_number<kart_amount; kart_number++)
    {
        if(m_kart[kart_number]->isEliminated()) continue;
        if(m_kart[kart_number]->getPosition()==position_to_remove)
            break;
    }
    if(kart_number==kart_amount)
    {
        fprintf(stderr,"Problem with removing leader: position %d not found\n",
                position_to_remove);
        for(int i=0; i<kart_amount; i++)
        {
            fprintf(stderr,"kart %d: eliminated %d position %d\n",
                    i,m_kart[i]->isEliminated(), m_kart[i]->getPosition());
        }   // for i
    }  // kart_number==m_kart.size()
    else
    {
        // In case that the kart on position 1 was removed, we have to set
        // the correct position (which equals the remaining number of karts).
        m_kart[kart_number]->setPosition(getCurrentNumKarts());
        removeKart(kart_number);
    }
    
    // almost over, use fast music
    if(getCurrentNumKarts()==3)  
        sound_manager->switchToFastMusic();
    
    // The follow the leader race is over if there is only one kart left,
    // or if all players have gone
    if(getCurrentNumKarts()==2 || getCurrentNumPlayers()==0)
    {
        // Note: LinearWorld::terminateRace adds the scores for all remaining
        // karts in the race.
        TimedRace::enterRaceOverState();
        return;
    }
}
//-----------------------------------------------------------------------------
void FollowTheLeaderRace::onGo()
{
    // Reset the brakes now that the prestart 
    // phase is over (braking prevents the karts 
    // from sliding downhill)
    for(unsigned int i=0; i<m_kart.size(); i++) 
    {
        m_kart[i]->resetBrakes();
    }
}
//-----------------------------------------------------------------------------
void FollowTheLeaderRace::terminateRace()
{
    LinearWorld::terminateRace();
}


#if 0
#pragma mark -
#pragma mark overridden from World
#endif

//-----------------------------------------------------------------------------
void FollowTheLeaderRace::update(float delta)
{   
    LinearWorld::update(delta);
}
//-----------------------------------------------------------------------------
void FollowTheLeaderRace::restartRace()
{
    LinearWorld::restartRace();
	m_leader_intervals.clear();
    m_leader_intervals    = stk_config->m_leader_intervals;
	TimedRace::setClockMode(COUNTDOWN, m_leader_intervals[0]);
}
//-----------------------------------------------------------------------------
std::string FollowTheLeaderRace::getInternalCode() const
{
    return "FOLLOW_LEADER";
}
//-----------------------------------------------------------------------------
KartIconDisplayInfo* FollowTheLeaderRace::getKartsDisplayInfo(const RaceGUI* caller)
{
    LinearWorld::getKartsDisplayInfo(caller);
    m_kart_display_info[0].special_title = _("Leader");
    return m_kart_display_info;
}
//-----------------------------------------------------------------------------
void FollowTheLeaderRace::raceResultOrder( int* order )
{
    const unsigned int NUM_KARTS = race_manager->getNumKarts();
    
    int *scores       = new int[NUM_KARTS];
    double *race_time = new double[NUM_KARTS];
    
    // Ignore kart 0, since it was the leader
    order[0] = -1;
    for( unsigned int kart_id = 1; kart_id < NUM_KARTS; ++kart_id )
    {
        order[kart_id]     = kart_id;
        scores[kart_id]    = race_manager->getKartScore(kart_id);
        race_time[kart_id] = race_manager->getOverallTime(kart_id);
		
		// check this kart is not in front of leader. If it is, give a score of 0
		if(m_kart_info[kart_id].m_race_lap * RaceManager::getTrack()->getTrackLength() + getDistanceDownTrackForKart(kart_id) >
		   m_kart_info[0].m_race_lap * RaceManager::getTrack()->getTrackLength() + getDistanceDownTrackForKart(0))
			scores[kart_id] = 0;
    }
    
    //Bubblesort
    bool sorted;
    do
    {
        sorted = true;
        for( unsigned int i = 1; i < NUM_KARTS - 1; ++i )
        {
            if( scores[order[i]] < scores[order[i+1]] || 
                (scores[order[i]] == scores[order[i+1]] 
                 && race_time[order[i]] > race_time[order[i+1]]) )
            {
                int tmp     = order[i];
                order[i]    = order[i+1];
                order[i+1]  = tmp;
                sorted      = false;
            }
        }
    } while(!sorted);
	
    for(unsigned int i=1; i<NUM_KARTS; i++)
        RaceManager::getKart(order[i])->setPosition(i);
    
    delete []scores;
    delete []race_time;
}

