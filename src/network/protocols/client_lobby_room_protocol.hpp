#ifndef CLIENT_LOBBY_ROOM_PROTOCOL_HPP
#define CLIENT_LOBBY_ROOM_PROTOCOL_HPP

#include "network/protocols/lobby_room_protocol.hpp"
#include "network/transport_address.hpp"

class STKPeer;

class ClientLobbyRoomProtocol : public LobbyRoomProtocol
{
private:
    void newPlayer(Event* event);
    void disconnectedPlayer(Event* event);
    void connectionAccepted(Event* event); //!< Callback function on connection acceptation
    void connectionRefused(Event* event); //!< Callback function on connection refusal
    void kartSelectionRefused(Event* event);
    void kartSelectionUpdate(Event* event);
    void startGame(Event* event);
    void startSelection(Event* event);
    void raceFinished(Event* event);
    // race votes
    void playerMajorVote(Event* event);
    void playerRaceCountVote(Event* event);
    void playerMinorVote(Event* event);
    void playerTrackVote(Event* event);
    void playerReversedVote(Event* event);
    void playerLapsVote(Event* event);

    TransportAddress m_server_address;

    STKPeer* m_server;

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

public:
    ClientLobbyRoomProtocol(const TransportAddress& server_address);
    virtual ~ClientLobbyRoomProtocol();

    void requestKartSelection(uint8_t player_id,
        const std::string &kart_name);
    void voteMajor(uint8_t player_id, uint32_t major);
    void voteRaceCount(uint8_t player_id, uint8_t count);
    void voteMinor(uint8_t player_id, uint32_t minor);
    void voteTrack(uint8_t player_id, const std::string &track,
                   uint8_t track_nb = 0);
    void voteReversed(uint8_t player_id, bool reversed, uint8_t track_nb = 0);
    void voteLaps(uint8_t player_id, uint8_t laps, uint8_t track_nb = 0);
    void leave();

    virtual bool notifyEvent(Event* event);
    virtual bool notifyEventAsynchronous(Event* event);
    virtual void setup();
    virtual void update();
    virtual void asynchronousUpdate() {}

};

#endif // CLIENT_LOBBY_ROOM_PROTOCOL_HPP
