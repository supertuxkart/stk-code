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

#include "network/stk_host.hpp"

#include "config/user_config.hpp"
#include "io/file_manager.hpp"
#include "network/event.hpp"
#include "network/network_manager.hpp"
#include "network/protocols/server_lobby_room_protocol.hpp"
#include "network/server_console.hpp"
#include "utils/log.hpp"
#include "utils/time.hpp"

#include <string.h>
#if defined(WIN32)
#  include "ws2tcpip.h"
#  define inet_ntop InetNtop
#else
#  include <arpa/inet.h>
#  include <errno.h>
#  include <sys/socket.h>
#endif
#include <pthread.h>
#include <signal.h>

STKHost  *STKHost::m_stk_host    = NULL;
int       STKHost::m_max_players = 0;
bool      STKHost::m_is_server   = false;

// ============================================================================
/** Constructor that just initialises this object (esp. opening the packet
 *  log file), but it does not start a listener thread.
 */
STKHost::STKHost()
{
    m_network          = NULL;
    m_listening_thread = NULL;
    pthread_mutex_init(&m_exit_mutex, NULL);

    Network::openLog();
    ProtocolManager::getInstance<ProtocolManager>();

    if (m_is_server)
    {
        ServerConsole *sc = new ServerConsole();
        sc->run();
        setupServer(STKHost::HOST_ANY, 7321, 16, 2, 0, 0);
        startListening();
        ProtocolManager::getInstance()->requestStart(new ServerLobbyRoomProtocol());
    }
}   // STKHost

// ----------------------------------------------------------------------------
/** Destructor. Stops the listening thread, closes the packet log file and
 *  destroys the enet host.
 */
STKHost::~STKHost()
{
    Network::closeLog();
    stopListening();
    delete m_network;
}   // ~STKHost

// ----------------------------------------------------------------------------
/** \brief Starts the listening of events from ENet.
 *  Starts a thread for receiveData that updates it as often as possible.
 */
void STKHost::startListening()
{
    pthread_mutex_lock(&m_exit_mutex); // will let the update function start
    m_listening_thread = new pthread_t;
    pthread_create(m_listening_thread, NULL, &STKHost::mainLoop, this);
}   // startListening

// ----------------------------------------------------------------------------
/** \brief Stops the listening of events from ENet.
 *  Stops the thread that was receiving events.
 */
void STKHost::stopListening()
{
    if (m_listening_thread)
    {
        // This will stop the update function on its next update
        pthread_mutex_unlock(&m_exit_mutex);
        pthread_join(*m_listening_thread, NULL); // wait for the thread to end
    }
}   // stopListening

// ---------------------------------------------------------------------------
/** \brief Returns true when the thread should stop listening.
 */
int STKHost::mustStopListening()
{
    switch (pthread_mutex_trylock(&m_exit_mutex)) {
    case 0: /* if we got the lock, unlock and return 1 (true) */
        pthread_mutex_unlock(&m_exit_mutex);
        return 1;
    case EBUSY: /* return 0 (false) if the mutex was locked */
        return 0;
    }
    return 1;
}   // mustStopListening

// ----------------------------------------------------------------------------
/** \brief Thread function checking if data is received.
 *  This function tries to get data from network low-level functions as
 *  often as possible. When something is received, it generates an
 *  event and passes it to the Network Manager.
 *  \param self : used to pass the ENet host to the function.
 */
void* STKHost::mainLoop(void* self)
{
    ENetEvent event;
    STKHost* myself = (STKHost*)(self);
    ENetHost* host = myself->m_network->getENetHost();
    while (!myself->mustStopListening())
    {
        while (enet_host_service(host, &event, 20) != 0)
        {
            if (event.type == ENET_EVENT_TYPE_NONE)
                continue;

            // Create an STKEvent with the event data
            Event* stk_event = new Event(&event);
            if (stk_event->getType() == EVENT_TYPE_MESSAGE)
                Network::logPacket(stk_event->data(), true);

            // The event is forwarded to the NetworkManger and from there
            // there to the ProtocolManager. The ProtocolManager is
            // responsible for freeing the memory.
            NetworkManager::getInstance()->propagateEvent(stk_event);
            
        }   // while enet_host_service
    }   // while !mustStopListening

    free(myself->m_listening_thread);
    myself->m_listening_thread = NULL;
    Log::info("STKHost", "Listening has been stopped");
    return NULL;
}   // mainLoop

