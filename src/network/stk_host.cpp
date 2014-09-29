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

#include "config/user_config.hpp"
#include "io/file_manager.hpp"
#include "network/network_manager.hpp"
#include "utils/log.hpp"
#include "utils/time.hpp"

#include <string.h>
#if defined(WIN32)
#  include "Ws2tcpip.h"
#  define inet_ntop InetNtop
#else
#  include <arpa/inet.h>
#  include <errno.h>
#endif
#include <pthread.h>
#include <signal.h>

#ifdef __MINGW32__
const char* inet_ntop(int af, const void* src, char* dst, int cnt)
{
    struct sockaddr_in srcaddr;

    memset(&srcaddr, 0, sizeof(struct sockaddr_in));
    memcpy(&(srcaddr.sin_addr), src, sizeof(srcaddr.sin_addr));

    srcaddr.sin_family = af;
    if (WSAAddressToString((struct sockaddr*) &srcaddr,
        sizeof(struct sockaddr_in), 0, dst, (LPDWORD) &cnt) != 0)
    {
        return NULL;
    }
    return dst;
}
#endif


FILE* STKHost::m_log_file = NULL;
pthread_mutex_t STKHost::m_log_mutex;

void STKHost::logPacket(const NetworkString ns, bool incoming)
{
    if (m_log_file == NULL)
        return;
    pthread_mutex_lock(&m_log_mutex);
    if (incoming)
        fprintf(m_log_file, "[%d\t]  <--  ", (int)(StkTime::getRealTime()));
    else
        fprintf(m_log_file, "[%d\t]  -->  ", (int)(StkTime::getRealTime()));
    for (int i = 0; i < ns.size(); i++)
    {
        fprintf(m_log_file, "%d.", ns[i]);
    }
    fprintf(m_log_file, "\n");
    pthread_mutex_unlock(&m_log_mutex);
}

// ----------------------------------------------------------------------------

void* STKHost::receive_data(void* self)
{
    ENetEvent event;
    STKHost* myself = (STKHost*)(self);
    ENetHost* host = myself->m_host;
    while (!myself->mustStopListening())
    {
        while (enet_host_service(host, &event, 20) != 0) {
            Event* evt = new Event(&event);
            if (evt->type == EVENT_TYPE_MESSAGE)
                logPacket(evt->data(), true);
            if (event.type != ENET_EVENT_TYPE_NONE)
                NetworkManager::getInstance()->notifyEvent(evt);
            delete evt;
        }
    }
    myself->m_listening = false;
    free(myself->m_listening_thread);
    myself->m_listening_thread = NULL;
    Log::info("STKHost", "Listening has been stopped");
    return NULL;
}

// ----------------------------------------------------------------------------

STKHost::STKHost()
{
    m_host             = NULL;
    m_listening_thread = NULL;
    m_log_file         = NULL;
    pthread_mutex_init(&m_exit_mutex, NULL);
    pthread_mutex_init(&m_log_mutex, NULL);
    if (UserConfigParams::m_packets_log_filename.toString() != "")
    {
        std::string s =
            file_manager->getUserConfigFile(UserConfigParams::m_packets_log_filename);
        m_log_file = fopen(s.c_str(), "w+");
    }
    if (!m_log_file)
        Log::warn("STKHost", "Network packets won't be logged: no file.");
}

// ----------------------------------------------------------------------------

