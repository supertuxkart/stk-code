//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2018 SuperTuxKart-Team
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

#ifndef SERVER_LOBBY_HPP
#define SERVER_LOBBY_HPP

#include "network/protocols/lobby_protocol.hpp"
#include "network/transport_address.hpp"
#include "utils/cpp2011.hpp"
#include "utils/time.hpp"

#include "irrString.h"

#include <array>
#include <atomic>
#include <map>
#include <memory>
#include <mutex>
#include <set>
#include <tuple>

class BareNetworkString;
class NetworkString;
class NetworkPlayerProfile;
class STKPeer;

class ServerLobby : public LobbyProtocol
{
public:
    /* The state for a small finite state machine. */
    enum ServerState : unsigned int
    {
        SET_PUBLIC_ADDRESS,       // Waiting to receive its public ip address
        REGISTER_SELF_ADDRESS,    // Register with STK online server
        WAITING_FOR_START_GAME,   // In lobby, waiting for (auto) start game
        SELECTING,                // kart, track, ... selection started
        LOAD_WORLD,               // Server starts loading world
        WAIT_FOR_WORLD_LOADED,    // Wait for clients and server to load world
        WAIT_FOR_RACE_STARTED,    // Wait for all clients to have started the race
        RACING,                   // racing
        WAIT_FOR_RACE_STOPPED,    // Wait server for stopping all race protocols
        RESULT_DISPLAY,           // Show result screen
        ERROR_LEAVE,              // shutting down server
        EXITING
    };
private:
    struct KeyData
    {
        std::string m_aes_key;
        std::string m_aes_iv;
        irr::core::stringw m_name;
        bool m_tried = false;
    };

    std::atomic<ServerState> m_state;

    /** Hold the next connected peer for server owner if current one expired
     * (disconnected). */
    std::weak_ptr<STKPeer> m_server_owner;

    std::atomic<uint32_t> m_server_owner_id;

    /** Available karts and tracks for all clients, this will be initialized
     *  with data in server first. */
    std::pair<std::set<std::string>, std::set<std::string> > m_available_kts;

    /** Keeps track of the server state. */
    std::atomic_bool m_server_has_loaded_world;

    /** Counts how many peers have finished loading the world. */
    std::map<std::weak_ptr<STKPeer>, bool,
        std::owner_less<std::weak_ptr<STKPeer> > > m_peers_ready;

    /** Vote from each peer. */
    std::map<std::weak_ptr<STKPeer>, std::tuple<std::string, uint8_t, bool>,
        std::owner_less<std::weak_ptr<STKPeer> > > m_peers_votes;

    bool m_has_created_server_id_file;

    /** It indicates if this server is unregistered with the stk server. */
    std::weak_ptr<bool> m_server_unregistered;

    /** Timeout counter for various state. */
    std::atomic<int64_t> m_timeout;

    /** Lock this mutex whenever a client is connect / disconnect or
     *  starting race. */
    mutable std::mutex m_connection_mutex;

    /** Ban list of ip ranges. */
    std::map</*ip_start*/uint32_t, std::tuple</*ip_end*/uint32_t,
        /*CIDR*/std::string, /*expired time epoch*/uint32_t> >
        m_ip_ban_list;

    /** Ban list of online user id. */
    std::map<uint32_t, /*expired time epoch*/uint32_t> m_online_id_ban_list;

    TransportAddress m_server_address;

    std::mutex m_keys_mutex;

    std::map<uint32_t, KeyData> m_keys;

    std::map<std::weak_ptr<STKPeer>,
        std::pair<uint32_t, BareNetworkString>,
        std::owner_less<std::weak_ptr<STKPeer> > > m_pending_connection;

    std::map<std::string, uint64_t> m_pending_peer_connection;

    /* Ranking related variables */
    // If updating the base points, update the base points distribution in DB
    const double BASE_RANKING_POINTS   = 4000.0;
    const double MAX_SCALING_TIME      = 500.0;
    const double MAX_POINTS_PER_SECOND = 0.125;

    /** Online id to profile map, handling disconnection in ranked server */
    std::map<uint32_t, std::weak_ptr<NetworkPlayerProfile> > m_ranked_players;

    /** Multi-session ranking scores for each current player */
    std::map<uint32_t, double> m_scores;

    /** The maximum ranking scores achieved for each current player */
    std::map<uint32_t, double> m_max_scores;

    /** Number of ranked races done for each current players */
    std::map<uint32_t, unsigned> m_num_ranked_races;

    bool m_waiting_for_reset;

    NetworkString* m_result_ns;

    std::vector<std::weak_ptr<NetworkPlayerProfile> > m_waiting_players;

    std::atomic<uint32_t> m_waiting_players_counts;

    uint64_t m_server_started_at, m_server_delay;

