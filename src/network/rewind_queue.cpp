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

#include "network/rewind_queue.hpp"

#include "modes/world.hpp"
#include "network/rewind_info.hpp"
#include "network/rewind_manager.hpp"

#include <algorithm>

/** The constructor.
 */
RewindQueue::RewindQueue()
{
    reset();
}   // RewindQueue

// ----------------------------------------------------------------------------
/** Frees all saved state information. Note that the Rewinder data must be
 *  freed elsewhere.
 */
RewindQueue::~RewindQueue()
{
    // Destroying the 
    AllRewindInfo::const_iterator i;

    for(i=m_rewind_info.begin(); i!=m_rewind_info.end(); ++i)
    {
        delete *i;
    }
    m_rewind_info.clear();
}   // ~RewindQueue

// ----------------------------------------------------------------------------
/** Frees all saved state information and all destroyable rewinder.
 */
void RewindQueue::reset()
{
#ifdef REWIND_SEARCH_STATS
    m_count_of_comparisons = 0;
    m_count_of_searches    = 0;
#endif

    AllRewindInfo::const_iterator i;
    for(i=m_rewind_info.begin(); i!=m_rewind_info.end(); i++)
    {
        delete *i;
    }
    m_rewind_info.clear();
    m_current = m_rewind_info.begin();

    m_network_events.lock();
    const AllRewindInfo &info = m_network_events.getData();
    for (i = info.begin(); i != info.end(); ++i)
    {
        delete *i;
    }
    m_network_events.getData().clear();
    m_network_events.unlock();
}   // reset

// ----------------------------------------------------------------------------
/** A compare function used when sorting the event lists. It sorts events by
 *  time. In case of equal times, it sorts states and events first (since the
 *  state needs to be restored when replaying first before any other events).
 */
bool RewindQueue::_RewindInfoCompare::operator()(const RewindInfo *ri1,
    const RewindInfo *ri2) const
{
    if (ri1->getTime() < ri2->getTime()) return true;
    if (ri1->getTime() > ri2->getTime()) return false;

    // Now the times are equal. In this case make sure that states are sorted
    // before times and they before events. If two identical types are found,
    // sort them by pointer address (to guarantee that (a,b) and (b,a) give
    // consistent results (otherwise e.g. VS in debug mode will detect
    // inconsistencies and throw an exception).
    if (ri1->isState())
    {
        if (ri2->isState()) return ri1 < ri2;
        return true;  // state first, i.e smallest
    }
    if (ri1->isTime())
    {
        if (ri2->isState()) return false;
        if (ri2->isEvent()) return true;
        return ri1 < ri2;
    }
    // Now ri1 must be event
    if (ri2->isEvent()) return ri1 < ri2;
    // ri2 is state or time which must come first
    return false;

}   // RewindQueue::operator()

// ----------------------------------------------------------------------------
/** Inserts a RewindInfo object in the list of all events at the correct time.
 *  If there are several RewindInfo at the exact same time, state RewindInfo
 *  will be insert at the front, and event and time info at the end of the
 *  RewindInfo with the same time.
 *  \param ri The RewindInfo object to insert.
 */
void RewindQueue::insertRewindInfo(RewindInfo *ri)
{
    AllRewindInfo::iterator i = 
        std::upper_bound(m_rewind_info.begin(),
                         m_rewind_info.end(), ri,
                         RewindQueue::m_rewind_info_compare);
    AllRewindInfo::iterator new_pos = m_rewind_info.insert(i, ri);
    if (m_current == m_rewind_info.end())
        m_current = new_pos;
}   // insertRewindInfo

// ----------------------------------------------------------------------------
/** Adds an event to the rewind data. The data to be stored must be allocated
 *  and not freed by the caller!
 *  \param time Time at which the event was recorded.
 *  \param buffer Pointer to the event data. 
 */
void RewindQueue::addEvent(EventRewinder *event_rewinder,
                             BareNetworkString *buffer, bool confirmed,
                             float time                                   )
{
    RewindInfo *ri = new RewindInfoEvent(time, event_rewinder,
                                         buffer, confirmed);
    insertRewindInfo(ri);
}   // addEvent

// ----------------------------------------------------------------------------
/** Adds a state from the local simulation. It is not thread-safe, so needs to
 *  be called from the main thread
 */
void RewindQueue::addState(Rewinder *rewinder, BareNetworkString *buffer,
                           bool confirmed, float time)
{
    RewindInfo *ri = new RewindInfoState(time, rewinder, buffer, confirmed);
    assert(ri);
    insertRewindInfo(ri);
}   // addState

