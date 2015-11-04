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
#include "network/protocols/connect_to_server.hpp"
#include "network/protocols/server_lobby_room_protocol.hpp"
#include "network/protocol_manager.hpp"
#include "network/network_console.hpp"
#include "network/stk_peer.hpp"
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

STKHost             *STKHost::m_stk_host     = NULL;
int                  STKHost::m_max_players  = 0;
bool                 STKHost::m_is_server    = false;
STKHost::NetworkType STKHost::m_network_type = STKHost::NETWORK_NONE;


/** \class STKHost. This is the main class for online games. It can be
 *  either instantiated as server, or as client. The online game works
 *  closely together with the stk server: a (game) server first connects
 *  to the stk server and registers itself, clients find the list of servers
 *  from the stk server. They insert a connections request into the stk
 *  server, which is regularly polled by the client. On detecting a new
 *  connection request the server will try to send a message to the client.
 *  This allows connections between server and client even if they are 
 *  sitting behind a NAT translating firewall. The following tables on
 *  the stk server are used:
 *  client_sessions: It stores the list of all online users (so loging in
 *         means to insert a row in this table), including their token
 *         used for authentication. In case of a client or server, their
 *         public ip address and port number and private port (for LAN)
 *         are added to the entry.
 *  servers: Registers all servers and gives them a unique id, together
 *         with the user id (which is stored as host_id in this table).
 *  server_conn: This table stores connection requests from clients to
 *         servers. A 'request' bit is set to 1 if the request has not
 *         been handled, and is reset to 0 the moment the server receives
 *         the information about the client request.
 *
 *  The following outlines the protocol happening in order to connect a
 *  client to a server in more details:
 *
 *  Server:
 *    1) ServerLobbyRoomProtocol:
 *       Spawns the following sub-protocols:
 *       a) GetPublicAddress: Use STUN to discover the public ip address
 *          and port number of this host.
 *       b) Register this server with stk server (i.e. publish its public
 *          ip address and port number) - 'start' request. This enters the
 *          public information into the 'client_sessions' table, and then
 *          the server into the 'servers' table. This server can now
 *          be detected by other clients, so they can request a connection.
 *       c) The server lobby now polls the stk server for client connection
 *          requests using the 'poll-connection-requests', which queries the
 *          servers table to get the server id (based on address and user id),
 *          and then the server_conn table. The rows in this table are updated
 *          by setting the 'request' bit to 0 (i.e. connection request was
 *          send to server).
 *      
 *  Client: The GUI queries the stk server to get a list of available servers
 *    ('get-all' request, submitted from ServersManager to query the 'servers'
 *    table). The user picks one (or in case of quick play one is picked
 *    randomly), and then instantiates STKHost with the id of this server.
 *    STKHost then triggers:
 *    1) ConnectToServer, which starts the following protocols:
 *       a) GetPublicAccress: Use STUN to discover the public ip address
 *          and port number of this host.
 *       b) Register the client with the STK host ('set' command, into the
 *          table 'client_sessions'). Its public ip address and port will
 *          be registerd.
 *       c) GetPeerAddress. Submits a 'get' request to the STK server to get
 *          the ip address and port for the selected server from
 *          'client_sessions'. 
 *          If the ip address of the server is the same as this client, they
 *          will connect using the LAN connection.
 *       d) RequestConnection will do a 'request-connection' to the stk server.
 *          The user id and server id are stored in server_conn. This is the
 *          request that the server will detect using polling.
 * Server:
 *   1) ServerLobbyRoomProtocol
 *      Will the detect the above client requests, and start a ConnectToPeer
 *      protocol for each incoming client:
 *      a) ConnectToPeer
 *         This protocol uses:
 *         A) GetPeerAddress to get the ip address and port of the client.
 *            Once this is received, it will start the:
 *         B) PingProtocol
 *            This sends a raw packet (i.e. no enet header) to the
 *            destination.
 *            (unless if it is a LAN connection, then UDP broadcasts will
 *            be used). 
 */

// ============================================================================
/** Constructor for a client
 */
