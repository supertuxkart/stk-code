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
#include "network/network_config.hpp"
#include "network/network_console.hpp"
#include "network/network_string.hpp"
#include "network/protocols/connect_to_peer.hpp"
#include "network/protocols/connect_to_server.hpp"
#include "network/protocols/server_lobby.hpp"
#include "network/protocol_manager.hpp"
#include "network/servers_manager.hpp"
#include "network/stk_peer.hpp"
#include "utils/log.hpp"
#include "utils/time.hpp"
#include "utils/vs.hpp"

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

STKHost *STKHost::m_stk_host       = NULL;
bool     STKHost::m_enable_console = false;

void STKHost::create()
{
    assert(m_stk_host == NULL);
    if (NetworkConfig::get()->isServer())
        m_stk_host = new STKHost(NetworkConfig::get()->getServerName());
    else
    {
        Server *server = ServersManager::get()->getJoinedServer();
        m_stk_host = new STKHost(server->getServerId(), 0);
    }
    if (!m_stk_host->m_network)
    {
        delete m_stk_host;
        m_stk_host = NULL;
    }
}   // create

// ============================================================================
/** \class STKHost
 *  \brief Represents the local host. It is the main managing point for 
 *  networking. It is responsible for sending and receiving messages,
 *  and keeping track of onnected peers. It also provides some low
 *  level socket functions (i.e. to avoid that enet adds its headers
 *  to messages, useful for broadcast in LAN and for stun). It can be
 *  either instantiated as server, or as client. 
 *  Additionally this object stores information from the various protocols,
 *  which can be queried by the GUI. The online game works
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
 *
 *    1. ServerLobby:
 *       Spawns the following sub-protocols:
 *       1. GetPublicAddress: Use STUN to discover the public ip address
 *          and port number of this host.
 *       2. Register this server with stk server (i.e. publish its public
 *          ip address and port number) - 'start' request. This enters the
 *          public information into the 'client_sessions' table, and then
 *          the server into the 'servers' table. This server can now
 *          be detected by other clients, so they can request a connection.
 *       3. The server lobby now polls the stk server for client connection
 *          requests using the 'poll-connection-requests', which queries the
 *          servers table to get the server id (based on address and user id),
 *          and then the server_conn table. The rows in this table are updated
 *          by setting the 'request' bit to 0 (i.e. connection request was
 *          send to server).
 *      
 *  Client:
 *
 *    The GUI queries the stk server to get a list of available servers
 *    ('get-all' request, submitted from ServersManager to query the 'servers'
 *    table). The user picks one (or in case of quick play one is picked
 *    randomly), and then instantiates STKHost with the id of this server.
 *    STKHost then triggers ConnectToServer, which starts the following
 *    protocols:
 *       1. GetPublicAddress: Use STUN to discover the public ip address
 *          and port number of this host.
 *       2. Register the client with the STK host ('set' command, into the
 *          table 'client_sessions'). Its public ip address and port will
 *          be registerd.
 *       3. GetPeerAddress. Submits a 'get' request to the STK server to get
 *          the ip address and port for the selected server from
 *          'client_sessions'. 
 *          If the ip address of the server is the same as this client, they
 *          will connect using the LAN connection.
 *       4. RequestConnection will do a 'request-connection' to the stk server.
 *          The user id and server id are stored in server_conn. This is the
 *          request that the server will detect using polling.
 *
 * Server:
 *
 *   The ServerLobby (SLR) will then detect the above client
 *   requests, and start a ConnectToPeer protocol for each incoming client.
 *   The ConnectToPeer protocol uses:
 *         1. GetPeerAddress to get the ip address and port of the client.
 *            Once this is received, it will start the:
 *         2. PingProtocol
 *            This sends a raw packet (i.e. no enet header) to the
 *            destination (unless if it is a LAN connection, then UDP
 *            broadcasts will be used). 
 *
 *  Each client will run a ClientLobbyProtocol (CLR) to handle the further
 *  interaction with the server. The client will first request a connection
 *  with the server (this is for the 'logical' connection to the server; so
 *  far it was mostly about the 'physical' connection, i.e. being able to send
 *  a message to the server).
 *
 *  Each protocol has its own protocol id, which is added to each message in
 *  Protocol::sendMessage(). The ProtocolManager will automatically forward
 *  each received message to the protocol with the same id. So any message
 *  sent by protocol X on the server will be received by protocol X on the
 *  client and vice versa. The only exception are the client- and server-lobby:
 *  They share the same id (set in LobbyProtocol), so a message sent by
 *  the SLR will be received by the CLR, and a message from the CLR will be
 *  received by the SLR.
 *
 *  The server will reply with either a reject message (e.g. too many clients
 *  already connected), or an accept message. The accept message will contain
 *  the global player id of the client, and a unique (random) token used to
 *  authenticate all further messages from the server: each message from the
 *  client to the server and vice versa will contain this token. The message
 *  also contains the global ids and names of all currently connected
 *  clients for the new client. The server then informs all existing clients
 *  about the newly connected client, and its global player id.
 *
 *  --> At this stage all clients and the server know the name and global id
 *  of all connected clients. This information is stored in an array of
 *  NetworkPlayerProfile managed in GameSetup (which is stored in STKHost).
 *
 *  When the authorised clients starts the kart selection, the SLR
 *  informs all clients to start the kart selection (SLR::startSelection).
 *  This triggers the creation of the kart selection screen in 
 *  CLR::startSelection / CLR::update for all clients. The clients create
 *  the ActivePlayer object (which stores which device is used by which
 *  player).  The kart selection in a client calls
 *  (NetworkKartSelection::playerConfirm) which calls CLR::requestKartSelection.
 *  This sends a message to SLR::kartSelectionRequested, which verifies the
 *  selected kart and sends this information to all clients (including the
 *  client selecting the kart in the first place). This message is handled
 *  by CLR::kartSelectionUpdate. Server and all clients store this information
 *  in the NetworkPlayerProfile for the corresponding player, so server and
 *  all clients now have identical information about global player id, player
 *  name and selected kart. The authorised client will set some default votes
 *  for game modes, number of laps etc (temporary, see
 *  NetworkKartSelection::playerSelected).
 *
 *  After selecting a kart, the track selection screen is shown. On selecting
 *  a track, a vote for the track is sent to the client
 *  (TrackScreen::eventCallback, using CLR::voteTrack). The server will send
 *  all votes (track, #laps, ...) to all clients (see e.g. SLR::playerTrackVote
 *  etc), which are handled in e.g. CLR::playerTrackVote().
 *
 *  --> Server and all clients have identical information about all votes
 *  stored in RaceConfig of GameSetup.
 * 
 *  The server will detect when the track votes from each client have been
 *  received and will inform all clients to load the world (playerTrackVote).
 *  Then (state LOAD_GAME) the server will load the world and wait for all
 *  clients to finish loading (WAIT_FOR_WORLD_LOADED).
 *  
 *  In LR::loadWorld all ActivePlayers for all non-local players are created.
 *  (on a server all karts are non-local). On a client, the ActivePlayer
 *  objects for each local players have been created (to store the device
 *  used by each player when joining), so they are used to create the 
 *  LocalPlayerController for each kart. Each remote player gets a
 *  NULL ActivePlayer (the ActivePlayer is only used for assigning the input
 *  device to each kart, achievements and highscores, so it's not needed for
 *  remote players). It will also start the LatencyProtocol, 
 *  RaceEventManager and then load the world.

 * TODO:
 *  Once the server has received all
 *  messages in notifyEventAsynchronous(), it will call startCountdown()
 *  in the LatencyProtocol. The LatencyProtocol is 
 *  sending regular (once per second) pings to the clients and measure
 *  the averate latency. Upon starting the countdown this information
 *  is included in the ping request, so the clients can start the countdown
 *  at that stage as well.
 * 
 *  Once the countdown is 0 (or below), the Synchronization Protocol will
 *  start the protocols: KartUpdateProtocol, ControllerEventsProtocol,
 *  GameEventsProtocol. Then the LatencyProtocol is terminated
 *  which indicates to the main loop to start the actual game.
 */

