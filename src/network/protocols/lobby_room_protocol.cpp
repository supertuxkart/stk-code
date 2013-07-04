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

#include "network/network_manager.hpp"
#include "utils/log.hpp"
#include "online/current_online_user.hpp"

#include <assert.h>

LobbyRoomProtocol::LobbyRoomProtocol(CallbackObject* callback_object) : Protocol(callback_object, PROTOCOL_LOBBY_ROOM)
{
    m_setup = NULL;
    m_next_id = 0;
}

LobbyRoomProtocol::~LobbyRoomProtocol()
{
}

void LobbyRoomProtocol::notifyEvent(Event* event) 
{
    assert(m_setup); // assert that the setup exists
    Log::setLogLevel(1);
    if (event->type == EVENT_TYPE_MESSAGE)
    {
        assert(event->data.size()); // assert that data isn't empty
        Log::verbose("LobbyRoomProtocol", "Message from %u : \"%s\"", event->peer->getAddress(), event->data.c_str());
        if (!m_listener->isServer()) // if we're a client
        {
            if (event->data[0] == (char)(1)) // new player connected
            {
                assert(event->data[1] == (char)(4)); // id is on 4 bytes
                assert(event->data[6] == (char)(1)); // local id on 1 byte
                assert(event->data.size() == 8);     // the message was 8 bytes

                NetworkPlayerProfile profile;
                profile.kart_name = "";
                profile.race_id = (uint8_t)(event->data[7]);
                uint32_t global_id = 0;
                global_id += (uint8_t)(event->data[2])<<24;
                global_id += (uint8_t)(event->data[3])<<16;
                global_id += (uint8_t)(event->data[4])<<8;
                global_id += (uint8_t)(event->data[5]);
                if (global_id == CurrentOnlineUser::get()->getUserID())
                {
                    Log::verbose("LobbyRoomProtocol", "The server confirmed that we are logged.");
                    profile.user_profile = CurrentOnlineUser::get();
                }
                else
                {
                    Log::verbose("LobbyRoomProtocol", "New player connected.");
                    profile.user_profile = new OnlineUser(""); ///! INSERT THE ID OF THE PLAYER HERE (global_id)
                }
                m_setup->addPlayer(profile);
            } // if (event->data[0] == (char)(1)) 
            else
            {
            }
        } // if (!m_listener->isServer())
    } // if (event->type == EVENT_TYPE_MESSAGE)
    if (event->type == EVENT_TYPE_CONNECTED)
    {
        if (m_listener->isServer()) // if we're the server
        {
            Log::verbose("LobbyRoomProtocol", "New player.");
            int player_id = 0;
            // add the player to the game setup
            while(m_setup->getProfile(m_next_id)!=NULL)
                m_next_id++;
            NetworkPlayerProfile profile;
            profile.race_id = m_next_id;
            profile.kart_name = "";
            profile.user_profile = new OnlineUser("Unnamed Player");
            m_setup->addPlayer(profile);
            // notify everybody that there is a new player
            std::string message;
            message += (char)(1);                       // 1 means new player
            message += (char)(4);                       // 4 bytes for the id
            message += (char)((player_id<<24)&0xff);    // 1rst id byte
            message += (char)((player_id<<16)&0xff);    // 2nd id byte
            message += (char)((player_id<<8)&0xff);     // 3rd id byte
            message += (char)(player_id&0xff);          // 4th id byte
            message += (char)(1);                       // 1 byte for local id
            message += (char)(m_next_id);               // 1 byte of local id
            m_listener->sendMessage(this, message);
        }
        else 
        {
            Log::verbose("LobbyRoomProtocol", "Connected.");
        }
    } // if (event->type == EVENT_TYPE_CONNECTED)
    Log::setLogLevel(3);
}

void LobbyRoomProtocol::setup()
{
    m_setup = NetworkManager::getInstance()->setupNewGame(); // create a new setup
    m_next_id = 0;
}

void LobbyRoomProtocol::update()
{
}

void LobbyRoomProtocol::sendMessage(std::string message)
{
    
}
