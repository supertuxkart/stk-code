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
        delete m_protocols.getData()[i].protocol;
    for (unsigned int i = 0; i < m_events_to_process.getData().size() ; i++)
        delete m_events_to_process.getData()[i].event;
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
void ProtocolManager::propagateEvent(Event* event)
{
    m_events_to_process.lock();

    // register protocols that will receive this event
    PROTOCOL_TYPE searched_protocol = PROTOCOL_NONE;
    if (event->getType() == EVENT_TYPE_MESSAGE)
    {
        if (event->data().size() > 0)
        {
            searched_protocol = (PROTOCOL_TYPE)(event->data()[0]);
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
        const ProtocolInfo &pi = m_protocols.getData()[i];
        // Pass data to protocols even when paused
        if (pi.protocol->getProtocolType() == searched_protocol ||
            event->getType() == EVENT_TYPE_DISCONNECTED)
        {
            protocols_ids.push_back(pi.id);
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
        epi.arrival_time = (double)StkTime::getTimeSinceEpoch();
        epi.event = event;
        epi.protocols_ids = protocols_ids;
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
    NetworkString newMessage(1+message.size());
    newMessage.ai8(sender->getProtocolType()); // add one byte to add protocol type
    newMessage += message;
    NetworkManager::getInstance()->sendPacketExcept(peer, newMessage, reliable);
}   // sendMessageExcept

// ----------------------------------------------------------------------------
uint32_t ProtocolManager::requestStart(Protocol* protocol)
{
    // create the request
    ProtocolRequest req;
    ProtocolInfo info;
    info.protocol = protocol;
    info.state = PROTOCOL_STATE_RUNNING;
    assignProtocolId(&info); // assign a unique id to the protocol.
    req.protocol_info = info;
    req.type = PROTOCOL_REQUEST_START;
    // add it to the request stack
    pthread_mutex_lock(&m_requests_mutex);
    m_requests.push_back(req);
    pthread_mutex_unlock(&m_requests_mutex);

    return info.id;
}   // requestStart

// ----------------------------------------------------------------------------
void ProtocolManager::requestStop(Protocol* protocol)
{
    if (!protocol)
        return;
    // create the request
    ProtocolRequest req;
    req.protocol_info.protocol = protocol;
    req.type = PROTOCOL_REQUEST_STOP;
    // add it to the request stack
    pthread_mutex_lock(&m_requests_mutex);
    m_requests.push_back(req);
    pthread_mutex_unlock(&m_requests_mutex);
}   // requestStop

// ----------------------------------------------------------------------------
void ProtocolManager::requestPause(Protocol* protocol)
{
    if (!protocol)
        return;
    // create the request
    ProtocolRequest req;
    req.protocol_info.protocol = protocol;
    req.type = PROTOCOL_REQUEST_PAUSE;
    // add it to the request stack
    pthread_mutex_lock(&m_requests_mutex);
    m_requests.push_back(req);
    pthread_mutex_unlock(&m_requests_mutex);
}   // requestPause

// ----------------------------------------------------------------------------
void ProtocolManager::requestUnpause(Protocol* protocol)
{
    if (!protocol)
        return;
    // create the request
    ProtocolRequest req;
    req.protocol_info.protocol = protocol;
    req.type = PROTOCOL_REQUEST_UNPAUSE;
    // add it to the request stack
    pthread_mutex_lock(&m_requests_mutex);
    m_requests.push_back(req);
    pthread_mutex_unlock(&m_requests_mutex);
}   // requestUnpause

// ----------------------------------------------------------------------------
void ProtocolManager::requestTerminate(Protocol* protocol)
{
    if (!protocol)
        return;
    // create the request
    ProtocolRequest req;
    req.protocol_info.protocol = protocol;
    req.type = PROTOCOL_REQUEST_TERMINATE;
    // add it to the request stack
    pthread_mutex_lock(&m_requests_mutex);
    // check that the request does not already exist :
    for (unsigned int i = 0; i < m_requests.size(); i++)
    {
        if (m_requests[i].protocol_info.protocol == protocol)
        {
            pthread_mutex_unlock(&m_requests_mutex);
            return;
        }
    }
    m_requests.push_back(req);
    pthread_mutex_unlock(&m_requests_mutex);
}   // requestTerminate

// ----------------------------------------------------------------------------
void ProtocolManager::startProtocol(ProtocolInfo protocol)
{
    // add the protocol to the protocol vector so that it's updated
    m_protocols.lock();
    pthread_mutex_lock(&m_asynchronous_protocols_mutex);
    Log::info("ProtocolManager",
        "A %s protocol with id=%u has been started. There are %ld protocols running.", 
              typeid(*protocol.protocol).name(), protocol.id,
              m_protocols.getData().size()+1);
    m_protocols.getData().push_back(protocol);
    // setup the protocol and notify it that it's started
    protocol.protocol->setListener(this);
    protocol.protocol->setup();
    m_protocols.unlock();
    pthread_mutex_unlock(&m_asynchronous_protocols_mutex);
}   // startProtocol

// ----------------------------------------------------------------------------
void ProtocolManager::stopProtocol(ProtocolInfo protocol)
{
}   // stopProtocol

// ----------------------------------------------------------------------------
void ProtocolManager::pauseProtocol(ProtocolInfo protocol)
{
    // FIXME Does this need to be locked?
    for (unsigned int i = 0; i < m_protocols.getData().size(); i++)
    {
        ProtocolInfo &p = m_protocols.getData()[i];
        if (p.protocol == protocol.protocol &&
            p.state == PROTOCOL_STATE_RUNNING)
        {
            p.state = PROTOCOL_STATE_PAUSED;
            p.protocol->pause();
        }
    }
}   // pauseProtocol

// ----------------------------------------------------------------------------
void ProtocolManager::unpauseProtocol(ProtocolInfo protocol)
{
    // FIXME Does this need to be locked??
    for (unsigned int i = 0; i < m_protocols.getData().size(); i++)
    {
        ProtocolInfo &p = m_protocols.getData()[i];
        if (p.protocol == protocol.protocol &&
            p.state == PROTOCOL_STATE_PAUSED)
        {
            p.state = PROTOCOL_STATE_RUNNING;
            p.protocol->unpause();
        }
    }
}   // unpauseProtocol

// ----------------------------------------------------------------------------
void ProtocolManager::protocolTerminated(ProtocolInfo protocol)
{
    // Be sure that noone accesses the protocols vector while we erase a protocol
    m_protocols.lock();
    pthread_mutex_lock(&m_asynchronous_protocols_mutex);
    int offset = 0;
    std::string protocol_type = typeid(*protocol.protocol).name();
    for (unsigned int i = 0; i < m_protocols.getData().size(); i++)
    {
        if (m_protocols.getData()[i-offset].protocol == protocol.protocol)
        {
            delete m_protocols.getData()[i].protocol;
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
        if (event->protocols_ids[index] == m_protocols.getData()[i].id)
        {
            bool result = false;
            if (synchronous)
                result = m_protocols.getData()[i].protocol
                         ->notifyEvent(event->event);
            else
                result = m_protocols.getData()[i].protocol
                         ->notifyEventAsynchronous(event->event);
            if (result)
                event->protocols_ids.pop_back();
            else
                index++;
        }
    }
    m_protocols.unlock();

    if (event->protocols_ids.size() == 0 || 
        (StkTime::getTimeSinceEpoch()-event->arrival_time) >= TIME_TO_KEEP_EVENTS)
    {
        delete event->event;
        return true;
    }
    return false;
}   // sendEvent

// ----------------------------------------------------------------------------
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
        if (m_protocols.getData()[i].state == PROTOCOL_STATE_RUNNING)
            m_protocols.getData()[i].protocol->update();
    }
    m_protocols.unlock();
}   // update

// ----------------------------------------------------------------------------
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
        if (m_protocols.getData()[i].state == PROTOCOL_STATE_RUNNING)
            m_protocols.getData()[i].protocol->asynchronousUpdate();
    }
    pthread_mutex_unlock(&m_asynchronous_protocols_mutex);

    // process queued events for protocols
    // these requests are asynchronous
    pthread_mutex_lock(&m_requests_mutex);
    for (unsigned int i = 0; i < m_requests.size(); i++)
    {
        switch (m_requests[i].type)
        {
            case PROTOCOL_REQUEST_START:
                startProtocol(m_requests[i].protocol_info);
                break;
            case PROTOCOL_REQUEST_STOP:
                stopProtocol(m_requests[i].protocol_info);
                break;
            case PROTOCOL_REQUEST_PAUSE:
                pauseProtocol(m_requests[i].protocol_info);
                break;
            case PROTOCOL_REQUEST_UNPAUSE:
                unpauseProtocol(m_requests[i].protocol_info);
                break;
            case PROTOCOL_REQUEST_TERMINATE:
                protocolTerminated(m_requests[i].protocol_info);
                break;
        }
    }
    m_requests.clear();
    pthread_mutex_unlock(&m_requests_mutex);
}   // asynchronousUpdate

