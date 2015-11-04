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
#include "network/transport_address.hpp"
#include "utils/synchronised.hpp"

#include "irrString.h"

// enet.h includes win32.h, which without lean_and_mean includes
// winspool.h, which defines MAX_PRIORITY as a macro, which then
// results in request_manager.hpp not being compilable.
#define WIN32_LEAN_AND_MEAN
#include <enet/enet.h>

#include <pthread.h>

class GameSetup;
class NetworkConsole;
class STKPeer;

/** \class STKHost
 *  \brief Represents the local host. It is the main managing point for 
 *  networking. It is responsible for sending and receiving messages,
 *  and keeping track of onnected peers. It also provides some low
 *  level socket functions (i.e. to avoid that enet adds its headers
 *  to messages, useful for broadcast in LAN and for stun).
 *  This host is either a server host or a client host. A client host is in
 *  charge of connecting to a server. A server opens a socket for incoming
 *  connections. 
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

    /** Network console */
    NetworkConsole *m_network_console;

    /** The list of peers connected to this instance. */
    std::vector<STKPeer*> m_peers;

    /** This computer's public IP address. With lock since it can
     *  be updated from a separate thread. */
    Synchronised<TransportAddress> m_public_address;

    /** Stores data about the online game to play. */
    GameSetup* m_game_setup;

    /** Id of thread listening to enet events. */
    pthread_t*  m_listening_thread;

    /** Mutex used to stop this thread. */
    pthread_mutex_t m_exit_mutex;

    /** Maximum number of players on the server. */
    static int m_max_players;

    /** If this is a server, the server name. */
    irr::core::stringw m_server_name;

    enum NetworkType
    { NETWORK_NONE, NETWORK_WAN, NETWORK_LAN };

    /** Keeps the type of network connection: none (yet), LAN or WAN. */
    static NetworkType m_network_type;

             STKHost(uint32_t server_id, uint32_t host_id);
             STKHost(const irr::core::stringw &server_name);
    virtual ~STKHost();
    void init();

public:

    /** Creates the singleton for a client. In case of an error
     *  m_stk_host is NULL (which can be tested using isNetworking(). */
    static void create(uint32_t server_id, uint32_t host_id)
    {
        assert(m_stk_host == NULL);
        m_stk_host = new STKHost(server_id, host_id);
        if(!m_stk_host->m_network)
        {
            delete m_stk_host;
            m_stk_host = NULL;
        }
    }   // create
    // ------------------------------------------------------------------------
    /** Creates the singleton for a server. In case of an error
     *  m_stk_host is NULL (which can be tested using isNetworking(). */
    static void create(const irr::core::stringw &server_name)
    {
        assert(m_stk_host == NULL);
        m_stk_host  = new STKHost(server_name);
        if(!m_stk_host->m_network)
        {
            delete m_stk_host;
            m_stk_host = NULL;
        }
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
    /** Return if a network setting is happening. A network setting is active
     *  if a host (server or client) exists. */
    static bool isNetworking() { return m_stk_host!=NULL; }
    // ------------------------------------------------------------------------
    /** Return true if it's a networked game with a LAN server. */
    static bool isLAN() { return m_network_type == NETWORK_LAN; }
    // ------------------------------------------------------------------------
    /** Return true if it's a networked game but with a WAN server. */
    static bool isWAN() { return m_network_type == NETWORK_WAN; }
    // ------------------------------------------------------------------------
    static void setIsLAN() { m_network_type = NETWORK_LAN; }
    // ------------------------------------------------------------------------
    static void setIsWAN() { m_network_type = NETWORK_WAN; }
    // ------------------------------------------------------------------------

    static void* mainLoop(void* self);

    virtual GameSetup* setupNewGame();
    void setPublicAddress(const TransportAddress& addr);
    void abort();
    void deleteAllPeers();
    void reset();
    bool connect(const TransportAddress& peer);

    void sendMessage(const NetworkString& data, bool reliable = true);
    void sendPacketExcept(STKPeer* peer,
                          const NetworkString& data,
                          bool reliable = true);
    void        setupClient(int peer_count, int channel_limit,
                            uint32_t max_incoming_bandwidth,
                            uint32_t max_outgoing_bandwidth);
    void        startListening();
    void        stopListening();
    uint8_t*    receiveRawPacket(const TransportAddress& sender,
                                 int max_tries = -1);
    bool        peerExists(const TransportAddress& peer_address);
    void        removePeer(const STKPeer* peer);
    bool        isConnectedTo(const TransportAddress& peer_address);
    int         mustStopListening();
    uint16_t    getPort() const;

    // --------------------------------------------------------------------
    /** Returns the current game setup. */
    GameSetup* getGameSetup() { return m_game_setup; }
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

    // --------------------------------------------------------------------
    /** Returns a const reference to the list of peers. */
    const std::vector<STKPeer*> &getPeers() { return m_peers; }

    // --------------------------------------------------------------------
    /** Returns the number of currently connected peers. */
    unsigned int getPeerCount() { return (int)m_peers.size(); }
    // --------------------------------------------------------------------
    /** Returns if this instance is a server. */
    static bool isServer() { return m_is_server;  }
    // --------------------------------------------------------------------
    /** Returns if this instance is a client. */
    static bool isclient() { return !m_is_server; }
    // --------------------------------------------------------------------
    /** Sets the name of the server if this instance is a server. */
    void setServerName(const irr::core::stringw &name)
    {
        assert(isServer());
        m_server_name = name;
    }
    // --------------------------------------------------------------------
    /** Returns the server name. */
    const irr::core::stringw& getServerName() const
    {
        assert(isServer());
        return m_server_name;
    }
    // --------------------------------------------------------------------

};   // class STKHost

#endif // STK_HOST_HPP
