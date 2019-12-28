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

#include "network/protocols/connect_to_server.hpp"

#include "config/user_config.hpp"
#include "io/file_manager.hpp"
#include "io/xml_node.hpp"
#include "network/crypto.hpp"
#include "network/event.hpp"
#include "network/network.hpp"
#include "network/network_config.hpp"
#include "network/protocols/client_lobby.hpp"
#include "network/protocol_manager.hpp"
#include "network/servers_manager.hpp"
#include "network/server.hpp"
#include "network/stk_ipv6.hpp"
#include "network/stk_host.hpp"
#include "network/stk_peer.hpp"
#include "online/xml_request.hpp"
#include "states_screens/online/networking_lobby.hpp"
#include "utils/time.hpp"
#include "utils/log.hpp"
#include "utils/string_utils.hpp"
#include "utils/translation.hpp"

#ifdef WIN32
#  include <ws2tcpip.h>
#else
#  include <netdb.h>
#endif

#include <algorithm>
// ============================================================================
std::weak_ptr<Online::Request> ConnectToServer::m_previous_unjoin;
TransportAddress ConnectToServer::m_server_address;
int ConnectToServer::m_retry_count = 0;
bool ConnectToServer::m_done_intecept = false;
// ----------------------------------------------------------------------------
/** Specify server to connect to.
 *  \param server Server to connect to (if nullptr than we use quick play).
 */
ConnectToServer::ConnectToServer(std::shared_ptr<Server> server)
               : Protocol(PROTOCOL_CONNECTION)
{
    m_quick_play_err_msg = _("No quick play server available.");
    if (server)
    {
        m_server         = server;
        m_server_address = m_server->getAddress();
    }
}   // ConnectToServer(server, host)

// ----------------------------------------------------------------------------
/** Destructor. 
 */
ConnectToServer::~ConnectToServer()
{
    auto cl = LobbyProtocol::get<ClientLobby>();
    if (!cl && m_server && m_server->supportsEncryption())
    {
        auto request = std::make_shared<Online::XMLRequest>();
        NetworkConfig::get()->setServerDetails(request,
            "clear-user-joined-server");
        request->queue();
        m_previous_unjoin = request;
    }
}   // ~ConnectToServer

// ----------------------------------------------------------------------------
/** Initialise the protocol.
 */
void ConnectToServer::setup()
{
    Log::info("ConnectToServer", "SETUP");
    // In case of LAN or client-server we already have the server's
    // and our ip address, so we can immediately start requesting a connection.
    if (NetworkConfig::get()->isLAN())
    {
        m_state = GOT_SERVER_ADDRESS;
        if (m_server->useIPV6Connection())
            setIPV6(1);
    }
    else
        m_state = SET_PUBLIC_ADDRESS;
}   // setup

// ----------------------------------------------------------------------------
void ConnectToServer::getClientServerInfo()
{
    assert(m_server);
    // Allow up to 10 seconds for the separate process to fully start-up
    bool started = false;
    uint64_t timeout = StkTime::getMonoTimeMs() + 10000;
    const std::string& sid = NetworkConfig::get()->getServerIdFile();
    assert(!sid.empty());
    const std::string dir = StringUtils::getPath(sid);
    const std::string server_id_file = StringUtils::getBasename(sid);
    uint16_t port = 0;
    unsigned server_id = 0;
    while (!ProtocolManager::lock()->isExiting() &&
        StkTime::getMonoTimeMs() < timeout)
    {
        std::set<std::string> files;
        file_manager->listFiles(files, dir);
        for (auto& f : files)
        {
            if (f.find(server_id_file) != std::string::npos)
            {
                auto split = StringUtils::split(f, '_');
                if (split.size() != 3)
                    continue;
                if (!StringUtils::fromString(split[1], server_id))
                    continue;
                if (!StringUtils::fromString(split[2], port))
                    continue;
                file_manager->removeFile(dir + "/" + f);
                started = true;
                break;
            }
        }
        if (started)
            break;
        StkTime::sleep(10);
    }
    NetworkConfig::get()->setServerIdFile("");
    if (!started)
    {
        Log::error("ConnectToServer",
            "Separate server process failed to started");
        m_state = DONE;
        return;
    }
    else
    {
        assert(port != 0);
        m_server_address.setPort(port);
        m_server->setPrivatePort(port);
        if (server_id != 0)
        {
            m_server->setSupportsEncryption(true);
            m_server->setServerId(server_id);
        }
    }
}   // getClientServerInfo

