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

#include "config/stk_config.hpp"
#include "modes/world.hpp"
#include "network/dummy_rewinder.hpp"
#include "network/network.hpp"
#include "network/network_config.hpp"
#include "network/rewinder.hpp"
#include "network/rewind_info.hpp"
#include "network/rewind_manager.hpp"

#include <algorithm>

/** The RewindQueue stores one TimeStepInfo for each time step done.
 *  The TimeStepInfo stores all states and events to be used at the
 *  given timestep. 
 *  All network events (i.e. new states or client events) are stored in a
 *  separate list m_network_events. At the very start of a new time step
 *  a new TimeStepInfo object is added. Then all network events that are
 *  supposed to happen between t and t+dt are added to this newly added
 *  TimeStep (see mergeNetworkData), and are then being executed.
 *  In case of a rewind the RewindQueue finds the last TimeStepInfo with
 *  a confirmed server state (undoing the events, see undoUntil). Then
 *  the state is restored from the TimeStepInfo object (see replayAllStates)
 *  then the rewind manager re-executes the time steps (using the events
 *  stored at each timestep).
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
    // This frees all current data
    reset();
}   // ~RewindQueue

// ----------------------------------------------------------------------------
/** Frees all saved state information and all destroyable rewinder.
 */
void RewindQueue::reset()
{
    m_network_events.lock();

    AllNetworkRewindInfo &info = m_network_events.getData();
    for (AllNetworkRewindInfo::const_iterator i  = info.begin(); 
                                              i != info.end(); ++i)
    {
        delete *i;
    }
    m_network_events.getData().clear();
    m_network_events.unlock();

    AllRewindInfo::const_iterator i;
    for (i = m_all_rewind_info.begin(); i != m_all_rewind_info.end(); ++i)
        delete *i;

    m_all_rewind_info.clear();
    m_current = m_all_rewind_info.end();
    m_latest_confirmed_state_time = -1;
}   // reset

// ----------------------------------------------------------------------------
/** Inserts a RewindInfo object in the list of all events at the correct time.
 *  If there are several RewindInfo at the exact same time, state RewindInfo
 *  will be insert at the front, and event info at the end of the RewindInfo
 *  with the same time.
 *  \param ri The RewindInfo object to insert.
 *  \param update_current If set, the current pointer will be updated if
 *         necessary to point to the new event
 */
void RewindQueue::insertRewindInfo(RewindInfo *ri)
{
    AllRewindInfo::iterator i = m_all_rewind_info.end();

    while (i != m_all_rewind_info.begin())
    {
        AllRewindInfo::iterator i_prev = i;
        i_prev--;
        // Now test if 'ri' needs to be inserted after the
        // previous element, i.e. before the current element:
        if ((*i_prev)->getTicks() < ri->getTicks()) break;
        if ((*i_prev)->getTicks() == ri->getTicks() &&
            ri->isEvent()                              ) break;
        i = i_prev;
    }
    if(m_current == m_all_rewind_info.end())
        m_current = m_all_rewind_info.insert(i, ri);
    else
        m_all_rewind_info.insert(i, ri);  
}   // insertRewindInfo

// ----------------------------------------------------------------------------
/** Adds an event to the rewind data. The data to be stored must be allocated
 *  and not freed by the caller!
 *  \param buffer Pointer to the event data. 
 *  \param ticks Time at which the event happened.
 */
void RewindQueue::addLocalEvent(EventRewinder *event_rewinder,
                                BareNetworkString *buffer, bool confirmed,
                                int ticks                                  )
{
    RewindInfo *ri = new RewindInfoEvent(ticks, event_rewinder,
                                         buffer, confirmed);
    insertRewindInfo(ri);
}   // addLocalEvent

// ----------------------------------------------------------------------------
/** Adds a state from the local simulation to the last created TimeStepInfo
 *  container with the current world time. It is not thread-safe, so needs
 *  to be called from the main thread.
 *  \param rewinder The rewinder object for this state.
 *  \param buffer The state information.
 *  \param confirmed If this state is confirmed to be correct (e.g. is
 *         being received from the servrer), or just a local state for
 *         faster rewinds.
 *  \param ticks Time at which the event happened.
 */
void RewindQueue::addLocalState(BareNetworkString *buffer,
                                bool confirmed, int ticks)
{
    RewindInfo *ri = new RewindInfoState(ticks, buffer, confirmed);
    assert(ri);
    insertRewindInfo(ri);
    if (confirmed && m_latest_confirmed_state_time < ticks)
    {
        cleanupOldRewindInfo(ticks);
    }
}   // addLocalState

