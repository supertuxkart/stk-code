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

/*! \file protocol_manager.hpp
 *  \brief Contains structures and enumerations related to protocol management.
 */

#ifndef PROTOCOL_MANAGER_HPP
#define PROTOCOL_MANAGER_HPP

#include "network/network_string.hpp"
#include "network/protocol.hpp"
#include "utils/no_copy.hpp"
#include "utils/singleton.hpp"
#include "utils/synchronised.hpp"
#include "utils/types.hpp"

#include <vector>

class Event;
class STKPeer;

#define TIME_TO_KEEP_EVENTS 1.0

// ----------------------------------------------------------------------------
/** \enum ProtocolRequestType
 *  \brief Defines actions that can be done about protocols.
 *  This enum is used essentially to keep the manager thread-safe and
 *  to avoid protocols modifying directly their state.
 */
enum ProtocolRequestType
{
    PROTOCOL_REQUEST_START,     //!< Start a protocol
    PROTOCOL_REQUEST_PAUSE,     //!< Pause a protocol
    PROTOCOL_REQUEST_UNPAUSE,   //!< Unpause a protocol
    PROTOCOL_REQUEST_TERMINATE  //!< Terminate a protocol
};   // ProtocolRequestType

// ----------------------------------------------------------------------------
/** \struct ProtocolRequest
 *  \brief Represents a request to do an action about a protocol.
 */
class ProtocolRequest
{
public:
    /** The type of request. */
    ProtocolRequestType m_type;

    /** The concerned protocol information. */
    Protocol *m_protocol;

public:
    ProtocolRequest(ProtocolRequestType type, Protocol *protocol)
    {
        m_type     = type;
        m_protocol = protocol;
    }   // ProtocolRequest
    // ------------------------------------------------------------------------
    /** Returns the request type. */
    ProtocolRequestType getType() const { return m_type;  }
    // ------------------------------------------------------------------------
    /** Returns the protocol for this request. */
    Protocol *getProtocol() { return m_protocol;  }
};   // class ProtocolRequest;

// ============================================================================
/** \class ProtocolManager
 *  \brief Manages the protocols at runtime.
 *
 *  This class is in charge of storing and managing protocols.
 *  It is a singleton as there can be only one protocol manager per game
 *  instance. Any game object that wants to start a protocol must create a
 *  protocol and give it to this singleton. The protocols are updated in a
 *  special thread, to ensure that they are processed independently from the
 *  frames per second. Then, the management of protocols is thread-safe: any
 *  object can start/pause/... protocols whithout problems.
 */ 
class ProtocolManager : public AbstractSingleton<ProtocolManager>,
                        public NoCopy
{
    friend class AbstractSingleton<ProtocolManager>;
private:

    /** Contains the running protocols.
     *  This stores the protocols that are either running or paused, their
     *  state and their unique id. */
    Synchronised<std::vector<Protocol*> >m_protocols;

    /** Contains the network events to pass asynchronously to protocols
     *  (i.e. from the separate ProtocolManager thread). */
    Synchronised<std::vector<Event*> > m_events_to_process;

    /** Contains the requests to start/pause etc... protocols. */
    Synchronised< std::vector<ProtocolRequest> > m_requests;

    /*! \brief The next id to assign to a protocol.
     * This value is incremented by 1 each time a protocol is started.
     * If a protocol has an id lower than this value, it means that it has
     * been formerly started.
     */
    Synchronised<uint32_t> m_next_protocol_id;

    /** When set to true, the main thread will exit. */
    Synchronised<bool> m_exit;

    // mutexes:
    /*! Used to ensure that the protocol vector is used thread-safely.   */
    pthread_mutex_t m_asynchronous_protocols_mutex;

    /*! Asynchronous update thread.*/
    pthread_t* m_asynchronous_update_thread;

                 ProtocolManager();
    virtual     ~ProtocolManager();
    static void* mainLoop(void *data);
    uint32_t     getNextProtocolId();
    bool         sendEvent(Event* event);

    virtual void startProtocol(Protocol *protocol);
    virtual void terminateProtocol(Protocol *protocol);
    virtual void asynchronousUpdate();
    virtual void pauseProtocol(Protocol *protocol);
    virtual void unpauseProtocol(Protocol *protocol);

public:
    virtual void      abort();
    virtual void      propagateEvent(Event* event);
    virtual uint32_t  requestStart(Protocol* protocol);
    virtual void      requestPause(Protocol* protocol);
    virtual void      requestUnpause(Protocol* protocol);
    virtual void      requestTerminate(Protocol* protocol);
    virtual void      update(float dt);
    virtual Protocol* getProtocol(uint32_t id);
    virtual Protocol* getProtocol(ProtocolType type);
};   // class ProtocolManager

#endif // PROTOCOL_MANAGER_HPP
