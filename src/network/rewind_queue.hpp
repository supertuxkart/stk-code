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

class RewindInfo;
class EventRewinder;

/** \ingroup network
 */

class RewindQueue
{
private:

    /** Pointer to all saved states. */
    typedef std::list<RewindInfo*> AllRewindInfo;

    /** The list of all events that are affected by a rewind. */
    AllRewindInfo m_rewind_info;

    /** The list of all events received from the network. They are stored
     *  in a separate thread (so this data structure is thread-save), and
     *  merged into m_rewind_info from the main thread. This design (as
     *  opposed to locking m_rewind_info) reduces the synchronisation
     *  between main thread and network thread. */
    Synchronised<AllRewindInfo> m_network_events;

    /** Iterator to the next rewind info to be handled. */
    AllRewindInfo::iterator m_current;

#define REWIND_SEARCH_STATS

#ifdef REWIND_SEARCH_STATS
    /** Gather some statistics about how many comparisons we do, 
     *  to find out if it's worth doing a binary search.*/
    mutable int m_count_of_comparisons;
    mutable int m_count_of_searches;
#endif

    void insertRewindInfo(RewindInfo *ri);

    struct _RewindInfoCompare
    {
        bool operator()(const RewindInfo *ri1, const RewindInfo *ri2) const;
    } m_rewind_info_compare;

    void testingSortingOrderType(EventRewinder *rewinder, int types[3]);
    void testingSortingOrderTime(EventRewinder *rewinder, int types[3],
                                 float times[3]                       );

public:
        static void unitTesting();

         RewindQueue();
        ~RewindQueue();
    void reset();
    void addEvent(EventRewinder *event_rewinder, BareNetworkString *buffer,
                  bool confirmed, float time);
    void addState(Rewinder *rewinder, BareNetworkString *buffer,
                  bool confirmed, float time);
    void addNetworkEvent(EventRewinder *event_rewinder,
                         BareNetworkString *buffer, float time);
    void addNetworkState(Rewinder *rewinder, BareNetworkString *buffer,
                         float time);
    void addTimeEvent(float time);
    void mergeNetworkData(bool *needs_rewind, float *rewind_time);
    bool isEmpty() const;
    bool hasMoreRewindInfo() const;
    void undoUntil(float undo_time);
    float determineNextDT(float max_time);

    // ------------------------------------------------------------------------
    RewindQueue::AllRewindInfo::iterator& operator++()
    {
        assert(m_current != m_rewind_info.end());
        m_current++;
        return m_current;
    }   // operator++

    // ------------------------------------------------------------------------
    /** Returns the next event. Caller must make sure that there is at least
     *  one more RewindInfo (see hasMoreRewindInfo()). */
    RewindInfo *getNext()
    {
        assert(m_current != m_rewind_info.end());
        return *m_current;
    }   // getNext

};   // RewindQueue


#endif

