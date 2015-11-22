#ifndef SERVER_LOBBY_ROOM_PROTOCOL_HPP
#define SERVER_LOBBY_ROOM_PROTOCOL_HPP

#include "network/protocols/lobby_room_protocol.hpp"
#include "utils/cpp2011.hpp"

class ServerLobbyRoomProtocol : public LobbyRoomProtocol
                              , public CallbackObject
{
private:
    /* The state for a small finite state machine. */
    enum
    {
        NONE,
        GETTING_PUBLIC_ADDRESS,
        WORKING,
        DONE,
        EXITING
    } m_state;

    /** Next id to assign to a peer. */
    uint8_t m_next_id;

    Protocol *m_current_protocol;
    bool m_selection_enabled;
    bool m_in_race;

    // connection management
    void kartDisconnected(Event* event);
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
    void registerServer();

public:
             ServerLobbyRoomProtocol();
    virtual ~ServerLobbyRoomProtocol();

    virtual bool notifyEventAsynchronous(Event* event);
    virtual void setup();
    virtual void update();
    virtual void asynchronousUpdate() {};

    void startGame();
    void startSelection(const Event *event=NULL);
    void checkIncomingConnectionRequests();
    void checkRaceFinished();

    virtual void callback(Protocol *protocol) OVERRIDE;

};   // class ServerLobbyRoomProtocol

#endif // SERVER_LOBBY_ROOM_PROTOCOL_HPP