// ----------------------------------------------------------------------------
/** Adds an event to the list of network rewind data. This function is
 *  threadsafe so can be called by the network thread. The data is synched
 *  to m_rewind_info by the main thread. The data to be stored must be
 *  allocated and not freed by the caller!
 *  \param time Time at which the event was recorded.
 *  \param buffer Pointer to the event data.
 */
void RewindQueue::addNetworkEvent(EventRewinder *event_rewinder,
                                    BareNetworkString *buffer, float time)
{
    RewindInfo *ri = new RewindInfoEvent(time, event_rewinder,
                                         buffer, /*confirmed*/true);

    // Sort the incoming network events so that we can use list::merge
    // to move the network events into the RewindInfo main list.
    m_network_events.lock();
    AllRewindInfo::iterator i =
        std::upper_bound(m_network_events.getData().begin(),
                         m_network_events.getData().end()  , ri,
                         RewindQueue::m_rewind_info_compare        );
    m_network_events.getData().insert(i, ri);
    m_network_events.unlock();
}   // addEvent

// ----------------------------------------------------------------------------
/** Adds a state to the list of network rewind data. This function is
 *  threadsafe so can be called by the network thread. The data is synched
 *  to m_rewind_info by the main thread. The data to be stored must be
 *  allocated and not freed by the caller!
 *  \param time Time at which the event was recorded.
 *  \param buffer Pointer to the event data.
 */
void RewindQueue::addNetworkState(Rewinder *rewinder,
                                  BareNetworkString *buffer, float time)
{
    RewindInfo *ri = new RewindInfoState(time, rewinder,
                                         buffer, /*confirmed*/true);

    // Sort the incoming network events so that we can use list::merge
    // to move the network events into the RewindInfo main list.
    m_network_events.lock();
    AllRewindInfo::iterator i =
        std::upper_bound(m_network_events.getData().begin(),
                         m_network_events.getData().end(), ri,
                         RewindQueue::m_rewind_info_compare      );
    m_network_events.getData().insert(i, ri);
    m_network_events.unlock();
}   // addState

// ----------------------------------------------------------------------------
/** Adds a (dummy) time event to store the time. This enables rewinding to 
 *  replay using the same time step size, minimising errors.
 */
void RewindQueue::addTimeEvent(float time)
{
    RewindInfo *ri = new RewindInfoTime(time);
    insertRewindInfo(ri);
}   // addTimeEvent

// ----------------------------------------------------------------------------
/** Merges thread-safe all data received from the network with the current
 *  local rewind information.
 *  \param needs_rewind True if network rewind information was received which
 *         was in the past (of this simulation), so a rewind must be performed.
 *  \param rewind_time If needs_rewind is true, the time to which a rewind must
 *         be performed (at least). Otherwise undefined, but the value might
 *         be modified in this function.
 */
void RewindQueue::mergeNetworkData(bool *needs_rewind, float *rewind_time)
{
    *needs_rewind = false;
    m_network_events.lock();
    if (m_rewind_info.empty() && m_network_events.getData().empty())
    {
        m_network_events.unlock();
        return;
    }

    /** First merge all newly received network events into the main
    *  event list */

    *rewind_time = -1.0f;
    bool adjust_next = false;

    if (!m_network_events.getData().empty())
    {
        // Check if a rewind is necessary
        *rewind_time = m_network_events.getData().front()->getTime();
        if (*rewind_time < World::getWorld()->getTime())
        {
            *needs_rewind = true;
        }
        // It is possible that m_current points m_rewind_info.end()
        // and that new elements are added to the end of the list. In this
        // case m_rewind_info end would still point to end(), but should
        // point to the newly added element. To achieve this, we first
        // decrement m_current (note that it was tested previously that
        // the list is not empty), merge in the new events, and then
        // increment m_current again. If elements have been added to
        // the end of the list, m_current will now corretly point to them.

        adjust_next = !m_rewind_info.empty();
        if (adjust_next) m_current--;
        m_rewind_info.merge(m_network_events.getData(), m_rewind_info_compare);

        //                    RewindInfoCompare::RewindInfoCompare);
        if (adjust_next)
            m_current++;
        else   // rewind_info was empty, set pointer to first element
            m_current = m_rewind_info.begin();
    }   // if !m_network_events empty

    m_network_events.unlock();


}   // mergeNetworkData

// ----------------------------------------------------------------------------
bool RewindQueue::isEmpty() const
{
    if (m_current != m_rewind_info.end())
        return false;

    m_network_events.lock();
    bool no_network_events = m_network_events.getData().empty();
    m_network_events.unlock();

    return no_network_events;
}   // isEmpty

