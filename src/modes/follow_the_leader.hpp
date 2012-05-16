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
    virtual void countdownReachedZero() OVERRIDE;
    
    // overriding World methods
    virtual void restartRace() OVERRIDE;
    virtual const std::string& getIdent() const OVERRIDE;
    virtual float getClockStartTime();
    virtual bool useFastMusicNearEnd() const OVERRIDE { return false; }
    virtual RaceGUIBase::KartIconDisplayInfo* getKartsDisplayInfo() OVERRIDE;
    virtual void init() OVERRIDE;
    
    virtual bool isRaceOver() OVERRIDE;
    virtual bool raceHasLaps() OVERRIDE { return false; }
    
};   // FollowTheLeader


#endif
