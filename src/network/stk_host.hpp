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
#include "utils/time.hpp"

#include "irrString.h"

// enet.h includes win32.h, which without lean_and_mean includes
// winspool.h, which defines MAX_PRIORITY as a macro, which then
// results in request_manager.hpp not being compilable.
#define WIN32_LEAN_AND_MEAN
#include <enet/enet.h>

#include <atomic>
#include <cassert>
#include <list>
#include <functional>
#include <map>
#include <memory>
#include <mutex>
#include <set>
#include <thread>
#include <tuple>

class GameSetup;
class LobbyProtocol;
class NetworkPlayerProfile;
class NetworkTimerSynchronizer;
class Server;
class ServerLobby;
class SeparateProcess;

enum ENetCommandType : unsigned int
{
    ECT_SEND_PACKET = 0,
    ECT_DISCONNECT = 1,
    ECT_RESET = 2
};

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

private:
    /** Singleton pointer to the instance. */
    static STKHost* m_stk_host;

    /** Separate process of server instance. */
    SeparateProcess* m_separate_process;

    /** ENet host interfacing sockets. */
    Network* m_network;

    /** Network console thread */
    std::thread m_network_console;

    /** Make sure the removing or adding a peer is thread-safe. */
    mutable std::mutex m_peers_mutex;

    /** Let (atm enet_peer_send and enet_peer_disconnect) run in the listening
     *  thread. */
    std::list<std::tuple</*peer receive*/ENetPeer*,
        /*packet to send*/ENetPacket*, /*integer data*/uint32_t,
        ENetCommandType> > m_enet_cmd;

    /** Protect \ref m_enet_cmd from multiple threads usage. */
    std::mutex m_enet_cmd_mutex;

    /** The list of peers connected to this instance. */
    std::map<ENetPeer*, std::shared_ptr<STKPeer> > m_peers;

    /** Next unique host id. It is increased whenever a new peer is added (see
     *  getPeer()), but not decreased whena host (=peer) disconnects. This
     *  results in a unique host id for each host, even when a host should
     *  disconnect and then reconnect. */
    uint32_t m_next_unique_host_id = 0;

    /** Host id of this host. */
    uint32_t m_host_id = 0;

    /** Id of thread listening to enet events. */
    std::thread m_listening_thread;

    /** The private port enet socket is bound. */
    uint16_t m_private_port;

    /** Flag which is set from the protocol manager thread which
     *  triggers a shutdown of the STKHost (and the Protocolmanager). */
    std::atomic_bool m_shutdown;

    /** True if this local host is authorised to control a server. */
    std::atomic_bool m_authorised;

    /** Use as a timeout to waiting a disconnect event when exiting. */
    std::atomic<uint64_t> m_exit_timeout;

    /** An error message, which is set by a protocol to be displayed
     *  in the GUI. */
    irr::core::stringw m_error_message;

    /** The public address found by stun (if WAN is used). */
    TransportAddress m_public_address;

    /** The public IPv6 address found by stun (if WAN is used). */
    std::string m_public_ipv6_address;

    /** The public address stun server used. */
    TransportAddress m_stun_address;

    Synchronised<std::map<uint32_t, uint32_t> > m_peer_pings;

    std::atomic<uint32_t> m_client_ping;

    std::atomic<uint32_t> m_upload_speed;

    std::atomic<uint32_t> m_download_speed;

    std::atomic<uint32_t> m_players_in_game;

    std::atomic<uint32_t> m_players_waiting;

    std::atomic<uint32_t> m_total_players;

    std::atomic<int64_t> m_network_timer;

    std::unique_ptr<NetworkTimerSynchronizer> m_nts;

    // ------------------------------------------------------------------------
    STKHost(bool server);
    // ------------------------------------------------------------------------
    ~STKHost();
    // ------------------------------------------------------------------------
    void init();
    // ------------------------------------------------------------------------
    void handleDirectSocketRequest(Network* direct_socket,
                                   std::shared_ptr<ServerLobby> sl,
                                   std::map<std::string, uint64_t>& ctp);
    // ------------------------------------------------------------------------
    void mainLoop();
    // ------------------------------------------------------------------------
    std::string getIPFromStun(int socket, const std::string& stun_address,
                              bool ipv4);
