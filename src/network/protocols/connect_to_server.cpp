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

#include "config/player_manager.hpp"
#include "network/event.hpp"
#include "network/network_config.hpp"
#include "network/protocols/get_peer_address.hpp"
#include "network/protocols/hide_public_address.hpp"
#include "network/protocols/request_connection.hpp"
#include "network/protocols/client_lobby.hpp"
#include "network/protocol_manager.hpp"
#include "network/servers_manager.hpp"
#include "network/stk_host.hpp"
#include "network/stk_peer.hpp"
#include "utils/time.hpp"
#include "utils/log.hpp"

#ifdef WIN32
#  include <iphlpapi.h>
#else
#include <ifaddrs.h>
#endif

// ----------------------------------------------------------------------------
/** Connects to a server. This is the quick connect constructor, which 
 *  will pick a server randomly.
 */
ConnectToServer::ConnectToServer() : Protocol(PROTOCOL_CONNECTION)
{
    m_server_id  = 0;
    m_host_id    = 0;
    m_quick_join = true;
    m_server_address.clear();
    setHandleConnections(true);
}   // ConnectToServer()

// ----------------------------------------------------------------------------
/** Specify server to connect to.
 *  \param server_id Id of server to connect to.
 *  \param host_id Id of host.
 */
ConnectToServer::ConnectToServer(uint32_t server_id, uint32_t host_id)
               : Protocol(PROTOCOL_CONNECTION)
{
    m_server_id  = server_id;
    m_host_id    = host_id;
    m_quick_join = false;
    const Server *server = ServersManager::get()->getServerByID(server_id);
    m_server_address.copy(server->getAddress());
    setHandleConnections(true);
}   // ConnectToServer(server, host)

// ----------------------------------------------------------------------------
/** Destructor. 
 */
ConnectToServer::~ConnectToServer()
{
}   // ~ConnectToServer

// ----------------------------------------------------------------------------
/** Initialise the protocol.
 */
void ConnectToServer::setup()
{
    Log::info("ConnectToServer", "SETUP");
    m_current_protocol.reset();
    // In case of LAN we already have the server's and our ip address,
    // so we can immediately start requesting a connection.
    m_state = NetworkConfig::get()->isLAN() ? GOT_SERVER_ADDRESS :
        REGISTER_SELF_ADDRESS;
    
}   // setup

