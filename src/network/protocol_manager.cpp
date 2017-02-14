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

#include "network/protocol_manager.hpp"

#include "network/event.hpp"
#include "network/protocol.hpp"
#include "network/stk_host.hpp"
#include "network/stk_peer.hpp"
#include "utils/log.hpp"
#include "utils/time.hpp"
#include "utils/vs.hpp"

#include <assert.h>
#include <cstdlib>
#include <errno.h>
#include <typeinfo>


ProtocolManager::ProtocolManager()
{
    m_exit.setAtomic(false);

    m_all_protocols.resize(PROTOCOL_MAX);

    pthread_create(&m_asynchronous_update_thread, NULL,
                   ProtocolManager::mainLoop, this);
}   // ProtocolManager

// ----------------------------------------------------------------------------

void* ProtocolManager::mainLoop(void* data)
{
    VS::setThreadName("ProtocolManager");

    ProtocolManager* manager = static_cast<ProtocolManager*>(data);
    while(manager && !manager->m_exit.getAtomic())
    {
        manager->asynchronousUpdate();
        StkTime::sleep(2);
    }
    return NULL;
}   // protocolManagerAsynchronousUpdate


// ----------------------------------------------------------------------------
ProtocolManager::~ProtocolManager()
{
}   // ~ProtocolManager

// ----------------------------------------------------------------------------
void ProtocolManager::OneProtocolType::abort()
{
    for (unsigned int i = 0; i < m_protocols.getData().size(); i++)
        delete m_protocols.getData()[i];
    m_protocols.getData().clear();
}   // OneProtocolType::abort

// ----------------------------------------------------------------------------
/** \brief Stops the protocol manager.
 */
void ProtocolManager::abort()
{
    m_exit.setAtomic(true);
    pthread_join(m_asynchronous_update_thread, NULL); // wait the thread to finish

    // Now only this main thread is active, no more need for locks
    for (unsigned int i = 0; i < m_all_protocols.size(); i++)
    {
        m_all_protocols[i].abort();
    }

    m_sync_events_to_process.lock();
    for (EventList::iterator i =m_sync_events_to_process.getData().begin();
                             i!=m_sync_events_to_process.getData().end(); ++i)
        delete *i;
    m_sync_events_to_process.getData().clear();
    m_sync_events_to_process.unlock();

    m_async_events_to_process.lock();
    for (EventList::iterator i = m_async_events_to_process.getData().begin();
                             i!= m_async_events_to_process.getData().end(); ++i)
        delete *i;
    m_async_events_to_process.getData().clear();
    m_async_events_to_process.unlock();

    m_requests.lock();
    m_requests.getData().clear();
    m_requests.unlock();

}   // abort

// ----------------------------------------------------------------------------
/** \brief Function that processes incoming events.
 *  This function is called by the network manager each time there is an
 *  incoming packet.
 */
void ProtocolManager::propagateEvent(Event* event)
{
    if (event->isSynchronous())
    {
        m_sync_events_to_process.lock();
        m_sync_events_to_process.getData().push_back(event);
        m_sync_events_to_process.unlock();
    }
    else
    {
        m_async_events_to_process.lock();
        m_async_events_to_process.getData().push_back(event);
        m_async_events_to_process.unlock();
    }
    return;
}   // propagateEvent

// ----------------------------------------------------------------------------
/** \brief Asks the manager to start a protocol.
 * This function will store the request, and process it at a time when it is
 * thread-safe.
 * \param protocol : A pointer to the protocol to start
 * \return The unique id of the protocol that is being started.
 */
void ProtocolManager::requestStart(Protocol* protocol)
{
    // create the request
    ProtocolRequest req(PROTOCOL_REQUEST_START, protocol);
    // add it to the request stack
    m_requests.lock();
    m_requests.getData().push_back(req);
    m_requests.unlock();
}   // requestStart

// ----------------------------------------------------------------------------
/** \brief Asks the manager to pause a protocol.
 *  This function will store the request, and process it at a time it is
 *  thread-safe.
 *  \param protocol : A pointer to the protocol to pause
 */
void ProtocolManager::requestPause(Protocol* protocol)
{
    if (!protocol)
        return;
    // create the request
    ProtocolRequest req(PROTOCOL_REQUEST_PAUSE, protocol);
    // add it to the request stack
    m_requests.lock();
    m_requests.getData().push_back(req);
    m_requests.unlock();
}   // requestPause

