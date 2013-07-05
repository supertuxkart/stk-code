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
            if ((uint8_t)(event->data[0]) == 1) // new player connected
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
                    Log::fatal("LobbyRoomProtocol", "The server sent a message that he shouldn't.");
                }
                else
                {
                    Log::verbose("LobbyRoomProtocol", "New player connected.");
                    profile.user_profile = new OnlineUser(""); ///! INSERT THE ID OF THE PLAYER HERE (global_id)
                    m_setup->addPlayer(profile);
                }
            } // if (event->data[0] == (char)(1)) 
            else if ((uint8_t)(event->data[0]) == 0b10000001) // connection accepted
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
                    Log::info("LobbyRoomProtocol", "The server accepted the connection.");
                    profile.user_profile = CurrentOnlineUser::get();
                    m_setup->addPlayer(profile);
                }
                else
                {
                    Log::fatal("LobbyRoomProtocol", "The server told you that you have a different id than yours.");
                }
            } // connection accepted
            else if ((uint8_t)(event->data[0]) == 0b10000000) // connection refused
            {
                if ((uint8_t)(event->data[1]) != 1 || event->data.size() != 2)
                {
                    Log::warn("LobbyRoomProtocol", "Inappropriate answer from the server.");
                }
                else // the message is correctly made
                {
                    Log::info("LobbyRoomProtocol", "The connection has been refused.");
                    switch ((uint8_t)(event->data[2]))
                    {
                        case 0:
                            Log::info("LobbyRoomProtocol", "Too many clients in the race.");
                        break;
                        case 1:
                            Log::info("LobbyRoomProtocol", "The host has banned you.");
                        break;
                        default:
                        break;
                    }
                }
            } // connection refused
        } // if we're a client
        else // we're the server
        {
            if ((uint8_t)(event->data[0]) == 1) // player requesting connection
            {
                if (event->data.size() != 5 || event->data[1] != (char)(4))
                {
                    Log::warn("LobbyRoomProtocol", "A player is sending a badly formated message.");
                }
                else // well-formated message
                {
                    Log::verbose("LobbyRoomProtocol", "New player.");
                    int player_id = 0;
                    // can we add the player ?
                    if (m_setup->getPlayerCount() < 16)
                    {
                        // add the player to the game setup
                        while(m_setup->getProfile(m_next_id)!=NULL)
                            m_next_id++;
                        NetworkPlayerProfile profile;
                        profile.race_id = m_next_id;
                        profile.kart_name = "";
                        profile.user_profile = new OnlineUser("Unnamed Player");
                        m_setup->addPlayer(profile);
                        // notify everybody that there is a new player
                        NetworkString message;
                        // type -- size of id -- id -- size of local id -- local id;
                        message.ai8(1).ai8(4).ai32(player_id).ai8(1).ai8(m_next_id);
                        m_listener->sendMessage(this, message);
                    }
                    else  // refuse the connection with code 0 (too much players)
                    {
                        NetworkString message;
                        message.ai8(0b10000000);      // 128 means connection refused
                        message.ai8(1);               // 1 bytes for the error code
                        message.ai8(0);               // 0 = too much players
                        m_listener->sendMessage(this, message);
                    }
                }
            }
        } // we're the server
    } // if (event->type == EVENT_TYPE_MESSAGE)
    else if (event->type == EVENT_TYPE_CONNECTED)
    {
        if (m_listener->isServer()) // if we're the server
        {
        }
        else 
        {
            Log::verbose("LobbyRoomProtocol", "Connected.");
        }
    } // if (event->type == EVENT_TYPE_CONNECTED)
    else if (event->type == EVENT_TYPE_DISCONNECTED)
    {
        if (m_listener->isServer()) // if we're the server
        {
            Log::fatal("LobbyRoomProtocol", "A client is now disconnected.");
        }
        else
        {
            Log::verbose("LobbyRoomProtocol", "Player Connected.");
        }
    } // if (event->type == EVENT_TYPE_DISCONNECTED)
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
