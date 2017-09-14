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
#include "network/rewinder.hpp"
#include "utils/leak_check.hpp"
#include "utils/ptr_vector.hpp"

#include <assert.h>
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

    /** Time when this state was taken. */
    float m_time;

    /** A confirmed event is one that was sent from the server. When
     *  rewinding we have to start with a confirmed state for each
     *  object.  */
    bool m_is_confirmed;

public:
    RewindInfo(float time, bool is_confirmed);

    /** Called when going back in time to undo any rewind information. */
    virtual void undo() = 0;

    /** This is called while going forwards in time again to reach current
     *  time. */
    virtual void rewind() = 0;

    // ------------------------------------------------------------------------
    virtual ~RewindInfo() { }
    // ------------------------------------------------------------------------
    /** Returns the time at which this rewind state was saved. */
    float getTime() const { return m_time; }
    // ------------------------------------------------------------------------
    /** Sets if this RewindInfo is confirmed or not. */
    void setConfirmed(bool b) { m_is_confirmed = b; }
    // ------------------------------------------------------------------------
    /** Returns if this state is confirmed. */
    bool isConfirmed() const { return m_is_confirmed; }
    // ------------------------------------------------------------------------
    /** If this rewind info is an event. Subclasses will overwrite this. */
    virtual bool isEvent() const { return false; }
    // ------------------------------------------------------------------------
    /** If this rewind info is time info. Subclasses will overwrite this. */
    virtual bool isTime() const { return false; }
    // ------------------------------------------------------------------------
    /** If this rewind info is an event. Subclasses will overwrite this. */
    virtual bool isState() const { return false; }
    // ------------------------------------------------------------------------
};   // RewindInfo

// ============================================================================
/** A rewind info abstract class that keeps track of a rewinder object, and
 *  has a BareNetworkString buffer which is used to store a state or event.
 */
class RewindInfoRewinder : public RewindInfo
{
private:
    /** Pointer to the buffer which stores all states. */
    BareNetworkString *m_buffer;

protected:
    /** The Rewinder instance for which this data is. */
    Rewinder *m_rewinder;

public:
    RewindInfoRewinder(float time, Rewinder *rewinder,
                       BareNetworkString *buffer, bool is_confirmed)
        : RewindInfo(time, is_confirmed)
    {
        m_rewinder = rewinder;
        m_buffer = buffer;
    }   // RewindInfoRewinder
    // ------------------------------------------------------------------------
    virtual ~RewindInfoRewinder()
    {
        delete m_buffer;
    }   // ~RewindInfoRewinder
    // ------------------------------------------------------------------------
    /** Returns a pointer to the state buffer. */
    BareNetworkString *getBuffer() const { return m_buffer; }
};   // RewindInfoRewinder

// ============================================================================
class RewindInfoTime : public RewindInfo
{
private:

public:
             RewindInfoTime(float time);
    virtual ~RewindInfoTime() {};

    // ------------------------------------------------------------------------
    virtual bool isTime() const { return true; }
    // ------------------------------------------------------------------------
    /** Called when going back in time to undo any rewind information.
     *  Does actually nothing. */
    virtual void undo() {} 
    // ------------------------------------------------------------------------
    /** Rewinds to this state. Nothing to be done for time info. */
    virtual void rewind() {}
};   // class RewindInfoTime

// ============================================================================
class RewindInfoState: public RewindInfoRewinder
{
private:
    /** The 'left over' time from the physics. */
    float m_local_physics_time;

public:
             RewindInfoState(float time, Rewinder *rewinder, 
                             BareNetworkString *buffer, bool is_confirmed);
    virtual ~RewindInfoState() {};

    // ------------------------------------------------------------------------
    /** Returns the left-over physics time. */
    float getLocalPhysicsTime() const { return m_local_physics_time; }
    // ------------------------------------------------------------------------
    virtual bool isState() const { return true; }
    // ------------------------------------------------------------------------
    /** Called when going back in time to undo any rewind information.
     *  It calls undoState in the rewinder. */
    virtual void undo()
    {
        m_rewinder->undoState(getBuffer());
    }   // undo
    // ------------------------------------------------------------------------
    /** Rewinds to this state. This is called while going forwards in time
     *  again to reach current time. It will call rewindToState().
     *  if the state is a confirmed state. */
    virtual void rewind()
    {
        if (isConfirmed())
            m_rewinder->rewindToState(getBuffer());
        else
        {
            // TODO
            // Handle replacing of stored states.
        }
    }   // rewind
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
             RewindInfoEvent(float time, EventRewinder *event_rewinder,
                             BareNetworkString *buffer, bool is_confirmed);
    virtual ~RewindInfoEvent()
    {
        delete m_buffer;
    }   // ~RewindInfoEvent

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
     *  time. Calls rewindEvent().
     */
    virtual void rewind()
    {
        // Make sure to reset the buffer so we read from the beginning
        m_buffer->reset();
        m_event_rewinder->rewind(m_buffer);
    }   // rewind
    // ------------------------------------------------------------------------
    /** Returns the buffer with the event information in it. */
    BareNetworkString *getBuffer() { return m_buffer; }
};   // class RewindIndoEvent

#endif
