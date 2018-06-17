#ifndef SERVER_LOBBY_HPP
#define SERVER_LOBBY_HPP

#include "network/protocols/lobby_protocol.hpp"
#include "network/transport_address.hpp"
#include "utils/cpp2011.hpp"

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
        ACCEPTING_CLIENTS,        // In lobby, accepting clients
        SELECTING,                // kart, track, ... selection started
        LOAD_WORLD,               // Server starts loading world
        WAIT_FOR_WORLD_LOADED,    // Wait for clients and server to load world
        WAIT_FOR_RACE_STARTED,    // Wait for all clients to have started the race
        DELAY_SERVER,             // Additional server delay
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
    std::map<std::weak_ptr<STKPeer>, std::pair<bool, double>,
        std::owner_less<std::weak_ptr<STKPeer> > > m_peers_ready;

    /** Vote from each peer. */
    std::map<std::weak_ptr<STKPeer>, std::tuple<std::string, uint8_t, bool>,
        std::owner_less<std::weak_ptr<STKPeer> > > m_peers_votes;

    /** Keeps track of an artificial server delay (which makes sure that the
     *  data from all clients has arrived when the server computes a certain
     *  timestep.(. It stores the real time since epoch + delta (atm 0.1
     *  seconds), which is the real time at which the server should start. */
    double m_server_delay;

    bool m_has_created_server_id_file;

    /** It indicates if this server is unregistered with the stk server. */
    std::weak_ptr<bool> m_server_unregistered;

    /** Timeout counter for various state. */
    std::atomic<float> m_timeout;

    /** Lock this mutex whenever a client is connect / disconnect or
     *  starting race. */
    std::mutex m_connection_mutex;

    /** Ban list ip (in decimal) with online user id. */
    std::map<uint32_t, uint32_t> m_ban_list;

    TransportAddress m_server_address;

    std::mutex m_keys_mutex;

    std::map<uint32_t, KeyData> m_keys;

    std::map<std::weak_ptr<STKPeer>,
        std::pair<uint32_t, BareNetworkString>,
        std::owner_less<std::weak_ptr<STKPeer> > > m_pending_connection;

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
    void startedRaceOnClient(Event *event);
    void kickHost(Event* event);
    void handleChat(Event* event);
    void unregisterServer(bool now);
    void createServerIdFile();
    void updatePlayerList(bool force_update = false);
    void updateServerOwner();
    bool checkPeersReady() const
    {
        bool all_ready = true;
        for (auto p : m_peers_ready)
        {
            if (p.first.expired())
                continue;
            all_ready = all_ready && p.second.first;
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
                it->second.first = false;
                it->second.second = 0.0;
                it++;
            }
        }
    }
    void addAndReplaceKeys(const std::map<uint32_t, KeyData>& new_keys)
    {
        std::lock_guard<std::mutex> lock(m_keys_mutex);
        for (auto& k : new_keys)
            m_keys[k.first] = k.second;
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

public:
             ServerLobby();
    virtual ~ServerLobby();

    virtual bool notifyEventAsynchronous(Event* event) OVERRIDE;
    virtual bool notifyEvent(Event* event) OVERRIDE;
    virtual void setup() OVERRIDE;
    virtual void update(int ticks) OVERRIDE;
    virtual void asynchronousUpdate() OVERRIDE;

    void signalRaceStartToClients();
    void startSelection(const Event *event=NULL);
    void checkIncomingConnectionRequests();
    void finishedLoadingWorld() OVERRIDE;
    ServerState getCurrentState() const { return m_state.load(); }
    void updateBanList();
    virtual bool waitingForPlayers() const OVERRIDE;
    virtual bool allPlayersReady() const OVERRIDE
                            { return m_state.load() >= WAIT_FOR_RACE_STARTED; }

};   // class ServerLobby

#endif // SERVER_LOBBY_HPP
