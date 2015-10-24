#ifndef SERVER_LOBBY_ROOM_PROTOCOL_HPP
#define SERVER_LOBBY_ROOM_PROTOCOL_HPP

#include "network/protocols/lobby_room_protocol.hpp"

class ServerLobbyRoomProtocol : public LobbyRoomProtocol
{
private:
    uint8_t m_next_id; //!< Next id to assign to a peer.
    std::vector<uint32_t> m_incoming_peers_ids;
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


    enum STATE
    {
        NONE,
        GETTING_PUBLIC_ADDRESS,
        LAUNCHING_SERVER,
        WORKING,
        DONE,
        EXITING
    };
    STATE m_state;    public:
        ServerLobbyRoomProtocol();
        virtual ~ServerLobbyRoomProtocol();

        virtual bool notifyEventAsynchronous(Event* event);
        virtual void setup();
        virtual void update();
        virtual void asynchronousUpdate() {};

        void startGame();
        void startSelection();
        void checkIncomingConnectionRequests();
        void checkRaceFinished();


};
#endif // SERVER_LOBBY_ROOM_PROTOCOL_HPP
