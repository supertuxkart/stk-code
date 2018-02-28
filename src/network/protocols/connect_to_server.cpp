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
    m_server = ServersManager::get()->getServerByID(m_server_id);
    m_server_address.copy(m_server->getAddress());
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
    // In case of LAN or client-server we already have the server's
    // and our ip address, so we can immediately start requesting a connection.
    m_state = (NetworkConfig::get()->isLAN() ||
        STKHost::get()->isClientServer()) ?
        GOT_SERVER_ADDRESS : SET_PUBLIC_ADDRESS;
}   // setup

// ----------------------------------------------------------------------------
void ConnectToServer::asynchronousUpdate()
{
    switch(m_state.load())
    {
        case SET_PUBLIC_ADDRESS:
        {
            STKHost::get()->setPublicAddress();
            // Set to DONE will stop STKHost is not connected
            m_state = STKHost::get()->getPublicAddress().isUnset() ?
                DONE : REGISTER_SELF_ADDRESS;
        }
        break;
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
            m_timer = 0.0;
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
            if (m_tried_connection++ > 10)
            {
                Log::error("ConnectToServer", "Timeout waiting for aloha");
                m_state = NetworkConfig::get()->isWAN() ?
                    HIDING_ADDRESS : DONE;
            }
            if ((!NetworkConfig::m_disable_lan && 
                m_server_address.getIP() ==
                STKHost::get()->getPublicAddress().getIP()) ||
                (NetworkConfig::get()->isLAN() ||
                STKHost::get()->isClientServer()))
            {
                // We're in the same lan (same public ip address).
                // The state will change to CONNECTING
                waitingAloha(false/*is_wan*/);
            }
            else
            {
                // Send a 1-byte datagram,  the remote host can simply ignore
                // this datagram, to keep the port open (2 second each)
                if (StkTime::getRealTime() > m_timer + 2.0)
                {
                    m_timer = StkTime::getRealTime();
                    BareNetworkString data;
                    data.addUInt8(0);
                    STKHost::get()->sendRawPacket(data, m_server_address);
                }
                waitingAloha(true/*is_wan*/);
            }
            break;
        case CONNECTING: // waiting the server to answer our connection
        {
            // Every 5 seconds
            if (StkTime::getRealTime() > m_timer + 5.0)
            {
                m_timer = StkTime::getRealTime();
                STKHost::get()->connect(m_server_address);
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
            if (NetworkConfig::get()->isWAN() &&
                !STKHost::get()->isClientServer())
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
            break;
        case DONE:
        case EXITING:
            break;
    }
}   // asynchronousUpdate

// ----------------------------------------------------------------------------
void ConnectToServer::update(float dt)
{
    switch(m_state.load())
    {
        case DONE:
        {
            // lobby room protocol if we're connected only
            if (STKHost::get()->getPeerCount() > 0 &&
                STKHost::get()->getPeers()[0]->isConnected() &&
                !m_server_address.isUnset())
            {
                // Let main thread create ClientLobby for better
                // synchronization with GUI
                auto cl = LobbyProtocol::create<ClientLobby>();
                cl->setAddress(m_server_address);
                cl->requestStart();
            }
            if (STKHost::get()->getPeerCount() == 0)
            {
                // Shutdown STKHost (go back to online menu too)
                STKHost::get()->setErrorMessage(
                    _("Cannot connect to server %s.",  m_server->getName()));
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
/** Register this client with the STK server.
 */
void ConnectToServer::registerWithSTKServer()
{
    // Our public address is now known, register details with
    // STK server.
    const TransportAddress& addr = STKHost::get()->getPublicAddress();
    Online::XMLRequest *request  = new Online::XMLRequest();
    NetworkConfig::get()->setUserDetails(request, "set");
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
        irr::core::stringc error(request->getInfo().c_str());
        Log::error("ConnectToServer", "Failed to register client address: %s",
            error.c_str());
        m_state = DONE;
    }
    delete request;

}   // registerWithSTKServer

// ----------------------------------------------------------------------------
/** Called to request a quick connect from the STK server.
 */
void ConnectToServer::handleQuickConnect()
{
    Online::XMLRequest *request = new Online::XMLRequest();
    NetworkConfig::get()->setUserDetails(request, "quick-join");
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
        if (!is_wan)
        {
            if (sender.isPublicAddressLAN())
                sender.setIP(0x7f000001); // 127.0.0.1
        }
        m_server_address.copy(sender);
        m_state = CONNECTING;
        // Reset timer for next usage
        m_timer = 0.0;
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

