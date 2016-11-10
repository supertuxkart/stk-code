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
#include "network/protocols/get_public_address.hpp"
#include "network/protocols/get_peer_address.hpp"
#include "network/protocols/hide_public_address.hpp"
#include "network/protocols/request_connection.hpp"
#include "network/protocols/ping_protocol.hpp"
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
    m_current_protocol = NULL;
    m_is_lan           = false;
    setHandleConnections(true);
}   // ConnectToPeer(peer_id)

// ----------------------------------------------------------------------------
/** Constructor for a LAN connection.
 *  \param address The address to connect to.
 */
ConnectToPeer::ConnectToPeer(const TransportAddress &address)
             : Protocol(PROTOCOL_CONNECTION)
{
    m_peer_address.copy(address);
    // We don't need to find the peer address, so we can start
    // with the state when we found the peer address.
    m_state            = RECEIVED_PEER_ADDRESS;
    m_current_protocol = NULL;
    m_is_lan           = true;
    setHandleConnections(true);
}   // ConnectToPeers(TransportAddress)

// ----------------------------------------------------------------------------

ConnectToPeer::~ConnectToPeer()
{
}   // ~ConnectToPeer

// ----------------------------------------------------------------------------

void ConnectToPeer::setup()
{
    m_broadcast_count     = 0;
    m_time_last_broadcast = 0;
}   // setup
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
            m_current_protocol = new GetPeerAddress(m_peer_id, this); 
            m_current_protocol->requestStart();

            // Pause this protocol till we receive an answer
            // The GetPeerAddress protocol will change the state and
            // unpause this protocol
            requestPause();
            m_state = RECEIVED_PEER_ADDRESS;
            break;
        }
        case RECEIVED_PEER_ADDRESS:
        {
            if (m_peer_address.getIP() == 0 || m_peer_address.getPort() == 0)
            {
                Log::error("ConnectToPeer",
                    "The peer you want to connect to has hidden his address.");
                m_state = DONE;
                break;
            }
            delete m_current_protocol;
            m_current_protocol = 0;

            // Now we know the peer address. If it's a non-local host, start
            // the Ping protocol to keep the port available. We can't rely on
            // STKHost::isLAN(), since we might get a LAN connection even if
            // the server itself accepts connections from anywhere.
            if ( (!m_is_lan &&
                  m_peer_address.getIP() !=
                      NetworkConfig::get()->getMyAddress().getIP() ) || 
                  NetworkConfig::m_disable_lan                            )
            {
                m_current_protocol = new PingProtocol(m_peer_address,
                                                      /*time-between-ping*/2.0);
                ProtocolManager::getInstance()->requestStart(m_current_protocol);
                m_state = CONNECTING;
            }
            else
            {
                m_broadcast_count     = 0;
                // Make sure we trigger the broadcast operation next
                m_time_last_broadcast = float(StkTime::getRealTime()-100.0f);
                m_state               = WAIT_FOR_LAN;
            }
            break;
        }
        case WAIT_FOR_LAN:
        {
            // Broadcast once per second
            if (StkTime::getRealTime()  < m_time_last_broadcast + 1.0f)
            {
                break;
            }
            m_time_last_broadcast = float(StkTime::getRealTime());
            m_broadcast_count++;
            if (m_broadcast_count > 100)
            {
                // Not much we can do about if we don't receive the client
                // connection - it could have stopped, lost network, ...
                // Terminate this protocol.
                Log::error("ConnectToPeer", "Time out trying to connect to %s",
                           m_peer_address.toString().c_str());
                requestTerminate();
            }

            // Otherwise we are in the same LAN  (same public ip address).
            // Just send a broadcast packet with the string aloha_stk inside,
            // the client will know our ip address and will connect
            TransportAddress broadcast_address;
            if(NetworkConfig::get()->isWAN())
            {
                broadcast_address.setIP(-1); // 255.255.255.255
                broadcast_address.setPort(m_peer_address.getPort());
            }
            else
                broadcast_address.copy(m_peer_address);


            broadcast_address.copy(m_peer_address);

            BareNetworkString aloha(std::string("aloha_stk"));
            STKHost::get()->sendRawPacket(aloha, broadcast_address);
            Log::info("ConnectToPeer", "Broadcast aloha sent.");
            StkTime::sleep(1);

            broadcast_address.setIP(0x7f000001); // 127.0.0.1 (localhost)
            broadcast_address.setPort(m_peer_address.getPort());
            STKHost::get()->sendRawPacket(aloha, broadcast_address);
            Log::info("ConnectToPeer", "Broadcast aloha to self.");
            break;
        }
        case CONNECTING: // waiting for the peer to connect
            // If we receive a 'connected' event from enet, our
            // notifyEventAsynchronous is called, which will move
            // the FSM to the next state CONNECTED
            break;
        case CONNECTED:
        {
            // If the ping protocol is there for NAT traversal terminate it.
            // Ping is not running when connecting to a LAN peer.
            if (m_current_protocol)
            {
                // Kill the ping protocol because we're connected
                m_current_protocol->requestTerminate();
                m_current_protocol = NULL;
            }
            m_state = DONE;
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

// ----------------------------------------------------------------------------
/** Callback from the GetPeerAddress protocol. It copies the received peer
 *  address so that it can be used in the next states of the connection
 *  protocol.
 */
void ConnectToPeer::callback(Protocol *protocol)
{
    assert(m_state==RECEIVED_PEER_ADDRESS);
    m_peer_address.copy( ((GetPeerAddress*)protocol)->getAddress() );
    // Reactivate this protocol
    requestUnpause();
}   // callback
