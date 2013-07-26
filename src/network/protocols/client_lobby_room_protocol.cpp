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

#include "network/protocols/client_lobby_room_protocol.hpp"

#include "network/network_manager.hpp"
#include "network/protocols/start_game_protocol.hpp"
#include "online/current_user.hpp"
#include "states_screens/state_manager.hpp"
#include "states_screens/network_kart_selection.hpp"
#include "utils/log.hpp"

ClientLobbyRoomProtocol::ClientLobbyRoomProtocol(const TransportAddress& server_address)
    : LobbyRoomProtocol(NULL)
{
    m_server_address = server_address;
    m_server = NULL;
}

//-----------------------------------------------------------------------------

ClientLobbyRoomProtocol::~ClientLobbyRoomProtocol()
{
}

//-----------------------------------------------------------------------------

void ClientLobbyRoomProtocol::setup()
{
    m_setup = NetworkManager::getInstance()->setupNewGame(); // create a new setup
    m_state = NONE;
}

//-----------------------------------------------------------------------------

void ClientLobbyRoomProtocol::requestKartSelection(std::string kart_name)
{
    NetworkString request;
    // 0x02 : kart selection request, size_token (4), token, size kart name, kart name
    request.ai8(0x02).ai8(4).ai32(m_server->getClientServerToken()).ai8(kart_name.size()).as(kart_name);
    m_listener->sendMessage(this, request);
}

//-----------------------------------------------------------------------------

void ClientLobbyRoomProtocol::notifyEvent(Event* event)
{
    assert(m_setup); // assert that the setup exists
    if (event->type == EVENT_TYPE_MESSAGE)
    {
        assert(event->data.size()); // assert that data isn't empty
        uint8_t message_type = event->data.getAndRemoveUInt8();

        Log::info("ClientLobbyRoomProtocol", "Message of type %d", message_type);
        if (message_type == 0x01) // new player connected
            newPlayer(event);
        else if (message_type == 0x02) // player disconnected
            disconnectedPlayer(event);
        else if (message_type == 0x03) // kart selection update
            kartSelectionUpdate(event);
        else if (message_type == 0x04) // start race
            startGame(event);
        else if (message_type == 0x05) // start selection phase
            startSelection(event);
        else if (message_type == 0x80) // connection refused
            connectionRefused(event);
        else if (message_type == 0x81) // connection accepted
            connectionAccepted(event);
        else if (message_type == 0x82) // kart selection refused
            kartSelectionRefused(event);

    } // message
    else if (event->type == EVENT_TYPE_CONNECTED)
    {
    } // connection
    else if (event->type == EVENT_TYPE_DISCONNECTED)
    {
    } // disconnection
}

//-----------------------------------------------------------------------------

void ClientLobbyRoomProtocol::update()
{
    switch (m_state)
    {
    case NONE:
        if (NetworkManager::getInstance()->isConnectedTo(m_server_address))
        {
            m_state = LINKED;
        }
        break;
    case LINKED:
    {
        NetworkString ns;
        // 1 (connection request), 4 (size of id), global id
        ns.ai8(1).ai8(4).ai32(Online::CurrentUser::acquire()->getUserID());
        Online::CurrentUser::release();
        m_listener->sendMessage(this, ns);
        m_state = REQUESTING_CONNECTION;
    } break;
    case REQUESTING_CONNECTION:
        break;
    case CONNECTED:
        break;
    case KART_SELECTION:
        StateManager::get()->pushScreen(NetworkKartSelectionScreen::getInstance());
        m_state = SELECTING_KARTS;
        break;
    case SELECTING_KARTS:
        break;
    case PLAYING:
        break;
    case DONE:
        m_state = EXITING;
        m_listener->requestTerminate(this);
        break;
    case EXITING:
        break;
    }
}

//-----------------------------------------------------------------------------

/*! \brief Called when a new player is connected to the server
 *  \param event : Event providing the information.
 *
 *  Format of the data :
 *  Byte 0   1                  5   6                   7
 *       ------------------------------------------------
 *  Size | 1 |         4        | 1 |          1        |
 *  Data | 4 | player global id | 1 | 0 <= race id < 16 |
 *       ------------------------------------------------
 */