// ----------------------------------------------------------------------------
/** Returns true if there is at least one more RewindInfo available.
 */
bool RewindQueue::hasMoreRewindInfo() const
{
    return m_current != m_rewind_info.end();
}   // hasMoreRewindInfo

// ----------------------------------------------------------------------------
/** Determines the next time step size to use when recomputing the physics.
 *  The time step size is either 1/60 (default physics), or less, if there
 *  is an even to handle before that time.
 *  \param next_state The next state to replay.
 *  \param end_time The end time to which we must replay forward. Don't
 *         return a dt that would be bigger tham this value.
 *  \return The time step size to use in the next simulation step.
 */
float RewindQueue::determineNextDT(float end_time)
{
    // If there is a next state (which is known to have a different time)
    // use the time difference to determine the time step size.
    if(m_current !=m_rewind_info.end())
        return (*m_current)->getTime() - World::getWorld()->getTime();

    // Otherwise, i.e. we are rewinding the last state/event, take the
    // difference between that time and the world time at which the rewind
    // was triggered.
    return end_time -  (*(--m_current))->getTime();

}   // determineNextDT

// ----------------------------------------------------------------------------
/** Rewinds the rewind queue and undos all events/states stored. It stops
 *  when the first state is reached that was recorded before the undo_time.
 *  It sets the internal 'current' pointer to this state.
 *  \param undo_time To what at least events need to be undone.
 */
void RewindQueue::undoUntil(float undo_time)
{
    AllRewindInfo::iterator i = m_rewind_info.end();

    while (i != m_rewind_info.begin())
    {
        --i;
        if ((*i)->isState() &&
            (*i)->getTime() <= undo_time)
        {
            // FIXME: we might have more than one state, so go back along
            // states at the same time
            m_current = i;
            return;
        }

        // Undo the effect of the event.
        (*i)->undo();

        // Any states saved after the undo_time are invalid. For now mark
        // them as being unconfirmed.
        if ((*i)->isState() &&
            (*i)->getTime() > undo_time)
        {
            (*i)->setConfirmed(false);
        }
    }   // while i!=m_rewind_info.begin()

    Log::error("RewindManager", "No state for rewind to %f",
               undo_time);
    m_current = m_rewind_info.begin();

}   // undoUntil

// ----------------------------------------------------------------------------
/** Tests the sorting order of events with the same time stamp: states must
 *  be first, then time info, then events.
 */
void RewindQueue::testingSortingOrderType(EventRewinder *rewinder, int types[3])
{
    for(unsigned int i=0; i<3; i++)
    {
        switch (types[i])
        {
        case 1:   // Insert State
        {
            BareNetworkString *s = new BareNetworkString();
            s->addUInt8(1);
            addState(NULL, s, true, 0.0f);
            break;
        }
        case 2:
            addTimeEvent(0.0f);
            break;
        case 3:
        {
            BareNetworkString *s = new BareNetworkString();
            s->addUInt8(2);
            addEvent(rewinder, s, true, 0.0f);
            break;
        }
        default: assert(false);
        }   // switch
    }   // for i =0; i<3

    // This should go back to the first state
    undoUntil(0.0);

    // Now test if the three events are sorted in the right order:
    // State, then time, then event
    assert(hasMoreRewindInfo());
    assert(!isEmpty());
    RewindInfo *ri = getNext();
    assert(ri->getTime() == 0.0f);
    assert(ri->isState());
    RewindInfoState *ris = dynamic_cast<RewindInfoState*>(ri);
    assert(ris->getBuffer()->getTotalSize() == 1);
    assert(ris->getBuffer()->getUInt8() == 1);

    operator++();
    ri = getNext();
    assert(!isEmpty());
    assert(hasMoreRewindInfo());
    assert(ri->getTime() == 0.0f);
    assert(ri->isTime());

    operator++();
    ri = getNext();
    assert(!isEmpty());
    assert(hasMoreRewindInfo());
    assert(ri->getTime() == 0.0f);
    assert(ri->isEvent());
    RewindInfoEvent *rie = dynamic_cast<RewindInfoEvent*>(ri);
    assert(rie->getBuffer()->getTotalSize() == 1);
    assert(rie->getBuffer()->getUInt8() == 2);

    operator++();
    assert(isEmpty());
    assert(!hasMoreRewindInfo());

}    // testingSortingOrderType