// ============================================================================
/** Constructor for a client
 */
STKHost::STKHost(uint32_t server_id, uint32_t host_id)
{
    m_next_unique_host_id = -1;
    // Will be overwritten with the correct value once a connection with the
    // server is made.
    m_host_id = 0;
    init();
    TransportAddress a;
    a.setIP(0);
    a.setPort(NetworkConfig::get()->getClientPort());
    ENetAddress ea = a.toEnetAddress();

    m_network = new Network(/*peer_count*/1,       /*channel_limit*/2,
                            /*max_in_bandwidth*/0, /*max_out_bandwidth*/0, &ea);
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
 *  The server control flow starts with the ServerLobby.
 */
STKHost::STKHost(const irr::core::stringw &server_name)
{
    init();
    // The host id will be increased whenever a new peer is added, so the
    // first client will have host id 1 (host id 0 is the server).
    m_next_unique_host_id = 0;
    m_host_id = 0;   // indicates a server host.

    ENetAddress addr;
    addr.host = STKHost::HOST_ANY;
    addr.port = NetworkConfig::get()->getServerPort();

    m_network= new Network(NetworkConfig::get()->getMaxPlayers(),
                           /*channel_limit*/2,
                           /*max_in_bandwidth*/0,
                           /*max_out_bandwidth*/ 0, &addr);
    if (!m_network)
    {
        Log::fatal("STKHost", "An error occurred while trying to create an "
                              "ENet server host.");
    }

    startListening();
    Protocol *p = LobbyProtocol::create<ServerLobby>();
    ProtocolManager::getInstance()->requestStart(p);

}   // STKHost(server_name)

// ----------------------------------------------------------------------------
/** Initialises the internal data structures and starts the protocol manager
 *  and the debug console.
 */
void STKHost::init()
{
    m_shutdown         = false;
    m_network          = NULL;
    m_lan_network      = NULL;
    m_listening_thread = NULL;
    m_game_setup       = NULL;
    m_is_registered    = false;
    m_error_message    = "";

    pthread_mutex_init(&m_exit_mutex, NULL);

    // Start with initialising ENet
    // ============================
    if (enet_initialize() != 0)
    {
        Log::error("STKHost", "Could not initialize enet.");
        return;
    }

    Log::info("STKHost", "Host initialized.");
    Network::openLog();  // Open packet log file
    ProtocolManager::getInstance<ProtocolManager>();

    // Optional: start the network console
    m_network_console = NULL;
    if(m_enable_console)
    {
        m_network_console = new NetworkConsole();
        m_network_console->run();
    } 
}  // STKHost

// ----------------------------------------------------------------------------
/** Destructor. Stops the listening thread, closes the packet log file and
 *  destroys the enet host.
 */
STKHost::~STKHost()
{
    ProtocolManager::kill();
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

    delete m_network;
}   // ~STKHost

//-----------------------------------------------------------------------------
/** Requests that the network infrastructure is to be shut down. This function
 *  is called from a thread, but the actual shutdown needs to be done from 
 *  the main thread to avoid race conditions (e.g. ProtocolManager might still
 *  access data structures when the main thread tests if STKHost exist (which
 *  it does, but ProtocolManager might be shut down already.
 */
void STKHost::requestShutdown()
{
    m_shutdown = true;
}   // requestExit

//-----------------------------------------------------------------------------
/** Called from the main thread when the network infrastructure is to be shut
 *  down.
 */
void STKHost::shutdown()
{
    ServersManager::get()->unsetJoinedServer();
    ProtocolManager::getInstance()->abort();
    deleteAllPeers();
    destroy();
}   // shutdown

//-----------------------------------------------------------------------------
/** A previous GameSetup is deletea and a new one is created.
 *  \return Newly create GameSetup object.
 */
GameSetup* STKHost::setupNewGame()
{
    if (m_game_setup)
        delete m_game_setup;
    m_game_setup = new GameSetup();
    return m_game_setup;
}   // setupNewGame

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
    // Finish protocol manager first, to avoid that it access data
    // in STKHost.
    ProtocolManager::getInstance()->abort();
    stopListening();
}   // abort

