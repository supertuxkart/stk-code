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
#include "network/network_manager.hpp"
#include "network/protocol.hpp"
#include "utils/log.hpp"
#include "utils/time.hpp"

#include <assert.h>
#include <cstdlib>
#include <errno.h>
#include <typeinfo>


ProtocolManager::ProtocolManager()
{
    pthread_mutex_init(&m_asynchronous_protocols_mutex, NULL);
    pthread_mutex_init(&m_requests_mutex, NULL);
    pthread_mutex_init(&m_id_mutex, NULL);
    pthread_mutex_init(&m_exit_mutex, NULL);
    m_next_protocol_id = 0;

    pthread_mutex_lock(&m_exit_mutex); // will let the update function run

    m_asynchronous_update_thread = (pthread_t*)(malloc(sizeof(pthread_t)));
    pthread_create(m_asynchronous_update_thread, NULL,
                   ProtocolManager::mainLoop, this);
}   // ProtocolManager

// ----------------------------------------------------------------------------

void* ProtocolManager::mainLoop(void* data)
{
    ProtocolManager* manager = static_cast<ProtocolManager*>(data);
    manager->m_asynchronous_thread_running = true;
    while(manager && !manager->exit())
    {
        manager->asynchronousUpdate();
        StkTime::sleep(2);
    }
    manager->m_asynchronous_thread_running = false;
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
    pthread_mutex_unlock(&m_exit_mutex); // will stop the update function
    pthread_join(*m_asynchronous_update_thread, NULL); // wait the thread to finish
    m_events_to_process.lock();
    m_protocols.lock();
    pthread_mutex_lock(&m_asynchronous_protocols_mutex);
    pthread_mutex_lock(&m_requests_mutex);
    pthread_mutex_lock(&m_id_mutex);
    for (unsigned int i = 0; i < m_protocols.getData().size() ; i++)
        delete m_protocols.getData()[i];
    for (unsigned int i = 0; i < m_events_to_process.getData().size() ; i++)
        delete m_events_to_process.getData()[i].m_event;
    m_protocols.getData().clear();
    m_requests.clear();
    m_events_to_process.getData().clear();
    m_events_to_process.unlock();
    m_protocols.unlock();
    pthread_mutex_unlock(&m_asynchronous_protocols_mutex);
    pthread_mutex_unlock(&m_requests_mutex);
    pthread_mutex_unlock(&m_id_mutex);

    pthread_mutex_destroy(&m_asynchronous_protocols_mutex);
    pthread_mutex_destroy(&m_requests_mutex);
    pthread_mutex_destroy(&m_id_mutex);
    pthread_mutex_destroy(&m_exit_mutex);
}   // abort

// ----------------------------------------------------------------------------
/** \brief Function that processes incoming events.
 *  This function is called by the network manager each time there is an
 *  incoming packet.
 */
void ProtocolManager::propagateEvent(Event* event)
{
    m_events_to_process.lock();

    // register protocols that will receive this event
    ProtocolType searched_protocol = PROTOCOL_NONE;
    if (event->getType() == EVENT_TYPE_MESSAGE)
    {
        if (event->data().size() > 0)
        {
            searched_protocol = (ProtocolType)(event->data()[0]);
            event->removeFront(1);
        }
        else
        {
            Log::warn("ProtocolManager", "Not enough data.");
        }
    }
    else if (event->getType() == EVENT_TYPE_CONNECTED)
    {
        searched_protocol = PROTOCOL_CONNECTION;
    }
    Log::verbose("ProtocolManager", "Received event for protocols of type %d",
                  searched_protocol);

    std::vector<unsigned int> protocols_ids;
    m_protocols.lock();
    for (unsigned int i = 0; i < m_protocols.getData().size() ; i++)
    {
        const Protocol *p = m_protocols.getData()[i];
        // Pass data to protocols even when paused
        if (p->getProtocolType() == searched_protocol ||
            event->getType() == EVENT_TYPE_DISCONNECTED)
        {
            protocols_ids.push_back(p->getId());
        }
    }    // for i in m_protocols
    m_protocols.unlock();

    // no protocol was aimed, show the msg to debug
    if (searched_protocol == PROTOCOL_NONE)
    {
        Log::debug("ProtocolManager", "NO PROTOCOL : Message is \"%s\"",
                    event->data().std_string().c_str());
    }

    if (protocols_ids.size() != 0)
    {
        EventProcessingInfo epi;
        epi.m_arrival_time = (double)StkTime::getTimeSinceEpoch();
        epi.m_event = event;
        epi.m_protocols_ids = protocols_ids;
        // Add the event to the queue. After the event is handled
        // its memory will be freed.
        m_events_to_process.getData().push_back(epi); 
    }
    else
    {
        Log::warn("ProtocolManager",
                  "Received an event for %d that has no destination protocol.",
                  searched_protocol);
        // Free the memory for the vent
        delete event;
    }
    m_events_to_process.unlock();
}   // propagateEvent

