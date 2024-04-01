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
#include "utils/cpp2011.hpp"
#include "utils/time.hpp"

#include "irrString.h"

#include <algorithm>
#include <array>
#include <atomic>
#include <functional>
#include <map>
#include <memory>
#include <mutex>
#include <set>

#ifdef ENABLE_SQLITE3
#include <sqlite3.h>
#endif

class BareNetworkString;
class NetworkItemManager;
class NetworkString;
class NetworkPlayerProfile;
class STKPeer;
class SocketAddress;

namespace Online
{
    class Request;
}

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
        std::string m_country_code;
        bool m_tried = false;
    };
    bool m_player_reports_table_exists;

#ifdef ENABLE_SQLITE3
    sqlite3* m_db;

    std::string m_server_stats_table;

    bool m_ip_ban_table_exists;

    bool m_ipv6_ban_table_exists;

    bool m_online_id_ban_table_exists;

    bool m_ip_geolocation_table_exists;

    bool m_ipv6_geolocation_table_exists;

    uint64_t m_last_poll_db_time;

    void pollDatabase();

    bool easySQLQuery(const std::string& query,
        std::function<void(sqlite3_stmt* stmt)> bind_function = nullptr) const;

    void checkTableExists(const std::string& table, bool& result);

    std::string ip2Country(const SocketAddress& addr) const;

    std::string ipv62Country(const SocketAddress& addr) const;
