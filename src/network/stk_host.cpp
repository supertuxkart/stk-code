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

#include "config/stk_config.hpp"
#include "config/user_config.hpp"
#include "io/file_manager.hpp"
#include "network/event.hpp"
#include "network/game_setup.hpp"
#include "network/network_config.hpp"
#include "network/network_console.hpp"
#include "network/network_string.hpp"
#include "network/network_timer_synchronizer.hpp"
#include "network/protocols/connect_to_peer.hpp"
#include "network/protocols/server_lobby.hpp"
#include "network/protocol_manager.hpp"
#include "network/server_config.hpp"
#include "network/stk_peer.hpp"
#include "utils/log.hpp"
#include "utils/separate_process.hpp"
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

#ifdef __MINGW32__
#  undef _WIN32_WINNT
#  define _WIN32_WINNT 0x501
#endif

#ifdef WIN32
#  include <winsock2.h>
#  include <ws2tcpip.h>
#else
#  include <netdb.h>
#endif
#include <sys/types.h>

#include <algorithm>
#include <functional>
#include <limits>
#include <random>
#include <string>
#include <utility>

STKHost *STKHost::m_stk_host       = NULL;
bool     STKHost::m_enable_console = false;

std::shared_ptr<LobbyProtocol> STKHost::create(SeparateProcess* p)
{
    assert(m_stk_host == NULL);
    std::shared_ptr<LobbyProtocol> lp;
    if (NetworkConfig::get()->isServer())
    {
        lp = LobbyProtocol::create<ServerLobby>();
        m_stk_host = new STKHost(true/*server*/);
    }
    else
    {
        m_stk_host = new STKHost(false/*server*/);
    }
    // Separate process for client-server gui if exists
    m_stk_host->m_separate_process = p;
    if (!m_stk_host->m_network)
    {
        delete m_stk_host;
        m_stk_host = NULL;
    }
    return lp;
}   // create

// ============================================================================
/** \class STKHost
 *  \brief Represents the local host. It is the main managing point for 
 *  networking. It is responsible for sending and receiving messages,
 *  and keeping track of connected peers. It also provides some low
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
 *   The ServerLobbyProtocol (SLP) will then detect the above client
 *   requests, and start a ConnectToPeer protocol for each incoming client.
 *   The ConnectToPeer protocol uses:
 *         1. GetPeerAddress to get the ip address and port of the client.
 *            Once this is received, it will start the:
 *         2. PingProtocol
 *            This sends a raw packet (i.e. no enet header) to the
 *            destination (unless if it is a LAN connection, then UDP
 *            broadcasts will be used). 
 *
 *  Each client will run a ClientLobbyProtocol (CLP) to handle the further
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
 *  the SLP will be received by the CLP, and a message from the CLP will be
 *  received by the SLP.
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
 *  When the authorised clients starts the kart selection, the SLP
 *  informs all clients to start the kart selection (SLP::startSelection).
 *  This triggers the creation of the kart selection screen in 
 *  CLP::startSelection / CLP::update for all clients. The clients create
 *  the ActivePlayer object (which stores which device is used by which
 *  player).  The kart selection in a client calls
 *  (NetworkKartSelection::playerConfirm) which calls CLP::requestKartSelection.
 *  This sends a message to SLP::kartSelectionRequested, which verifies the
 *  selected kart and sends this information to all clients (including the
 *  client selecting the kart in the first place). This message is handled
 *  by CLP::kartSelectionUpdate. Server and all clients store this information
 *  in the NetworkPlayerProfile for the corresponding player, so server and
 *  all clients now have identical information about global player id, player
 *  name and selected kart. The authorised client will set some default votes
 *  for game modes, number of laps etc (temporary, see
 *  NetworkKartSelection::playerSelected).
 *
 *  After selecting a kart, the track selection screen is shown. On selecting
 *  a track, a vote for the track is sent to the client
 *  (TrackScreen::eventCallback, using CLP::voteTrack). The server will send
 *  all votes (track, #laps, ...) to all clients (see e.g. SLP::playerTrackVote
 *  etc), which are handled in e.g. CLP::playerTrackVote().
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
 *  remote players). It will also start the RaceEventManager and then load the
 *  world.
 */
// ============================================================================
constexpr std::array<uint8_t, 5> g_ping_packet {{ 255, 'p', 'i', 'n', 'g' }};

// ============================================================================
constexpr bool isPingPacket(unsigned char* data, size_t length)
{
    return length > g_ping_packet.size() && data[0] == g_ping_packet[0] &&
        data[1] == g_ping_packet[1] && data[2] == g_ping_packet[2] &&
        data[3] == g_ping_packet[3] && data[4] == g_ping_packet[4];
}   // isPingPacket

