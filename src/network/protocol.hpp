//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2013-2015 SuperTuxKart-Team
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

/*! \file protocol.hpp
 *  \brief Generic protocols declarations.
 */

#ifndef PROTOCOL_HPP
#define PROTOCOL_HPP

#include "utils/leak_check.hpp"
#include "utils/no_copy.hpp"
#include "utils/types.hpp"

#include <stddef.h>

class Event;
class NetworkString;
class STKPeer;


/** \enum ProtocolType
 *  \brief The types that protocols can have. This is used to select which 
 *   protocol receives which event.
 *  \ingroup network
 */
enum ProtocolType
{
    PROTOCOL_NONE              = 0x00,  //!< No protocol type assigned.
    PROTOCOL_CONNECTION        = 0x01,  //!< Protocol that deals with client-server connection.
    PROTOCOL_LOBBY_ROOM        = 0x02,  //!< Protocol that is used during the lobby room phase.
    PROTOCOL_START_GAME        = 0x03,  //!< Protocol used when starting the game.
    PROTOCOL_SYNCHRONIZATION   = 0x04,  //!<Protocol used to synchronize clocks.
    PROTOCOL_KART_UPDATE       = 0x05,  //!< Protocol to update karts position, rotation etc...
    PROTOCOL_GAME_EVENTS       = 0x06,  //!< Protocol to communicate the game events.
    PROTOCOL_CONTROLLER_EVENTS = 0x07,  //!< Protocol to transfer controller modifications
    PROTOCOL_SYNCHRONOUS       = 0x80,  //!< Flag, indicates synchronous delivery
    PROTOCOL_SILENT            = 0xff   //!< Used for protocols that do not subscribe to any network event.
};   // ProtocolType

// ----------------------------------------------------------------------------
/** \enum ProtocolState
 *  \brief Defines the three states that a protocol can have.
 */
enum ProtocolState
{
    PROTOCOL_STATE_INITIALISING, //!< The protocol is waiting to be started
    PROTOCOL_STATE_RUNNING,      //!< The protocol is being updated everytime.
    PROTOCOL_STATE_PAUSED,       //!< The protocol is paused.
    PROTOCOL_STATE_TERMINATED    //!< The protocol is terminated/does not exist.
};   // ProtocolState

class Protocol;

// ============================================================================*
/** \class CallbackObject
 *  \brief Class that must be inherited to pass objects to protocols.
 */
class CallbackObject
{
public:
             CallbackObject() {}
    virtual ~CallbackObject() {}

    virtual void callback(Protocol *protocol) = 0;
};   // CallbackObject

// ============================================================================
/** \class Protocol
 *  \brief Abstract class used to define the global protocol functions.
 *  A protocol is an entity that is started at a point, and that is updated
 *  by a thread. A protocol can be terminated by an other class, or it can
 *  terminate itself if has fulfilled its role. This class must be inherited
 *  to make any network job.
 * \ingroup network
 */
class Protocol : public NoCopy
{
    LEAK_CHECK()
protected:
    /** The type of the protocol. */
    ProtocolType m_type;

    /** The callback object, if needed. */
    CallbackObject* m_callback_object;

    /** The state this protocol is in (e.g. running, paused, ...). */
    ProtocolState m_state;

    /** The unique id of the protocol. */
    uint32_t        m_id;

    /** True if this protocol should receive connection events. */
    bool m_handle_connections;

    /** TRue if this protocol should recceiver disconnection events. */
    bool m_handle_disconnections;
public:
             Protocol(ProtocolType type, CallbackObject* callback_object=NULL);
    virtual ~Protocol();

    /** \brief Called when the protocol is going to start. Must be re-defined
     *  by subclasses. */
    virtual void setup() = 0;
    // ------------------------------------------------------------------------

    /** \brief Called by the protocol listener, synchronously with the main
     *  loop. Must be re-defined.*/
    virtual void update(float dt) = 0;

    /** \brief Called by the protocol listener as often as possible.
     *  Must be re-defined. */
    virtual void asynchronousUpdate() = 0;

    /// functions to check incoming data easily
    NetworkString* getNetworkString(size_t capacity = 16);
    bool checkDataSize(Event* event, unsigned int minimum_size);
    void sendMessageToPeersChangingToken(NetworkString *message,
                                         bool reliable = true);
    void sendToServer(NetworkString *message,
                      bool reliable = true);
    void requestStart();
    void requestPause();
    void requestUnpause();
    void requestTerminate();
    void findAndTerminateProtocol(ProtocolType type);

    // ------------------------------------------------------------------------
    /** \brief Called when the protocol is paused (by an other entity or by
    *  itself). */
    virtual void paused() { }
    // ------------------------------------------------------------------------
    /** \brief Called when the protocol is used.
    */
    virtual void unpaused() { }
    // ------------------------------------------------------------------------
    /** \brief Called when the protocol was just killed. It triggers the 
     *  callback if defined. */
    virtual void terminated()
    {
        if (m_callback_object)
            m_callback_object->callback(this);
    }   // terminated
    // ------------------------------------------------------------------------
    /** Returns the current protocol state. */
    ProtocolState getState() const { return m_state;  }
    // ------------------------------------------------------------------------
    /** Sets the current protocol state. */
    void setState(ProtocolState s) { m_state = s; }
    // ------------------------------------------------------------------------
    /** Returns the unique protocol ID. */
    uint32_t getId() const { return m_id;  }
    // ------------------------------------------------------------------------
    /** Sets the unique protocol id. */
    void setId(uint32_t id) { m_id = id;  }
    // ------------------------------------------------------------------------
    /** \brief Notify a protocol matching the Event type of that event.
     *  \param event : Pointer to the event.
     *  \return True if the event has been treated, false otherwise. */
    virtual bool notifyEvent(Event* event) { return false; }
    // ------------------------------------------------------------------------
    /** \brief Notify a protocol matching the Event type of that event.
     *  This update is done asynchronously :
     *  \param event : Pointer to the event.
     *  \return True if the event has been treated, false otherwise */
    virtual bool notifyEventAsynchronous(Event* event) { return false; }
    // ------------------------------------------------------------------------
    /** \brief Method to get a protocol's type.
     *  \return The protocol type. */
    ProtocolType getProtocolType() const { return m_type; }
    // ------------------------------------------------------------------------
    /** Sets if this protocol should receive connection events. */
    void setHandleConnections(bool b) { m_handle_connections = b; }
    // ------------------------------------------------------------------------
    /** Sets if this protocol should receive disconnection events. */
    void setHandleDisconnections(bool b) { m_handle_disconnections = b; }
    // ------------------------------------------------------------------------
    /** Return true if this protocol should be informed about connects. */
    virtual bool handleConnects() const { return m_handle_connections; }
    // ------------------------------------------------------------------------
    /** Return true if this protocol should be informed about disconnects. */
    virtual bool handleDisconnects() const { return m_handle_disconnections; }

};   // class Protocol

#endif // PROTOCOL_HPP