// ----------------------------------------------------------------------------
/** \brief Asks the manager to unpause a protocol.
 *  This function will store the request, and process it at a time it is
 *  thread-safe.
 *  \param protocol : A pointer to the protocol to unpause
 */
void ProtocolManager::requestUnpause(Protocol* protocol)
{
    if (!protocol)
        return;
    // create the request
    ProtocolRequest req(PROTOCOL_REQUEST_UNPAUSE, protocol);;
    // add it to the request stack
    m_requests.lock();
    m_requests.getData().push_back(req);
    m_requests.unlock();
}   // requestUnpause

// ----------------------------------------------------------------------------
/** \brief Notifies the manager that a protocol is terminated.
 *  This function will store the request, and process it at a time it is
 *  thread-safe.
 *  \param protocol : A pointer to the protocol that is finished
 */
void ProtocolManager::requestTerminate(Protocol* protocol)
{
    if (!protocol)
        return;
    // create the request
    ProtocolRequest req(PROTOCOL_REQUEST_TERMINATE, protocol);
    // add it to the request stack
    m_requests.lock();
    // check that the request does not already exist :
    for (unsigned int i = 0; i < m_requests.getData().size(); i++)
    {
        if (m_requests.getData()[i].m_protocol == protocol)
        {
            m_requests.unlock();
            return;
        }
    }
    m_requests.getData().push_back(req);
    m_requests.unlock();
}   // requestTerminate

// ----------------------------------------------------------------------------

/** \brief Starts a protocol.
 *  Add the protocol info to the m_protocols vector.
 *  \param protocol : ProtocolInfo to start.
 */
void ProtocolManager::startProtocol(Protocol *protocol)
{
    assert(pthread_equal(pthread_self(), m_asynchronous_update_thread));
    OneProtocolType &opt = m_all_protocols[protocol->getProtocolType()];
    opt.lock();
    opt.addProtocol(protocol);
    protocol->setup();
    protocol->setState(PROTOCOL_STATE_RUNNING);
    opt.unlock();
    Log::info("ProtocolManager",
        "A %s protocol has been started.", typeid(*protocol).name());

    // setup the protocol and notify it that it's started
}   // startProtocol

// ----------------------------------------------------------------------------
/** \brief Pauses a protocol.
 *  Pauses a protocol and tells it that it's being paused.
 *  \param protocol : Protocol to pause.
 */
void ProtocolManager::pauseProtocol(Protocol *protocol)
{
    assert(pthread_equal(pthread_self(), m_asynchronous_update_thread));
    assert(protocol->getState() == PROTOCOL_STATE_RUNNING);
    // We lock the protocol to avoid that paused() is called at the same
    // time that the main thread delivers an event or calls update
    m_all_protocols[protocol->getProtocolType()].lock();
    protocol->setState(PROTOCOL_STATE_PAUSED);
    protocol->paused();
    m_all_protocols[protocol->getProtocolType()].unlock();
}   // pauseProtocol

// ----------------------------------------------------------------------------
/** \brief Unpauses a protocol.
 *  Unpauses a protocol and notifies it.
 *  \param protocol : Protocol to unpause.
 */
void ProtocolManager::unpauseProtocol(Protocol *protocol)
{
    assert(pthread_equal(pthread_self(), m_asynchronous_update_thread));
    assert(protocol->getState() == PROTOCOL_STATE_PAUSED);
    // No lock necessary, since the protocol is paused, no other thread will
    // be executing 
    protocol->setState(PROTOCOL_STATE_RUNNING);
    protocol->unpaused();
}   // unpauseProtocol

// ----------------------------------------------------------------------------
/** Removes a protocol from the list of protocols of a certain type.
 *  Note that the protocol is not deleted.
 *  \param p The protocol to be removed.
*/
void ProtocolManager::OneProtocolType::removeProtocol(Protocol *p)
{
    std::vector<Protocol*>::iterator i =
        std::find(m_protocols.getData().begin(),
                  m_protocols.getData().end(),   p);
    if (i == m_protocols.getData().end())
    {
        Log::error("ProtocolManager",
                   "Trying to delete protocol '%s', which was not found",
                   typeid(*p).name());
    }
    else
    {
        m_protocols.getData().erase(i);
    }
}   // deleteProtocol

// ----------------------------------------------------------------------------
/** \brief Notes that a protocol is terminated.
 *  Remove a protocol from the protocols vector.
 *  \param protocol : Protocol concerned.
 */
