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

#define SERVER_REFRESH_INTERVAL 5.0f

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
    m_last_load_time.store(0.0f);
    m_list_updated = false;
}   // ServersManager

// ----------------------------------------------------------------------------
ServersManager::~ServersManager()
{
}   // ~ServersManager

// ----------------------------------------------------------------------------
/** Returns a WAN update-list-of-servers request. It queries the
 *  STK server for an up-to-date list of servers.
 */
Online::XMLRequest* ServersManager::getWANRefreshRequest() const
{
    // ========================================================================
    /** A small local class that triggers an update of the ServersManager
     *  when the request is finished. */
    class WANRefreshRequest : public Online::XMLRequest
    {
    public:
        WANRefreshRequest() : Online::XMLRequest(/*manage_memory*/true,
                                                 /*priority*/100) {}
        // --------------------------------------------------------------------
        virtual void afterOperation() OVERRIDE
        {
            Online::XMLRequest::afterOperation();
            ServersManager::get()->setWanServers(isSuccess(), getXMLData());
        }   // callback
        // --------------------------------------------------------------------
    };   // RefreshRequest
    // ========================================================================

    Online::XMLRequest *request = new WANRefreshRequest();
    request->setApiURL(Online::API::SERVER_PATH, "get-all");

    return request;
}   // getWANRefreshRequest

// ----------------------------------------------------------------------------
/** Returns a LAN update-list-of-servers request. It uses UDP broadcasts
 *  to find LAN servers, and waits for a certain amount of time fr 
 *  answers.
 */
Online::XMLRequest* ServersManager::getLANRefreshRequest() const
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
        LANRefreshRequest() : XMLRequest(true, 100) {m_success = false;}
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
                broadcast->getBroadcastAddresses();
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
            double start_time = StkTime::getRealTime();
            const double DURATION = 1.0;
            const auto& servers = ServersManager::get()->getServers();
            int cur_server_id = (int)servers.size();
            assert(cur_server_id == 0);
            // Use a map with the server name as key to automatically remove
            // duplicated answers from a server (since we potentially do
            // multiple broadcasts). We can not use the sender ip address,
            // because e.g. a local client would answer as 127.0.0.1 and
            // 192.168.**.
            std::map<irr::core::stringw, std::shared_ptr<Server> > servers_now;
            while (StkTime::getRealTime() - start_time < DURATION)
            {
                TransportAddress sender;
                int len = broadcast->receiveRawPacket(buffer, LEN, &sender, 1);
                if (len > 0)
                {
                    BareNetworkString s(buffer, len);
                    int version = s.getUInt8();
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
                    servers_now.emplace(name, std::make_shared<Server>
                        (cur_server_id++, name, max_players, players,
                        difficulty, mode, sender, password == 1)       );
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

    return new LANRefreshRequest();

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
    m_list_updated = true;

}
// ----------------------------------------------------------------------------
/** Factory function to create either a LAN or a WAN update-of-server
 *  requests. The current list of servers is also cleared.
 */
bool ServersManager::refresh()
{
    if (StkTime::getRealTime() - m_last_load_time.load()
        < SERVER_REFRESH_INTERVAL)
    {
        // Avoid too frequent refreshing
        return false;
    }

    cleanUpServers();
    m_list_updated = false;
    if (NetworkConfig::get()->isWAN())
       Online::RequestManager::get()->addRequest(getWANRefreshRequest());
    else
       Online::RequestManager::get()->addRequest(getLANRefreshRequest());
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
        int version = 0;
        servers_xml->getNode(i)->get("version", &version);
        assert(version != 0);
        if (version < stk_config->m_max_server_version ||
            version > stk_config->m_max_server_version)
        {
            Log::verbose("ServersManager", "Skipping a server");
            continue;
        }
        m_servers.emplace_back(
            std::make_shared<Server>(*servers_xml->getNode(i)));
    }
    m_last_load_time.store((float)StkTime::getRealTime());
    m_list_updated = true;
}   // refresh
