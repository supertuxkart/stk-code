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

#include "network/game_setup.hpp"
#include "network/network_string.hpp"

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
    enum 
    { 
        LE_CONNECTION_REQUESTED   = 1,    // a connection to the server
        LE_CONNECTION_REFUSED,            // Connection to server refused
        LE_CONNECTION_ACCEPTED,           // Connection to server accepted
        LE_KART_SELECTION_UPDATE,         // inform client about kart selected
        LE_REQUEST_BEGIN,                 // begin of kart selection
        LE_KART_SELECTION_REFUSED,        // Client not auth. to start selection
        LE_NEW_PLAYER_CONNECTED,          // inform client about new player
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
        LE_VOTE,                          // Any vote (race mode, track, ...)
        LE_VOTE_MAJOR,                    // vote of major race mode
        LE_VOTE_MINOR,                    // vote for minor race mode
        LE_VOTE_RACE_COUNT,               // vote for number of tracks
        LE_VOTE_TRACK,                    // vote for a track
        LE_VOTE_REVERSE,                  // vote if race in reverse
        LE_VOTE_LAPS,                     // vote number of laps
    };

protected:
    static LobbyProtocol *m_lobby;

    /** The game setup. */
    GameSetup* m_game_setup;


public:

    /** Creates either a client or server lobby protocol as a singleton. */
    template<typename S> static S* create()
    {
        assert(m_lobby == NULL);
        m_lobby = new S();
        return dynamic_cast<S*>(m_lobby);
    }   // create

    // ------------------------------------------------------------------------
    /** Returns the singleton client or server lobby protocol. */
    static LobbyProtocol *get()
    {
        assert(m_lobby);
        return m_lobby;
    }   // get

    // ------------------------------------------------------------------------

             LobbyProtocol(CallbackObject* callback_object);
    virtual ~LobbyProtocol();
    virtual void setup()                = 0;
    virtual void update(float dt)       = 0;
    virtual void finishedLoadingWorld() = 0;
    virtual void loadWorld();
    void terminateLatencyProtocol();
    virtual void requestKartSelection(uint8_t player_id,
                                      const std::string &kart_name)
    {
        assert(false);   // Only defined in client
    };

};   // class LobbyProtocol

#endif // LOBBY_PROTOCOL_HPP