// ============================================================================
/** The constructor for a server or client.
 */
STKHost::STKHost(bool server)
{
    init();
    m_host_id = std::numeric_limits<uint32_t>::max();

    ENetAddress addr;
    addr.host = STKHost::HOST_ANY;

    if (server)
    {
        addr.port = ServerConfig::m_server_port;
        if (addr.port == 0 && !UserConfigParams::m_random_server_port)
            addr.port = stk_config->m_server_port;
        // Reserve 1 peer to deliver full server message
        m_network = new Network(ServerConfig::m_server_max_players + 1,
            /*channel_limit*/EVENT_CHANNEL_COUNT, /*max_in_bandwidth*/0,
            /*max_out_bandwidth*/ 0, &addr, true/*change_port_if_bound*/);
    }
    else
    {
        addr.port = NetworkConfig::get()->getClientPort();
        // Client only has 1 peer
        m_network = new Network(/*peer_count*/1,
            /*channel_limit*/EVENT_CHANNEL_COUNT,
            /*max_in_bandwidth*/0, /*max_out_bandwidth*/0, &addr,
            true/*change_port_if_bound*/);
        m_nts.reset(new NetworkTimerSynchronizer());
    }

    if (!m_network)
    {
        Log::fatal("STKHost", "An error occurred while trying to create an "
                              "ENet server host.");
    }
    setPrivatePort();
    if (server)
        Log::info("STKHost", "Server port is %d", m_private_port);
}   // STKHost

// ----------------------------------------------------------------------------
/** Initialises the internal data structures and starts the protocol manager
 *  and the debug console.
 */
void STKHost::init()
{
    m_network_timer.store(StkTime::getRealTimeMs());
    m_shutdown         = false;
    m_authorised       = false;
    m_network          = NULL;
    m_exit_timeout.store(std::numeric_limits<uint64_t>::max());
    m_client_ping.store(0);

    // Start with initialising ENet
    // ============================
    if (enet_initialize() != 0)
    {
        Log::error("STKHost", "Could not initialize enet.");
        return;
    }

    Log::info("STKHost", "Host initialized.");
    Network::openLog();  // Open packet log file
    ProtocolManager::createInstance();

    // Optional: start the network console
    if (m_enable_console)
    {
        m_network_console = std::thread(std::bind(&NetworkConsole::mainLoop,
            this));
    }
}  // STKHost

// ----------------------------------------------------------------------------
/** Destructor. Stops the listening thread, closes the packet log file and
 *  destroys the enet host.
 */
STKHost::~STKHost()
{
    requestShutdown();
    if (m_network_console.joinable())
        m_network_console.join();

    disconnectAllPeers(true/*timeout_waiting*/);
    Network::closeLog();
    stopListening();

    delete m_network;
    enet_deinitialize();
    delete m_separate_process;
}   // ~STKHost

//-----------------------------------------------------------------------------
/** Called from the main thread when the network infrastructure is to be shut
 *  down.
 */
void STKHost::shutdown()
{
    ProtocolManager::lock()->abort();
    destroy();
}   // shutdown

//-----------------------------------------------------------------------------
/** Set the public address using stun protocol.
 */
