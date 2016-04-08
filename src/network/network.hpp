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

/*! \file network.hpp
 *  \brief Defines an interface to use network low-level functions easily.
 */
#ifndef HEADER_NETWORK_HPP
#define HEADER_NETWORK_HPP

#include "utils/synchronised.hpp"
#include "utils/types.hpp"

// enet.h includes win32.h, which without lean_and_mean includes
// winspool.h, which defines MAX_PRIORITY as a macro, which then
// results in request_manager.hpp not being compilable.
#define WIN32_LEAN_AND_MEAN
#include <enet/enet.h>

#include <stdio.h>

class BareNetworkString;
class NetworkString;
class TransportAddress;

/** \class EnetHost
 *  A small wrapper around enet to allow sending and receiving
 *  packages.
 */
class Network
{
private:
    /** ENet host interfacing sockets. */
    ENetHost*  m_host;

    /** Where to log packets. If NULL for FILE* logging is disabled. */
    static Synchronised<FILE*> m_log_file;

public:
              Network(int peer_count, int channel_limit,
                      uint32_t max_incoming_bandwidth,
                      uint32_t max_outgoing_bandwidth,
                      ENetAddress* address = NULL);
    virtual  ~Network();

    static void openLog();
    static void logPacket(const BareNetworkString &ns, bool incoming);
    static void closeLog();
    ENetPeer *connectTo(const TransportAddress &address);
    void     sendRawPacket(const BareNetworkString &buffer,
                           const TransportAddress& dst);
    int receiveRawPacket(char *buffer, int buf_len,
                         TransportAddress* sender, int max_tries = -1);
    void     broadcastPacket(NetworkString *data,
                             bool reliable = true);
    // ------------------------------------------------------------------------
    /** Returns a pointer to the ENet host object. */
    ENetHost* getENetHost() { return m_host; }
};   // class Network

#endif // HEADER_ENET_SOCKET_HPP
