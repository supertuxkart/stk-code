//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2013-2015 Glenn De Jonghe
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

#include "network/servers_manager.hpp"

#include "config/stk_config.hpp"
#include "config/user_config.hpp"
#include "io/xml_node.hpp"
#include "network/network.hpp"
#include "network/network_config.hpp"
#include "network/network_string.hpp"
#include "network/server.hpp"
#include "network/socket_address.hpp"
#include "network/stk_host.hpp"
#include "network/stk_ipv6.hpp"
#include "online/xml_request.hpp"
#include "online/request_manager.hpp"
#include "utils/translation.hpp"
#include "utils/time.hpp"

#include <assert.h>
#include <functional>
#include <set>
#include <string>
#include <thread>

#if defined(WIN32)
#  undef _WIN32_WINNT
#  define _WIN32_WINNT 0x600
#  include <iphlpapi.h>
#else
#ifndef __SWITCH__
#  include <ifaddrs.h>
#else
extern "C" {
  #define u64 uint64_t
  #define u32 uint32_t
  #define s64 int64_t
  #define s32 int32_t
  #define Event libnx_Event
  #include <switch/services/nifm.h>
  #undef Event
  #undef u64
  #undef u32
  #undef s32
  #undef s64
}
#endif
#  include <net/if.h>
#endif

static ServersManager* g_manager_singleton(NULL);

// ============================================================================
ServersManager* ServersManager::get()
{
    if (g_manager_singleton == NULL)
        g_manager_singleton = new ServersManager();

    return g_manager_singleton;
}   // get

// ============================================================================
void ServersManager::deallocate()
{
    delete g_manager_singleton;
    g_manager_singleton = NULL;
}   // deallocate

// ----------------------------------------------------------------------------
ServersManager::ServersManager()
{
}   // ServersManager

// ----------------------------------------------------------------------------
ServersManager::~ServersManager()
{
}   // ~ServersManager

// ----------------------------------------------------------------------------
/** Returns a WAN update-list-of-servers request. It queries the
 *  STK server for an up-to-date list of servers.
 */
std::shared_ptr<ServerList> ServersManager::getWANRefreshRequest() const
{
    // ========================================================================
    /** A small local class that triggers an update of the ServersManager
     *  when the request is finished. */
    class WANRefreshRequest : public Online::XMLRequest
    {
    private:
        std::weak_ptr<ServerList> m_server_list;
        // Run the ip detect in separate thread, so it can be done parallel
        // with the wan server request (which takes few seconds too)
        uint64_t m_creation_time;
    public:
        WANRefreshRequest(std::shared_ptr<ServerList> server_list)
        : Online::XMLRequest(/*priority*/100)
        {
            NetworkConfig::queueIPDetection();
            m_creation_time = StkTime::getMonoTimeMs();
            m_server_list = server_list;
        }
        // --------------------------------------------------------------------
        virtual void afterOperation() OVERRIDE
        {
            Online::XMLRequest::afterOperation();
            // Wait at most 2 seconds for ip detection
            uint64_t timeout = StkTime::getMonoTimeMs() - m_creation_time;
            if (timeout > 2000)
                timeout = 0;
            else
                timeout = 2000 - timeout;
            NetworkConfig::get()->getIPDetectionResult(timeout);
            auto server_list = m_server_list.lock();
            if (!server_list)
                return;

            if (!isSuccess())
            {
                Log::error("ServersManager", "Could not refresh server list");
                server_list->m_list_updated = true;
                return;
            }

            const XMLNode *servers_xml = getXMLData()->getNode("servers");
            for (unsigned int i = 0; i < servers_xml->getNumNodes(); i++)
            {
                const XMLNode* s = servers_xml->getNode(i);
                assert(s);
                const XMLNode* si = s->getNode("server-info");
                assert(si);
                int version = 0;
                si->get("version", &version);
                assert(version != 0);
                if (version < stk_config->m_max_server_version ||
                    version > stk_config->m_max_server_version)
                {
                    Log::verbose("ServersManager", "Skipping a server");
                    continue;
                }
                std::shared_ptr<Server> ser = std::make_shared<Server>(*s);
                if (ser->getAddress().isUnset() &&
                    NetworkConfig::get()->getIPType() == NetworkConfig::IP_V4)
                {
                    Log::verbose("ServersManager",
                        "Skipping an IPv6 only server");
                    continue;
                }
                server_list->m_servers.emplace_back(ser);
            }
            server_list->m_list_updated = true;
        }   // afterOperation
        // --------------------------------------------------------------------
    };   // RefreshRequest
    // ========================================================================

    auto server_list = std::make_shared<ServerList>();
    auto request = std::make_shared<WANRefreshRequest>(server_list);
    request->setApiURL(Online::API::SERVER_PATH, "get-all");
    Online::RequestManager::get()->addRequest(request);
    return server_list;
}   // getWANRefreshRequest

