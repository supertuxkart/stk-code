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

#include "network/network.hpp"
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
public:
    /** \brief Defines three host types for the server.
    *  These values tells the host where he will accept connections from.
    */
    enum
    {
        HOST_ANY = 0,             //!< Any host.
        HOST_BROADCAST = 0xFFFFFFFF,    //!< Defines the broadcast address.
        PORT_ANY = 0              //!< Any port.
    };


    friend class STKPeer; // allow direct enet modifications in implementations

private:
    /** Singleton pointer to the instance. */
    static STKHost* m_stk_host;

    /** True if this host is a server, false otherwise. */
    static bool m_is_server;

    /** ENet host interfacing sockets. */
    Network* m_network;

    /** This computer's public IP address. With lock since it can
     *  be updated from a separate thread. */
    Synchronised<TransportAddress> m_public_address;

    /** Id of thread listening to enet events. */
    pthread_t*  m_listening_thread;

    /** Mutex used to stop this thread. */
    pthread_mutex_t m_exit_mutex;

    /** Maximum number of players on the server. */
    static int m_max_players;

             STKHost();
    virtual ~STKHost();

public:

    /** Creates the singleton. */
    static void create(bool is_server)
    {
        m_is_server = is_server;
        assert(m_stk_host == NULL);
        m_stk_host  = new STKHost();
    }   // create
    // ------------------------------------------------------------------------
    /** Returns the instance of STKHost. */
    static STKHost *get()
    {
        assert(m_stk_host != NULL);
        return m_stk_host;
    }   // get
    // ------------------------------------------------------------------------
    static void destroy()
    {
        assert(m_stk_host != NULL);
        delete m_stk_host;
        m_stk_host = NULL;
    }
    // ------------------------------------------------------------------------

    static void* mainLoop(void* self);

    void setPublicAddress(const TransportAddress& addr);

    void        setupServer(uint32_t address, uint16_t port,
                            int peer_count, int channel_limit,
                            uint32_t max_incoming_bandwidth,
                            uint32_t max_outgoing_bandwidth);
    void        setupClient(int peer_count, int channel_limit,
                            uint32_t max_incoming_bandwidth,
                            uint32_t max_outgoing_bandwidth);
    void        startListening();
    void        stopListening();
    uint8_t*    receiveRawPacket(const TransportAddress& sender,
                                 int max_tries = -1);
    bool        peerExists(const TransportAddress& peer_address);
    bool        isConnectedTo(const TransportAddress& peer_address);
    int         mustStopListening();
    uint16_t    getPort() const;

    // --------------------------------------------------------------------
    ENetPeer* connectTo(const TransportAddress &address)
    {
        return m_network->connectTo(address);
    }   // connectTo

    // --------------------------------------------------------------------
    uint8_t* receiveRawPacket(TransportAddress* sender)
    {
        return m_network->receiveRawPacket(sender);
    }   // receiveRawPacket

    // --------------------------------------------------------------------
    void broadcastPacket(const NetworkString& data,
                         bool reliable = true)
    {
        m_network->broadcastPacket(data, reliable);
    }   // broadcastPacket

    // --------------------------------------------------------------------
    void sendRawPacket(uint8_t* data, int length,
                       const TransportAddress& dst)
    {
        m_network->sendRawPacket(data, length, dst);
    }  // sendRawPacket

    // --------------------------------------------------------------------
    /** Returns the IP address of this host. */
    uint32_t  getAddress() const
    {
        return m_network->getENetHost()->address.host;
    }   // getAddress

    // --------------------------------------------------------------------
    /** Returns the public IP address (thread safe). The network manager
     *  is a friend of TransportAddress and so has access to the copy
     *  constructor, which is otherwise declared private. */
    const TransportAddress getPublicAddress()
    {
        m_public_address.lock();
        TransportAddress a;
        a.copy(m_public_address.getData());
        m_public_address.unlock();
        return a;
    }   // getPublicAddress

    // --------------------------------------------------------------------
    /** Sets the maximum number of players for this server. */
    static void setMaxPlayers(int n) { m_max_players = n; }

    // --------------------------------------------------------------------
    /** Returns the maximum number of players for this server. */
    static int getMaxPlayers() { return m_max_players; }

};   // class STKHost

#endif // STK_HOST_HPP
