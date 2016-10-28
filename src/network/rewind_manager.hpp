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

#ifndef HEADER_REWIND_MANAGER_HPP
#define HEADER_REWIND_MANAGER_HPP

#include "network/rewinder.hpp"
#include "utils/ptr_vector.hpp"

#include <assert.h>
#include <vector>

class RewindInfo;
class EventRewinder;

/** \ingroup network
 *  This class manages rewinding. It keeps track of:
 *  -  states for each rewindable object (for example a kart would have
 *     its position, rotation, linear and angular velocity etc as state)
 *     States can be confirmed (i.e. were received by the network server
 *     and are therefore confirmed to be conrrect), or not (just a snapshot
 *     on this client, which can save time in rewinding later).
 *  -  events for each rewindable object (for example any change in the kart
 *     controls, like steering, fire, ... are an event). While states can be
 *     discarded (especially unconfirmed ones), e.g. to save space, events
 *     will always be kept (in order to allow replaying).
 *  For each object that is to be rewinded an instance of Rewinder needs to be
 *  declared (usually inside of the object it can rewind). This instance
 *  is automatically registered with the RewindManager.
 *  All states and events are stored in a RewindInfo object. All RewindInfo
 *  objects are stored in a list sorted by time.
 *  When a rewind to time T is requested, the following takes place:
 *  1. Go back in time:
 *     Determine the latest time t_min < T so that each rewindable objects
 *     has at least one state before T. For each state that is skipped during
 *     this process `undoState()` is being called, and for each event
 *     `undoEvent()` of the Rewinder.
 *  2. Restore state at time `t_min`
 *     For each Rewinder the state at time t_min is restored by calling
 *     `rewindToState(char *)`.
 *     TODO: atm there is no guarantee that each object will have a state
 *     at a given time. We either need to work around that, or make sure
 *     to store at least an unconfirmed state whenever we receive a 
 *     confirmed state.
 *  3. Rerun the simulation till the current time t_current is reached:
 *     1. Determine the time `t_next` of the next frame. This is either
 *        current_time + 1/60 (physics default time step size), or less
 *        if RewindInfo at an earlier time is available).
 *        This determines the time step size for the next frame (i.e.
 *        `t_next - t_current`).
 *     2. For all RewindInfo at time t_next call:
 *        - `restoreState()` if the RewindInfo is a confirmed state
 *        - `discardState()` if the RewindInfo is an unconfirmed state
 *          TODO: still missing, and instead of discard perhaps
 *                store a new state??
 *        - `rewindToEvent()` if the RewindInfo is an event
 *     3. Do one step of world simulation, using the updated (confirmed)
 *        states and newly set events (e.g. kart input).
 */

class RewindManager
{
private:
    /** Singleton pointer. */
    static RewindManager *m_rewind_manager;

    /** En- or Disable the rewind manager. This is used to disable storing
     *  rewind data in case of local races only. */
    static bool           m_enable_rewind_manager;

    typedef std::vector<Rewinder *> AllRewinder;

    /** A list of all objects that can be rewound. */
    AllRewinder m_all_rewinder;

    /** Pointer to all saved states. */
    typedef std::vector<RewindInfo*> AllRewindInfo;

    AllRewindInfo m_rewind_info;

    /** Overall amount of memory allocated by states. */
    unsigned int m_overall_state_size;

    /** Indicates if currently a rewind is happening. */
    bool m_is_rewinding;

    /** How much time between consecutive state saves. */
    float m_state_frequency;

    /** Time at which the last state was saved. */
    float m_last_saved_state;

    /** The current time to be used in all states/events. This is used to
     *  give all states and events during one frame the same time, even
     *  if e.g. states are saved before world time is increased, other
     *  events later. */
    float m_current_time;

    /** The current time step size. */
    float m_time_step;

#define REWIND_SEARCH_STATS

#ifdef REWIND_SEARCH_STATS
    /** Gather some statistics about how many comparisons we do, 
     *  to find out if it's worth doing a binary search.*/
    mutable int m_count_of_comparisons;
    mutable int m_count_of_searches;
#endif

    RewindManager();
    ~RewindManager();
    unsigned int findFirstIndex(float time) const;
    void insertRewindInfo(RewindInfo *ri);
    float determineTimeStepSize(int state, float max_time);
public:
    // First static functions to manage rewinding.
    // ===========================================
    static RewindManager *create();
    static void destroy();
    // ------------------------------------------------------------------------
    /** Sets the time that is to be used for all further states or events,
     *  and the time step size. This is necessary so that states/events before 
     *  and after World::m_time is increased have the same time stamp. 
     *  \param t Time.
     *  \param dt Time step size.
     */
    void setCurrentTime(float t, float dt)
    {
        m_current_time = t;
        m_time_step    = dt;
    }   // setCurrentTime

    // ------------------------------------------------------------------------
    /** Returns the current time. */
    float getCurrentTime() const { return m_current_time; }
    // ------------------------------------------------------------------------
    float getCurrentTimeStep() const { return m_time_step; }
    // ------------------------------------------------------------------------
    /** En- or disables rewinding. */
    static void setEnable(bool m) { m_enable_rewind_manager = m;}

    // ------------------------------------------------------------------------
    /** Returns if rewinding is enabled or not. */
    static bool isEnabled() { return m_enable_rewind_manager; }
        
    // ------------------------------------------------------------------------
    /** Returns the singleton. This function will not automatically create 
     *  the singleton. */
    static RewindManager *get()
    {
        assert(m_rewind_manager);
        return m_rewind_manager;
    }   // get

    // ------------------------------------------------------------------------

    void reset();
    void saveStates();
    void rewindTo(float target_time);
    void addEvent(EventRewinder *event_rewinder, BareNetworkString *buffer);
    // ------------------------------------------------------------------------
    /** Adds a Rewinder to the list of all rewinders.
     *  \return true If rewinding is enabled, false otherwise. 
     */
    bool addRewinder(Rewinder *rewinder)
    {
        if(!m_enable_rewind_manager) return false;
        m_all_rewinder.push_back(rewinder);
        return true;
    }   // addRewinder
    // ------------------------------------------------------------------------
    /** Returns true if currently a rewind is happening. */
    bool isRewinding() const { return m_is_rewinding; }
};   // RewindManager


#endif

