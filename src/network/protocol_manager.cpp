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
#include "network/network_config.hpp"
#include "network/protocols/game_protocol.hpp"
#include "network/protocols/server_lobby.hpp"
#include "network/socket_address.hpp"
#include "network/stk_peer.hpp"
#include "utils/log.hpp"
#include "utils/profiler.hpp"
#include "utils/time.hpp"
#include "utils/vs.hpp"

#include <algorithm>
#include <assert.h>
#include <cstdlib>
#include <errno.h>
#include <functional>
#include <typeinfo>

// ============================================================================
std::weak_ptr<ProtocolManager> ProtocolManager::m_protocol_manager[PT_COUNT];
// ============================================================================
std::shared_ptr<ProtocolManager> ProtocolManager::createInstance()
{
    if (!emptyInstance())
    {
        Log::fatal("ProtocolManager",
            "Create only 1 instance of ProtocolManager!");
        return NULL;
    }
    // This is called in STKHost creation, so its process type will be told
    // here
    ProcessType pt = STKProcess::getType();
    auto pm = std::make_shared<ProtocolManager>();
    pm->m_asynchronous_update_thread = std::thread([pm, pt]()
        {
            std::string thread_name = "PtlMgr";
            if (pt == PT_CHILD)
                thread_name += "_child";
            VS::setThreadName(thread_name.c_str());
            STKProcess::init(pt);
            while(!pm->m_exit.load())
            {
                pm->asynchronousUpdate();
                PROFILER_PUSH_CPU_MARKER("sleep", 0, 255, 255);
                StkTime::sleep(2);
                PROFILER_POP_CPU_MARKER();
            }
        });
    if (NetworkConfig::get()->isServer())
    {
        pm->m_game_protocol_thread = std::thread([pm, pt]()
            {
                VS::setThreadName("CtrlEvents");
                STKProcess::init(pt);
                while (true)
                {
                    std::unique_lock<std::mutex> ul(pm->m_game_protocol_mutex);
                    pm->m_game_protocol_cv.wait(ul, [&pm]
                        {
                            return !pm->m_controller_events_list.empty();
                        });
                    Event* event_top = pm->m_controller_events_list.front();
                    pm->m_controller_events_list.pop_front();
                    ul.unlock();
                    if (event_top == NULL)
                        break;
                    auto sl = LobbyProtocol::get<ServerLobby>();
                    if (sl)
                    {
                        ServerLobby::ServerState ss = sl->getCurrentState();
                        if (!(ss >= ServerLobby::WAIT_FOR_WORLD_LOADED &&
                            ss <= ServerLobby::RACING))
                        {
                            delete event_top;
                            continue;
                        }
                    }
                    auto gp = GameProtocol::lock();
                    if (gp)
                        gp->notifyEventAsynchronous(event_top);
                    delete event_top;
                }
            });
    }
    m_protocol_manager[pt] = pm;
    return pm;
}   // createInstance

// ----------------------------------------------------------------------------
ProtocolManager::ProtocolManager()
{
    m_exit.store(false);
}   // ProtocolManager

// ----------------------------------------------------------------------------
ProtocolManager::~ProtocolManager()
{
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

    for (EventList::iterator i = m_controller_events_list.begin();
                             i!= m_controller_events_list.end(); ++i)
        delete *i;
    m_controller_events_list.clear();

}   // ~ProtocolManager

// ----------------------------------------------------------------------------
void ProtocolManager::OneProtocolType::abort()
{
    for (auto& p : m_protocols)
    {
        Protocol* protocol_ptr = p.get();
        Log::info("ProtocolManager", "A %s protocol has been terminated.",
            typeid(*protocol_ptr).name());
    }
    m_protocols.clear();
}   // OneProtocolType::abort

// ----------------------------------------------------------------------------
/** \brief Stops the protocol manager.
 */
void ProtocolManager::abort()
{
    m_exit.store(true);
    if (NetworkConfig::get()->isServer())
    {
        std::unique_lock<std::mutex> ul(m_game_protocol_mutex);
        m_controller_events_list.push_back(NULL);
        m_game_protocol_cv.notify_one();
        ul.unlock();
        m_game_protocol_thread.join();
    }
    // wait the thread to finish
    m_asynchronous_update_thread.join();
}   // abort

// ----------------------------------------------------------------------------
/** \brief Function that processes incoming events.
 *  This function is called by the network manager each time there is an
 *  incoming packet.
 */
void ProtocolManager::propagateEvent(Event* event)
{
    // Special handling for contoller events in server
    if (NetworkConfig::get()->isServer() &&
        event->getType() == EVENT_TYPE_MESSAGE &&
        event->data().getProtocolType() == PROTOCOL_CONTROLLER_EVENTS)
    {
        std::lock_guard<std::mutex> lock(m_game_protocol_mutex);
        m_controller_events_list.push_back(event);
        m_game_protocol_cv.notify_one();
        return;
    }
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
}   // propagateEvent

