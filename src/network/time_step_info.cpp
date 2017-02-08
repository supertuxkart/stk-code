//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2013 Joerg Henrichs
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

#include "network/time_step_info.hpp"

#include "network/rewind_info.hpp"
#include "physics/physics.hpp"

/** Creates a new TimeStepInfo for a given time and given dt.
 *  \param time Time for this TimeStepInfo object.
 *  \param dt Time step size.
 */
TimeStepInfo::TimeStepInfo(float time, float dt)
{
    m_time = time;
    m_dt = dt;
    m_local_physics_time = Physics::getInstance()->getPhysicsWorld()
                                                 ->getLocalTime();
}   // StateEventList

// --------------------------------------------------------------------
/** Adds a state. State must be saved first, i.e. before events. The
 *  RewindManager guarantees this order
 *  \param ri The RewindInfo object for the state.
 */
void TimeStepInfo::insert(RewindInfo *ri)
{
    if (ri->isState())
    {
        // States need to be inserted first.
        // FIXME: handle duplicated states, e.g. server doing a rewind
        // and sending another updated state
        AllRewindInfo::iterator i = m_list_of_events.begin();
        while (i != m_list_of_events.end() && (*i)->isState())
            ++i;
        m_list_of_events.insert(i, ri);
    }
    else
    {
        // Events at the same time are just added to the end
        m_list_of_events.push_back(ri);
    }
}   // addStaet

// --------------------------------------------------------------------
/** Undos all events and states for this time step.
 */
void TimeStepInfo::undoAll()
{
    AllRewindInfo::reverse_iterator i;
    for (i = m_list_of_events.rbegin(); i != m_list_of_events.rend(); i++)
    {
        (*i)->undo();
    }
}   // undoAll
// --------------------------------------------------------------------
/** Replays all events for this TimeStepInfo.
 */
void TimeStepInfo::replayAllEvents()
{
    AllRewindInfo::reverse_iterator i;
    for (i = m_list_of_events.rbegin(); i != m_list_of_events.rend(); i++)
    {
        if ((*i)->isEvent())
            (*i)->rewind();
    }
}   // replayAllEvents

// --------------------------------------------------------------------
/** Replays all state information for this TimeStepInfo.
*/
void TimeStepInfo::replayAllStates()
{
    AllRewindInfo::reverse_iterator i;
    for (i = m_list_of_events.rbegin(); i != m_list_of_events.rend(); i++)
    {
        if ((*i)->isState())
            (*i)->rewind();
    }
}   // replayAllStates

