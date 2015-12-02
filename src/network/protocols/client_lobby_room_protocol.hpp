#ifndef CLIENT_LOBBY_ROOM_PROTOCOL_HPP
#define CLIENT_LOBBY_ROOM_PROTOCOL_HPP

#include "network/protocols/lobby_room_protocol.hpp"
#include "network/transport_address.hpp"

class STKPeer;

class ClientLobbyRoomProtocol : public LobbyRoomProtocol
{
    public:
        ClientLobbyRoomProtocol(const TransportAddress& server_address);
        virtual ~ClientLobbyRoomProtocol();

        void requestKartSelection(const std::string &kart_name);
        void voteMajor(uint32_t major);
        void voteRaceCount(uint8_t count);
        void voteMinor(uint32_t minor);
        void voteTrack(const std::string &track, uint8_t track_nb = 0);
        void voteReversed(bool reversed, uint8_t track_nb = 0);
        void voteLaps(uint8_t laps, uint8_t track_nb = 0);
        void leave();

        virtual bool notifyEvent(Event* event);
        virtual bool notifyEventAsynchronous(Event* event);
        virtual void setup();
        virtual void update();
        virtual void asynchronousUpdate() {}

    protected:
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
            CONNECTED, // means in the lobby room
            KART_SELECTION,
            SELECTING_KARTS, // in the network kart selection screen
            PLAYING,
            RACE_FINISHED,
            DONE,
            EXITING
        };
        STATE m_state;
};

#endif // CLIENT_LOBBY_ROOM_PROTOCOL_HPP
