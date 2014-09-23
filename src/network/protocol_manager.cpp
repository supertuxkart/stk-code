//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2013 SuperTuxKart-Team
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

#include "network/protocol.hpp"
#include "network/network_manager.hpp"
#include "utils/log.hpp"
#include "utils/time.hpp"

#include <assert.h>
#include <cstdlib>
#include <errno.h>
#include <typeinfo>

void* protocolManagerUpdate(void* data)
{
    ProtocolManager* manager = static_cast<ProtocolManager*>(data);
    while(manager && !manager->exit())
    {
        manager->update();
        StkTime::sleep(2);
    }
    return NULL;
}
void* protocolManagerAsynchronousUpdate(void* data)
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
}

ProtocolManager::ProtocolManager()
{
    pthread_mutex_init(&m_events_mutex, NULL);
    pthread_mutex_init(&m_protocols_mutex, NULL);
    pthread_mutex_init(&m_asynchronous_protocols_mutex, NULL);
    pthread_mutex_init(&m_requests_mutex, NULL);
    pthread_mutex_init(&m_id_mutex, NULL);
    pthread_mutex_init(&m_exit_mutex, NULL);
    m_next_protocol_id = 0;


    pthread_mutex_lock(&m_exit_mutex); // will let the update function run
    /// FIXME used on server because mainloop never running
    /*if (NetworkManager::getInstance()->isServer())
    {
        m_update_thread = (pthread_t*)(malloc(sizeof(pthread_t)));
        pthread_create(m_update_thread, NULL, protocolManagerUpdate, this);
    }*/
    // always run this one
    m_asynchronous_update_thread = (pthread_t*)(malloc(sizeof(pthread_t)));
    pthread_create(m_asynchronous_update_thread, NULL, protocolManagerAsynchronousUpdate, this);
}

ProtocolManager::~ProtocolManager()
{
}


void ProtocolManager::abort()
{
    pthread_mutex_unlock(&m_exit_mutex); // will stop the update function
    pthread_join(*m_asynchronous_update_thread, NULL); // wait the thread to finish
    pthread_mutex_lock(&m_events_mutex);
    pthread_mutex_lock(&m_protocols_mutex);
    pthread_mutex_lock(&m_asynchronous_protocols_mutex);
    pthread_mutex_lock(&m_requests_mutex);
    pthread_mutex_lock(&m_id_mutex);
    for (unsigned int i = 0; i < m_protocols.size() ; i++)
        delete m_protocols[i].protocol;
    for (unsigned int i = 0; i < m_events_to_process.size() ; i++)
        delete m_events_to_process[i].event;
    m_protocols.clear();
    m_requests.clear();
    m_events_to_process.clear();
    pthread_mutex_unlock(&m_events_mutex);
    pthread_mutex_unlock(&m_protocols_mutex);
    pthread_mutex_unlock(&m_asynchronous_protocols_mutex);
    pthread_mutex_unlock(&m_requests_mutex);
    pthread_mutex_unlock(&m_id_mutex);

    pthread_mutex_destroy(&m_events_mutex);
    pthread_mutex_destroy(&m_protocols_mutex);
    pthread_mutex_destroy(&m_asynchronous_protocols_mutex);
    pthread_mutex_destroy(&m_requests_mutex);
    pthread_mutex_destroy(&m_id_mutex);
    pthread_mutex_destroy(&m_exit_mutex);
}

void ProtocolManager::notifyEvent(Event* event)
{
    pthread_mutex_lock(&m_events_mutex);
    Event* event2 = new Event(*event);
    // register protocols that will receive this event
    std::vector<unsigned int> protocols_ids;
    PROTOCOL_TYPE searchedProtocol = PROTOCOL_NONE;
    if (event2->type == EVENT_TYPE_MESSAGE)
    {
        if (event2->data().size() > 0)
        {
            searchedProtocol = (PROTOCOL_TYPE)(event2->data()[0]);
            event2->removeFront(1);
        }
        else
        {
            Log::warn("ProtocolManager", "Not enough data.");
        }
    }
    if (event2->type == EVENT_TYPE_CONNECTED)
    {
        searchedProtocol = PROTOCOL_CONNECTION;
    }
    Log::verbose("ProtocolManager", "Received event for protocols of type %d", searchedProtocol);
    pthread_mutex_lock(&m_protocols_mutex);
    for (unsigned int i = 0; i < m_protocols.size() ; i++)
    {
        if (m_protocols[i].protocol->getProtocolType() == searchedProtocol || event2->type == EVENT_TYPE_DISCONNECTED) // pass data to protocols even when paused
        {
            protocols_ids.push_back(m_protocols[i].id);
        }
    }
    pthread_mutex_unlock(&m_protocols_mutex);
    if (searchedProtocol == PROTOCOL_NONE) // no protocol was aimed, show the msg to debug
    {
        Log::debug("ProtocolManager", "NO PROTOCOL : Message is \"%s\"", event2->data().std_string().c_str());
    }

    if (protocols_ids.size() != 0)
    {
        EventProcessingInfo epi;
        epi.arrival_time = (double)StkTime::getTimeSinceEpoch();
        epi.event = event2;
        epi.protocols_ids = protocols_ids;
        m_events_to_process.push_back(epi); // add the event to the queue
    }
    else
        Log::warn("ProtocolManager", "Received an event for %d that has no destination protocol.", searchedProtocol);
    pthread_mutex_unlock(&m_events_mutex);
}

