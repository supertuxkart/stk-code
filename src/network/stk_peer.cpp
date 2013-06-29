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

#include "stk_peer.hpp"

#include <stdio.h>
#include <string.h>

STKPeer::STKPeer()
{
    m_peer = NULL;
}

STKPeer::~STKPeer()
{
    if (m_peer)
        delete m_peer;
}

bool STKPeer::connectToHost(STKHost* localhost, TransportAddress host, uint32_t channel_count, uint32_t data)
{
    ENetAddress  address;
    address.host = host.ip;
    address.port = host.port;
    
    ENetPeer* peer = enet_host_connect(localhost->m_host, &address, 2, 0);
    if (peer == NULL) 
    {
        printf("Could not try to connect to server.\n");
        return false;
    }
    printf("Connecting to %i.%i.%i.%i:%i.\n", (peer->address.host>>0)&0xff,(peer->address.host>>8)&0xff,(peer->address.host>>16)&0xff,(peer->address.host>>24)&0xff,peer->address.port);
    return true;
}

void STKPeer::sendPacket(char* data)
{
    //printf("sending packet to %i.%i.%i.%i:%i", (m_peer->address.host>>24)&0xff,(m_peer->address.host>>16)&0xff,(m_peer->address.host>>8)&0xff,(m_peer->address.host>>0)&0xff,m_peer->address.port);
    
    ENetPacket* packet = enet_packet_create(data, strlen(data)+1,ENET_PACKET_FLAG_RELIABLE);
    enet_peer_send(m_peer, 0, packet);
}

uint32_t STKPeer::getAddress()
{
    return m_peer->address.host;
}

uint16_t STKPeer::getPort()
{
    return m_peer->address.port;
}

bool STKPeer::isConnected()
{
    printf("PEER STATE %i\n", m_peer->state);
    return (m_peer->state == ENET_PEER_STATE_CONNECTED);
}
bool STKPeer::operator==(ENetPeer* peer)
{
    return peer==m_peer;
}
