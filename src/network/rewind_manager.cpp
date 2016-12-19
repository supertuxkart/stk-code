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

#include "network/rewind_manager.hpp"

#include "graphics/irr_driver.hpp"
#include "modes/world.hpp"
#include "network/network_string.hpp"
#include "network/rewinder.hpp"
#include "network/rewind_info.hpp"
#include "physics/physics.hpp"
#include "race/history.hpp"
#include "utils/log.hpp"

#include <algorithm>

RewindManager* RewindManager::m_rewind_manager        = NULL;
bool           RewindManager::m_enable_rewind_manager = false;

/** Creates the singleton. */
RewindManager *RewindManager::create()
{
    assert(!m_rewind_manager);
    m_rewind_manager = new RewindManager();
    return m_rewind_manager;
}   // create

// ----------------------------------------------------------------------------
/** Destroys the singleton. */
void RewindManager::destroy()
{
    assert(m_rewind_manager);
    delete m_rewind_manager;
    m_rewind_manager = NULL;
}   // destroy

// ============================================================================
/** The constructor.
 */
RewindManager::RewindManager()
{
    reset();
}   // RewindManager

// ----------------------------------------------------------------------------
/** Frees all saved state information. Note that the Rewinder data must be
 *  freed elsewhere.
 */
RewindManager::~RewindManager()
{
    // Destroying the 
    AllRewindInfo::const_iterator i;

    for(i=m_rewind_info.begin(); i!=m_rewind_info.end(); ++i)
    {
        delete *i;
    }
    m_rewind_info.clear();
}   // ~RewindManager

// ----------------------------------------------------------------------------
/** Frees all saved state information and all destroyable rewinder.
 */
void RewindManager::reset()
{
#ifdef REWIND_SEARCH_STATS
    m_count_of_comparisons = 0;
    m_count_of_searches    = 0;
#endif
    m_is_rewinding         = false;
    m_overall_state_size   = 0;
    m_state_frequency      = 0.1f;   // save 10 states a second
    m_last_saved_state     = -9999.9f;  // forces initial state save

    if(!m_enable_rewind_manager) return;

    AllRewinder::iterator r = m_all_rewinder.begin();
    while(r!=m_all_rewinder.end())
    {
        if(!(*r)->canBeDestroyed())
        {
            r++;
            continue;
        }
        Rewinder *rewinder = *r;
        r = m_all_rewinder.erase(r);
        delete rewinder;
    }

    AllRewindInfo::const_iterator i;
    for(i=m_rewind_info.begin(); i!=m_rewind_info.end(); i++)
    {
        delete *i;
    }
    m_rewind_info.clear();
    m_next_event = m_rewind_info.begin();

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
bool RewindManager::_RewindInfoCompare::operator()(const RewindInfo *ri1,
    const RewindInfo *ri2) const
{
    if (ri1->getTime() < ri2->getTime()) return true;
    if (ri1->getTime() > ri2->getTime()) return false;

    // Now the times are equal. In this case make sure that states and
    // time events are 'smallest', i.e. sorted first (which is necessary for
    // replay). Time and State events at the same time do not happen, so no
    // further test are necessary
    return ri1->isState() || ri1->isTime();
}   // RewindInfoCompare::operator()

// ----------------------------------------------------------------------------
/** Inserts a RewindInfo object in the list of all events at the correct time.
 *  If there are several RewindInfo at the exact same time, state RewindInfo
 *  will be insert at the front, and event and time info at the end of the
 *  RewindInfo with the same time.
 *  \param ri The RewindInfo object to insert.
 */
void RewindManager::insertRewindInfo(RewindInfo *ri)
{
    AllRewindInfo::iterator i = std::upper_bound(m_rewind_info.begin(),
                                                 m_rewind_info.end(), ri,
                                                 RewindManager::RewindInfoCompare);
    m_rewind_info.insert(i, ri);
}   // insertRewindInfo

// ----------------------------------------------------------------------------
/** Returns the first (i.e. lowest) index i in m_rewind_info which fulfills 
 *  time(i) < target_time <= time(i+1) and is a state. This is the state
 *  from which a rewind can start - all states for the karts will be well
 *  defined.
 *  \param time Time for which an index is searched.
 *  \return Index in m_rewind_info after which to add rewind data.
 */
RewindManager::AllRewindInfo::reverse_iterator 
                         RewindManager::findFirstIndex(float target_time)
{
    // For now do a linear search, even though m_rewind_info is sorted
    // I would expect that most insertions will be towards the (very)
    // end of the list, since rewinds should be for short periods of time. 
    // Note that after finding an entry in a binary search, you still
    // have to do a linear search to find the last entry with the same
    // time in order to minimise the later necessary memory move.

    // Gather some statistics about search for now:
#ifdef REWIND_SEARCH_STATS
    m_count_of_searches++;
#endif
    AllRewindInfo::reverse_iterator index = m_rewind_info.rbegin();
    AllRewindInfo::reverse_iterator index_last_state = m_rewind_info.rend();
    while(index!=m_rewind_info.rend())
    {
#ifdef REWIND_SEARCH_STATS
        m_count_of_comparisons++;
#endif
        if((*index)->isState())
        {
            if((*index)->getTime()<target_time)
            {
                return index;
            }
            index_last_state = index;
        }
        index++;
    }

    if(index_last_state==m_rewind_info.rend())
    {
        Log::fatal("RewindManager",
                   "Can't find any state when rewinding to %f - aborting.",
                   target_time);
    }

    // Otherwise use the last found state - not much we can do in this case.
    Log::error("RewindManager",
               "Can't find state to rewind to for time %f, using %f.",
               target_time, (*index_last_state)->getTime());
    return index_last_state;  // avoid compiler warning
}   // findFirstIndex

// ----------------------------------------------------------------------------
/** Adds an event to the rewind data. The data to be stored must be allocated
 *  and not freed by the caller!
 *  \param time Time at which the event was recorded.
 *  \param buffer Pointer to the event data. 
 */
void RewindManager::addEvent(EventRewinder *event_rewinder,
                             BareNetworkString *buffer, bool confirmed,
                             float time                                   )
{
    if(m_is_rewinding) 
    {
        delete buffer;
        Log::error("RewindManager", "Adding event when rewinding");
        return;
    }
    Log::verbose("time", "wolrd %f rewind %f", World::getWorld()->getTime(), getCurrentTime());
    if (time < 0)
        time = getCurrentTime();
    RewindInfo *ri = new RewindInfoEvent(time, event_rewinder,
                                         buffer, confirmed);
    insertRewindInfo(ri);
}   // addEvent

// ----------------------------------------------------------------------------
/** Adds an event to the list of network rewind data. This function is
 *  threadsafe so can be called by the network thread. The data is synched
 *  to m_rewind_info by the main thread. The data to be stored must be
 *  allocated and not freed by the caller!
 *  \param time Time at which the event was recorded.
 *  \param buffer Pointer to the event data.
 */
void RewindManager::addNetworkEvent(EventRewinder *event_rewinder,
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
                         RewindManager::RewindInfoCompare        );
    m_network_events.getData().insert(i, ri);
    m_network_events.unlock();
}   // addEvent