// ----------------------------------------------------------------------------
void ConnectToServer::asynchronousUpdate()
{
    if (STKHost::get()->isClientServer() &&
        !NetworkConfig::get()->getServerIdFile().empty())
    {
        getClientServerInfo();
    }

    switch(m_state.load())
    {
        case SET_PUBLIC_ADDRESS:
        {
            if (!m_server)
            {
                while (!ServersManager::get()->refresh(false))
                {
                    if (ProtocolManager::lock()->isExiting())
                        return;
                    StkTime::sleep(1);
                }
                while (!ServersManager::get()->listUpdated())
                {
                    if (ProtocolManager::lock()->isExiting())
                        return;
                    StkTime::sleep(1);
                }
                auto servers = std::move(ServersManager::get()->getServers());

                // Remove password protected servers
                servers.erase(std::remove_if(servers.begin(), servers.end(), []
                    (const std::shared_ptr<Server> a)->bool
                    {
                        return a->isPasswordProtected();
                    }), servers.end());

                if (!servers.empty())
                {
                    // For quick play we choose the server with the shortest
                    // distance and not empty and full server
                    std::sort(servers.begin(), servers.end(), []
                        (const std::shared_ptr<Server> a,
                        const std::shared_ptr<Server> b)->bool
                        {
                            return a->getDistance() < b->getDistance();
                        });
                    std::stable_partition(servers.begin(), servers.end(), []
                        (const std::shared_ptr<Server> a)->bool
                        {
                            return a->getCurrentPlayers() != 0 &&
                                a->getCurrentPlayers() != a->getMaxPlayers();
                        });
                    m_server = servers[0];
                    m_server_address = m_server->getAddress();
                }
                else
                {
                    // Shutdown STKHost (go back to online menu too)
                    STKHost::get()->setErrorMessage(m_quick_play_err_msg);
                    STKHost::get()->requestShutdown();
                    m_state = EXITING;
                    return;
                }
                servers.clear();
            }
            if (m_server->useIPV6Connection())
            {
                // Disable STUN if using IPv6 (check in setPublicAddress)
                setIPV6(1);
            }
            if (m_server->supportsEncryption())
            {
                STKHost::get()->setPublicAddress();
                registerWithSTKServer();
            }
            // Set to DONE will stop STKHost is not connected
            m_state = STKHost::get()->getPublicAddress().isUnset() ?
                DONE : GOT_SERVER_ADDRESS;
            break;
        }
        case GOT_SERVER_ADDRESS:
        {
            if (!STKHost::get()->isClientServer() &&
                m_server_address.getIP() ==
                STKHost::get()->getPublicAddress().getIP())
            {
                Log::info("ConnectToServer", "Server is in the same lan");
                std::string str_msg("connection-request");
                BareNetworkString message(str_msg +
                    StringUtils::toString(m_server->getPrivatePort()));
                // If use lan connection for wan server, send to all broadcast
                // addresses
                for (auto& addr :
                    ServersManager::get()->getBroadcastAddresses())
                {
                    for (int i = 0; i < 5; i++)
                    {
                        STKHost::get()->sendRawPacket(message, addr);
                        StkTime::sleep(1);
                    }
                }
            }
            // 30 seconds connecting timeout total, use another port to try
            // direct connection to server first, if failed than use the one
            // that has stun mapped, the first 8 seconds allow the server to
            // start the connect to peer protocol first before the port is
            // remapped. IPv6 has no stun so try once with any port
            if (isIPV6())
            {
                if (!tryConnect(2000, 15, true/*another_port*/, true/*IPv6*/))
                    m_state = DONE;
            }
            else
            {
                if (tryConnect(2000, 4, true/*another_port*/))
                    break;
                if (!tryConnect(2000, 11))
                    m_state = DONE;
            }
            break;
        }
        case DONE:
        case EXITING:
            break;
    }
}   // asynchronousUpdate

