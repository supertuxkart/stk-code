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

#include "network/stk_host.hpp"

#include "graphics/irr_driver.hpp"    // get access to irrlicht sleep function
#include "network/network_manager.hpp"
#include "utils/log.hpp"

#include <string.h>
#ifdef WIN32
#  include "Ws2tcpip.h"
#  define   inet_ntop  InetNtop
#else
#  include <arpa/inet.h>
#endif
#include <pthread.h>
#include <signal.h>

// ----------------------------------------------------------------------------

void* STKHost::receive_data(void* self)
{
    ENetEvent event;
    ENetHost* host = (((STKHost*)(self))->m_host);
    while (1)
    {
        while (enet_host_service(host, &event, 20) != 0) {
            Event* evt = new Event(&event);
            if (event.type != ENET_EVENT_TYPE_NONE)
                NetworkManager::getInstance()->notifyEvent(evt);
            delete evt;
        }
    }
    return NULL;
}

// ----------------------------------------------------------------------------

STKHost::STKHost()
{
    m_host = NULL;
    m_listening_thread = NULL;
}

// ----------------------------------------------------------------------------

STKHost::~STKHost()
{
    if (m_listening_thread)
    {
        pthread_cancel(*m_listening_thread);//, SIGKILL); with kill
        delete m_listening_thread;
        m_listening_thread = NULL;
    }
    if (m_host)
    {
        enet_host_destroy(m_host);
    }
}

// ----------------------------------------------------------------------------

void STKHost::setupServer(uint32_t address, uint16_t port, int peer_count,
                            int channel_limit, uint32_t max_incoming_bandwidth,
                            uint32_t max_outgoing_bandwidth)
{
    ENetAddress* addr = (ENetAddress*)(malloc(sizeof(ENetAddress)));
    addr->host = address;
    addr->port = port;

    m_host = enet_host_create(addr, peer_count, channel_limit,
                            max_incoming_bandwidth, max_outgoing_bandwidth);
    if (m_host == NULL)
    {
        Log::error("STKHost", "An error occurred while trying to create an ENet"
                          " server host.");
        exit (EXIT_FAILURE);
    }
}

// ----------------------------------------------------------------------------

void STKHost::setupClient(int peer_count, int channel_limit,
                            uint32_t max_incoming_bandwidth,
                            uint32_t max_outgoing_bandwidth)
{
    m_host = enet_host_create(NULL, peer_count, channel_limit,
                            max_incoming_bandwidth, max_outgoing_bandwidth);
    if (m_host == NULL)
    {
        Log::error("STKHost", "An error occurred while trying to create an ENet"
                          " client host.");
        exit (EXIT_FAILURE);
    }
}

// ----------------------------------------------------------------------------

void STKHost::startListening()
{
    m_listening_thread = (pthread_t*)(malloc(sizeof(pthread_t)));
    pthread_create(m_listening_thread, NULL, &STKHost::receive_data, this);
}

// ----------------------------------------------------------------------------

void STKHost::stopListening()
{
    if(m_listening_thread)
    {
        pthread_cancel(*m_listening_thread);
        m_listening_thread = NULL;
    }
}

// ----------------------------------------------------------------------------

void STKHost::sendRawPacket(uint8_t* data, int length, TransportAddress dst)
{
    struct sockaddr_in to;
    int to_len = sizeof(to);
    memset(&to,0,to_len);

    to.sin_family = AF_INET;
    to.sin_port = htons(dst.port);
    to.sin_addr.s_addr = htonl(dst.ip);

    sendto(m_host->socket, (char*)data, length, 0,(sockaddr*)&to, to_len);
    Log::verbose("STKHost", "Raw packet sent to %i.%i.%i.%i:%u", ((dst.ip>>24)&0xff)
    , ((dst.ip>>16)&0xff), ((dst.ip>>8)&0xff), ((dst.ip>>0)&0xff), dst.port);
}

// ----------------------------------------------------------------------------

uint8_t* STKHost::receiveRawPacket()
{
    uint8_t* buffer; // max size needed normally (only used for stun)
    buffer = (uint8_t*)(malloc(sizeof(uint8_t)*2048));
    memset(buffer, 0, 2048);

    int len = recv(m_host->socket,(char*)buffer,2048, 0);
    int i = 0;
    // wait to receive the message because enet sockets are non-blocking
    while(len < 0)
    {
        i++;
        len = recv(m_host->socket,(char*)buffer,2048, 0);
        usleep(1000);
    }
    return buffer;
}