// --------------------------------------------------------------------
/** Sets an error message for the gui.
 */
void STKHost::setErrorMessage(const irr::core::stringw &message)
{
    irr::core::stringc s(message.c_str());
    Log::error("STKHost", "%s", s.c_str());
    m_error_message = message;
}   // setErrorMessage

// --------------------------------------------------------------------
/** Returns the last error (or "" if no error has happened). */
const irr::core::stringw& STKHost::getErrorMessage() const
{
    return m_error_message; 
}   // getErrorMessage

// --------------------------------------------------------------------
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
/** Returns true if this client instance is allowed to control the server.
 *  A client can authorise itself by providing the server's password. It is
 *  then allowed to control the server (e.g. start kart selection).
 *  The information if this client was authorised by the server is actually
 *  stored in the peer (which is the server peer on a client).
 */
bool STKHost::isAuthorisedToControl() const 
{
    assert(NetworkConfig::get()->isClient());
    // If we are not properly connected (i.e. only enet connection, but not
    // stk logic), no peer is authorised.
    if(m_peers.size()==0)
        return false;
    return m_peers[0]->isAuthorised();
}   // isAuthorisedToControl

// ----------------------------------------------------------------------------
/** \brief Thread function checking if data is received.
 *  This function tries to get data from network low-level functions as
 *  often as possible. When something is received, it generates an
 *  event and passes it to the Network Manager.
 *  \param self : used to pass the ENet host to the function.
 */