void ClientLobbyRoomProtocol::newPlayer(Event* event)
{
    if (event->data.size() != 7 || event->data[0] != 4 || event->data[5] != 1) // 7 bytes remains now
    {
        Log::error("ClientLobbyRoomProtocol", "A message notifying a new player wasn't formated as expected.");
        return;
    }

    uint32_t global_id = event->data.gui32(1);
    uint8_t race_id = event->data.gui8(6);

    if (global_id == Online::CurrentUser::acquire()->getUserID())
    {
        Online::CurrentUser::release();
        Log::error("ClientLobbyRoomProtocol", "The server notified me that i'm a new player in the room (not normal).");
    }
    else if (m_setup->getProfile(race_id) == NULL || m_setup->getProfile(global_id) == NULL)
    {
        Log::verbose("ClientLobbyRoomProtocol", "New player connected.");
        NetworkPlayerProfile* profile = new NetworkPlayerProfile();
        profile->kart_name = "";
        profile->race_id = race_id;
        profile->user_profile = new Online::User(global_id);
        m_setup->addPlayer(profile);
    }
    else
    {
        Log::error("ClientLobbyRoomProtocol", "One of the player notified in the list is myself.");
    }
}

//-----------------------------------------------------------------------------

/*! \brief Called when a new player is disconnected
 *  \param event : Event providing the information.
 *
 *  Format of the data :
 *  Byte 0   1                   2
 *       -------------------------
 *  Size | 1 |         1         |
 *  Data | 1 | 0 <= race id < 16 |
 *       -------------------------
 */
void ClientLobbyRoomProtocol::disconnectedPlayer(Event* event)
{
    if (event->data.size() != 2 || event->data[0] != 1)
    {
        Log::error("ClientLobbyRoomProtocol", "A message notifying a new player wasn't formated as expected.");
        return;
    }
    uint8_t id = event->data[1];
    if (m_setup->removePlayer(id))
    {
        Log::info("ClientLobbyRoomProtocol", "Peer removed successfully.");
    }
    else
    {
        Log::error("ClientLobbyRoomProtocol", "The disconnected peer wasn't known.");
    }
}

//-----------------------------------------------------------------------------

/*! \brief Called when the server accepts the connection.
 *  \param event : Event providing the information.
 *
 *  Format of the data :
 *  Byte 0   1                   2   3            7   8           12
 *       ----------------------------------------------------------
 *  Size | 1 |         1         | 1 |      4     | 1 |     4     |
 *  Data | 1 | 0 <= race id < 16 | 4 | priv token | 4 | global id |
 *       ----------------------------------------------------------
 */
void ClientLobbyRoomProtocol::connectionAccepted(Event* event)
{
    if (event->data.size() < 12 || event->data[0] != 1 || event->data[2] != 4 || event->data[7] != 4) // 12 bytes remains now
    {
        Log::error("ClientLobbyRoomProtocol", "A message notifying an accepted connection wasn't formated as expected.");
        return;
    }
    STKPeer* peer = *(event->peer);

    uint32_t global_id = event->data.gui32(8);
    if (global_id == Online::CurrentUser::acquire()->getUserID())
    {
        Online::CurrentUser::release();
        Log::info("ClientLobbyRoomProtocol", "The server accepted the connection.");

        // self profile
        NetworkPlayerProfile* profile = new NetworkPlayerProfile();
        profile->kart_name = "";
        profile->race_id = event->data.gui8(1);
        profile->user_profile = Online::CurrentUser::acquire();
        Online::CurrentUser::release();
        m_setup->addPlayer(profile);
        // connection token
        uint32_t token = event->data.gui32(3);
        peer->setClientServerToken(token);
        // add all players
        event->data.removeFront(12); // remove the 12 first bytes
        int remaining = event->data.size();
        if (remaining%7 != 0)
        {
            Log::error("ClientLobbyRoomProtocol", "ConnectionAccepted : Error in the server list");
        }
        remaining /= 7;
        for (int i = 0; i < remaining; i++)
        {
            if (event->data[0] != 1 || event->data[2] != 4)
                Log::error("ClientLobbyRoomProtocol", "Bad format in players list.");
            uint8_t race_id = event->data[1];
            uint32_t global_id = event->data.gui32(3);
            Online::User* new_user = new Online::User(global_id);
            NetworkPlayerProfile* profile2 = new NetworkPlayerProfile();
            profile2->race_id = race_id;
            profile2->user_profile = new_user;
            profile2->kart_name = "";
            m_setup->addPlayer(profile2);
            event->data.removeFront(7);
        }

        // add self
        m_server = *(event->peer);
        m_state = CONNECTED;
    }
    else
        Log::info("ClientLobbyRoomProtocol", "Failure during the connection acceptation process.");
}

//-----------------------------------------------------------------------------

/*! \brief Called when the server refuses the connection.
 *  \param event : Event providing the information.
 *
 *  Format of the data :
 *  Byte 0   1              2
 *       --------------------
 *  Size | 1 |         1    |
 *  Data | 1 | refusal code |
 *       --------------------
 */
