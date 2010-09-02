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

#ifndef _follow_the_leader_hpp_
#define _follow_the_leader_hpp_

#include "modes/linear_world.hpp"
#include "states_screens/race_gui_base.hpp"

/**
  * \brief An implementation of World, based on LinearWorld, to provide the Follow-the-leader game mode
  * \ingroup modes
  */
class FollowTheLeaderRace : public LinearWorld
{
private:
    std::vector<float>  m_leader_intervals;    // time till elimination in follow leader

public:
    
             FollowTheLeaderRace();
    virtual ~FollowTheLeaderRace();
    
    // clock events
    virtual void countdownReachedZero();
    
    // overriding World methods
    virtual void restartRace();
    virtual std::string getIdent() const;
    virtual bool useFastMusicNearEnd() const { return false; }
    virtual RaceGUIBase::KartIconDisplayInfo* getKartsDisplayInfo();
    
    virtual bool isRaceOver();
    virtual bool raceHasLaps(){ return false; }
    
    /** Called by the race result GUI at the end of the race to know the final order
        (fill in the 'order' array) */
    virtual void getRaceResultOrder(std::vector<int> *order);
};


#endif
