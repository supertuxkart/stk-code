//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2013 SuperTuxKart-Team
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

#include "network/client_network_manager.hpp"
#include "network/protocols/get_public_address.hpp"
#include "network/protocols/get_peer_address.hpp"
#include "network/protocols/show_public_address.hpp"
#include "network/protocols/hide_public_address.hpp"
#include "network/protocols/request_connection.hpp"
#include "network/protocols/ping_protocol.hpp"
#include "network/protocols/quick_join_protocol.hpp"
#include "network/protocols/client_lobby_room_protocol.hpp"
#include "online/current_user.hpp"
#include "utils/time.hpp"
#include "utils/log.hpp"

// ----------------------------------------------------------------------------

ConnectToServer::ConnectToServer() :
    Protocol(NULL, PROTOCOL_CONNECTION)
{
    m_server_id = 0;
    m_quick_join = true;
    m_state = NONE;
}

// ----------------------------------------------------------------------------

ConnectToServer::ConnectToServer(uint32_t server_id, uint32_t host_id) :
    Protocol(NULL, PROTOCOL_CONNECTION)
{
    m_server_id = server_id;
    m_host_id = host_id;
    m_quick_join = false;
    m_state = NONE;
}

// ----------------------------------------------------------------------------

ConnectToServer::~ConnectToServer()
{
}

// ----------------------------------------------------------------------------

void ConnectToServer::notifyEvent(Event* event)
{
    if (event->type == EVENT_TYPE_CONNECTED)
    {
        Log::info("ConnectToServer", "The Connect To Server protocol has "
                "received an event notifying that he's connected to the peer.");
        m_state = CONNECTED; // we received a message, we are connected
    }
}

// ----------------------------------------------------------------------------

void ConnectToServer::setup()
{
    Log::info("ConnectToServer", "SETUPP");
    m_state = NONE;
    m_public_address.ip = 0;
    m_public_address.port = 0;
    m_server_address.ip = 0;
    m_server_address.port = 0;
    m_current_protocol_id = 0;
}

// ----------------------------------------------------------------------------

void ConnectToServer::asynchronousUpdate()
{
    switch(m_state)
    {
        case NONE:
        {
            Log::info("ConnectToServer", "Protocol starting");
            m_current_protocol_id = m_listener->requestStart(new GetPublicAddress(&m_public_address));
            m_state = GETTING_SELF_ADDRESS;
            break;
        }
        case GETTING_SELF_ADDRESS:
            if (m_listener->getProtocolState(m_current_protocol_id)
            == PROTOCOL_STATE_TERMINATED) // now we know the public addr
            {
                m_state = SHOWING_SELF_ADDRESS;
                NetworkManager::getInstance()->setPublicAddress(m_public_address); // set our public address
                m_current_protocol_id = m_listener->requestStart(new ShowPublicAddress());
                Log::info("ConnectToServer", "Public address known");
                /*
                if (m_quick_join)
                    m_current_protocol_id = m_listener->requestStart(new QuickJoinProtocol(&m_server_address, &m_server_id));
                else
                    m_current_protocol_id = m_listener->requestStart(new GetPeerAddress(m_server_id, &m_server_address));*/
            }
            break;
        case SHOWING_SELF_ADDRESS:
            if (m_listener->getProtocolState(m_current_protocol_id)
            == PROTOCOL_STATE_TERMINATED) // now our public address is in the database
            {
                Log::info("ConnectToServer", "Public address shown");
                if (m_quick_join)
                {
                    m_current_protocol_id = m_listener->requestStart(new QuickJoinProtocol(&m_server_address, &m_server_id));
                    m_state = REQUESTING_CONNECTION;
                }
                else
                {
                    m_current_protocol_id = m_listener->requestStart(new GetPeerAddress(m_host_id, &m_server_address));
                    m_state = GETTING_SERVER_ADDRESS;
                }
            }
            break;
        case GETTING_SERVER_ADDRESS:
            if (m_listener->getProtocolState(m_current_protocol_id)
            == PROTOCOL_STATE_TERMINATED) // we know the server address
            {
                m_state = REQUESTING_CONNECTION;
                m_current_protocol_id = m_listener->requestStart(new RequestConnection(m_server_id));
                Log::info("ConnectToServer", "Server's address known");
            }
            break;
        case REQUESTING_CONNECTION:
            if (m_listener->getProtocolState(m_current_protocol_id)
            == PROTOCOL_STATE_TERMINATED) // server knows we wanna connect
            {
                Log::info("ConnectToServer", "Connection request made");
                if (m_server_address.ip == 0 || m_server_address.port == 0)
                { // server data not correct, hide address and stop
                    m_state = HIDING_ADDRESS;
                    m_current_protocol_id = m_listener->requestStart(new HidePublicAddress());
                    return;
                }
                m_state = CONNECTING;
                m_current_protocol_id = m_listener->requestStart(new PingProtocol(m_server_address, 2.0));
            }
            break;
        case CONNECTING: // waiting the server to answer our connection
            {
                static double timer = 0;
                if (Time::getRealTime() > timer+5.0) // every 5 seconds
                {
                    timer = Time::getRealTime();
                    NetworkManager::getInstance()->connect(m_server_address);
                }
                break;
            }
        case CONNECTED:
        {
            Log::info("ConnectToServer", "Connected");
            m_listener->requestTerminate( m_listener->getProtocol(m_current_protocol_id)); // kill the ping protocol because we're connected
            m_current_protocol_id = m_listener->requestStart(new HidePublicAddress());
            ClientNetworkManager::getInstance()->setConnected(true);
            m_state = HIDING_ADDRESS;
            break;
        }
        case HIDING_ADDRESS:
            if (m_listener->getProtocolState(m_current_protocol_id)
            == PROTOCOL_STATE_TERMINATED) // we have hidden our address
            {
                Log::info("ConnectToServer", "Address hidden");
                m_state = DONE;
                if (ClientNetworkManager::getInstance()->isConnected()) // lobby room protocol if we're connected only
                    m_listener->requestStart(new ClientLobbyRoomProtocol(m_server_address));
            }
            break;
        case DONE:
            m_listener->requestTerminate(this);
            break;
    }
}

// ----------------------------------------------------------------------------

