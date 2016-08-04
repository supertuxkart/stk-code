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
    for(unsigned int i=0; i<m_rewind_info.size(); i++)
    {
        delete m_rewind_info[i];
        m_rewind_info[i] = NULL;
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
        // FIXME Do we really want to delete this here?
        delete rewinder;
    }

    for(unsigned int i=0; i<m_rewind_info.size(); i++)
    {
        delete m_rewind_info[i];
    }
    m_rewind_info.clear();
}   // reset

// ----------------------------------------------------------------------------
void RewindManager::insertRewindInfo(RewindInfo *ri)
{
#ifdef REWIND_SEARCH_STATS
    m_count_of_searches++;
#endif
    float t = ri->getTime();

    if(ri->isEvent())
    {
        // If there are several infos for the same time t,
        // events must be inserted at the end 
        AllRewindInfo::reverse_iterator i = m_rewind_info.rbegin();
        while(i!=m_rewind_info.rend() && 
            (*i)->getTime() > t)
        {
#ifdef REWIND_SEARCH_STATS
            m_count_of_comparisons++;
#endif
            i++;
        }
        AllRewindInfo::iterator insert_point = i.base();
        m_rewind_info.insert(insert_point,ri);
        return;

    }
    else   // is a state
    {
        // If there are several infos for the same time t,
        // a state must be inserted first
        AllRewindInfo::reverse_iterator i = m_rewind_info.rbegin();
        while(i!=m_rewind_info.rend() && (*i)->getTime() >= t)
        {
#ifdef REWIND_SEARCH_STATS
            m_count_of_comparisons++;
#endif
            i++;
        }
        AllRewindInfo::iterator insert_point = i.base();
        m_rewind_info.insert(insert_point,ri);
        return;
    }
}   // insertRewindInfo

// ----------------------------------------------------------------------------
/** Returns the first (i.e. lowest) index i in m_rewind_info which fulfills 
 *  time(i) < target_time <= time(i+1) and is a state. This is the state
 *  from which a rewind can start - all states for the karts will be well
 *  defined.
 *  \param time Time for which an index is searched.
 *  \return Index in m_rewind_info after which to add rewind data.
 */
unsigned int RewindManager::findFirstIndex(float target_time) const
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
    int index = m_rewind_info.size()-1;
    int index_last_state = -1;
    while(index>=0)
    {
#ifdef REWIND_SEARCH_STATS
        m_count_of_comparisons++;
#endif
        if(m_rewind_info[index]->isState())
        {
            if(m_rewind_info[index]->getTime()<target_time)
            {
                return index;
            }
            index_last_state = index;
        }
        index--;
    }

    if(index_last_state<0)
    {
        Log::fatal("RewindManager",
                   "Can't find any state when rewinding to %f - aborting.",
                   target_time);
    }

    // Otherwise use the last found state - not much we can do in this case.
    Log::error("RewindManager",
               "Can't find state to rewind to for time %f, using %f.",
               target_time, m_rewind_info[index_last_state]->getTime());
    return index_last_state;  // avoid compiler warning
}   // findFirstIndex

// ----------------------------------------------------------------------------
/** Adds an event to the rewind data. The data to be stored must be allocated
 *  and not freed by the caller!
 *  \param time Time at which the event was recorded.
 *  \param buffer Pointer to the event data. 
 */
void RewindManager::addEvent(Rewinder *rewinder, BareNetworkString *buffer)
{
    if(m_is_rewinding) return;
    RewindInfo *ri = new RewindInfoEvent(getCurrentTime(), rewinder,
                                         buffer, /*is_confirmed*/true   );
    insertRewindInfo(ri);
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
        // No full state necessary, add a dummy entry for the time
        // which increases replay precision (same time step size)
        RewindInfo *ri = new RewindInfoTime(getCurrentTime());
        insertRewindInfo(ri);
        return;
    }

    // For now always create a snapshot.
    for(unsigned int i=0; i<m_all_rewinder.size(); i++)
    {
        BareNetworkString *buffer = m_all_rewinder[i]->getState();
        if(buffer && buffer->size()>=0)
        {
            m_overall_state_size += buffer->size();
            RewindInfo *ri = new RewindInfoState(getCurrentTime(),
                                                 m_all_rewinder[i], buffer,
                                                 /*is_confirmed*/true);
            assert(ri);
            insertRewindInfo(ri);
        }   // size >= 0
        else
            delete buffer;   // NULL or 0 byte buffer
    }

    Log::verbose("RewindManager", "%f allocated %ld bytes search %d/%d=%f",
                 World::getWorld()->getTime(), m_overall_state_size,
                 m_count_of_comparisons, m_count_of_searches,
                 float(m_count_of_comparisons)/ float(m_count_of_searches) );

    m_last_saved_state = time;
}   // saveStates