// ----------------------------------------------------------------------------
void ConnectToServer::asynchronousUpdate()
{
    switch(m_state)
    {
        case REGISTER_SELF_ADDRESS:
        {
            registerWithSTKServer();  // Register us with STK server

            if (m_quick_join)
            {
                handleQuickConnect();
                // Quick connect will give us the server details,
                // so we can immediately try to connect to the server
                m_state = REQUESTING_CONNECTION;
            }
            else
            {
                m_state = GOT_SERVER_ADDRESS;
            }
        }
        break;
        case GOT_SERVER_ADDRESS:
        {
            assert(!m_quick_join);
            Log::info("ConnectToServer", "Server's address known");
            m_state = REQUESTING_CONNECTION;
            auto request_connection =
                std::make_shared<RequestConnection>(m_server_id);
            request_connection->requestStart();
            m_current_protocol = request_connection;
            // Reset timer for next usage
            resetTimer();
            break;
        }
        case REQUESTING_CONNECTION:
            if (!m_current_protocol.expired())
            {
                return;
            }

            // Server knows we want to connect
            Log::info("ConnectToServer", "Connection request made");
            if (m_server_address.isUnset())
            {
                // server data not correct, hide address and stop
                m_state = HIDING_ADDRESS;
                Log::error("ConnectToServer", "Server address is %s",
                        m_server_address.toString().c_str());
                auto hide_address = std::make_shared<HidePublicAddress>();
                hide_address->requestStart();
                m_current_protocol = hide_address;
                return;
            }
            if (m_tried_connection++ > 5)
            {
                Log::error("ConnectToServer", "Timeout waiting for aloha");
                m_state = NetworkConfig::get()->isWAN() ?
                    HIDING_ADDRESS : DONE;
            }
            if ((!NetworkConfig::m_disable_lan && 
                m_server_address.getIP() ==
                STKHost::get()->getPublicAddress().getIP()) ||
                NetworkConfig::get()->isLAN())
            {
                // We're in the same lan (same public ip address).
                // The state will change to CONNECTING
                waitingAloha(false/*is_wan*/);
            }
            else
            {
                // Send a 1-byte datagram,  the remote host can simply ignore
                // this datagram, to keep the port open (2 second each)
                if (m_timer > m_timer + std::chrono::seconds(2))
                {
                    resetTimer();
                    BareNetworkString data;
                    data.addUInt8(0);
                    STKHost::get()->sendRawPacket(data, m_server_address);
                }
                waitingAloha(true/*is_wan*/);
            }
            break;
        case CONNECTING: // waiting the server to answer our connection
        {
            if (m_timer > m_timer + std::chrono::seconds(5)) // every 5 seconds
            {
                STKHost::get()->connect(m_server_address);
                resetTimer();
                Log::info("ConnectToServer", "Trying to connect to %s",
                    m_server_address.toString().c_str());
                if (m_tried_connection++ > 3)
                {
                    Log::error("ConnectToServer", "Timeout connect to %s",
                        m_server_address.toString().c_str());
                    m_state = NetworkConfig::get()->isWAN() ?
                        HIDING_ADDRESS : DONE;
                }
            }
            break;
        }
        case CONNECTED:
        {
            Log::info("ConnectToServer", "Connected");
            // LAN networking does not use the stk server tables.
            if (NetworkConfig::get()->isWAN())
            {
                auto hide_address = std::make_shared<HidePublicAddress>();
                hide_address->requestStart();
                m_current_protocol = hide_address;
            }
            m_state = HIDING_ADDRESS;
            break;
        }
        case HIDING_ADDRESS:
            // Wait till we have hidden our address
            if (!m_current_protocol.expired())
            {
                return;
            }
            m_state = DONE;
            // lobby room protocol if we're connected only
            if (STKHost::get()->getPeers()[0]->isConnected() &&
                !m_server_address.isUnset())
            {
                auto cl = LobbyProtocol::create<ClientLobby>();
                cl->setAddress(m_server_address);
                cl->requestStart();
            }
            break;
        case DONE:
            requestTerminate();
            m_state = EXITING;
            if (STKHost::get()->getPeerCount() == 0)
            {
                // Shutdown STKHost (go back to online menu too)
                STKHost::get()->requestShutdown();
            }
            break;
        case EXITING:
            break;
    }
}   // asynchronousUpdate

// ----------------------------------------------------------------------------
/** Register this client with the STK server.
 */
void ConnectToServer::registerWithSTKServer()
{
    // Our public address is now known, register details with
    // STK server.
    const TransportAddress& addr = STKHost::get()->getPublicAddress();
    Online::XMLRequest *request  = new Online::XMLRequest();
    PlayerManager::setUserDetails(request, "set",
                                  Online::API::SERVER_PATH);
    request->addParameter("address", addr.getIP());
    request->addParameter("port", addr.getPort());
    request->addParameter("private_port", STKHost::get()->getPrivatePort());

    Log::info("ConnectToServer", "Registering addr %s",
              addr.toString().c_str());

    // This can be done blocking: till we are registered with the
    // stk server, there is no need to to react to any other 
    // network requests
    request->executeNow();

    const XMLNode * result = request->getXMLData();

    std::string success;
    if(result->get("success", &success) && success == "yes")
    {
        Log::debug("ConnectToServer", "Address registered successfully.");
    }
    else
    {
        Log::error("ConnectToServer", "Failed to register address.");
    }
    delete request;

}   // registerWithSTKServer

// ----------------------------------------------------------------------------
/** Called to request a quick connect from the STK server.
 */