// ----------------------------------------------------------------------------
/** Returns a LAN update-list-of-servers request. It uses UDP broadcasts
 *  to find LAN servers, and waits for a certain amount of time fr 
 *  answers.
 */
std::shared_ptr<ServerList> ServersManager::getLANRefreshRequest() const
{
    /** A simple class that uses LAN broadcasts to find local servers.
     *  It is based on XML request, but actually does not use any of the
     *  XML/HTTP based infrastructure, but implements the same interface.
     *  This way the already existing request thread can be used.
     */
    class LANRefreshRequest : public Online::XMLRequest
    {
    private:
        std::weak_ptr<ServerList> m_server_list;
    public:

        /** High priority for this request. */
        LANRefreshRequest(std::shared_ptr<ServerList> server_list)
            : XMLRequest(/*priority*/100)
        {
            m_success = false;
            m_server_list = server_list;
        }
        // --------------------------------------------------------------------
        virtual ~LANRefreshRequest() {}
        // --------------------------------------------------------------------
        /** Get the downloaded XML tree.
         *  \pre request has to be executed.
         *  \return get the complete result from the request reply. */
        virtual const XMLNode * getXMLData() const
        {
            assert(hasBeenExecuted());
            return NULL;
        }   // getXMLData
        // --------------------------------------------------------------------
        virtual void prepareOperation() OVERRIDE
        {
        }   // prepareOperation
        // --------------------------------------------------------------------
        virtual void operation() OVERRIDE
        {
            auto server_list = m_server_list.lock();
            if (!server_list)
                return;

            ENetAddress addr = {};
            setIPv6Socket(UserConfigParams::m_ipv6_lan ? 1 : 0);
            NetworkConfig::get()->setIPType(UserConfigParams::m_ipv6_lan ?
                NetworkConfig::IP_DUAL_STACK : NetworkConfig::IP_V4);
            Network *broadcast = new Network(1, 1, 0, 0, &addr);
            if (!broadcast->getENetHost())
            {
                setIPv6Socket(0);
                m_success = true;
                delete broadcast;
                server_list->m_list_updated = true;
                return;
            }
            const std::vector<SocketAddress> &all_bcast =
                ServersManager::get()->getBroadcastAddresses(
                UserConfigParams::m_ipv6_lan);
            for (auto &bcast_addr : all_bcast)
            {
                Log::info("Server Discovery", "Broadcasting to %s",
                          bcast_addr.toString().c_str());
                broadcast->sendRawPacket(std::string("stk-server"), bcast_addr);
            }

            Log::info("ServersManager", "Sent broadcast message.");

            const int LEN=2048;
            char buffer[LEN];
            // Wait for up to 0.5 seconds to receive an answer from 
            // any local servers.
            uint64_t start_time = StkTime::getMonoTimeMs();
            const uint64_t DURATION = 1000;
            int cur_server_id = 0;
            // Use a map with the server name as key to automatically remove
            // duplicated answers from a server (since we potentially do
            // multiple broadcasts). We can not use the sender ip address,
            // because e.g. a local client would answer as 127.0.0.1 and
            // 192.168.**.
            std::map<irr::core::stringw, std::shared_ptr<Server> > servers_now;
            while (StkTime::getMonoTimeMs() - start_time < DURATION)
            {
                SocketAddress sender;
                int len = broadcast->receiveRawPacket(buffer, LEN, &sender, 1);
                if (len > 0)
                {
                    BareNetworkString s(buffer, len);
                    int version = s.getUInt32();
                    if (version < stk_config->m_max_server_version ||
                        version > stk_config->m_max_server_version)
                    {
                        Log::verbose("ServersManager", "Skipping a server");
                        continue;
                    }
                    irr::core::stringw name;
                    // bytes_read is the number of bytes read
                    s.decodeStringW(&name);
                    uint8_t max_players = s.getUInt8();
                    uint8_t players     = s.getUInt8();
                    uint16_t port       = s.getUInt16();
                    uint8_t difficulty  = s.getUInt8();
                    uint8_t mode        = s.getUInt8();
                    sender.setPort(port);
                    uint8_t password    = s.getUInt8();
                    uint8_t game_started = s.getUInt8();
                    std::string current_track;
                    try
                    {
                        s.decodeString(&current_track);
                    }
                    catch (std::exception& e)
                    {
                        (void)e;
                    }
                    auto server = std::make_shared<Server>(cur_server_id++,
                        name, max_players, players, difficulty, mode,
                        SocketAddress(sender.getIP(), sender.getPort()),
                        password == 1, game_started == 1, current_track);
                    if (sender.isIPv6())
                    {
                        server->setIPV6Address(sender);
                        server->setIPV6Connection(true);
                    }
                    servers_now.insert(std::make_pair(name, server));
                    //all_servers.[name] = servers_now.back();
                }   // if received_data
            }    // while still waiting
            setIPv6Socket(0);
            delete broadcast;
            m_success = true;
            for (auto& i : servers_now)
                server_list->m_servers.emplace_back(i.second);
            server_list->m_list_updated = true;
        }   // operation
        // --------------------------------------------------------------------
        /** This function is necessary, otherwise the XML- and HTTP-Request
         *  functions are called, which will cause a crash. */
        virtual void afterOperation() OVERRIDE {}
        // --------------------------------------------------------------------
    };   // LANRefreshRequest
    // ========================================================================

    auto server_list = std::make_shared<ServerList>();
    auto request = std::make_shared<LANRefreshRequest>(server_list);
    Online::RequestManager::get()->addRequest(request);
    return server_list;

}   // getLANRefreshRequest

