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

#include <list>
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
 *  \brief Represents a request to do an action about a protocol, e.g. to
 *         start, pause, unpause or terminate a protocol.
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

    /** A simple class that stores all protocols of a certain type. While
     *  many protocols have at most one instance running, some (e.g. 
     *  GetPublicAddress, ConntectToPeer, ...) can have several instances
     *  active at the same time. */
    class OneProtocolType
    {
    private:
        Synchronised< std::vector<Protocol*> > m_protocols;
    public:
        void removeProtocol(Protocol *p);
        void requestTerminateAll();
        bool notifyEvent(Event *event);
        void update(float dt, bool async);
        void abort();
        // --------------------------------------------------------------------
        /** Returns the first protocol of a given type. It is assumed that
         *  there is a protocol of that type. */
        Protocol *getFirstProtocol() { return m_protocols.getData()[0]; }
        // --------------------------------------------------------------------
        /** Returns if this protocol class handles connect events. Protocols
         *  of the same class either all handle a connect event, or none, so
         *  only the first protocol is actually tested. */
        bool handleConnects() const
        {
            return !m_protocols.getData().empty() &&
                    m_protocols.getData()[0]->handleConnects();
        }   // handleConnects
        // --------------------------------------------------------------------
        /** Returns if this protocol class handles disconnect events. Protocols
        *  of the same class either all handle a disconnect event, or none, so
        *  only the first protocol is actually tested. */
        bool handleDisconnects() const
        {
            return !m_protocols.getData().empty() &&
                m_protocols.getData()[0]->handleDisconnects();
        }   // handleDisconnects
        // --------------------------------------------------------------------
        /** Locks access to this list of all protocols of a certain type. */
        void lock() { m_protocols.lock(); }
        // --------------------------------------------------------------------
        /** Locks access to this list of all protocols of a certain type. */
        void unlock() { m_protocols.unlock(); }
        // --------------------------------------------------------------------
        void addProtocol(Protocol *p)
        {
            m_protocols.getData().push_back(p); 
        }   // addProtocol
        // --------------------------------------------------------------------
        /** Returns if there are no protocols of this type registered. */
        bool isEmpty() const { return m_protocols.getData().empty(); }
        // --------------------------------------------------------------------

    };   // class OneProtocolType

    // ------------------------------------------------------------------------
    
    /** The list of all protocol types, each one containing a (potentially
     *  empty) list of protocols. */
    std::vector<OneProtocolType> m_all_protocols;

    /** A list of network events - messages, disconnect and disconnects. */
    typedef std::list<Event*> EventList;

    /** Contains the network events to pass synchronously to protocols
     *  (i.e. from the main thread). */
    Synchronised<EventList> m_sync_events_to_process;

    /** Contains the network events to pass asynchronously to protocols
    *  (i.e. from the separate ProtocolManager thread). */
    Synchronised<EventList> m_async_events_to_process;

    /** Contains the requests to start/pause etc... protocols. */
    Synchronised< std::vector<ProtocolRequest> > m_requests;

    /** When set to true, the main thread will exit. */
    Synchronised<bool> m_exit;

    /*! Asynchronous update thread.*/
    pthread_t m_asynchronous_update_thread;

                 ProtocolManager();
    virtual     ~ProtocolManager();
    static void* mainLoop(void *data);
    bool         sendEvent(Event* event);

    virtual void startProtocol(Protocol *protocol);
    virtual void terminateProtocol(Protocol *protocol);
    virtual void asynchronousUpdate();
    virtual void pauseProtocol(Protocol *protocol);
    virtual void unpauseProtocol(Protocol *protocol);

public:
    void      abort();
    void      propagateEvent(Event* event);
    Protocol* getProtocol(ProtocolType type);
    void      requestStart(Protocol* protocol);
    void      requestPause(Protocol* protocol);
    void      requestUnpause(Protocol* protocol);
    void      requestTerminate(Protocol* protocol);
    void      findAndTerminate(ProtocolType type);
    void      update(float dt);
    // ------------------------------------------------------------------------
    const pthread_t & getThreadID() const
    {
        return m_asynchronous_update_thread; 
    }   // getThreadID

};   // class ProtocolManager

#endif // PROTOCOL_MANAGER_HPP
