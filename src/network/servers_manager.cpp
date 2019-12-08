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
#include "network/stk_host.hpp"
#include "online/xml_request.hpp"
#include "online/request_manager.hpp"
#include "utils/translation.hpp"
#include "utils/time.hpp"

#include <assert.h>
#include <string>

#if defined(WIN32)
#  undef _WIN32_WINNT
#  define _WIN32_WINNT 0x600
#  include <iphlpapi.h>
#else
#  include <ifaddrs.h>
#endif

const int64_t SERVER_REFRESH_INTERVAL = 5000;

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
    reset();
}   // ServersManager

// ----------------------------------------------------------------------------
ServersManager::~ServersManager()
{
}   // ~ServersManager

// ----------------------------------------------------------------------------
/** Returns a WAN update-list-of-servers request. It queries the
 *  STK server for an up-to-date list of servers.
 */
std::shared_ptr<Online::XMLRequest> ServersManager::getWANRefreshRequest() const
{
    // ========================================================================
    /** A small local class that triggers an update of the ServersManager
     *  when the request is finished. */
    class WANRefreshRequest : public Online::XMLRequest
    {
    public:
        WANRefreshRequest() : Online::XMLRequest(/*priority*/100) {}
        // --------------------------------------------------------------------
        virtual void afterOperation() OVERRIDE
        {
            Online::XMLRequest::afterOperation();
            ServersManager::get()->setWanServers(isSuccess(), getXMLData());
        }   // callback
        // --------------------------------------------------------------------
    };   // RefreshRequest
    // ========================================================================

    auto request = std::make_shared<WANRefreshRequest>();
    request->setApiURL(Online::API::SERVER_PATH, "get-all");

    return request;
}   // getWANRefreshRequest

// ----------------------------------------------------------------------------
/** Returns a LAN update-list-of-servers request. It uses UDP broadcasts
 *  to find LAN servers, and waits for a certain amount of time fr 
 *  answers.
 */
std::shared_ptr<Online::XMLRequest> ServersManager::getLANRefreshRequest() const
{
    /** A simple class that uses LAN broadcasts to find local servers.
     *  It is based on XML request, but actually does not use any of the
     *  XML/HTTP based infrastructure, but implements the same interface.
     *  This way the already existing request thread can be used.
     */
    class LANRefreshRequest : public Online::XMLRequest
    {
    public:

        /** High priority for this request. */
        LANRefreshRequest() : XMLRequest(/*priority*/100) {m_success = false;}
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
            ENetAddress addr;
            addr.host = STKHost::HOST_ANY;
            addr.port = STKHost::PORT_ANY;
            Network *broadcast = new Network(1, 1, 0, 0, &addr);
            const std::vector<TransportAddress> &all_bcast =
                ServersManager::get()->getBroadcastAddresses();
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
            const auto& servers = ServersManager::get()->getServers();
            int cur_server_id = (int)servers.size();
            assert(cur_server_id == 0);
            // Use a map with the server name as key to automatically remove
            // duplicated answers from a server (since we potentially do
            // multiple broadcasts). We can not use the sender ip address,
            // because e.g. a local client would answer as 127.0.0.1 and
            // 192.168.**.
            std::map<irr::core::stringw, std::shared_ptr<Server> > servers_now;
            while (StkTime::getMonoTimeMs() - start_time < DURATION)
            {
                TransportAddress sender;
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
                    servers_now.insert(std::make_pair(name, 
                        std::make_shared<Server>(cur_server_id++, name, 
                        max_players, players, difficulty, mode, sender, 
                        password == 1, game_started == 1, current_track)));
                    //all_servers.[name] = servers_now.back();
                }   // if received_data
            }    // while still waiting
            m_success = true;
            ServersManager::get()->setLanServers(servers_now);
            delete broadcast;
        }   // operation
        // --------------------------------------------------------------------
        /** This function is necessary, otherwise the XML- and HTTP-Request
         *  functions are called, which will cause a crash. */
        virtual void afterOperation() OVERRIDE {}
        // --------------------------------------------------------------------
    };   // LANRefreshRequest
    // ========================================================================

    return std::make_shared<LANRefreshRequest>();

}   // getLANRefreshRequest

