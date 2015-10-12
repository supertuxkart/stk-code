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
#include "network/network_manager.hpp"
#include "utils/log.hpp"

#include <string.h>

STKPeer::STKPeer()
{
    m_peer = NULL;
    m_player_profile = new NetworkPlayerProfile*;
    *m_player_profile = NULL;
    m_client_server_token = new uint32_t;
    *m_client_server_token = 0;
    m_token_set = new bool;
    *m_token_set = false;
}

//-----------------------------------------------------------------------------

STKPeer::STKPeer(const STKPeer& peer)
{
    m_peer = peer.m_peer;
    m_player_profile = peer.m_player_profile;
    m_client_server_token = peer.m_client_server_token;
    m_token_set = peer.m_token_set;
}

//-----------------------------------------------------------------------------

STKPeer::~STKPeer()
{
    if (m_peer)
        m_peer = NULL;
    if (m_player_profile)
        delete m_player_profile;
    m_player_profile = NULL;
    if (m_client_server_token)
        delete m_client_server_token;
    m_client_server_token = NULL;
    if (m_token_set)
        delete m_token_set;
    m_token_set = NULL;
}

//-----------------------------------------------------------------------------

bool STKPeer::connectToHost(STKHost* localhost, TransportAddress host,
                uint32_t channel_count, uint32_t data)
{
    ENetAddress  address;
    address.host =
         ((host.ip & 0xff000000) >> 24)
       + ((host.ip & 0x00ff0000) >> 8)
       + ((host.ip & 0x0000ff00) << 8)
       + ((host.ip & 0x000000ff) << 24); // because ENet wants little endian
    address.port = host.port;

    ENetPeer* peer = enet_host_connect(localhost->m_host, &address, 2, 0);
    if (peer == NULL)
    {
        Log::error("STKPeer", "Could not try to connect to server.\n");
        return false;
    }
    Log::verbose("STKPeer", "Connecting to %i.%i.%i.%i:%i.\nENetPeer address "
                "is %p", (peer->address.host>>0)&0xff,
                (peer->address.host>>8)&0xff,(peer->address.host>>16)&0xff,
                (peer->address.host>>24)&0xff,peer->address.port, peer);
    return true;
}

//-----------------------------------------------------------------------------

void STKPeer::disconnect()
{
    enet_peer_disconnect(m_peer, 0);
}

//-----------------------------------------------------------------------------

void STKPeer::sendPacket(NetworkString const& data, bool reliable)
{
    Log::verbose("STKPeer", "sending packet of size %d to %i.%i.%i.%i:%i",
                data.size(), (m_peer->address.host>>0)&0xff,
                (m_peer->address.host>>8)&0xff,(m_peer->address.host>>16)&0xff,
                (m_peer->address.host>>24)&0xff,m_peer->address.port);
    ENetPacket* packet = enet_packet_create(data.getBytes(), data.size() + 1,
                (reliable ? ENET_PACKET_FLAG_RELIABLE : ENET_PACKET_FLAG_UNSEQUENCED));
    /* to debug the packet output
    printf("STKPeer: ");
    for (unsigned int i = 0; i < data.size(); i++)
    {
        printf("%d ", (uint8_t)(data[i]));
    }
    printf("\n");
    */
    enet_peer_send(m_peer, 0, packet);
}

//-----------------------------------------------------------------------------

uint32_t STKPeer::getAddress() const
{
    return ntohl(m_peer->address.host);
}

//-----------------------------------------------------------------------------

uint16_t STKPeer::getPort() const
{
    return m_peer->address.port;
}

//-----------------------------------------------------------------------------

bool STKPeer::isConnected() const
{
    Log::info("STKPeer", "The peer state is %i", m_peer->state);
    return (m_peer->state == ENET_PEER_STATE_CONNECTED);
}

//-----------------------------------------------------------------------------

bool STKPeer::exists() const
{
    return (m_peer != NULL); // assert that the peer exists
}

//-----------------------------------------------------------------------------

bool STKPeer::isSamePeer(const STKPeer* peer) const
{
    return peer->m_peer==m_peer;
}

//-----------------------------------------------------------------------------

