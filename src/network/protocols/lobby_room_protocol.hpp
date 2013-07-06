//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2013 SuperTuxKart-Team
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

#ifndef LOBBY_ROOM_PROTOCOL_HPP
#define LOBBY_ROOM_PROTOCOL_HPP

#include "network/protocol.hpp"

#include "network/game_setup.hpp"
#include "network/network_string.hpp"

/*!
 * \class LobbyRoomProtocol
 * \brief Class used while the game is being prepared.
 * This protocol starts when a server opens a game, or when a client joins a game.
 * It is used to exchange data about the race settings, like kart selection.
 */
class LobbyRoomProtocol : public Protocol
{
    public:
        LobbyRoomProtocol(CallbackObject* callback_object);
        virtual ~LobbyRoomProtocol();
        
        virtual void notifyEvent(Event* event) = 0;
        virtual void setup() = 0;
        virtual void update() = 0;
        
    protected:
        GameSetup* m_setup; //!< The game setup.
};

class ClientLobbyRoomProtocol : public LobbyRoomProtocol
{
    public:
        ClientLobbyRoomProtocol() : LobbyRoomProtocol(NULL) {}
        virtual ~ClientLobbyRoomProtocol() {}
    
        virtual void notifyEvent(Event* event);
        virtual void setup();
        virtual void update();
        
        void sendMessage(std::string message);
        
    protected:
        TransportAddress m_server_address; 
        
        enum STATE
        {
            NONE,
            GETTING_SERVER_ADDRESS,
            REQUESTING_CONNECTION,
            CONNECTED,
            DONE
        };
        STATE m_state;
};

class ServerLobbyRoomProtocol : public LobbyRoomProtocol
{
    public:
        ServerLobbyRoomProtocol() : LobbyRoomProtocol(NULL) {}
        virtual ~ServerLobbyRoomProtocol() {}
        
        virtual void notifyEvent(Event* event);
        virtual void setup();
        virtual void update();
    
    protected:
        uint8_t m_next_id; //!< Next id to assign to a peer.
        std::vector<TransportAddress> m_peers;
         
        enum STATE
        {
            WORKING,
            DONE
        };
        STATE m_state;
};

#endif // LOBBY_ROOM_PROTOCOL_HPP
