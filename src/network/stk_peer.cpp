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
#include "config/stk_config.hpp"
#include "config/user_config.hpp"
#include "network/crypto.hpp"
#include "network/event.hpp"
#include "network/network.hpp"
#include "network/network_config.hpp"
#include "network/network_string.hpp"
#include "network/socket_address.hpp"
#include "network/stk_ipv6.hpp"
#include "network/stk_host.hpp"
#include "utils/log.hpp"
#include "utils/string_utils.hpp"
#include "utils/time.hpp"

#include <string.h>

/** Constructor for an empty peer.
 */
STKPeer::STKPeer(ENetPeer *enet_peer, STKHost* host, uint32_t host_id)
       : m_address(enet_peer->address), m_host(host)
{
    m_addons_scores.fill(-1);
    m_socket_address.reset(new SocketAddress(m_address));
    m_enet_peer           = enet_peer;
    m_host_id             = host_id;
    m_connected_time      = StkTime::getMonoTimeMs();
    m_validated.store(false);
    m_always_spectate.store(ASM_NONE);
    m_average_ping.store(0);
    m_packet_loss.store(0);
    m_waiting_for_game.store(true);
    m_spectator.store(false);
    m_disconnected.store(false);
    m_warned_for_high_ping.store(false);
    m_last_activity.store((int64_t)StkTime::getMonoTimeMs());
    m_last_message.store(0);
    m_consecutive_messages = 0;
}   // STKPeer

//-----------------------------------------------------------------------------
STKPeer::~STKPeer()
{
}   // ~STKPeer

//-----------------------------------------------------------------------------
void STKPeer::disconnect()
{
    if (m_disconnected.load())
        return;
    m_disconnected.store(true);
    m_host->addEnetCommand(m_enet_peer, NULL, PDI_NORMAL, ECT_DISCONNECT,
        m_address);
}   // disconnect

//-----------------------------------------------------------------------------
/** Kick this peer (used by server).
 */
void STKPeer::kick()
{
    if (m_disconnected.load())
        return;
    m_disconnected.store(true);
    m_host->addEnetCommand(m_enet_peer, NULL, PDI_KICK, ECT_DISCONNECT,
        m_address);
}   // kick

//-----------------------------------------------------------------------------
/** Forcefully disconnects a peer (used by server).
 */
void STKPeer::reset()
{
    if (m_disconnected.load())
        return;
    m_disconnected.store(true);
    m_host->addEnetCommand(m_enet_peer, NULL, 0, ECT_RESET, m_address);
}   // reset

//-----------------------------------------------------------------------------
/** Sends a packet to this host.
 *  \param data The data to send.
 *  \param reliable If the data is sent reliable or not.
 *  \param encrypted If the data is sent encrypted or not.
 */
void STKPeer::sendPacket(NetworkString *data, bool reliable, bool encrypted)
{
    if (m_disconnected.load())
        return;

    ENetPacket* packet = NULL;
    if (m_crypto && encrypted)
    {
        packet = m_crypto->encryptSend(*data, reliable);
    }
    else
    {
        packet = enet_packet_create(data->getData(),
            data->getTotalSize(), (reliable ?
            ENET_PACKET_FLAG_RELIABLE :
            (ENET_PACKET_FLAG_UNSEQUENCED |
            ENET_PACKET_FLAG_UNRELIABLE_FRAGMENT)));
    }

    if (packet)
    {
        if (Network::m_connection_debug)
        {
            Log::verbose("STKPeer", "sending packet of size %d to %s at %lf",
                packet->dataLength, getAddress().toString().c_str(),
                StkTime::getRealTime());
        }
        m_host->addEnetCommand(m_enet_peer, packet,
                encrypted ? EVENT_CHANNEL_NORMAL : EVENT_CHANNEL_UNENCRYPTED,
                ECT_SEND_PACKET, m_address);
    }
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
/** Returns the ping to this peer from host, it waits for 3 seconds for a
 *  stable ping returned by enet measured in ms.
 */
uint32_t STKPeer::getPing()
{
    if (getConnectedTime() < 3.0f)
    {
        m_average_ping.store(m_enet_peer->roundTripTime);
        return 0;
    }
    if (NetworkConfig::get()->isServer())
    {
        // Average ping in 5 seconds
        // Frequency is 10 packets per second as seen in STKHost
        const unsigned ap = 10 * 5;
        m_previous_pings.push_back(m_enet_peer->roundTripTime);
        while (m_previous_pings.size() > ap)
        {
            m_previous_pings.pop_front();
            m_average_ping.store(
                (uint32_t)(std::accumulate(m_previous_pings.begin(),
                m_previous_pings.end(), 0) / m_previous_pings.size()));
        }
    }
    return m_enet_peer->roundTripTime;
}   // getPing

//-----------------------------------------------------------------------------
void STKPeer::setCrypto(std::unique_ptr<Crypto>&& c)
{
    m_crypto = std::move(c);
}   // setCrypto
