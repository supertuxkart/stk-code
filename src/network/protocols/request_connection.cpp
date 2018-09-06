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
#include "network/server.hpp"
#include "network/servers_manager.hpp"
#include "network/stk_host.hpp"
#include "online/xml_request.hpp"
#include "utils/string_utils.hpp"

using namespace Online;

/** Constructor. Stores the server id.
 *  \param server Server to be joined.
 */
RequestConnection::RequestConnection(std::shared_ptr<Server> server)
                 : Protocol(PROTOCOL_SILENT)
{
    m_server  = server;
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
/** This implements a finite state machine to monitor the server join
 *  request asynchronously.
 */
void RequestConnection::asynchronousUpdate()
{
    switch (m_state)
    {
        case NONE:
        {
            if ((!NetworkConfig::m_disable_lan &&
                m_server->getAddress().getIP() ==
                STKHost::get()->getPublicAddress().getIP()) ||
                (NetworkConfig::get()->isLAN() ||
                STKHost::get()->isClientServer()))
            {
                if (NetworkConfig::get()->isWAN())
                {
                    Log::info("RequestConnection",
                        "LAN connection to WAN server will be used.");
                }

                std::string str_msg("connection-request");
                BareNetworkString message(str_msg +
                    StringUtils::toString(m_server->getPrivatePort()));

                if (!NetworkConfig::m_disable_lan &&
                    m_server->getAddress().getIP() ==
                    STKHost::get()->getPublicAddress().getIP() &&
                    !STKHost::get()->isClientServer())
                {
                    // If use lan connection in wan server, send to all
                    // broadcast address
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
                else
                {
                    TransportAddress server_addr;
                    server_addr.setIP(m_server->getAddress().getIP());
                    // Direct socket always listens on server discovery port
                    server_addr.setPort(NetworkConfig::get()
                        ->getServerDiscoveryPort());
                    // Avoid possible packet loss, the connect to peer done by
                    // server will auto terminate if same peer from same port
                    // has connected already
                    for (int i = 0; i < 5; i++)
                    {
                        STKHost::get()->sendRawPacket(message, server_addr);
                        StkTime::sleep(1);
                    }
                }

            }
            m_state = EXITING;
            requestTerminate();
            break;
        }
        case EXITING:
            break;
    }
}   // asynchronousUpdate

