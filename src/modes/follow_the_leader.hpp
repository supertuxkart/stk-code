//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2004-2015 SuperTuxKart-Team
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
        // time till elimination in follow leader
    std::vector<float>  m_leader_intervals;

    /** A timer used before terminating the race. */
    float m_is_over_delay;

    /** Time the last kart was eliminated. It is used to assign each
     *  kart a 'finish' time (i.e. how long they lasted). */
    float m_last_eliminated_time;
public:

             FollowTheLeaderRace();
    virtual ~FollowTheLeaderRace();

    // clock events
    virtual void countdownReachedZero() OVERRIDE;
    virtual int  getScoreForPosition(int p) OVERRIDE;

    // overriding World methods
    virtual void reset() OVERRIDE;
    virtual const std::string& getIdent() const OVERRIDE;
    virtual const btTransform &getStartTransform(int index) OVERRIDE;
    virtual void getKartsDisplayInfo(
                 std::vector<RaceGUIBase::KartIconDisplayInfo> *info) OVERRIDE;
    virtual void init() OVERRIDE;
    virtual void terminateRace() OVERRIDE;
    virtual bool isRaceOver() OVERRIDE;
    // ------------------------------------------------------------------------
    /** Returns if this type of race has laps. */
    virtual bool raceHasLaps() OVERRIDE { return false; }
    // ------------------------------------------------------------------------
    /** Returns if faster music should be used at the end. */
    virtual bool useFastMusicNearEnd() const OVERRIDE { return false; }
};   // FollowTheLeader


#endif