void* STKHost::mainLoop(void* self)
{
    VS::setThreadName("STKHost");
    ENetEvent event;
    STKHost* myself = (STKHost*)(self);
    ENetHost* host = myself->m_network->getENetHost();

    if(NetworkConfig::get()->isServer() && 
        (NetworkConfig::get()->isLAN() || NetworkConfig::get()->isPublicServer()) )
    {
        TransportAddress address(0, NetworkConfig::get()->getServerDiscoveryPort());
        ENetAddress eaddr = address.toEnetAddress();
        myself->m_lan_network = new Network(1, 1, 0, 0, &eaddr);
    }

    while (!myself->mustStopListening())
    {
        if(myself->m_lan_network)
        {
            myself->handleDirectSocketRequest();
        }   // if discovery host

        while (enet_host_service(host, &event, 20) != 0)
        {
            if (event.type == ENET_EVENT_TYPE_NONE)
                continue;

            // Create an STKEvent with the event data. This will also
            // create the peer if it doesn't exist already
            Event* stk_event = new Event(&event);
            Log::verbose("STKHost", "Event of type %d received",
                         (int)(stk_event->getType()));
            STKPeer* peer = stk_event->getPeer();
            if (stk_event->getType() == EVENT_TYPE_CONNECTED)
            {
                Log::info("STKHost", "A client has just connected. There are "
                          "now %lu peers.", myself->m_peers.size());
                Log::debug("STKHost", "Addresses are : %lx, %lx",
                           stk_event->getPeer(), peer);
            }   // EVENT_TYPE_CONNECTED
            else if (stk_event->getType() == EVENT_TYPE_MESSAGE)
            {
                Network::logPacket(stk_event->data(), true);
                TransportAddress stk_addr(peer->getAddress());
                Log::verbose("NetworkManager",
                             "Message, Sender : %s, message:",
                             stk_addr.toString(/*show port*/false).c_str());
                Log::verbose("NetworkManager", "%s",
                             stk_event->data().getLogMessage().c_str());
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
/** Handles a direct request given to a socket. This is typically a LAN 
 *  request, but can also be used if the server is public (i.e. not behind
 *  a fire wall) to allow direct connection to the server (without using the
 *  STK server). It checks for any messages (i.e. a LAN broadcast requesting
 *  server details or a connection request) and if a valid LAN server-request
 *  message is received, will answer with a message containing server details
 *  (and sender IP address and port).
 */
void STKHost::handleDirectSocketRequest()
{
    const int LEN=2048;
    char buffer[LEN];

    TransportAddress sender;
    int len = m_lan_network->receiveRawPacket(buffer, LEN, &sender, 1);
    if(len<=0) return;
    BareNetworkString message(buffer, len);
    std::string command;
    message.decodeString(&command);

    if (command == "stk-server")
    {
        Log::verbose("STKHost", "Received LAN server query");
        std::string name = 
            StringUtils::wideToUtf8(NetworkConfig::get()->getServerName());
        // Avoid buffer overflows
        if (name.size() > 255)
            name = name.substr(0, 255);

        // Send the answer, consisting of server name, max players, 
        // current players, and the client's ip address and port
        // number (which solves the problem which network interface
        // might be the right one if there is more than one).
        BareNetworkString s((int)name.size()+1+11);
        s.encodeString(name);
        s.addUInt8(NetworkConfig::get()->getMaxPlayers());
        s.addUInt8(0);   // FIXME: current number of connected players
        s.addUInt32(sender.getIP());
        s.addUInt16(sender.getPort());
        s.addUInt16((uint16_t)race_manager->getMinorMode());
        s.addUInt8((uint8_t)race_manager->getDifficulty());
        m_lan_network->sendRawPacket(s, sender);
    }   // if message is server-requested
    else if (command == "connection-request")
    {
        // In case of a LAN connection, we only allow connections from
        // a LAN address (192.168*, ..., and 127.*).
        if (NetworkConfig::get()->isLAN() && !sender.isLAN())
        {
            Log::error("STKHost", "Client trying to connect from '%s'",
                       sender.toString().c_str());
            Log::error("STKHost", "which is outside of LAN - rejected.");
            return;
        }
        Protocol *c = new ConnectToPeer(sender);
        c->requestStart();
    }
    else
        Log::info("STKHost", "Received unknown command '%s'",
                  std::string(buffer, len).c_str());

}   // handleDirectSocketRequest

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
std::vector<NetworkPlayerProfile*> STKHost::getMyPlayerProfiles()
{
    return m_game_setup->getAllPlayersOnHost(m_host_id);
}   // getMyPlayerProfiles

// ----------------------------------------------------------------------------
/** Returns the STK peer belonging to the given enet_peer. If no STKPeer
 *  exists, create a new STKPeer.
 *  \param enet_peer The EnetPeer.
 */
STKPeer* STKHost::getPeer(ENetPeer *enet_peer)
{
    for(unsigned int i=0; i<m_peers.size(); i++)
    {
        if(m_peers[i]->isSamePeer(enet_peer))
            return m_peers[i];
    }

    // Make sure that a client only adds one other peer (=the server).
    if(NetworkConfig::get()->isClient() && m_peers.size()>0)
    {
        Log::error("STKHost",
                   "Client is adding more than one server, ignored for now.");
    }


    //FIXME Should we check #clients here? It might be easier to only
    // handle this at connect time, not in all getPeer calls.
    STKPeer *peer = new STKPeer(enet_peer);
    Log::debug("getPeer", 
               "Creating a new peer, address are STKPeer:%p, Peer:%p",
               peer, enet_peer);

    m_peers.push_back(peer);
    m_next_unique_host_id ++;
    return peer;
}   // getPeer
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
void STKHost::sendPacketExcept(STKPeer* peer, NetworkString *data,
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

