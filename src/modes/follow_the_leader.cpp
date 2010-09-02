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

#include "modes/follow_the_leader.hpp"

#include "audio/music_manager.hpp"
#include "challenges/unlock_manager.hpp"
#include "config/user_config.hpp"
#include "items/powerup_manager.hpp"
#include "states_screens/race_gui_base.hpp"
#include "tracks/track.hpp"
#include "utils/translation.hpp"

//-----------------------------------------------------------------------------
FollowTheLeaderRace::FollowTheLeaderRace() : LinearWorld()
{
    m_leader_intervals = stk_config->m_leader_intervals;
    m_use_highscores   = false;  // disable high scores
    setClockMode(WorldStatus::CLOCK_COUNTDOWN, m_leader_intervals[0]);
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
/** Called when a kart must be eliminated.
 */
void FollowTheLeaderRace::countdownReachedZero()
{
    if(m_leader_intervals.size()>1)
        m_leader_intervals.erase(m_leader_intervals.begin());
    WorldStatus::setTime(m_leader_intervals[0]);
    int kart_number;
    // If the leader kart is not the first kart, remove the first
    // kart, otherwise remove the last kart.
    int position_to_remove = m_karts[0]->getPosition()==1 
                           ? getCurrentNumKarts() : 1;
    const int kart_amount = m_karts.size();
    for (kart_number=0; kart_number<kart_amount; kart_number++)
    {
        if(m_karts[kart_number]->isEliminated()) continue;
        if(m_karts[kart_number]->getPosition()==position_to_remove)
            break;
    }
    if(kart_number==kart_amount)
    {
        fprintf(stderr,"Problem with removing leader: position %d not found\n",
                position_to_remove);
        for(int i=0; i<kart_amount; i++)
        {
            fprintf(stderr,"kart %d: eliminated %d position %d\n",
                    i,m_karts[i]->isEliminated(), m_karts[i]->getPosition());
        }   // for i
    }  // kart_number==m_kart.size()
    else
    {
        // In case that the kart on position 1 was removed, we have to set
        // the correct position (which equals the remaining number of karts).
        m_karts[kart_number]->setPosition(getCurrentNumKarts());
        removeKart(kart_number);
    }
    
    // almost over, use fast music
    if(getCurrentNumKarts()==3)
    {
        music_manager->switchToFastMusic();
    }
    
    if (isRaceOver())
    {
        // mark leader as finished
        m_karts[0]->finishedRace(getTime());
        
        // mark last human player as finished
        for (unsigned int n=0; n<m_karts.size(); n++)
        {
            if (!m_karts[n]->isEliminated() && 
                 m_karts[n]->getController()->getPlayer() != NULL) // if player kart
            {
                m_karts[n]->finishedRace(getTime());
                //irr::core::stringw message(_("You won the race!"));
                //getRaceGUI()->addMessage( message, m_karts[n], 2.0f, 60 );
            }
        }
    }
    // End of race is detected from World::updateWorld()
    
}   // countdownReachedZero

//-----------------------------------------------------------------------------
/** The follow the leader race is over if there is only one kart left (plus
 *  the leader), or if all (human) players have been eliminated.
 */
bool FollowTheLeaderRace::isRaceOver()
{
    return getCurrentNumKarts()==2 || getCurrentNumPlayers()==0;
}   // isRaceOver

//-----------------------------------------------------------------------------
void FollowTheLeaderRace::restartRace()
{
    LinearWorld::restartRace();
    m_leader_intervals.clear();
    m_leader_intervals    = stk_config->m_leader_intervals;
    WorldStatus::setClockMode(WorldStatus::CLOCK_COUNTDOWN, 
                              m_leader_intervals[0]);
}   // restartRace

//-----------------------------------------------------------------------------
/** Returns the internal identifier for this kind of race. 
 */
std::string FollowTheLeaderRace::getIdent() const
{
    return FTL_IDENT;
}
//-----------------------------------------------------------------------------
RaceGUIBase::KartIconDisplayInfo* FollowTheLeaderRace::getKartsDisplayInfo()
{
    LinearWorld::getKartsDisplayInfo();
    m_kart_display_info[0].special_title = _("Leader");
    return m_kart_display_info;
}
//-----------------------------------------------------------------------------
void FollowTheLeaderRace::getRaceResultOrder(std::vector<int> *order)
{
    const unsigned int num_karts = getNumKarts();
    order->resize(num_karts);

    int *scores       = new int[num_karts];
    double *race_time = new double[num_karts];
    World *world      = World::getWorld();

    // Ignore kart 0, since it was the leader
    (*order)[0] = -1;
    for( unsigned int kart_id = 1; kart_id < num_karts; ++kart_id )
    {
        (*order)[kart_id]  = kart_id;
        scores[kart_id]    = race_manager->getKartScore(kart_id);
        race_time[kart_id] = race_manager->getOverallTime(kart_id);
        
        // check this kart is not in front of leader. If it is, give a score of 0
        if(   getLapForKart(kart_id) * world->getTrack()->getTrackLength() 
              + getDistanceDownTrackForKart(kart_id)
            > getLapForKart(0)       * world->getTrack()->getTrackLength() 
              + getDistanceDownTrackForKart(0))
        {
            scores[kart_id] = 0;
        }
    }
    
    //Bubblesort
    bool sorted;
    do
    {
        sorted = true;
        for( unsigned int i = 1; i < num_karts - 1; ++i )
        {
            if( scores[(*order)[i]] < scores[(*order)[i+1]] || 
                (scores[(*order)[i]] == scores[(*order)[i+1]] 
                 && race_time[(*order)[i]] > race_time[(*order)[i+1]]) )
            {
                int tmp       = (*order)[i];
                (*order)[i]   = (*order)[i+1];
                (*order)[i+1] = tmp;
                sorted        = false;
            }
        }
    } while(!sorted);
    
    for(unsigned int i=1; i<num_karts; i++)
    {
        world->getKart((*order)[i])->setPosition(i);
        setKartPosition((*order)[i], i);
    }
    
    delete []scores;
    delete []race_time;
}