// ----------------------------------------------------------------------------
/** Sets a list of default broadcast addresses which is used in case no valid
 *  broadcast address is found. This list includes default private network
 *  addresses.
 */
std::vector<SocketAddress> ServersManager::getDefaultBroadcastAddresses()
{
    // Add some common LAN addresses
    std::vector<SocketAddress> result;
    uint16_t port = stk_config->m_server_discovery_port;
    result.emplace_back(std::string("192.168.255.255"), port);
    result.emplace_back(std::string("192.168.0.255"), port);
    result.emplace_back(std::string("192.168.1.255"), port);
    result.emplace_back(std::string("172.31.255.255"), port);
    result.emplace_back(std::string("172.16.255.255"), port);
    result.emplace_back(std::string("172.16.0.255"), port);
    result.emplace_back(std::string("10.255.255.255"), port);
    result.emplace_back(std::string("10.0.255.255"), port);
    result.emplace_back(std::string("10.0.0.255"), port);
    result.emplace_back(std::string("255.255.255.255"), port);
    result.emplace_back(std::string("127.0.0.255"), port);
    result.emplace_back(std::string("127.0.0.1"), port);
    return result;
}   // getDefaultBroadcastAddresses

// ----------------------------------------------------------------------------
/** This masks various possible broadcast addresses. For example, in a /16
 *  network it would first use *.*.255.255, then *.*.*.255. Also if the
 *  length of the mask is not a multiple of 8, the original value will
 *  be used, before multiple of 8 are create: /22 (*.3f.ff.ff), then
 *  /16 (*.*.ff.ff), /8 (*.*.*.ff). While this is usually an overkill,
 *  it can help in the case that the router does not forward a broadcast
 *  as expected (this problem often happens with 255.255.255.255, which is
 *  why this broadcast address creation code was added).
 *  \param a The transport address for which the broadcast addresses need
 *         to be created.
 *  \param len Number of bits to be or'ed.
 *  \param result Location to put address.
 */