// ----------------------------------------------------------------------------
/** Takes a mapping of server name to server data (to avoid having the same 
 *  server listed more than once since the client will be doing multiple
 *  broadcasts to find a server), and converts this into a list of servers.
 *  \param servers Mapping of server name to Server object.
 */
void ServersManager::setLanServers(const std::map<irr::core::stringw,
                                                  std::shared_ptr<Server> >& servers)
{
    m_servers.clear();
    for (auto i : servers) m_servers.emplace_back(i.second);
    m_last_load_time.store(StkTime::getMonoTimeMs());
    m_list_updated = true;

}
// ----------------------------------------------------------------------------
/** Factory function to create either a LAN or a WAN update-of-server
 *  requests. The current list of servers is also cleared.
 */
bool ServersManager::refresh(bool full_refresh)
{
    if ((int64_t)StkTime::getMonoTimeMs() - m_last_load_time.load()
        < SERVER_REFRESH_INTERVAL)
    {
        // Avoid too frequent refreshing
        return false;
    }

    cleanUpServers();
    m_list_updated = false;

    if (NetworkConfig::get()->isWAN())
    {
        Online::RequestManager::get()->addRequest(getWANRefreshRequest());
    }
    else
    {
        if (full_refresh)
        {
            updateBroadcastAddresses();
        }
        
        Online::RequestManager::get()->addRequest(getLANRefreshRequest());
    }
    
    return true;
}   // refresh

// ----------------------------------------------------------------------------
/** Callback from the refresh request for wan servers.
 *  \param success If the refresh was successful.
 *  \param input The XML data describing the server.
 */
void ServersManager::setWanServers(bool success, const XMLNode* input)
{
    if (!success)
    {
        Log::error("Server Manager", "Could not refresh server list");
        m_list_updated = true;
        return;
    }

    const XMLNode *servers_xml = input->getNode("servers");
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
        m_servers.emplace_back(std::make_shared<Server>(*s));
    }
    m_last_load_time.store(StkTime::getMonoTimeMs());
    m_list_updated = true;
}   // refresh

// ----------------------------------------------------------------------------
/** Sets a list of default broadcast addresses which is used in case no valid
 *  broadcast address is found. This list includes default private network
 *  addresses.
 */
void ServersManager::setDefaultBroadcastAddresses()
{
    // Add some common LAN addresses
    m_broadcast_address.emplace_back(std::string("192.168.255.255"));
    m_broadcast_address.emplace_back(std::string("192.168.0.255")  );
    m_broadcast_address.emplace_back(std::string("192.168.1.255")  );
    m_broadcast_address.emplace_back(std::string("172.31.255.255") );
    m_broadcast_address.emplace_back(std::string("172.16.255.255") );
    m_broadcast_address.emplace_back(std::string("172.16.0.255")   );
    m_broadcast_address.emplace_back(std::string("10.255.255.255") );
    m_broadcast_address.emplace_back(std::string("10.0.255.255")   );
    m_broadcast_address.emplace_back(std::string("10.0.0.255")     );
    m_broadcast_address.emplace_back(std::string("255.255.255.255"));
    m_broadcast_address.emplace_back(std::string("127.0.0.255")    );
    m_broadcast_address.emplace_back(std::string("127.0.0.1")      );
    for (auto& addr : m_broadcast_address)
        addr.setPort(stk_config->m_server_discovery_port);
}   // setDefaultBroadcastAddresses

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
 */
void ServersManager::addAllBroadcastAddresses(const TransportAddress &a, int len)
{
    // Try different broadcast addresses - by masking on
    // byte boundaries
    while (len > 0)
    {
        unsigned int mask = (1 << len) - 1;
        TransportAddress bcast(a.getIP() | mask,
            stk_config->m_server_discovery_port);
        Log::info("Broadcast", "address %s length %d mask %x --> %s",
            a.toString().c_str(),
            len, mask,
            bcast.toString().c_str());
        m_broadcast_address.push_back(bcast);
        if (len % 8 != 0)
            len -= (len % 8);
        else
            len = len - 8;
    }   // while len > 0
}   // addAllBroadcastAddresses