void ProtocolManager::terminateProtocol(Protocol *protocol)
{
    assert(pthread_equal(pthread_self(), m_asynchronous_update_thread));

    OneProtocolType &opt = m_all_protocols[protocol->getProtocolType()];
    // Be sure that noone accesses the protocols vector 
    // while the protocol is being removed.
    opt.lock();
    opt.removeProtocol(protocol);
    opt.unlock();
    protocol->setState(PROTOCOL_STATE_TERMINATED);
    protocol->terminated();
}   // terminateProtocol

// ----------------------------------------------------------------------------
/** Requests to terminate all protocols of the given protocol type.
 *  This function must be called from the ProtocolManager thread in order
 *  to avoid a race condition (only the ProtocolManager thread can change the
 *  number of elements in that list).
 */
void ProtocolManager::OneProtocolType::requestTerminateAll()
{
    for (unsigned int i = 0; i < m_protocols.getData().size(); i++)
    {
        m_protocols.getData()[i]->requestTerminate();
    }
}   // requestTerminateAll

// ----------------------------------------------------------------------------
/** Finds a protocol with the given type and requests it to be terminated.
 *  If no such protocol exist, log an error message.
 *  \param type The protocol type to delete.
 */
void ProtocolManager::findAndTerminate(ProtocolType type)
{
    OneProtocolType &opt = m_all_protocols[type];
    if (opt.isEmpty())
        Log::error("ProtocolManager",
                   "findAndTerminate: No protocol %d registered.", type);

    opt.requestTerminateAll();
}   // findAndTerminate

// ----------------------------------------------------------------------------
/** Calls either notifyEvent(event) or notifyEventAsynchronous(evet) on all
 *  protocols. Note that no locking is done, it is the responsibility of the
 *  caller to avoid race conditions.
 *  \param event The event to deliver to the protocols.
 */
bool ProtocolManager::OneProtocolType::notifyEvent(Event *event)
{
    if (m_protocols.getData().empty()) return false;

    // Either all protocols of a certain type handle connects, or none.
    // So we tet only one of them
    if (event->getType() == EVENT_TYPE_CONNECTED &&
        !m_protocols.getData()[0]->handleConnects()) return false;
    if (event->getType() == EVENT_TYPE_DISCONNECTED &&
        !m_protocols.getData()[0]->handleDisconnects()) return false;

    bool can_be_deleted = false;
    for (unsigned int i = 0; i < m_protocols.getData().size(); i++)
    {
        bool done = event->isSynchronous()
                  ? m_protocols.getData()[i]->notifyEvent(event)
                  : m_protocols.getData()[i]->notifyEventAsynchronous(event);
        can_be_deleted |= done;
    }
    return can_be_deleted;
}   // notifyEvent

// ----------------------------------------------------------------------------
/** Sends the event to the corresponding protocol. Returns true if the event
 *  can be ignored, or false otherwise.
 */
bool ProtocolManager::sendEvent(Event* event)
{
    bool can_be_deleted = false;
    if (event->getType() == EVENT_TYPE_MESSAGE)
    {
        OneProtocolType &opt = m_all_protocols[event->data().getProtocolType()];
        can_be_deleted = opt.notifyEvent(event);
    }
    else   // connect or disconnect event --> test all protocols
    {
        for (unsigned int i = 0; i < m_all_protocols.size(); i++)
        {
            can_be_deleted |= m_all_protocols[i].notifyEvent(event);
        }
    }
    return can_be_deleted || StkTime::getTimeSinceEpoch() - event->getArrivalTime()
                              >= TIME_TO_KEEP_EVENTS;
}   // sendEvent

// ----------------------------------------------------------------------------
/** Calls either the synchronous update or asynchronous update function in all
 *  protocols of this type.
 *  \param dt Time step size.
 *  \param async True if asynchronousUpdate() should be called.
 */
void ProtocolManager::OneProtocolType::update(float dt, bool async)
{
    for (unsigned int i = 0; i < m_protocols.getData().size(); i++)
    {
        if (m_protocols.getData()[i]->getState() == PROTOCOL_STATE_RUNNING)
        {
            async ? m_protocols.getData()[i]->asynchronousUpdate()
                  : m_protocols.getData()[i]->update(dt);
        }
    }
}   // update

// ----------------------------------------------------------------------------
/** \brief Updates the manager.
 *
 *  This function processes the events queue, notifies the concerned
 *  protocols that they have events to process. Then asks all protocols
 *  to update themselves. Finally it processes stored requests about
 *  starting, stoping, pausing etc... protocols.
 *  This function is called by the main thread (i.e. from main_loop).
 *  This function IS FPS-dependant.
 */
