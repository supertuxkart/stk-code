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

#include "config/user_config.hpp"
#include "network/network.hpp"
#include "network/network_config.hpp"
#include "network/network_string.hpp"
#include "online/xml_request.hpp"
#include "utils/translation.hpp"
#include "utils/time.hpp"

#include <assert.h>
#include <irrString.h>
#include <string>

#define SERVER_REFRESH_INTERVAL 5.0f

static ServersManager* manager_singleton(NULL);

ServersManager* ServersManager::get()
{
    if (manager_singleton == NULL)
        manager_singleton = new ServersManager();

    return manager_singleton;
}   // get

// ------------------------------------------------------------------------
void ServersManager::deallocate()
{
    delete manager_singleton;
    manager_singleton = NULL;
}   // deallocate

// ===========================================================-=============
ServersManager::ServersManager()
{
    m_last_load_time.setAtomic(0.0f);
    m_joined_server.setAtomic(NULL);
}   // ServersManager

// ------------------------------------------------------------------------
ServersManager::~ServersManager()
{
    cleanUpServers();
}   // ~ServersManager

// ------------------------------------------------------------------------
/** Removes all stored server information. After this call the list of
 *  servers can be refreshed.
 */
void ServersManager::cleanUpServers()
{
    if(m_joined_server.getAtomic()!=NULL)
    {
        // m_joinsed_server is a pointer into the m_server structure,
        // we can not modify this data structure while this pointer exists.
        Log::warn("ServersManager", "Server cleanUp while being already "
                                    "connected to a server.");
        return;
    }

    m_sorted_servers.lock();
    m_sorted_servers.getData().clearAndDeleteAll();
    m_sorted_servers.unlock();
    m_mapped_servers.lock();
    m_mapped_servers.getData().clear();
    m_mapped_servers.unlock();
}   // cleanUpServers

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
        WANRefreshRequest() : Online::XMLRequest(/*manage_memory*/false,
                                                 /*priority*/100) {}
        // --------------------------------------------------------------------
        virtual void callback()
        {
            ServersManager::get()->refresh(isSuccess(), getXMLData());
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
        LANRefreshRequest() : XMLRequest(false, 100) {m_success = false;}
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
            Network *broadcast = new Network(1, 1, 0, 0);

            BareNetworkString s(std::string("stk-server"));
            TransportAddress broadcast_address(-1, 
                               NetworkConfig::get()->getServerDiscoveryPort());
            broadcast->sendRawPacket(s, broadcast_address);

            Log::info("ServersManager", "Sent broadcast message.");

            const int LEN=2048;
            char buffer[LEN];
            // Wait for up to 0.5 seconds to receive an answer from 
            // any local servers.
            double start_time = StkTime::getRealTime();
            const double DURATION = 1.0;
            while(StkTime::getRealTime() - start_time < DURATION)
            {
                TransportAddress sender;
                int len = broadcast->receiveRawPacket(buffer, LEN, &sender, 1);
                if(len>0)
                {
                    BareNetworkString s(buffer, len);
                    irr::core::stringw name;
                    // bytes_read is the number of bytes read
                    uint8_t bytes_read  = s.decodeStringW(&name);
                    uint8_t max_players = s.getUInt8();
                    uint8_t players     = s.getUInt8();
                    uint32_t my_ip      = s.getUInt32();
                    uint16_t my_port    = s.getUInt16();
                    uint16_t mode       = s.getUInt16();
                    uint8_t difficulty  = s.getUInt8();
                    Server* server = new Server(name, /*lan*/true,
                        max_players, players, sender);
                    server->setDifficulty((RaceManager::Difficulty)difficulty);
                    server->setRaceMinorMode((RaceManager::MinorRaceModeType)mode);
                    ServersManager::get()->addServer(server);

                    TransportAddress me(my_ip, my_port);
                    NetworkConfig::get()->setMyAddress(me);
                    m_success = true;
                }   // if received_data
            }    // while still waiting

            delete broadcast;
            if (!m_success)
                m_info = _("No LAN server detected");
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
/** Factory function to create either a LAN or a WAN update-of-server
 *  requests. The current list of servers is also cleared/
 */
Online::XMLRequest* ServersManager::getRefreshRequest(bool request_now)
{
    if (StkTime::getRealTime() - m_last_load_time.getAtomic()
        < SERVER_REFRESH_INTERVAL)
    {
        // Avoid too frequent refreshing
        return NULL;
    }

    if(m_joined_server.getAtomic()!=NULL)
    {
        // m_joinsed_server is a pointer into the m_server structure,
        // we can not modify this data structure while this pointer exists.
        Log::warn("ServersManager", "Server refresh while being already "
                                    "connected to a server.");
        return NULL;
    }

    cleanUpServers();
    Online::XMLRequest *request = 
        NetworkConfig::get()->isWAN() ? getWANRefreshRequest()
                                      : getLANRefreshRequest();

    if (request_now)
        Online::RequestManager::get()->addRequest(request);

    return request;
}   // getRefreshRequest

// ----------------------------------------------------------------------------
/** Callback from the refresh request.
 *  \param success If the refresh was successful.
 *  \param input The XML data describing the server.
 */
void ServersManager::refresh(bool success, const XMLNode *input)
{
    if (!success)
    {
        Log::error("Server Manager", "Could not refresh server list");
        return;
    }

    const XMLNode *servers_xml = input->getNode("servers");
    for (unsigned int i = 0; i < servers_xml->getNumNodes(); i++)
    {
        addServer(new Server(*servers_xml->getNode(i), /*is_lan*/false));
    }
    m_last_load_time.setAtomic((float)StkTime::getRealTime());
}   // refresh

// ----------------------------------------------------------------------------
const Server* ServersManager::getQuickPlay() const
{
    if (m_sorted_servers.getData().size() > 0)
        return getServerBySort(0);

    return NULL;
}   // getQuickPlay

// ----------------------------------------------------------------------------
/** Sets a pointer to the server to which this client is connected. From now
 *  on the list of servers must not be modified (else this pointer might
 *  become invalid).
 */
void ServersManager::setJoinedServer(uint32_t id)
{
    MutexLocker(m_mapped_servers);
    m_joined_server.getData() = m_mapped_servers.getData().at(id);
}   // setJoinedServer

// ----------------------------------------------------------------------------
/** Unsets the server to which this client is connected.
 */
void ServersManager::unsetJoinedServer()
{
    MutexLocker(m_joined_server);
    m_joined_server.getData() = NULL;
}   // unsetJoinedServer

// ----------------------------------------------------------------------------
/** Adds a server to the list of known servers.
 *  \param server The server to add.
 */
void ServersManager::addServer(Server *server)
{
    m_sorted_servers.lock();
    m_sorted_servers.getData().push_back(server);
    m_sorted_servers.unlock();

    m_mapped_servers.lock();
    m_mapped_servers.getData()[server->getServerId()] = server;
    m_mapped_servers.unlock();
}   // addServer

// ----------------------------------------------------------------------------
int ServersManager::getNumServers() const
{
    MutexLocker(m_sorted_servers);
    return m_sorted_servers.getData().size();
}   // getNumServers

// ----------------------------------------------------------------------------
const Server* ServersManager::getServerBySort(int index) const
{
    MutexLocker(m_sorted_servers);
    return m_sorted_servers.getData().get(index);
}   // getServerBySort

// ------------------------------------------------------------------------
const Server* ServersManager::getServerByID(uint32_t id) const
{
    MutexLocker(m_mapped_servers);
    return m_mapped_servers.getData().at(id);
}   // getServerByID

// ----------------------------------------------------------------------------
Server* ServersManager::getJoinedServer() const
{
    return m_joined_server.getAtomic();
}   // getJoinedServer

// ----------------------------------------------------------------------------
void ServersManager::sort(bool sort_desc)
{
    MutexLocker(m_sorted_servers);
    m_sorted_servers.getData().insertionSort(0, sort_desc);
}   // sort

