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

/*! \file stk_host.hpp
 *  \brief Defines an interface to use network low-level functions easily.
 */
#ifndef STK_HOST_HPP
#define STK_HOST_HPP

#include "network/types.hpp"

#include "network/network_string.hpp"

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
        
        /*! \brief Log packets into a file
         *  \param ns : The data in the packet
         *  \param incoming : True if the packet comes from a peer.
         *  False if it's sent to a peer.
         */
        static void logPacket(const NetworkString& ns, bool incoming);

        /*! \brief Thread function checking if data is received.
         *  This function tries to get data from network low-level functions as
         *  often as possible. When something is received, it generates an
         *  event and passes it to the Network Manager.
         *  \param self : used to pass the ENet host to the function.
         */
        static void* receive_data(void* self);

        /*! \brief Setups the host as a server.
         *  \param address : The IPv4 address of incoming connections.
         *  \param port : The port on which the server listens.
         *  \param peer_count : The maximum number of peers.
         *  \param channel_limit : The maximum number of channels per peer.
         *  \param max_incoming_bandwidth : The maximum incoming bandwidth.
         *  \param max_outgoing_bandwidth : The maximum outgoing bandwidth.
         */
        void        setupServer(uint32_t address, uint16_t port,
                                int peer_count, int channel_limit,
                                uint32_t max_incoming_bandwidth,
                                uint32_t max_outgoing_bandwidth);
        /*! \brief Setups the host as a client.
         *  In fact there is only one peer connected to this host.
         *  \param peer_count : The maximum number of peers.
         *  \param channel_limit : The maximum number of channels per peer.
         *  \param max_incoming_bandwidth : The maximum incoming bandwidth.
         *  \param max_outgoing_bandwidth : The maximum outgoing bandwidth.
         */
        void        setupClient(int peer_count, int channel_limit,
                                uint32_t max_incoming_bandwidth,
                                uint32_t max_outgoing_bandwidth);

        /*! \brief Starts the listening of events from ENet.
         *  Starts a thread that updates it as often as possible.
         */
        void        startListening();
        /*! \brief Stops the listening of events from ENet.
         *  Stops the thread that was receiving events.
         */
        void        stopListening();

        /*! \brief Sends a packet whithout ENet adding its headers.
         *  This function is used in particular to achieve the STUN protocol.
         *  \param data : Data to send.
         *  \param length : Length of the sent data.
         *  \param dst : Destination of the packet.
         */
        void        sendRawPacket(uint8_t* data, int length,
                                TransportAddress dst);
        /*! \brief Receives a packet directly from the network interface.
         *  Receive a packet whithout ENet processing it.
         *  \return A string containing the data of the received packet.
         */
        uint8_t*    receiveRawPacket();
        uint8_t*    receiveRawPacket(TransportAddress* sender);
        /*! \brief Receives a packet directly from the network interface and
         *  filter its address.
         *  Receive a packet whithout ENet processing it. Checks that the
         *  sender of the packet is the one that corresponds to the sender
         *  parameter. Does not check the port right now.
         *  \param sender : Transport address of the original sender of the
         *  wanted packet.
         *  \param max_tries : Number of times we try to read data from the
         *  socket. This is aproximately the time we wait in milliseconds.
         *  -1 means eternal tries.
         *  \return A string containing the data of the received packet
         *  matching the sender's ip address.
         */
        uint8_t*    receiveRawPacket(TransportAddress sender, int max_tries = -1);
        /*! \brief Broadcasts a packet to all peers.
         *  \param data : Data to send.
         */
        void        broadcastPacket(const NetworkString& data, bool reliable = true);

        /*! \brief Tells if a peer is known.
         *  \return True if the peer is known, false elseway.
         */
        bool        peerExists(TransportAddress peer_address);
        /*! \brief Tells if a peer is known and connected.
         *  \return True if the peer is known and connected, false elseway.
         */
        bool        isConnectedTo(TransportAddress peer_address);

        /*! \brief Returns true when the thread should stop listening. */
        int         mustStopListening();
        /*! \brief Returns true when the thread has stopped listening. */
        bool        hasStoppedListening() const { return m_listening; }

        uint32_t    getAddress() const          { return m_host->address.host; }
        uint16_t    getPort() const;
    protected:
        ENetHost*   m_host;             //!< ENet host interfacing sockets.
        pthread_t*  m_listening_thread; //!< Thread listening network events.
        pthread_mutex_t m_exit_mutex;   //!< Mutex to kill properly the thread
        bool        m_listening;
        static FILE*       m_log_file;         //!< Where to log packets
        static pthread_mutex_t m_log_mutex;    //!< To write in the log only once at a time

};

#endif // STK_HOST_HPP