void STKHost::setPublicAddress()
{
    std::vector<std::pair<std::string, uint32_t> > untried_server;
    for (auto& p : UserConfigParams::m_stun_servers)
        untried_server.push_back(p);

    assert(untried_server.size() > 2);
    // Randomly use stun servers of the low ping from top-half of the list
    std::sort(untried_server.begin(), untried_server.end(),
        [] (const std::pair<std::string, uint32_t>& a,
        const std::pair<std::string, uint32_t>& b)->bool
        {
            return a.second > b.second;
        });
    std::random_device rd;
    std::mt19937 g(rd());
    std::shuffle(untried_server.begin() + (untried_server.size() / 2),
        untried_server.end(), g);

    while (!untried_server.empty())
    {
        // Pick last element in untried servers
        std::string server_name = untried_server.back().first.c_str();
        UserConfigParams::m_stun_servers[server_name] = (uint32_t)-1;
        Log::debug("STKHost", "Using STUN server %s", server_name.c_str());

        std::vector<std::string> addr_and_port =
            StringUtils::split(server_name, ':');
        uint16_t port = 0;
        if (addr_and_port.size() != 2 ||
            !StringUtils::fromString(addr_and_port[1], port))
        {
            Log::error("STKHost", "Wrong server address and port");
            untried_server.pop_back();
            continue;
        }
        struct addrinfo hints, *res;

        memset(&hints, 0, sizeof hints);
        hints.ai_family = AF_UNSPEC; // AF_INET or AF_INET6 to force version
        hints.ai_socktype = SOCK_STREAM;

        // Resolve the stun server name so we can send it a STUN request
        int status = getaddrinfo(addr_and_port[0].c_str(), NULL, &hints, &res);
        if (status != 0)
        {
            Log::error("STKHost", "Error in getaddrinfo for stun server"
                " %s: %s", addr_and_port[0].c_str(), gai_strerror(status));
            untried_server.pop_back();
            continue;
        }
        untried_server.pop_back();
        // documentation says it points to "one or more addrinfo structures"
        assert(res != NULL);
        struct sockaddr_in* current_interface = (struct sockaddr_in*)(res->ai_addr);
        m_stun_address.setIP(ntohl(current_interface->sin_addr.s_addr));
        m_stun_address.setPort(port);

        // Assemble the message for the stun server
        BareNetworkString s(20);

        constexpr uint32_t magic_cookie = 0x2112A442;
        // bytes 0-1: the type of the message
        // bytes 2-3: message length added to header (attributes)
        uint16_t message_type = 0x0001; // binding request
        uint16_t message_length = 0x0000;
        s.addUInt16(message_type).addUInt16(message_length)
                                .addUInt32(magic_cookie);
        uint8_t stun_tansaction_id[12];
        // bytes 8-19: the transaction id
        for (int i = 0; i < 12; i++)
        {
            uint8_t random_byte = rand() % 256;
            s.addUInt8(random_byte);
            stun_tansaction_id[i] = random_byte;
        }

        m_network->sendRawPacket(s, m_stun_address);
        uint64_t ping = StkTime::getRealTimeMs();
        freeaddrinfo(res);

        // Recieve now
        TransportAddress sender;
        const int LEN = 2048;
        char buffer[LEN];
        int len = m_network->receiveRawPacket(buffer, LEN, &sender, 2000);
        ping = StkTime::getRealTimeMs() - ping;

        if (sender.getIP() != m_stun_address.getIP())
        {
            Log::warn("STKHost", 
                "Received stun response from %s instead of %s.",
                sender.toString().c_str(), m_stun_address.toString().c_str());
        }

        if (len <= 0)
        {
            Log::error("STKHost", "STUN response contains no data at all");
            continue;
        }

        // Convert to network string.
        BareNetworkString response(buffer, len);
        if (response.size() < 20)
        {
            Log::error("STKHost", "STUN response should be at least 20 bytes.");
            continue;
        }

        if (response.getUInt16() != 0x0101)
        {
            Log::error("STKHost", "STUN has no binding success response.");
            continue;
        }

        // Skip message size
        response.getUInt16();

        if (response.getUInt32() != magic_cookie)
        {
            Log::error("STKHost", "STUN response doesn't contain the magic "
                "cookie");
            continue;
        }

        for (int i = 0; i < 12; i++)
        {
            if (response.getUInt8() != stun_tansaction_id[i])
            {
                Log::error("STKHost", "STUN response doesn't contain the "
                    "transaction ID");
                continue;
            }
        }

        Log::debug("GetPublicAddress",
                "The STUN server responded with a valid answer");

        // The stun message is valid, so we parse it now:
        // Those are the port and the address to be detected
        TransportAddress non_xor_addr, xor_addr;
        while (true)
        {
            if (response.size() < 4)
            {
                break;
            }
            unsigned type = response.getUInt16();
            unsigned size = response.getUInt16();

            // Bit determining whether comprehension of an attribute is optional.
            // Described in section 15 of RFC 5389.
            constexpr uint16_t comprehension_optional = 0x1 << 15;

            // Bit determining whether the bit was assigned by IETF Review.
            // Described in section 18.1. of RFC 5389.
            constexpr uint16_t IETF_review = 0x1 << 14;

            // Defined in section 15.1 of RFC 5389
            constexpr uint8_t ipv4 = 0x01;

            // Defined in section 18.2 of RFC 5389
            constexpr uint16_t mapped_address = 0x001;
            constexpr uint16_t xor_mapped_address = 0x0020;
            // The first two bits are irrelevant to the type
            type &= ~(comprehension_optional | IETF_review);
            if (type == mapped_address || type == xor_mapped_address)
            {
                if (size != 8 || response.size() < 8)
                {
                    Log::error("STKHost", "Invalid STUN mapped address "
                        "length");
                    break;
                }
                // Ignore the first byte as mentioned in Section 15.1 of RFC
                // 5389.
                uint8_t ip_type = response.getUInt8();
                ip_type = response.getUInt8();
                if (ip_type != ipv4)
                {
                    Log::error("STKHost", "Only IPv4 is supported");
                    break;
                }

                uint16_t port = response.getUInt16();
                uint32_t ip = response.getUInt32();
                if (type == xor_mapped_address)
                {
                    // Obfuscation is described in Section 15.2 of RFC 5389.
                    port ^= magic_cookie >> 16;
                    ip ^= magic_cookie;
                    xor_addr.setPort(port);
                    xor_addr.setIP(ip);
                }
                else
                {
                    non_xor_addr.setPort(port);
                    non_xor_addr.setIP(ip);
                }
            }   // type == mapped_address || type == xor_mapped_address
            else
            {
                response.skip(size);
                int padding = size % 4;
                if (padding != 0)
                    response.skip(4 - padding);
            }
        }   // while true
        // Found public address and port
        if (!xor_addr.isUnset() || !non_xor_addr.isUnset())
        {
            // Use XOR mapped address when possible to avoid translation of
            // the packet content by application layer gateways (ALGs) that
            // perform deep packet inspection in an attempt to perform
            // alternate NAT traversal methods.
            if (!xor_addr.isUnset())
            {
                m_public_address = xor_addr;
            }
            else
            {
                Log::warn("STKHost", "Only non xor-mapped address returned.");
                m_public_address = non_xor_addr;
            }
            // Succeed, save ping
            UserConfigParams::m_stun_servers[server_name] = (uint32_t)(ping);
            untried_server.clear();
        }
    }
}   // setPublicAddress

