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
    pthread_mutex_init(&m_asynchronous_protocols_mutex, NULL);
    m_exit.setAtomic(false);
    m_next_protocol_id.setAtomic(0);

    m_asynchronous_update_thread = (pthread_t*)(malloc(sizeof(pthread_t)));
    pthread_create(m_asynchronous_update_thread, NULL,
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
/** \brief Stops the protocol manager.
 */
void ProtocolManager::abort()
{
    m_exit.setAtomic(true);
    pthread_mutex_lock(&m_asynchronous_protocols_mutex);

    m_protocols.lock();
    for (unsigned int i = 0; i < m_protocols.getData().size() ; i++)
        delete m_protocols.getData()[i];
    m_protocols.getData().clear();
    m_protocols.unlock();

    m_events_to_process.lock();
    for (unsigned int i = 0; i < m_events_to_process.getData().size() ; i++)
        delete m_events_to_process.getData()[i];
    m_events_to_process.getData().clear();
    m_events_to_process.unlock();
    
    
    m_requests.lock();
    m_requests.getData().clear();
    m_requests.unlock();

    pthread_mutex_unlock(&m_asynchronous_protocols_mutex);

    pthread_mutex_destroy(&m_asynchronous_protocols_mutex);
    pthread_join(*m_asynchronous_update_thread, NULL); // wait the thread to finish
}   // abort

// ----------------------------------------------------------------------------
/** \brief Function that processes incoming events.
 *  This function is called by the network manager each time there is an
 *  incoming packet.
 */
void ProtocolManager::propagateEvent(Event* event)
{
    m_events_to_process.lock();
    m_events_to_process.getData().push_back(event); 
    m_events_to_process.unlock();
    return;
}   // propagateEvent

// ----------------------------------------------------------------------------
/** \brief Asks the manager to start a protocol.
 * This function will store the request, and process it at a time it is
 * thread-safe.
 * \param protocol : A pointer to the protocol to start
 * \return The unique id of the protocol that is being started.
 */
uint32_t ProtocolManager::requestStart(Protocol* protocol)
{
    // assign a unique id to the protocol.
    protocol->setId(getNextProtocolId());
    // create the request
    ProtocolRequest req(PROTOCOL_REQUEST_START, protocol);
    // add it to the request stack
    m_requests.lock();
    m_requests.getData().push_back(req);
    m_requests.unlock();

    return req.getProtocol()->getId();
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
    // add the protocol to the protocol vector so that it's updated
    m_protocols.lock();
    pthread_mutex_lock(&m_asynchronous_protocols_mutex);
    Log::info("ProtocolManager",
        "A %s protocol with id=%u has been started. There are %ld protocols running.", 
              typeid(*protocol).name(), protocol->getId(),
              m_protocols.getData().size()+1);
    m_protocols.getData().push_back(protocol);
    // setup the protocol and notify it that it's started
    protocol->setup();
    protocol->setState(PROTOCOL_STATE_RUNNING);
    m_protocols.unlock();
    pthread_mutex_unlock(&m_asynchronous_protocols_mutex);
}   // startProtocol

// ----------------------------------------------------------------------------
/** \brief Pauses a protocol.
 *  Pauses a protocol and tells it that it's being paused.
 *  \param protocol : Protocol to pause.
 */
void ProtocolManager::pauseProtocol(Protocol *protocol)
{
    assert(protocol->getState() == PROTOCOL_STATE_RUNNING);
    protocol->setState(PROTOCOL_STATE_PAUSED);
    protocol->paused();
}   // pauseProtocol

// ----------------------------------------------------------------------------
/** \brief Unpauses a protocol.
 *  Unpauses a protocol and notifies it.
 *  \param protocol : Protocol to unpause.
 */
void ProtocolManager::unpauseProtocol(Protocol *protocol)
{
    assert(protocol->getState() == PROTOCOL_STATE_PAUSED);
    protocol->setState(PROTOCOL_STATE_RUNNING);
    protocol->unpaused();
}   // unpauseProtocol

// ----------------------------------------------------------------------------
/** \brief Notes that a protocol is terminated.
 *  Remove a protocol from the protocols vector.
 *  \param protocol : Protocol concerned.
 */
void ProtocolManager::terminateProtocol(Protocol *protocol)
{
    // Be sure that noone accesses the protocols vector while we erase a protocol
    m_protocols.lock();
    pthread_mutex_lock(&m_asynchronous_protocols_mutex);
    int offset = 0;
    std::string protocol_type = typeid(*protocol).name();
    for (unsigned int i = 0; i < m_protocols.getData().size(); i++)
    {
        if (m_protocols.getData()[i-offset] == protocol)
        {
            protocol->setState(PROTOCOL_STATE_TERMINATED);
            m_protocols.getData().erase(m_protocols.getData().begin()+(i-offset),
                                        m_protocols.getData().begin()+(i-offset)+1);
            offset++;
        }
    }
    Log::info("ProtocolManager",
              "A %s protocol has been terminated. There are %ld protocols running.",
              protocol_type.c_str(), m_protocols.getData().size());
    pthread_mutex_unlock(&m_asynchronous_protocols_mutex);
    m_protocols.unlock();
    protocol->terminated();
}   // terminateProtocol

// ----------------------------------------------------------------------------
/** Sends the event to the corresponding protocol.
 */
bool ProtocolManager::sendEvent(Event* event)
{
    m_protocols.lock();
    int count=0;
    for(unsigned int i=0; i<m_protocols.getData().size(); i++)
    {
        Protocol *p = m_protocols.getData()[i];
        bool is_right_protocol = false;
        switch(event->getType())
        {
        case EVENT_TYPE_MESSAGE:
            is_right_protocol = event->data().getProtocolType()==p->getProtocolType();
            break;
        case EVENT_TYPE_DISCONNECTED:
            is_right_protocol = p->handleDisconnects();
            break;
        case EVENT_TYPE_CONNECTED:
            is_right_protocol = p->handleConnects(); 
            break;
        }   // switch event->getType()

        if( is_right_protocol)  
        {
            count ++;
            event->isSynchronous() ? p->notifyEvent(event)
                                   : p->notifyEventAsynchronous(event);
        }
    }   // for i in protocols

    m_protocols.unlock();

    if (count>0 || StkTime::getTimeSinceEpoch()-event->getArrivalTime()
                    >= TIME_TO_KEEP_EVENTS                                  )
    {
        delete event;
        return true;
    }
    return false;
}   // sendEvent

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
    // before updating, notify protocols that they have received events
    m_events_to_process.lock();
    int size = (int)m_events_to_process.getData().size();
    int offset = 0;
    for (int i = 0; i < size; i++)
    {
        // Don't handle asynchronous events here.
        if(!m_events_to_process.getData()[i+offset]->isSynchronous()) continue;
        bool result = sendEvent(m_events_to_process.getData()[i+offset]);
        if (result)
        {
            m_events_to_process.getData()
                               .erase(m_events_to_process.getData().begin()+(i+offset),
                                      m_events_to_process.getData().begin()+(i+offset+1));
            offset --;
        }
    }
    m_events_to_process.unlock();
    // now update all protocols
    m_protocols.lock();
    for (unsigned int i = 0; i < m_protocols.getData().size(); i++)
    {
        if (m_protocols.getData()[i]->getState() == PROTOCOL_STATE_RUNNING)
            m_protocols.getData()[i]->update(dt);
    }
    m_protocols.unlock();
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
    // before updating, notice protocols that they have received information
    m_events_to_process.lock();
    int size = (int)m_events_to_process.getData().size();
    int offset = 0;
    for (int i = 0; i < size; i++)
    {
        // Don't handle synchronous events here.
        if(m_events_to_process.getData()[i+offset]->isSynchronous()) continue;
        bool result = sendEvent(m_events_to_process.getData()[i+offset]);
        if (result)
        {
            m_events_to_process.getData()
                               .erase(m_events_to_process.getData().begin()+(i+offset),
                                      m_events_to_process.getData().begin()+(i+offset+1));
            offset --;
        }
    }
    m_events_to_process.unlock();

    // now update all protocols that need to be updated in asynchronous mode
    pthread_mutex_lock(&m_asynchronous_protocols_mutex);
    // FIXME: does m_protocols need to be locked???
    for (unsigned int i = 0; i < m_protocols.getData().size(); i++)
    {
        if (m_protocols.getData()[i]->getState() == PROTOCOL_STATE_RUNNING)
            m_protocols.getData()[i]->asynchronousUpdate();
    }
    pthread_mutex_unlock(&m_asynchronous_protocols_mutex);

    // Process queued events for protocols
    // these requests are asynchronous
    m_requests.lock();
    while(m_requests.getData().size()>0)
    {
        ProtocolRequest request = m_requests.getData()[0];
        m_requests.getData().erase(m_requests.getData().begin());
        m_requests.unlock();
        // Make sure new requests can be queued up while handling requests.
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
/** \brief Get a protocol using its id.
 *  \param id : Unique ID of the seek protocol.
 *  \return The protocol that has the ID id.
 */
Protocol* ProtocolManager::getProtocol(uint32_t id)
{
    // FIXME: does m_protocols need to be locked??
    for (unsigned int i = 0; i < m_protocols.getData().size(); i++)
    {
        if (m_protocols.getData()[i]->getId() == id)
            return m_protocols.getData()[i];
    }
    return NULL;
}   // getProtocol

// ----------------------------------------------------------------------------
/** \brief Get a protocol using its type.
 *  \param type : The type of the protocol.
 *  \return The protocol that matches the given type.
 */
Protocol* ProtocolManager::getProtocol(ProtocolType type)
{
    // FIXME: Does m_protocols need to be locked?
    for (unsigned int i = 0; i < m_protocols.getData().size(); i++)
    {
        if (m_protocols.getData()[i]->getProtocolType() == type)
            return m_protocols.getData()[i];
    }
    return NULL;
}   // getProtocol

// ----------------------------------------------------------------------------
/** \brief Assign an id to a protocol.
 *  This function will assign m_next_protocol_id as the protocol id.
 *  This id starts at 0 at the beginning and is increased by 1 each time
 *  a protocol starts.
 *  \param protocol_info : The protocol info that needs an id.
 */
uint32_t ProtocolManager::getNextProtocolId()
{
    m_next_protocol_id.lock();
    uint32_t id = m_next_protocol_id.getData();
    m_next_protocol_id.getData()++;
    m_next_protocol_id.unlock();
    return id;
}   // getNextProtocolId