void ProtocolManager::sendMessage(Protocol* sender, const NetworkString& message, bool reliable)
{
    NetworkString newMessage;
    newMessage.ai8(sender->getProtocolType()); // add one byte to add protocol type
    newMessage += message;
    NetworkManager::getInstance()->sendPacket(newMessage, reliable);
}

void ProtocolManager::sendMessage(Protocol* sender, STKPeer* peer, const NetworkString& message, bool reliable)
{
    NetworkString newMessage;
    newMessage.ai8(sender->getProtocolType()); // add one byte to add protocol type
    newMessage += message;
    NetworkManager::getInstance()->sendPacket(peer, newMessage, reliable);
}
void ProtocolManager::sendMessageExcept(Protocol* sender, STKPeer* peer, const NetworkString& message, bool reliable)
{
    NetworkString newMessage;
    newMessage.ai8(sender->getProtocolType()); // add one byte to add protocol type
    newMessage += message;
    NetworkManager::getInstance()->sendPacketExcept(peer, newMessage, reliable);
}

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
}

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
}

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
}

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
}

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
}

void ProtocolManager::startProtocol(ProtocolInfo protocol)
{
    // add the protocol to the protocol vector so that it's updated
    pthread_mutex_lock(&m_protocols_mutex);
    pthread_mutex_lock(&m_asynchronous_protocols_mutex);
    Log::info("ProtocolManager", "A %s protocol with id=%u has been started. There are %ld protocols running.", typeid(*protocol.protocol).name(), protocol.id, m_protocols.size()+1);
    m_protocols.push_back(protocol);
    // setup the protocol and notify it that it's started
    protocol.protocol->setListener(this);
    protocol.protocol->setup();
    pthread_mutex_unlock(&m_protocols_mutex);
    pthread_mutex_unlock(&m_asynchronous_protocols_mutex);
}
void ProtocolManager::stopProtocol(ProtocolInfo protocol)
{

}
void ProtocolManager::pauseProtocol(ProtocolInfo protocol)
{
    for (unsigned int i = 0; i < m_protocols.size(); i++)
    {
        if (m_protocols[i].protocol == protocol.protocol && m_protocols[i].state == PROTOCOL_STATE_RUNNING)
        {
            m_protocols[i].state = PROTOCOL_STATE_PAUSED;
            m_protocols[i].protocol->pause();
        }
    }
}
void ProtocolManager::unpauseProtocol(ProtocolInfo protocol)
{
    for (unsigned int i = 0; i < m_protocols.size(); i++)
    {
        if (m_protocols[i].protocol == protocol.protocol && m_protocols[i].state == PROTOCOL_STATE_PAUSED)
        {
            m_protocols[i].state = PROTOCOL_STATE_RUNNING;
            m_protocols[i].protocol->unpause();
        }
    }
}
void ProtocolManager::protocolTerminated(ProtocolInfo protocol)
{
    pthread_mutex_lock(&m_protocols_mutex); // be sure that noone accesses the protocols vector while we erase a protocol
    pthread_mutex_lock(&m_asynchronous_protocols_mutex);
    int offset = 0;
    std::string protocol_type = typeid(*protocol.protocol).name();
    for (unsigned int i = 0; i < m_protocols.size(); i++)
    {
        if (m_protocols[i-offset].protocol == protocol.protocol)
        {
            delete m_protocols[i].protocol;
            m_protocols.erase(m_protocols.begin()+(i-offset), m_protocols.begin()+(i-offset)+1);
            offset++;
        }
    }
    Log::info("ProtocolManager", "A %s protocol has been terminated. There are %ld protocols running.", protocol_type.c_str(), m_protocols.size());
    pthread_mutex_unlock(&m_asynchronous_protocols_mutex);
    pthread_mutex_unlock(&m_protocols_mutex);
}

bool ProtocolManager::propagateEvent(EventProcessingInfo* event, bool synchronous)
{
    int index = 0;
    for (unsigned int i = 0; i < m_protocols.size(); i++)
    {
        if (event->protocols_ids[index] == m_protocols[i].id)
        {
            bool result = false;
            if (synchronous)
                result = m_protocols[i].protocol->notifyEvent(event->event);
            else
                result = m_protocols[i].protocol->notifyEventAsynchronous(event->event);
            if (result)
                event->protocols_ids.pop_back();
            else
                index++;
        }
    }
    if (event->protocols_ids.size() == 0 || (StkTime::getTimeSinceEpoch()-event->arrival_time) >= TIME_TO_KEEP_EVENTS)
    {
        // because we made a copy of the event
        delete event->event->peer; // no more need of that
        delete event->event;
        return true;
    }
    return false;
}

