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

#include "network/protocols/request_connection.hpp"

#include "config/user_config.hpp"
#include "network/network.hpp"
#include "network/network_config.hpp"
#include "network/protocol_manager.hpp"
#include "network/servers_manager.hpp"
#include "network/stk_host.hpp"
#include "online/xml_request.hpp"

using namespace Online;

/** Constructor. Stores the server id.
 *  \param server_id Id of the server.
 */
RequestConnection::RequestConnection(uint32_t server_id)
                 : Protocol(PROTOCOL_SILENT)
{
    m_server_id = server_id;
    m_request   = NULL;
}   // RequestConnection

// ----------------------------------------------------------------------------
RequestConnection::~RequestConnection()
{
}   // ~RequestConnection

// ----------------------------------------------------------------------------
/** Setup of this request, sets state to none.
 */
void RequestConnection::setup()
{
    m_state = NONE;
}   // setup

// ----------------------------------------------------------------------------
/** The callback for the server join request. It informs the server manager
 *  of a successful join request.
 */
void RequestConnection::ServerJoinRequest::callback()
{
    if (isSuccess())
    {
        uint32_t server_id;
        getXMLData()->get("serverid", &server_id);
        ServersManager::get()->setJoinedServer(server_id);
    }
}   // ServerJoinRequest::callback

// ----------------------------------------------------------------------------
/** This implements a finite state machine to monitor the server join
 *  request asynchronously.
 */
void RequestConnection::asynchronousUpdate()
{
    switch (m_state)
    {
        case NONE:
        {
            if (NetworkConfig::get()->isLAN() ||
                NetworkConfig::get()->isDirectConnect() ||
                STKHost::get()->isClientServer())
            {
                if (STKHost::get()->isClientServer())
                {
                    // Allow up to 10 seconds for the separate process to
                    // fully start-up
                    double timeout = StkTime::getRealTime() + 10.;
                    while (StkTime::getRealTime() < timeout)
                    {
                        const std::string& sid = NetworkConfig::get()
                            ->getServerIdFile();
                        assert(!sid.empty());
                        if (file_manager->fileExists(sid))
                        {
                            file_manager->removeFile(sid);
                            break;
                        }
                        StkTime::sleep(10);
                    }
                    NetworkConfig::get()->setServerIdFile("");
                }
                const Server *server =
                    ServersManager::get()->getServerByID(m_server_id);
                BareNetworkString message(std::string("connection-request"));
                STKHost::get()->sendRawPacket(message, server->getAddress());
                NetworkConfig::get()->setDirectConnect(false);
                m_state = DONE;
            }
            else
            {
                m_request = new ServerJoinRequest();
                NetworkConfig::get()->setUserDetails(m_request,
                    "request-connection");
                m_request->addParameter("server_id", m_server_id);
                m_request->queue();
                m_state = REQUEST_PENDING;
            }
            break;
        }
        case REQUEST_PENDING:
        {
            if (!m_request->isDone())
                return;

            const XMLNode * result = m_request->getXMLData();
            std::string rec_success;

            if(result->get("success", &rec_success))
            {
                if (rec_success == "yes")
                {
                    Log::debug("RequestConnection",
                               "Connection Request made successfully.");
                }
                else
                {
                    Log::error("RequestConnection",
                             "Fail to make a request to connecto to server %d",
                               m_server_id);
                }
            }
            else
            {
                Log::error("RequestConnection", "Fail to make a request.");
            }
            m_state = DONE;

            break;
        }
        case DONE:
            m_state = EXITING;
            delete m_request;
            m_request = NULL;
            requestTerminate();
            break;
        case EXITING:
            break;
    }
}   // asynchronousUpdate