STKHost::~STKHost()
{
    stopListening();
    if (m_log_file)
    {
        fclose(m_log_file);
        Log::warn("STKHost", "Packet logging file has been closed.");
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

#ifdef WIN32/*
    addr->host = 0;
    addr->host += ((unsigned int)(192)<<0); // 192.168.0.11
    addr->host += ((unsigned int)(168)<<8); // 192.168.0.11
    addr->host += ((unsigned int)(11)<<24); // 192.168.0.11*/
#endif

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
    pthread_mutex_lock(&m_exit_mutex); // will let the update function start
    m_listening_thread = (pthread_t*)(malloc(sizeof(pthread_t)));
    pthread_create(m_listening_thread, NULL, &STKHost::receive_data, this);
    m_listening = true;
}

// ----------------------------------------------------------------------------

void STKHost::stopListening()
{
    if(m_listening_thread)
    {
        pthread_mutex_unlock(&m_exit_mutex); // will stop the update function on its next update
        pthread_join(*m_listening_thread, NULL); // wait the thread to end
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
    STKHost::logPacket(NetworkString(std::string((char*)(data), length)), false);
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
        StkTime::sleep(1);
    }
    STKHost::logPacket(NetworkString(std::string((char*)(buffer), len)), true);
    return buffer;
}

// ----------------------------------------------------------------------------

uint8_t* STKHost::receiveRawPacket(TransportAddress* sender)
{
    uint8_t* buffer; // max size needed normally (only used for stun)
    buffer = (uint8_t*)(malloc(sizeof(uint8_t)*2048));
    memset(buffer, 0, 2048);

    socklen_t from_len;
    struct sockaddr_in addr;

    from_len = sizeof(addr);
    int len = recvfrom(m_host->socket, (char*)buffer, 2048, 0, (struct sockaddr*)(&addr), &from_len);

    int i = 0;
     // wait to receive the message because enet sockets are non-blocking
    while(len == -1) // nothing received
    {
        i++;
        len = recvfrom(m_host->socket, (char*)buffer, 2048, 0, (struct sockaddr*)(&addr), &from_len);
        StkTime::sleep(1); // wait 1 millisecond between two checks
    }
    if (len == SOCKET_ERROR)
    {
        Log::error("STKHost", "Problem with the socket. Please contact the dev team.");
    }
    // we received the data
    sender->ip = ntohl((uint32_t)(addr.sin_addr.s_addr));
    sender->port = ntohs(addr.sin_port);

    if (addr.sin_family == AF_INET)
    {
        char s[20];
        inet_ntop(AF_INET, &(addr.sin_addr), s, 20);
        Log::info("STKHost", "IPv4 Address of the sender was %s", s);
    }
    STKHost::logPacket(NetworkString(std::string((char*)(buffer), len)), true);
    return buffer;
}

// ----------------------------------------------------------------------------

uint8_t* STKHost::receiveRawPacket(TransportAddress sender, int max_tries)
{
    uint8_t* buffer; // max size needed normally (only used for stun)
    buffer = (uint8_t*)(malloc(sizeof(uint8_t)*2048));
    memset(buffer, 0, 2048);

    socklen_t from_len;
    struct sockaddr_in addr;

    from_len = sizeof(addr);
    int len = recvfrom(m_host->socket, (char*)buffer, 2048, 0, (struct sockaddr*)(&addr), &from_len);

    int i = 0;
     // wait to receive the message because enet sockets are non-blocking
    while(len < 0 || addr.sin_addr.s_addr == sender.ip)
    {
        i++;
        if (len>=0)
        {
            Log::info("STKHost", "Message received but the ip address didn't match the expected one.");
        }
        len = recvfrom(m_host->socket, (char*)buffer, 2048, 0, (struct sockaddr*)(&addr), &from_len);
        StkTime::sleep(1); // wait 1 millisecond between two checks
        if (i >= max_tries && max_tries != -1)
        {
            Log::verbose("STKHost", "No answer from the server on %u.%u.%u.%u:%u", (m_host->address.host&0xff),
                        (m_host->address.host>>8&0xff),
                        (m_host->address.host>>16&0xff),
                        (m_host->address.host>>24&0xff),
                        (m_host->address.port));
            return NULL;
        }
    }
    if (addr.sin_family == AF_INET)
    {
        char s[20];
        inet_ntop(AF_INET, &(addr.sin_addr), s, 20);
        Log::info("STKHost", "IPv4 Address of the sender was %s", s);
    }
    STKHost::logPacket(NetworkString(std::string((char*)(buffer), len)), true);
    return buffer;
}

// ----------------------------------------------------------------------------

void STKHost::broadcastPacket(const NetworkString& data, bool reliable)
{
    ENetPacket* packet = enet_packet_create(data.getBytes(), data.size() + 1,
               (reliable ? ENET_PACKET_FLAG_RELIABLE : ENET_PACKET_FLAG_UNSEQUENCED));
    enet_host_broadcast(m_host, 0, packet);
    STKHost::logPacket(data, false);
}

// ----------------------------------------------------------------------------

bool STKHost::peerExists(TransportAddress peer)
{
    for (unsigned int i = 0; i < m_host->peerCount; i++)
    {
        if (m_host->peers[i].address.host == ntohl(peer.ip) &&
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
        if (m_host->peers[i].address.host == ntohl(peer.ip) &&
            m_host->peers[i].address.port == peer.port &&
            m_host->peers[i].state == ENET_PEER_STATE_CONNECTED)
        {
            return true;
        }
    }
    return false;
}

// ---------------------------------------------------------------------------

int STKHost::mustStopListening()
{
  switch(pthread_mutex_trylock(&m_exit_mutex)) {
    case 0: /* if we got the lock, unlock and return 1 (true) */
      pthread_mutex_unlock(&m_exit_mutex);
      return 1;
    case EBUSY: /* return 0 (false) if the mutex was locked */
      return 0;
  }
  return 1;
}

uint16_t STKHost::getPort() const
{
    struct sockaddr_in sin;
    socklen_t len = sizeof(sin);
    if (getsockname(m_host->socket, (struct sockaddr *)&sin, &len) == -1)
        Log::error("STKHost", "Error while using getsockname().");
    else
        return ntohs(sin.sin_port);
    return 0;
}
