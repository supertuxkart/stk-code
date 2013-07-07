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
#include "network/client_network_manager.hpp"
#include "online/current_online_user.hpp"
#include "online/http_connector.hpp"
#include "config/user_config.hpp"
#include "utils/log.hpp"
#include "utils/random_generator.hpp"
#include "utils/time.hpp"

#include <assert.h>

LobbyRoomProtocol::LobbyRoomProtocol(CallbackObject* callback_object) : Protocol(callback_object, PROTOCOL_LOBBY_ROOM)
{
    m_setup = NULL;
}

//-----------------------------------------------------------------------------

LobbyRoomProtocol::~LobbyRoomProtocol()
{
}

//-----------------------------------------------------------------------------

void ClientLobbyRoomProtocol::setup()
{
    m_setup = NetworkManager::getInstance()->setupNewGame(); // create a new setup
    m_state = NONE;
}

//-----------------------------------------------------------------------------

void ServerLobbyRoomProtocol::setup()
{
    m_setup = NetworkManager::getInstance()->setupNewGame(); // create a new setup
    m_next_id = 0;
    m_state = WORKING;
}

//-----------------------------------------------------------------------------

void ClientLobbyRoomProtocol::notifyEvent(Event* event)
{
    assert(m_setup); // assert that the setup exists
    Log::setLogLevel(1);
    if (event->type == EVENT_TYPE_MESSAGE)
    {
        assert(event->data.size()); // assert that data isn't empty
        Log::verbose("LobbyRoomProtocol", "Message from %u : \"%s\"", event->peer->getAddress(), event->data.c_str());
        uint8_t message_type = 0;
        event->data.gui8(&message_type);
        if (message_type == 1) // new player connected
        {
            if (event->data.size() != 7 || event->data[0] != 4 || event->data[5] != 1) // 7 bytes remains now
            {
                Log::error("LobbyRoomProtocol", "A message notifying a new player wasn't formated as expected.");
                return;
            }

            uint32_t global_id = event->data.gui32(1);

            NetworkPlayerProfile profile;
            profile.kart_name = "";
            profile.race_id = event->data.gui8(6);

            if (global_id == CurrentOnlineUser::get()->getUserID())
            {
                Log::error("LobbyRoomProtocol", "The server notified me that i'm a new player in the room (not normal).");
            }
            else
            {
                Log::verbose("LobbyRoomProtocol", "New player connected.");
                profile.user_profile = new OnlineUser(""); ///! INSERT THE ID OF THE PLAYER HERE (global_id)
                m_setup->addPlayer(profile);
            }
        } // new player connected
        else if (message_type == 0b10000001) // connection accepted
        {
            if (event->data.size() != 12 || event->data[0] != 1 || event->data[2] != 4 || event->data[7] != 4) // 7 bytes remains now
            {
                Log::error("LobbyRoomProtocol", "A message notifying an accepted connection wasn't formated as expected.");
                return;
            }

            NetworkPlayerProfile profile;
            profile.kart_name = "";
            profile.race_id = event->data.gui8(1);
            uint32_t token = event->data.gui32(3);
            uint32_t global_id = event->data.gui32(8);
            if (global_id == CurrentOnlineUser::get()->getUserID())
            {
                Log::info("LobbyRoomProtocol", "The server accepted the connection.");
                profile.user_profile = CurrentOnlineUser::get();
                m_setup->addPlayer(profile);
                event->peer->setClientServerToken(token);
            }
        } // connection accepted
        else if (message_type == 0b10000000) // connection refused
        {
            if (event->data.size() != 2 || event->data[0] != 1) // 2 bytes remains now
            {
                Log::error("LobbyRoomProtocol", "A message notifying a refused connection wasn't formated as expected.");
                return;
            }

            Log::info("LobbyRoomProtocol", "The connection has been refused.");
            switch (event->data[1]) // the second byte
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
        } // connection refused
    } // if (event->type == EVENT_TYPE_MESSAGE)
    else if (event->type == EVENT_TYPE_CONNECTED)
    {
    } // if (event->type == EVENT_TYPE_CONNECTED)
    else if (event->type == EVENT_TYPE_DISCONNECTED)
    {
    } // if (event->type == EVENT_TYPE_DISCONNECTED)
    Log::setLogLevel(3);
}

