//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2017 Joerg Henrichs
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

#ifndef HEADER_TIME_STEP_INFO_HPP
#define HEADER_TIME_STEP_INFO_HPP

#include "network/rewind_info.hpp"
#include <assert.h>
#include <vector>

class RewindInfo;
class EventRewinder;

/** \ingroup network
 */

class RewindInfo;
class RewindInfoEvent;
class RewindInfoState;

/** This class stores information about each time step on a client or server.
 *  Firstly it stores the world time and time step size. In case of a rewind
 *  this allows the rewind to use the same time step size, which reduces
 *  jitter caused by different time step size. Secondly for each time it
 *  stores (if exist) all states, and all events. The TimeStepInfo acts as a
 *  container and will store all states and events that happened 'around' time,
 *  i.e. between time-X and time+Y (X and Y are implicitely defined in the
 *  RewindQueue). This avoids that messages from clients to the server create
 *  more and more TimeStepInfo, with smaller and smaller dt, which would make
 *  rewinds more expensive. 
 */
class TimeStepInfo
{
private:
    typedef std::vector<RewindInfo*> AllRewindInfo;

    /** The list of all states and events at a certain time. */
    AllRewindInfo m_list_of_events;

    /** Time at which those events should be executed here. */
    float m_time;

    /** Time step to be used. */
    float m_dt;

    /** Bullet maintains a 'left over' time since it is running with a fixed
     *  60 fps. Restoring this value exactly improves accuracy of rewinds. */
    float m_local_physics_time;
public:
         TimeStepInfo(float time, float dt);
    void insert(RewindInfo *ri);
    void undoAll();
    void replayAllEvents();
    void replayAllStates();
    // ------------------------------------------------------------------------
    /** Returns the time for this TimeStepInfo instance. */
    float getTime() const { return m_time;  }
    // ------------------------------------------------------------------------
    /** Returns the left-over physics time. */
    float getLocalPhysicsTime() const { return m_local_physics_time;  }
    // ------------------------------------------------------------------------
    /** Returns the (previous) time step size, so that rewindw can be done
     *  with same time step size. */
    float getDT() const { return m_dt;  }
    // ------------------------------------------------------------------------
    /** Returns if this TimeStepInfo instance has a confirmed state, i.e. if
     *  a rewind can start from this time. */
    bool hasConfirmedState() const 
    { 
        if (m_list_of_events.empty()) return false;
        const RewindInfo *ri = m_list_of_events[0];
        return ri->isState() && ri->isConfirmed();
    }   // hasConfirmedState
};   // TimeStepInfo

#endif

