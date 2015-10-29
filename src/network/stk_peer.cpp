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

#include "network/stk_peer.hpp"
#include "network/game_setup.hpp"
#include "network/network_string.hpp"
#include "network/transport_address.hpp"
#include "utils/log.hpp"

#include <string.h>

/** Constructor for an empty peer.
 */
STKPeer::STKPeer()
{
    m_enet_peer           = NULL;
    m_player_profile      = NULL;
    m_client_server_token = 0;
    m_token_set           = false;
}   // STKPeer

//-----------------------------------------------------------------------------
/** Destructor.
 */
STKPeer::~STKPeer()
{
    if (m_enet_peer)
        m_enet_peer = NULL;
    if (m_player_profile)
        delete m_player_profile;
    m_player_profile = NULL;
    m_client_server_token = 0;
}   // ~STKPeer

//-----------------------------------------------------------------------------
/** Disconnect from the server.
 */
void STKPeer::disconnect()
{
    enet_peer_disconnect(m_enet_peer, 0);
}   // disconnect

//-----------------------------------------------------------------------------
/** Sends a packet to this host.
 *  \param data The data to send.
 *  \param reliable If the data is sent reliable or not.
 */
void STKPeer::sendPacket(NetworkString const& data, bool reliable)
{
    TransportAddress a(m_enet_peer->address);
    Log::verbose("STKPeer", "sending packet of size %d to %s",
                 a.toString().c_str());
         
    ENetPacket* packet = enet_packet_create(data.getBytes(), data.size() + 1,
                                    (reliable ? ENET_PACKET_FLAG_RELIABLE
                                              : ENET_PACKET_FLAG_UNSEQUENCED));
    enet_peer_send(m_enet_peer, 0, packet);
}   // sendPacket

//-----------------------------------------------------------------------------
/** Returns the IP address (in host format) of this client.
 */
uint32_t STKPeer::getAddress() const
{
    return ntohl(m_enet_peer->address.host);
}   // getAddress

//-----------------------------------------------------------------------------
/** Returns the port of this peer.
 */
uint16_t STKPeer::getPort() const
{
    return m_enet_peer->address.port;
}

//-----------------------------------------------------------------------------
/** Returns if the peer is connected or not.
 */
bool STKPeer::isConnected() const
{
    Log::info("STKPeer", "The peer state is %i", m_enet_peer->state);
    return (m_enet_peer->state == ENET_PEER_STATE_CONNECTED);
}   // isConnected

//-----------------------------------------------------------------------------

bool STKPeer::exists() const
{
    return (m_enet_peer != NULL); // assert that the peer exists
}

//-----------------------------------------------------------------------------

bool STKPeer::isSamePeer(const STKPeer* peer) const
{
    return peer->m_enet_peer==m_enet_peer;
}

//-----------------------------------------------------------------------------