// ----------------------------------------------------------------------------
void ConnectToServer::update(int ticks)
{
    switch(m_state.load())
    {
        case GOT_SERVER_ADDRESS:
        {
            // Make sure lobby display the quick play server name
            assert(m_server);
            NetworkingLobby::getInstance()->setJoinedServer(m_server);
            break;
        }
        case DONE:
        {
            // lobby room protocol if we're connected only
            if (STKHost::get()->getPeerCount() > 0 &&
                STKHost::get()->getServerPeerForClient()->isConnected() &&
                !m_server_address.isUnset())
            {
                // Let main thread create ClientLobby for better
                // synchronization with GUI
                NetworkConfig::get()->clearActivePlayersForClient();
                auto cl = LobbyProtocol::create<ClientLobby>(m_server_address,
                    m_server);
                STKHost::get()->startListening();
                cl->requestStart();
            }
            if (STKHost::get()->getPeerCount() == 0)
            {
                // Shutdown STKHost (go back to online menu too)
                STKHost::get()->setErrorMessage(
                    _("Cannot connect to server %s.", m_server->getName()));
                STKHost::get()->requestShutdown();
            }
            requestTerminate();
            m_state = EXITING;
            break;
        }
        default:
            break;
    }
}   // update

// ----------------------------------------------------------------------------
/** Intercept callback in enet to allow change server address and port if
 *  needed (Happens when there is firewall in between)
 */
int ConnectToServer::interceptCallback(ENetHost* host, ENetEvent* event)
{
    if (m_done_intecept)
        return 0;
    // The first two bytes of a valid ENet protocol packet will never be 0xFFFF
    // and then try decode the string "aloha-stk"
    if (host->receivedDataLength == 12 &&
        host->receivedData[0] == 0xFF && host->receivedData[1]  == 0xFF &&
        host->receivedData[2] == 0x09 && host->receivedData[3] == 'a' &&
        host->receivedData[4]  == 'l' && host->receivedData[5] == 'o' &&
        host->receivedData[6] == 'h' && host->receivedData[7] == 'a' &&
        host->receivedData[8] == '-' && host->receivedData[9] == 's' &&
        host->receivedData[10] == 't' && host->receivedData[11] == 'k')
    {
        TransportAddress server_addr = host->receivedAddress;
        if (server_addr != m_server_address)
        {
            Log::info("ConnectToServer", "Using new server address %s",
                server_addr.toString().c_str());
            m_retry_count = 15;
            m_server_address = server_addr;
            m_done_intecept = true;
            return 1;
        }
    }
    // 0 means let enet handle this packet
    return 0;
}   // interceptCallback

