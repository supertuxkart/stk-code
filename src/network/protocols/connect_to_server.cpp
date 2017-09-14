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
#include "network/protocols/get_public_address.hpp"
#include "network/protocols/get_peer_address.hpp"
#include "network/protocols/hide_public_address.hpp"
#include "network/protocols/request_connection.hpp"
#include "network/protocols/ping_protocol.hpp"
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
    m_current_protocol = NULL;
    // In case of LAN we already have the server's and our ip address,
    // so we can immediately start requesting a connection.
    m_state = NetworkConfig::get()->isLAN() ? GOT_SERVER_ADDRESS : NONE;
}   // setup

// ----------------------------------------------------------------------------
/** Sets the server transport address. This is used in case of LAN networking,
 *  when we do not query the stk server and instead have the address from the
 *  LAN server directly.
 *  \param address Address of server to connect to.
 */
void ConnectToServer::setServerAddress(const TransportAddress &address)
{
}   // setServerAddress

// ----------------------------------------------------------------------------
void ConnectToServer::asynchronousUpdate()
{
    switch(m_state)
    {
        case NONE:
        {
            Log::info("ConnectToServer", "Protocol starting");
            // This protocol will write the public address of this
            // instance to STKHost.
            m_current_protocol = new GetPublicAddress(this);
            m_current_protocol->requestStart();
            // This protocol will be unpaused in the callback from 
            // GetPublicAddress
            requestPause();
            m_state = GETTING_SELF_ADDRESS;
            break;
        }
        case GETTING_SELF_ADDRESS:
        {
            delete m_current_protocol;   // delete GetPublicAddress
            m_current_protocol = NULL;

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
            delete m_current_protocol;
            m_current_protocol = NULL;
            Log::info("ConnectToServer", "Server's address known");

            // we're in the same lan (same public ip address) !!
            if (m_server_address.getIP() ==
                NetworkConfig::get()->getMyAddress().getIP())
            {
                Log::info("ConnectToServer",
                    "Server appears to be in the same LAN.");
            }
            m_state = REQUESTING_CONNECTION;
            m_current_protocol = new RequestConnection(m_server_id);
            m_current_protocol->requestStart();
            break;
        }
        case REQUESTING_CONNECTION:
            // In case of a LAN server, m_crrent_protocol is NULL
            if (!m_current_protocol ||
                m_current_protocol->getState() == PROTOCOL_STATE_TERMINATED)
            {
                delete m_current_protocol;
                m_current_protocol = NULL;
                // Server knows we want to connect
                Log::info("ConnectToServer", "Connection request made");
                if (m_server_address.getIP() == 0 ||
                    m_server_address.getPort() == 0  )
                { 
                    // server data not correct, hide address and stop
                    m_state = HIDING_ADDRESS;
                    Log::error("ConnectToServer", "Server address is %s",
                               m_server_address.toString().c_str());
                    m_current_protocol = new HidePublicAddress();
                    m_current_protocol->requestStart();
                    return;
                }
                if( ( !NetworkConfig::m_disable_lan && 
                       m_server_address.getIP() 
                         == NetworkConfig::get()->getMyAddress().getIP() )  ||
                      NetworkConfig::get()->isLAN()                            )
                {
                    // We're in the same lan (same public ip address).
                    // The state will change to CONNECTING
                    handleSameLAN();
                }
                else
                {
                    m_state = CONNECTING;
                    m_current_protocol = new PingProtocol(m_server_address, 2.0);
                    m_current_protocol->requestStart();
                }
            }
            break;
        case CONNECTING: // waiting the server to answer our connection
            {
                static double timer = 0;
                if (StkTime::getRealTime() > timer+5.0) // every 5 seconds
                {
                    STKHost::get()->connect(m_server_address);
                    timer = StkTime::getRealTime();
                    Log::info("ConnectToServer", "Trying to connect to %s",
                              m_server_address.toString().c_str());
                }
                break;
            }
        case CONNECTED:
        {
            Log::info("ConnectToServer", "Connected");
            if(m_current_protocol)
            {
                // Kill the ping protocol because we're connected
                m_current_protocol->requestTerminate();
            }
            delete m_current_protocol;
            m_current_protocol = NULL;
            // LAN networking does not use the stk server tables.
            if(NetworkConfig::get()->isWAN())
            {
                m_current_protocol = new HidePublicAddress();
                m_current_protocol->requestStart();
            }
            m_state = HIDING_ADDRESS;
            break;
        }
        case HIDING_ADDRESS:
            // Wait till we have hidden our address
            if (!m_current_protocol ||
                m_current_protocol->getState() == PROTOCOL_STATE_TERMINATED)
            {
                if(m_current_protocol)
                {
                    delete m_current_protocol;
                    m_current_protocol = NULL;
                    Log::info("ConnectToServer", "Address hidden");
                }
                m_state = DONE;
                // lobby room protocol if we're connected only
                if(STKHost::get()->getPeers()[0]->isConnected())
                {
                    ClientLobby *p = 
                        LobbyProtocol::create<ClientLobby>();
                    p->setAddress(m_server_address);
                    p->requestStart();
                }
            }
            break;
        case DONE:
            requestTerminate();
            m_state = EXITING;
            break;
        case EXITING:
            break;
    }
}   // asynchronousUpdate

 // ----------------------------------------------------------------------------
/** Called when the GetPeerAddress protocol terminates.
 */
void ConnectToServer::callback(Protocol *protocol)
{
    switch(m_state)
    {
        case GETTING_SELF_ADDRESS:
            // The GetPublicAddress protocol stores our address in
            // STKHost, so we only need to unpause this protocol
            requestUnpause();
            break;
        default:
            Log::error("ConnectToServer",
                       "Received unexpected callback while in state %d.",
                       m_state);
    }   // case m_state
}   // callback

// ----------------------------------------------------------------------------
/** Register this client with the STK server.
 */
void ConnectToServer::registerWithSTKServer()
{
    // Our public address is now known, register details with
    // STK server.
    const TransportAddress& addr = NetworkConfig::get()->getMyAddress();
    Online::XMLRequest *request  = new Online::XMLRequest();
    PlayerManager::setUserDetails(request, "set",
                                  Online::API::SERVER_PATH);
    request->addParameter("address", addr.getIP());
    request->addParameter("port", addr.getPort());
    request->addParameter("private_port",
                          NetworkConfig::get()->getClientPort());

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
    delete request;
    std::string success;

    if(result->get("success", &success) && success=="yes")
    {
        uint32_t ip;
        result->get("ip", &ip);
        m_server_address.setIP(ip);

        uint16_t port;
        // If we are using a LAN connection, we need the private (local) port
        if (m_server_address.getIP() == 
            NetworkConfig::get()->getMyAddress().getIP())
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
}   // handleQuickConnect

// ----------------------------------------------------------------------------
/** Called when the server is on the same LAN. It uses broadcast to
 *  find and conntect to the server.
 */
void ConnectToServer::handleSameLAN()
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
        Log::info("ConnectToServer", "LAN Server found : %s",
                   sender.toString().c_str());
#ifndef WIN32
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
        m_state = CONNECTING;
    }
}  // handleSameLAN

// ----------------------------------------------------------------------------

bool ConnectToServer::notifyEventAsynchronous(Event* event)
{
    if (event->getType() == EVENT_TYPE_CONNECTED)
    {
        Log::info("ConnectToServer", "The Connect To Server protocol has "
            "received an event notifying that he's connected to the peer.");
        m_state = CONNECTED; // we received a message, we are connected
        Server *server = ServersManager::get()->getJoinedServer();
    }
    return true;
}   // notifyEventAsynchronous