//-----------------------------------------------------------------------------
void STKHost::setPrivatePort()
{
    struct sockaddr_in sin;
    socklen_t len = sizeof(sin);
    ENetHost *host = m_network->getENetHost();
    if (getsockname(host->socket, (struct sockaddr *)&sin, &len) == -1)
    {
        Log::error("STKHost", "Error while using getsockname().");
        m_private_port = 0;
    }
    else
        m_private_port = ntohs(sin.sin_port);
}   // setPrivatePort

//-----------------------------------------------------------------------------
/** Disconnect all connected peers.
*/
void STKHost::disconnectAllPeers(bool timeout_waiting)
{
    std::lock_guard<std::mutex> lock(m_peers_mutex);
    if (!m_peers.empty() && timeout_waiting)
    {
        for (auto peer : m_peers)
            peer.second->disconnect();
        // Wait for at most 2 seconds for disconnect event to be generated
        m_exit_timeout.store(StkTime::getRealTimeMs() + 2000);
    }
    m_peers.clear();
}   // disconnectAllPeers

//-----------------------------------------------------------------------------
/** Sets an error message for the gui.
 */
void STKHost::setErrorMessage(const irr::core::stringw &message)
{
    if (!message.empty())
    {
        irr::core::stringc s(message.c_str());
        Log::error("STKHost", "%s", s.c_str());
    }
    m_error_message = message;
}   // setErrorMessage

//-----------------------------------------------------------------------------
/** \brief Try to establish a connection to a given transport address.
 *  \param peer : The transport address which you want to connect to.
 *  \return True if we're successfully connected. False elseway.
 */
bool STKHost::connect(const TransportAddress& address)
{
    assert(NetworkConfig::get()->isClient());
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
    m_exit_timeout.store(std::numeric_limits<uint64_t>::max());
    m_listening_thread = std::thread(std::bind(&STKHost::mainLoop, this));
}   // startListening

// ----------------------------------------------------------------------------
/** \brief Stops the listening of events from ENet.
 *  Stops the thread that was receiving events.
 */
void STKHost::stopListening()
{
    if (m_exit_timeout.load() == std::numeric_limits<uint64_t>::max())
        m_exit_timeout.store(0);
    if (m_listening_thread.joinable())
        m_listening_thread.join();
}   // stopListening

// ----------------------------------------------------------------------------
/** \brief Thread function checking if data is received.
 *  This function tries to get data from network low-level functions as
 *  often as possible. When something is received, it generates an
 *  event and passes it to the Network Manager.
 *  \param self : used to pass the ENet host to the function.
 */