STKHost::STKHost(uint32_t server_id, uint32_t host_id)
{
    m_is_server = false;
    init();

    m_network = new Network(/*peer_count*/1,       /*channel_limit*/2,
                            /*max_in_bandwidth*/0, /*max_out_bandwidth*/0);
    if (!m_network)
    {
        Log::fatal ("STKHost", "An error occurred while trying to create "
                               "an ENet client host.");
    }

    Protocol *connect = new ConnectToServer(server_id, host_id);
    connect->requestStart();
}   // STKHost

// ----------------------------------------------------------------------------
/** The constructor for a server.
 *  The server control flow starts with the ServerLobbyRoomProtocol.
 */
STKHost::STKHost(const irr::core::stringw &server_name)
{
    m_is_server = true;
    init();

    ENetAddress addr;
    addr.host = STKHost::HOST_ANY;
    addr.port = 2758;

    m_network= new Network(getMaxPlayers(),
                           /*channel_limit*/2,
                           /*max_in_bandwidth*/0,
                           /*max_out_bandwidth*/ 0, &addr);
    if (!m_network)
    {
        Log::fatal("STKHost", "An error occurred while trying to create an "
                              "ENet server host.");
    }

    startListening();
    ProtocolManager::getInstance()->requestStart(new ServerLobbyRoomProtocol());

}   // STKHost(server_name)

// ----------------------------------------------------------------------------
/** Initialises the internal data structures and starts the protocol manager
 *  and the debug console.
 */
void STKHost::init()
{
    m_network          = NULL;
    m_listening_thread = NULL;
    m_game_setup       = NULL;

    m_public_address.lock();
    m_public_address.getData().clear();
    m_public_address.unlock();

    pthread_mutex_init(&m_exit_mutex, NULL);

    // Start with initialising ENet
    // ============================
    if (enet_initialize() != 0)
    {
        Log::error("NetworkConsole", "Could not initialize enet.");
        return;
    }

    Log::info("NetworkConsole", "Host initialized.");
    Network::openLog();  // Open packet log file
    ProtocolManager::getInstance<ProtocolManager>();

    // Optional: start the network console
    m_network_console = new NetworkConsole();
    m_network_console->run();

}   // STKHost

// ----------------------------------------------------------------------------
/** Destructor. Stops the listening thread, closes the packet log file and
 *  destroys the enet host.
 */
STKHost::~STKHost()
{
    // delete the game setup
    if (m_game_setup)
        delete m_game_setup;
    m_game_setup = NULL;

    // Delete all connected peers
    while (!m_peers.empty())
    {
        delete m_peers.back();
        m_peers.pop_back();
    }

    Network::closeLog();
    stopListening();
    ProtocolManager::kill();

    delete m_network;
}   // ~STKHost

//-----------------------------------------------------------------------------
/** Stores the public address of this host.
 */
void STKHost::setPublicAddress(const TransportAddress& addr)
{
    m_public_address.lock();
    m_public_address.getData().copy(addr);
    m_public_address.unlock();
}   // setPublicAddress

//-----------------------------------------------------------------------------
/** A previous GameSetup is deletea and a new one is created.
 *  \return Newly create GameSetup object.
 */
GameSetup* STKHost::setupNewGame()
{
    if (m_game_setup)
        delete m_game_setup;
    m_game_setup = new GameSetup();
    m_game_setup->getRaceConfig()->setMaxPlayerCount(m_max_players);
    return m_game_setup;
}   // setupNewGame

//-----------------------------------------------------------------------------
/** \brief Function to reset the host - called in case that a client
 *  is disconnected from a server. 
 *  This function resets the peers and the listening host.
 */
void STKHost::reset()
{
    deleteAllPeers();
    destroy();
}   // reset

//-----------------------------------------------------------------------------
/** Called when you leave a server.
*/
void STKHost::deleteAllPeers()
{
    // remove all peers
    for (unsigned int i = 0; i < m_peers.size(); i++)
    {
        delete m_peers[i];
        m_peers[i] = NULL;
    }
    m_peers.clear();
}   // deleteAllPeers

// ----------------------------------------------------------------------------
/** Called when STK exits. It stops the listening thread and the
 *  ProtocolManager.
 */
void STKHost::abort()
{
    stopListening();
    // FIXME: Why a reset here? This creates a new stk_host, which will open
    // a new packet_log file (and therefore delete the previous file)???
    // reset();
    ProtocolManager::getInstance()->abort();
}   // abort

