#ifndef SERVER_LOBBY_ROOM_PROTOCOL_HPP
#define SERVER_LOBBY_ROOM_PROTOCOL_HPP

#include "network/protocols/lobby_room_protocol.hpp"
#include "utils/cpp2011.hpp"
#include "utils/synchronised.hpp"

class ServerLobbyRoomProtocol : public LobbyRoomProtocol
                              , public CallbackObject
{
private:
    /* The state for a small finite state machine. */
    enum
    {
        NONE,
        GETTING_PUBLIC_ADDRESS,   // Waiting to receive its public ip address
        ACCEPTING_CLIENTS,        // In lobby, accepting clients
        SELECTING,                // kart, track, ... selection started
        RACING,                   // racing
        RESULT_DISPLAY,           // Show result screen
        DONE,                     // shutting down server
        EXITING
    } m_state;

    /** Next id to assign to a peer. */
    Synchronised<int> m_next_player_id;

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

public:
             ServerLobbyRoomProtocol();
    virtual ~ServerLobbyRoomProtocol();

    virtual bool notifyEventAsynchronous(Event* event) OVERRIDE;
    virtual void setup() OVERRIDE;
    virtual void update() OVERRIDE;
    virtual void asynchronousUpdate() OVERRIDE {};

    void startGame();
    void startSelection(const Event *event=NULL);
    void checkIncomingConnectionRequests();
    void checkRaceFinished();

    virtual void callback(Protocol *protocol) OVERRIDE;

};   // class ServerLobbyRoomProtocol

#endif // SERVER_LOBBY_ROOM_PROTOCOL_HPP
