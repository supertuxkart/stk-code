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
#include "network/network_config.hpp"
#include "network/rewind_info.hpp"
#include "network/rewind_manager.hpp"
#include "network/time_step_info.hpp"

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
    m_current = m_time_step_info.begin();
    reset();
}   // RewindQueue

// ----------------------------------------------------------------------------
/** Frees all saved state information. Note that the Rewinder data must be
 *  freed elsewhere.
 */
RewindQueue::~RewindQueue()
{
    // Destroying the 
    AllTimeStepInfo::const_iterator i;

    for(i=m_time_step_info.begin(); i!=m_time_step_info.end(); ++i)
    {
        delete *i;
    }
    m_time_step_info.clear();
}   // ~RewindQueue

// ----------------------------------------------------------------------------
/** Frees all saved state information and all destroyable rewinder.
 */
void RewindQueue::reset()
{

    for(AllTimeStepInfo::const_iterator i =m_time_step_info.begin();
                                        i!=m_time_step_info.end(); i++)
    {
        delete *i;
    }

    m_time_step_info.clear();
    m_current = m_time_step_info.begin();

    m_network_events.lock();

    AllNetworkRewindInfo &info = m_network_events.getData();
    for (AllNetworkRewindInfo::const_iterator i  = info.begin(); 
                                              i != info.end(); ++i)
    {
        delete *i;
    }
    m_network_events.getData().clear();
    m_network_events.unlock();
}   // reset

// ----------------------------------------------------------------------------
/** Adds a new TimeStepInfo for the specified time. The TimeStepInfo acts
 *  as an container to store all states and events that happen at this time
 *  (or at least close to this time, since e.g. several events from clients
 *  happening at slightly different times will be all handled in the same
 *  timestep.
 *  \param time New time to add.
 *  \param dt Time step size that is going to be used for this time step.
 */
void RewindQueue::addNewTimeStep(int ticks, float dt)
{
    TimeStepInfo *tsi = new TimeStepInfo(ticks, dt);
    assert(m_time_step_info.empty()                 ||
           ticks > m_time_step_info.back()->getTicks()  );
    m_time_step_info.push_back(tsi);

    // If current was not initialised
    if (m_current == m_time_step_info.end())
    {
        m_current--;
    }
}   // addNewTimeStep

// ----------------------------------------------------------------------------
/** Finds the TimeStepInfo object to which an event at time t should be added.
 *  The TimeStepInfo object might not have the exacct same time, it can be
 *  the closest existing (or in future this function might even add a totally
 *  new TimeStepInfo object).
 *  \param Time at which the event that needs to be added hapened.
 */
RewindQueue::AllTimeStepInfo::iterator 
             RewindQueue::findPreviousTimeStepInfo(int ticks)
{
    AllTimeStepInfo::iterator i = m_time_step_info.end();
    while(i!=m_time_step_info.begin())
    {
        i--;
        if ((*i)->getTicks() <= ticks) return i;
    } 
    return i;
}   // findPreviousTimeStepInfo

// ----------------------------------------------------------------------------
/** A compare function used when sorting the event lists. It sorts events by
 *  time. In case of equal times, it sorts states and events first (since the
 *  state needs to be restored when replaying first before any other events).
 */
bool RewindQueue::_TimeStepInfoCompare::operator()(const TimeStepInfo * const ri1,
                                                   const TimeStepInfo * const ri2) const
{
    return ri1->getTicks() < ri2->getTicks();
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
    // FIXME: this should always be the last element in the list(??)
    AllTimeStepInfo::iterator bucket = findPreviousTimeStepInfo(ri->getTicks());

    // FIXME: In case of a history replay an element could be inserted in the
    // very first frame (on very quick recorded start, and if the first frame
    // takes a long time - e.g. in networking startup), i.e. before a TimeStep
    // info was added. Since this is mostly for debugging, just ignore this
    // this for now.
    if(bucket!=m_time_step_info.end())
        (*bucket)->insert(ri);
}   // insertRewindInfo

// ----------------------------------------------------------------------------
/** Adds an event to the rewind data. The data to be stored must be allocated
 *  and not freed by the caller!
 *  \param time Time at which the event was recorded.
 *  \param buffer Pointer to the event data. 
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
 *  \param time Time at which the state was captured.
 */
void RewindQueue::addLocalState(Rewinder *rewinder, BareNetworkString *buffer,
                                bool confirmed, int ticks)
{
    RewindInfo *ri = new RewindInfoState(ticks, rewinder, buffer, confirmed);
    assert(ri);
    insertRewindInfo(ri);
}   // addLocalState