// ----------------------------------------------------------------------------
/** Adds an event to the list of network rewind data. This function is
 *  threadsafe so can be called by the network thread. The data is synched
 *  to m_tRewindInformation list by the main thread. The data to be stored
 *  must be allocated and not freed by the caller!
 *  \param buffer Pointer to the event data.
 *  \param ticks Time at which the event happened.
 */
void RewindQueue::addNetworkEvent(EventRewinder *event_rewinder,
                                  BareNetworkString *buffer, int ticks)
{
    RewindInfo *ri = new RewindInfoEvent(ticks, event_rewinder,
                                         buffer, /*confirmed*/true);

    m_network_events.lock();
    m_network_events.getData().push_back(ri);
    m_network_events.unlock();
}   // addNetworkEvent

// ----------------------------------------------------------------------------
/** Adds a state to the list of network rewind data. This function is
 *  threadsafe so can be called by the network thread. The data is synched
 *  to RewindInfo list by the main thread. The data to be stored must be
 *  allocated and not freed by the caller!
 *  \param buffer Pointer to the event data.
 *  \param ticks Time at which the event happened.
 */
void RewindQueue::addNetworkState(BareNetworkString *buffer, int ticks)
{
    RewindInfo *ri = new RewindInfoState(ticks, buffer, /*confirmed*/true);

    m_network_events.lock();
    m_network_events.getData().push_back(ri);
    m_network_events.unlock();
}   // addNetworkState

// ----------------------------------------------------------------------------
/** Merges thread-safe all data received from the network up to and including
 *  the current time (tick) with the current local rewind information.
 *  \param world_ticks[in] Current world time up to which network events will be
 *         merged in.
 *  \param needs_rewind[out] True if a network event/state was received
 *         which was in the past (of this simulation), so a rewind must be
 *         performed.
 *  \param rewind_time[out] If needs_rewind is true, the time to which a rewind
 *         must be performed (at least). Otherwise undefined.
 */
void RewindQueue::mergeNetworkData(int world_ticks, bool *needs_rewind,
                                   int *rewind_ticks)
{
    *needs_rewind = false;
    m_network_events.lock();
    if(m_network_events.getData().empty())
    {
        m_network_events.unlock();
        return;
    }

    // Merge all newly received network events into the main event list.
    // Only a client ever rewinds. So the rewind time should be the latest
    // received state before current world time (if any)
    *rewind_ticks = -9999;

    // FIXME: making m_network_events sorted would prevent the need to 
    // go through the whole list of events
    int latest_confirmed_state = -1;
    AllNetworkRewindInfo::iterator i = m_network_events.getData().begin();
    while (i != m_network_events.getData().end())
    {
        // Ignore any events that will happen in the future. The current
        // time step is world_ticks.
        if ((*i)->getTicks() > world_ticks)
        {
            i++;
            continue;
        }
        // Any state of event that is received before the latest confirmed
        // state can be deleted.
        if ((*i)->getTicks() < m_latest_confirmed_state_time)
        {
            Log::info("RewindQueue",
                      "Deleting %s at %d because it's before confirmed state %d",
                      (*i)->isEvent() ? "event" : "state",
                      (*i)->getTicks(),
                      m_latest_confirmed_state_time);
            delete *i;
            i = m_network_events.getData().erase(i);
            continue;
        }

        // A server never rewinds (otherwise we would have to handle 
        // duplicated states, which in the best case would then have
        // a negative effect for every player, when in fact only one
        // player might have a network hickup).
        if (NetworkConfig::get()->isServer() && (*i)->getTicks() < world_ticks)
        {
            if (Network::m_connection_debug)
            {
                Log::warn("RewindQueue",
                    "Server received at %d message from %d",
                    world_ticks, (*i)->getTicks());
            }
            // Server received an event in the past. Adjust this event
            // to be executed 'now' - at least we get a bit closer to the
            // client state.
            (*i)->setTicks(world_ticks);
        }

        insertRewindInfo(*i);

        // Check if a rewind is necessary, i.e. a message is received in the
        // past of client (server never rewinds). Even if
        // getTicks()==world_ticks (which should not happen in reality, since
        // any server message should be in the client's past - but it can
        // happen during debugging) we need to rewind to getTicks (in order
        // to get the latest state).
        if (NetworkConfig::get()->isClient() &&
            (*i)->getTicks() <= world_ticks && (*i)->isState())
        {
            // We need rewind if we receive an event in the past. This will
            // then trigger a rewind later. Note that we only rewind to the
            // latest event that happened earlier than 'now' - if there is
            // more than one event in the past, we rewind to the last event.
            // Since we restore a state before the rewind, this state will
            // either include the earlier event or the state will be before
            // the earlier event, and the event will be replayed anyway. This
            // makes it easy to handle lost event messages.
            *needs_rewind = true;
            if ((*i)->getTicks() > *rewind_ticks)
                *rewind_ticks = (*i)->getTicks();
        }   // if client and ticks < world_ticks

        if ((*i)->isState() && (*i)->getTicks() > latest_confirmed_state &&
            (*i)->isConfirmed())
        {
            latest_confirmed_state = (*i)->getTicks();
        }

        i = m_network_events.getData().erase(i);
    }   // for i in m_network_events

    m_network_events.unlock();

    if (latest_confirmed_state > m_latest_confirmed_state_time)
    {
        cleanupOldRewindInfo(latest_confirmed_state);
        m_latest_confirmed_state_time = latest_confirmed_state;
    }

    if (NetworkConfig::get()->isServer())
        cleanupOldRewindInfo(world_ticks);

    // If the computed rewind time is before the last confirmed
    // state, instead rewind from the latest confirmed state.
    // This should not be necessary anymore, but I'll leave it
    // in just in case.
    if (*needs_rewind && 
        *rewind_ticks < m_latest_confirmed_state_time && 
        NetworkConfig::get()->isClient())
    {
        Log::verbose("rewindqueue",
                     "world %d rewindticks %d latest_confirmed %d",
                     World::getWorld()->getTicksSinceStart(), *rewind_ticks,
                     m_latest_confirmed_state_time);
        *rewind_ticks = m_latest_confirmed_state_time;
        *needs_rewind = m_latest_confirmed_state_time < world_ticks;
    }

}   // mergeNetworkData