// ----------------------------------------------------------------------------
/** Tests sorting of rewind infos according to their time. It assumes
 *  different times for all events.
 *  \param types The types for the three events.
 *  \param times The times at which the events happened. Must be >0
 *         (since an additional state at t=0 is added to avoid 
 *         warnings during undoUntil() ).
 */
void RewindQueue::testingSortingOrderTime(EventRewinder *rewinder,
                                          int types[3], float times[3])
{
    float min_rewind_time = std::min({ times[0], times[1], times[2] });
    
    // Avoid warnings about 'no state found' when rewinding
    // and there is indeed no state at the given time.
    addState(NULL, new BareNetworkString(), true, 0);
    for (unsigned int i = 0; i<3; i++)
    {
        switch (types[i])
        {
        case 1:   // Insert State
        {
            BareNetworkString *s = new BareNetworkString();
            s->addUInt8(1);
            addState(NULL, s, true, times[i]);
            break;
        }
        case 2:
            addTimeEvent(times[i]);
            break;
        case 3:
        {
            BareNetworkString *s = new BareNetworkString();
            s->addUInt8(2);
            addEvent(rewinder, s, true, times[i]);
            break;
        }
        default: assert(false);
        }   // switch

    }   // for i =0; i<3

        // This should go back to the first state
    undoUntil(min_rewind_time);

    RewindInfo *ri_prev = getNext();
    operator++();
    while (hasMoreRewindInfo())
    {
        RewindInfo *ri = getNext();
        assert(ri_prev->getTime() < ri->getTime());
        ri_prev = ri;
        operator++();
    }    // while hasMoreRewindInfo

}    // testingSortingOrderTime

// ----------------------------------------------------------------------------
/** Unit tests for RewindQueue. It tests:
 *  - Sorting order of RewindInfos at the same time (i.e. state before time
 *    before events).
 *  - Sorting order of RewindInfos with different timestamps (and a mixture
 *    of types).
 *  - Special cases that triggered incorrect behaviour previously.
 */
void RewindQueue::unitTesting()
{
    // Some classes need the RewindManager (to register themselves with)
    RewindManager::create();

    // A dummy Rewinder and EventRewinder class since some of the calls being
    // tested here need an instance.
    class DummyRewinder : public Rewinder, public EventRewinder
    {
    public:
        BareNetworkString* saveState() const { return NULL; }
        virtual void undoEvent(BareNetworkString *s) {}
        virtual void rewindToEvent(BareNetworkString *s) {}
        virtual void rewindToState(BareNetworkString *s) {}
        virtual void undoState(BareNetworkString *s) {}
        virtual void undo(BareNetworkString *s) {}
        virtual void rewind(BareNetworkString *s) {}
        DummyRewinder() : Rewinder(true) {}
    };
    DummyRewinder *dummy_rewinder = new DummyRewinder();

    RewindQueue q0;
    assert(q0.isEmpty());

    // Test sorting of different RewindInfo with identical time stamps.
    // ----------------------------------------------------------------
    int types[6][3]   = { { 1,2,3 }, { 1,3,2 }, { 2,1,3 }, 
                          { 2,3,1 }, { 3,1,2 }, { 3,2,1 } };
    float times[6][3] = { { 1,2,3 }, { 1,3,2 }, { 2,1,3 },
                          { 2,3,1 }, { 3,1,2 }, { 3,2,1 } };
    for (unsigned int i = 0; i < 6; i++)
    {
        RewindQueue *rq = new RewindQueue();
        rq->testingSortingOrderType(dummy_rewinder, types[i]);
        delete rq;
    }

    // Test sorting of different RewindInfo at different times
    // Checks all combinations of times and types.
    // -------------------------------------------------------
    for (unsigned int i = 0; i < 6; i++)
    {
        for (unsigned int j = 0; j < 6; j++)
        {
            RewindQueue *rq = new RewindQueue();
            rq->testingSortingOrderTime(dummy_rewinder, types[i], times[j]);
            delete rq;
        }   // for j in times
    }   // for i in types

    // Bugs seen before
    // ----------------
    // 1) Current pointer was not reset from end of list when an event
    //    was added and the pointer was already at end of list
    RewindQueue b1;
    b1.addState(NULL, new BareNetworkString(), true, 1.0f);
    ++b1;    // Should now point at end of list
    b1.hasMoreRewindInfo();
    b1.addEvent(NULL, new BareNetworkString(), true, 2.0f);
    RewindInfo *ri = b1.getNext();
    assert(ri->getTime() == 2.0f);
    assert(ri->isEvent());

}   // unitTesting