// ----------------------------------------------------------------------------
/** Updates a list of all possible broadcast addresses on this machine.
 *  It queries all adapters for active IPv4 interfaces, determines their
 *  netmask to create the broadcast addresses. It will also add 'smaller'
 *  broadcast addesses, e.g. in a /16 network, it will add *.*.255.255 and
 *  *.*.*.255, since it was sometimes observed that routers would not let
 *  all broadcast addresses through. Duplicated answers (from the same server
 *  to different addersses) will be filtered out in ServersManager.
 */
void ServersManager::updateBroadcastAddresses()
{
    m_broadcast_address.clear();
    
#ifdef WIN32
    IP_ADAPTER_ADDRESSES *addresses;
    int count = 100, return_code;

    int iteration = 0;
    do
    {
        addresses = new IP_ADAPTER_ADDRESSES[count];
        ULONG buf_len = sizeof(IP_ADAPTER_ADDRESSES)*count;
        long flags = 0;
        return_code = GetAdaptersAddresses(AF_INET, flags, NULL, addresses,
            &buf_len);
        iteration++;
    } while (return_code == ERROR_BUFFER_OVERFLOW && iteration<10);

    if (return_code == ERROR_BUFFER_OVERFLOW)
    {
        Log::warn("ServerManager", "Can not get broadcast addresses.");
        setDefaultBroadcastAddresses();
        return;
    }

    for (IP_ADAPTER_ADDRESSES *p = addresses; p; p = p->Next)
    {
        // Check all operational IP4 adapters
        if (p->OperStatus == IfOperStatusUp &&
            p->FirstUnicastAddress->Address.lpSockaddr->sa_family == AF_INET)
        {
            const sockaddr_in *sa = (sockaddr_in*)p->FirstUnicastAddress->Address.lpSockaddr;
            // Use sa->sin_addr.S_un.S_addr and htonl?
            TransportAddress ta(sa->sin_addr.S_un.S_un_b.s_b1,
                sa->sin_addr.S_un.S_un_b.s_b2,
                sa->sin_addr.S_un.S_un_b.s_b3,
                sa->sin_addr.S_un.S_un_b.s_b4);
            int len = 32 - p->FirstUnicastAddress->OnLinkPrefixLength;
            addAllBroadcastAddresses(ta, len);
        }
    }
#else
    struct ifaddrs *addresses, *p;

    if (getifaddrs(&addresses) == -1)
    {
        Log::warn("ServerManager", "Error in getifaddrs");
        return;
    }
    for (p = addresses; p; p = p->ifa_next)
    {
        if (p->ifa_addr != NULL && p->ifa_addr->sa_family == AF_INET)
        {
            struct sockaddr_in *sa = (struct sockaddr_in *) p->ifa_addr;
            TransportAddress ta(htonl(sa->sin_addr.s_addr), 0);
            uint32_t u = ((sockaddr_in*)(p->ifa_netmask))->sin_addr.s_addr;
            // Convert mask to #bits:  SWAT algorithm
            u = u - ((u >> 1) & 0x55555555);
            u = (u & 0x33333333) + ((u >> 2) & 0x33333333);
            u = (((u + (u >> 4)) & 0x0F0F0F0F) * 0x01010101) >> 24;            

            Log::debug("ServerManager",
                "Interface: %s\tAddress: %s\tmask: %x\n", p->ifa_name,
                ta.toString().c_str(), u);
            addAllBroadcastAddresses(ta, u);
        }
    }
    freeifaddrs(addresses);
#endif
}   // updateBroadcastAddresses

// ----------------------------------------------------------------------------
/** Returns a list of all possible broadcast addresses on this machine.
 */
const std::vector<TransportAddress>& ServersManager::getBroadcastAddresses()
{
    if (m_broadcast_address.empty())
    {
        updateBroadcastAddresses();
    }

    return m_broadcast_address;
}   // getBroadcastAddresses