// ----------------------------------------------------------------------------
void ProtocolManager::sendMessage(Protocol* sender, const NetworkString& message,
                                  bool reliable)
{
    NetworkString newMessage(1+message.size());
    newMessage.ai8(sender->getProtocolType()); // add one byte to add protocol type
    newMessage += message;
    NetworkManager::getInstance()->sendPacket(newMessage, reliable);
}   // sendMessage

// ----------------------------------------------------------------------------
void ProtocolManager::sendMessage(Protocol* sender, STKPeer* peer,
                                  const NetworkString& message, bool reliable)
{
    NetworkString newMessage(1+message.size());
    newMessage.ai8(sender->getProtocolType()); // add one byte to add protocol type
    newMessage += message;
    NetworkManager::getInstance()->sendPacket(peer, newMessage, reliable);
}   // sendMessage

// ----------------------------------------------------------------------------
void ProtocolManager::sendMessageExcept(Protocol* sender, STKPeer* peer,
                                        const NetworkString& message,
                                        bool reliable)
{
    NetworkString new_message(1+message.size());
    new_message.ai8(sender->getProtocolType()); // add one byte to add protocol type
    new_message += message;
    STKHost::get()->sendPacketExcept(peer, new_message, reliable);
}   // sendMessageExcept

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
    pthread_mutex_lock(&m_requests_mutex);
    m_requests.push_back(req);
    pthread_mutex_unlock(&m_requests_mutex);

    return req.getProtocol()->getId();
}   // requestStart

// ----------------------------------------------------------------------------
/** \brief Asks the manager to stop a protocol.
 *  This function will store the request, and process it at a time it is
 *  thread-safe.
 *  \param protocol : A pointer to the protocol to stop
 */
void ProtocolManager::requestStop(Protocol* protocol)
{
    if (!protocol)
        return;
    // create the request
    ProtocolRequest req(PROTOCOL_REQUEST_STOP, protocol);
    // add it to the request stack
    pthread_mutex_lock(&m_requests_mutex);
    m_requests.push_back(req);
    pthread_mutex_unlock(&m_requests_mutex);
}   // requestStop

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
    pthread_mutex_lock(&m_requests_mutex);
    m_requests.push_back(req);
    pthread_mutex_unlock(&m_requests_mutex);
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
    pthread_mutex_lock(&m_requests_mutex);
    m_requests.push_back(req);
    pthread_mutex_unlock(&m_requests_mutex);
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
    pthread_mutex_lock(&m_requests_mutex);
    // check that the request does not already exist :
    for (unsigned int i = 0; i < m_requests.size(); i++)
    {
        if (m_requests[i].m_protocol == protocol)
        {
            pthread_mutex_unlock(&m_requests_mutex);
            return;
        }
    }
    m_requests.push_back(req);
    pthread_mutex_unlock(&m_requests_mutex);
}   // requestTerminate

// ----------------------------------------------------------------------------
/** \brief Starts a protocol.
 *  Add the protocol info to the m_protocols vector.
 *  \param protocol : ProtocolInfo to start.
 */
void ProtocolManager::startProtocol(Protocol *protocol)
{
  //  assert(protocol_info.m_state == PROTOCOL_STATE_INITIALISING);
    // add the protocol to the protocol vector so that it's updated
    m_protocols.lock();
    pthread_mutex_lock(&m_asynchronous_protocols_mutex);
    Log::info("ProtocolManager",
        "A %s protocol with id=%u has been started. There are %ld protocols running.", 
              typeid(protocol).name(), protocol->getId(),
              m_protocols.getData().size()+1);
    m_protocols.getData().push_back(protocol);
    // setup the protocol and notify it that it's started
    protocol->setup();
    protocol->setState(PROTOCOL_STATE_RUNNING);
    m_protocols.unlock();
    pthread_mutex_unlock(&m_asynchronous_protocols_mutex);
}   // startProtocol

// ----------------------------------------------------------------------------
void ProtocolManager::stopProtocol(Protocol *protocol)
{
}   // stopProtocol

// ----------------------------------------------------------------------------
void ProtocolManager::pauseProtocol(Protocol *protocol)
{
    assert(protocol->getState() == PROTOCOL_STATE_RUNNING);
    protocol->setState(PROTOCOL_STATE_PAUSED);

    return;

    // FIXME ... why so complicated???
#ifdef XX
    // FIXME Does this need to be locked?
    for (unsigned int i = 0; i < m_protocols.getData().size(); i++)
    {
        //ProtocolInfo *pi = m_protocols.getData()[i];
        if (pi->m_protocol == protocol.m_protocol &&
            pi->m_state == PROTOCOL_STATE_RUNNING)
        {
            pi->m_state = PROTOCOL_STATE_PAUSED;
            pi->m_protocol->pause();
        }
    }
#endif
}   // pauseProtocol