// ----------------------------------------------------------------------------
/** Rewinds to the specified time.
 *  \param t Time to rewind to.
 */
void RewindManager::rewindTo(float rewind_time)
{
    assert(!m_is_rewinding);
    m_is_rewinding = true;

    history->doReplayHistory(History::HISTORY_NONE);

    // First find the state to which we need to rewind
    // ------------------------------------------------
    int state_index = findFirstIndex(rewind_time);

    if(!m_rewind_info[state_index]->isState())
    {
        Log::error("RewindManager", "No state for rewind to %f, state %d.",
                   rewind_time, state_index);
        return;
    }

    // Then undo the states that are skipped
    // -------------------------------------
    for(int i=m_rewind_info.size()-1; i>(int)state_index; i--)
    {
        m_rewind_info[i]->undo();
    }   // for i>state


    // TODO: we need some logic here to handle that confirmed states are not
    // necessarily the same for each rewinder. So if we rewind to t, one
    // rewinder forces us to go back to t1 (latest state saved before t),
    // another one to t2. 
    // So we have to detect the latest time t_min < t for which each 
    // rewinder has a confirmed state. Then we rewind from t_min. But if we 
    // find another confirmed state for a rewinder at time t1 (t_min<t1<t), 
    // we have to overwrite the current state of the rewinder with that at
    // time t1. 
    // Also care needs to be taken with events: e.g. either we make steering
    // information (for kars) part of the state, or we have to go even further
    // back in time (before t_min) to find what state steering was in at time
    // t_min.

    // Rewind to the required state
    // ----------------------------

    RewindInfoState *state =
        dynamic_cast<RewindInfoState*>(m_rewind_info[state_index]);
    float exact_rewind_time = state->getTime();
    float local_physics_time = state->getLocalPhysicsTime();
    World *world = World::getWorld();
    world->getPhysics()->getPhysicsWorld()->setLocalTime(local_physics_time);

    // Rewind all objects (and also all state) that happen at the
    // current time. 
    // TODO: this assumes atm that all rewinder have an initial state
    // for 'current_time'!!!

    // Store the time to which we have to replay to
    float current_time = world->getTime();

    world->setTime(exact_rewind_time);

    // Now go forward through the saved states, and
    // replay taking the events into account.
    while( world->getTime() < current_time &&
          state_index < (int)m_rewind_info.size()                 )
    {

        // Now set all events and confirmed states
        while(state_index < (int)m_rewind_info.size() &&
            m_rewind_info[state_index]->getTime()<=world->getTime()+0.001f)
        {
            if(m_rewind_info[state_index]->isEvent() ||
               m_rewind_info[state_index]->isConfirmed() )
            {
                m_rewind_info[state_index]->rewind();
            }
            state_index++;
        }
        float dt = determineTimeStepSize(state_index, current_time);
        world->updateWorld(dt);
#define SHOW_ROLLBACK
#ifdef SHOW_ROLLBACK
        irr_driver->update(dt);
#endif

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
float RewindManager::determineTimeStepSize(int next_state, float end_time)
{
    // If there is a next state (which is known to have a different time)
    // use the time difference to determine the time step size.
    if(next_state < (int)m_rewind_info.size())
        return m_rewind_info[next_state]->getTime() - World::getWorld()->getTime();

    // Otherwise, i.e. we are rewinding the last state/event, take the
    // difference between that time and the world time at which the rewind
    // was triggered.
    return end_time -  m_rewind_info[next_state-1]->getTime();



    float dt = 1.0f/60.0f;
    float t = World::getWorld()->getTime();
    if(m_rewind_info[next_state]->getTime() < t + dt)
    {
        // Since we have RewindInfo at that time, it is certain that
        /// this time is before (or at) end_time, not after.
        return m_rewind_info[next_state]->getTime()-t;
    }
    return t+dt < end_time ? dt : end_time - t;
}   // determineTimeStepSize