// ----------------------------------------------------------------------------
/** Determines if a new state snapshot should be taken, and if so calls all
 *  rewinder to do so.
 *  \param dt Time step size.
 */
void RewindManager::saveStates()
{
    if(!m_enable_rewind_manager  || 
        m_all_rewinder.size()==0 ||
        m_is_rewinding              )  return;
   
    float time = World::getWorld()->getTime();
    if(time - m_last_saved_state < m_state_frequency)
    {
        if (time > m_last_saved_state)
        {
            // No full state necessary, add a dummy entry for the time
            // which increases replay precision (same time step size)
            RewindInfo *ri = new RewindInfoTime(getCurrentTime());
            insertRewindInfo(ri);
            m_last_saved_state = getCurrentTime();
        }
        return;
    }

    bool was_empty = m_rewind_info.empty();

    // For now always create a snapshot.
    AllRewinder::const_iterator i;
    for(i=m_all_rewinder.begin(); i!=m_all_rewinder.end(); ++i)
    {
        BareNetworkString *buffer = (*i)->saveState();
        if(buffer && buffer->size()>=0)
        {
            m_overall_state_size += buffer->size();
            RewindInfo *ri = new RewindInfoState(getCurrentTime(),
                                                 *i, buffer,
                                                 /*is_confirmed*/true);
            assert(ri);
            insertRewindInfo(ri);
        }   // size >= 0
        else
            delete buffer;   // NULL or 0 byte buffer
    }

    if (was_empty && !m_rewind_info.empty())
        m_next_event = m_rewind_info.begin();
    Log::verbose("RewindManager", "%f allocated %ld bytes search %d/%d=%f",
                 World::getWorld()->getTime(), m_overall_state_size,
                 m_count_of_comparisons, m_count_of_searches,
                 float(m_count_of_comparisons)/ float(m_count_of_searches) );

    m_last_saved_state = time;
}   // saveStates

// ----------------------------------------------------------------------------
/** Replays all events from the last event played till the specified time.
 *  \param time Up to (and inclusive) which time events will be replayed.
 */