void STKHost::mainLoop()
{
    VS::setThreadName("STKHost");
    Log::info("STKHost", "Listening has been started.");
    ENetEvent event;
    ENetHost* host = m_network->getENetHost();
    const bool is_server = NetworkConfig::get()->isServer();

    // A separate network connection (socket) to handle LAN requests.
    Network* direct_socket = NULL;
    if ((NetworkConfig::get()->isLAN() && is_server) ||
        NetworkConfig::get()->isPublicServer())
    {
        TransportAddress address(0, stk_config->m_server_discovery_port);
        ENetAddress eaddr = address.toEnetAddress();
        direct_socket = new Network(1, 1, 0, 0, &eaddr);
        if (direct_socket->getENetHost() == NULL)
        {
            Log::warn("STKHost", "No direct socket available, this "
                "server may not be connected by lan network");
            delete direct_socket;
            direct_socket = NULL;
        }
    }

    uint64_t last_ping_time = StkTime::getRealTimeMs();
    uint64_t last_ping_time_update_for_client = StkTime::getRealTimeMs();
    std::map<std::string, uint64_t> ctp;
    while (m_exit_timeout.load() > StkTime::getRealTimeMs())
    {
        // Clear outdated connect to peer list every 15 seconds
        for (auto it = ctp.begin(); it != ctp.end();)
        {
            if (it->second + 15000 < StkTime::getRealTimeMs())
                it = ctp.erase(it);
            else
                it++;
        }

        auto sl = LobbyProtocol::get<ServerLobby>();
        if (direct_socket && sl && sl->waitingForPlayers())
        {
            try
            {
                handleDirectSocketRequest(direct_socket, sl, ctp);
            }
            catch (std::exception& e)
            {
                Log::warn("STKHost", "Direct socket error: %s",
                    e.what());
            }
        }   // if discovery host

        if (is_server)
        {
            std::unique_lock<std::mutex> peer_lock(m_peers_mutex);
            const float timeout = ServerConfig::m_validation_timeout;
            bool need_ping = false;
            if (sl && (!sl->isRacing() || sl->allowJoinedPlayersWaiting()) &&
                last_ping_time < StkTime::getRealTimeMs())
            {
                // If not racing, send an reliable packet at the same rate with
                // state exchange to keep enet ping accurate
                last_ping_time = StkTime::getRealTimeMs() +
                    (uint64_t)((1.0f /
                    (float)(stk_config->m_network_state_frequeny)) * 1000.0f);
                need_ping = true;
            }

            ENetPacket* packet = NULL;
            bool need_destroy_packet = true;
            if (need_ping)
            {
                m_peer_pings.getData().clear();
                for (auto& p : m_peers)
                {
                    m_peer_pings.getData()[p.second->getHostId()] =
                        p.second->getPing();
                    const unsigned ap = p.second->getAveragePing();
                    const unsigned max_ping = ServerConfig::m_max_ping;
                    if (ServerConfig::m_kick_high_ping_players &&
                        p.second->isValidated() &&
                        p.second->getConnectedTime() > 5.0f && ap > max_ping)
                    {
                        Log::info("STKHost", "%s with ping %d is higher than"
                            " %d ms, kick.",
                            p.second->getAddress().toString().c_str(),
                            ap, max_ping);
                        std::lock_guard<std::mutex> lock(m_enet_cmd_mutex);
                        m_enet_cmd.emplace_back(p.second->getENetPeer(),
                            (ENetPacket*)NULL, PDI_BAD_CONNECTION,
                            ECT_DISCONNECT);
                    }
                }
                BareNetworkString ping_packet;
                uint64_t network_timer = getNetworkTimer();
                ping_packet.addUInt64(network_timer);
                ping_packet.addUInt8((uint8_t)m_peer_pings.getData().size());
                for (auto& p : m_peer_pings.getData())
                    ping_packet.addUInt32(p.first).addUInt32(p.second);
                ping_packet.getBuffer().insert(
                    ping_packet.getBuffer().begin(), g_ping_packet.begin(),
                    g_ping_packet.end());
                packet = enet_packet_create(ping_packet.getData(),
                    ping_packet.getTotalSize(), ENET_PACKET_FLAG_RELIABLE);
            }

            for (auto it = m_peers.begin(); it != m_peers.end();)
            {
                if (need_ping &&
                    (!sl->allowJoinedPlayersWaiting() ||
                    !sl->isRacing() || it->second->isWaitingForGame()))
                {
                    need_destroy_packet = false;
                    enet_peer_send(it->first, EVENT_CHANNEL_UNENCRYPTED, packet);
                }

                // Remove peer which has not been validated after a specific time
                // It is validated when the first connection request has finished
                if (!it->second->isValidated() &&
                    it->second->getConnectedTime() > timeout)
                {
                    Log::info("STKHost", "%s has not been validated for more"
                        " than %f seconds, disconnect it by force.",
                        it->second->getAddress().toString().c_str(),
                        timeout);
                    enet_host_flush(host);
                    enet_peer_reset(it->first);
                    it = m_peers.erase(it);
                }
                else
                {
                    it++;
                }
            }
            peer_lock.unlock();
            if (need_destroy_packet && packet != NULL)
                enet_packet_destroy(packet);
        }

        std::list<std::tuple<ENetPeer*, ENetPacket*, uint32_t,
            ENetCommandType> > copied_list;
        std::unique_lock<std::mutex> lock(m_enet_cmd_mutex);
        std::swap(copied_list, m_enet_cmd);
        lock.unlock();
        for (auto& p : copied_list)
        {
            switch (std::get<3>(p))
            {
            case ECT_SEND_PACKET:
                enet_peer_send(std::get<0>(p), (uint8_t)std::get<2>(p),
                    std::get<1>(p));
                break;
            case ECT_DISCONNECT:
                enet_peer_disconnect(std::get<0>(p), std::get<2>(p));
                break;
            case ECT_RESET:
                // Flush enet before reset (so previous command is send)
                enet_host_flush(host);
                enet_peer_reset(std::get<0>(p));
                // Remove the stk peer of it
                std::lock_guard<std::mutex> lock(m_peers_mutex);
                m_peers.erase(std::get<0>(p));
                break;
            }
        }

        bool need_ping_update = false;
        while (enet_host_service(host, &event, 10) != 0)
        {
            if (!is_server &&
                last_ping_time_update_for_client < StkTime::getRealTimeMs())
            {
                last_ping_time_update_for_client =
                    StkTime::getRealTimeMs() + 2000;
                auto lp = LobbyProtocol::get<LobbyProtocol>();
                if (lp && lp->isRacing())
                {
                    auto p = getServerPeerForClient();
                    if (p)
                    {
                        m_client_ping.store(p->getPing(),
                            std::memory_order_relaxed);
                    }
                    need_ping_update = false;
                }
                else
                    need_ping_update = true;
            }
            if (event.type == ENET_EVENT_TYPE_NONE)
                continue;

            Event* stk_event = NULL;
            if (event.type == ENET_EVENT_TYPE_CONNECT)
            {
                auto stk_peer = std::make_shared<STKPeer>
                    (event.peer, this, m_next_unique_host_id++);
                std::unique_lock<std::mutex> lock(m_peers_mutex);
                m_peers[event.peer] = stk_peer;
                lock.unlock();
                stk_event = new Event(&event, stk_peer);
                TransportAddress addr(event.peer->address);
                Log::info("STKHost", "%s has just connected. There are "
                    "now %u peers.", addr.toString().c_str(), getPeerCount());
                // Client always trust the server
                if (!is_server)
                    stk_peer->setValidated();
            }   // ENET_EVENT_TYPE_CONNECT
            else if (event.type == ENET_EVENT_TYPE_DISCONNECT)
            {
                Log::flushBuffers();

                // If used a timeout waiting disconnect, exit now
                if (m_exit_timeout.load() !=
                    std::numeric_limits<uint64_t>::max())
                {
                    m_exit_timeout.store(0);
                    break;
                }
                // Use the previous stk peer so protocol can see the network
                // profile and handle it for disconnection
                if (m_peers.find(event.peer) != m_peers.end())
                {
                    stk_event = new Event(&event, m_peers.at(event.peer));
                    std::lock_guard<std::mutex> lock(m_peers_mutex);
                    m_peers.erase(event.peer);
                }
                TransportAddress addr(event.peer->address);
                Log::info("STKHost", "%s has just disconnected. There are "
                    "now %u peers.", addr.toString().c_str(), getPeerCount());
            }   // ENET_EVENT_TYPE_DISCONNECT

            if (!stk_event && m_peers.find(event.peer) != m_peers.end())
            {
                auto& peer = m_peers.at(event.peer);
                if (isPingPacket(event.packet->data, event.packet->dataLength))
                {
                    if (!is_server)
                    {
                        BareNetworkString ping_packet((char*)event.packet->data,
                            (int)event.packet->dataLength);
                        std::map<uint32_t, uint32_t> peer_pings;
                        ping_packet.skip((int)g_ping_packet.size());
                        uint64_t server_time = ping_packet.getUInt64();
                        unsigned peer_size = ping_packet.getUInt8();
                        for (unsigned i = 0; i < peer_size; i++)
                        {
                            unsigned host_id = ping_packet.getUInt32();
                            unsigned ping = ping_packet.getUInt32();
                            peer_pings[host_id] = ping;
                        }
                        const uint32_t client_ping =
                            peer_pings.find(m_host_id) != peer_pings.end() ?
                            peer_pings.at(m_host_id) : 0;

                        if (client_ping > 0)
                        {
                            assert(m_nts);
                            m_nts->addAndSetTime(client_ping, server_time);
                        }
                        if (need_ping_update)
                        {
                            m_peer_pings.lock();
                            std::swap(m_peer_pings.getData(), peer_pings);
                            m_peer_pings.unlock();
                            m_client_ping.store(client_ping,
                                std::memory_order_relaxed);
                        }
                    }
                    enet_packet_destroy(event.packet);
                    continue;
                }
                try
                {
                    stk_event = new Event(&event, peer);
                }
                catch (std::exception& e)
                {
                    Log::warn("STKHost", "%s", e.what());
                    enet_packet_destroy(event.packet);
                    continue;
                }
            }
            else if (!stk_event)
            {
                enet_packet_destroy(event.packet);
                continue;
            }
            if (stk_event->getType() == EVENT_TYPE_MESSAGE)
            {
                Network::logPacket(stk_event->data(), true);
#ifdef DEBUG_MESSAGE_CONTENT
                Log::verbose("NetworkManager",
                             "Message, Sender : %s time %f message:",
                             stk_event->getPeer()->getAddress()
                             .toString(/*show port*/false).c_str(),
                             StkTime::getRealTime());
                Log::verbose("NetworkManager", "%s",
                             stk_event->data().getLogMessage().c_str());
#endif
            }   // if message event

            // notify for the event now.
            auto pm = ProtocolManager::lock();
            if (pm && !pm->isExiting())
                pm->propagateEvent(stk_event);
            else
                delete stk_event;
        }   // while enet_host_service
    }   // while m_exit_timeout.load() > StkTime::getRealTimeMs()
    delete direct_socket;
    Log::info("STKHost", "Listening has been stopped.");
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
void STKHost::handleDirectSocketRequest(Network* direct_socket,
                                        std::shared_ptr<ServerLobby> sl,
                                        std::map<std::string, uint64_t>& ctp)
{
    const int LEN=2048;
    char buffer[LEN];

    TransportAddress sender;
    int len = direct_socket->receiveRawPacket(buffer, LEN, &sender, 1);
    if(len<=0) return;
    BareNetworkString message(buffer, len);
    std::string command;
    message.decodeString(&command);
    const std::string connection_cmd = std::string("connection-request") +
        StringUtils::toString(m_private_port);

    if (command == "stk-server")
    {
        Log::verbose("STKHost", "Received LAN server query");
        const std::string& name = sl->getGameSetup()->getServerNameUtf8();
        // Send the answer, consisting of server name, max players, 
        // current players
        const std::string& pw = ServerConfig::m_private_server_password;
        BareNetworkString s((int)name.size()+1+11);
        s.addUInt32(ServerConfig::m_server_version);
        s.encodeString(name);
        s.addUInt8((uint8_t)ServerConfig::m_server_max_players);
        s.addUInt8((uint8_t)(sl->getGameSetup()->getPlayerCount() +
            sl->getWaitingPlayersCount()));
        s.addUInt16(m_private_port);
        s.addUInt8((uint8_t)ServerConfig::m_server_difficulty);
        s.addUInt8((uint8_t)ServerConfig::m_server_mode);
        s.addUInt8(!pw.empty());
        s.addUInt8((uint8_t)
            (sl->getCurrentState() == ServerLobby::WAITING_FOR_START_GAME ?
            0 : 1));
        direct_socket->sendRawPacket(s, sender);
    }   // if message is server-requested
    else if (command == connection_cmd)
    {
        const std::string& peer_addr = sender.toString();
        // In case of a LAN connection, we only allow connections from
        // a LAN address (192.168*, ..., and 127.*).
        if (!sender.isLAN() && !sender.isPublicAddressLocalhost() &&
            !NetworkConfig::get()->isPublicServer())
        {
            Log::error("STKHost", "Client trying to connect from '%s'",
                peer_addr.c_str());
            Log::error("STKHost", "which is outside of LAN - rejected.");
            return;
        }
        if (ctp.find(peer_addr) == ctp.end())
        {
            ctp[peer_addr] = StkTime::getRealTimeMs();
            std::make_shared<ConnectToPeer>(sender)->requestStart();
        }
    }
    else if (command == "stk-server-port")
    {
        BareNetworkString s;
        s.addUInt16(m_private_port);
        direct_socket->sendRawPacket(s, sender);
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
    std::lock_guard<std::mutex> lock(m_peers_mutex);
    for (auto p : m_peers)
    {
        auto stk_peer = p.second;
        if (stk_peer->getAddress() == peer ||
            ((stk_peer->getAddress().isPublicAddressLocalhost() ||
            peer.isPublicAddressLocalhost()) &&
            stk_peer->getAddress().getPort() == peer.getPort()))
            return true;
    }
    return false;
}   // peerExists

// ----------------------------------------------------------------------------
/** \brief Return the only server peer for client.
 *  \return STKPeer the STKPeer of server.
 */
std::shared_ptr<STKPeer> STKHost::getServerPeerForClient() const
{
    assert(NetworkConfig::get()->isClient());
    if (m_peers.size() != 1)
        return nullptr;
    return m_peers.begin()->second;
}   // getServerPeerForClient

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

//-----------------------------------------------------------------------------
/** Sends data to all validated peers currently in server
 *  \param data Data to sent.
 *  \param reliable If the data should be sent reliable or now.
 */
void STKHost::sendPacketToAllPeersInServer(NetworkString *data, bool reliable)
{
    std::lock_guard<std::mutex> lock(m_peers_mutex);
    for (auto p : m_peers)
    {
        if (p.second->isValidated())
            p.second->sendPacket(data, reliable);
    }
}   // sendPacketToAllPeersInServer

//-----------------------------------------------------------------------------
/** Sends data to all validated peers currently in game
 *  \param data Data to sent.
 *  \param reliable If the data should be sent reliable or now.
 */
void STKHost::sendPacketToAllPeers(NetworkString *data, bool reliable)
{
    std::lock_guard<std::mutex> lock(m_peers_mutex);
    for (auto p : m_peers)
    {
        if (p.second->isValidated() && !p.second->isWaitingForGame())
            p.second->sendPacket(data, reliable);
    }
}   // sendPacketToAllPeers

//-----------------------------------------------------------------------------
/** Sends data to all validated peers except the specified currently in game
 *  \param peer Peer which will not receive the message.
 *  \param data Data to sent.
 *  \param reliable If the data should be sent reliable or now.
 */
void STKHost::sendPacketExcept(STKPeer* peer, NetworkString *data,
                               bool reliable)
{
    std::lock_guard<std::mutex> lock(m_peers_mutex);
    for (auto p : m_peers)
    {
        STKPeer* stk_peer = p.second.get();
        if (!stk_peer->isSamePeer(peer) && p.second->isValidated() &&
            !p.second->isWaitingForGame())
        {
            stk_peer->sendPacket(data, reliable);
        }
    }
}   // sendPacketExcept

//-----------------------------------------------------------------------------
/** Sends data to peers with custom rule
 *  \param predicate boolean function for peer to predicate whether to send
 *  \param data Data to sent.
 *  \param reliable If the data should be sent reliable or now.
 */
void STKHost::sendPacketToAllPeersWith(std::function<bool(STKPeer*)> predicate,
                                       NetworkString* data, bool reliable)
{
    std::lock_guard<std::mutex> lock(m_peers_mutex);
    for (auto p : m_peers)
    {
        STKPeer* stk_peer = p.second.get();
        if (predicate(stk_peer))
            stk_peer->sendPacket(data, reliable);
    }
}   // sendPacketToAllPeersWith

//-----------------------------------------------------------------------------
/** Sends a message from a client to the server. */
void STKHost::sendToServer(NetworkString *data, bool reliable)
{
    std::lock_guard<std::mutex> lock(m_peers_mutex);
    if (m_peers.empty())
        return;
    assert(NetworkConfig::get()->isClient());
    m_peers.begin()->second->sendPacket(data, reliable);
}   // sendToServer

//-----------------------------------------------------------------------------
std::vector<std::shared_ptr<NetworkPlayerProfile> >
    STKHost::getAllPlayerProfiles() const
{
    std::vector<std::shared_ptr<NetworkPlayerProfile> > p;
    std::unique_lock<std::mutex> lock(m_peers_mutex);
    for (auto peer : m_peers)
    {
        if (peer.second->isDisconnected())
            continue;
        auto peer_profile = peer.second->getPlayerProfiles();
        p.insert(p.end(), peer_profile.begin(), peer_profile.end());
    }
    lock.unlock();
    return p;
}   // getAllPlayerProfiles

//-----------------------------------------------------------------------------
std::shared_ptr<STKPeer> STKHost::findPeerByHostId(uint32_t id) const
{
    std::lock_guard<std::mutex> lock(m_peers_mutex);
    auto ret = std::find_if(m_peers.begin(), m_peers.end(),
        [id](const std::pair<ENetPeer*, std::shared_ptr<STKPeer> >& p)
        {
            return p.second->getHostId() == id;
        });
    return ret != m_peers.end() ? ret->second : nullptr;
}   // findPeerByHostId

//-----------------------------------------------------------------------------
void STKHost::initClientNetwork(ENetEvent& event, Network* new_network)
{
    assert(NetworkConfig::get()->isClient());
    assert(!m_listening_thread.joinable());
    assert(new_network->getENetHost()->peerCount == 1);
    if (m_network != new_network)
    {
        delete m_network;
        m_network = new_network;
    }
    auto stk_peer = std::make_shared<STKPeer>(event.peer, this,
        m_next_unique_host_id++);
    stk_peer->setValidated();
    m_peers[event.peer] = stk_peer;
    setPrivatePort();
    startListening();
    auto pm = ProtocolManager::lock();
    if (pm && !pm->isExiting())
        pm->propagateEvent(new Event(&event, stk_peer));
}   // replaceNetwork
