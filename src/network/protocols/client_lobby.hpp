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

#include "network/protocols/lobby_protocol.hpp"
#include "network/transport_address.hpp"
#include "utils/cpp2011.hpp"

#include <atomic>
#include <memory>
#include <set>

class BareNetworkString;
class Server;

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
    void handleBadTeam();
    void handleBadConnection();
    void becomingServerOwner();

    void clearPlayers();

    TransportAddress m_server_address;

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

    /** The state of the finite state machine. */
    std::atomic<ClientState> m_state;

    std::set<std::string> m_available_karts;
    std::set<std::string> m_available_tracks;

    bool m_received_server_result = false;

    void addAllPlayers(Event* event);
    void finalizeConnectionRequest(NetworkString* header,
                                   BareNetworkString* rest, bool encrypt);

public:
             ClientLobby(const TransportAddress& a, std::shared_ptr<Server> s);
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
    virtual bool waitingForPlayers() const OVERRIDE
                                        { return m_state.load() == CONNECTED; }
    virtual void asynchronousUpdate() OVERRIDE {}
    virtual bool allPlayersReady() const OVERRIDE
                                           { return m_state.load() >= RACING; }
    bool waitingForServerRespond() const
                            { return m_state.load() == REQUESTING_CONNECTION; }
    virtual bool isRacing() const OVERRIDE { return m_state.load() == RACING; }

};

#endif // CLIENT_LOBBY_HPP
