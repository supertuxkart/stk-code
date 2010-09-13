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

    // If the leader kart is not the first kart, remove the first
    // kart, otherwise remove the last kart.
    int position_to_remove = m_karts[0]->getPosition()==1 
                           ? getCurrentNumKarts() : 1;
    Kart *kart = getKartAtPosition(position_to_remove);
    if(!kart || kart->isEliminated())
    {
        fprintf(stderr,"Problem with removing leader: position %d not found\n",
                position_to_remove);
        for(unsigned int i=0; i<m_karts.size(); i++)
        {
            fprintf(stderr,"kart %d: eliminated %d position %d\n",
                    i,m_karts[i]->isEliminated(), m_karts[i]->getPosition());
        }   // for i
    }  // 
    else
    {
        removeKart(kart->getWorldKartId());

        // In case that the kart on position 1 was removed, we have 
        // to set the correct position (which equals the remaining 
        // number of karts) for the removed kart, as well as recompute
        // the position for all other karts
        if(position_to_remove==1)
        {
            // We have to add 1 to the number of karts to get the correct
            // position, since the eliminated kart was already removed
            // from the value returned by getCurrentNumKarts (and we have
            // to remove the kart before we can call updateRacePosition).
            // Note that we can not call WorldWithRank::setKartPosition
            // here, since it would not properly support debugging kart
            // ranks (since this kart would get its position set again
            // in updateRacePosition). We only set the rank of the eliminated
            // kart, and updateRacePosition will then call setKartPosition
            // for the now eliminated kart.
            kart->setPosition(getCurrentNumKarts()+1);
            updateRacePosition();
        }
    }
    
    // almost over, use fast music
    if(getCurrentNumKarts()==3)
    {
        music_manager->switchToFastMusic();
    }
    
    if (isRaceOver())
    {
        // Mark all still racing karts to be finished.
        for (unsigned int n=0; n<m_karts.size(); n++)
        {
            if (!m_karts[n]->isEliminated() && 
                !m_karts[n]->hasFinishedRace())
            {
                m_karts[n]->finishedRace(getTime());
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
}   // getIdent

//-----------------------------------------------------------------------------
/** Sets the title for all karts that is displayed in the icon list. In
 *  this mode the title for the first kart is set to 'leader'.
 */
RaceGUIBase::KartIconDisplayInfo* FollowTheLeaderRace::getKartsDisplayInfo()
{
    LinearWorld::getKartsDisplayInfo();
    m_kart_display_info[0].special_title = _("Leader");
    return m_kart_display_info;
}   // getKartsDisplayInfo
