#ifndef SERVER_LOBBY_HPP
#define SERVER_LOBBY_HPP

#include "network/protocols/lobby_protocol.hpp"
#include "utils/cpp2011.hpp"
#include "utils/synchronised.hpp"

class ServerLobby : public LobbyProtocol
                  , public CallbackObject
{
private:
    /* The state for a small finite state machine. */
    enum
    {
        INIT_WAN,                 // Start state for WAN game
        GETTING_PUBLIC_ADDRESS,   // Waiting to receive its public ip address
        ACCEPTING_CLIENTS,        // In lobby, accepting clients
        SELECTING,                // kart, track, ... selection started
        LOAD_WORLD,               // Server starts loading world
        WAIT_FOR_WORLD_LOADED,    // Wait for clients and server to load world
        WAIT_FOR_RACE_STARTED,    // Wait for all clients to have started the race
        START_RACE,               // Inform clients to start race
        DELAY_SERVER,             // Additional server delay
        RACING,                   // racing
        RESULT_DISPLAY,           // Show result screen
        DONE,                     // shutting down server
        EXITING
    } m_state;

    /** Next id to assign to a peer. */
    Synchronised<int> m_next_player_id;

    /** Keeps track of the server state. */
    bool m_server_has_loaded_world;

    /** Counts how many clients have finished loading the world. */
    Synchronised<int> m_client_ready_count;

    /** For debugging: keep track of the state (ready or not) of each player,
     *  to make sure no client/player reports more than once. Needs to be a
     *  map since the client IDs can be non-consecutive. */
    std::map<uint8_t, bool> m_player_states;

    /** Keeps track of an artificial server delay (which makes sure that the
     *  data from all clients has arrived when the server computes a certain
     *  timestep. */
    float m_server_delay;

    Protocol *m_current_protocol;
    bool m_selection_enabled;

    /** Counts how many players are ready to go on. */
    int m_player_ready_counter;

    /** Timeout counter for showing the result screen. */
    float m_timeout;

    // connection management
    void clientDisconnected(Event* event);
    void connectionRequested(Event* event);
    // kart selection
    void kartSelectionRequested(Event* event);
    // race votes
    void playerMajorVote(Event* event);
    void playerRaceCountVote(Event* event);
    void playerMinorVote(Event* event);
    void playerTrackVote(Event* event);
    void playerReversedVote(Event* event);
    void playerLapsVote(Event* event);
    void playerFinishedResult(Event *event);
    void registerServer();
    void finishedLoadingWorldClient(Event *event);
    void startedRaceOnClient(Event *event);
public:
             ServerLobby();
    virtual ~ServerLobby();

    virtual bool notifyEventAsynchronous(Event* event) OVERRIDE;
    virtual void setup() OVERRIDE;
    virtual void update(float dt) OVERRIDE;
    virtual void asynchronousUpdate() OVERRIDE {};

    void signalRaceStartToClients();
    void startSelection(const Event *event=NULL);
    void checkIncomingConnectionRequests();
    void checkRaceFinished();
    void finishedLoadingWorld();

    virtual void callback(Protocol *protocol) OVERRIDE;

};   // class ServerLobby

#endif // SERVER_LOBBY_HPP