// ----------------------------------------------------------------------------
/** Deletes all states and event before the given time.
 *  \param ticks Time (in ticks).
 */
void RewindQueue::cleanupOldRewindInfo(int ticks)
{
    auto i = m_all_rewind_info.begin();

    while (!m_all_rewind_info.empty() &&
        (*i)->getTicks() < ticks)
    {
        if (m_current == i) next();
        delete *i;
        i = m_all_rewind_info.erase(i);
    }

    if (m_all_rewind_info.empty())
        m_current = m_all_rewind_info.end();

}   // cleanupOldRewindInfo

// ----------------------------------------------------------------------------
bool RewindQueue::isEmpty() const
{
    return m_current == m_all_rewind_info.end();
}   // isEmpty

// ----------------------------------------------------------------------------
/** Returns true if there is at least one more RewindInfo available.
 */
bool RewindQueue::hasMoreRewindInfo() const
{
    return m_current != m_all_rewind_info.end();
}   // hasMoreRewindInfo

// ----------------------------------------------------------------------------
/** Rewinds the rewind queue and undos all events/states stored. It stops
 *  when the first confirmed state is reached that was recorded before the
 *  undo_time and sets the internal 'current' pointer to this state. 
 *  \param undo_time To what at least events need to be undone.
 *  \return The time in ticks of the confirmed state
 */
int RewindQueue::undoUntil(int undo_ticks)
{
    // A rewind is done after a state in the past is inserted. This function
    // makes sure that m_current is not end()
    //assert(m_current != m_all_rewind_info.end());
    assert(!m_all_rewind_info.empty());
    m_current = m_all_rewind_info.end();
    m_current--;
    while((*m_current)->getTicks() > undo_ticks ||
        (*m_current)->isEvent() || !(*m_current)->isConfirmed())
    {
        // Undo all events and states from the current time
        (*m_current)->undo();
        if(m_current == m_all_rewind_info.begin())
        {
            // This shouldn't happen, but add some debug info just in case
            Log::error("undoUntil",
                       "At %d rewinding to %d current = %d = begin",
                       World::getWorld()->getTicksSinceStart(), undo_ticks, 
                       (*m_current)->getTicks());
        }
        m_current--;
    }

    return (*m_current)->getTicks();
}   // undoUntil

// ----------------------------------------------------------------------------
/** Replays all events (not states) that happened at the specified time.
 *  \param ticks Time in ticks.
 */
