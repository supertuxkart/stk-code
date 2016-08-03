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

#include "network/rewinder.hpp"
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
/** A rewind info abstract class that keeps track of a rewinder object.
 */
class RewindInfoRewinder : public RewindInfo
{
protected:
    /** The Rewinder instance for which this data is. */
    Rewinder *m_rewinder;

    /** Pointer to the buffer which stores all states. */
    char *m_buffer;

public:
    RewindInfoRewinder(float time, Rewinder *rewinder, char *buffer,
                       bool is_confirmed)
        : RewindInfo(time, is_confirmed)
    {
        m_rewinder = rewinder;
        m_buffer = buffer;
    }   // RewindInfoRewinder
    // ------------------------------------------------------------------------
    ~RewindInfoRewinder()
    {
    }   // ~RewindInfoRewinder

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
             RewindInfoState(float time, Rewinder *rewinder, char *buffer,
                             bool is_confirmed);
    virtual ~RewindInfoState() {};

    // ------------------------------------------------------------------------
    /** Returns the left-over physics time. */
    float getLocalPhysicsTime() const { return m_local_physics_time; }
    // ------------------------------------------------------------------------
    /** Returns a pointer to the state buffer. */
    char *getBuffer() const { return m_buffer; }
    // ------------------------------------------------------------------------
    virtual bool isState() const { return true; }
    // ------------------------------------------------------------------------
    /** Called when going back in time to undo any rewind information.
     *  It calls undoState in the rewinder. */
    virtual void undo()
    {
        m_rewinder->undoState(m_buffer);
    }   // undoEvent
    // ------------------------------------------------------------------------
    /** Rewinds to this state. This is called while going forwards in time
     *  again to reach current time. It will call rewindToState(char *)
     *  if the state is a confirmed state. */
    virtual void rewind()
    {
        if (isConfirmed())
            m_rewinder->rewindToState(m_buffer);
        else
        {
            // TODO
            // Handle replacing of stored states.
        }
    }   // rewind
};   // class RewindInfoState

// ============================================================================
class RewindInfoEvent : public RewindInfoRewinder
{
public:
             RewindInfoEvent(float time, Rewinder *rewinder, char *buffer,
                             bool is_confirmed);
    virtual ~RewindInfoEvent() {}

    // ------------------------------------------------------------------------
    /** Returns a pointer to the state buffer. */
    char *getBuffer() const { return m_buffer; }
    // ------------------------------------------------------------------------
    virtual bool isEvent() const { return true; }
    // ------------------------------------------------------------------------
    /** Called when going back in time to undo any rewind information.
    *  It calls undoEvent in the rewinder. */
    virtual void undo()
    {
        m_rewinder->undoEvent(m_buffer);
    }   // undo
    // ------------------------------------------------------------------------
    /** This is called while going forwards in time again to reach current
     *  time. Calls rewindEvent(char*).
     */
    virtual void rewind()
    {
        m_rewinder->rewindToEvent(m_buffer);
    }   // rewind
};   // class RewindIndoEvent

#endif
