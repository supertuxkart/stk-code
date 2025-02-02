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
#include "network/remote_kart_info.hpp"
#include "race/race_manager.hpp"
#include "race/kart_restriction.hpp"
#include "utils/cpp2011.hpp"
#include "utils/time.hpp"
#include "network/servers_manager.hpp"

#include "irrString.h"

#include <algorithm>
//#include <array>
#include <atomic>
#include <functional>
#include <limits>
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
    typedef std::map<STKPeer*,
                      std::weak_ptr<NetworkPlayerProfile>>
        PoleVoterMap;
    typedef std::map<STKPeer* const,
                      std::weak_ptr<NetworkPlayerProfile>>
        PoleVoterConstMap;
    typedef std::pair<STKPeer* const,
                      std::weak_ptr<NetworkPlayerProfile>>
        PoleVoterConstEntry;
    typedef std::pair<std::weak_ptr<NetworkPlayerProfile>,
                      unsigned int>
        PoleVoterResultEntry;
    typedef std::pair<const std::weak_ptr<NetworkPlayerProfile>,
                      unsigned int>
        PoleVoterConstResultEntry;


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

    /* Moderation toolkit */
    enum ServerPermissionLevel : int
    {
        PERM_NONE = -100,        // no chat, nothing
        PERM_SPECTATOR = -20,    // chat is allowed, can spectate the game
        PERM_PRISONER = -10,     // can play the game, but unable to change teams
        PERM_PLAYER = 0,         // can participate in the game and change teams
        PERM_MODERATOR = 80,     // staff, allowed to change other players perm status
                                 // (only less to the own)
                                 // and use general moderation commands such as 
                                 // mute, kick, ban.
        PERM_REFEREE = 90,       // only active during tournament
        PERM_ADMINISTRATOR = 100,// staff, can change current server's mode and toggle
                                 // between owner-less on or off, can disable command 
                                 // voting
        PERM_OWNER = std::numeric_limits<int>::max(),
                                 // Special peer, has all permissions,
                                 // including giving the administrator permission level.
                                 // Specified in the configuration file.
    };