// ----------------------------------------------------------------------------
/** \brief Asks the manager to start a protocol.
 *  Add the protocol to the protocols vector.
 *  \param protocol : Protocol concerned.
 */
void ProtocolManager::requestStart(std::shared_ptr<Protocol> protocol)
{
    if (!protocol)
        return;
    std::lock_guard<std::mutex> lock(m_protocols_mutex);
    OneProtocolType &opt = m_all_protocols[protocol->getProtocolType()];
    opt.addProtocol(protocol);
}   // requestStart

// ----------------------------------------------------------------------------
/** \brief Notifies the manager that a protocol is terminated.
 *  Remove a protocol from the protocols vector.
 *  \param protocol : Protocol concerned.
 */
void ProtocolManager::requestTerminate(std::shared_ptr<Protocol> protocol)
{
    if (!protocol)
        return;
    std::lock_guard<std::mutex> lock(m_protocols_mutex);
    OneProtocolType &opt = m_all_protocols[protocol->getProtocolType()];
    opt.removeProtocol(protocol);
}   // requestTerminate

// ----------------------------------------------------------------------------
void ProtocolManager::OneProtocolType::addProtocol(std::shared_ptr<Protocol> p)
{
    auto i = std::find(m_protocols.begin(), m_protocols.end(), p);
    Protocol* protocol_ptr = p.get();
    if (i == m_protocols.end())
    {
        // setup the protocol and notify it that it's started
        p->setup();
        Log::info("ProtocolManager",
            "A %s protocol has been started.", typeid(*protocol_ptr).name());
        m_protocols.push_back(p);
    }
    else
    {
        Log::warn("ProtocolManager", "A %s protocol has already started.",
            typeid(*protocol_ptr).name());
    }
}   // addProtocol

// ----------------------------------------------------------------------------
/** Removes a protocol from the list of protocols of a certain type.
 *  Note that the protocol is not deleted.
 *  \param p The protocol to be removed.
*/
void ProtocolManager::OneProtocolType::removeProtocol(std::shared_ptr<Protocol> p)
{
    auto i = std::find(m_protocols.begin(), m_protocols.end(), p);
    Protocol* protocol_ptr = p.get();
    if (i != m_protocols.end())
    {
        m_protocols.erase(i);
        Log::info("ProtocolManager",
            "A %s protocol has been terminated.", typeid(*protocol_ptr).name());
    }
    else
    {
        Log::warn("ProtocolManager",
            "A %s protocol not found in list for removal.",
            typeid(*protocol_ptr).name());
    }
}   // deleteProtocol

// ----------------------------------------------------------------------------
/** Finds a protocol with the given type and requests it to be terminated.
 *  If no such protocol exist it will do nothing
 *  \param type The protocol type to delete.
 */
void ProtocolManager::findAndTerminate(ProtocolType type)
{
    std::lock_guard<std::mutex> lock(m_protocols_mutex);
    OneProtocolType &opt = m_all_protocols[type];
    opt.abort();
}   // findAndTerminate

// ----------------------------------------------------------------------------
/** Calls either notifyEvent(event) or notifyEventAsynchronous(evet) on all
 *  protocols. Note that no locking is done, it is the responsibility of the
 *  caller to avoid race conditions.
 *  \param event The event to deliver to the protocols.
 */
bool ProtocolManager::OneProtocolType::notifyEvent(Event *event)
{
    if (m_protocols.empty()) return false;

    // Either all protocols of a certain type handle connects, or none.
    // So we tet only one of them
    if (event->getType() == EVENT_TYPE_CONNECTED &&
        !m_protocols[0]->handleConnects()) return false;
    if (event->getType() == EVENT_TYPE_DISCONNECTED &&
        !m_protocols[0]->handleDisconnects()) return false;

    bool can_be_deleted = false;
    for (unsigned int i = 0; i < m_protocols.size(); i++)
    {
        bool done = event->isSynchronous()
                  ? m_protocols[i]->notifyEvent(event)
                  : m_protocols[i]->notifyEventAsynchronous(event);
        can_be_deleted |= done;
    }
    return can_be_deleted;
}   // notifyEvent

// ----------------------------------------------------------------------------
/** Sends the event to the corresponding protocol. Returns true if the event
 *  can be ignored, or false otherwise.
 */
bool ProtocolManager::sendEvent(Event* event,
                          std::array<OneProtocolType, PROTOCOL_MAX>& protocols)
{
    bool can_be_deleted = false;
    if (event->getType() == EVENT_TYPE_MESSAGE)
    {
        OneProtocolType &opt =
            protocols.at(event->data().getProtocolType());
        can_be_deleted = opt.notifyEvent(event);
    }
    else   // connect or disconnect event --> test all protocols
    {
        for (unsigned int i = 0; i < protocols.size(); i++)
        {
            can_be_deleted |= protocols.at(i).notifyEvent(event);
        }
    }
    const uint64_t TIME_TO_KEEP_EVENTS = 1000;
    return can_be_deleted || StkTime::getMonoTimeMs() - event->getArrivalTime()
                              >= TIME_TO_KEEP_EVENTS;
}   // sendEvent

