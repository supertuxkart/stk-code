//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2013-2015 SuperTuxKart-Team
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

#ifndef LOBBY_PROTOCOL_HPP
#define LOBBY_PROTOCOL_HPP

#include "network/protocol.hpp"
#include "network/network_string.hpp"

class GameSetup;
class NetworkPlayerProfile;

#include <memory>
#include <vector>

/*!
 * \class LobbyProtocol
 * \brief Base class for both client and server lobby. The lobbies are started
 *  when a server opens a game, or when a client joins a game.
 *  It is used to exchange data about the race settings, like kart selection.
 */
class LobbyProtocol : public Protocol
{
public:
    /** Lists all lobby events (LE). */
    enum : uint8_t
    {
        LE_CONNECTION_REQUESTED   = 1,    // a connection to the server
        LE_CONNECTION_REFUSED,            // Connection to server refused
        LE_CONNECTION_ACCEPTED,           // Connection to server accepted
        LE_SERVER_INFO,                   // inform client about server info
        LE_REQUEST_BEGIN,                 // begin of kart selection
        LE_UPDATE_PLAYER_LIST,            // inform client about player list update
        LE_KART_SELECTION,                // Player selected kart
        LE_PLAYER_DISCONNECTED,           // Client disconnected
        LE_CLIENT_LOADED_WORLD,           // Client finished loading world
        LE_LOAD_WORLD,                    // Clients should load world
        LE_START_RACE,                    // Server to client to start race
        LE_STARTED_RACE,                  // Client to server that it has started race
        LE_START_SELECTION,               // inform client to start selection
        LE_RACE_FINISHED,                 // race has finished, display result
        LE_RACE_FINISHED_ACK,             // client went back to lobby
        LE_EXIT_RESULT,                   // Force clients to exit race result screen
        LE_VOTE,                          // Track vote
        LE_CHAT,
        LE_SERVER_OWNERSHIP,
        LE_KICK_HOST
    };

    enum RejectReason : uint8_t
    {
        RR_BUSY = 0,
        RR_BANNED = 1,
        RR_INCORRECT_PASSWORD = 2,
        RR_INCOMPATIBLE_DATA = 3,
        RR_TOO_MANY_PLAYERS = 4
    };

protected:
    static std::weak_ptr<LobbyProtocol> m_lobby;

    /** Stores data about the online game to play. */
    GameSetup* m_game_setup;

    void configRemoteKart(
     const std::vector<std::shared_ptr<NetworkPlayerProfile> >& players) const;

public:

    /** Creates either a client or server lobby protocol as a singleton. */
    template<typename singleton> static std::shared_ptr<singleton> create()
    {
        assert(m_lobby.expired());
        auto ret = std::make_shared<singleton>();
        m_lobby = ret;
        return std::dynamic_pointer_cast<singleton>(ret);
    }   // create

    // ------------------------------------------------------------------------
    /** Returns the singleton client or server lobby protocol. */
    template<class T> static std::shared_ptr<T> get()
    {
        if (std::shared_ptr<LobbyProtocol> lp = m_lobby.lock())
        {
            std::shared_ptr<T> new_type = std::dynamic_pointer_cast<T>(lp);
            if (new_type)
                return new_type;
        }
        return nullptr;
    }   // get

    // ------------------------------------------------------------------------

             LobbyProtocol(CallbackObject* callback_object);
    virtual ~LobbyProtocol();
    virtual void setup()                = 0;
    virtual void update(int ticks)      = 0;
    virtual void finishedLoadingWorld() = 0;
    virtual void loadWorld();
    virtual bool waitingForPlayers() const = 0;
    virtual bool allPlayersReady() const = 0;
    GameSetup* getGameSetup() const { return m_game_setup; }

};   // class LobbyProtocol

#endif // LOBBY_PROTOCOL_HPP
