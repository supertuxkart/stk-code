#include "stk_peer.hpp"

#include <stdio.h>
#include <string.h>

STKPeer::STKPeer()
{
    m_peer = NULL;
}

STKPeer::~STKPeer()
{
}

bool STKPeer::connectToServer(STKHost* host, uint32_t ip, uint16_t port, uint32_t channelCount, uint32_t data)
{
    ENetAddress  address;
    address.host = ip;
    address.port = port;
    
    m_peer = enet_host_connect(host->m_host, &address, 2, 0);
    if (m_peer == NULL) 
    {
        printf("Could not try to connect to server.\n");
        return false;
    }
    printf("Connecting to %i.%i.%i.%i:%i.\n", (m_peer->address.host>>0)&0xff,(m_peer->address.host>>8)&0xff,(m_peer->address.host>>16)&0xff,(m_peer->address.host>>24)&0xff,m_peer->address.port);
    return true;
}

void STKPeer::sendPacket(char* data)
{
    //printf("sending packet to %i.%i.%i.%i:%i", (m_peer->address.host>>24)&0xff,(m_peer->address.host>>16)&0xff,(m_peer->address.host>>8)&0xff,(m_peer->address.host>>0)&0xff,m_peer->address.port);
    
    ENetPacket* packet = enet_packet_create(data, strlen(data)+1,ENET_PACKET_FLAG_RELIABLE);
    enet_peer_send(m_peer, 0, packet);
}

bool STKPeer::isConnected()
{
    printf("PEER STATE %i\n", m_peer->state);
    return (m_peer->state == ENET_PEER_STATE_CONNECTED);
}