void ServersManager::addAllBroadcastAddresses(const SocketAddress &a, int len,
                                            std::vector<SocketAddress>* result)
{
    // Try different broadcast addresses - by masking on
    // byte boundaries
    while (len > 0)
    {
        unsigned int mask = (1 << len) - 1;
        SocketAddress bcast(a.getIP() | mask,
            stk_config->m_server_discovery_port);
        Log::info("Broadcast", "address %s length %d mask %x --> %s",
            a.toString().c_str(),
            len, mask,
            bcast.toString().c_str());
        result->push_back(bcast);
        if (len % 8 != 0)
            len -= (len % 8);
        else
            len = len - 8;
    }   // while len > 0
}   // addAllBroadcastAddresses

// ----------------------------------------------------------------------------
/** Returns a list of all possible broadcast addresses on this machine.
 *  It queries all adapters for active IPv4 interfaces, determines their
 *  netmask to create the broadcast addresses. It will also add 'smaller'
 *  broadcast addesses, e.g. in a /16 network, it will add *.*.255.255 and
 *  *.*.*.255, since it was sometimes observed that routers would not let
 *  all broadcast addresses through. Duplicated answers (from the same server
 *  to different addersses) will be filtered out in ServersManager.
 */
std::vector<SocketAddress> ServersManager::getBroadcastAddresses(bool ipv6)
{
    std::vector<SocketAddress> result;
#ifdef __SWITCH__
    uint32_t addr;
    uint32_t u;

    Result resultCode = nifmGetCurrentIpConfigInfo(&addr, &u, NULL, NULL, NULL);

    if(R_SUCCEEDED(resultCode))
    {
        u = htonl(u);
        // Convert mask to #bits:  SWAT algorithm
        u = u - ((u >> 1) & 0x55555555);
        u = (u & 0x33333333) + ((u >> 2) & 0x33333333);
        u = (((u + (u >> 4)) & 0x0F0F0F0F) * 0x01010101) >> 24;
        SocketAddress saddr(htonl(addr));
        addAllBroadcastAddresses(saddr, u, &result);
    }
    else
    {
        Log::warn("ServersManager", "Failed to get broadcast address! Error 0x%x", resultCode);
        result = getDefaultBroadcastAddresses();
    }
#elif !defined(WIN32)
    struct ifaddrs *addresses, *p;

    if (getifaddrs(&addresses) == -1)
    {
        Log::warn("SocketAddress", "Error in getifaddrs");
        return result;
    }
    std::set<uint32_t> used_scope_id;
    for (p = addresses; p; p = p->ifa_next)
    {
        SocketAddress socket_address;
        if (p->ifa_addr == NULL)
            continue;
        if (p->ifa_addr->sa_family == AF_INET)
        {
            struct sockaddr_in *sa = (struct sockaddr_in *) p->ifa_addr;
            uint32_t addr = htonl(sa->sin_addr.s_addr);

            // Skip 169.254.*.* local link address
            if (((addr >> 24) & 0xff) == 169 &&
                ((addr >> 16) & 0xff) == 254)
                continue;

            SocketAddress saddr(addr, 0);
            uint32_t u = ((sockaddr_in*)(p->ifa_netmask))->sin_addr.s_addr;
            // Convert mask to #bits:  SWAT algorithm
            u = u - ((u >> 1) & 0x55555555);
            u = (u & 0x33333333) + ((u >> 2) & 0x33333333);
            u = (((u + (u >> 4)) & 0x0F0F0F0F) * 0x01010101) >> 24;            

            Log::debug("ServerManager",
                "Interface: %s\tAddress: %s\tmask: %x\n", p->ifa_name,
                saddr.toString().c_str(), u);
            addAllBroadcastAddresses(saddr, u, &result);
        }
        else if (p->ifa_addr->sa_family == AF_INET6 && ipv6)
        {
            uint32_t idx = if_nametoindex(p->ifa_name);
            if (used_scope_id.find(idx) != used_scope_id.end())
                continue;
            used_scope_id.insert(idx);
            SocketAddress socket_address("ff02::1",
                stk_config->m_server_discovery_port);
            sockaddr_in6* in6 = (sockaddr_in6*)socket_address.getSockaddr();
            in6->sin6_scope_id = idx;
            result.push_back(socket_address);
        }
    }
    freeifaddrs(addresses);
#else
    // From docs from microsoft it recommends 15k size
    const int WORKING_BUFFER_SIZE = 15000;
    PIP_ADAPTER_ADDRESSES paddr = NULL;
    unsigned long len = WORKING_BUFFER_SIZE;
    int return_code = 0;
    int iteration = 0;
    do
    {
        paddr = (IP_ADAPTER_ADDRESSES*)malloc(len);
        if (paddr == NULL)
            return result;
        long flags = 0;
        return_code = GetAdaptersAddresses(AF_UNSPEC, flags, NULL, paddr,
            &len);
        if (return_code == ERROR_BUFFER_OVERFLOW)
        {
            free(paddr);
            paddr = NULL;
        }
        else
            break;
        iteration++;
    } while ((return_code == ERROR_BUFFER_OVERFLOW) && (iteration < 10));

    if (return_code == ERROR_BUFFER_OVERFLOW)
    {
        Log::warn("ServerManager", "Can not get broadcast addresses.");
        result = getDefaultBroadcastAddresses();
        paddr = NULL;
    }

    for (IP_ADAPTER_ADDRESSES *p = paddr; p; p = p->Next)
    {
        if (p->OperStatus != IfOperStatusUp)
            continue;

        std::set<uint32_t> used_scope_id;
        for (PIP_ADAPTER_UNICAST_ADDRESS unicast = p->FirstUnicastAddress;
            unicast != NULL; unicast = unicast->Next)
        {
            SocketAddress socket_address;
            if (unicast->Address.lpSockaddr->sa_family == AF_INET)
            {
                const sockaddr_in *sa =
                    (const sockaddr_in*)unicast->Address.lpSockaddr;

                // Skip 169.254.*.* local link address
                if (sa->sin_addr.S_un.S_un_b.s_b1 == 169 &&
                    sa->sin_addr.S_un.S_un_b.s_b2 == 254)
                    continue;

                // Use sa->sin_addr.S_un.S_addr and htonl?
                SocketAddress ta(sa->sin_addr.S_un.S_un_b.s_b1,
                    sa->sin_addr.S_un.S_un_b.s_b2,
                    sa->sin_addr.S_un.S_un_b.s_b3,
                    sa->sin_addr.S_un.S_un_b.s_b4);
                int len = 32 - unicast->OnLinkPrefixLength;
                addAllBroadcastAddresses(ta, len, &result);
            }
            if (unicast->Address.lpSockaddr->sa_family == AF_INET6 && ipv6)
            {
                sockaddr_in6* in6 =
                    (sockaddr_in6*)unicast->Address.lpSockaddr;
                const uint32_t scope_id = in6->sin6_scope_id;
                if (used_scope_id.find(scope_id) !=
                    used_scope_id.end())
                    continue;
                used_scope_id.insert(scope_id);
                SocketAddress socket_address("ff02::1",
                    stk_config->m_server_discovery_port);
                in6 = (sockaddr_in6*)socket_address.getSockaddr();
                in6->sin6_scope_id = scope_id;
                result.push_back(socket_address);
            }
        }
    }
    free(paddr);
#endif
    if (ipv6)
    {
        // Convert IPv4 socket address to ::ffff:x.y.z.w
        for (auto& addr : result)
            addr.convertForIPv6Socket(ipv6);
    }
    return result;
}   // getBroadcastAddresses
