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

#include "network/stk_host.hpp"

#include "config/user_config.hpp"
#include "io/file_manager.hpp"
#include "network/network_manager.hpp"
#include "utils/log.hpp"
#include "utils/time.hpp"

#include <string.h>
#if defined(WIN32)
#  include "ws2tcpip.h"
#  define inet_ntop InetNtop
#else
#  include <arpa/inet.h>
#  include <errno.h>
#  include <sys/socket.h>
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

Synchronised<FILE*> STKHost::m_log_file = NULL;

// ============================================================================
/** Constructor that just initialises this object (esp. opening the packet
 *  log file), but it does not start a listener thread.
 */
STKHost::STKHost()
{
    m_host             = NULL;
    m_listening_thread = NULL;
    m_log_file.setAtomic(NULL);
    pthread_mutex_init(&m_exit_mutex, NULL);
    if (UserConfigParams::m_packets_log_filename.toString() != "")
    {
        std::string s = file_manager
                 ->getUserConfigFile(UserConfigParams::m_packets_log_filename);
        m_log_file.setAtomic(fopen(s.c_str(), "w+"));
    }
    if (!m_log_file.getData())
        Log::warn("STKHost", "Network packets won't be logged: no file.");
}   // STKHost

// ----------------------------------------------------------------------------
/** Destructor. Stops the listening thread, closes the packet log file and
 *  destroys the enet host.
 */
STKHost::~STKHost()
{
    stopListening();
    if (m_log_file.getData())
    {
        m_log_file.lock();
        fclose(m_log_file.getData());
        Log::warn("STKHost", "Packet logging file has been closed.");
        m_log_file.getData() = NULL;
        m_log_file.unlock();
    }
    if (m_host)
    {
        enet_host_destroy(m_host);
    }
}   // ~STKHost

// ----------------------------------------------------------------------------
/** \brief Log packets into a file
 *  \param ns : The data in the packet
 *  \param incoming : True if the packet comes from a peer.
 *  False if it's sent to a peer.
 */
void STKHost::logPacket(const NetworkString &ns, bool incoming)
{
    if (m_log_file.getData() == NULL) // read only access, no need to lock
        return;

    const char *arrow = incoming ? "<--" : "-->";

    m_log_file.lock();
    fprintf(m_log_file.getData(), "[%d\t]  %s  ",
            (int)(StkTime::getRealTime()), arrow);

    for (int i = 0; i < ns.size(); i++)
    {
        fprintf(m_log_file.getData(), "%d.", ns[i]);
    }
    fprintf(m_log_file.getData(), "\n");
    m_log_file.unlock();
}   // logPacket

// ----------------------------------------------------------------------------
/** \brief Starts the listening of events from ENet.
 *  Starts a thread for receiveData that updates it as often as possible.
 */
void STKHost::startListening()
{
    pthread_mutex_lock(&m_exit_mutex); // will let the update function start
    m_listening_thread = new pthread_t;
    pthread_create(m_listening_thread, NULL, &STKHost::mainLoop, this);
}   // startListening

// ----------------------------------------------------------------------------
/** \brief Stops the listening of events from ENet.
 *  Stops the thread that was receiving events.
 */
void STKHost::stopListening()
{
    if (m_listening_thread)
    {
        // This will stop the update function on its next update
        pthread_mutex_unlock(&m_exit_mutex);
        pthread_join(*m_listening_thread, NULL); // wait for the thread to end
    }
}   // stopListening

// ---------------------------------------------------------------------------
/** \brief Returns true when the thread should stop listening.
 */
int STKHost::mustStopListening()
{
    switch (pthread_mutex_trylock(&m_exit_mutex)) {
    case 0: /* if we got the lock, unlock and return 1 (true) */
        pthread_mutex_unlock(&m_exit_mutex);
        return 1;
    case EBUSY: /* return 0 (false) if the mutex was locked */
        return 0;
    }
    return 1;
}   // mustStopListening

// ----------------------------------------------------------------------------
/** \brief Thread function checking if data is received.
 *  This function tries to get data from network low-level functions as
 *  often as possible. When something is received, it generates an
 *  event and passes it to the Network Manager.
 *  \param self : used to pass the ENet host to the function.
 */