// ----------------------------------------------------------------------------
/** Adds an event to the list of network rewind data. This function is
 *  threadsafe so can be called by the network thread. The data is synched
 *  to m_time_step_info by the main thread. The data to be stored must be
 *  allocated and not freed by the caller!
 *  \param time Time at which the event was recorded.
 *  \param buffer Pointer to the event data.
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
 *  to m_time_step_info by the main thread. The data to be stored must be
 *  allocated and not freed by the caller!
 *  \param time Time at which the event was recorded.
 *  \param buffer Pointer to the event data.
 */
void RewindQueue::addNetworkState(Rewinder *rewinder, BareNetworkString *buffer,
                                  int ticks, float dt)
{
    RewindInfo *ri = new RewindInfoState(ticks, rewinder,
                                         buffer, /*confirmed*/true);

    m_network_events.lock();
    m_network_events.getData().push_back(ri);
    m_network_events.unlock();
}   // addNetworkState

// ----------------------------------------------------------------------------
/** Merges thread-safe all data received from the network with the current
 *  local rewind information.
 *  \param world_ticks[in] Current world time up to which network events will be
 *         merged in.
 *  \param needs_rewind[out] True if network rewind information was received
 *         which was in the past (of this simulation), so a rewind must be
 *         performed.
 *  \param rewind_time[out] If needs_rewind is true, the time to which a rewind
 *         must be performed (at least). Otherwise undefined, but the value
 *         might be modified in this function.
 */
void RewindQueue::mergeNetworkData(int world_ticks,
                                   bool *needs_rewind, int *rewind_ticks)
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
    bool adjust_next = false;

    // FIXME: making m_network_events sorted would prevent the need to 
    // go through the whole list of events
    AllNetworkRewindInfo::iterator i = m_network_events.getData().begin();
    while( i!=m_network_events.getData().end() )
    {
        // Ignore any events that will happen in the future. An event needs
        // to be handled at the closest time to its original time. The current
        // time step is world_ticks.
        if ((*i)->getTicks() > world_ticks)
        {
            i++;
            continue;
        }
        // A server never rewinds (otherwise we would have to handle 
        // duplicated states, which in the best case would then have
        // a negative effect for every player, when in fact only one
        // player might have a network hickup).
        if (NetworkConfig::get()->isServer() && (*i)->getTicks() < world_ticks)
        {
            Log::warn("RewindQueue", "At %d received message from %d",
                      world_ticks, (*i)->getTicks());
            // Server received an event in the past. Adjust this event
            // to be executed now - at least we get a bit closer to the
            // client state.
            (*i)->setTicks(world_ticks);
        }

        // Find closest previous time step.
        AllTimeStepInfo::iterator prev =
                         findPreviousTimeStepInfo((*i)->getTicks());
        AllTimeStepInfo::iterator next = prev;
        next++;

        int event_ticks = (*i)->getTicks();

        TimeStepInfo *tsi;

        // Assign this event to the closest of the two existing timesteps
        // prev and next (inserting an additional event in the past would
        // mean more CPU work in the rewind this will very likely trigger).
        if (next == m_time_step_info.end())
            tsi = *prev;
        else if ( (*next)->getTicks()-event_ticks < event_ticks-(*prev)->getTicks() )
            tsi = *next;
        else
            tsi = *prev;

        tsi->insert(*i);
        Log::info("Rewind", "Inserting event from time %d type %c to timstepinfo %d prev %d next %d",
                  (*i)->getTicks(), 
                  (*i)->isEvent() ? 'E' : ((*i)->isState() ? 'S' : 'T'),
                  tsi->getTicks(),
                  (*prev)->getTicks(), 
                  next != m_time_step_info.end() ? (*next)->getTicks() : 9999 );

        // Check if a rewind is necessary: either an message arrived in the past
        // or if the time is between world_time and world_time+dt (otherwise
        // the message would have been ignored further up), 'rewind' to this new
        // state anyway
        if (NetworkConfig::get()->isClient())
        {
            // We need rewind if we either receive an event in the past
            // (FIXME: maybe we can just ignore this since we will also get
            // a state update??), or receive a state from the current time
            // (i.e. between world_time and world_time+dt). In the latter
            // case we can just 'rewind' to this stage instead of doing a
            // full simulation - though this client should potentially 
            // speed up a bit: if it receives a state from the server
            // at the time the client is currently simulating (instead of
            // triggering a rollback) it is not ahead enough of the server
            // which will trigger a time adjustment from the server anyway.
            if (tsi->getTicks() < world_ticks ||
                (*i)->isState() && tsi == m_time_step_info.back())
            {
                *needs_rewind = true;
                if (tsi->getTicks() > *rewind_ticks) *rewind_ticks = tsi->getTicks();
            }
        }   // if client
        i = m_network_events.getData().erase(i);
    }   // for i in m_network_events

    m_network_events.unlock();

}   // mergeNetworkData

