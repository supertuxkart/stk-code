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
#include "network/rewind_info.hpp"
#include "network/rewind_manager.hpp"
#include "network/time_step_info.hpp"

#include <algorithm>

/** The constructor.
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
void RewindQueue::addNewTimeStep(float time, float dt)
{
    TimeStepInfo *tsi = new TimeStepInfo(time, dt);

    AllTimeStepInfo::iterator i = m_time_step_info.end();
    while (i != m_time_step_info.begin())
    {
        --i;
        if ((*i)->getTime() < time)
        {
            i++;
            break;
        }
    };

    // If current was not initialised
    AllTimeStepInfo::iterator new_tsi = m_time_step_info.insert(i, tsi);
    if (m_current == m_time_step_info.end())
        m_current = new_tsi;
}   // addNewTimeStep

// ----------------------------------------------------------------------------
/** Finds the TimeStepInfo object to which an event at time t should be added.
 *  The TimeStepInfo object might not have the exacct same time, it can be
 *  the closest existing (or in future this function might even add a totally
 *  new TimeStepInfo object).
 *  \param Time at which the event that needs to be added hapened.
 */
RewindQueue::AllTimeStepInfo::iterator 
             RewindQueue::findPreviousTimeStepInfo(float t)
{
    AllTimeStepInfo::iterator i = m_time_step_info.end();
    while(i!=m_time_step_info.begin())
    {
        i--;
        if ((*i)->getTime() <= t) return i;
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
    return ri1->getTime() < ri2->getTime();
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
    AllTimeStepInfo::iterator bucket = findPreviousTimeStepInfo(ri->getTime());
    Log::verbose("RewindQueue", "Insert rewind from %f at %f",
                 ri->getTime(), (*bucket)->getTime());
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
                                float time                                   )
{
    RewindInfo *ri = new RewindInfoEvent(time, event_rewinder,
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
                                bool confirmed, float time)
{
    RewindInfo *ri = new RewindInfoState(time, rewinder, buffer, confirmed);
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
                                    BareNetworkString *buffer, float time)
{
    RewindInfo *ri = new RewindInfoEvent(time, event_rewinder,
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
                                  float time, float dt)
{
    RewindInfo *ri = new RewindInfoState(time, rewinder,
                                         buffer, /*confirmed*/true);

    m_network_events.lock();
    m_network_events.getData().push_back(ri);
    m_network_events.unlock();
}   // addNetworkState

// ----------------------------------------------------------------------------
/** Merges thread-safe all data received from the network with the current
 *  local rewind information.
 *  \param world_time Current world time up to which network events will be
 *         merged in.
 *  \param needs_rewind True if network rewind information was received which
 *         was in the past (of this simulation), so a rewind must be performed.
 *  \param rewind_time If needs_rewind is true, the time to which a rewind must
 *         be performed (at least). Otherwise undefined, but the value might
 *         be modified in this function.
 */
void RewindQueue::mergeNetworkData(float world_time, float dt,
                                   bool *needs_rewind, float *rewind_time)
{
    *needs_rewind = false;
    m_network_events.lock();
    Log::verbose("RewindQueue", "#events in queue at %f %f are %d",
        world_time, dt, m_network_events.getData().size());
    if(m_network_events.getData().empty())
    {
        m_network_events.unlock();
        return;
    }

    /* Merge all newly received network events into the main event list. */

    *rewind_time = 99999.9f;
    bool adjust_next = false;

    // FIXME: making m_network_events sorted would prevent the need to 
    // go through the whole list of events
    AllNetworkRewindInfo::iterator i = m_network_events.getData().begin();
    while( i!=m_network_events.getData().end() )
    {
        // Ignore any events that will happen in the future.
        if ((*i)->getTime() > world_time+dt)
        {
            i++;
            continue;
        }

        // Find closest previous time step.
        AllTimeStepInfo::iterator prev =
                         findPreviousTimeStepInfo((*i)->getTime());
        AllTimeStepInfo::iterator next = prev;
        next++;

        float event_time = (*i)->getTime();

        TimeStepInfo *tsi;

        // Assign this event to the closest of the two existing timesteps
        // prev and next (inserting an additional event in the past would
        // mean more CPU work in the rewind this will very likely trigger.
        if (next == m_time_step_info.end())
            tsi = *prev;
        else if ( (*next)->getTime()-event_time < event_time-(*prev)->getTime() )
            tsi = *next;
        else
            tsi = *prev;

        tsi->insert(*i);

        Log::info("Rewind", "Inserting event from time %f to timstepinfo %f prev %f next %f",
                  (*i)->getTime(), tsi->getTime(),
                  (*prev)->getTime(), 
                  next != m_time_step_info.end() ? (*next)->getTime() : 9999 );


        // Check if a rewind is necessary
        if (tsi->getTime() < world_time ||
            (*i)->isState() && tsi == m_time_step_info.back())
        {
            *needs_rewind = true;
            if (tsi->getTime() < *rewind_time) *rewind_time = tsi->getTime();
        }
        i = m_network_events.getData().erase(i);
    }   // for i in m_network_events

    m_network_events.unlock();

}   // mergeNetworkData

// ----------------------------------------------------------------------------
bool RewindQueue::isEmpty() const
{
    if (m_current != m_time_step_info.end())
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
float RewindQueue::determineNextDT(float end_time)
{
    // If there is a next state (which is known to have a different time)
    // use the time difference to determine the time step size.
    if(m_current !=m_time_step_info.end())
        return (*m_current)->getTime() - World::getWorld()->getTime();

    // Otherwise, i.e. we are rewinding the last state/event, take the
    // difference between that time and the world time at which the rewind
    // was triggered.
    return end_time -  (*(--m_current))->getTime();

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
void RewindQueue::undoUntil(float undo_time)
{
    while (m_current != m_time_step_info.begin())
    {
        --m_current;
        // Undo all events and states from the current time
        (*m_current)->undoAll();

        if ((*m_current)->getTime() <= undo_time &&
            (*m_current)->hasConfirmedState())
        {
            return;
        }

    }   // while m_current!=m_time_step_info.begin()

    Log::error("RewindManager", "No state for rewind to %f",
               undo_time);

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
        DummyRewinder() : Rewinder(true) {}
    };
    DummyRewinder *dummy_rewinder = new DummyRewinder();

    RewindQueue q0;
    assert(q0.isEmpty());
    assert(!q0.hasMoreRewindInfo());

    q0.addNewTimeStep(0.0f, 0.5f);
    q0.m_current = q0.m_time_step_info.begin();
    assert(!q0.isEmpty());
    assert(q0.hasMoreRewindInfo());
    assert(q0.m_time_step_info.size() == 1);

    assert((*q0.m_time_step_info.begin())->getNumberOfEvents() == 0);
    q0.addLocalState(NULL, NULL, true, 0.0f);
    assert((*q0.m_time_step_info.begin())->getNumberOfEvents() == 1);

    q0.addNewTimeStep(1.0f, 0.5f);
    assert(q0.m_time_step_info.size() == 2);

    q0.addNetworkEvent(dummy_rewinder, NULL, 0.0f);

    bool needs_rewind;
    float rewind_time;
    float world_time = 0.0f;
    float dt = 0.01f;
    assert((*q0.m_time_step_info.begin())->getNumberOfEvents() == 1);
    q0.mergeNetworkData(world_time, dt, &needs_rewind, &rewind_time);
    assert((*q0.m_time_step_info.begin())->getNumberOfEvents() == 2);

    // This will be added to timestep 0
    q0.addNetworkEvent(dummy_rewinder, NULL, 0.2f);
    dt = 0.01f;   // to small, event from 0.2 will not be merged
    q0.mergeNetworkData(world_time, dt, &needs_rewind, &rewind_time);
    assert(q0.m_time_step_info.size() == 2);
    assert((*q0.m_time_step_info.begin())->getNumberOfEvents() == 2);
    dt = 0.3f;
    q0.mergeNetworkData(world_time, dt, &needs_rewind, &rewind_time);
    assert(q0.m_time_step_info.size() == 2);
    assert((*q0.m_time_step_info.begin())->getNumberOfEvents() == 3);

    // This event will get added to the last time step info at 1.0:
    q0.addNetworkEvent(dummy_rewinder, NULL, 1.0f);
    world_time = 0.8f;
    dt = 0.3f;
    q0.mergeNetworkData(world_time, dt, &needs_rewind, &rewind_time);
    // Note that end() is behind the list, i.e. invalid, but rbegin()
    // is the last element
    assert((*q0.m_time_step_info.rbegin())->getNumberOfEvents() == 1);

    // Bugs seen before
    // ----------------
    // 1) Current pointer was not reset from end of list when an event
    //    was added and the pointer was already at end of list
    RewindQueue b1;
    b1.addNewTimeStep(1.0f, 0.1f);
    ++b1;    // Should now point at end of list
    b1.hasMoreRewindInfo();
    b1.addNewTimeStep(2.0f, 0.1f);
    TimeStepInfo *tsi = b1.getCurrent();
    assert(tsi->getTime() == 2.0f);
}   // unitTesting