void* STKHost::mainLoop(void* self)
{
    ENetEvent event;
    STKHost* myself = (STKHost*)(self);
    ENetHost* host = myself->m_host;
    while (!myself->mustStopListening())
    {
        while (enet_host_service(host, &event, 20) != 0)
        {
            Event* stk_event = new Event(&event);
            if (stk_event->type == EVENT_TYPE_MESSAGE)
                logPacket(stk_event->data(), true);
            if (event.type != ENET_EVENT_TYPE_NONE)
                NetworkManager::getInstance()->notifyEvent(stk_event);
            delete stk_event;
        }   // while enet_host_service
    }   // while !mustStopListening

    free(myself->m_listening_thread);
    myself->m_listening_thread = NULL;
    Log::info("STKHost", "Listening has been stopped");
    return NULL;
}   // mainLoop

// ----------------------------------------------------------------------------
/** \brief Setups this host as a server.
 *  \param address : The IPv4 address of incoming connections.
 *  \param port : The port on which the server listens.
 *  \param peer_count : The maximum number of peers.
 *  \param channel_limit : The maximum number of channels per peer.
 *  \param max_incoming_bandwidth : The maximum incoming bandwidth.
 *  \param max_outgoing_bandwidth : The maximum outgoing bandwidth.
 */
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
    if (!m_host)
    {
        Log::fatal("STKHost", "An error occurred while trying to create an ENet"
                          " server host.");
    }
}   // setupServer

// ----------------------------------------------------------------------------
/** \brief Setups the host as a client.
 *  In fact there is only one peer connected to this host.
 *  \param peer_count : The maximum number of peers.
 *  \param channel_limit : The maximum number of channels per peer.
 *  \param max_incoming_bandwidth : The maximum incoming bandwidth.
 *  \param max_outgoing_bandwidth : The maximum outgoing bandwidth.
 */

void STKHost::setupClient(int peer_count, int channel_limit,
                          uint32_t max_incoming_bandwidth,
                          uint32_t max_outgoing_bandwidth)
{
    m_host = enet_host_create(NULL, peer_count, channel_limit,
                              max_incoming_bandwidth, max_outgoing_bandwidth);
    if (!m_host)
    {
        Log::fatal ("STKHost", "An error occurred while trying to create an "
                          "ENet client host.");
    }
}   // setupClient

// ----------------------------------------------------------------------------
/** \brief Sends a packet whithout ENet adding its headers.
 *  This function is used in particular to achieve the STUN protocol.
 *  \param data : Data to send.
 *  \param length : Length of the sent data.
 *  \param dst : Destination of the packet.
 */
void STKHost::sendRawPacket(uint8_t* data, int length,
                            const TransportAddress& dst)
{
    struct sockaddr_in to;
    int to_len = sizeof(to);
    memset(&to,0,to_len);

    to.sin_family = AF_INET;
    to.sin_port = htons(dst.getPort());
    to.sin_addr.s_addr = htonl(dst.getIP());

    sendto(m_host->socket, (char*)data, length, 0,(sockaddr*)&to, to_len);
    Log::verbose("STKHost", "Raw packet sent to %s", dst.toString().c_str());
    STKHost::logPacket(NetworkString(std::string((char*)(data), length)),
                       false);
}   // sendRawPacket

// ----------------------------------------------------------------------------
/** \brief Receives a packet directly from the network interface.
 *  Receive a packet whithout ENet processing it and returns the
 *  sender's ip address and port in the TransportAddress structure.
 *  \param sender : Stores the transport address of the sender of the
 *                  received packet.
 *  \return A string containing the data of the received packet.
 */
uint8_t* STKHost::receiveRawPacket(TransportAddress* sender)
{
    const int LEN = 2048;
    // max size needed normally (only used for stun)
    uint8_t* buffer = new uint8_t[LEN]; 
    memset(buffer, 0, LEN);

    socklen_t from_len;
    struct sockaddr_in addr;

    from_len = sizeof(addr);
    int len = recvfrom(m_host->socket, (char*)buffer, LEN, 0,
                       (struct sockaddr*)(&addr), &from_len   );

    int i = 0;
     // wait to receive the message because enet sockets are non-blocking
    while(len == -1) // nothing received
    {
        StkTime::sleep(1); // wait 1 millisecond between two checks
        i++;
        len = recvfrom(m_host->socket, (char*)buffer, LEN, 0,
                       (struct sockaddr*)(&addr), &from_len   );
    }
    if (len == SOCKET_ERROR)
    {
        Log::error("STKHost",
                   "Problem with the socket. Please contact the dev team.");
    }
    // we received the data
    sender->setIP( ntohl((uint32_t)(addr.sin_addr.s_addr)) );
    sender->setPort( ntohs(addr.sin_port) );

    if (addr.sin_family == AF_INET)
    {
        Log::info("STKHost", "IPv4 Address of the sender was %s",
                  sender->toString().c_str());
    }
    STKHost::logPacket(NetworkString(std::string((char*)(buffer), len)), true);
    return buffer;
}   // receiveRawPacket(TransportAddress* sender)

