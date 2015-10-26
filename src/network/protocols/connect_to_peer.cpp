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

#include "network/protocols/connect_to_peer.hpp"

#include "network/client_network_manager.hpp"
#include "network/event.hpp"
#include "network/protocols/get_public_address.hpp"
#include "network/protocols/get_peer_address.hpp"
#include "network/protocols/show_public_address.hpp"
#include "network/protocols/hide_public_address.hpp"
#include "network/protocols/request_connection.hpp"
#include "network/protocols/ping_protocol.hpp"
#include "network/protocol_manager.hpp"
#include "network/stk_host.hpp"
#include "utils/time.hpp"
#include "utils/log.hpp"

// ----------------------------------------------------------------------------

ConnectToPeer::ConnectToPeer(uint32_t peer_id) 
             : Protocol(NULL, PROTOCOL_CONNECTION)
{
    m_peer_id = peer_id;
    m_state = NONE;
}   // ConnectToPeer

// ----------------------------------------------------------------------------

ConnectToPeer::~ConnectToPeer()
{
}   // ~ConnectToPeer

// ----------------------------------------------------------------------------

bool ConnectToPeer::notifyEventAsynchronous(Event* event)
{
    if (event->getType() == EVENT_TYPE_CONNECTED)
    {
        Log::debug("ConnectToPeer", "Received event notifying peer connection.");
        m_state = CONNECTED; // we received a message, we are connected
    }
    return true;
}   // notifyEventAsynchronous

// ----------------------------------------------------------------------------

void ConnectToPeer::setup()
{
    m_public_address.clear();
    m_peer_address.clear();
    m_current_protocol = NULL;
    m_state            = NONE;
}   // setup

// ----------------------------------------------------------------------------

void ConnectToPeer::asynchronousUpdate()
{
    switch(m_state)
    {
        case NONE:
        {
            m_current_protocol = new GetPeerAddress(m_peer_id, &m_peer_address); 
            ProtocolManager::getInstance()->requestStart(m_current_protocol);
            m_state = WAITING_PEER_ADDRESS;
            break;
        }
        case WAITING_PEER_ADDRESS:
            // Wait till we know the peer address
            if (m_current_protocol->getState()== PROTOCOL_STATE_TERMINATED)
            {
                if (m_peer_address.getIP() != 0 && m_peer_address.getPort() != 0)
                {
                    // we're in the same lan (same public ip address) !!
                    if (m_peer_address.getIP() == STKHost::get()->getPublicAddress().getIP())
                    {
                        // just send a broadcast packet with the string aloha_stk inside, the client will know our ip address and will connect
                        TransportAddress broadcast_address;
                        broadcast_address.setIP(-1); // 255.255.255.255
                        broadcast_address.setPort(m_peer_address.getPort()); // 0b10101100000101101101111111111111; // for test
                        char data[] = "aloha_stk\0";
                        STKHost::get()->sendRawPacket((uint8_t*)(data), 10, broadcast_address);
                        Log::info("ConnectToPeer", "Broadcast aloha sent.");
                        StkTime::sleep(1);
                        broadcast_address.setIP(0x7f000001); // 127.0.0.1 (localhost)
                        broadcast_address.setPort(m_peer_address.getPort());
                        STKHost::get()->sendRawPacket((uint8_t*)(data), 10, broadcast_address);
                        Log::info("ConnectToPeer", "Broadcast aloha to self.");
                    }
                    else
                    {
                        m_current_protocol = new PingProtocol(m_peer_address, 2.0);
                        ProtocolManager::getInstance()->requestStart(m_current_protocol);
                    }
                    m_state = CONNECTING;
                }
                else
                {
                    Log::error("ConnectToPeer", "The peer you want to connect to has hidden his address.");
                    m_state = DONE;
                }
            }
            break;
        case CONNECTING: // waiting the peer to connect
            break;
        case CONNECTED:
        {
            // the ping protocol is there for NAT traversal (disabled when connecting to LAN peer)
            if (m_peer_address != STKHost::get()->getPublicAddress())
            {
                // Kill the ping protocol because we're connected
                ProtocolManager::getInstance()
                               ->requestTerminate(m_current_protocol);
            }
            m_state = DONE;
            break;
        }
        case DONE:
            m_state = EXITING;
            ProtocolManager::getInstance()->requestTerminate(this);
            break;
        case EXITING:
            break;
    }
}   // asynchronousUpdate

// ----------------------------------------------------------------------------