//-----------------------------------------------------------------------------

void ServerLobbyRoomProtocol::notifyEvent(Event* event)
{
    assert(m_setup); // assert that the setup exists
    Log::setLogLevel(1);
    if (event->type == EVENT_TYPE_MESSAGE)
    {
        assert(event->data.size()); // message not empty
        uint8_t message_type;
        event->data.gui8(&message_type);
        if (message_type == 1) // player requesting connection
        {
            if (event->data.size() != 5 || event->data[0] != 4)
            {
                Log::warn("LobbyRoomProtocol", "A player is sending a badly formated message.");
                return;
            }
            Log::verbose("LobbyRoomProtocol", "New player.");
            int player_id = 0;
            // can we add the player ?
            if (m_setup->getPlayerCount() < 16) // accept player
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
                // new player (1) -- size of id -- id -- size of local id -- local id;
                message.ai8(1).ai8(4).ai32(player_id).ai8(1).ai8(m_next_id);
                m_listener->sendMessageExcept(this, event->peer, message);
                // send a message to the one that asked to connect
                NetworkString message_ack;
                // 0b1000001 (connection success) ;
                RandomGenerator token_generator;
                // use 4 random numbers because rand_max is probably 2^15-1.
                uint32_t token = (uint32_t)(((token_generator.get(RAND_MAX)<<24) & 0xff) +
                                            ((token_generator.get(RAND_MAX)<<16) & 0xff) +
                                            ((token_generator.get(RAND_MAX)<<8)  & 0xff) +
                                            ((token_generator.get(RAND_MAX)      & 0xff)));
                // connection success (129) -- size of token -- token
                message_ack.ai8(0b1000001).ai8(4).ai32(token).ai8(4).ai32(token);
                m_listener->sendMessage(this, event->peer, message_ack);
            } // accept player
            else  // refuse the connection with code 0 (too much players)
            {
                NetworkString message;
                message.ai8(0b10000000);      // 128 means connection refused
                message.ai8(1);               // 1 bytes for the error code
                message.ai8(0);               // 0 = too much players
                // send only to the peer that made the request
                m_listener->sendMessage(this, event->peer, message);
            }
        }
    } // if (event->type == EVENT_TYPE_MESSAGE)
    else if (event->type == EVENT_TYPE_CONNECTED)
    {
    } // if (event->type == EVENT_TYPE_CONNECTED)
    else if (event->type == EVENT_TYPE_DISCONNECTED)
    {

    } // if (event->type == EVENT_TYPE_DISCONNECTED)
    Log::setLogLevel(3);
}

//-----------------------------------------------------------------------------

void ClientLobbyRoomProtocol::update()
{
    switch (m_state)
    {
        case NONE:
            break;
        case CONNECTED:
            break;
        case DONE:
            m_listener->requestTerminate(this);
            break;
    }
}

//-----------------------------------------------------------------------------

void ServerLobbyRoomProtocol::update()
{
    switch (m_state)
    {
        case WORKING:
            static double last_poll_time = 0;
            if (Time::getRealTime() > last_poll_time+5.0)
            {
                last_poll_time = Time::getRealTime();
                HTTPConnector * connector = new HTTPConnector((std::string)UserConfigParams::m_server_multiplayer + "address-management.php");
                connector->setParameter("id",CurrentOnlineUser::get()->getUserID());
                connector->setParameter("token",CurrentOnlineUser::get()->getToken());
                connector->setParameter("action","poll-connection-requests");

                const XMLNode * result = connector->getXMLFromPage();
                std::string rec_success;

                if(result->getName() == "users")
                {
                    uint32_t id = 0;
                    for (unsigned int i = 0; i < result->getNumNodes(); i++)
                    {
                        result->getNode(i)->get("id", &id);
                        Log::info("ServerLobbyRoomProtocol", "User with id %d wants to connect.", id);
                    }   
                }
                else
                {
                    Log::error("ServerLobbyRoomProtocol", "Cannot retreive the list");
                }
            }
            break;
        case DONE:
            m_listener->requestTerminate(this);
            break;
    }
}
//-----------------------------------------------------------------------------



