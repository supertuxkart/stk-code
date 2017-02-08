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

#include "network/rewinder.hpp"
#include "utils/ptr_vector.hpp"
#include "utils/synchronised.hpp"

#include <assert.h>
#include <list>
#include <vector>

class EventRewinder;
class RewindInfo;
class TimeStepInfo;

/** \ingroup network
 */

class RewindQueue
{
private:
    /** Pointer to all saved */
    typedef std::list<TimeStepInfo*> AllTimeStepInfo;

    /** The list of all events that are affected by a rewind. */
    AllTimeStepInfo m_time_step_info;

    /** The list of all events received from the network. They are stored
     *  in a separate thread (so this data structure is thread-save), and
     *  merged into m_rewind_info from the main thread. This design (as
     *  opposed to locking m_rewind_info) reduces the synchronisation
     *  between main thread and network thread. */
    typedef std::vector<RewindInfo*> AllNetworkRewindInfo;
    Synchronised<AllNetworkRewindInfo> m_network_events;

    /** Iterator to the curren time step info to be handled. This should
     *  always be at the same time as World::getTime(). */
    AllTimeStepInfo::iterator m_current;

    TimeStepInfo *findClosestTimeStepInfo(float t);
    void insertRewindInfo(RewindInfo *ri);

    struct _TimeStepInfoCompare
    {
        bool operator()(const TimeStepInfo * const ri1, const TimeStepInfo * const ri2) const;
    } m_time_step_info_compare;

    void testingSortingOrderType(EventRewinder *rewinder, int types[3]);
    void testingSortingOrderTime(EventRewinder *rewinder, int types[3],
                                 float times[3]                       );

public:
        static void unitTesting();

         RewindQueue();
        ~RewindQueue();
    void reset();
    void addNewTimeStep(float time, float dt);
    void addLocalEvent(EventRewinder *event_rewinder, BareNetworkString *buffer,
                       bool confirmed, float time);
    void addLocalState(Rewinder *rewinder, BareNetworkString *buffer,
                       bool confirmed);
    void addNetworkEvent(EventRewinder *event_rewinder,
                         BareNetworkString *buffer, float time);
    void addNetworkState(Rewinder *rewinder, BareNetworkString *buffer,
                         float time, float dt);
    void mergeNetworkData(bool *needs_rewind, float *rewind_time);
    bool isEmpty() const;
    bool hasMoreRewindInfo() const;
    void undoUntil(float undo_time);
    float determineNextDT(float max_time);

    // ------------------------------------------------------------------------
    RewindQueue::AllTimeStepInfo::iterator& operator++()
    {
        assert(m_current != m_time_step_info.end());
        m_current++;
        return m_current;
    }   // operator++

    // ------------------------------------------------------------------------
    /** Returns the current RewindInfo. Caller must make sure that there is at least
     *  one more RewindInfo (see hasMoreRewindInfo()). */
    TimeStepInfo *getCurrent()
    {
        assert(m_current != m_time_step_info.end());
        return *m_current;
    }   // getNext

};   // RewindQueue


#endif