void ConnectToServer::handleQuickConnect()
{
    Online::XMLRequest *request = new Online::XMLRequest();
    PlayerManager::setUserDetails(request, "quick-join",
                                  Online::API::SERVER_PATH);
    request->executeNow();

    const XMLNode * result = request->getXMLData();
    std::string success;

    if(result->get("success", &success) && success=="yes")
    {
        uint32_t ip;
        result->get("ip", &ip);
        m_server_address.setIP(ip);

        uint16_t port;
        // If we are using a LAN connection, we need the private (local) port
        if (m_server_address.getIP() == 
            STKHost::get()->getPublicAddress().getIP())
        {
            result->get("private_port", &port);
        }
        else
            result->get("port", &port);

        m_server_address.setPort(port);

        Log::debug("GetPeerAddress", "Address gotten successfully.");
    }
    else
    {
        Log::error("GetPeerAddress", "Failed to get address.");
    }
    delete request;
}   // handleQuickConnect

// ----------------------------------------------------------------------------
/** Called when the server is on the same LAN. It uses broadcast to
 *  find and conntect to the server. For WAN game, it makes sure server recieve
 *  request from stk addons first before continuing.
 */
void ConnectToServer::waitingAloha(bool is_wan)
{
    // just send a broadcast packet, the client will know our 
    // ip address and will connect
    STKHost* host = STKHost::get();
    host->stopListening(); // stop the listening

    Log::info("ConnectToServer", "Waiting broadcast message.");

    TransportAddress sender;
    // get the sender
    const int LEN=256;
    char buffer[LEN];
    int len = host->receiveRawPacket(buffer, LEN, &sender, 2000);
    if(len<0)
    {
        Log::warn("ConnectToServer",
                  "Received invalid server information message.");
        return;
    }

    BareNetworkString message(buffer, len);
    std::string received;
    message.decodeString(&received);
    host->startListening(); // start listening again
    std::string aloha("aloha_stk");
    if (received==aloha)
    {
        Log::info("ConnectToServer", "Server found : %s",
                   sender.toString().c_str());
#ifndef WIN32
        if (!is_wan)
        {
            // just check if the ip is ours : if so, 
            // then just use localhost (127.0.0.1)
            struct ifaddrs *ifap, *ifa;
            struct sockaddr_in *sa;
            getifaddrs(&ifap); // get the info
            for (ifa = ifap; ifa; ifa = ifa->ifa_next)
            {
                if (ifa->ifa_addr->sa_family == AF_INET)
                {
                    sa = (struct sockaddr_in *) ifa->ifa_addr;

                    // This interface is ours
                    if (ntohl(sa->sin_addr.s_addr) == sender.getIP())
                        sender.setIP(0x7f000001); // 127.0.0.1
                }
            }
            freeifaddrs(ifap);
#else
            // Query the list of all IP addresses on the local host
            // First call to GetIpAddrTable with 0 bytes buffer
            // will return insufficient buffer error, and size
            // will contain the number of bytes needed for all
            // data. Repeat the process of querying the size
            // using GetIpAddrTable in a while loop since it
            // can happen that an interface comes online between
            // the previous call to GetIpAddrTable and the next
            // call.
            MIB_IPADDRTABLE *table = NULL;
            unsigned long size = 0;
            int error = GetIpAddrTable(table, &size, 0);
            // Also add a count to limit the while loop - in
            // case that something strange is going on.
            int count = 0;
            while (error == ERROR_INSUFFICIENT_BUFFER && count < 10)
            {
                delete[] table;   // deleting NULL is legal
                table = (MIB_IPADDRTABLE*)new char[size];
                error = GetIpAddrTable(table, &size, 0);
                count++;
            }   // while insufficient buffer
            for (unsigned int i = 0; i < table->dwNumEntries; i++)
            {
                unsigned int ip = ntohl(table->table[i].dwAddr);
                if (sender.getIP() == ip) // this interface is ours
                {
                    sender.setIP(0x7f000001); // 127.0.0.1
                    break;
                }
            }
            delete[] table;
#endif
            m_server_address.copy(sender);
        }
        m_state = CONNECTING;
        // Reset timer for next usage
        resetTimer();
        m_tried_connection = 0;
    }
}  // waitingAloha

// ----------------------------------------------------------------------------

bool ConnectToServer::notifyEventAsynchronous(Event* event)
{
    if (event->getType() == EVENT_TYPE_CONNECTED)
    {
        Log::info("ConnectToServer", "The Connect To Server protocol has "
            "received an event notifying that he's connected to the peer.");
        m_state = CONNECTED; // we received a message, we are connected
    }
    return true;
}   // notifyEventAsynchronous

