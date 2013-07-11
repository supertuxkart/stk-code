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
#include "online/current_online_user.hpp"
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
        Log::verbose("ClientLobbyRoomProtocol", "Message from %u : \"%s\"", event->peer->getAddress(), event->data.c_str());
        uint8_t message_type = event->data.getAndRemoveUInt8();

        if (message_type == 0x01) // new player connected
            newPlayer(event);
        else if (message_type == 0x02) // player disconnected
            disconnectedPlayer(event);
        else if (message_type == 0x03) // kart selection update
            kartSelectionUpdate(event);
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
        ns.ai8(1).ai8(4).ai32(CurrentOnlineUser::get()->getUserID());
        m_listener->sendMessage(this, ns);
        m_state = REQUESTING_CONNECTION;
    } break;
    case REQUESTING_CONNECTION:
        break;
    case CONNECTED:
        break;
    case DONE:
        m_listener->requestTerminate(this);
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

    NetworkPlayerProfile* profile = new NetworkPlayerProfile();
    profile->kart_name = "";
    profile->race_id = event->data.gui8(6);

    if (global_id == CurrentOnlineUser::get()->getUserID())
    {
        Log::error("ClientLobbyRoomProtocol", "The server notified me that i'm a new player in the room (not normal).");
    }
    else
    {
        Log::verbose("ClientLobbyRoomProtocol", "New player connected.");
        profile->user_profile = new OnlineUser(global_id);
        m_setup->addPlayer(profile);
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
    if (event->data.size() != 12 || event->data[0] != 1 || event->data[2] != 4 || event->data[7] != 4) // 12 bytes remains now
    {
        Log::error("ClientLobbyRoomProtocol", "A message notifying an accepted connection wasn't formated as expected.");
        return;
    }

    NetworkPlayerProfile* profile = new NetworkPlayerProfile();
    profile->kart_name = "";
    profile->race_id = event->data.gui8(1);
    uint32_t token = event->data.gui32(3);
    uint32_t global_id = event->data.gui32(8);
    if (global_id == CurrentOnlineUser::get()->getUserID())
    {
        Log::info("ClientLobbyRoomProtocol", "The server accepted the connection.");
        profile->user_profile = CurrentOnlineUser::get();
        m_setup->addPlayer(profile);
        event->peer->setClientServerToken(token);
        m_server = event->peer;
        m_state = CONNECTED;
    }
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
    std::string data = event->data.getString(3);
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