// ----------------------------------------------------------------------------
void ProtocolManager::unpauseProtocol(Protocol *protocol)
{
    protocol->setState(PROTOCOL_STATE_RUNNING);
    protocol->unpause();
    //FIXME: why call protocol->unpause() (which would queue a new request and
    // then calls this function again) ... and why so complicated??
#ifdef XX
    // FIXME Does this need to be locked??
    for (unsigned int i = 0; i < m_protocols.getData().size(); i++)
    {
        ProtocolInfo *p = m_protocols.getData()[i];
        if (p->m_protocol == protocol.m_protocol &&
            p->m_state == PROTOCOL_STATE_PAUSED)
        {
            p->m_state = PROTOCOL_STATE_RUNNING;
            p->m_protocol->unpause();
        }
    }
#endif
}   // unpauseProtocol

// ----------------------------------------------------------------------------
void ProtocolManager::protocolTerminated(Protocol *protocol)
{
    // Be sure that noone accesses the protocols vector while we erase a protocol
    m_protocols.lock();
    pthread_mutex_lock(&m_asynchronous_protocols_mutex);
    int offset = 0;
    std::string protocol_type = typeid(protocol).name();
    for (unsigned int i = 0; i < m_protocols.getData().size(); i++)
    {
        if (m_protocols.getData()[i-offset] == protocol)
        {
            delete protocol;
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
}   // protocolTerminated

// ----------------------------------------------------------------------------
/** Sends the event to the corresponding protocol.
 */
bool ProtocolManager::sendEvent(EventProcessingInfo* event, bool synchronous)
{
    m_protocols.lock();
    int index = 0;
    for (unsigned int i = 0; i < m_protocols.getData().size(); i++)
    {
        if (event->m_protocols_ids[index] == m_protocols.getData()[i]->getId())
        {
            bool result = false;
            if (synchronous)
                result = m_protocols.getData()[i]
                         ->notifyEvent(event->m_event);
            else
                result = m_protocols.getData()[i]
                         ->notifyEventAsynchronous(event->m_event);
            if (result)
                event->m_protocols_ids.pop_back();
            else
                index++;
        }
    }
    m_protocols.unlock();

    if (event->m_protocols_ids.size() == 0 || 
        (StkTime::getTimeSinceEpoch()-event->m_arrival_time) >= TIME_TO_KEEP_EVENTS)
    {
        delete event->m_event;
        return true;
    }
    return false;
}   // sendEvent

// ----------------------------------------------------------------------------
/** \brief Updates the manager.
 *
 *  This function processes the events queue, notifies the concerned
 *  protocols that they have events to process. Then ask all protocols
 *  to update themselves. Finally processes stored requests about
 *  starting, stoping, pausing etc... protocols.
 *  This function is called by the main loop.
 *  This function IS FPS-dependant.
 */
void ProtocolManager::update()
{
    // before updating, notify protocols that they have received events
    m_events_to_process.lock();
    int size = (int)m_events_to_process.getData().size();
    int offset = 0;
    for (int i = 0; i < size; i++)
    {
        bool result = sendEvent(&m_events_to_process.getData()[i+offset], true);
        if (result)
        {
            m_events_to_process.getData()
                               .erase(m_events_to_process.getData().begin()+i+offset,
                                      m_events_to_process.getData().begin()+i+offset+1);
            offset --;
        }
    }
    m_events_to_process.unlock();
    // now update all protocols
    m_protocols.lock();
    for (unsigned int i = 0; i < m_protocols.getData().size(); i++)
    {
        if (m_protocols.getData()[i]->getState() == PROTOCOL_STATE_RUNNING)
            m_protocols.getData()[i]->update();
    }
    m_protocols.unlock();
}   // update

// ----------------------------------------------------------------------------
/** \brief Updates the manager.
 *  This function processes the events queue, notifies the concerned
 *  protocols that they have events to process. Then ask all protocols
 *  to update themselves. Finally processes stored requests about
 *  starting, stoping, pausing etc... protocols.
 *  This function is called in a thread.
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
        bool result = sendEvent(&m_events_to_process.getData()[i+offset], false);
        if (result)
        {
            m_events_to_process.getData()
                               .erase(m_events_to_process.getData().begin()+i+offset,
                                      m_events_to_process.getData().begin()+i+offset+1);
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

    // process queued events for protocols
    // these requests are asynchronous
    pthread_mutex_lock(&m_requests_mutex);
    for (unsigned int i = 0; i < m_requests.size(); i++)
    {
        switch (m_requests[i].getType())
        {
            case PROTOCOL_REQUEST_START:
                startProtocol(m_requests[i].getProtocol());
                break;
            case PROTOCOL_REQUEST_STOP:
                stopProtocol(m_requests[i].getProtocol());
                break;
            case PROTOCOL_REQUEST_PAUSE:
                pauseProtocol(m_requests[i].getProtocol());
                break;
            case PROTOCOL_REQUEST_UNPAUSE:
                unpauseProtocol(m_requests[i].getProtocol());
                break;
            case PROTOCOL_REQUEST_TERMINATE:
                protocolTerminated(m_requests[i].getProtocol());
                break;
        }
    }
    m_requests.clear();
    pthread_mutex_unlock(&m_requests_mutex);
}   // asynchronousUpdate

// ----------------------------------------------------------------------------
/** \brief Get the state of a protocol using its id.
 *  \param id : The id of the protocol you seek the state.
 *  \return The state of the protocol.
 */
ProtocolState ProtocolManager::getProtocolState(uint32_t id)
{
    //FIXME that actually need a lock, but it also can be called from
    // a locked section anyway
    for (unsigned int i = 0; i < m_protocols.getData().size(); i++)
    {
        if (m_protocols.getData()[i]->getId() == id) // we know a protocol with that id
            return m_protocols.getData()[i]->getState();
    }
    // the protocol isn't running right now
    for (unsigned int i = 0; i < m_requests.size(); i++)
    {
        // the protocol is going to be started
        if (m_requests[i].m_protocol->getId() == id)
            return PROTOCOL_STATE_RUNNING; // we can say it's running
    }
    return PROTOCOL_STATE_TERMINATED; // else, it's already finished
}   // getProtocolState

// ----------------------------------------------------------------------------
/** \brief Get the state of a protocol using a pointer on it.
 *  \param protocol : A pointer to the protocol you seek the state.
 *  \return The state of the protocol.
 */
ProtocolState ProtocolManager::getProtocolState(Protocol* protocol)
{
    // FIXME Does this need to be locked?
    for (unsigned int i = 0; i < m_protocols.getData().size(); i++)
    {
        if (m_protocols.getData()[i] == protocol) // the protocol is known
            return  m_protocols.getData()[i]->getState();
    }
    for (unsigned int i = 0; i < m_requests.size(); i++)
    {
        // the protocol is going to be started
        if (m_requests[i].m_protocol == protocol)
            return PROTOCOL_STATE_RUNNING; // we can say it's running
    }
    // we don't know this protocol at all, it's finished
    return PROTOCOL_STATE_TERMINATED;
}   // getProtocolState

// ----------------------------------------------------------------------------
/** \brief Get the id of a protocol.
 *  \param protocol : A pointer to the protocol you seek the id.
 *  \return The id of the protocol pointed by the protocol parameter.
 */
uint32_t ProtocolManager::getProtocolID(Protocol* protocol)
{
    // FIXME: Does this need to be locked?
    for (unsigned int i = 0; i < m_protocols.getData().size(); i++)
    {
        if (m_protocols.getData()[i] == protocol)
            return m_protocols.getData()[i]->getId();
    }
    return 0;
}   // getProtocolID

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
/** \brief Know whether the app is a server.
 *  \return True if this application is in server mode, false elseway.
 */
bool ProtocolManager::isServer()
{
    return NetworkManager::getInstance()->isServer();
}   // isServer

// ----------------------------------------------------------------------------
/*! \brief Tells if we need to stop the update thread.
 */
int ProtocolManager::exit()
{
  switch(pthread_mutex_trylock(&m_exit_mutex)) {
    case 0: /* if we got the lock, unlock and return 1 (true) */
      pthread_mutex_unlock(&m_exit_mutex);
      return 1;
    case EBUSY: /* return 0 (false) if the mutex was locked */
      return 0;
  }
  return 1;
}   // exit

// ----------------------------------------------------------------------------
/** \brief Assign an id to a protocol.
 *  This function will assign m_next_protocol_id as the protocol id.
 *  This id starts at 0 at the beginning and is increased by 1 each time
 *  a protocol starts.
 *  \param protocol_info : The protocol info that needs an id.
 */
uint32_t ProtocolManager::getNextProtocolId()
{
    pthread_mutex_lock(&m_id_mutex);
    uint32_t id = m_next_protocol_id;
    m_next_protocol_id++;
    pthread_mutex_unlock(&m_id_mutex);
    return id;
}   // getNextProtocolId


