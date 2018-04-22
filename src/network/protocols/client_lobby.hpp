#ifndef CLIENT_LOBBY_HPP
#define CLIENT_LOBBY_HPP

#include "network/protocols/lobby_protocol.hpp"
#include "network/transport_address.hpp"
#include "utils/cpp2011.hpp"

#include <atomic>
#include <set>

class ClientLobby : public LobbyProtocol
{
private:
    void disconnectedPlayer(Event* event);
    void connectionAccepted(Event* event); //!< Callback function on connection acceptation
    void connectionRefused(Event* event); //!< Callback function on connection refusal
    void startGame(Event* event);
    void startSelection(Event* event);
    void raceFinished(Event* event);
    void exitResultScreen(Event *event);
    // race votes
    void displayPlayerVote(Event* event);
    void updatePlayerList(Event* event);
    void handleChat(Event* event);
    void handleServerInfo(Event* event);
    void becomingServerOwner();

    void clearPlayers();

    TransportAddress m_server_address;

    enum ClientState : unsigned int
    {
        NONE,
        LINKED,
        REQUESTING_CONNECTION,
        CONNECTED,              // means in the lobby room
        SELECTING_ASSETS,       // in the kart selection or tracks screen
        PLAYING,                // racing
        RACE_FINISHED,          // race result shown
        DONE,
        EXITING
    };

    /** The state of the finite state machine. */
    std::atomic<ClientState> m_state;

    std::set<std::string> m_available_karts;
    std::set<std::string> m_available_tracks;

    bool m_received_server_result = false;

    void addAllPlayers(Event* event);

public:
             ClientLobby();
    virtual ~ClientLobby();
    void setAddress(const TransportAddress &address);
    void doneWithResults();
    bool receivedServerResult()            { return m_received_server_result; }
    void startingRaceNow();
    const std::set<std::string>& getAvailableKarts() const
                                                  { return m_available_karts; }
    const std::set<std::string>& getAvailableTracks() const
                                                 { return m_available_tracks; }
    virtual bool notifyEvent(Event* event) OVERRIDE;
    virtual bool notifyEventAsynchronous(Event* event) OVERRIDE;
    virtual void finishedLoadingWorld() OVERRIDE;
    virtual void setup() OVERRIDE;
    virtual void update(int ticks) OVERRIDE;
    virtual bool waitingForPlayers() const OVERRIDE
                                        { return m_state.load() == CONNECTED; }
    virtual void asynchronousUpdate() OVERRIDE {}
    virtual bool allPlayersReady() const OVERRIDE
                                          { return m_state.load() >= PLAYING; }
};

#endif // CLIENT_LOBBY_HPP