// ----------------------------------------------------------------------------
bool RewindQueue::isEmpty() const
{
    return m_time_step_info.empty();
}   // isEmpty

// ----------------------------------------------------------------------------
/** Returns true if there is at least one more RewindInfo available.
 */
bool RewindQueue::hasMoreRewindInfo() const
{
    return m_current != m_time_step_info.end();
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
float RewindQueue::determineNextDT()
{
    return stk_config->ticks2Time(1);
}   // determineNextDT

// ----------------------------------------------------------------------------
/** Rewinds the rewind queue and undos all events/states stored. It stops
 *  when the first confirmed state is reached that was recorded before the
 *  undo_time and sets the internal 'current' pointer to this state. It is
 *  assumed that this function is called after a new TimeStepInfo instance
 *  was added (i.e. after RewindManager::update() was called), so the state
 *  m_current is pointing to is ignored.
 *  \param undo_time To what at least events need to be undone.
 */
void RewindQueue::undoUntil(int undo_ticks)
{
    while (m_current != m_time_step_info.begin())
    {
        --m_current;
        // Undo all events and states from the current time
        (*m_current)->undoAll();

        if ((*m_current)->getTicks() <= undo_ticks &&
            (*m_current)->hasConfirmedState())
        {
            return;
        }

    }   // while m_current!=m_time_step_info.begin()

    Log::error("RewindManager", "No state for rewind to %d",
               undo_ticks);

}   // undoUntil

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
        virtual void saveTransform() {}
        virtual void computeError() {}
        DummyRewinder() : Rewinder(true) {}
    };
    DummyRewinder *dummy_rewinder = new DummyRewinder();

    RewindQueue q0;
    assert(q0.isEmpty());
    assert(!q0.hasMoreRewindInfo());

    q0.addNewTimeStep(0, 0.5f);
    q0.m_current = q0.m_time_step_info.begin();
    assert(!q0.isEmpty());
    assert(q0.hasMoreRewindInfo());
    assert(q0.m_time_step_info.size() == 1);

    assert((*q0.m_time_step_info.begin())->getNumberOfEvents() == 0);
    q0.addLocalState(NULL, NULL, true, 0);
    assert((*q0.m_time_step_info.begin())->getNumberOfEvents() == 1);

    q0.addNewTimeStep(1, 0.5f);
    assert(q0.m_time_step_info.size() == 2);

    q0.addNetworkEvent(dummy_rewinder, NULL, 0);

    bool needs_rewind;
    int rewind_ticks;
    int world_ticks = 0;
    float dt = 0.01f;
    assert((*q0.m_time_step_info.begin())->getNumberOfEvents() == 1);
    q0.mergeNetworkData(world_ticks, &needs_rewind, &rewind_ticks);
    assert((*q0.m_time_step_info.begin())->getNumberOfEvents() == 2);

    // This will be added to timestep 0
    q0.addNetworkEvent(dummy_rewinder, NULL, 2);
    dt = 0.01f;   // to small, event from 0.2 will not be merged
    q0.mergeNetworkData(world_ticks, &needs_rewind, &rewind_ticks);
    assert(q0.m_time_step_info.size() == 2);
    assert((*q0.m_time_step_info.begin())->getNumberOfEvents() == 2);
    dt = 0.3f;
    q0.mergeNetworkData(world_ticks, &needs_rewind, &rewind_ticks);
    assert(q0.m_time_step_info.size() == 2);
    assert((*q0.m_time_step_info.begin())->getNumberOfEvents() == 3);

    // This event will get added to the last time step info at 1.0:
    q0.addNetworkEvent(dummy_rewinder, NULL, 1);
    world_ticks = 8;
    dt = 0.3f;
    q0.mergeNetworkData(world_ticks, &needs_rewind, &rewind_ticks);
    // Note that end() is behind the list, i.e. invalid, but rbegin()
    // is the last element
    assert((*q0.m_time_step_info.rbegin())->getNumberOfEvents() == 1);

    // Bugs seen before
    // ----------------
    // 1) Current pointer was not reset from end of list when an event
    //    was added and the pointer was already at end of list
    RewindQueue b1;
    b1.addNewTimeStep(1, 0.1f);
    ++b1;    // Should now point at end of list
    b1.hasMoreRewindInfo();
    b1.addNewTimeStep(2, 0.1f);
    TimeStepInfo *tsi = b1.getCurrent();
    assert(tsi->getTicks() == 2);
}   // unitTesting
