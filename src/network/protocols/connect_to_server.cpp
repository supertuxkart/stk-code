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
#include "online/current_online_user.hpp"
#include "utils/time.hpp"
#include "utils/log.hpp"

// ----------------------------------------------------------------------------

ConnectToServer::ConnectToServer(uint32_t server_id) : 
    Protocol(NULL, PROTOCOL_CONNECTION)
{
    m_server_id = server_id;
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
        Log::info("ConnectToServer", "The Connect To Server protocol has \
                received an event notifying that he's connected to the peer.");
        m_state = CONNECTED; // we received a message, we are connected
    }
}

// ----------------------------------------------------------------------------

void ConnectToServer::setup()
{
    m_state = NONE;
    m_public_address.ip = 0;
    m_public_address.port = 0;
    m_server_address.ip = 0;
    m_server_address.port = 0;
    m_current_protocol_id = 0;
}

// ----------------------------------------------------------------------------

void ConnectToServer::update()
{
    switch(m_state)
    {
        case NONE:
        {
            m_current_protocol_id = m_listener->requestStart(new GetPublicAddress(&m_public_address));
            m_state = WAITING_SELF_ADDRESS;
            break;
        }
        case WAITING_SELF_ADDRESS:
            if (m_listener->getProtocolState(m_current_protocol_id) 
            == PROTOCOL_STATE_TERMINATED) // now we know the public addr
            {
                m_state = SELF_ADDRESS_KNOWN;
                NetworkManager::getInstance()->setPublicAddress(m_public_address); // set our public address
                m_current_protocol_id = m_listener->requestStart(new GetPeerAddress(m_server_id, &m_server_address));
            }
            break;
        case SELF_ADDRESS_KNOWN:
            if (m_listener->getProtocolState(m_current_protocol_id) 
            == PROTOCOL_STATE_TERMINATED) // now we have the server's address
            {
                m_state = PEER_ADDRESS_KNOWN;
                m_current_protocol_id = m_listener->requestStart(new ShowPublicAddress());
            }
            break;
        case PEER_ADDRESS_KNOWN:
            if (m_listener->getProtocolState(m_current_protocol_id) 
            == PROTOCOL_STATE_TERMINATED) // now our public address is public
            {
                m_state = SELF_ADDRESS_SHOWN;
                m_current_protocol_id = m_listener->requestStart(new RequestConnection(m_server_id));
            }
            break;
        case SELF_ADDRESS_SHOWN:
            if (m_listener->getProtocolState(m_current_protocol_id) 
            == PROTOCOL_STATE_TERMINATED) // we have put a request to access the server
            {
                m_state = REQUEST_DONE;
                m_current_protocol_id = m_listener->requestStart(new PingProtocol(m_server_address, 0.5));
            }
            break;
        case REQUEST_DONE:
            if (m_listener->getProtocolState(m_current_protocol_id) 
            == PROTOCOL_STATE_TERMINATED) // we have put a request to access the server
            {
                m_state = CONNECTING;
            }
            break;
        case CONNECTING: // waiting the server to answer our connection
            break;
        case CONNECTED:
        {
            m_listener->requestTerminate( m_listener->getProtocol(m_current_protocol_id)); // kill the ping protocol because we're connected
            m_current_protocol_id = m_listener->requestStart(new HidePublicAddress());
            m_state = HIDING_ADDRESS;
            break;
        }
        case HIDING_ADDRESS:
            if (m_listener->getProtocolState(m_current_protocol_id) 
            == PROTOCOL_STATE_TERMINATED) // we have hidden our address
            {
                m_state = DONE;
            }
            break;
        case DONE:
            m_listener->requestTerminate(this);
            break;
    }
}

// ----------------------------------------------------------------------------