void ProtocolManager::update()
{
    // before updating, notice protocols that they have received events
    pthread_mutex_lock(&m_events_mutex); // secure threads
    int size = (int)m_events_to_process.size();
    int offset = 0;
    for (int i = 0; i < size; i++)
    {
        bool result = propagateEvent(&m_events_to_process[i+offset], true);
        if (result)
        {
            m_events_to_process.erase(m_events_to_process.begin()+i+offset,m_events_to_process.begin()+i+offset+1);
            offset --;
        }
    }
    pthread_mutex_unlock(&m_events_mutex); // release the mutex
    // now update all protocols
    pthread_mutex_lock(&m_protocols_mutex);
    for (unsigned int i = 0; i < m_protocols.size(); i++)
    {
        if (m_protocols[i].state == PROTOCOL_STATE_RUNNING)
            m_protocols[i].protocol->update();
    }
    pthread_mutex_unlock(&m_protocols_mutex);
}

void ProtocolManager::asynchronousUpdate()
{
    // before updating, notice protocols that they have received information
    pthread_mutex_lock(&m_events_mutex); // secure threads
    int size = (int)m_events_to_process.size();
    int offset = 0;
    for (int i = 0; i < size; i++)
    {
        bool result = propagateEvent(&m_events_to_process[i+offset], false);
        if (result)
        {
            m_events_to_process.erase(m_events_to_process.begin()+i+offset,m_events_to_process.begin()+i+offset+1);
            offset --;
        }
    }
    pthread_mutex_unlock(&m_events_mutex); // release the mutex

    // now update all protocols that need to be updated in asynchronous mode
    pthread_mutex_lock(&m_asynchronous_protocols_mutex);
    for (unsigned int i = 0; i < m_protocols.size(); i++)
    {
        if (m_protocols[i].state == PROTOCOL_STATE_RUNNING)
            m_protocols[i].protocol->asynchronousUpdate();
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
}

int ProtocolManager::runningProtocolsCount()
{
    return (int)m_protocols.size();
}

PROTOCOL_STATE ProtocolManager::getProtocolState(uint32_t id)
{
    for (unsigned int i = 0; i < m_protocols.size(); i++)
    {
        if (m_protocols[i].id == id) // we know a protocol with that id
            return m_protocols[i].state; // return its state
    }
    // the protocol isn't running right now
    for (unsigned int i = 0; i < m_requests.size(); i++)
    {
        if (m_requests[i].protocol_info.id == id) // the protocol is going to be started
            return PROTOCOL_STATE_RUNNING; // we can say it's running
    }
    return PROTOCOL_STATE_TERMINATED; // else, it's already finished
}

PROTOCOL_STATE ProtocolManager::getProtocolState(Protocol* protocol)
{
    for (unsigned int i = 0; i < m_protocols.size(); i++)
    {
        if (m_protocols[i].protocol == protocol) // the protocol is known
            return m_protocols[i].state; // return its state
    }
    for (unsigned int i = 0; i < m_requests.size(); i++)
    {
        if (m_requests[i].protocol_info.protocol == protocol) // the protocol is going to be started
            return PROTOCOL_STATE_RUNNING; // we can say it's running
    }
    return PROTOCOL_STATE_TERMINATED; // we don't know this protocol at all, it's finished
}

uint32_t ProtocolManager::getProtocolID(Protocol* protocol)
{
    for (unsigned int i = 0; i < m_protocols.size(); i++)
    {
        if (m_protocols[i].protocol == protocol)
            return m_protocols[i].id;
    }
    return 0;
}

Protocol* ProtocolManager::getProtocol(uint32_t id)
{
    for (unsigned int i = 0; i < m_protocols.size(); i++)
    {
        if (m_protocols[i].id == id)
            return m_protocols[i].protocol;
    }
    return NULL;
}

Protocol* ProtocolManager::getProtocol(PROTOCOL_TYPE type)
{
    for (unsigned int i = 0; i < m_protocols.size(); i++)
    {
        if (m_protocols[i].protocol->getProtocolType() == type)
            return m_protocols[i].protocol;
    }
    return NULL;
}

bool ProtocolManager::isServer()
{
    return NetworkManager::getInstance()->isServer();
}

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
}

void ProtocolManager::assignProtocolId(ProtocolInfo* protocol_info)
{
    pthread_mutex_lock(&m_id_mutex);
    protocol_info->id = m_next_protocol_id;
    m_next_protocol_id++;
    pthread_mutex_unlock(&m_id_mutex);
}


