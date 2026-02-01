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
#include "guiengine/engine.hpp"
#include "network/crypto.hpp"
#include "network/event.hpp"
#include "network/network.hpp"
#include "network/network_config.hpp"
#include "network/protocols/client_lobby.hpp"
#include "network/protocol_manager.hpp"
#include "network/servers_manager.hpp"
#include "network/server.hpp"
#include "network/child_loop.hpp"
#include "network/socket_address.hpp"
#include "network/stk_ipv6.hpp"
#include "network/stk_host.hpp"
#include "network/stk_peer.hpp"
#include "online/xml_request.hpp"
#include "states_screens/online/networking_lobby.hpp"
#include "utils/time.hpp"
#include "utils/log.hpp"
#include "utils/string_utils.hpp"
#include "utils/translation.hpp"

#ifdef ANDROID
#include <jni.h>
#include "SDL_system.h"
#include "utils/utf8/unchecked.h"
#endif

#ifdef DNS_C
#  include <dns.h>
#else
#ifdef WIN32
#  include <windns.h>
#  include <ws2tcpip.h>
#ifndef __MINGW32__
#  pragma comment(lib, "dnsapi.lib")
#endif
#else
#ifndef __SWITCH__
#  include <arpa/nameser.h>
#  include <arpa/nameser_compat.h>
#endif
#  include <netdb.h>
#  include <netinet/in.h>
#ifndef __SWITCH__
#  include <resolv.h>
#endif
#endif
#endif

#include <algorithm>
// ============================================================================
ENetAddress ConnectToServer::m_server_address;
int ConnectToServer::m_retry_count = 0;
bool ConnectToServer::m_done_intecept = false;
// ----------------------------------------------------------------------------
/** Specify server to connect to.
 *  \param server Server to connect to (if nullptr than we use quick play).
 */