void ClientLobbyRoomProtocol::connectionRefused(Event* event)
{
    if (event->data.size() != 2 || event->data[0] != 1) // 2 bytes remains now
    {
        Log::error("ClientLobbyRoomProtocol", "A message notifying a refused connection wasn't formated as expected.");
        return;
    }

    switch (event->data[1]) // the second byte
    {
    case 0:
        Log::info("ClientLobbyRoomProtocol", "Connection refused : too many players.");
        break;
    case 1:
        Log::info("ClientLobbyRoomProtocol", "Connection refused : banned.");
        break;
    default:
        Log::info("ClientLobbyRoomProtocol", "Connection refused.");
        break;
    }
}

//-----------------------------------------------------------------------------

/*! \brief Called when the server refuses the kart selection request.
 *  \param event : Event providing the information.
 *
 *  Format of the data :
 *  Byte 0   1              2
 *       --------------------
 *  Size | 1 |      1       |
 *  Data | 1 | refusal code |
 *       --------------------
 */
void ClientLobbyRoomProtocol::kartSelectionRefused(Event* event)
{
    if (event->data.size() != 2 || event->data[0] != 1)
    {
        Log::error("ClientLobbyRoomProtocol", "A message notifying a refused kart selection wasn't formated as expected.");
        return;
    }

    switch (event->data[1]) // the error code
    {
    case 0:
        Log::info("ClientLobbyRoomProtocol", "Kart selection refused : already taken.");
        break;
    case 1:
        Log::info("ClientLobbyRoomProtocol", "Kart selection refused : not available.");
        break;
    default:
        Log::info("ClientLobbyRoomProtocol", "Kart selection refused.");
        break;
    }
}

//-----------------------------------------------------------------------------

/*! \brief Called when the server tells to update a player's kart.
 *  \param event : Event providing the information.
 *
 *  Format of the data :
 *  Byte 0   1         2                    3           N+3
 *       ------------------------------------------------
 *  Size | 1 |    1    |       1            |     N     |
 *  Data | 1 | race id | N (kart name size) | kart name |
 *       ------------------------------------------------
 */
void ClientLobbyRoomProtocol::kartSelectionUpdate(Event* event)
{
    if (event->data.size() < 3 || event->data[0] != 1)
    {
        Log::error("ClientLobbyRoomProtocol", "A message notifying a kart selection update wasn't formated as expected.");
        return;
    }
    uint8_t player_id = event->data[1];
    uint8_t kart_name_length = event->data[2];
    std::string data = event->data.getString(3, kart_name_length);
    if (data.size() != kart_name_length)
    {
        Log::error("ClientLobbyRoomProtocol", "Kart names sizes differ: told: %d, real: %d.", kart_name_length, data.size());
        return;
    }
    if (!m_setup->isKartAvailable(data))
    {
        Log::error("ClientLobbyRoomProtocol", "The updated kart is taken already.");
    }
    m_setup->setPlayerKart(player_id, data);
}

//-----------------------------------------------------------------------------

/*! \brief Called when the race needs to be started.
 *  \param event : Event providing the information.
 *
 *  Format of the data :
 *  Byte 0   1       5
 *       -------------
 *  Size | 1 |    4  |
 *  Data | 4 | token |
 *       -------------
 */
void ClientLobbyRoomProtocol::startGame(Event* event)
{
    if (event->data.size() < 5 || event->data[0] != 4)
    {
        Log::error("ClientLobbyRoomProtocol", "A message notifying a kart "
                    "selection update wasn't formated as expected.");
        return;
    }
    uint8_t token = event->data.gui32(1);
    if (token == NetworkManager::getInstance()->getPeers()[0]->getClientServerToken())
    {
        m_state = PLAYING;
        m_listener->requestStart(new StartGameProtocol(m_setup));
        Log::error("ClientLobbyRoomProtocol", "Starting new game");
    }
    else
        Log::error("ClientLobbyRoomProtocol", "Bad token when starting game");

}

//-----------------------------------------------------------------------------

/*! \brief Called when the kart selection starts.
 *  \param event : Event providing the information.
 *
 *  Format of the data :
 *  Byte 0   1       5
 *       -------------
 *  Size | 1 |    4  |
 *  Data | 4 | token |
 *       -------------
 */
void ClientLobbyRoomProtocol::startSelection(Event* event)
{
    if (event->data.size() < 5 || event->data[0] != 4)
    {
        Log::error("ClientLobbyRoomProtocol", "A message notifying a kart "
                    "selection update wasn't formated as expected.");
        return;
    }
    uint8_t token = event->data.gui32(1);
    if (token == NetworkManager::getInstance()->getPeers()[0]->getClientServerToken())
    {
        m_state = KART_SELECTION;
        Log::info("ClientLobbyRoomProtocol", "Kart selection starts now");
    }
    else
        Log::error("ClientLobbyRoomProtocol", "Bad token");

}

//-----------------------------------------------------------------------------