// ----------------------------------------------------------------------------
/** \brief Setup this host as a server.
 *  \param address : The IPv4 address of incoming connections.
 *  \param port : The port on which the server listens.
 *  \param peer_count : The maximum number of peers.
 *  \param channel_limit : The maximum number of channels per peer.
 *  \param max_incoming_bandwidth : The maximum incoming bandwidth.
 *  \param max_outgoing_bandwidth : The maximum outgoing bandwidth.
 */
void STKHost::setupServer(uint32_t address, uint16_t port, int peer_count,
                            int channel_limit, uint32_t max_incoming_bandwidth,
                            uint32_t max_outgoing_bandwidth)
{
    ENetAddress* addr = (ENetAddress*)(malloc(sizeof(ENetAddress)));
    addr->host = address;
    addr->port = port;

    m_network= new Network(peer_count, channel_limit,
                           max_incoming_bandwidth,
                           max_outgoing_bandwidth, addr);
    if (!m_network)
    {
        Log::fatal("STKHost", "An error occurred while trying to create an ENet"
                          " server host.");
    }
}   // setupServer

// ----------------------------------------------------------------------------
/** \brief Setups the host as a client.
 *  In fact there is only one peer connected to this host.
 *  \param peer_count : The maximum number of peers.
 *  \param channel_limit : The maximum number of channels per peer.
 *  \param max_incoming_bandwidth : The maximum incoming bandwidth.
 *  \param max_outgoing_bandwidth : The maximum outgoing bandwidth.
 */

void STKHost::setupClient(int peer_count, int channel_limit,
                          uint32_t max_incoming_bandwidth,
                          uint32_t max_outgoing_bandwidth)
{
    m_network = new Network(peer_count, channel_limit,
                            max_incoming_bandwidth,
                            max_outgoing_bandwidth, NULL);
    if (!m_network)
    {
        Log::fatal ("STKHost", "An error occurred while trying to create "
                    "an ENet client host.");
    }
}   // setupClient


// ----------------------------------------------------------------------------
/** \brief Tells if a peer is known.
 *  \return True if the peer is known, false elseway.
 */
bool STKHost::peerExists(const TransportAddress& peer)
{
    ENetHost *host = m_network->getENetHost();
    for (unsigned int i = 0; i < host->peerCount ; i++)
    {
        if (host->peers[i].address.host == ntohl(peer.getIP()) &&
            host->peers[i].address.port == peer.getPort()        )
        {
            return true;
        }
    }
    return false;
}   // peerExists

// ----------------------------------------------------------------------------
/** \brief Tells if a peer is known and connected.
 *  \return True if the peer is known and connected, false elseway.
 */
bool STKHost::isConnectedTo(const TransportAddress& peer)
{
    ENetHost *host = m_network->getENetHost();
    for (unsigned int i = 0; i < host->peerCount; i++)
    {
        if (peer == host->peers[i].address &&
            host->peers[i].state == ENET_PEER_STATE_CONNECTED)
        {
            return true;
        }
    }
    return false;
}   // isConnectedTo

// ----------------------------------------------------------------------------
uint16_t STKHost::getPort() const
{
    struct sockaddr_in sin;
    socklen_t len = sizeof(sin);
    ENetHost *host = m_network->getENetHost();
    if (getsockname(host->socket, (struct sockaddr *)&sin, &len) == -1)
        Log::error("STKHost", "Error while using getsockname().");
    else
        return ntohs(sin.sin_port);
    return 0;
}