#endif
    void initDatabase();

    void destroyDatabase();

    std::atomic<ServerState> m_state;

    /* The state used in multiple threads when reseting server. */
    enum ResetState : unsigned int
    {
        RS_NONE, // Default state
        RS_WAITING, // Waiting for reseting finished
        RS_ASYNC_RESET // Finished reseting server in main thread, now async
                       // thread
    };

    std::atomic<ResetState> m_rs_state;

    /** Hold the next connected peer for server owner if current one expired
     * (disconnected). */
    std::weak_ptr<STKPeer> m_server_owner;

    /** AI peer which holds the list of reserved AI for dedicated server. */
    std::weak_ptr<STKPeer> m_ai_peer;

    /** AI profiles for all-in-one graphical client server, this will be a
     *  fixed count thorough the live time of server, which its value is
     *  configured in NetworkConfig. */
    std::vector<std::shared_ptr<NetworkPlayerProfile> > m_ai_profiles;

    std::atomic<uint32_t> m_server_owner_id;

    /** Official karts and tracks available in server. */
    std::pair<std::set<std::string>, std::set<std::string> > m_official_kts;

    /** Addon karts and tracks available in server. */
    std::pair<std::set<std::string>, std::set<std::string> > m_addon_kts;

    /** Addon arenas available in server. */
    std::set<std::string> m_addon_arenas;

    /** Addon soccers available in server. */
    std::set<std::string> m_addon_soccers;

    /** Available karts and tracks for all clients, this will be initialized
     *  with data in server first. */
    std::pair<std::set<std::string>, std::set<std::string> > m_available_kts;

    /** Keeps track of the server state. */
    std::atomic_bool m_server_has_loaded_world;

    bool m_registered_for_once_only;

    bool m_save_server_config;

    /** Counts how many peers have finished loading the world. */
    std::map<std::weak_ptr<STKPeer>, bool,
        std::owner_less<std::weak_ptr<STKPeer> > > m_peers_ready;

    std::map<std::weak_ptr<STKPeer>, std::set<irr::core::stringw>,
        std::owner_less<std::weak_ptr<STKPeer> > > m_peers_muted_players;

    std::weak_ptr<Online::Request> m_server_registering;

    /** Timeout counter for various state. */
    std::atomic<int64_t> m_timeout;

    std::mutex m_keys_mutex;

    std::map<uint32_t, KeyData> m_keys;

    std::map<std::weak_ptr<STKPeer>,
        std::pair<uint32_t, BareNetworkString>,
        std::owner_less<std::weak_ptr<STKPeer> > > m_pending_connection;

    std::map<std::string, uint64_t> m_pending_peer_connection;

    /* Ranking related variables */
    // If updating the base points, update the base points distribution in DB
    const double BASE_RANKING_POINTS    = 4000.0; // Given to a new player on 1st connection to a ranked server
    const double BASE_RATING_DEVIATION  = 1000.0; // Given to a new player on 1st connection to a ranked server
    const double MIN_RATING_DEVIATION   = 100.0; // A server cron job makes RD go up if a player is inactive
    const double BASE_RD_PER_DISCONNECT = 15.0;
    const double VAR_RD_PER_DISCONNECT  = 3.0;
    const double MAX_SCALING_TIME       = 360.0;
    const double BASE_POINTS_PER_SECOND = 0.18;
    const double HANDICAP_OFFSET        = 2000.0;

    /** Online id to profile map, handling disconnection in ranked server */
    std::map<uint32_t, std::weak_ptr<NetworkPlayerProfile> > m_ranked_players;

    /** Multi-session rating for each current player */
    std::map<uint32_t, double> m_raw_scores;

    /** The rating uncertainty for each current player */
    std::map<uint32_t, double> m_rating_deviations;

    /** A single number compounding "raw score" and RD,
      * for rating display to players and rankings */
    std::map<uint32_t, double> m_scores;

    /** The maximum rating obtained for each current player.
      * This is based on m_scores, not m_raw_scores */
    std::map<uint32_t, double> m_max_scores;

    /** Number of disconnects in the previous 64 ranked races for each current players */
    std::map<uint32_t, uint64_t> m_num_ranked_disconnects;

    /** Number of ranked races done for each current players */
    std::map<uint32_t, unsigned> m_num_ranked_races;

    /* Saved the last game result */
    NetworkString* m_result_ns;

    /* Used to make sure clients are having same item list at start */
    BareNetworkString* m_items_complete_state;

    std::atomic<uint32_t> m_server_id_online;

    std::atomic<uint32_t> m_client_server_host_id;

    std::atomic<int> m_difficulty;

    std::atomic<int> m_game_mode;

    std::atomic<int> m_lobby_players;

    std::atomic<int> m_current_ai_count;

    std::atomic<uint64_t> m_last_success_poll_time;

    uint64_t m_last_unsuccess_poll_time, m_server_started_at, m_server_delay;

    // Default game settings if no one has ever vote, and save inside here for
    // final vote (for live join)
    PeerVote* m_default_vote;

    int m_battle_hit_capture_limit;

    float m_battle_time_limit;

    unsigned m_item_seed;

    uint32_t m_winner_peer_id;

    uint64_t m_client_starting_time;

    // Calculated before each game started
    unsigned m_ai_count;

    // connection management
    void clientDisconnected(Event* event);
    void connectionRequested(Event* event);
    // kart selection
    void kartSelectionRequested(Event* event);
    // Track(s) votes
    void handlePlayerVote(Event *event);
    void playerFinishedResult(Event *event);
    void registerServer(bool first_time);
    void finishedLoadingWorldClient(Event *event);
    void finishedLoadingLiveJoinClient(Event *event);
    void kickHost(Event* event);
    void changeTeam(Event* event);
    void handleChat(Event* event);
    void unregisterServer(bool now,
        std::weak_ptr<ServerLobby> sl = std::weak_ptr<ServerLobby>());
    void updatePlayerList(bool update_when_reset_server = false);
    void updateServerOwner();
    void handleServerConfiguration(Event* event);
    void updateTracksForMode();
    bool checkPeersReady(bool ignore_ai_peer) const;
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
        m_pending_peer_connection[addr_str] = StkTime::getMonoTimeMs();
    }
    void removeExpiredPeerConnection()
    {
        // Remove connect to peer protocol running more than a 45 seconds
        // (from stk addons poll server request),
        for (auto it = m_pending_peer_connection.begin();
             it != m_pending_peer_connection.end();)
        {
            if (StkTime::getMonoTimeMs() - it->second > 45000)
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
                                     const irr::core::stringw& online_name,
                                     bool is_pending_connection,
                                     std::string country_code = "");
    bool decryptConnectionRequest(std::shared_ptr<STKPeer> peer,
                                  BareNetworkString& data,
                                  const std::string& key,
                                  const std::string& iv,
                                  uint32_t online_id,
                                  const irr::core::stringw& online_name,
                                  const std::string& country_code);
    bool handleAllVotes(PeerVote* winner, uint32_t* winner_peer_id);
    template<typename T>
    void findMajorityValue(const std::map<T, unsigned>& choices, unsigned cur_players,
                           T* best_choice, float* rate);
    void getRankingForPlayer(std::shared_ptr<NetworkPlayerProfile> p);
    void submitRankingsToAddons();
    void computeNewRankings();
    void clearDisconnectedRankedPlayer();
    double getModeFactor();
    double getModeSpread();
    double getTimeSpread(double time);
    double getUncertaintySpread(uint32_t online_id);
    double scalingValueForTime(double time);
    double computeH2HResult(double player1_time, double player2_time);
    double computeDataAccuracy(double player1_rd, double player2_rd,
                               double player1_scores, double player2_scores,
                               int player_count, bool handicap_used);
    void checkRaceFinished();
    void getHitCaptureLimit();
    void configPeersStartTime();
    void resetServer();
    void addWaitingPlayersToGame();
    void changeHandicap(Event* event);
    void handlePlayerDisconnection() const;
    void addLiveJoinPlaceholder(
        std::vector<std::shared_ptr<NetworkPlayerProfile> >& players) const;
    NetworkString* getLoadWorldMessage(
        std::vector<std::shared_ptr<NetworkPlayerProfile> >& players,
        bool live_join) const;
    void encodePlayers(BareNetworkString* bns,
        std::vector<std::shared_ptr<NetworkPlayerProfile> >& players) const;
    std::vector<std::shared_ptr<NetworkPlayerProfile> > getLivePlayers() const;
    void setPlayerKarts(const NetworkString& ns, STKPeer* peer) const;
    bool handleAssets(const NetworkString& ns, STKPeer* peer);
    void handleServerCommand(Event* event, std::shared_ptr<STKPeer> peer);
    void liveJoinRequest(Event* event);
    void rejectLiveJoin(STKPeer* peer, BackLobbyReason blr);
    bool canLiveJoinNow() const;
    bool worldIsActive() const;
    int getReservedId(std::shared_ptr<NetworkPlayerProfile>& p,
                      unsigned local_id) const;
    void handleKartInfo(Event* event);
    void clientInGameWantsToBackLobby(Event* event);
    void clientSelectingAssetsWantsToBackLobby(Event* event);
    std::set<std::shared_ptr<STKPeer>> getSpectatorsByLimit();
    void kickPlayerWithReason(STKPeer* peer, const char* reason) const;
    void testBannedForIP(STKPeer* peer) const;
    void testBannedForIPv6(STKPeer* peer) const;
    void testBannedForOnlineId(STKPeer* peer, uint32_t online_id) const;
    void writeDisconnectInfoTable(STKPeer* peer);
    void writePlayerReport(Event* event);
    bool supportsAI();
    void updateAddons();
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
    bool waitingForPlayers() const;
    virtual bool allPlayersReady() const OVERRIDE
                            { return m_state.load() >= WAIT_FOR_RACE_STARTED; }
    virtual bool isRacing() const OVERRIDE { return m_state.load() == RACING; }
    bool allowJoinedPlayersWaiting() const;
    void setSaveServerConfig(bool val)          { m_save_server_config = val; }
    float getStartupBoostOrPenaltyForKart(uint32_t ping, unsigned kart_id);
    int getDifficulty() const                   { return m_difficulty.load(); }
    int getGameMode() const                      { return m_game_mode.load(); }
    int getLobbyPlayers() const              { return m_lobby_players.load(); }
    void saveInitialItems(std::shared_ptr<NetworkItemManager> nim);
    void saveIPBanTable(const SocketAddress& addr);
    void listBanTable();
    void initServerStatsTable();
    bool isAIProfile(const std::shared_ptr<NetworkPlayerProfile>& npp) const
    {
        return std::find(m_ai_profiles.begin(), m_ai_profiles.end(), npp) !=
            m_ai_profiles.end();
    }
    uint32_t getServerIdOnline() const           { return m_server_id_online; }
    void setClientServerHostId(uint32_t id)   { m_client_server_host_id = id; }
    static int m_fixed_laps;
};   // class ServerLobby

#endif // SERVER_LOBBY_HPP
