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

#include "network/protocols/connect_to_peer.hpp"

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

ConnectToPeer::ConnectToPeer(uint32_t peer_id) :
    Protocol(NULL, PROTOCOL_CONNECTION)
{
    m_peer_id = peer_id;
    m_state = NONE;
}

// ----------------------------------------------------------------------------

ConnectToPeer::~ConnectToPeer()
{
}

// ----------------------------------------------------------------------------

void ConnectToPeer::notifyEvent(Event* event)
{
    if (event->type == EVENT_TYPE_CONNECTED)
    {
        Log::debug("ConnectToPeer", "Received event notifying peer connection.");
        m_state = CONNECTED; // we received a message, we are connected
    }
}

// ----------------------------------------------------------------------------

void ConnectToPeer::setup()
{
    m_state = NONE;
    m_public_address.ip = 0;
    m_public_address.port = 0;
    m_peer_address.ip = 0;
    m_peer_address.port = 0;
    m_current_protocol_id = 0;
}

// ----------------------------------------------------------------------------

void ConnectToPeer::asynchronousUpdate()
{
    switch(m_state)
    {
        case NONE:
        {
            m_current_protocol_id = m_listener->requestStart(new GetPeerAddress(m_peer_id, &m_peer_address));
            m_state = WAITING_PEER_ADDRESS;
            break;
        }
        case WAITING_PEER_ADDRESS:
            if (m_listener->getProtocolState(m_current_protocol_id)
            == PROTOCOL_STATE_TERMINATED) // we know the peer address
            {
                if (m_peer_address.ip != 0 && m_peer_address.port != 0)
                {
                    m_state = CONNECTING;
                    m_current_protocol_id = m_listener->requestStart(new PingProtocol(m_peer_address, 2.0));
                }
                else
                {
                    Log::error("ConnectToPeer", "The peer you want to connect to has hidden his address.");
                    m_state = DONE;
                }
            }
            break;
        case CONNECTING: // waiting the peer to connect
        case CONNECTED:
        {
            m_listener->requestTerminate( m_listener->getProtocol(m_current_protocol_id)); // kill the ping protocol because we're connected
            m_state = DONE;
            break;
        }
        case DONE:
            m_state = EXITING;
            m_listener->requestTerminate(this);
            break;
        case EXITING:
            break;
    }
}

// ----------------------------------------------------------------------------

