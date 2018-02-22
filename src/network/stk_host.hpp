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
#include "network/network_config.hpp"
#include "network/network_string.hpp"
#include "network/servers_manager.hpp"
#include "network/stk_peer.hpp"
#include "network/transport_address.hpp"
#include "utils/synchronised.hpp"

#include "irrString.h"

// enet.h includes win32.h, which without lean_and_mean includes
// winspool.h, which defines MAX_PRIORITY as a macro, which then
// results in request_manager.hpp not being compilable.
#define WIN32_LEAN_AND_MEAN
#include <enet/enet.h>

#include <atomic>
#include <thread>

class GameSetup;
class NetworkConsole;

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

    /** ENet host interfacing sockets. */
    Network* m_network;

    /** Network console */
    NetworkConsole *m_network_console;

    /** The list of peers connected to this instance. */
    std::vector<STKPeer*> m_peers;

    /** Next unique host id. It is increased whenever a new peer is added (see
     *  getPeer()), but not decreased whena host (=peer) disconnects. This
     *  results in a unique host id for each host, even when a host should
     *  disconnect and then reconnect. */
    int m_next_unique_host_id;

    /** Host id of this host. */
    uint8_t m_host_id;

    /** Stores data about the online game to play. */
    GameSetup* m_game_setup;

    /** Id of thread listening to enet events. */
    std::thread m_listening_thread;

    /** Flag which is set from the protocol manager thread which
     *  triggers a shutdown of the STKHost (and the Protocolmanager). */
    std::atomic_bool m_shutdown;

    /** Atomic flag used to stop this thread. */
    std::atomic_flag m_exit_flag = ATOMIC_FLAG_INIT;

    /** If this is a server, it indicates if this server is registered
     *  with the stk server. */
    bool m_is_registered;

    /** An error message, which is set by a protocol to be displayed
     *  in the GUI. */
    irr::core::stringw m_error_message;

    /** The public address found by stun (if WAN is used). */
    TransportAddress m_public_address;

    /** The private port enet socket is bound. */
    uint16_t m_private_port;

    /** An error message, which is set by a protocol to be displayed
     *  in the GUI. */

             STKHost(uint32_t server_id, uint32_t host_id);
             STKHost(const irr::core::stringw &server_name);
    virtual ~STKHost();
    void init();
    void handleDirectSocketRequest(Network* lan_network);
    // ------------------------------------------------------------------------
    void setPublicAddress();
    // ------------------------------------------------------------------------
    void mainLoop();

public:
    /** If a network console should be started. Note that the console can cause
    *  a crash in release mode on windows (see #1529). */
    static bool m_enable_console;


    /** Creates the STKHost. It takes all confifguration parameters from
     *  NetworkConfig. This STKHost can either be a client or a server.
     */
    static void create();

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
    }   // destroy
    // ------------------------------------------------------------------------
    /** Checks if the STKHost has been created. */
    static bool existHost() { return m_stk_host != NULL; }
    // ------------------------------------------------------------------------
    const TransportAddress& getPublicAddress() const
                                                   { return m_public_address; }
    // ------------------------------------------------------------------------
    uint16_t getPrivatePort() const
                                                     { return m_private_port; }
    // ------------------------------------------------------------------------
    void setPrivatePort();
    // ------------------------------------------------------------------------
    virtual GameSetup* setupNewGame();
    void abort();
    void deleteAllPeers();
    bool connect(const TransportAddress& peer);

    //-------------------------------------------------------------------------
    /** Requests that the network infrastructure is to be shut down. This
    *   function is called from a thread, but the actual shutdown needs to be
    *   done from the main thread to avoid race conditions (e.g.
    *   ProtocolManager might still access data structures when the main thread
    *   tests if STKHost exist (which it does, but ProtocolManager might be
    *   shut down already.
    */
    void requestShutdown()
    {
        m_shutdown.store(true);
    }   // requestExit
    //-------------------------------------------------------------------------
    void shutdown();

    void sendPacketExcept(STKPeer* peer,
                          NetworkString *data,
                          bool reliable = true);
    void        setupClient(int peer_count, int channel_limit,
                            uint32_t max_incoming_bandwidth,
                            uint32_t max_outgoing_bandwidth);
    void        startListening();
    void        stopListening();
    bool        peerExists(const TransportAddress& peer_address);
    void        removePeer(const STKPeer* peer);
    bool        isConnectedTo(const TransportAddress& peer_address);
    STKPeer    *getPeer(ENetPeer *enet_peer);
    STKPeer    *getServerPeerForClient() const;
    std::vector<NetworkPlayerProfile*> getMyPlayerProfiles();
    void        setErrorMessage(const irr::core::stringw &message);
    bool        isAuthorisedToControl() const;
    const irr::core::stringw& 
                getErrorMessage() const;

    // --------------------------------------------------------------------
    /** Returns true if a shutdown of the network infrastructure was
     *  requested. */
    bool requestedShutdown() const { return m_shutdown.load(); }
    // --------------------------------------------------------------------
    /** Returns the current game setup. */
    GameSetup* getGameSetup() { return m_game_setup; }
    // --------------------------------------------------------------------
    int receiveRawPacket(char *buffer, int buffer_len, 
                         TransportAddress* sender, int max_tries = -1)
    {
        return m_network->receiveRawPacket(buffer, buffer_len, sender,
                                           max_tries);
    }   // receiveRawPacket

    // --------------------------------------------------------------------
    void sendRawPacket(const BareNetworkString &buffer,
                       const TransportAddress& dst)
    {
        m_network->sendRawPacket(buffer, dst);
    }  // sendRawPacket

    // --------------------------------------------------------------------
    /** Returns the IP address of this host. */
    uint32_t  getAddress() const
    {
        return m_network->getENetHost()->address.host;
    }   // getAddress


    // --------------------------------------------------------------------
    /** Returns a const reference to the list of peers. */
    const std::vector<STKPeer*> &getPeers() { return m_peers; }
    // --------------------------------------------------------------------
    /** Returns the next (unique) host id. */
    unsigned int getNextHostId() const
    {
        assert(m_next_unique_host_id >= 0);
        return m_next_unique_host_id;
    }
    // --------------------------------------------------------------------
    /** Returns the number of currently connected peers. */
    unsigned int getPeerCount() { return (int)m_peers.size(); }
    // --------------------------------------------------------------------
    /** Sets if this server is registered with the stk server. */
    void setRegistered(bool registered)
    {
        m_is_registered = registered; 
    }   // setRegistered
    // --------------------------------------------------------------------
    /** Returns if this server is registered with the stk server. */
    bool isRegistered() const
    {
        return m_is_registered;
    }   // isRegistered
    // --------------------------------------------------------------------
    /** Sets the global host id of this host. */
    void setMyHostId(uint8_t my_host_id) { m_host_id = my_host_id; }
    // --------------------------------------------------------------------
    /** Returns the host id of this host. */
    uint8_t getMyHostId() const { return m_host_id; }
    // --------------------------------------------------------------------
    /** Sends a message from a client to the server. */
    void sendToServer(NetworkString *data, bool reliable = true)
    {
        assert(NetworkConfig::get()->isClient());
        m_peers[0]->sendPacket(data, reliable);
    }   // sendToServer
};   // class STKHost

#endif // STK_HOST_HPP
