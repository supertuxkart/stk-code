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

#include "network/protocols/lobby_room_protocol.hpp"

#include "utils/log.hpp"

LobbyRoomProtocol::LobbyRoomProtocol(CallbackObject* callback_object) : Protocol(callback_object, PROTOCOL_LOBBY_ROOM)
{
}

LobbyRoomProtocol::~LobbyRoomProtocol()
{
}

void LobbyRoomProtocol::notifyEvent(Event* event) 
{
    Log::setLogLevel(1);
    if (event->type == EVENT_TYPE_MESSAGE)
    {
        
        Log::verbose("LobbyRoomProtocol", "Message from %u : \"%s\"", event->peer->getAddress(), event->data.c_str());
    }
    if (event->type == EVENT_TYPE_CONNECTED)
    {
        Log::verbose("LobbyRoomProtocol", "New player.");
        // add the player to the game setup
        
    }
    Log::setLogLevel(3);
}

void LobbyRoomProtocol::setup()
{
}

void LobbyRoomProtocol::update()
{
}

void LobbyRoomProtocol::sendMessage(std::string message)
{
    
}
