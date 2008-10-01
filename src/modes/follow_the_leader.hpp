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

class FollowTheLeaderRace : public LinearWorld, public Clock::ClockListener
{
    std::vector<float>  m_leader_intervals;    // time till elimination in follow leader
public:
    FollowTheLeaderRace();
    virtual ~FollowTheLeaderRace();
    
    // clock events
    virtual void countdownReachedZero();
    virtual void onGo();
    virtual void onTerminate();
    
    // overriding World methods
    virtual void update(float delta);
    virtual void restartRace();
    virtual std::string getInternalCode() const;
    virtual bool useFastMusicNearEnd() const { return false; }
    virtual KartIconDisplayInfo* getKartsDisplayInfo(const RaceGUI* caller);
    
    virtual bool raceHasLaps(){ return false; }
};


#endif
