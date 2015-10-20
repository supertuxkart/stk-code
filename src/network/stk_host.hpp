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

/*! \file stk_host.hpp
 *  \brief Defines an interface to use network low-level functions easily.
 */
#ifndef STK_HOST_HPP
#define STK_HOST_HPP

#include "network/network_string.hpp"
#include "network/types.hpp"
#include "utils/synchronised.hpp"

// enet.h includes win32.h, which without lean_and_mean includes
// winspool.h, which defines MAX_PRIORITY as a macro, which then
// results in request_manager.hpp not being compilable.
#define WIN32_LEAN_AND_MEAN
#include <enet/enet.h>

#include <pthread.h>

/*! \class STKHost
 *  \brief Represents the local host.
 *  This host is either a server host or a client host. A client host is in
 *  charge of connecting to a server. A server opens a socket for incoming
 *  connections.
 *  By default, this host will use ENet to exchange packets. It also defines an
 *  interface for ENet use. Nevertheless, this class can be used to send and/or
 *  receive packets whithout ENet adding its headers.
 *  This class is used by the Network Manager to send packets.
 */
class STKHost
{
    friend class STKPeer; // allow direct enet modifications in implementations

private:
    /** ENet host interfacing sockets. */
    ENetHost*       m_host;

    /** Id of thread listening to enet events. */
    pthread_t*      m_listening_thread;

    /** Mutex used to stop this thread. */
    pthread_mutex_t m_exit_mutex;

    //** Where to log packets. If NULL for FILE* logging is disabled. */
    static Synchronised<FILE*> m_log_file;

   public:
        /*! \enum HOST_TYPE
         *  \brief Defines three host types for the server.
         *  These values tells the host where he will accept connections from.
         */
        enum HOST_TYPE
        {
            HOST_ANY       = 0,             //!< Any host.
            HOST_BROADCAST = 0xFFFFFFFF,    //!< Defines the broadcast address.
            PORT_ANY       = 0              //!< Any port.
        };

        /*! \brief Constructor                                              */
        STKHost();
        /*! \brief Destructor                                               */
        virtual ~STKHost();

        static void logPacket(const NetworkString &ns, bool incoming);
        static void* mainLoop(void* self);

        void        setupServer(uint32_t address, uint16_t port,
                                int peer_count, int channel_limit,
                                uint32_t max_incoming_bandwidth,
                                uint32_t max_outgoing_bandwidth);
        void        setupClient(int peer_count, int channel_limit,
                                uint32_t max_incoming_bandwidth,
                                uint32_t max_outgoing_bandwidth);
        void        startListening();
        void        stopListening();
        void        sendRawPacket(uint8_t* data, int length,
                                  const TransportAddress& dst);
        uint8_t*    receiveRawPacket(TransportAddress* sender);
        uint8_t*    receiveRawPacket(const TransportAddress& sender,
                                     int max_tries = -1);
        void        broadcastPacket(const NetworkString& data,
                                    bool reliable = true);
        bool        peerExists(const TransportAddress& peer_address);
        bool        isConnectedTo(const TransportAddress& peer_address);
        int         mustStopListening();
        uint16_t    getPort() const;
        // --------------------------------------------------------------------
        uint32_t    getAddress() const { return m_host->address.host; }

};

#endif // STK_HOST_HPP
