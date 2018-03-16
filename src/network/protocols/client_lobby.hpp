#ifndef CLIENT_LOBBY_HPP
#define CLIENT_LOBBY_HPP

#include "network/protocols/lobby_protocol.hpp"
#include "network/transport_address.hpp"
#include "utils/cpp2011.hpp"
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
    void becomingServerOwner();

    TransportAddress m_server_address;

    enum STATE
    {
        NONE,
        LINKED,
        REQUESTING_CONNECTION,
        CONNECTED,              // means in the lobby room
        KART_SELECTION,         // Start kart selection, then go to next state
        SELECTING_KARTS,        // in the network kart selection screen
        PLAYING,                // racing
        RACE_FINISHED,          // race result shown
        DONE,
        EXITING
    };

    /** The state of the finite state machine. */
    STATE m_state;

    std::set<std::string> m_available_karts;
    std::set<std::string> m_available_tracks;

    void addAllPlayers(Event* event);

public:
             ClientLobby();
    virtual ~ClientLobby();
    void setAddress(const TransportAddress &address);
    void doneWithResults();
    void startingRaceNow();
    const std::set<std::string>& getAvailableKarts() const
                                                  { return m_available_karts; }
    const std::set<std::string>& getAvailableTracks() const
                                                 { return m_available_tracks; }
    virtual bool notifyEvent(Event* event) OVERRIDE;
    virtual bool notifyEventAsynchronous(Event* event) OVERRIDE;
    virtual void finishedLoadingWorld() OVERRIDE;
    virtual void setup() OVERRIDE;
    virtual void update(float dt) OVERRIDE;
    virtual bool waitingForPlayers() const OVERRIDE
                                                  { return m_state == LINKED; }
    virtual void asynchronousUpdate() OVERRIDE {}

};

#endif // CLIENT_LOBBY_HPP