void RewindQueue::replayAllEvents(int ticks)
{
    // Replay all events that happened at the current time step
    while ( hasMoreRewindInfo() && (*m_current)->getTicks() == ticks )
    {
        if ((*m_current)->isEvent())
            (*m_current)->replay();
        m_current++;
    }   // while current->getTIcks == ticks

}   // replayAllEvents

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
    auto dummy_rewinder = std::make_shared<DummyRewinder>();

    // First tests: add a state first, then an event, and make
    // sure the state stays first
    RewindQueue q0;
    assert(q0.isEmpty());
    assert(!q0.hasMoreRewindInfo());

    q0.addLocalState(NULL, /*confirmed*/true, 0);
    assert(q0.m_all_rewind_info.front()->isState());
    assert(!q0.m_all_rewind_info.front()->isEvent());
    assert(q0.hasMoreRewindInfo());
    assert(q0.undoUntil(0) == 0);

    q0.addNetworkEvent(dummy_rewinder.get(), NULL, 0);
    // Network events are not immediately merged
    assert(q0.m_all_rewind_info.size() == 1);

    bool needs_rewind;
    int rewind_ticks;
    int world_ticks = 0;
    q0.mergeNetworkData(world_ticks, &needs_rewind, &rewind_ticks);
    assert(q0.hasMoreRewindInfo());
    assert(q0.m_all_rewind_info.size() == 2);
    AllRewindInfo::iterator rii = q0.m_all_rewind_info.begin();
    assert((*rii)->isState());
    rii++;
    assert((*rii)->isEvent());

    // Another state must be sorted before the event:
    q0.addNetworkState(NULL, 0);
    assert(q0.hasMoreRewindInfo());
    q0.mergeNetworkData(world_ticks, &needs_rewind, &rewind_ticks);
    assert(q0.m_all_rewind_info.size() == 3);
    rii = q0.m_all_rewind_info.begin();
    assert((*rii)->isState());
    rii++;
    assert((*rii)->isState());
    rii++;
    assert((*rii)->isEvent());

    // Test time base comparisons: adding an event to the end
    q0.addLocalEvent(dummy_rewinder.get(), NULL, true, 4);
    // Then adding an earlier event
    q0.addLocalEvent(dummy_rewinder.get(), NULL, false, 1);
    // rii points to the 3rd element, the ones added just now
    // should be elements4 and 5:
    rii++;
    assert((*rii)->getTicks()==1);
    rii++;
    assert((*rii)->getTicks()==4);

    // Now test inserting an event first, then the state
    RewindQueue q1;
    q1.addLocalEvent(NULL, NULL, true, 5);
    q1.addLocalState(NULL, true, 5);
    rii = q1.m_all_rewind_info.begin();
    assert((*rii)->isState());
    rii++;
    assert((*rii)->isEvent());

    // Bugs seen before
    // ----------------
    // 1) Current pointer was not reset from end of list when an event
    //    was added and the pointer was already at end of list
    RewindQueue b1;
    b1.addLocalEvent(NULL, NULL, true, 1);
    b1.next();    // Should now point at end of list
    assert(!b1.hasMoreRewindInfo());
    b1.addLocalEvent(NULL, NULL, true, 2);
    RewindInfo *ri = b1.getCurrent();
    if (ri->getTicks() != 2)
        Log::fatal("RewindQueue", "ri->getTicks() != 2");

    // 2) Make sure when adding an event at the same time as an existing
    //    event, that m_current pooints to the first event, otherwise
    //    events with same time stamp will not be handled correctly.
    //    At this stage current points to the event at time 2 from above
    AllRewindInfo::iterator current_old = b1.m_current;
    b1.addLocalEvent(NULL, NULL, true, 2);
    // Make sure that current was not modified, i.e. the new event at time
    // 2 was added at the end of the list:
    if (current_old != b1.m_current)
        Log::fatal("RewindQueue", "current_old != b1.m_current");

    // This should not trigger an exception, now current points to the
    // second event at the same time:
    b1.next();
    assert(ri->getTicks() == 2);
    assert(ri->isEvent());
    b1.next();
    assert(b1.m_current == b1.m_all_rewind_info.end());

    // 3) Test that if cleanupOldRewindInfo is called, it will if necessary
    //    adjust m_current to point to the latest confirmed state.
    RewindQueue b2;
    b2.addNetworkState(NULL, 1);
    b2.addNetworkState(NULL, 2);
    b2.addNetworkState(NULL, 3);
    b2.mergeNetworkData(4, &needs_rewind, &rewind_ticks);
    assert((*b2.m_current)->getTicks() == 3);


}   // unitTesting
