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
#include "unlock_manager.hpp"
#include "gui/menu_manager.hpp"
#include "user_config.hpp"

//-----------------------------------------------------------------------------
FollowTheLeaderRace::FollowTheLeaderRace() : World(), Clock::ClockListener()
{
    m_leader_intervals    = stk_config->m_leader_intervals;
    
    m_clock.registerEventListener(this);
    m_clock.setMode(COUNTDOWN, m_leader_intervals[0]);
}

//-----------------------------------------------------------------------------
FollowTheLeaderRace::~FollowTheLeaderRace()
{
}


#pragma mark -
#pragma mark clock events

//-----------------------------------------------------------------------------
void FollowTheLeaderRace::countdownReachedZero()
{
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
void FollowTheLeaderRace::onTerminate()
{
    World::terminateRace();
}

#pragma mark -
#pragma mark overridden from World

//-----------------------------------------------------------------------------
void FollowTheLeaderRace::update(float delta)
{
    m_clock.updateClock(delta);
    
    World::update(delta);
    if(!m_clock.isRacePhase()) return;
    
    if(m_clock.getTime() < 0.0f)
    {
        if(m_leader_intervals.size()>1)
            m_leader_intervals.erase(m_leader_intervals.begin());
        m_clock.setTime(m_leader_intervals[0]);
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
            removeKart(kart_number);
        }
        // The follow the leader race is over if there is only one kart left,
        // or if all players have gone
        if(getCurrentNumKarts()==2 || getCurrentNumPlayers()==0)
        {
            // Add the results for the remaining kart
            for(int i=1; i<(int)race_manager->getNumKarts(); i++)
                if(!m_kart[i]->isEliminated()) 
                    race_manager->RaceFinished(m_kart[i], m_clock.getTime());
            
            m_clock.raceOver();
            return;
        }
    }
}
//-----------------------------------------------------------------------------
void FollowTheLeaderRace::restartRace()
{
    World::restartRace();
    m_leader_intervals    = stk_config->m_leader_intervals;
}
//-----------------------------------------------------------------------------
std::string FollowTheLeaderRace::getInternalCode() const
{
    return "FOLLOW_LEADER";
}