// ----------------------------------------------------------------------------
/** \brief Receives a packet directly from the network interface and
 *  filter its address.
 *  Receive a packet whithout ENet processing it. Checks that the
 *  sender of the packet is the one that corresponds to the sender
 *  parameter. Does not check the port right now.
 *  \param sender : Transport address of the original sender of the
 *                  wanted packet.
 *  \param max_tries : Number of times we try to read data from the
 *                  socket. This is aproximately the time we wait in
 *                  milliseconds. -1 means eternal tries.
 *  \return A string containing the data of the received packet
 *          matching the sender's ip address.
 */
uint8_t* STKHost::receiveRawPacket(const TransportAddress& sender,
                                   int max_tries)
{
    const int LEN = 2048;
    uint8_t* buffer = new uint8_t[LEN];
    memset(buffer, 0, LEN);

    socklen_t from_len;
    struct sockaddr_in addr;

    from_len = sizeof(addr);
    int len = recvfrom(m_host->socket, (char*)buffer, LEN, 0,
                       (struct sockaddr*)(&addr), &from_len    );

    int count = 0;
     // wait to receive the message because enet sockets are non-blocking
    while(len < 0 || addr.sin_addr.s_addr == sender.getIP())
    {
        count++;
        if (len>=0)
        {
            Log::info("STKHost", "Message received but the ip address didn't "
                                 "match the expected one.");
        }
        len = recvfrom(m_host->socket, (char*)buffer, LEN, 0, 
                       (struct sockaddr*)(&addr), &from_len);
        StkTime::sleep(1); // wait 1 millisecond between two checks
        if (count >= max_tries && max_tries != -1)
        {
            TransportAddress a(m_host->address);
            Log::verbose("STKHost", "No answer from the server on %s",
                         a.toString().c_str());
            return NULL;
        }
    }
    if (addr.sin_family == AF_INET)
    {
        TransportAddress a(addr.sin_addr.s_addr);
        Log::info("STKHost", "IPv4 Address of the sender was %s",
                  a.toString(false).c_str());
    }
    STKHost::logPacket(NetworkString(std::string((char*)(buffer), len)), true);
    return buffer;
}   // receiveRawPacket(const TransportAddress& sender, int max_tries)

// ----------------------------------------------------------------------------
/** \brief Broadcasts a packet to all peers.
 *  \param data : Data to send.
 */
void STKHost::broadcastPacket(const NetworkString& data, bool reliable)
{
    ENetPacket* packet = enet_packet_create(data.getBytes(), data.size() + 1,
                                      reliable ? ENET_PACKET_FLAG_RELIABLE
                                               : ENET_PACKET_FLAG_UNSEQUENCED);
    enet_host_broadcast(m_host, 0, packet);
    STKHost::logPacket(data, false);
}   // broadcastPacket

// ----------------------------------------------------------------------------
/** \brief Tells if a peer is known.
 *  \return True if the peer is known, false elseway.
 */
bool STKHost::peerExists(const TransportAddress& peer)
{
    for (unsigned int i = 0; i < m_host->peerCount; i++)
    {
        if (m_host->peers[i].address.host == ntohl(peer.getIP()) &&
            m_host->peers[i].address.port == peer.getPort()        )
        {
            return true;
        }
    }
    return false;
}   // peerExists

// ----------------------------------------------------------------------------
/** \brief Tells if a peer is known and connected.
 *  \return True if the peer is known and connected, false elseway.
 */
bool STKHost::isConnectedTo(const TransportAddress& peer)
{
    for (unsigned int i = 0; i < m_host->peerCount; i++)
    {
        if (peer == m_host->peers[i].address &&
            m_host->peers[i].state == ENET_PEER_STATE_CONNECTED)
        {
            return true;
        }
    }
    return false;
}   // isConnectedTo

// ----------------------------------------------------------------------------
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
