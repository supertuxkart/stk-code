//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2016 Joerg Henrichs
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

#ifndef HEADER_REWIND_INFO_HPP
#define HEADER_REWIND_INFO_HPP

#include "network/event_rewinder.hpp"
#include "network/network_string.hpp"
#include "utils/cpp2011.hpp"
#include "utils/leak_check.hpp"
#include "utils/ptr_vector.hpp"

#include <assert.h>
#include <functional>
#include <string>
#include <vector>

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
    LEAK_CHECK();

    /** Time when this RewindInfo was taken. */
    int m_ticks;

    /** A confirmed event is one that was sent from the server. When
     *  rewinding we have to start with a confirmed state for each
     *  object.  */
    bool m_is_confirmed;

public:
    RewindInfo(int ticks, bool is_confirmed);

    void setTicks(int ticks);

    /** Called when going back in time to undo any rewind information. */
    virtual void undo() = 0;
    /** This is called to restore a state before replaying the events. */
    virtual void restore() = 0;
    /** This is called while going forwards in time again to reach current
    *  time. */
    virtual void replay() = 0;
    // ------------------------------------------------------------------------
    virtual ~RewindInfo() { }
    // ------------------------------------------------------------------------
    /** Returns the time at which this RewindInfo was saved. */
    int getTicks() const { return m_ticks; }
    // ------------------------------------------------------------------------
    /** Sets if this RewindInfo is confirmed or not. */
    void setConfirmed(bool b) { m_is_confirmed = b; }
    // ------------------------------------------------------------------------
    /** Returns if this RewindInfo is confirmed. */
    bool isConfirmed() const { return m_is_confirmed; }
    // ------------------------------------------------------------------------
    /** If this RewindInfo is an event. Subclasses will overwrite this. */
    virtual bool isEvent() const { return false; }
    // ------------------------------------------------------------------------
    /** If this RewindInfo is an event. Subclasses will overwrite this. */
    virtual bool isState() const { return false; }
    // ------------------------------------------------------------------------
};   // RewindInfo

// ============================================================================
/** A class that stores a game state and can rewind it.
 */
class RewindInfoState: public RewindInfo
{
private:
    std::vector<std::string> m_rewinder_using;

    int m_start_offset;

    /** Pointer to the buffer which stores all states. */
    BareNetworkString *m_buffer;

public:
    // ------------------------------------------------------------------------
    RewindInfoState(int ticks, int start_offset,
                    std::vector<std::string>& rewinder_using,
                    std::vector<uint8_t>& buffer);
    // ------------------------------------------------------------------------
    RewindInfoState(int ticks, BareNetworkString *buffer, bool is_confirmed);
    // ------------------------------------------------------------------------
    virtual ~RewindInfoState()                             { delete m_buffer; }
    // ------------------------------------------------------------------------
    virtual void restore();
    // ------------------------------------------------------------------------
    /** Returns a pointer to the state buffer. */
    BareNetworkString *getBuffer() const { return m_buffer; }
    // ------------------------------------------------------------------------
    virtual bool isState() const { return true; }
    // ------------------------------------------------------------------------
    /** Called when going back in time to undo any rewind information.
     *  It calls undoState in the rewinder. */
    virtual void undo()
    {
        // Nothing being done in case of an undo that goes further back
    }   // undo
    // ------------------------------------------------------------------------
    /** Called when rewinding from a past state to 'now'. This should never
     *  be called for a state (a state calles restore()). */
    virtual void replay()
    {
        assert(false);
    }   // replay
};   // class RewindInfoState

// ============================================================================
class RewindInfoEvent : public RewindInfo
{
private:
    /** Pointer to the event rewinder responsible for this event. */
    EventRewinder *m_event_rewinder;

    /** Buffer with the event data. */
    BareNetworkString *m_buffer;
public:
             RewindInfoEvent(int ticks, EventRewinder *event_rewinder,
                             BareNetworkString *buffer, bool is_confirmed);
    virtual ~RewindInfoEvent()
    {
        delete m_buffer;
    }   // ~RewindInfoEvent

    // ------------------------------------------------------------------------
    /** An event is never 'restored', it is only rewound. */
    void restore() {}
    // ------------------------------------------------------------------------
    virtual bool isEvent() const { return true; }
    // ------------------------------------------------------------------------
    /** Called when going back in time to undo any rewind information.
     *  It calls undoEvent in the rewinder. */
    virtual void undo()
    {
        m_buffer->reset();
        m_event_rewinder->undo(m_buffer);
    }   // undo
    // ------------------------------------------------------------------------
    /** This is called while going forwards in time again to reach current
     *  time. Calls rewind() in the event rewinder.
     */
    virtual void replay()
    {
        // Make sure to reset the buffer so we read from the beginning
        m_buffer->reset();
        m_event_rewinder->rewind(m_buffer);
    }   // rewind
    // ------------------------------------------------------------------------
    /** Returns the buffer with the event information in it. */
    BareNetworkString *getBuffer() { return m_buffer; }
};   // class RewindIndoEvent


// ============================================================================
class RewindInfoEventFunction : public RewindInfo
{
private:
    const std::function<void()> m_undo_function, m_replay_function,
        m_destroy_function;
public:
    RewindInfoEventFunction(int ticks,
                            std::function<void()> undo_function = [](){},
                            std::function<void()> replay_function = [](){},
                            std::function<void()> destroy_function = [](){})
        : RewindInfo(ticks, true/*is_confirmed*/),
        m_undo_function(undo_function), m_replay_function(replay_function),
        m_destroy_function(destroy_function) {}
    // ------------------------------------------------------------------------
    ~RewindInfoEventFunction()                       {  m_destroy_function(); }
    // ------------------------------------------------------------------------
    /** An event is never 'restored', it is only rewound. */
    void restore() {}
    // ------------------------------------------------------------------------
    virtual bool isEvent() const                               { return true; }
    // ------------------------------------------------------------------------
    virtual void undo()                                  { m_undo_function(); }
    // ------------------------------------------------------------------------
    virtual void replay()                              { m_replay_function(); }
};   // class RewindInfoEventFunction

#endif