// ----------------------------------------------------------------------------

uint8_t* STKHost::receiveRawPacket(TransportAddress* sender)
{
    uint8_t* buffer; // max size needed normally (only used for stun)
    buffer = (uint8_t*)(malloc(sizeof(uint8_t)*2048));
    memset(buffer, 0, 2048);

    socklen_t from_len;
    struct sockaddr addr;

    from_len = sizeof(addr);
    int len = recvfrom(m_host->socket, (char*)buffer, 2048, 0, &addr, &from_len);

    int i = 0;
     // wait to receive the message because enet sockets are non-blocking
    while(len < 0)
    {
        i++;
        len = recvfrom(m_host->socket, (char*)buffer, 2048, 0, &addr, &from_len);
        usleep(1000); // wait 1 millisecond between two checks
    }
    struct sockaddr_in *sin = (struct sockaddr_in *) (&addr);
    // we received the data
    sender->ip = turnEndianness(sin->sin_addr.s_addr);
    sender->port = turnEndianness(sin->sin_port);

    if (addr.sa_family == AF_INET)
    {
        char s[20];
        inet_ntop(AF_INET, &(((struct sockaddr_in *)&addr)->sin_addr), s, 20);
        Log::info("STKHost", "IPv4 Address of the sender was %s", s);
    }
    return buffer;
}

// ----------------------------------------------------------------------------

uint8_t* STKHost::receiveRawPacket(TransportAddress sender)
{
    uint8_t* buffer; // max size needed normally (only used for stun)
    buffer = (uint8_t*)(malloc(sizeof(uint8_t)*2048));
    memset(buffer, 0, 2048);

    socklen_t from_len;
    struct sockaddr addr;

    from_len = sizeof(addr);
    int len = recvfrom(m_host->socket, (char*)buffer, 2048, 0, &addr, &from_len);

    int i = 0;
     // wait to receive the message because enet sockets are non-blocking
    while(len < 0 || (
               (uint8_t)(addr.sa_data[2]) != (sender.ip>>24&0xff)
            && (uint8_t)(addr.sa_data[3]) != (sender.ip>>16&0xff)
            && (uint8_t)(addr.sa_data[4]) != (sender.ip>>8&0xff)
            && (uint8_t)(addr.sa_data[5]) != (sender.ip&0xff)))
    {
        i++;
        len = recvfrom(m_host->socket, (char*)buffer, 2048, 0, &addr, &from_len);
        usleep(1000); // wait 1 millisecond between two checks
    }
    if (addr.sa_family == AF_INET)
    {
        char s[20];
        inet_ntop(AF_INET, &(((struct sockaddr_in *)&addr)->sin_addr), s, 20);
        Log::info("STKHost", "IPv4 Address of the sender was %s", s);
    }
    return buffer;
}

// ----------------------------------------------------------------------------

void STKHost::broadcastPacket(const NetworkString& data, bool reliable)
{
    ENetPacket* packet = enet_packet_create(data.c_str(), data.size()+1,
               (reliable ? ENET_PACKET_FLAG_RELIABLE : ENET_PACKET_FLAG_UNSEQUENCED));
    enet_host_broadcast(m_host, 0, packet);
}

// ----------------------------------------------------------------------------

bool STKHost::peerExists(TransportAddress peer)
{
    for (unsigned int i = 0; i < m_host->peerCount; i++)
    {
        if (m_host->peers[i].address.host == turnEndianness(peer.ip) &&
            m_host->peers[i].address.port == peer.port)
        {
            return true;
        }
    }
    return false;
}

// ----------------------------------------------------------------------------

bool STKHost::isConnectedTo(TransportAddress peer)
{
    for (unsigned int i = 0; i < m_host->peerCount; i++)
    {
        if (m_host->peers[i].address.host == turnEndianness(peer.ip) &&
            m_host->peers[i].address.port == peer.port &&
            m_host->peers[i].state == ENET_PEER_STATE_CONNECTED)
        {
            return true;
        }
    }
    return false;
}