// ----------------------------------------------------------------------------
PROTOCOL_STATE ProtocolManager::getProtocolState(uint32_t id)
{
    //FIXME that actually need a lock, but it also can be called from
    // a locked section anyway
    for (unsigned int i = 0; i < m_protocols.getData().size(); i++)
    {
        if (m_protocols.getData()[i].id == id) // we know a protocol with that id
            return m_protocols.getData()[i].state;
    }
    // the protocol isn't running right now
    for (unsigned int i = 0; i < m_requests.size(); i++)
    {
        // the protocol is going to be started
        if (m_requests[i].protocol_info.id == id)
            return PROTOCOL_STATE_RUNNING; // we can say it's running
    }
    return PROTOCOL_STATE_TERMINATED; // else, it's already finished
}   // getProtocolState

// ----------------------------------------------------------------------------
PROTOCOL_STATE ProtocolManager::getProtocolState(Protocol* protocol)
{
    // FIXME Does this need to be locked?
    for (unsigned int i = 0; i < m_protocols.getData().size(); i++)
    {
        if (m_protocols.getData()[i].protocol == protocol) // the protocol is known
            return  m_protocols.getData()[i].state;
    }
    for (unsigned int i = 0; i < m_requests.size(); i++)
    {
        // the protocol is going to be started
        if (m_requests[i].protocol_info.protocol == protocol)
            return PROTOCOL_STATE_RUNNING; // we can say it's running
    }
    // we don't know this protocol at all, it's finished
    return PROTOCOL_STATE_TERMINATED;
}   // getProtocolState

