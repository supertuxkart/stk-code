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

#include "utils/no_copy.hpp"
#include "utils/types.hpp"

#include <memory>
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
    PROTOCOL_GAME_EVENTS       = 0x03,  //!< Protocol to communicate the game events.
    PROTOCOL_CONTROLLER_EVENTS = 0x04,  //!< Protocol to transfer controller modifications
    PROTOCOL_SILENT            = 0x05,  //!< Used for protocols that do not subscribe to any network event.
    PROTOCOL_MAX                     ,  //!< Maximum number of different protocol types
    PROTOCOL_SYNCHRONOUS       = 0x80,  //!< Flag, indicates synchronous delivery
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
class Protocol : public std::enable_shared_from_this<Protocol>,
                 public NoCopy
{
protected:
    /** The type of the protocol. */
    ProtocolType m_type;

    /** True if this protocol should receive connection events. */
    bool m_handle_connections;

    /** TRue if this protocol should recceiver disconnection events. */
    bool m_handle_disconnections;
public:
             Protocol(ProtocolType type);
    virtual ~Protocol();

    /** \brief Called when the protocol is going to start. Must be re-defined
     *  by subclasses. */
    virtual void setup() = 0;
    // ------------------------------------------------------------------------

    /** \brief Called by the protocol listener, synchronously with the main
     *  loop. Must be re-defined.*/
    virtual void update(int ticks) = 0;

    /** \brief Called by the protocol listener as often as possible.
     *  Must be re-defined. */
    virtual void asynchronousUpdate() = 0;

    /// functions to check incoming data easily
    NetworkString* getNetworkString(size_t capacity = 16) const;
    bool checkDataSize(Event* event, unsigned int minimum_size);
    void sendMessageToPeers(NetworkString *message, bool reliable = true);
    void sendMessageToPeersInServer(NetworkString *message,
                                    bool reliable = true);
    void sendToServer(NetworkString *message, bool reliable = true);
    virtual void requestStart();
    virtual void requestTerminate();
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
