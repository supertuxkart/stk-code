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

#ifndef HEADER_REWIND_QUEUE_HPP
#define HEADER_REWIND_QUEUE_HPP

#include "utils/synchronised.hpp"

#include <assert.h>
#include <list>
#include <vector>

class BareNetworkString;
class EventRewinder;
class RewindInfo;
class TimeStepInfo;

/** \ingroup network
 */

class RewindQueue
{
private:

    typedef std::list<RewindInfo*> AllRewindInfo;

    AllRewindInfo m_all_rewind_info;

    /** The list of all events received from the network. They are stored
     *  in a separate thread (so this data structure is thread-save), and
     *  merged into m_rewind_info from the main thread. This design (as
     *  opposed to locking m_rewind_info) reduces the synchronisation
     *  between main thread and network thread. */
    typedef std::vector<RewindInfo*> AllNetworkRewindInfo;
    Synchronised<AllNetworkRewindInfo> m_network_events;

    /** Iterator to the curren time step info to be handled. */
    AllRewindInfo::iterator m_current;

    /** Time at which the latest confirmed state is at. */
    int m_latest_confirmed_state_time;


    void cleanupOldRewindInfo(int ticks);

public:
        static void unitTesting();

         RewindQueue();
        ~RewindQueue();
    void reset();
    void addLocalEvent(EventRewinder *event_rewinder, BareNetworkString *buffer,
                       bool confirmed, int ticks);
    void addLocalState(BareNetworkString *buffer, bool confirmed, int ticks);
    void addNetworkEvent(EventRewinder *event_rewinder,
                         BareNetworkString *buffer, int ticks);
    void addNetworkState(BareNetworkString *buffer, int ticks);
    void addNetworkRewindInfo(RewindInfo* ri)
    {
        m_network_events.lock();
        m_network_events.getData().push_back(ri);
        m_network_events.unlock();
    }
    void mergeNetworkData(int world_ticks,  bool *needs_rewind, 
                          int *rewind_ticks);
    void replayAllEvents(int ticks);
    bool isEmpty() const;
    bool hasMoreRewindInfo() const;
    int  undoUntil(int undo_ticks);
    void insertRewindInfo(RewindInfo *ri);

    // ------------------------------------------------------------------------
    /** Returns the time of the latest confirmed state. */
    int getLatestConfirmedState() const
    {
        return m_latest_confirmed_state_time;
    }
    // ------------------------------------------------------------------------
    /** Sets the current element to be the next one and returns the next
     *  RewindInfo element. */
    void next()
    {
        assert(m_current != m_all_rewind_info.end());
        m_current++;
        return;
    }   // operator++

    // ------------------------------------------------------------------------
    /** Returns the current RewindInfo. Caller must make sure that there is at
     *  least one more RewindInfo (see hasMoreRewindInfo()). */
    RewindInfo* getCurrent()
    {
        return (m_current != m_all_rewind_info.end() ) ? *m_current : NULL;
    }   // getNext

};   // RewindQueue


#endif

