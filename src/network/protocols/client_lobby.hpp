//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2018 SuperTuxKart-Team
//
//  This program is free software; you can redistribute it and/or
//  modify it under the terms of the GNU General Public License
//  as published by the Free Software Foundation; either version 3
//  of the License, or (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

#ifndef CLIENT_LOBBY_HPP
#define CLIENT_LOBBY_HPP

#include "input/input.hpp"
#include "network/protocols/lobby_protocol.hpp"
#include "utils/cpp2011.hpp"

#include <enet/enet.h>

#include <atomic>
#include <map>
#include <memory>
#include <set>

enum PeerDisconnectInfo : unsigned int;
enum KartTeam : int8_t;
enum HandicapLevel : uint8_t;

class BareNetworkString;
class Server;

namespace Online
{
    class HTTPRequest;
}

struct LobbyPlayer
{
    irr::core::stringw m_user_name;
    int m_local_player_id;
    uint32_t m_host_id;
    KartTeam m_kart_team;
    HandicapLevel m_handicap;
    uint32_t m_online_id;
    /* Icon used in networking lobby, see NetworkingLobby::loadedFromFile. */
    int m_icon_id;
    std::string m_country_code;
    /* Icon id for spectator in NetworkingLobby::loadedFromFile is 5. */
    bool isSpectator() const { return m_icon_id == 5; }
    bool isAI() const { return m_icon_id == 6; }
};

class ClientLobby : public LobbyProtocol
{
private:
    void disconnectedPlayer(Event* event);
    void connectionAccepted(Event* event); //!< Callback function on connection acceptation
    void connectionRefused(Event* event); //!< Callback function on connection refusal
    void startGame(Event* event);
    void startSelection(Event* event);
    void raceFinished(Event* event);
    void backToLobby(Event *event);
    // race votes
    void receivePlayerVote(Event* event);
    void updatePlayerList(Event* event);
    void handleChat(Event* event);
    void handleServerInfo(Event* event);
    void reportSuccess(Event* event);
    void handleBadTeam();
    void handleBadConnection();
    void becomingServerOwner();

    std::shared_ptr<Server> m_server;

    enum ClientState : unsigned int
    {
        NONE,
        LINKED,
        REQUESTING_CONNECTION,
        CONNECTED,              // means in the lobby room
        SELECTING_ASSETS,       // in the kart selection or tracks screen
        RACING,                 // racing
        RACE_FINISHED,          // race result shown
        DONE,
        EXITING
    };

    bool m_waiting_for_game;

    bool m_server_auto_game_time;

    bool m_received_server_result;

    bool m_auto_started;

    bool m_first_connect;

    bool m_spectator;

    bool m_server_live_joinable;

    bool m_server_send_live_load_world;

    bool m_server_enabled_chat;

    bool m_server_enabled_track_voting;

    bool m_server_enabled_report_player;

    uint64_t m_auto_back_to_lobby_time;

    uint64_t m_start_live_game_time;

    /** The state of the finite state machine. */
    std::atomic<ClientState> m_state;

    std::set<std::string> m_available_karts;
    std::set<std::string> m_available_tracks;

    void addAllPlayers(Event* event);
    void finalizeConnectionRequest(NetworkString* header,
                                   BareNetworkString* rest, bool encrypt);

    std::map<PeerDisconnectInfo, irr::core::stringw> m_disconnected_msg;

    std::vector<LobbyPlayer> m_lobby_players;

    std::vector<float> m_ranking_changes;

    irr::core::stringw m_total_players;

    static std::thread m_background_download;

    static std::shared_ptr<Online::HTTPRequest> m_download_request;

    void liveJoinAcknowledged(Event* event);
    void handleKartInfo(Event* event);
    void finishLiveJoin();
    std::vector<std::shared_ptr<NetworkPlayerProfile> >
         decodePlayers(const BareNetworkString& data,
         std::shared_ptr<STKPeer> peer = nullptr,
         bool* is_spectator = NULL) const;
    void getPlayersAddonKartType(const BareNetworkString& data,
        std::vector<std::shared_ptr<NetworkPlayerProfile> >& players) const;
    void getKartsTracksNetworkString(BareNetworkString* ns);
    void doInstallAddonsPack();
public:
             ClientLobby(std::shared_ptr<Server> s);
    virtual ~ClientLobby();
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
    virtual void asynchronousUpdate() OVERRIDE {}
    virtual bool allPlayersReady() const OVERRIDE
                                           { return m_state.load() >= RACING; }
    bool waitingForServerRespond() const
                            { return m_state.load() == REQUESTING_CONNECTION; }
    bool isLobbyReady() const                      { return !m_first_connect; }
    bool isWaitingForGame() const                { return m_waiting_for_game; }
    bool isServerAutoGameTime() const       { return m_server_auto_game_time; }
    virtual bool isRacing() const OVERRIDE { return m_state.load() == RACING; }
    void requestKartInfo(uint8_t kart_id);
    void setSpectator(bool val)                          { m_spectator = val; }
    bool isSpectator() const
                     { return m_spectator && m_state.load() != RACE_FINISHED; }
    void startLiveJoinKartSelection();
    void sendChat(irr::core::stringw text, KartTeam team);
    const std::vector<LobbyPlayer>& getLobbyPlayers() const
                                                    { return m_lobby_players; }
    bool isServerLiveJoinable() const        { return m_server_live_joinable; }
    void changeSpectateTarget(PlayerAction action, int value,
                              Input::InputType type) const;
    void addSpectateHelperMessage() const;
    bool serverEnabledChat() const            { return m_server_enabled_chat; }
    bool serverEnabledTrackVoting() const
                                      { return m_server_enabled_track_voting; }
    bool serverEnabledReportPlayer() const
                                     { return m_server_enabled_report_player; }
    const std::vector<float>& getRankingChanges() const
                                                  { return m_ranking_changes; }
    void handleClientCommand(const std::string& cmd);
    ClientState getCurrentState() const { return m_state.load(); }
    std::shared_ptr<Server> getJoinedServer() const { return m_server; }
    static bool startedDownloadAddonsPack()
             { return m_background_download.joinable() || m_download_request; }
    static void downloadAddonsPack(std::shared_ptr<Online::HTTPRequest> r);
    static void destroyBackgroundDownload();
    void updateAssetsToServer();
};

#endif // CLIENT_LOBBY_HPP