// ----------------------------------------------------------------------------
uint32_t ProtocolManager::getProtocolID(Protocol* protocol)
{
    // FIXME: Does this need to be locked?
    for (unsigned int i = 0; i < m_protocols.getData().size(); i++)
    {
        if (m_protocols.getData()[i].protocol == protocol)
            return m_protocols.getData()[i].id;
    }
    return 0;
}   // getProtocolID

// ----------------------------------------------------------------------------
Protocol* ProtocolManager::getProtocol(uint32_t id)
{
    // FIXME: does m_protocols need to be locked??
    for (unsigned int i = 0; i < m_protocols.getData().size(); i++)
    {
        if (m_protocols.getData()[i].id == id)
            return m_protocols.getData()[i].protocol;
    }
    return NULL;
}   // getProtocol

// ----------------------------------------------------------------------------
Protocol* ProtocolManager::getProtocol(PROTOCOL_TYPE type)
{
    // FIXME: Does m_protocols need to be locked?
    for (unsigned int i = 0; i < m_protocols.getData().size(); i++)
    {
        if (m_protocols.getData()[i].protocol->getProtocolType() == type)
            return m_protocols.getData()[i].protocol;
    }
    return NULL;
}   // getProtocol

// ----------------------------------------------------------------------------
bool ProtocolManager::isServer()
{
    return NetworkManager::getInstance()->isServer();
}   // isServer

// ----------------------------------------------------------------------------
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
void ProtocolManager::assignProtocolId(ProtocolInfo* protocol_info)
{
    pthread_mutex_lock(&m_id_mutex);
    protocol_info->id = m_next_protocol_id;
    m_next_protocol_id++;
    pthread_mutex_unlock(&m_id_mutex);
}   // assignProtocolId