public:
    /** If a network console should be started. */
    static bool m_enable_console;

    /** Creates the STKHost. It takes all confifguration parameters from
     *  NetworkConfig. This STKHost can either be a client or a server.
     */
    static std::shared_ptr<LobbyProtocol> create(SeparateProcess* p = NULL);
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
    const std::string& getPublicIPV6Address() const
                                              { return m_public_ipv6_address; }
    // ------------------------------------------------------------------------
    const TransportAddress& getStunAddress() const   { return m_stun_address; }
    // ------------------------------------------------------------------------
    uint16_t getPrivatePort() const                  { return m_private_port; }
    // ------------------------------------------------------------------------
    void setPrivatePort();
    // ------------------------------------------------------------------------
    void setPublicAddress();
    // ------------------------------------------------------------------------
    void disconnectAllPeers(bool timeout_waiting = false);
    // ------------------------------------------------------------------------
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
    //-------------------------------------------------------------------------
    void sendPacketToAllPeersInServer(NetworkString *data,
                                      bool reliable = true);
    // ------------------------------------------------------------------------
    void sendPacketToAllPeers(NetworkString *data, bool reliable = true);
    // ------------------------------------------------------------------------
    void sendPacketToAllPeersWith(std::function<bool(STKPeer*)> predicate,
                                  NetworkString* data, bool reliable = true);
    // ------------------------------------------------------------------------
    /** Returns true if this client instance is allowed to control the server.
     *  It will auto transfer ownership if previous server owner disconnected.
     */
    bool isAuthorisedToControl() const          { return m_authorised.load(); }
    // ------------------------------------------------------------------------
    /** Sets if this local host is authorised to control the server. */
    void setAuthorisedToControl(bool authorised)
                                            { m_authorised.store(authorised); }
    // ------------------------------------------------------------------------
    std::vector<std::shared_ptr<NetworkPlayerProfile> >
                                                  getAllPlayerProfiles() const;
    // ------------------------------------------------------------------------
    std::set<uint32_t> getAllPlayerOnlineIds() const;
    // ------------------------------------------------------------------------
    std::shared_ptr<STKPeer> findPeerByHostId(uint32_t id) const;
    // ------------------------------------------------------------------------
    std::shared_ptr<STKPeer> findPeerByName(const core::stringw& name) const;
    // ------------------------------------------------------------------------
    void sendPacketExcept(STKPeer* peer, NetworkString *data,
                          bool reliable = true);
    // ------------------------------------------------------------------------
    void setupClient(int peer_count, int channel_limit,
                     uint32_t max_incoming_bandwidth,
                     uint32_t max_outgoing_bandwidth);
    // ------------------------------------------------------------------------
    void startListening();
    // ------------------------------------------------------------------------
    void stopListening();
    // ------------------------------------------------------------------------
    bool peerExists(const TransportAddress& peer_address);
    // ------------------------------------------------------------------------
    bool isConnectedTo(const TransportAddress& peer_address);
    // ------------------------------------------------------------------------
    std::shared_ptr<STKPeer> getServerPeerForClient() const;
    // ------------------------------------------------------------------------
    void setErrorMessage(const irr::core::stringw &message);
    // ------------------------------------------------------------------------
    void addEnetCommand(ENetPeer* peer, ENetPacket* packet, uint32_t i,
                        ENetCommandType ect)
    {
        std::lock_guard<std::mutex> lock(m_enet_cmd_mutex);
        m_enet_cmd.emplace_back(peer, packet, i, ect);
    }
    // ------------------------------------------------------------------------
    /** Returns the last error (or "" if no error has happened). */
    const irr::core::stringw& getErrorMessage() const
                                                    { return m_error_message; }
    // ------------------------------------------------------------------------
    /** Returns true if a shutdown of the network infrastructure was
     *  requested. */
    bool requestedShutdown() const                { return m_shutdown.load(); }
    // ------------------------------------------------------------------------
    int receiveRawPacket(char *buffer, int buffer_len, 
                         TransportAddress* sender, int max_tries = -1)
    {
        return m_network->receiveRawPacket(buffer, buffer_len, sender,
                                           max_tries);
    }   // receiveRawPacket
    // ------------------------------------------------------------------------
    void sendRawPacket(const BareNetworkString &buffer,
                       const TransportAddress& dst)
    {
        m_network->sendRawPacket(buffer, dst);
    }  // sendRawPacket
    // ------------------------------------------------------------------------
    Network* getNetwork() const                           { return m_network; }
    // ------------------------------------------------------------------------
    /** Returns a copied list of peers. */
    std::vector<std::shared_ptr<STKPeer> > getPeers() const
    {
        std::lock_guard<std::mutex> lock(m_peers_mutex);
        std::vector<std::shared_ptr<STKPeer> > peers;
        for (auto p : m_peers)
        {
            peers.push_back(p.second);
        }
        return peers;
    }
    // ------------------------------------------------------------------------
    /** Returns the next (unique) host id. */
    unsigned int getNextHostId() const
    {
        assert(m_next_unique_host_id >= 0);
        return m_next_unique_host_id;
    }
    // ------------------------------------------------------------------------
    void setNextHostId(uint32_t id)             { m_next_unique_host_id = id; }
    // ------------------------------------------------------------------------
    /** Returns the number of currently connected peers. */
    unsigned int getPeerCount() const
    {
        std::lock_guard<std::mutex> lock(m_peers_mutex);
        return (unsigned)m_peers.size();
    }
    // ------------------------------------------------------------------------
    /** Sets the global host id of this host (client use). */
    void setMyHostId(uint32_t my_host_id)           { m_host_id = my_host_id; }
    // ------------------------------------------------------------------------
    /** Returns the host id of this host. */
    uint32_t getMyHostId() const                          { return m_host_id; }
    // ------------------------------------------------------------------------
    void sendToServer(NetworkString *data, bool reliable = true);
    // ------------------------------------------------------------------------
    bool isClientServer() const;
    // ------------------------------------------------------------------------
    bool hasServerAI() const;
    // ------------------------------------------------------------------------
    void setSeparateProcess(SeparateProcess* p)
    {
        assert(m_separate_process == NULL);
        m_separate_process = p;
    }
    // ------------------------------------------------------------------------
    void initClientNetwork(ENetEvent& event, Network* new_network);
    // ------------------------------------------------------------------------
    std::map<uint32_t, uint32_t> getPeerPings()
                                           { return m_peer_pings.getAtomic(); }
    // ------------------------------------------------------------------------
    uint32_t getClientPingToServer() const
                      { return m_client_ping.load(std::memory_order_relaxed); }
    // ------------------------------------------------------------------------
    NetworkTimerSynchronizer* getNetworkTimerSynchronizer() const
                                                        { return m_nts.get(); }
    // ------------------------------------------------------------------------
    uint64_t getNetworkTimer() const
                  { return StkTime::getMonoTimeMs() - m_network_timer.load(); }
    // ------------------------------------------------------------------------
    void setNetworkTimer(uint64_t ticks)
    {
        m_network_timer.store(
            (int64_t)StkTime::getMonoTimeMs() - (int64_t)ticks);
    }
    // ------------------------------------------------------------------------
    std::pair<int, int> getAllPlayersTeamInfo() const;
    // ------------------------------------------------------------------------
    /* Return upload speed in bytes per second. */
    unsigned getUploadSpeed() const           { return m_upload_speed.load(); }
    // ------------------------------------------------------------------------
    /* Return download speed in bytes per second. */
    unsigned getDownloadSpeed() const       { return m_download_speed.load(); }
    // ------------------------------------------------------------------------
    void updatePlayers(unsigned* ingame = NULL,
                       unsigned* waiting = NULL,
                       unsigned* total = NULL);
    // ------------------------------------------------------------------------
    uint32_t getPlayersInGame() const      { return m_players_in_game.load(); }
    // ------------------------------------------------------------------------
    uint32_t getWaitingPlayers() const     { return m_players_waiting.load(); }
    // ------------------------------------------------------------------------
    uint32_t getTotalPlayers() const         { return m_total_players.load(); }
    // ------------------------------------------------------------------------
    std::vector<std::shared_ptr<NetworkPlayerProfile> >
        getPlayersForNewGame() const;
};   // class STKHost

#endif // STK_HOST_HPP