    unsigned m_server_id_online;

    bool m_registered_for_once_only;

    bool m_save_server_config;

    // connection management
    void clientDisconnected(Event* event);
    void connectionRequested(Event* event);
    // kart selection
    void kartSelectionRequested(Event* event);
    // Track(s) votes
    void playerVote(Event *event);
    void playerFinishedResult(Event *event);
    bool registerServer();
    void finishedLoadingWorldClient(Event *event);
    void kickHost(Event* event);
    void changeTeam(Event* event);
    void handleChat(Event* event);
    void unregisterServer(bool now);
    void createServerIdFile();
    void updatePlayerList(bool update_when_reset_server = false);
    void updateServerOwner();
    bool checkPeersReady() const
    {
        bool all_ready = true;
        for (auto p : m_peers_ready)
        {
            if (p.first.expired())
                continue;
            all_ready = all_ready && p.second;
            if (!all_ready)
                return false;
        }
        return true;
    }
    void resetPeersReady()
    {
        for (auto it = m_peers_ready.begin(); it != m_peers_ready.end();)
        {
            if (it->first.expired())
            {
                it = m_peers_ready.erase(it);
            }
            else
            {
                it->second = false;
                it++;
            }
        }
    }
    void addPeerConnection(const std::string& addr_str)
    {
        m_pending_peer_connection[addr_str] = StkTime::getRealTimeMs();
    }
    void removeExpiredPeerConnection()
    {
        // Remove connect to peer protocol running more than a 45 seconds
        // (from stk addons poll server request),
        for (auto it = m_pending_peer_connection.begin();
             it != m_pending_peer_connection.end();)
        {
            if (StkTime::getRealTimeMs() - it->second > 45000)
                it = m_pending_peer_connection.erase(it);
            else
                it++;
        }
    }
    void replaceKeys(std::map<uint32_t, KeyData>& new_keys)
    {
        std::lock_guard<std::mutex> lock(m_keys_mutex);
        std::swap(m_keys, new_keys);
    }
    void handlePendingConnection();
    void handleUnencryptedConnection(std::shared_ptr<STKPeer> peer,
                                     BareNetworkString& data,
                                     uint32_t online_id,
                                     const irr::core::stringw& online_name);
    bool decryptConnectionRequest(std::shared_ptr<STKPeer> peer,
                                  BareNetworkString& data,
                                  const std::string& key,
                                  const std::string& iv,
                                  uint32_t online_id,
                                  const irr::core::stringw& online_name);
    std::tuple<std::string, uint8_t, bool, bool> handleVote();
    void getRankingForPlayer(std::shared_ptr<NetworkPlayerProfile> p);
    void submitRankingsToAddons();
    void computeNewRankings();
    void clearDisconnectedRankedPlayer();
    double computeRankingFactor(uint32_t online_id);
    double distributeBasePoints(uint32_t online_id);
    double getModeFactor();
    double getModeSpread();
    double scalingValueForTime(double time);
    void checkRaceFinished();
    void sendBadConnectionMessageToPeer(std::shared_ptr<STKPeer> p);
    std::pair<int, float> getHitCaptureLimit(float num_karts);
    void configPeersStartTime();
    void updateWaitingPlayers();
    void resetServer();
    void addWaitingPlayersToGame();
public:
             ServerLobby();
    virtual ~ServerLobby();

    virtual bool notifyEventAsynchronous(Event* event) OVERRIDE;
    virtual bool notifyEvent(Event* event) OVERRIDE;
    virtual void setup() OVERRIDE;
    virtual void update(int ticks) OVERRIDE;
    virtual void asynchronousUpdate() OVERRIDE;

    void startSelection(const Event *event=NULL);
    void checkIncomingConnectionRequests();
    void finishedLoadingWorld() OVERRIDE;
    ServerState getCurrentState() const { return m_state.load(); }
    void updateBanList();
    std::unique_lock<std::mutex> acquireConnectionMutex() const
                   { return std::unique_lock<std::mutex>(m_connection_mutex); }
    bool waitingForPlayers() const;
    uint32_t getWaitingPlayersCount() const
                                    { return m_waiting_players_counts.load(); }
    virtual bool allPlayersReady() const OVERRIDE
                            { return m_state.load() >= WAIT_FOR_RACE_STARTED; }
    virtual bool isRacing() const OVERRIDE { return m_state.load() == RACING; }
    bool isBannedForIP(const TransportAddress& addr) const;
    bool allowJoinedPlayersWaiting() const;
    void setSaveServerConfig(bool val)          { m_save_server_config = val; }
    float getStartupBoostOrPenaltyForKart(uint32_t ping, unsigned kart_id);
};   // class ServerLobby

#endif // SERVER_LOBBY_HPP
