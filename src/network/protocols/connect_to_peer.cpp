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

#include "network/event.hpp"
#include "network/network_config.hpp"
#include "network/protocols/get_peer_address.hpp"
#include "network/protocols/request_connection.hpp"
#include "network/protocol_manager.hpp"
#include "network/stk_host.hpp"
#include "utils/time.hpp"
#include "utils/log.hpp"

// ----------------------------------------------------------------------------
/** Constructor for a WAN request. In this case we need to get the peer's
 *  ip address first. 
 *  \param peer_id ID of the peer in the stk client table.
 */
ConnectToPeer::ConnectToPeer(uint32_t peer_id)  : Protocol(PROTOCOL_CONNECTION)
{
    m_peer_address.clear();
    m_peer_id          = peer_id;
    m_state            = NONE;
}   // ConnectToPeer(peer_id)

// ----------------------------------------------------------------------------
/** Constructor for a LAN connection.
 *  \param address The address to connect to.
 */
ConnectToPeer::ConnectToPeer(const TransportAddress &address)
             : Protocol(PROTOCOL_CONNECTION)
{
    m_peer_address = address;
    // We don't need to find the peer address, so we can start
    // with the state when we found the peer address.
    m_state            = WAIT_FOR_CONNECTION;
}   // ConnectToPeers(TransportAddress)

// ----------------------------------------------------------------------------

ConnectToPeer::~ConnectToPeer()
{
}   // ~ConnectToPeer

// ----------------------------------------------------------------------------
/** Simple finite state machine: Start a GetPeerAddress protocol. Once the
 *  result has been received, start a ping protocol (hoping to be able
 *  to connect to the NAT peer using its public port). The ping protocol
 *  should make sure that the peer's firewall still lets packages through
 *  by the time the actual game starts.
 */
void ConnectToPeer::asynchronousUpdate()
{
    switch(m_state)
    {
        case NONE:
        {
            m_current_protocol = std::make_shared<GetPeerAddress>(m_peer_id);
            m_current_protocol->requestStart();
            m_state = RECEIVED_PEER_ADDRESS;
            break;
        }
        case RECEIVED_PEER_ADDRESS:
        {
            // Wait until we have peer address
            auto get_peer_address =
                std::dynamic_pointer_cast<GetPeerAddress>(m_current_protocol);
            assert(get_peer_address);
            if (get_peer_address->getAddress().isUnset())
                return;
            m_peer_address = get_peer_address->getAddress();
            m_current_protocol = nullptr;
            if (m_peer_address.isUnset())
            {
                Log::error("ConnectToPeer",
                    "The peer you want to connect to has hidden his address.");
                m_state = DONE;
                break;
            }

            m_state = WAIT_FOR_CONNECTION;
            m_timer = 0.0;
            break;
        }
        case WAIT_FOR_CONNECTION:
        {
            if (STKHost::get()->peerExists(m_peer_address))
            {
                Log::info("ConnectToPeer",
                    "Peer %s has established a connection.",
                    m_peer_address.toString().c_str());
                m_state = DONE;
                break;
            }
            // Each 2 second for a ping or broadcast
            if (StkTime::getRealTime() > m_timer + 2.0)
            {
                m_timer = StkTime::getRealTime();
                // Send a broadcast packet with the string aloha_stk inside,
                // the client will know our ip address and will connect
                // The wan remote should already start its ping message to us now
                // so we can send packet directly to it.
                TransportAddress broadcast_address;
                broadcast_address = m_peer_address;

                BareNetworkString aloha(std::string("aloha_stk"));
                STKHost::get()->sendRawPacket(aloha, broadcast_address);
                Log::verbose("ConnectToPeer", "Broadcast aloha sent.");
                StkTime::sleep(1);

                if (m_peer_address.isPublicAddressLocalhost())
                {
                    broadcast_address.setIP(0x7f000001); // 127.0.0.1 (localhost)
                    broadcast_address.setPort(m_peer_address.getPort());
                    STKHost::get()->sendRawPacket(aloha, broadcast_address);
                    Log::verbose("ConnectToPeer", "Broadcast aloha to self.");
                }

                // 20 seconds timeout
                if (m_tried_connection++ > 10)
                {
                    // Not much we can do about if we don't receive the client
                    // connection - it could have stopped, lost network, ...
                    // Terminate this protocol.
                    Log::warn("ConnectToPeer", "Time out trying to connect to %s",
                            m_peer_address.toString().c_str());
                    m_state = DONE;
                }
            }
            break;
        }
        case DONE:
            m_state = EXITING;
            requestTerminate();
            break;
        case EXITING:
            break;
    }
}   // asynchronousUpdate
