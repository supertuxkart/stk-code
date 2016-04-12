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

#include "network/network.hpp"

#include "config/user_config.hpp"
#include "io/file_manager.hpp"
#include "network/network_string.hpp"
#include "network/transport_address.hpp"
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

Synchronised<FILE*>Network::m_log_file = NULL;

// ============================================================================
/** Constructor that just initialises this object (esp. opening the packet
 *  log file), but it does not start a listener thread.
 *  \param peer_count : The maximum number of peers.
 *  \param channel_limit : The maximum number of channels per peer.
 *  \param max_incoming_bandwidth : The maximum incoming bandwidth.
 *  \param max_outgoing_bandwidth : The maximum outgoing bandwidth.
 */
Network::Network(int peer_count, int channel_limit,
                 uint32_t max_incoming_bandwidth,
                 uint32_t max_outgoing_bandwidth,
                 ENetAddress* address)
{
    m_host = enet_host_create(address, peer_count, channel_limit, 0, 0);
    if (!m_host)
    {
        Log::fatal("Network", "An error occurred while trying to create an "
                   "ENet client host.");
    }
}   // Network

// ----------------------------------------------------------------------------
/** Destructor. Stops the listening thread, closes the packet log file and
 *  destroys the enet host.
 */
Network::~Network()
{
    if (m_host)
    {
        enet_host_destroy(m_host);
    }
}   // ~Network

// ----------------------------------------------------------------------------
ENetPeer *Network::connectTo(const TransportAddress &address)
{
    const ENetAddress enet_address = address.toEnetAddress();
    return enet_host_connect(m_host, &enet_address, 2, 0);
}   // connectTo

// ----------------------------------------------------------------------------
/** \brief Sends a packet whithout ENet adding its headers.
 *  This function is used in particular to achieve the STUN protocol.
 *  \param data : Data to send.
 *  \param dst : Destination of the packet.
 */
void Network::sendRawPacket(const BareNetworkString &buffer,
                            const TransportAddress& dst)
{
    struct sockaddr_in to;
    int to_len = sizeof(to);
    memset(&to,0,to_len);

    to.sin_family = AF_INET;
    to.sin_port = htons(dst.getPort());
    to.sin_addr.s_addr = htonl(dst.getIP());

    sendto(m_host->socket, buffer.getData(), buffer.size(), 0,
           (sockaddr*)&to, to_len);
    Log::verbose("Network", "Raw packet sent to %s",
                 dst.toString().c_str());
    Network::logPacket(buffer, false);
}   // sendRawPacket

// ----------------------------------------------------------------------------
/** \brief Receives a packet directly from the network interface and
 *  filter its address.
 *  Receive a packet whithout ENet processing it. Checks that the
 *  sender of the packet is the one that corresponds to the sender
 *  parameter. Does not check the port right now.
 *  \param buffer A buffer to receive the data in.
 *  \param buf_len  Length of the buffer.
 *  \param[out] sender : Transport address of the original sender of the
 *                  wanted packet. If the ip address is 0, do not check
 *                  the sender's ip address, otherwise wait till a message
 *                  from the specified sender arrives. All other messages
 *                  are discarded.
 *  \param max_tries : Number of times we try to read data from the
 *                  socket. This is aproximately the time we wait in
 *                  milliseconds. -1 means eternal tries.
 *  \return Length of the received data, or -1 if no data was received.
 */
int Network::receiveRawPacket(char *buffer, int buf_len, 
                              TransportAddress *sender, int max_tries)
{
    memset(buffer, 0, buf_len);

    struct sockaddr_in addr;
    socklen_t from_len = sizeof(addr);

    int len = recvfrom(m_host->socket, buffer, buf_len, 0,
                       (struct sockaddr*)(&addr), &from_len    );

    int count = 0;
    // wait to receive the message because enet sockets are non-blocking
    while(len < 0 && (count<max_tries || max_tries==-1) )
    {
        count++;
        StkTime::sleep(1); // wait 1 millisecond between two checks
        len = recvfrom(m_host->socket, buffer, buf_len, 0, 
                       (struct sockaddr*)(&addr), &from_len);
    }

    // No message received
    if(len<0)
        return -1;

    Network::logPacket(BareNetworkString(buffer, len), true);
    sender->setIP(ntohl((uint32_t)(addr.sin_addr.s_addr)) );
    sender->setPort( ntohs(addr.sin_port) );
    if (addr.sin_family == AF_INET)
    {
        Log::info("Network", "IPv4 Address of the sender was %s",
                  sender->toString().c_str());
    }
    return len;
}   // receiveRawPacket

// ----------------------------------------------------------------------------
/** \brief Broadcasts a packet to all peers.
 *  \param data : Data to send.
 */
void Network::broadcastPacket(NetworkString *data, bool reliable)
{
    ENetPacket* packet = enet_packet_create(data->getData(), data->size() + 1,
                                      reliable ? ENET_PACKET_FLAG_RELIABLE
                                               : ENET_PACKET_FLAG_UNSEQUENCED);
    enet_host_broadcast(m_host, 0, packet);
}   // broadcastPacket

// ----------------------------------------------------------------------------
void Network::openLog()
{
    m_log_file.setAtomic(NULL);
    if (UserConfigParams::m_log_packets)
    {
        std::string s = file_manager
            ->getUserConfigFile(FileManager::getStdoutName()+".packet");
        m_log_file.setAtomic(fopen(s.c_str(), "w+"));
    }
    if (!m_log_file.getData())
        Log::warn("STKHost", "Network packets won't be logged: no file.");
}   // openLog

// ----------------------------------------------------------------------------
/** \brief Log packets into a file
 *  \param ns : The data in the packet
 *  \param incoming : True if the packet comes from a peer.
 *  False if it's sent to a peer.
 */
void Network::logPacket(const BareNetworkString &ns, bool incoming)
{
    if (m_log_file.getData() == NULL) // read only access, no need to lock
        return;

    const char *arrow = incoming ? "<--" : "-->";

    m_log_file.lock();
    fprintf(m_log_file.getData(), "[%d\t]  %s  ",
            (int)(StkTime::getRealTime()), arrow);
    // Indentation for all lines after the first, so that the dump
    // is nicely aligned.
    std::string indent("                ");
    fprintf(m_log_file.getData(), "%s", ns.getLogMessage(indent).c_str());
    m_log_file.unlock();
}   // logPacket
// ----------------------------------------------------------------------------
void Network::closeLog()
{
    if (m_log_file.getData())
    {
        m_log_file.lock();
        fclose(m_log_file.getData());
        Log::warn("STKHost", "Packet logging file has been closed.");
        m_log_file.getData() = NULL;
        m_log_file.unlock();
    }
}   // closeLog