// ----------------------------------------------------------------------------
bool ConnectToServer::tryConnect(int timeout, int retry, bool another_port,
                                 bool ipv6)
{
    m_retry_count = retry;
    ENetEvent event;
    ENetAddress ea;
    ea.host = STKHost::HOST_ANY;
    ea.port = STKHost::PORT_ANY;
    Network* nw = another_port ? new Network(/*peer_count*/1,
        /*channel_limit*/EVENT_CHANNEL_COUNT,
        /*max_in_bandwidth*/0, /*max_out_bandwidth*/0, &ea,
        true/*change_port_if_bound*/) : STKHost::get()->getNetwork();
    assert(nw);

    m_done_intecept = false;
    nw->getENetHost()->intercept = ConnectToServer::interceptCallback;

    std::string connecting_address = m_server_address.toString();
    if (ipv6)
    {
        struct addrinfo hints;
        struct addrinfo* res = NULL;
        memset(&hints, 0, sizeof(hints));
        hints.ai_family = AF_UNSPEC;
        hints.ai_socktype = SOCK_STREAM;
        std::string addr_string = m_server->getIPV6Address();
        std::string port =
            StringUtils::toString(m_server->getAddress().getPort());
#ifdef IOS_STK
        // The ability to synthesize IPv6 addresses was added to getaddrinfo
        // in iOS 9.2
        if (!m_server->useIPV6Connection())
        {
            // From IPv4
            addr_string = m_server->getAddress().toString(false/*show_port*/);
        }
#endif
        if (getaddrinfo_compat(addr_string.c_str(), port.c_str(),
            &hints, &res) != 0 || res == NULL)
            return false;
        for (const struct addrinfo* addr = res; addr != NULL;
             addr = addr->ai_next)
        {
            if (addr->ai_family == AF_INET6)
            {
                struct sockaddr_in6* ipv6_sock =
                    (struct sockaddr_in6*)addr->ai_addr;
                ENetAddress addr = m_server_address.toEnetAddress();
                connecting_address = std::string("[") + addr_string + "]:" +
                    StringUtils::toString(addr.port);
                addMappedAddress(&addr, ipv6_sock);
                break;
            }
        }
        freeaddrinfo(res);
    }

    while (--m_retry_count >= 0 && !ProtocolManager::lock()->isExiting())
    {
        ENetPeer* p = nw->connectTo(m_server_address);
        if (!p)
            break;
        Log::info("ConnectToServer", "Trying connecting to %s from port %d, "
            "retry remain: %d", connecting_address.c_str(),
            nw->getENetHost()->address.port, m_retry_count);
        while (enet_host_service(nw->getENetHost(), &event, timeout) != 0)
        {
            if (event.type == ENET_EVENT_TYPE_CONNECT)
            {
                Log::info("ConnectToServer", "Connected to %s",
                    connecting_address.c_str());
                nw->getENetHost()->intercept = NULL;
                STKHost::get()->initClientNetwork(event, nw);
                m_state = DONE;
                return true;
            }
        }
        // Reset old peer in case server address differs due to intercept
        enet_peer_reset(p);
    }
    if (another_port)
        delete nw;
    return false;
}   // tryConnect

// ----------------------------------------------------------------------------
/** Register this client with the STK server.
 */
void ConnectToServer::registerWithSTKServer()
{
    // Our public address is now known, register details with
    // STK server. If previous unjoin request is not finished, wait
    if (!m_previous_unjoin.expired())
    {
        if (ProtocolManager::lock()->isExiting())
            return;
        StkTime::sleep(1);
    }

    const TransportAddress& addr = STKHost::get()->getPublicAddress();
    auto request = std::make_shared<Online::XMLRequest>();
    NetworkConfig::get()->setServerDetails(request, "join-server-key");
    request->addParameter("server-id", m_server->getServerId());
    request->addParameter("address", addr.getIP());
    request->addParameter("port", addr.getPort());

    Crypto::initClientAES();
    request->addParameter("aes-key", Crypto::getClientKey());
    request->addParameter("aes-iv", Crypto::getClientIV());

    Log::info("ConnectToServer", "Registering addr %s",
        addr.toString().c_str());

    // This can be done blocking: till we are registered with the
    // stk server, there is no need to to react to any other 
    // network requests
    request->executeNow();

    const XMLNode* result = request->getXMLData();

    std::string success;
    if(result->get("success", &success) && success == "yes")
    {
        Log::debug("ConnectToServer", "Address registered successfully.");
    }
    else
    {
        irr::core::stringc error(request->getInfo().c_str());
        Log::error("ConnectToServer", "Failed to register client address: %s",
            error.c_str());
        m_state = DONE;
    }
}   // registerWithSTKServer