private:
    bool m_random_karts_enabled;
    void assignRandomKarts();
    void resetKartSelections();
    std::string m_replay_dir;
    bool m_replay_requested = false;    
    std::string getTimeStamp();    
    std::string exec_python_script();    
    std::string currentTrackName;
    std::string currentPlayerName;
    std::string currentRecordTime;
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

    bool m_permissions_table_exists;

    bool m_restrictions_table_exists;

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

    // TierS additional members
    uint64_t m_last_wanrefresh_cmd_time;
    std::shared_ptr<ServerList> m_last_wanrefresh_res;
    std::weak_ptr<STKPeer> m_last_wanrefresh_requester;
    std::mutex m_wanrefresh_lock;

    // Pole
    bool m_pole_enabled = false;
    // For which player each peer submits a vote
    std::map<STKPeer*, std::weak_ptr<NetworkPlayerProfile>>
        m_blue_pole_votes;
    std::map<STKPeer*, std::weak_ptr<NetworkPlayerProfile>>
        m_red_pole_votes;

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
        std::vector<std::shared_ptr<NetworkPlayerProfile> >& players,
        unsigned int push_front_blue = 0,
        unsigned int push_front_red = 0) const;
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

    void updatePlayerList(bool update_when_reset_server = false);
    void updateServerOwner(std::shared_ptr<STKPeer> owner = nullptr);
    void updateTracksForMode();
    bool checkPeersReady(bool ignore_ai_peer) const;
    bool checkPeersCanPlay(bool ignore_ai_peer) const;
    char checkPeersCanPlayAndReady(bool ignore_ai_peer) const;
    void handleServerConfiguration(Event* event);
    void updateServerConfiguration(int new_difficulty, int new_game_mode,
            std::int8_t new_soccer_goal_target);
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

    void insertKartsIntoNotType(std::set<std::string>& set, const char* type) const;
    std::set<std::string> getOtherKartsThan(const std::string& name) const;
    const char* kartRestrictedTypeName(const enum KartRestrictionMode mode) const;
    enum KartRestrictionMode getKartRestrictionMode() const { return m_kart_restriction; }
    void setKartRestrictionMode(enum KartRestrictionMode mode);
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
    void broadcastMessageInGame(const irr::core::stringw& message);
    void setSaveServerConfig(bool val)          { m_save_server_config = val; }
    float getStartupBoostOrPenaltyForKart(uint32_t ping, unsigned kart_id);
    int getDifficulty() const                   { return m_difficulty.load(); }
    int getGameMode() const                      { return m_game_mode.load(); }
    int getLobbyPlayers() const              { return m_lobby_players.load(); }
    void saveInitialItems(std::shared_ptr<NetworkItemManager> nim);
    void saveIPBanTable(const SocketAddress& addr);
    void removeIPBanTable(const SocketAddress& addr);
    void listBanTable();
    void initServerStatsTable();
    bool isAIProfile(const std::shared_ptr<NetworkPlayerProfile>& npp) const
    {
        return std::find(m_ai_profiles.begin(), m_ai_profiles.end(), npp) !=
            m_ai_profiles.end();
    }
    uint32_t getServerIdOnline() const           { return m_server_id_online; }
    void setClientServerHostId(uint32_t id)   { m_client_server_host_id = id; }
    bool isVIP(std::shared_ptr<STKPeer>& peer) const;
    bool isVIP(STKPeer* peer) const;
    bool isTrusted(std::shared_ptr<STKPeer>& peer) const;
    bool isTrusted(STKPeer* peer) const;
    std::set<std::string> m_vip_players;
    std::set<std::string> m_trusted_players;
    std::set<std::string> m_red_team;
    std::set<std::string> m_blue_team;
    std::set<std::string> m_must_have_tracks;
    std::set<std::string> m_only_played_tracks;
    std::vector<std::vector<std::string>>
                          m_tournament_fields_per_game;
    bool serverAndPeerHaveTrack(std::shared_ptr<STKPeer>& peer, std::string track_id) const;
    bool serverAndPeerHaveTrack(STKPeer* peer, std::string track_id) const;
    /* forced track to be playing, or field */
    std::string m_set_field;
    /* forced laps, or forced minutes to play in case of the soccer game. */
    int         m_set_laps;
    /* for race it's reverse on/off, for battle/soccer it's random items */
    bool        m_set_specvalue;
    bool canRace(std::shared_ptr<STKPeer>& peer) const;
    bool canRace(STKPeer* peer) const;
    static int m_fixed_laps;
    void sendStringToPeer(const std::string& s, std::shared_ptr<STKPeer>& peer) const;
    void sendStringToPeer(const irr::core::stringw& s, std::shared_ptr<STKPeer>& peer) const;
    void sendStringToAllPeers(std::string& s);
    void sendRandomInstalladdonLine(STKPeer* peer) const;
    void sendRandomInstalladdonLine(std::shared_ptr<STKPeer> peer) const;
    void sendCurrentModifiers(STKPeer* peer) const;
    void sendCurrentModifiers(std::shared_ptr<STKPeer>& peer) const;
    void sendWANListToPeer(std::shared_ptr<STKPeer> peer);
    bool voteForCommand(std::shared_ptr<STKPeer>& peer, std::string command);
    NetworkString* addRandomInstalladdonMessage(NetworkString* ns) const;
    void addKartRestrictionMessage(std::string& msg) const;
    void addPowerupSMMessage(std::string& msg) const;
    const std::string getRandomAddon(RaceManager::MinorRaceModeType m=RaceManager::MINOR_MODE_NONE) const;
    bool isPoleEnabled() const { return m_pole_enabled; }
    core::stringw formatTeammateList(
            const std::vector<std::shared_ptr<NetworkPlayerProfile>> &team) const;
    void setPoleEnabled(bool mode);
    void submitPoleVote(std::shared_ptr<STKPeer>& voter, unsigned int vote);

    std::shared_ptr<NetworkPlayerProfile> decidePoleFor(const PoleVoterMap& mapping, KartTeam team) const;

    std::pair<
        std::shared_ptr<NetworkPlayerProfile>,
        std::shared_ptr<NetworkPlayerProfile>> decidePoles();
    void announcePoleFor(std::shared_ptr<NetworkPlayerProfile>& p, KartTeam team) const;

    /* Moderation toolkit */
    bool moderationToolkitAvailable()
    {
#ifdef ENABLE_SQLITE3
        return m_permissions_table_exists;
#else
        return false;
#endif
    }
    int getPeerPermissionLevel(STKPeer* p);
    int loadPermissionLevelForOID(uint32_t online_id);
    int loadPermissionLevelForUsername(const core::stringw& name);
    void writePermissionLevelForOID(uint32_t online_id, int lvl);
    void writePermissionLevelForUsername(const core::stringw& name, int lvl);
    std::tuple<uint32_t, std::string> loadRestrictionsForOID(uint32_t online_id);
    std::tuple<uint32_t, std::string> loadRestrictionsForUsername(const core::stringw& name);
    void writeRestrictionsForOID(uint32_t online_id, uint32_t flags);
    void writeRestrictionsForOID(uint32_t online_id, uint32_t flags, const std::string& set_kart);
    void writeRestrictionsForOID(uint32_t online_id, const std::string& set_kart);
    void writeRestrictionsForUsername(const core::stringw& name, uint32_t flags);
    void writeRestrictionsForUsername(const core::stringw& name, uint32_t flags, const std::string& set_kart);
    void writeRestrictionsForUsername(const core::stringw& name, const std::string& set_kart);
    void sendNoPermissionToPeer(STKPeer* p, const std::vector<std::string>& argv);
    const char* getPermissionLevelName(int lvl) const;
    ServerPermissionLevel getPermissionLevelByName(const std::string& name) const;
    const char* getRestrictionName(PlayerRestriction prf) const;
    const std::string formatRestrictions(PlayerRestriction prf) const;
    PlayerRestriction getRestrictionValue(const std::string& restriction) const;
    void forceChangeTeam(NetworkPlayerProfile* player, KartTeam team);
    void forceChangeHandicap(NetworkPlayerProfile* player, HandicapLevel lvl);
    bool forceSetTrack(std::string track_id, int laps, bool specvalue = false,
            bool is_soccer = false, bool announce = true);
    uint32_t lookupOID(const std::string& name);
    uint32_t lookupOID(const core::stringw& name);
    int banPlayer(const std::string& name, const std::string& reason, int days = -1);
    int unbanPlayer(const std::string& name);
    const std::string formatBanList(unsigned int page = 0, unsigned int psize = 8);
    const std::string formatBanInfo(const std::string& name);
    int64_t getTimeout();
    void changeTimeout(long timeout, bool infinite = false, bool absolute = false);

    std::map<std::string, std::vector<std::string>> m_command_voters;
    std::set<STKPeer*> m_team_speakers;
    //Deprecated
    //std::map<STKPeer*, std::set<irr::core::stringw>> m_message_receivers;
    int m_max_players;
    int m_max_players_in_game;
    bool m_powerupper_active = false;
    // TODO:
    enum KartRestrictionMode m_kart_restriction = NONE;
    bool m_allow_powerupper = false;
    bool m_show_elo = false;
    bool m_show_rank = false;
    int getMaxPlayers() const                                           { return m_max_players; }
    int getMaxPlayersInGame() const                                     { return m_max_players_in_game; }
    void setMaxPlayersInGame(int value, bool notify = true);
    std::string get_elo_change_string();
    std::string getPlayerAlt(std::string username) const;
    std::pair<unsigned int, int> getPlayerRanking(std::string username) const;
    std::pair<std::vector<std::string>, std::vector<std::string>> createBalancedTeams(std::vector<std::pair<std::string, int>>& elo_players);
    void soccer_ranked_make_teams(std::pair<std::vector<std::string>, std::vector<std::string>> teams, int min, std::vector <std::pair<std::string, int>> player_vec);

    void onTournamentGameEnded();
    void updateTournamentTeams(const std::string& team_red, const std::string& team_blue);
    bool isReplayRequested() const                                      { return m_replay_requested; }
    void setReplayRequested(const bool value)                           { m_replay_requested = value; }
};   // class ServerLobby

#endif // SERVER_LOBBY_HPP
