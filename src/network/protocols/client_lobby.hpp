#ifndef CLIENT_LOBBY_HPP
#define CLIENT_LOBBY_HPP

#include "network/protocols/lobby_protocol.hpp"
#include "network/transport_address.hpp"
#include "utils/cpp2011.hpp"
#include <set>

class STKPeer;

class ClientLobby : public LobbyProtocol
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
    void exitResultScreen(Event *event);
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

    std::set<std::string> m_available_karts;
    std::set<std::string> m_available_tracks;

public:
             ClientLobby();
    virtual ~ClientLobby();

    virtual void requestKartSelection(uint8_t player_id,
                                      const std::string &kart_name) OVERRIDE;
    void setAddress(const TransportAddress &address);
    void voteMajor(uint8_t player_id, uint32_t major);
    void voteRaceCount(uint8_t player_id, uint8_t count);
    void voteMinor(uint8_t player_id, uint32_t minor);
    void voteTrack(uint8_t player_id, const std::string &track,
                   uint8_t track_nb = 0);
    void voteReversed(uint8_t player_id, bool reversed, uint8_t track_nb = 0);
    void voteLaps(uint8_t player_id, uint8_t laps, uint8_t track_nb = 0);
    void doneWithResults();
    void startingRaceNow();
    void leave();

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
                                                  { return m_state == LINKED; }
    virtual void asynchronousUpdate() OVERRIDE {}

};

#endif // CLIENT_LOBBY_HPP