// ----------------------------------------------------------------------------
/** Calls either the synchronous update or asynchronous update function in all
 *  protocols of this type.
 *  \param dt Time step size.
 *  \param async True if asynchronousUpdate() should be called.
 */
void ProtocolManager::OneProtocolType::update(int ticks, bool async)
{
    for (unsigned int i = 0; i < m_protocols.size(); i++)
    {
        if (async)
            m_protocols[i]->asynchronousUpdate();
        else
            m_protocols[i]->update(ticks);
    }
}   // update

// ----------------------------------------------------------------------------
/** \brief Updates the manager.
 *
 *  This function processes the events queue, notifies the concerned
 *  protocols that they have events to process. Then asks all protocols
 *  to update themselves. Finally it processes stored requests about
 *  starting, stopping, pausing etc... protocols.
 *  This function is called by the main thread (i.e. from main_loop).
 *  This function IS FPS-dependant.
 */
void ProtocolManager::update(int ticks)
{
    // Update from main thread only:
    assert(std::this_thread::get_id() != m_asynchronous_update_thread.get_id());

    // Get a copied of protocols to prevent long time locking;
    std::unique_lock<std::mutex> ul(m_protocols_mutex);
    auto all_protocols = m_all_protocols;
    ul.unlock();

    // before updating, notify protocols that they have received events
    m_sync_events_to_process.lock();
    EventList::iterator i = m_sync_events_to_process.getData().begin();

    while (i != m_sync_events_to_process.getData().end())
    {
        m_sync_events_to_process.unlock();
        bool can_be_deleted = true;
        try
        {
            can_be_deleted = sendEvent(*i, all_protocols);
        }
        catch (std::exception& e)
        {
            const std::string& name = (*i)->getPeer()->getAddress().toString();
            Log::error("ProtocolManager",
                "Synchronous event error from %s: %s", name.c_str(), e.what());
            Log::error("ProtocolManager", (*i)->data().getLogMessage().c_str());
        }
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
    for (unsigned int i = 0; i < all_protocols.size(); i++)
    {
        OneProtocolType &opt = all_protocols[i];
        opt.update(ticks, /*async*/false);
    }
}   // update

// ----------------------------------------------------------------------------
/** \brief Updates the manager.
 *  This function processes the events queue, notifies the concerned
 *  protocols that they have events to process. Then ask all protocols
 *  to update themselves. Finally processes stored requests about
 *  starting, stopping, pausing etc... protocols.
 *  This function is called in a separate thread running in this instance.
 *  This function IS NOT FPS-dependant.
 */
void ProtocolManager::asynchronousUpdate()
{
    PROFILER_PUSH_CPU_MARKER("Message delivery", 255, 0, 0);
    // First deliver asynchronous messages for all protocols
    // =====================================================
    // Get a copied of protocols to prevent long time locking;
    std::unique_lock<std::mutex> ul(m_protocols_mutex);
    auto all_protocols = m_all_protocols;
    ul.unlock();

    m_async_events_to_process.lock();
    EventList::iterator i = m_async_events_to_process.getData().begin();
    while (i != m_async_events_to_process.getData().end())
    {
        m_async_events_to_process.unlock();

        bool result = true;
        try
        {
            result = sendEvent(*i, all_protocols);
        }
        catch (std::exception& e)
        {
            const std::string& name = (*i)->getPeer()->getAddress().toString();
            Log::error("ProtocolManager", "Asynchronous event "
                "error from %s: %s", name.c_str(), e.what());
            Log::error("ProtocolManager",
                (*i)->data().getLogMessage().c_str());
        }

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

    PROFILER_POP_CPU_MARKER();
    PROFILER_PUSH_CPU_MARKER("Message delivery", 255, 0, 0);

    // Second: update all running protocols
    // ====================================
    // Now update all protocols.
    for (unsigned int i = 0; i < all_protocols.size(); i++)
    {
        OneProtocolType &opt = all_protocols[i];
        opt.update(0, /*async*/true);  // ticks does not matter, so set it to 0
    }

    PROFILER_POP_CPU_MARKER();
}   // asynchronousUpdate

// ----------------------------------------------------------------------------
/** \brief Get a protocol using its type.
 *  \param type : The type of the protocol.
 *  \return The protocol that matches the given type.
 */
std::shared_ptr<Protocol> ProtocolManager::getProtocol(ProtocolType type)
{
    std::lock_guard<std::mutex> lock(m_protocols_mutex);
    OneProtocolType &opt = m_all_protocols[type];
    if (opt.isEmpty())
        return nullptr;

    return opt.getFirstProtocol();
}   // getProtocol
