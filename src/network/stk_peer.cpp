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
#include "config/user_config.hpp"
#include "network/game_setup.hpp"
#include "network/network_string.hpp"
#include "network/network_player_profile.hpp"
#include "network/stk_host.hpp"
#include "network/transport_address.hpp"
#include "utils/log.hpp"
#include "utils/time.hpp"

#include <string.h>

/** Constructor for an empty peer.
 */
STKPeer::STKPeer(ENetPeer *enet_peer, STKHost* host, uint32_t host_id)
       : m_peer_address(enet_peer->address), m_host(host)
{
    m_enet_peer           = enet_peer;
    m_host_id             = host_id;
    m_connected_time      = (float)StkTime::getRealTime();
    m_token_set.store(false);
    m_client_server_token.store(0);
}   // STKPeer

//-----------------------------------------------------------------------------
void STKPeer::disconnect()
{
    TransportAddress a(m_enet_peer->address);
    if (m_enet_peer->state != ENET_PEER_STATE_CONNECTED ||
        a != m_peer_address)
        return;
    m_host->addEnetCommand(m_enet_peer, NULL, PDI_NORMAL, ECT_DISCONNECT);
}   // disconnect

//-----------------------------------------------------------------------------
/** Kick this peer (used by server).
 */
void STKPeer::kick()
{
    TransportAddress a(m_enet_peer->address);
    if (m_enet_peer->state != ENET_PEER_STATE_CONNECTED ||
        a != m_peer_address)
        return;
    m_host->addEnetCommand(m_enet_peer, NULL, PDI_KICK, ECT_DISCONNECT);
}   // kick

//-----------------------------------------------------------------------------
/** Forcefully disconnects a peer (used by server).
 */
void STKPeer::reset()
{
    TransportAddress a(m_enet_peer->address);
    if (m_enet_peer->state != ENET_PEER_STATE_CONNECTED ||
        a != m_peer_address)
        return;
    m_host->addEnetCommand(m_enet_peer, NULL, 0, ECT_RESET);
}   // reset

//-----------------------------------------------------------------------------
/** Sends a packet to this host.
 *  \param data The data to send.
 *  \param reliable If the data is sent reliable or not.
 */
void STKPeer::sendPacket(NetworkString *data, bool reliable)
{
    TransportAddress a(m_enet_peer->address);
    // Enet will reuse a disconnected peer so we check here to avoid sending
    // to wrong peer
    if (m_enet_peer->state != ENET_PEER_STATE_CONNECTED ||
        a != m_peer_address)
        return;
    data->setToken(m_client_server_token);
    Log::verbose("STKPeer", "sending packet of size %d to %s at %f",
                 data->size(), a.toString().c_str(),StkTime::getRealTime());

    ENetPacket* packet = enet_packet_create(data->getData(),
                                            data->getTotalSize(),
                                    (reliable ? ENET_PACKET_FLAG_RELIABLE
                                              : ENET_PACKET_FLAG_UNSEQUENCED));
    m_host->addEnetCommand(m_enet_peer, packet, 0, ECT_SEND_PACKET);
}   // sendPacket

//-----------------------------------------------------------------------------
/** Returns if the peer is connected or not.
 */
bool STKPeer::isConnected() const
{
    Log::debug("STKPeer", "The peer state is %i", m_enet_peer->state);
    return (m_enet_peer->state == ENET_PEER_STATE_CONNECTED);
}   // isConnected

//-----------------------------------------------------------------------------
/** Returns if this STKPeer is the same as the given peer.
 */
bool STKPeer::isSamePeer(const STKPeer* peer) const
{
    return peer->m_enet_peer==m_enet_peer;
}   // isSamePeer

//-----------------------------------------------------------------------------
/** Returns if this STKPeer is the same as the given peer.
*/
bool STKPeer::isSamePeer(const ENetPeer* peer) const
{
    return peer==m_enet_peer;
}   // isSamePeer

//-----------------------------------------------------------------------------
/** Returns the ping to this peer from host, it waits for 15 seconds for a
 *  stable ping returned by enet measured in ms.
 */
uint32_t STKPeer::getPing() const
{
    if ((float)StkTime::getRealTime() - m_connected_time < 15.0f)
        return 0;
    return m_enet_peer->lastRoundTripTime;
}   // getPing