ConnectToServer::ConnectToServer(std::shared_ptr<Server> server)
               : Protocol(PROTOCOL_CONNECTION)
{
    m_server_address = {};
    if (server)
        m_server = server;
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
        // For graphical client server the IPv6 socket is handled by server
        // process
        if (!STKHost::get()->isClientServer())
        {
            if (m_server->useIPV6Connection())
                setIPv6Socket(1);
            else
                setIPv6Socket(0);
        }
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
    uint16_t port = 0;
    unsigned server_id = 0;
    ChildLoop* sl = STKHost::get()->getChildLoop();
    assert(sl);
    while (!ProtocolManager::lock()->isExiting() &&
        StkTime::getMonoTimeMs() < timeout)
    {
        port = sl->getPort();
        server_id = sl->getServerOnlineId();
        started = port != 0;
        if (started)
            break;
        StkTime::sleep(1);
    }
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
        m_server->setAddress(SocketAddress("127.0.0.1", port));
        m_server->setPrivatePort(port);
        // The server will decide if to use IPv6 socket
        if (isIPv6Socket())
        {
            m_server->setIPV6Address(SocketAddress("::1", port));
            m_server->setIPV6Connection(true);
        }
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
        m_server->getAddress().getPort() == 0)
    {
        getClientServerInfo();
    }

    switch(m_state.load())
    {
        case SET_PUBLIC_ADDRESS:
        {
            if (!m_server)
            {
                auto server_list =
                    ServersManager::get()->getWANRefreshRequest();
                while (!server_list->m_list_updated)
                {
                    if (ProtocolManager::lock()->isExiting())
                        return;
                    StkTime::sleep(1);
                }
                auto& servers = server_list->m_servers;

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
                }
                else
                {
                    // Shutdown STKHost (go back to online menu too)
                    STKHost::get()->setErrorMessage(_("No quick play server available."));
                    STKHost::get()->requestShutdown();
                    m_state = EXITING;
                    return;
                }
                servers.clear();
            }
            // Always use IPv6 connection for IPv6 only server
            if (m_server->getAddress().isUnset() &&
                NetworkConfig::get()->getIPType() != NetworkConfig::IP_V4)
                m_server->setIPV6Connection(true);

            // Auto enable IPv6 socket in client with NAT64, then we convert
            // the IPv4 address to NAT64 one in GOT_SERVER_ADDRESS
            bool ipv6_socket = m_server->useIPV6Connection() ||
                NetworkConfig::get()->getIPType() == NetworkConfig::IP_V6_NAT64;
            if (STKHost::get()->getNetwork()->isIPv6Socket() != ipv6_socket)
            {
                // Free the bound socket first
                delete STKHost::get()->getNetwork();
                setIPv6Socket(ipv6_socket ? 1 : 0);
                ENetAddress addr = {};
                addr.port = NetworkConfig::get()->getClientPort();
                auto new_network = new Network(/*peer_count*/1,
                    /*channel_limit*/EVENT_CHANNEL_COUNT,
                    /*max_in_bandwidth*/0, /*max_out_bandwidth*/0, &addr,
                    true/*change_port_if_bound*/);
                STKHost::get()->replaceNetwork(new_network);
            }

            if (m_server->supportsEncryption())
            {
                STKHost::get()->setPublicAddress(
                    m_server->useIPV6Connection() ? AF_INET6 : AF_INET);
                if (STKHost::get()->getValidPublicAddress().empty() ||
                    registerWithSTKServer() == false)
                {
                    // Set to DONE will stop STKHost is not connected
                    m_state = DONE;
                    break;
                }
            }
            m_state = GOT_SERVER_ADDRESS;
            break;
        }
        case GOT_SERVER_ADDRESS:
        {
            // Convert to a NAT64 address from IPv4
            if (!m_server->useIPV6Connection() &&
                NetworkConfig::get()->getIPType() == NetworkConfig::IP_V6_NAT64)
            {
                // From IPv4
                std::string addr_string =
                    m_server->getAddress().toString(false/*show_port*/);
                addr_string =
                    NetworkConfig::get()->getNAT64Prefix() + addr_string;
                SocketAddress nat64(addr_string,
                    m_server->getAddress().getPort());
                if (nat64.isUnset() || !nat64.isIPv6())
                {
                    Log::error("ConnectToServer", "Failed to synthesize IPv6 "
                        "address from %s", addr_string.c_str());
                    STKHost::get()->requestShutdown();
                    m_state = EXITING;
                    return;
                }
                m_server->setIPV6Address(nat64);
                m_server->setIPV6Connection(true);
            }

            // Detect port from possible connect-now or enter server address
            // dialog
            if ((m_server->useIPV6Connection() &&
                m_server->getIPV6Address()->getPort() == 0) ||
                (!m_server->useIPV6Connection() &&
                m_server->getAddress().getPort() == 0))
            {
                if (!detectPort())
                    return;
            }

            if (!STKHost::get()->getPublicAddress().isUnset() &&
                !STKHost::get()->isClientServer() &&
                m_server->getAddress().getIP() ==
                STKHost::get()->getPublicAddress().getIP())
            {
                Log::info("ConnectToServer", "Server is in the same lan");
                std::string str_msg("connection-request");
                BareNetworkString message(str_msg +
                    StringUtils::toString(m_server->getPrivatePort()));
                // If use lan connection for wan server, send to all broadcast
                // addresses
                for (auto& addr : ServersManager::get()
                    ->getBroadcastAddresses(isIPv6Socket()))
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
            // remapped.
            if (tryConnect(2000, 4, true/*another_port*/, isIPv6Socket()))
                break;
            if (!tryConnect(2000, 11, false/*another_port*/, isIPv6Socket()))
                m_state = DONE;
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
            if (!GUIEngine::isNoGraphics())
                NetworkingLobby::getInstance()->setJoinedServer(m_server);
            break;
        }
        case DONE:
        {
            // lobby room protocol if we're connected only
            if (STKHost::get()->getPeerCount() > 0 &&
                STKHost::get()->getServerPeerForClient()->isConnected())
            {
                m_server->saveServer();
                // Let main thread create ClientLobby for better
                // synchronization with GUI
                NetworkConfig::get()->clearActivePlayersForClient();
                auto cl = LobbyProtocol::create<ClientLobby>(m_server);
                STKHost::get()->startListening();
                cl->requestStart();
            }
            if (STKHost::get()->getPeerCount() == 0)
            {
                // Shutdown STKHost (go back to online menu too)
                core::stringw err =
                    _("Cannot connect to server %s.", m_server->getName());
                if (!m_error_msg.empty())
                {
                    err += L"\n";
                    err += m_error_msg;
                }
                STKHost::get()->setErrorMessage(err);
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
#if defined(ENABLE_IPV6) || defined(__SWITCH__)
        if (enet_ip_not_equal(host->receivedAddress.host, m_server_address.host) ||
#else
        if (host->receivedAddress.host != m_server_address.host ||
#endif
            host->receivedAddress.port != m_server_address.port)
        {
            SocketAddress new_address(host->receivedAddress);
            Log::info("ConnectToServer", "Using new server address %s",
                new_address.toString().c_str());
            m_retry_count = 15;
            m_server_address = host->receivedAddress;
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
    ENetAddress ea = {};
    Network* nw = another_port ? new Network(/*peer_count*/1,
        /*channel_limit*/EVENT_CHANNEL_COUNT,
        /*max_in_bandwidth*/0, /*max_out_bandwidth*/0, &ea,
        true/*change_port_if_bound*/) : STKHost::get()->getNetwork();
    assert(nw);

    m_done_intecept = false;
    nw->getENetHost()->intercept = ConnectToServer::interceptCallback;

    const SocketAddress* sa;
    if (ipv6)
    {
        sa = m_server->getIPV6Address();
        if (!sa)
            return false;
    }
    else
        sa = &m_server->getAddress();
    m_server_address = sa->toENetAddress();

    while (--m_retry_count >= 0 && !ProtocolManager::lock()->isExiting())
    {
        std::string connecting_address =
            SocketAddress(m_server_address).toString();
        ENetPeer* p = nw->connectTo(m_server_address);
        if (!p)
            break;
        Log::info("ConnectToServer", "Trying connecting to %s from port %d, "
            "retry remain: %d", connecting_address.c_str(),
            nw->getPort(), m_retry_count);
        int res;
        while ((res = enet_host_service(nw->getENetHost(), &event, timeout)) != 0)
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
bool ConnectToServer::registerWithSTKServer()
{
    // Our public address is now known, register details with
    // STK server
    const SocketAddress& addr = STKHost::get()->getPublicAddress();
    auto request = std::make_shared<Online::XMLRequest>();
    NetworkConfig::get()->setServerDetails(request, "join-server-key");
    request->addParameter("server-id", m_server->getServerId());
    request->addParameter("address", addr.getIP());
    request->addParameter("address-ipv6",
        STKHost::get()->getPublicIPv6Address());
    request->addParameter("port", addr.getPort());

    Crypto::initClientAES();
    request->addParameter("aes-key", Crypto::getClientKey());
    request->addParameter("aes-iv", Crypto::getClientIV());

    Log::info("ConnectToServer", "Registering addr %s",
        STKHost::get()->getValidPublicAddress().c_str());

    // This can be done blocking: till we are registered with the
    // stk server, there is no need to to react to any other 
    // network requests
    request->executeNow();

    const XMLNode* result = request->getXMLData();

    std::string success;
    if(result->get("success", &success) && success == "yes")
    {
        Log::debug("ConnectToServer", "Address registered successfully.");
        return true;
    }
    else
    {
        m_error_msg = request->getInfo();
        Log::error("ConnectToServer", "Failed to register client address: %s",
            StringUtils::wideToUtf8(m_error_msg).c_str());
        return false;
    }
}   // registerWithSTKServer

#ifdef ANDROID
auto get_txt_record = [](const core::stringw& name)->std::vector<std::string>
{
    std::vector<std::string> result;
    JNIEnv* env = (JNIEnv*)SDL_AndroidGetJNIEnv();
    if (env == NULL)
    {
        Log::error("ConnectToServer",
            "getDNSSrvRecords unable to SDL_AndroidGetJNIEnv.");
        return result;
    }

    jobject native_activity = (jobject)SDL_AndroidGetActivity();

    if (native_activity == NULL)
    {
        Log::error("ConnectToServer",
            "getDNSSrvRecords unable to SDL_AndroidGetActivity.");
        return result;
    }

    jclass class_native_activity = env->GetObjectClass(native_activity);

    if (class_native_activity == NULL)
    {
        Log::error("ConnectToServer",
            "getDNSTxtRecords unable to find object class.");
        env->DeleteLocalRef(native_activity);
        return result;
    }

    jmethodID method_id = env->GetMethodID(class_native_activity,
        "getDNSTxtRecords", "(Ljava/lang/String;)[Ljava/lang/String;");

    if (method_id == NULL)
    {
        Log::error("ConnectToServer",
            "getDNSTxtRecords unable to find method id.");
        env->DeleteLocalRef(class_native_activity);
        env->DeleteLocalRef(native_activity);
        return result;
    }

    std::vector<uint16_t> jstr_data;
    utf8::unchecked::utf32to16(
        name.c_str(), name.c_str() + name.size(), std::back_inserter(jstr_data));
    jstring text =
        env->NewString((const jchar*)jstr_data.data(), jstr_data.size());
    if (text == NULL)
    {
        Log::error("ConnectToServer",
            "Failed to create text for domain name.");
        env->DeleteLocalRef(class_native_activity);
        env->DeleteLocalRef(native_activity);
        return result;
    }

    jobjectArray arr =
        (jobjectArray)env->CallObjectMethod(native_activity, method_id, text);
    if (arr == NULL)
    {
        Log::error("ConnectToServer", "No array is created.");
        env->DeleteLocalRef(text);
        env->DeleteLocalRef(class_native_activity);
        env->DeleteLocalRef(native_activity);
        return result;
    }
    int len = env->GetArrayLength(arr);
    for (int i = 0; i < len; i++)
    {
        jstring jstr = (jstring)(env->GetObjectArrayElement(arr, i));
        if (!jstr)
            continue;
        const uint16_t* utf16_text =
            (const uint16_t*)env->GetStringChars(jstr, NULL);
        if (utf16_text == NULL)
        {
            env->DeleteLocalRef(jstr);
            continue;
        }
        const size_t str_len = env->GetStringLength(jstr);
        std::string tmp;
        utf8::unchecked::utf16to8(
            utf16_text, utf16_text + str_len, std::back_inserter(tmp));
        result.push_back(tmp);
        env->ReleaseStringChars(jstr, utf16_text);
        env->DeleteLocalRef(jstr);
    }
    env->DeleteLocalRef(arr);
    env->DeleteLocalRef(text);
    env->DeleteLocalRef(class_native_activity);
    env->DeleteLocalRef(native_activity);
    return result;
};
#endif

// ----------------------------------------------------------------------------
bool ConnectToServer::detectPort()
{
    // DNS txt record lookup, we will do stk-server-port server discovery port
    // too to get an updated sever address, in case it's differ then the
    // A / AAAA record (possible in LAN environment with multiple IPs assigned)
    int port_from_dns = 0;
    auto get_port = [](const std::string& txt_record)->int
    {
        const char* match = "stk-server-port=";
        size_t len = strlen(match);
        auto it = txt_record.find(match);
        if (it != std::string::npos && it + len < txt_record.size())
        {
            const std::string& ss =
                txt_record.substr(it + len, txt_record.size());
            int result = atoi(ss.c_str());
            if (result > 65535)
                result = 0;
            if (result != 0)
            {
                Log::info("ConnectToServer",
                    "Port %d found in DNS txt record %s", result,
                    txt_record.c_str());
                return result;
            }
        }
        return 0;
    };
#if defined(WIN32)
    PDNS_RECORD dns_record = NULL;
    DnsQuery(m_server->getName().c_str(), DNS_TYPE_TEXT,
        DNS_QUERY_STANDARD, NULL, &dns_record, NULL);
    if (dns_record)
    {
        for (PDNS_RECORD curr = dns_record; curr; curr = curr->pNext)
        {
            if (curr->wType == DNS_TYPE_TEXT)
            {
                for (unsigned i = 0; i < curr->Data.TXT.dwStringCount; i++)
                {
                    std::string txt_record = StringUtils::wideToUtf8(
                        curr->Data.TXT.pStringArray[i]);
                    port_from_dns = get_port(txt_record);
                    if (port_from_dns != 0)
                        break;
                }
            }
            if (port_from_dns != 0)
                break;
        }
        DnsRecordListFree(dns_record, DnsFreeRecordListDeep);
    }
#elif defined(ANDROID)
    std::vector<std::string> txt_records = get_txt_record(m_server->getName());
    for (auto& txt_record : txt_records)
    {
        port_from_dns = get_port(txt_record);
        if (port_from_dns != 0)
            break;
    }

#elif defined(DNS_C)
    const std::string& utf8name = StringUtils::wideToUtf8(m_server->getName());

    int err = 0;
    struct dns_socket* so = NULL;
    struct dns_packet* ans = NULL;
    struct dns_packet* quest = NULL;

    dns_options options = {};
    struct dns_rr_i rri = {};
    struct dns_rr rr = {};
    dns_resolv_conf* conf = dns_resconf_open(&err);
    if (!conf)
    {
        Log::error("ConnectToServer", "Error dns_resconf_open: %s",
            dns_strerror(err));
        goto cleanup;
    }
    if ((err = dns_resconf_loadpath(conf, "/etc/resolv.conf")))
    {
        err = 0;
        // Fallback name server
        SocketAddress addr("8.8.8.8:53");
        memcpy(&conf->nameserver[0], addr.getSockaddr(), addr.getSocklen());
    }

    so = dns_so_open((sockaddr*)&conf->iface, 0, &options, &err);
    if (!so)
    {
        Log::error("ConnectToServer", "Error dns_so_open: %s",
            dns_strerror(err));
        goto cleanup;
    }

    quest = dns_p_make(512, &err);
    if (err)
    {
        Log::error("ConnectToServer", "Error dns_p_make: %s",
            dns_strerror(err));
        goto cleanup;
    }
    if ((err = dns_p_push(quest, DNS_S_QD, utf8name.c_str(),
        utf8name.size(),
        DNS_T_TXT, DNS_C_IN, 0, 0)))
    {
        Log::error("ConnectToServer", "Error dns_p_push: %s",
            dns_strerror(err));
        goto cleanup;
    }

    dns_header(quest)->rd = 1;
    while (!(ans = dns_so_query(so, quest, (sockaddr*)&conf->nameserver[0],
        &err)))
    {
        if (err == 0)
        {
            Log::error("ConnectToServer", "Error dns_so_query: %s",
                dns_strerror(err));
            goto cleanup;
        }
        if (dns_so_elapsed(so) > 10)
        {
            Log::error("ConnectToServer", "Timeout dns_so_query.");
            goto cleanup;
        }
        dns_so_poll(so, 1);
    }
    if (!ans)
    {
        Log::error("ConnectToServer", "Timeout dns_so_query.");
        goto cleanup;
    }

    rri.sort = dns_rr_i_packet;
    err = 0;
    while (dns_rr_grep(&rr, 1, &rri, ans, &err))
    {
        if (err != 0)
        {
            Log::error("ConnectToServer", "Error dns_rr_grep: %s",
                dns_strerror(err));
            goto cleanup;
        }
        if (rr.section != DNS_S_ANSWER)
            continue;
        dns_txt txt = {};
        dns_txt_init(&txt, DNS_TXT_MINDATA);
        if (dns_txt_parse(&txt, &rr, ans) != 0)
            goto cleanup;
        std::string txt_record((char*)&txt.data, txt.len);
        port_from_dns = get_port(txt_record);
        if (port_from_dns != 0)
            break;
    }

cleanup:
    free(ans);
    free(quest);
    dns_so_close(so);
    dns_resconf_close(conf);

#else
    const std::string& utf8name = StringUtils::wideToUtf8(m_server->getName());
    unsigned char response[512] = {};

    int response_len = res_query(utf8name.c_str(), C_IN, T_TXT, response, 512);
    if (response_len > 0)
    {
        ns_msg query;
        if (ns_initparse(response, response_len, &query) >= 0)
        {
            unsigned msg_count = ns_msg_count(query, ns_s_an);
            for (unsigned i = 0; i < msg_count; i++)
            {
                ns_rr rr;
                if (ns_parserr(&query, ns_s_an, i, &rr) >= 0)
                {
                    // obtain the record data
                    const unsigned char* rd = ns_rr_rdata(rr);
                    // the first byte is the length of the data
                    size_t length = rd[0];
                    if (length == 0)
                        continue;
                    std::string txt_record((char*)rd + 1, length);
                    port_from_dns = get_port(txt_record);
                    if (port_from_dns != 0)
                        break;
                }
            }
        }
    }
#endif

    ENetAddress ea = {};
    std::unique_ptr<Network> nw(new Network(/*peer_count*/1,
        /*channel_limit*/EVENT_CHANNEL_COUNT,
        /*max_in_bandwidth*/0, /*max_out_bandwidth*/0, &ea,
        true/*change_port_if_bound*/));
    BareNetworkString s(std::string("stk-server-port"));
    SocketAddress address;
    if (m_server->useIPV6Connection())
        address = *m_server->getIPV6Address();
    else
        address = m_server->getAddress();
    address.setPort(stk_config->m_server_discovery_port);
    nw->sendRawPacket(s, address);
    SocketAddress sender;
    const int LEN = 2048;
    char buffer[LEN];
    int len = nw->receiveRawPacket(buffer, LEN, &sender, 2000);
    if (len == 2)
    {
        BareNetworkString server_port(buffer, len);
        uint16_t port = server_port.getUInt16();
        sender.setPort(port);
        // Use the DNS detected port over direct socket one, because only
        // one direct socket exists in a host even they have many stk servers
        if (port_from_dns != 0)
            sender.setPort((uint16_t)port_from_dns);
        // We replace the server address with sender with the detected port
        // completely, so we can use input like ff02::1 and then get the
        // correct local link server address
        if (m_server->useIPV6Connection())
            m_server->setIPV6Address(sender);
        else
            m_server->setAddress(sender);
        Log::info("ConnectToServer",
            "Detected new address %s for server address: %s.",
            sender.toString().c_str(), address.toString(false).c_str());
    }
    else if (port_from_dns != 0)
    {
        // Use only from dns record
        if (m_server->useIPV6Connection())
            m_server->getIPV6Address()->setPort(port_from_dns);
        else
        {
            SocketAddress addr = m_server->getAddress();
            addr.setPort(port_from_dns);
            m_server->setAddress(addr);
        }
    }
    else
    {
        const core::stringw& n = m_server->getName();
        //I18N: Show the failed detect port server name
        core::stringw e = _("Failed to detect port number for server %s.", n);
        STKHost::get()->setErrorMessage(e);
        STKHost::get()->requestShutdown();
        m_state = EXITING;
        return false;
    }
    return true;
}   // detectPort