//-----------------------------------------------------------------------------
/** \brief Try to establish a connection to a given transport address.
 *  \param peer : The transport address which you want to connect to.
 *  \return True if we're successfully connected. False elseway.
 */
bool STKHost::connect(const TransportAddress& address)
{
    if (peerExists(address))
        return isConnectedTo(address);

    ENetPeer* peer = m_network->connectTo(address);

    if (peer == NULL)
    {
        Log::error("STKHost", "Could not try to connect to server.");
        return false;
    }
    TransportAddress a(peer->address);
    Log::verbose("STKPeer", "Connecting to %s", a.toString().c_str());
    return true;
}   // connect

// --------------------------------------------------------------------
/** Sends a message to the server if this is a client, or to all
 *  clients if this is the server.
 *  \param data Message to sent.
 *  \param reliable If the message is to be sent reliable.
 */
void STKHost::sendMessage(const NetworkString& data, bool reliable)
{
    if (m_is_server)
        broadcastPacket(data, reliable);
    else
    {
        if (m_peers.size() > 1)
            Log::warn("ClientNetworkManager", "Ambiguous send of data.");
        m_peers[0]->sendPacket(data, reliable);
    }
}   // sendMessage

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

            Log::verbose("STKHost", "Event of type %d received",
                         (int)(stk_event->getType()));
            STKPeer* peer = stk_event->getPeer();
            if (stk_event->getType() == EVENT_TYPE_CONNECTED)
            {
                // Add the new peer:
                myself->m_peers.push_back(peer);
                Log::info("STKHost", "A client has just connected. There are "
                          "now %lu peers.", myself->m_peers.size());
                Log::debug("STKHost", "Addresses are : %lx, %lx",
                           stk_event->getPeer(), peer);
            }   // EVENT_TYPE_CONNECTED
            else if (stk_event->getType() == EVENT_TYPE_MESSAGE)
            {
                TransportAddress stk_addr(peer->getAddress());
                Log::verbose("NetworkManager",
                             "Message, Sender : %s, message = \"%s\"",
                             stk_addr.toString(/*show port*/false).c_str(),
                             stk_event->data().std_string().c_str());

            }   // if message event

            // notify for the event now.
            ProtocolManager::getInstance()->propagateEvent(stk_event);
            
        }   // while enet_host_service
    }   // while !mustStopListening

    free(myself->m_listening_thread);
    myself->m_listening_thread = NULL;
    Log::info("STKHost", "Listening has been stopped");
    return NULL;
}   // mainLoop

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
void STKHost::removePeer(const STKPeer* peer)
{
    if (!peer || !peer->exists()) // peer does not exist (already removed)
        return;

    TransportAddress addr(peer->getAddress());
    Log::debug("STKHost", "Disconnected host: %s", addr.toString().c_str());
            
    // remove the peer:
    bool removed = false;
    for (unsigned int i = 0; i < m_peers.size(); i++)
    {
        if (m_peers[i]->isSamePeer(peer) && !removed) // remove only one
        {
            delete m_peers[i];
            m_peers.erase(m_peers.begin() + i, m_peers.begin() + i + 1);
            Log::verbose("NetworkManager",
                "The peer has been removed from the Network Manager.");
            removed = true;
        }
        else if (m_peers[i]->isSamePeer(peer))
        {
            Log::fatal("NetworkManager",
                       "Multiple peers match the disconnected one.");
        }
    }   // for i < m_peers.size()

    if (!removed)
        Log::warn("NetworkManager", "The peer that has been disconnected was "
                                    "not registered by the Network Manager.");

    Log::info("NetworkManager",
              "Somebody is now disconnected. There are now %lu peers.",
              m_peers.size());
}   // removePeer

//-----------------------------------------------------------------------------

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
}   // getPort

//-----------------------------------------------------------------------------
/** Sends data to all peers except the specified one.
 *  \param peer Peer which will not receive the message.
 *  \param data Data to sent.
 *  \param reliable If the data should be sent reliable or now.
 */
void STKHost::sendPacketExcept(STKPeer* peer, const NetworkString& data,
                               bool reliable)
{
    for (unsigned int i = 0; i < m_peers.size(); i++)
    {
        STKPeer* p = m_peers[i];
        if (!p->isSamePeer(peer))
        {
            p->sendPacket(data, reliable);
        }
    }
}   // sendPacketExcept

