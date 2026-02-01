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
#include "network/network.hpp"
#include "network/network_config.hpp"
#include "network/network_console.hpp"
#include "network/network_player_profile.hpp"
#include "network/network_string.hpp"
#include "network/network_timer_synchronizer.hpp"
#include "network/protocols/connect_to_peer.hpp"
#include "network/protocols/server_lobby.hpp"
#include "network/protocol_manager.hpp"
#include "network/server_config.hpp"
#include "network/child_loop.hpp"
#include "network/stk_ipv6.hpp"
#include "network/stk_peer.hpp"
#include "utils/log.hpp"
#include "utils/string_utils.hpp"
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

STKHost *STKHost::m_stk_host[PT_COUNT];
bool     STKHost::m_enable_console = false;

std::shared_ptr<LobbyProtocol> STKHost::create(ChildLoop* cl)
{
    ProcessType pt = STKProcess::getType();
    assert(m_stk_host[pt] == NULL);
    std::shared_ptr<LobbyProtocol> lp;
    if (NetworkConfig::get()->isServer())
    {
        std::shared_ptr<ServerLobby> sl =
            LobbyProtocol::create<ServerLobby>();
        m_stk_host[pt] = new STKHost(true/*server*/);
        sl->initServerStatsTable();
        lp = sl;
    }
    else
    {
        m_stk_host[pt] = new STKHost(false/*server*/);
    }
    // Separate process for client-server gui if exists
    m_stk_host[pt]->m_client_loop = cl;
    if (cl)
    {
        m_stk_host[pt]->m_client_loop_thread = std::thread(
            std::bind(&ChildLoop::run, cl));
    }
    if (!m_stk_host[pt]->m_network)
    {
        delete m_stk_host[pt];
        m_stk_host[pt] = NULL;
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
 *  sitting behind a NAT firewall. The following tables on
 *  the stk server are used:
 *  client_sessions: It stores the list of all online users (so loging in
 *         means to insert a row in this table), including their token
 *         used for authentication. In case of a client or server, their
 *         public ip address and port number and private port (for LAN)
 *         are added to the entry.
 *  servers: Registers all servers and gives them a unique id, together
 *         with the user id (which is stored as host_id in this table).
 *  server_conn: This table stores connection requests from clients to
 *         servers. It has the aes_key and aes_iv which are used to validate
 *         clients if it's the same online user id joining a server,
 *         ip and port for NAT penetration and a connected_since which
 *         stores the timestamp the user joined this server to know the time
 *         playing spent playing on it.
 *
 *  The following outlines the protocol happening in order to connect a
 *  client to a server in more details:
 *
 *  First it calls setPublicAddress() here to discover the public ip address
 *  and port number of this host,
 *  Server:
 *
 *    1. ServerLobby:
 *       1. Register this server with stk server (i.e. publish its public
 *          ip address, port number and game info) - 'create' request. This
 *          enters the server into the 'servers' table. This server can now
 *          be detected by other clients, so they can request a connection.
 *       2. The server lobby now polls the stk server for client connection
 *          requests using the 'poll-connection-requests' each 5 seconds, which
 *          queries the servers table to get the server id (based on address
 *          and user id), and then the server_conn table. The server will map
 *          each joined AES key within 45 seconds to each connected client to
 *          handle validation.
 *  Client:
 *
 *    The GUI queries the stk server to get a list of available servers
 *    ('get-all' request, submitted from ServersManager to query the 'servers'
 *    table). The user picks one (or in case of quick play one is picked
 *    randomly), and then instantiates STKHost with the id of this server.
 *    STKHost then triggers ConnectToServer, which do the following:
 *       1. Register the client with the STK host ('join-server-key' command,
 *          into the table 'server_conn'). Its public ip address and port will
 *          be registerd with a AES key and iv set by client.
 *       2. Run ConnectToServer::tryConnect for 30 seconds to connect to server,
 *          It will auto change to a correct server port in case the server is
 *          behind a strong firewall, see intercept below.
 *
 * Server:
 *
 *  The ServerLobbyProtocol (SLP) will then detect the above client
 *  requests, and start a ConnectToPeer protocol for each incoming client.
 *  The ConnectToPeer protocol send client a raw packet with
 *  "0xffff" and "aloha-stk", so the client can use the intercept in enet for
 *  advanced NAT penetration, see ConnectToServer::interceptCallback for
 *  details.
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
 *  the global player id of the client. Each time any client connect,
 *  disconnect or change team / handicap server will send a new list of
 *  currently available players and update them in the networking lobby.
 *
 *  When the authorised client or ownerless server timed up and start the kart
 *  selection, the SLP informs all clients to start the kart selection
 *  (SLP::startSelection). This triggers the creation of the kart selection
 *  (if grand prix in progress then goes track screen directly) screen in
 *  CLP::startSelection / CLP::update for all clients. The clients create
 *  the ActivePlayer object (which stores which device is used by which
 *  player).
 *
 *  After selecting a kart, the track selection screen is shown. On selecting
 *  a track, a vote for the track, laps and reversed is sent to the client
 *  (TrackScreen::eventCallback). The server will send
 *  all votes (track, #laps, ...) to all clients (see e.g. SLP::handlePlayerVote
 *  etc), which are handled in e.g. CLP::receivePlayerVote().
 *
 *  --> Server and all clients have identical information about all votes
 *  stored in m_peers_votes in LobbyProtocol base class.
 *
 *  The server will decide the best track vote based on the discussion from
 *  clients, then it will inform all clients to load the world (addAllPlayers)
 *  with the final players currently connected with team / handicap settings.
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
 *
 *  Below you can see the definition of ping packet, it's a special packet that
 *  will be sent to each client waiting in lobby, the 1st byte is 255 so
 *  ProtocolManager won't handle it, after 5 bytes it comes with real data:
 *  1. Server time in uint64_t (for synchronization)
 *  2. Host id with ping to each client currently connected
 *  3. If game is currently started, 2 uint32_t which tell remaining time or
 *     progress in percent
 *  4. If game is currently started, the track internal identity
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
    m_public_address.reset(new SocketAddress());
    init();
    m_host_id = std::numeric_limits<uint32_t>::max();

    ENetAddress addr = {};
    if (server)
    {
        setIPv6Socket(ServerConfig::m_ipv6_connection ? 1 : 0);
#ifdef ENABLE_IPV6
        if (NetworkConfig::get()->getIPType() == NetworkConfig::IP_V4 &&
            ServerConfig::m_ipv6_connection)
        {
            Log::warn("STKHost", "Disable IPv6 socket due to missing IPv6.");
            setIPv6Socket(0);
        }
#endif
        addr.port = ServerConfig::m_server_port;
        if (addr.port == 0 && !UserConfigParams::m_random_server_port)
            addr.port = stk_config->m_server_port;
        // Reserve 1 peer to deliver full server message
        int peer_count = ServerConfig::m_server_max_players + 1;
        // 1 more peer to hold ai peer
        if (ServerConfig::m_ai_handling)
            peer_count++;
        m_network = new Network(peer_count,
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
    if (server)
        Log::info("STKHost", "Server port is %d", getPrivatePort());
}   // STKHost

// ----------------------------------------------------------------------------
/** Initialises the internal data structures and starts the protocol manager
 *  and the debug console.
 */
void STKHost::init()
{
    m_players_in_game.store(0);
    m_players_waiting.store(0);
    m_total_players.store(0);
    m_network_timer.store((int64_t)StkTime::getMonoTimeMs());
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
    // Abort the server loop earlier so it can be stopped in background as
    // soon as possible
    if (m_client_loop)
        m_client_loop->abort();

    NetworkConfig::get()->clearActivePlayersForClient();
    requestShutdown();
    if (m_network_console.joinable())
        m_network_console.join();

    disconnectAllPeers(true/*timeout_waiting*/);
    Network::closeLog();
    stopListening();

    // Drop all unsent packets
    for (auto& p : m_enet_cmd)
    {
        if (std::get<3>(p) == ECT_SEND_PACKET)
        {
            ENetPacket* packet = std::get<1>(p);
            enet_packet_destroy(packet);
        }
    }
    delete m_network;
    enet_deinitialize();
    if (m_client_loop)
    {
        m_client_loop_thread.join();
        delete m_client_loop;
    }
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
/** Get the stun network string required for binding request
 *  \param stun_tansaction_id 16 bytes array for filling to validate later.
 */
BareNetworkString STKHost::getStunRequest(uint8_t* stun_tansaction_id)
{
    // Assemble the message for the stun server
    BareNetworkString s(20);

    constexpr uint32_t magic_cookie = 0x2112A442;
    // bytes 0-1: the type of the message
    // bytes 2-3: message length added to header (attributes)
    uint16_t message_type = 0x0001; // binding request
    uint16_t message_length = 0x0000;
    s.addUInt16(message_type).addUInt16(message_length)
        .addUInt32(magic_cookie);

    stun_tansaction_id[0] = 0x21;
    stun_tansaction_id[1] = 0x12;
    stun_tansaction_id[2] = 0xA4;
    stun_tansaction_id[3] = 0x42;
    // bytes 8-19: the transaction id
    for (int i = 0; i < 12; i++)
    {
        uint8_t random_byte = rand() % 256;
        s.addUInt8(random_byte);
        stun_tansaction_id[i + 4] = random_byte;
    }
    return s;
}   // getStunRequest

//-----------------------------------------------------------------------------
void STKHost::getIPFromStun(int socket, const std::string& stun_address,
                            short family, SocketAddress* result)
{
    // The IPv4 stun address has no AAAA record, so give it an AF_INET6 will
    // return one using NAT64 and DNS64
    if (isIPv6Socket() && family == AF_INET &&
        NetworkConfig::get()->getIPType() == NetworkConfig::IP_V6_NAT64)
        family = AF_INET6;

    SocketAddress stun(stun_address, 0/*port is specified in address*/,
        family);
    // We specify ai_family, so only IPv4 or IPv6 is found
    if (stun.isUnset())
        return;
    stun.convertForIPv6Socket(isIPv6Socket());

    // We only need to keep the stun address for server to keep the port open
    if (NetworkConfig::get()->isServer())
    {
        if (stun.isIPv6())
            m_stun_ipv6.reset(new SocketAddress(stun));
        else
            m_stun_ipv4.reset(new SocketAddress(stun));
    }

    uint8_t stun_tansaction_id[16];
    constexpr uint32_t magic_cookie = 0x2112A442;
    BareNetworkString s = getStunRequest(stun_tansaction_id);

    sendto(socket, s.getData(), s.size(), 0, stun.getSockaddr(),
        stun.getSocklen());

    // Recieve now
    const int LEN = 2048;
    char buffer[LEN];

    struct sockaddr_in addr4_rev;
    struct sockaddr_in6 addr6_rev;
    struct sockaddr* addr_rev = isIPv6Socket() ?
        (struct sockaddr*)(&addr6_rev) : (struct sockaddr*)(&addr4_rev);
    socklen_t from_len = isIPv6Socket() ? sizeof(addr6_rev) : sizeof(addr4_rev);
    int len = -1;
    int count = 0;
    while (len < 0 && count < 2000)
    {
        len = recvfrom(socket, buffer, LEN, 0, addr_rev,
            &from_len);
        if (len > 0)
            break;
        count++;
        StkTime::sleep(1);
    }

    if (len <= 0)
    {
        Log::error("STKHost", "STUN response contains no data at all");
        return;
    }

    // Convert to network string.
    BareNetworkString response(buffer, len);
    if (response.size() < 20)
    {
        Log::error("STKHost", "STUN response should be at least 20 bytes.");
        return;
    }

    if (response.getUInt16() != 0x0101)
    {
        Log::error("STKHost", "STUN has no binding success response.");
        return;
    }

    // Skip message size
    response.getUInt16();

    if (response.getUInt32() != magic_cookie)
    {
        Log::error("STKHost", "STUN response doesn't contain the magic "
            "cookie");
        return;
    }

    for (int i = 0; i < 12; i++)
    {
        if (response.getUInt8() != stun_tansaction_id[i + 4])
        {
            Log::error("STKHost", "STUN response doesn't contain the "
                "transaction ID");
            return;
        }
    }

    // The stun message is valid, so we parse it now:
    // Those are the port and the address to be detected
    SocketAddress non_xor_addr, xor_addr;
    while (true)
    {
        if (response.size() < 4)
            break;

        unsigned type = response.getUInt16();
        unsigned size = response.getUInt16();

        // Bit determining whether comprehension of an attribute is optional.
        // Described in section 15 of RFC 5389.
        constexpr uint16_t comprehension_optional = 0x1 << 15;

        // Bit determining whether the bit was assigned by IETF Review.
        // Described in section 18.1. of RFC 5389.
        constexpr uint16_t IETF_review = 0x1 << 14;

        // Defined in section 15.1 of RFC 5389
        constexpr uint8_t ipv4_returned = 0x01;
        constexpr uint8_t ipv6_returned = 0x02;

        // Defined in section 18.2 of RFC 5389
        constexpr uint16_t mapped_address = 0x001;
        constexpr uint16_t xor_mapped_address = 0x0020;
        // The first two bits are irrelevant to the type
        type &= ~(comprehension_optional | IETF_review);
        if (type == mapped_address || type == xor_mapped_address)
        {
            if (response.size() < 2)
            {
                Log::error("STKHost", "Invalid STUN mapped address length.");
                return;
            }
            // Ignore the first byte as mentioned in Section 15.1 of RFC 5389.
            uint8_t ip_type = response.getUInt8();
            ip_type = response.getUInt8();
            if (ip_type == ipv4_returned)
            {
                // Above got 2 bytes
                if (size != 8 || response.size() < 6)
                {
                    Log::error("STKHost", "Invalid STUN mapped address length.");
                    return;
                }
                uint16_t port = response.getUInt16();
                uint32_t ip = response.getUInt32();
                if (type == xor_mapped_address)
                {
                    // Obfuscation is described in Section 15.2 of RFC 5389.
                    port ^= magic_cookie >> 16;
                    ip ^= magic_cookie;
                    xor_addr.setIP(ip);
                    xor_addr.setPort(port);
                }
                else
                {
                    non_xor_addr.setIP(ip);
                    non_xor_addr.setPort(port);
                }
            }
            else if (ip_type == ipv6_returned)
            {
                // Above got 2 bytes
                if (size != 20 || response.size() < 18)
                {
                    Log::error("STKHost", "Invalid STUN mapped address length.");
                    return;
                }
                uint16_t port = response.getUInt16();
                uint8_t bytes[16];
                for (int i = 0; i < 16; i++)
                    bytes[i] = response.getUInt8();
                if (type == xor_mapped_address)
                {
                    port ^= magic_cookie >> 16;
                    for (int i = 0; i < 16; i++)
                        bytes[i] ^= stun_tansaction_id[i];
                    xor_addr.setIPv6(bytes, port);
                }
                else
                {
                    non_xor_addr.setIPv6(bytes, port);
                }
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
            *result = xor_addr;
        }
        else
        {
            Log::warn("STKHost", "Only non xor-mapped address returned.");
            *result = non_xor_addr;
        }
    }
}   // getIPFromStun

//-----------------------------------------------------------------------------
/** Set the public address using stun protocol.
 */
void STKHost::setPublicAddress(short family)
{
    auto& stunv4_map = UserConfigParams::m_stun_servers_v4;
    for (auto& s : NetworkConfig::getStunList(true/*ipv4*/))
    {
        if (s.second == 0)
            stunv4_map.erase(s.first);
        else if (stunv4_map.find(s.first) == stunv4_map.end())
            stunv4_map[s.first] = 0;
    }

    auto& stunv6_map = UserConfigParams::m_stun_servers;
    for (auto& s : NetworkConfig::getStunList(false/*ipv4*/))
    {
        if (s.second == 0)
            stunv6_map.erase(s.first);
        else if (stunv6_map.find(s.first) == stunv6_map.end())
            stunv6_map[s.first] = 0;
    }

    auto& stun_map = family == AF_INET ? UserConfigParams::m_stun_servers_v4 :
        UserConfigParams::m_stun_servers;
    std::vector<std::pair<std::string, uint32_t> > untried_server;
    for (auto& p : stun_map)
        untried_server.push_back(p);

    // Randomly use stun servers of the low ping from top-half of the list
    std::sort(untried_server.begin(), untried_server.end(),
        [] (const std::pair<std::string, uint32_t>& a,
        const std::pair<std::string, uint32_t>& b)->bool
        {
            return a.second > b.second;
        });
    std::random_device rd;
    std::mt19937 g(rd());
    if (untried_server.size() > 2)
    {
        std::shuffle(untried_server.begin() + (untried_server.size() / 2),
            untried_server.end(), g);
    }
    else
    {
        Log::warn("STKHost", "Failed to get enough stun servers using SRV"
            " record.");
    }

    while (!untried_server.empty() && !ProtocolManager::lock()->isExiting())
    {
        // Pick last element in untried servers
        std::string server_name = untried_server.back().first.c_str();
        stun_map[server_name] = (uint32_t)-1;
        Log::debug("STKHost", "Using STUN server %s", server_name.c_str());
        uint64_t ping = StkTime::getMonoTimeMs();
        SocketAddress result;
        getIPFromStun((int)m_network->getENetHost()->socket, server_name, family,
            &result);
        if (!result.isUnset())
        {
            if (result.getFamily() != family)
            {
                Log::error("STKHost",
                    "Public address family differ from request");
                // This happens when you force to use IPv6 connection to detect
                // public address if there is only IPv4 connection, which it
                // will give you an IPv4 address, so we can exit earlier
                return;
            }
            // For dual stack server we get the IPv6 address and then IPv4, if
            // their ports differ we discard the public IPv6 address of server
            if (NetworkConfig::get()->isServer() && family == AF_INET &&
                result.getPort() != m_public_address->getPort())
            {
                if (isIPv6Socket())
                {
                    Log::error("STKHost",
                        "IPv6 has different port than IPv4.");
                }
                m_public_ipv6_address.clear();
            }
            if (family == AF_INET)
                *m_public_address = result;
            else
                m_public_ipv6_address = result.toString(false/*show_port*/);
            m_public_address->setPort(result.getPort());
            ping = StkTime::getMonoTimeMs() - ping;
            // Succeed, save ping
            stun_map[server_name] = (uint32_t)(ping);
            untried_server.clear();
        }
        else
        {
            // Erase from user config in stun, if it's provide by SRV records
            // from STK then it will be re-added next time, and STK team will
            // remove it if it stops working
            stun_map.erase(untried_server.back().first);
            untried_server.pop_back();
        }
    }
}   // setPublicAddress

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
        m_exit_timeout.store(StkTime::getMonoTimeMs() + 2000);
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
        Log::error("STKHost", "%s", StringUtils::wideToUtf8(message).c_str());
    }
    m_error_message = message;
}   // setErrorMessage

// ----------------------------------------------------------------------------
/** \brief Starts the listening of events from ENet.
 *  Starts a thread for receiveData that updates it as often as possible.
 */
void STKHost::startListening()
{
    m_exit_timeout.store(std::numeric_limits<uint64_t>::max());
    m_listening_thread = std::thread(std::bind(&STKHost::mainLoop, this,
        STKProcess::getType()));
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
 *  \param pt : Used to register to different singleton.
 */
void STKHost::mainLoop(ProcessType pt)
{
    std::string thread_name = "STKHost";
    if (pt == PT_CHILD)
        thread_name += "_child";
    VS::setThreadName(thread_name.c_str());

    STKProcess::init(pt);
    Log::info("STKHost", "Listening has been started.");
    ENetEvent event;
    ENetHost* host = m_network->getENetHost();
    const bool is_server = NetworkConfig::get()->isServer();

    // A separate network connection (socket) to handle LAN requests.
    Network* direct_socket = NULL;
    if ((NetworkConfig::get()->isLAN() && is_server) ||
        NetworkConfig::get()->isPublicServer())
    {
        ENetAddress eaddr = {};
        eaddr.port = stk_config->m_server_discovery_port;
        direct_socket = new Network(1, 1, 0, 0, &eaddr);
        if (direct_socket->getENetHost() == NULL)
        {
            Log::warn("STKHost", "No direct socket available, this "
                "server may not be connected by lan network");
            delete direct_socket;
            direct_socket = NULL;
        }
    }

    uint64_t last_ping_time = StkTime::getMonoTimeMs();
    uint64_t last_update_speed_time = StkTime::getMonoTimeMs();
    uint64_t last_ping_time_update_for_client = StkTime::getMonoTimeMs();
    std::map<std::string, uint64_t> ctp;
    while (m_exit_timeout.load() > StkTime::getMonoTimeMs())
    {
        // Clear outdated connect to peer list every 15 seconds
        for (auto it = ctp.begin(); it != ctp.end();)
        {
            if (it->second + 15000 < StkTime::getMonoTimeMs())
                it = ctp.erase(it);
            else
                it++;
        }

        if (last_update_speed_time < StkTime::getMonoTimeMs())
        {
            // Update upload / download speed per second
            last_update_speed_time = StkTime::getMonoTimeMs() + 1000;
            m_upload_speed.store(getNetwork()->getENetHost()->totalSentData);
            m_download_speed.store(
                getNetwork()->getENetHost()->totalReceivedData);
            getNetwork()->getENetHost()->totalSentData = 0;
            getNetwork()->getENetHost()->totalReceivedData = 0;
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
                last_ping_time < StkTime::getMonoTimeMs())
            {
                // If not racing, send an reliable packet at the 10 packets
                // per second, which is for accurate ping calculation by enet
                last_ping_time = StkTime::getMonoTimeMs() +
                    (uint64_t)((1.0f / 10.0f) * 1000.0f);
                need_ping = true;
            }

            BareNetworkString ping_packet;
            if (need_ping)
            {
                m_peer_pings.getData().clear();
                for (auto& p : m_peers)
                {
                    m_peer_pings.getData()[p.second->getHostId()] =
                        p.second->getPing();
                    // Set packet loss before enet command, so if the peer is
                    // disconnected later the loss won't be cleared
                    p.second->setPacketLoss(p.first->packetLoss);
                    const unsigned ap = p.second->getAveragePing();
                    const unsigned max_ping = ServerConfig::m_max_ping;
                    if (p.second->isValidated() &&
                        p.second->getConnectedTime() > 5.0f && ap > max_ping)
                    {
                        std::string player_name;
                        if (!p.second->getPlayerProfiles().empty())
                        {
                            player_name = StringUtils::wideToUtf8
                                (p.second->getPlayerProfiles()[0]->getName());
                        }
                        const bool peer_not_in_game =
                            sl->getCurrentState() <= ServerLobby::SELECTING
                            || p.second->isWaitingForGame();
                        if (ServerConfig::m_kick_high_ping_players &&
                            !p.second->isDisconnected() && peer_not_in_game)
                        {
                            Log::info("STKHost", "%s %s with ping %d is higher"
                                " than %d ms when not in game, kick.",
                                p.second->getAddress().toString().c_str(),
                                player_name.c_str(), ap, max_ping);
                            p.second->setWarnedForHighPing(true);
                            p.second->setDisconnected(true);
                            std::lock_guard<std::mutex> lock(m_enet_cmd_mutex);
                            m_enet_cmd.emplace_back(p.second->getENetPeer(),
                                (ENetPacket*)NULL, PDI_KICK_HIGH_PING,
                                ECT_DISCONNECT, p.first->address);
                        }
                        else if (!p.second->hasWarnedForHighPing())
                        {
                            Log::info("STKHost", "%s %s with ping %d is higher"
                                " than %d ms.",
                                p.second->getAddress().toString().c_str(),
                                player_name.c_str(), ap, max_ping);
                            p.second->setWarnedForHighPing(true);
                            NetworkString msg(PROTOCOL_LOBBY_ROOM);
                            msg.setSynchronous(true);
                            msg.addUInt8(LobbyProtocol::LE_BAD_CONNECTION);
                            p.second->sendPacket(&msg, /*reliable*/true);
                        }
                    }
                }
                uint64_t network_timer = getNetworkTimer();
                ping_packet.addUInt64(network_timer);
                ping_packet.addUInt8((uint8_t)m_peer_pings.getData().size());
                for (auto& p : m_peer_pings.getData())
                    ping_packet.addUInt32(p.first).addUInt32(p.second);
                if (sl)
                {
                    auto progress = sl->getGameStartedProgress();
                    ping_packet.addUInt32(progress.first)
                        .addUInt32(progress.second);
                    ping_packet.encodeString(sl->getPlayingTrackIdent());
                }
                else
                {
                    ping_packet.addUInt32(std::numeric_limits<uint32_t>::max())
                        .addUInt32(std::numeric_limits<uint32_t>::max())
                        .addUInt8(0);
                }
                ping_packet.getBuffer().insert(
                    ping_packet.getBuffer().begin(), g_ping_packet.begin(),
                    g_ping_packet.end());
            }

            for (auto it = m_peers.begin(); it != m_peers.end();)
            {
                if (!ping_packet.getBuffer().empty() &&
                    (!sl->allowJoinedPlayersWaiting() ||
                    !sl->isRacing() || it->second->isWaitingForGame()))
                {
                    ENetPacket* packet = enet_packet_create(ping_packet.getData(),
                        ping_packet.getTotalSize(), ENET_PACKET_FLAG_RELIABLE);
                    if (packet)
                    {
                        // If enet_peer_send failed, destroy the packet to
                        // prevent leaking, this can only be done if the packet
                        // is copied instead of shared sending to all peers
                        if (enet_peer_send(
                            it->first, EVENT_CHANNEL_UNENCRYPTED, packet) < 0)
                        {
                            enet_packet_destroy(packet);
                        }
                    }
                }

                // Remove peer which has not been validated after a specific time
                // It is validated when the first connection request has finished
                if (!it->second->isAIPeer() &&
                    !it->second->isValidated() &&
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
        }

        std::vector<std::tuple<ENetPeer*, ENetPacket*, uint32_t,
            ENetCommandType, ENetAddress> > copied_list;
        std::unique_lock<std::mutex> lock(m_enet_cmd_mutex);
        std::swap(copied_list, m_enet_cmd);
        lock.unlock();
        for (auto& p : copied_list)
        {
            ENetPeer* peer = std::get<0>(p);
            ENetAddress& ea = std::get<4>(p);
            ENetAddress& ea_peer_now = peer->address;
            ENetPacket* packet = std::get<1>(p);
            // Enet will reuse a disconnected peer so we check here to avoid
            // sending to wrong peer
            if (peer->state != ENET_PEER_STATE_CONNECTED ||
#if defined(ENABLE_IPV6) || defined(__SWITCH__)
                (enet_ip_not_equal(ea_peer_now.host, ea.host) &&
                ea_peer_now.port != ea.port))
#else
                (ea_peer_now.host != ea.host && ea_peer_now.port != ea.port))
#endif
            {
                if (packet != NULL)
                    enet_packet_destroy(packet);
                continue;
            }

            switch (std::get<3>(p))
            {
            case ECT_SEND_PACKET:
            {
                // If enet_peer_send failed, destroy the packet to
                // prevent leaking, this can only be done if the packet
                // is copied instead of shared sending to all peers
                if (enet_peer_send(peer, (uint8_t)std::get<2>(p), packet) < 0)
                {
                    enet_packet_destroy(packet);
                }
                break;
            }
            case ECT_DISCONNECT:
                enet_peer_disconnect(peer, std::get<2>(p));
                break;
            case ECT_RESET:
                // Flush enet before reset (so previous command is send)
                enet_host_flush(host);
                enet_peer_reset(peer);
                // Remove the stk peer of it
                std::lock_guard<std::mutex> lock(m_peers_mutex);
                m_peers.erase(peer);
                break;
            }
        }

        bool need_ping_update = false;
        while (enet_host_service(host, &event, 10) != 0)
        {
            auto lp = LobbyProtocol::get<LobbyProtocol>();
            if (!is_server &&
                last_ping_time_update_for_client < StkTime::getMonoTimeMs())
            {
                last_ping_time_update_for_client =
                    StkTime::getMonoTimeMs() + 2000;
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
                // ++m_next_unique_host_id for unique host id for database
                auto stk_peer = std::make_shared<STKPeer>
                    (event.peer, this, ++m_next_unique_host_id);
                std::unique_lock<std::mutex> lock(m_peers_mutex);
                m_peers[event.peer] = stk_peer;
                size_t new_peer_count = m_peers.size();
                lock.unlock();
                stk_event = new Event(&event, stk_peer);
                Log::info("STKHost", "%s has just connected. There are "
                    "now %u peers.", stk_peer->getAddress().toString().c_str(),
                    new_peer_count);
                // Client always trust the server
                if (!is_server)
                    stk_peer->setValidated(true);
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
                std::string addr;
                std::lock_guard<std::mutex> lock(m_peers_mutex);
                size_t new_peer_count = m_peers.size();
                if (m_peers.find(event.peer) != m_peers.end())
                {
                    std::shared_ptr<STKPeer>& peer = m_peers.at(event.peer);
                    addr = peer->getAddress().toString();
                    stk_event = new Event(&event, peer);
                    m_peers.erase(event.peer);
                    new_peer_count = m_peers.size();
                }
                Log::info("STKHost", "%s has just disconnected. There are "
                    "now %u peers.", addr.c_str(), new_peer_count);
            }   // ENET_EVENT_TYPE_DISCONNECT

            std::unique_lock<std::mutex> lock(m_peers_mutex);
            if (!stk_event && m_peers.find(event.peer) != m_peers.end())
            {
                std::shared_ptr<STKPeer> peer = m_peers.at(event.peer);
                lock.unlock();
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
                        uint32_t remaining_time =
                            std::numeric_limits<uint32_t>::max();
                        uint32_t progress =
                            std::numeric_limits<uint32_t>::max();
                        std::string current_track;
                        try
                        {
                            remaining_time = ping_packet.getUInt32();
                            progress = ping_packet.getUInt32();
                            ping_packet.decodeString(&current_track);
                        }
                        catch (std::exception& e)
                        {
                            // For old server
                            Log::debug("STKHost", "%s", e.what());
                        }
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
                            if (lp)
                            {
                                lp->setGameStartedProgress(
                                    std::make_pair(remaining_time, progress));
                                lp->storePlayingTrack(current_track);
                            }
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
    }   // while m_exit_timeout.load() > StkTime::getMonoTimeMs()
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

    SocketAddress sender;
    int len = direct_socket->receiveRawPacket(buffer, LEN, &sender, 1);
    if(len<=0) return;
    BareNetworkString message(buffer, len);
    std::string command;
    message.decodeString(&command);
    const std::string connection_cmd = std::string("connection-request") +
        StringUtils::toString(getPrivatePort());

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
        s.addUInt8((uint8_t)sl->getLobbyPlayers());
        s.addUInt16(getPrivatePort());
        s.addUInt8((uint8_t)sl->getDifficulty());
        s.addUInt8((uint8_t)sl->getGameMode());
        s.addUInt8(!pw.empty());
        s.addUInt8((uint8_t)
            (sl->getCurrentState() == ServerLobby::WAITING_FOR_START_GAME ?
            0 : 1));
        s.encodeString(sl->getPlayingTrackIdent());
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
            ctp[peer_addr] = StkTime::getMonoTimeMs();
            std::make_shared<ConnectToPeer>(sender)->requestStart();
        }
    }
    else if (command == "stk-server-port")
    {
        BareNetworkString s;
        s.addUInt16(getPrivatePort());
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
bool STKHost::peerExists(const SocketAddress& peer)
{
    std::lock_guard<std::mutex> lock(m_peers_mutex);
    for (auto p : m_peers)
    {
        auto stk_peer = p.second;
        if (stk_peer->getAddress() == peer ||
            ((stk_peer->getAddress().isPublicAddressLocalhost() &&
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
        if (!stk_peer->isValidated())
            continue;
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
    for (auto& peer : m_peers)
    {
        if (peer.second->isDisconnected() || !peer.second->isValidated())
            continue;
        if (ServerConfig::m_ai_handling && peer.second->isAIPeer())
            continue;
        auto peer_profile = peer.second->getPlayerProfiles();
        p.insert(p.end(), peer_profile.begin(), peer_profile.end());
    }
    lock.unlock();
    return p;
}   // getAllPlayerProfiles

//-----------------------------------------------------------------------------
std::set<uint32_t> STKHost::getAllPlayerOnlineIds() const
{
    std::set<uint32_t> online_ids;
    std::unique_lock<std::mutex> lock(m_peers_mutex);
    for (auto& peer : m_peers)
    {
        if (peer.second->isDisconnected() || !peer.second->isValidated())
            continue;
        if (!peer.second->getPlayerProfiles().empty())
        {
            online_ids.insert(
                peer.second->getPlayerProfiles()[0]->getOnlineId());
        }
    }
    lock.unlock();
    return online_ids;
}   // getAllPlayerOnlineIds

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
std::shared_ptr<STKPeer>
    STKHost::findPeerByName(const core::stringw& name) const
{
    std::lock_guard<std::mutex> lock(m_peers_mutex);
    auto ret = std::find_if(m_peers.begin(), m_peers.end(),
        [name](const std::pair<ENetPeer*, std::shared_ptr<STKPeer> >& p)
        {
            bool found = false;
            for (auto& profile : p.second->getPlayerProfiles())
            {
                if (profile->getName() == name)
                {
                    found = true;
                    break;
                }
            }
            return found;
        });
    return ret != m_peers.end() ? ret->second : nullptr;
}   // findPeerByName

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
    stk_peer->setValidated(true);
    m_peers[event.peer] = stk_peer;
    auto pm = ProtocolManager::lock();
    if (pm && !pm->isExiting())
        pm->propagateEvent(new Event(&event, stk_peer));
}   // initClientNetwork

// ----------------------------------------------------------------------------
std::pair<int, int> STKHost::getAllPlayersTeamInfo() const
{
    int red_count = 0;
    int blue_count = 0;
    auto pp = getAllPlayerProfiles();
    for (auto& player : pp)
    {
        if (player->getTeam() == KART_TEAM_RED)
            red_count++;
        else if (player->getTeam() == KART_TEAM_BLUE)
            blue_count++;
    }
    return std::make_pair(red_count, blue_count);

}   // getAllPlayersTeamInfo

// ----------------------------------------------------------------------------
/** Get the players for starting a new game.
 *  \return A vector containing pointers on the players profiles. */
std::vector<std::shared_ptr<NetworkPlayerProfile> >
    STKHost::getPlayersForNewGame(bool* has_always_on_spectators) const
{
    std::vector<std::shared_ptr<NetworkPlayerProfile> > players;
    std::lock_guard<std::mutex> lock(m_peers_mutex);
    for (auto& p : m_peers)
    {
        auto& stk_peer = p.second;
        // Handle always spectate for peer
        if (has_always_on_spectators && stk_peer->alwaysSpectate())
        {
            *has_always_on_spectators = true;
            stk_peer->setWaitingForGame(false);
            stk_peer->setSpectator(true);
            continue;
        }
        if (stk_peer->isWaitingForGame())
            continue;
        if (ServerConfig::m_ai_handling && stk_peer->isAIPeer())
            continue;
        for (auto& q : stk_peer->getPlayerProfiles())
            players.push_back(q);
    }
    return players;
}   // getPlayersForNewGame

// ----------------------------------------------------------------------------
/** Update players count in server
 *  \param ingame store the in game players count now
 *  \param waiting store the waiting players count now
 *  \param total store the total players count now
 */
void STKHost::updatePlayers(unsigned* ingame, unsigned* waiting,
                            unsigned* total)
{
    uint32_t ingame_players = 0;
    uint32_t waiting_players = 0;
    uint32_t total_players = 0;
    std::lock_guard<std::mutex> lock(m_peers_mutex);
    for (auto& p : m_peers)
    {
        auto& stk_peer = p.second;
        if (!stk_peer->isValidated())
            continue;
        if (ServerConfig::m_ai_handling && stk_peer->isAIPeer())
            continue;
        if (stk_peer->isWaitingForGame())
            waiting_players += (uint32_t)stk_peer->getPlayerProfiles().size();
        else
            ingame_players += (uint32_t)stk_peer->getPlayerProfiles().size();
        total_players += (uint32_t)stk_peer->getPlayerProfiles().size();
    }
    m_players_in_game.store(ingame_players);
    m_players_waiting.store(waiting_players);
    m_total_players.store(total_players);
    if (ingame)
        *ingame = ingame_players;
    if (waiting)
        *waiting = waiting_players;
    if (total)
        *total = total_players;
}   // updatePlayers

// ----------------------------------------------------------------------------
/** True if this is a client and server in graphics mode made by server
  *  creation screen. */
bool STKHost::isClientServer() const
{
    return m_client_loop != NULL;
}   // isClientServer

// ----------------------------------------------------------------------------
/** Return an valid public IPv4 or IPv6 address with port, empty if both are
 *  unset, IPv6 will come first if both exists. */
std::string STKHost::getValidPublicAddress() const
{
    if (!m_public_ipv6_address.empty() && m_public_address->getPort() != 0)
    {
        return std::string("[") + m_public_ipv6_address + "]:" +
            StringUtils::toString(m_public_address->getPort());
    }
    if (!m_public_address->isUnset())
        return m_public_address->toString();
    return "";
}   // getValidPublicAddress

// ----------------------------------------------------------------------------
int STKHost::receiveRawPacket(char *buffer, int buffer_len,
                              SocketAddress* sender, int max_tries)
{
    return m_network->receiveRawPacket(buffer, buffer_len, sender,
                                           max_tries);
}   // receiveRawPacket

// ----------------------------------------------------------------------------
void STKHost::sendRawPacket(const BareNetworkString &buffer,
                            const SocketAddress& dst)
{
    m_network->sendRawPacket(buffer, dst);
}  // sendRawPacket

// ----------------------------------------------------------------------------
uint16_t STKHost::getPrivatePort() const
{
    return m_network->getPort();
}  // getPrivatePort