void ProtocolManager::update(float dt)
{
    // Update from main thread only:
    assert(!pthread_equal(pthread_self(), m_asynchronous_update_thread));

    // before updating, notify protocols that they have received events
    m_sync_events_to_process.lock();
    EventList::iterator i = m_sync_events_to_process.getData().begin();

    while (i != m_sync_events_to_process.getData().end())
    {
        m_sync_events_to_process.unlock();
        bool can_be_deleted = sendEvent(*i);
        m_sync_events_to_process.lock();
        if (can_be_deleted)
        {
            delete *i;
            i = m_sync_events_to_process.getData().erase(i);
        }
        else
        {
            // This should only happen if the protocol has not been started
            ++i;
        }
    }
    m_sync_events_to_process.unlock();

    // Now update all protocols.
    for (unsigned int i = 0; i < m_all_protocols.size(); i++)
    {
        OneProtocolType &opt = m_all_protocols[i];
        opt.lock();
        opt.update(dt, /*async*/false);
        opt.unlock();
    }
}   // update

// ----------------------------------------------------------------------------
/** \brief Updates the manager.
 *  This function processes the events queue, notifies the concerned
 *  protocols that they have events to process. Then ask all protocols
 *  to update themselves. Finally processes stored requests about
 *  starting, stoping, pausing etc... protocols.
 *  This function is called in a separate thread running in this instance.
 *  This function IS NOT FPS-dependant.
 */
void ProtocolManager::asynchronousUpdate()
{
    // Update from ProtocolManager thread only:
    assert(pthread_equal(pthread_self(), m_asynchronous_update_thread));

    // First deliver asynchronous messages for all protocols
    // =====================================================
    m_async_events_to_process.lock();
    EventList::iterator i = m_async_events_to_process.getData().begin();
    while (i != m_async_events_to_process.getData().end())
    {
        m_async_events_to_process.unlock();

        m_all_protocols[(*i)->getType()].lock();
        bool result = sendEvent(*i);
        m_all_protocols[(*i)->getType()].unlock();

        m_async_events_to_process.lock();
        if (result)
        {
            delete *i;
            i = m_async_events_to_process.getData().erase(i);
        }
        else
        {
            // This should only happen if the protocol has not been started
            // or already terminated (e.g. late ping answer)
            ++i;
        }
    }   // while i != m_events_to_process.end()
    m_async_events_to_process.unlock();

    // Second: update all running protocols
    // ====================================
    // Now update all protocols.
    for (unsigned int i = 0; i < m_all_protocols.size(); i++)
    {
        OneProtocolType &opt = m_all_protocols[i];
        // The lock is likely not necessary, since this function is only
        // called from the ProtocolManager thread, and this thread is also
        // the only one who changes the number of protocols.
        opt.lock();
        opt.update(0, /*async*/true);  // dt does not matter, so set it to 0
        opt.unlock();
    }

    // Process queued events (start, pause, ...) for protocols asynchronously
    // ======================================================================
    m_requests.lock();
    while(m_requests.getData().size()>0)
    {
        ProtocolRequest request = m_requests.getData()[0];
        m_requests.getData().erase(m_requests.getData().begin());
        // Make sure new requests can be queued up while handling requests.
        m_requests.unlock();
        // This is often used that terminating a protocol unpauses another,
        // so the m_requests queue must not be locked while executing requests.
        switch (request.getType())
        {
            case PROTOCOL_REQUEST_START:
                startProtocol(request.getProtocol());
                break;
            case PROTOCOL_REQUEST_PAUSE:
                pauseProtocol(request.getProtocol());
                break;
            case PROTOCOL_REQUEST_UNPAUSE:
                unpauseProtocol(request.getProtocol());
                break;
            case PROTOCOL_REQUEST_TERMINATE:
                terminateProtocol(request.getProtocol());
                break;
        }   // switch (type)
        m_requests.lock();
    }   // while m_requests.size()>0
    m_requests.unlock();
}   // asynchronousUpdate

// ----------------------------------------------------------------------------
/** \brief Get a protocol using its type.
 *  \param type : The type of the protocol.
 *  \return The protocol that matches the given type.
 */
Protocol* ProtocolManager::getProtocol(ProtocolType type)
{
    OneProtocolType &opt = m_all_protocols[type];
    if (opt.isEmpty()) return NULL;

    return opt.getFirstProtocol();
}   // getProtocol