void RewindManager::playEventsTill(float time)
{    
    if(m_rewind_info.empty())
        return;

    /** First merge all newly received network events into the main 
     *  event list */
    bool rewind_necessary = false;
    m_network_events.lock();
    if (!m_network_events.getData().empty())
    {
        // Check if a rewind is necessary
        if (m_network_events.getData().front()->getTime() < time)
        {
            Log::info("RewindManager", "Rewind necessary at %f because of event at %f",
                       time, 
                       m_network_events.getData().front()->getTime());
        }
        // It is possible that m_next_event points m_rewind_info.end()
        // and that new elements are added to the end of the list. In this
        // case m_rewind_info end would still point to end(), but should
        // point to the newly added element. To achieve this, we first
        // decrement m_next_event (note that it was tested previously that
        // the list is not empty), merge in the new events, and then
        // increment m_next_event again. If elements have been added to
        // the end of the list, m_next_event will now corretly point to them.
        m_next_event--;
        m_rewind_info.merge(m_network_events.getData(),
                            RewindManager::RewindInfoCompare);
        m_next_event++;
    }   // if !m_network_events empty

    m_network_events.unlock();

    assert(!m_is_rewinding);
    m_is_rewinding = true;
    while (m_next_event !=m_rewind_info.end() )
    {
        RewindInfo *ri = *m_next_event;
        if (ri->getTime() > time)
        {
            m_is_rewinding = false;
            return;
        }
        m_next_event++;
        if(ri->isEvent())
            ri->rewind();
    }
    m_is_rewinding = false;
}   // playEventsTill

// ----------------------------------------------------------------------------
/** Rewinds to the specified time.
 *  \param t Time to rewind to.
 */
void RewindManager::rewindTo(float rewind_time)
{
    assert(!m_is_rewinding);
    m_is_rewinding = true;
    Log::info("rewind", "Rewinding to %f", rewind_time);
    history->doReplayHistory(History::HISTORY_NONE);

    // First find the state to which we need to rewind
    // ------------------------------------------------
    AllRewindInfo::reverse_iterator rindex = findFirstIndex(rewind_time);
    rindex--;
    AllRewindInfo::iterator index = --(rindex.base());

    if(!(*index)->isState())
    {
        Log::error("RewindManager", "No state for rewind to %f, state %d.",
                   rewind_time, index);
        return;
    }

    // Then undo the rewind infos going backwards in time
    // --------------------------------------------------
    AllRewindInfo::reverse_iterator i;
    for(i= m_rewind_info.rbegin(); i!=rindex; i++)
    {
        (*i)->undo();

        // Now all states after the time we rewind to are not confirmed
        // anymore. They need to be rewritten when going forward during
        // the rewind.
        if((*i)->isState() && 
            (*i)->getTime() > (*index)->getTime() )
            (*i)->setConfirmed(false);
    }   // for i>state


    // Rewind the required state(s)
    // ----------------------------
    World *world = World::getWorld();
    float current_time = world->getTime();

    // Get the (first) full state to which we have to rewind
    RewindInfoState *state =
                    dynamic_cast<RewindInfoState*>(*index);

    // Store the time to which we have to replay to
    float exact_rewind_time = state->getTime();

    // Now start the rewind with the full state:
    world->setTime(exact_rewind_time);
    float local_physics_time = state->getLocalPhysicsTime();
    Physics::getInstance()->getPhysicsWorld()->setLocalTime(local_physics_time);

    // Restore all states from the current time - the full state of a race
    // will be potentially stored in several state objects. State can be NULL
    // if the next event is not a state
    while(state && state->getTime()==exact_rewind_time)
    {
        state->rewind();
        index++;
        if(index==m_rewind_info.end()) break;
        state = dynamic_cast<RewindInfoState*>(*index);
    }

    // Now go forward through the list of rewind infos:
    // ------------------------------------------------
    while( world->getTime() < current_time && index !=m_rewind_info.end() )
    {
        // Now handle all states and events at the current time before
        // updating the world:
        while(index !=m_rewind_info.end() &&
              (*index)->getTime()<=world->getTime()+0.001f)
        {
            if((*index)->isState())
            {
                // TOOD: replace the old state with a new state. 
                // For now just set it to confirmed
                (*index)->setConfirmed(true);
            }
            else if((*index)->isEvent())
            {
                (*index)->rewind();
            }
            index++;
        }
        float dt = determineTimeStepSize(index, current_time);
        world->updateWorld(dt);
#define SHOW_ROLLBACK
#ifdef SHOW_ROLLBACK
        irr_driver->update(dt);
#endif
        world->updateTime(dt);

    }
    m_is_rewinding = false;

}   // rewindTo

// ----------------------------------------------------------------------------
/** Determines the next time step size to use when recomputing the physics.
 *  The time step size is either 1/60 (default physics), or less, if there
 *  is an even to handle before that time.
 *  \param next_state The next state to replay.
 *  \param end_time The end time to which we must replay forward. Don't
 *         return a dt that would be bigger tham this value.
 *  \return The time step size to use in the next simulation step.
 */
float RewindManager::determineTimeStepSize(AllRewindInfo::iterator next_state,
                                           float end_time)
{
    // If there is a next state (which is known to have a different time)
    // use the time difference to determine the time step size.
    if(next_state !=m_rewind_info.end())
        return (*next_state)->getTime() - World::getWorld()->getTime();

    // Otherwise, i.e. we are rewinding the last state/event, take the
    // difference between that time and the world time at which the rewind
    // was triggered.
    return end_time -  (*(--next_state))->getTime();

}   // determineTimeStepSize
