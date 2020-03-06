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
#include "network/network.hpp"
#include "network/network_string.hpp"
#include "network/stk_host.hpp"
#include "utils/time.hpp"
#include "utils/log.hpp"

// ----------------------------------------------------------------------------
/** Constructor for peer address.
 *  \param address The address to connect to.
 */
ConnectToPeer::ConnectToPeer(const SocketAddress &address)
             : Protocol(PROTOCOL_CONNECTION)
{
    m_peer_address = address;
    m_state = WAIT_FOR_CONNECTION;
}   // ConnectToPeer

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
            if (StkTime::getMonoTimeMs() > m_timer + 2000)
            {
                m_timer = StkTime::getMonoTimeMs();
                // Send a broadcast packet with the string aloha_stk inside,
                // the client will use enet intercept to discover if server
                // address or port is different from stk addons database.
                // (Happens if there is firewall in between)
                BareNetworkString aloha("aloha-stk");

                // Enet packet will not have 0xFFFF for first 2 bytes
                // We use the feature to distinguish between the enet packets
                // and this aloha
                aloha.getBuffer().insert(aloha.getBuffer().begin(), 2, 0xFF);

                STKHost::get()->sendRawPacket(aloha, m_peer_address);
                Log::debug("ConnectToPeer", "Broadcast aloha sent.");
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
