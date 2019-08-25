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

#include <atomic>
#include <list>
#include <memory>
#include <vector>
#include <thread>

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
    std::shared_ptr<Protocol> m_protocol;

public:
    ProtocolRequest(ProtocolRequestType type, std::shared_ptr<Protocol> protocol)
    {
        m_type     = type;
        m_protocol = protocol;
    }   // ProtocolRequest
    // ------------------------------------------------------------------------
    /** Returns the request type. */
    ProtocolRequestType getType() const { return m_type;  }
    // ------------------------------------------------------------------------
    /** Returns the protocol for this request. */
    std::shared_ptr<Protocol> getProtocol() { return m_protocol;  }
};   // class ProtocolRequest;

// ============================================================================
/** \class ProtocolManager
 *  \brief Manages the protocols at runtime.
 *
 *  This class is in charge of storing and managing protocols.
 *  It is a singleton as there can be only one protocol manager per game
 *  instance. Any game object that wants to start a protocol must create a
 *  protocol and give it to this singleton. The protocols are updated in two
 *  different ways:
 *  1) Asynchronous updates:
 *     A separate threads runs that delivers asynchronous events 
 *     (i.e. messages), updates each protocol, and handles new requests
 *     (start/stop protocol etc). Protocols are updated using the
 *     Protocol::asynchronousUpdate() function.
 
 *  2) Synchronous updates:
 *     This is called from the main game thread, and will deliver synchronous
 *     events (i.e. messages), and updates each protocol using
 *     Protocol::update(). 
 *
 *  Since the STK main loop is not thread safe, any game changing events must
 *  (e.g. events that push a new screen, ...) be processed synchronoysly.
 *  On the other hand, asynchronous updates will be handled much more
 *  frequently, so synchronous updates should be avoided as much as possible.
 *  The sender selects if a message is synchronous or asynchronous. The
 *  network layer (separate thread) calls propagateEvent in the
 *  ProtocolManager, which will add the event to the synchronous or
 *  asynchornous queue.
 *  Protocol start/pause/... requests are also stored in a separate queue,
 *  which is thread-safe, and requests will be handled by the ProtocolManager
 *  thread, to ensure that they are processed independently from the
 *  frames per second.
 *
 *  Events received by ENET are queried and then handled by STKHost::mainLoop.
 *  Besides messages these events also include connection and disconnection
 *  notifications. Protocols can decide to receives those notifications or
 *  not. The Enet events are converted into STK events, which store e.g. the
 *  sender as STKPeer info, and the message data is converted into a
 *  NetworkString. This STK event is then forwarded to the corresponding
 *  protocols.
 *
 *  There are some protocols that can have more than one instance running at
 *  a time (e.g. on the server a connect to peer protocol). The Protocol
 *  Manager stores each protocol with the same protocol id in a OneProtocol
 *  structure (so in most cases this is just one protocol instance in one
 *  OneProtocol structure, but e.g. several connect_to_peer instances would
 *  be stored in one OneProtocoll instance. The OneProtocol instance is 
 *  responsible to forward events to all protocols with the same id.
 *  
 */ 
class ProtocolManager : public NoCopy
{
private:

    /** A simple class that stores all protocols of a certain type. While
     *  many protocols have at most one instance running, some (e.g. 
     *  GetPublicAddress, ConntectToPeer, ...) can have several instances
     *  active at the same time. */
    class OneProtocolType
    {
    private:
        Synchronised< std::vector<std::shared_ptr<Protocol> > > m_protocols;
    public:
        void removeProtocol(std::shared_ptr<Protocol> p);
        void requestTerminateAll();
        bool notifyEvent(Event *event);
        void update(int ticks, bool async);
        void abort();
        // --------------------------------------------------------------------
        /** Returns the first protocol of a given type. It is assumed that
         *  there is a protocol of that type. */
        std::shared_ptr<Protocol> getFirstProtocol()
                                           { return m_protocols.getData()[0]; }
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
        void addProtocol(std::shared_ptr<Protocol> p)
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
    std::atomic_bool m_exit;

    /*! Asynchronous update thread.*/
    std::thread m_asynchronous_update_thread;

    /*! Single instance of protocol manager.*/
    static std::weak_ptr<ProtocolManager> m_protocol_manager;

    bool         sendEvent(Event* event);

    virtual void startProtocol(std::shared_ptr<Protocol> protocol);
    virtual void terminateProtocol(std::shared_ptr<Protocol> protocol);
    virtual void asynchronousUpdate();
    virtual void pauseProtocol(std::shared_ptr<Protocol> protocol);
    virtual void unpauseProtocol(std::shared_ptr<Protocol> protocol);

public:
    // ===========================================
    // Public constructor is required for shared_ptr
              ProtocolManager();
    virtual  ~ProtocolManager();
    void      abort();
    void      propagateEvent(Event* event);
    std::shared_ptr<Protocol> getProtocol(ProtocolType type);
    void      requestStart(std::shared_ptr<Protocol> protocol);
    void      requestPause(std::shared_ptr<Protocol> protocol);
    void      requestUnpause(std::shared_ptr<Protocol> protocol);
    void      requestTerminate(std::shared_ptr<Protocol> protocol);
    void      findAndTerminate(ProtocolType type);
    void      update(int ticks);
    // ------------------------------------------------------------------------
    bool isExiting() const                            { return m_exit.load(); }
    // ------------------------------------------------------------------------
    const std::thread& getThread() const
    {
        return m_asynchronous_update_thread; 
    }   // getThreadID
    // ------------------------------------------------------------------------
    static std::shared_ptr<ProtocolManager> createInstance();
    // ------------------------------------------------------------------------
    static bool emptyInstance()
    {
        return m_protocol_manager.expired();
    }   // emptyInstance
    // ------------------------------------------------------------------------
    static std::shared_ptr<ProtocolManager> lock()
    {
        return m_protocol_manager.lock();
    }   // lock

};   // class ProtocolManager

#endif // PROTOCOL_MANAGER_HPP
