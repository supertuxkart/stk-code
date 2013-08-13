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

    /** Overall amount of memory allocated by states. */
    unsigned int m_overall_state_size;

    // ========================================================================
    /** Used to store rewind information for a given time for all rewind 
    *  instances. 
     *  Rewind information can either be a state (for example a kart would 
     *  have position, rotation, linear and angular velocity, ... as state),
     *  or an event (for a kart that would be pressing or releasing of a key).
     *  State changes and events can be delivered in different frequencies,
     *  and might be released (to save memory) differently: A state can be
     *  reproduced from a previous state by replaying the simulation taking
     *  all events into account.
     */
    class RewindInfo
    {
    private:
        /** Pointer to the buffer which stores all states. */
        char *m_buffer; 

        /** Time when this state was taken. */
        float m_time;

        /** True if this is an event, and not a state. */
        bool m_is_event;

        /** A confirmed event is one that was sent from the server. When
         *  rewinding we have to start with a confirmed state for each 
         *  object.  */
        bool m_is_confirmed;

        /** The Rewinder instance for which this data is. */
        Rewinder *m_rewinder;
    public:
        RewindInfo(Rewinder *rewinder, float time, char *buffer, 
                   bool is_event, bool is_confirmed);
        // --------------------------------------------------------------------
        ~RewindInfo()
        {
            delete m_buffer;
        }   // ~RewindInfo
        // --------------------------------------------------------------------
        /** Returns a pointer to the state buffer. */
        char *getBuffer() const { return m_buffer; }
        // --------------------------------------------------------------------
        /** Returns the time at which this rewind state was saved. */
        float getTime() const { return m_time; }
        // --------------------------------------------------------------------
        bool isEvent() const { return m_is_event; }
        // --------------------------------------------------------------------
        /** Returns if this state is confirmed. */
        bool isConfirmed() const { return m_is_confirmed; }
        // --------------------------------------------------------------------
        /** Called when going back in time to undo any rewind information. 
         *  It calls either undoEvent or undoState in the rewinder. */
        void undo()
        {
            if(m_is_event)
                m_rewinder->undoEvent(m_buffer);
            else
                m_rewinder->undoState(m_buffer);
        }   // undoEvent
        // --------------------------------------------------------------------
        /** Rewinds to this state. This is called while going forwards in time
         *  again to reach current time. If the info is a state, it will 
         *  call rewindToState(char *) if the state is a confirmed state, or
         *  rewindReplace(char*) in order to discard the old stored data,
         *  and replace it with the new state at that time. In case of an 
         *  event, rewindEvent(char*) is called.
         */
        void rewind()
        {
            if(m_is_event)
                m_rewinder->rewindToEvent(m_buffer);
            else
            {
                if(m_is_confirmed)
                    m_rewinder->rewindToState(m_buffer);
                else
                {
                    // TODO
                    // Handle replacing of stored states.
                }
            }
        }   // rewind
    };   // RewindInfo
    // ========================================================================

    /** Pointer to all saved states. */
    typedef std::vector<RewindInfo*> AllRewindInfo;

    AllRewindInfo m_rewind_info;

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
    float determineTimeStepSize(int state);
public:
    // First static functions to manage rewinding.
    // ===========================================
    static RewindManager *create();
    static void destroy();
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
    void update(float dt);
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
    void rewindTo(float target_time);
    // ------------------------------------------------------------------------
    void addEvent(Rewinder *rewinder, float time, char *buffer);
};   // RewindManager


#endif

