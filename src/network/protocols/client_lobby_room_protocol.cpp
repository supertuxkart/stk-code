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
#include "network/network_world.hpp"

#include "modes/world_with_rank.hpp"
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

void ClientLobbyRoomProtocol::leave()
{
    m_server->disconnect();
    m_server_address.ip = 0;
    m_server_address.port = 0;
}

//-----------------------------------------------------------------------------

bool ClientLobbyRoomProtocol::notifyEvent(Event* event)
{
    assert(m_setup); // assert that the setup exists
    if (event->type == EVENT_TYPE_MESSAGE)
    {
        NetworkString data = event->data();
        assert(data.size()); // assert that data isn't empty
        uint8_t message_type = data[0];
        if (message_type != 0x03 &&
            message_type != 0x06)
            return false; // don't treat the event

        event->removeFront(1);
        Log::info("ClientLobbyRoomProtocol", "Synchronous message of type %d", message_type);
        if (message_type == 0x03) // kart selection update
            kartSelectionUpdate(event);
        else if (message_type == 0x06) // end of race
            raceFinished(event);

        return true;
    }
    return false;
}

//-----------------------------------------------------------------------------

bool ClientLobbyRoomProtocol::notifyEventAsynchronous(Event* event)
{
    assert(m_setup); // assert that the setup exists
    if (event->type == EVENT_TYPE_MESSAGE)
    {
        NetworkString data = event->data();
        assert(data.size()); // assert that data isn't empty
        uint8_t message_type = data[0];
        if (message_type == 0x03 ||
            message_type == 0x06)
            return false; // don't treat the event

        event->removeFront(1);
        Log::info("ClientLobbyRoomProtocol", "Asynchronous message of type %d", message_type);
        if (message_type == 0x01) // new player connected
            newPlayer(event);
        else if (message_type == 0x02) // player disconnected
            disconnectedPlayer(event);
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

        return true;
    } // message
    else if (event->type == EVENT_TYPE_CONNECTED)
    {
        return true;
    } // connection
    else if (event->type == EVENT_TYPE_DISCONNECTED) // means we left essentially
    {
        NetworkManager::getInstance()->removePeer(m_server);
        m_server = NULL;
        NetworkManager::getInstance()->disconnected();
        m_listener->requestTerminate(this);
        NetworkManager::getInstance()->reset();
        NetworkManager::getInstance()->removePeer(*event->peer); // prolly the same as m_server
        return true;
    } // disconnection
    return false;
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
        ns.ai8(1).ai8(4).ai32(Online::CurrentUser::get()->getID());
        m_listener->sendMessage(this, ns);
        m_state = REQUESTING_CONNECTION;
    }
    break;
    case REQUESTING_CONNECTION:
        break;
    case CONNECTED:
        break;
    case KART_SELECTION:
    {
        NetworkKartSelectionScreen* screen = NetworkKartSelectionScreen::getInstance();
        StateManager::get()->pushScreen(screen);
        m_state = SELECTING_KARTS;
    }
    break;
    case SELECTING_KARTS:
        break;
    case PLAYING:
    {
        if (NetworkWorld::getInstance<NetworkWorld>()->isRaceOver()) // race is now over, kill race protocols and return to connected state
        {
            Log::info("ClientLobbyRoomProtocol", "Game finished.");
            m_state = RACE_FINISHED;
        }
    }
    break;
    case RACE_FINISHED:
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
    NetworkString data = event->data();
    if (data.size() != 7 || data[0] != 4 || data[5] != 1) // 7 bytes remains now
    {
        Log::error("ClientLobbyRoomProtocol", "A message notifying a new player wasn't formated as expected.");
        return;
    }

    uint32_t global_id = data.gui32(1);
    uint8_t race_id = data.gui8(6);

    if (global_id == Online::CurrentUser::get()->getID())
    {
        Log::error("ClientLobbyRoomProtocol", "The server notified me that i'm a new player in the room (not normal).");
    }
    else if (m_setup->getProfile(race_id) == NULL || m_setup->getProfile(global_id) == NULL)
    {
        Log::verbose("ClientLobbyRoomProtocol", "New player connected.");
        NetworkPlayerProfile* profile = new NetworkPlayerProfile();
        profile->kart_name = "";
        profile->race_id = race_id;
        profile->user_profile = new Online::Profile(global_id, "");
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
    NetworkString data = event->data();
    if (data.size() != 2 || data[0] != 1)
    {
        Log::error("ClientLobbyRoomProtocol", "A message notifying a new player wasn't formated as expected.");
        return;
    }
    uint8_t id = data[1];
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
    NetworkString data = event->data();
    if (data.size() < 12 || data[0] != 1 || data[2] != 4 || data[7] != 4) // 12 bytes remains now
    {
        Log::error("ClientLobbyRoomProtocol", "A message notifying an accepted connection wasn't formated as expected.");
        return;
    }
    STKPeer* peer = *(event->peer);

    uint32_t global_id = data.gui32(8);
    if (global_id == Online::CurrentUser::get()->getID())
    {
        Log::info("ClientLobbyRoomProtocol", "The server accepted the connection.");

        // self profile
        NetworkPlayerProfile* profile = new NetworkPlayerProfile();
        profile->kart_name = "";
        profile->race_id = data.gui8(1);
        profile->user_profile = Online::CurrentUser::get()->getProfile();
        m_setup->addPlayer(profile);
        // connection token
        uint32_t token = data.gui32(3);
        peer->setClientServerToken(token);
        // add all players
        data.removeFront(12); // remove the 12 first bytes
        int remaining = data.size();
        if (remaining%7 != 0)
        {
            Log::error("ClientLobbyRoomProtocol", "ConnectionAccepted : Error in the server list");
        }
        remaining /= 7;
        for (int i = 0; i < remaining; i++)
        {
            if (data[0] != 1 || data[2] != 4)
                Log::error("ClientLobbyRoomProtocol", "Bad format in players list.");

            uint8_t race_id = data[1];
            uint32_t global_id = data.gui32(3);
            Online::Profile* new_user = new Online::Profile(global_id, "");

            NetworkPlayerProfile* profile2 = new NetworkPlayerProfile();
            profile2->race_id = race_id;
            profile2->user_profile = new_user;
            profile2->kart_name = "";
            m_setup->addPlayer(profile2);
            data.removeFront(7);
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
    NetworkString data = event->data();
    if (data.size() != 2 || data[0] != 1) // 2 bytes remains now
    {
        Log::error("ClientLobbyRoomProtocol", "A message notifying a refused connection wasn't formated as expected.");
        return;
    }

    switch (data[1]) // the second byte
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
    NetworkString data = event->data();
    if (data.size() != 2 || data[0] != 1)
    {
        Log::error("ClientLobbyRoomProtocol", "A message notifying a refused kart selection wasn't formated as expected.");
        return;
    }

    switch (data[1]) // the error code
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
    NetworkString data = event->data();
    if (data.size() < 3 || data[0] != 1)
    {
        Log::error("ClientLobbyRoomProtocol", "A message notifying a kart selection update wasn't formated as expected.");
        return;
    }
    uint8_t player_id = data[1];
    uint8_t kart_name_length = data[2];
    std::string kart_name = data.getString(3, kart_name_length);
    if (kart_name.size() != kart_name_length)
    {
        Log::error("ClientLobbyRoomProtocol", "Kart names sizes differ: told: %d, real: %d.", kart_name_length, kart_name.size());
        return;
    }
    if (!m_setup->isKartAvailable(kart_name))
    {
        Log::error("ClientLobbyRoomProtocol", "The updated kart is taken already.");
    }
    m_setup->setPlayerKart(player_id, kart_name);
    NetworkKartSelectionScreen::getInstance()->playerSelected(player_id, kart_name);
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
    NetworkString data = event->data();
    if (data.size() < 5 || data[0] != 4)
    {
        Log::error("ClientLobbyRoomProtocol", "A message notifying a kart "
                   "selection update wasn't formated as expected.");
        return;
    }
    uint8_t token = data.gui32(1);
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
    NetworkString data = event->data();
    if (data.size() < 5 || data[0] != 4)
    {
        Log::error("ClientLobbyRoomProtocol", "A message notifying a kart "
                   "selection update wasn't formated as expected.");
        return;
    }
    uint8_t token = data.gui32(1);
    if (token == NetworkManager::getInstance()->getPeers()[0]->getClientServerToken())
    {
        m_state = KART_SELECTION;
        Log::info("ClientLobbyRoomProtocol", "Kart selection starts now");
    }
    else
        Log::error("ClientLobbyRoomProtocol", "Bad token");

}

//-----------------------------------------------------------------------------

/*! \brief Called when all karts have finished the race.
 *  \param event : Event providing the information.
 *
 *  Format of the data :
 *  Byte 0   1       5   6           7   8           9
 *       ---------------------------------------------------
 *  Size | 1 |    4  | 1 |     1     | 1 |     1     |     |
 *  Data | 4 | token | 1 | Kart 1 ID | 1 | kart id 2 | ... |
 *       ---------------------------------------------------
 */
void ClientLobbyRoomProtocol::raceFinished(Event* event)
{
    if (event->data().size() < 5)
    {
        Log::error("ClientLobbyRoomProtocol", "Not enough data provided.");
        return;
    }
    NetworkString data = event->data();
    if ((*event->peer)->getClientServerToken() != data.gui32(1))
    {
        Log::error("ClientLobbyRoomProtocol", "Bad token");
        return;
    }
    data.removeFront(5);
    Log::error("ClientLobbyRoomProtocol", "Server notified that the race is finished.");

    // stop race protocols
    Protocol* protocol = NULL;
    protocol = m_listener->getProtocol(PROTOCOL_CONTROLLER_EVENTS);
    if (protocol)
        m_listener->requestTerminate(protocol);
    else
        Log::error("ClientLobbyRoomProtocol", "No controller events protocol registered.");

    protocol = m_listener->getProtocol(PROTOCOL_KART_UPDATE);
    if (protocol)
        m_listener->requestTerminate(protocol);
    else
        Log::error("ClientLobbyRoomProtocol", "No kart update protocol registered.");

    protocol = m_listener->getProtocol(PROTOCOL_GAME_EVENTS);
    if (protocol)
        m_listener->requestTerminate(protocol);
    else
        Log::error("ClientLobbyRoomProtocol", "No game events protocol registered.");

    // finish the race
    WorldWithRank* ranked_world = (WorldWithRank*)(World::getWorld());
    ranked_world->beginSetKartPositions();
    ranked_world->setPhase(WorldStatus::RESULT_DISPLAY_PHASE);
    int position = 1;
    while(data.size()>0)
    {
        if (data.size() < 2)
        {
            Log::error("ClientLobbyRoomProtocol", "Incomplete field.");
            return;
        }
        if (data[0] != 1)
        {
            Log::error("ClientLobbyRoomProtocol", "Badly formatted field.");
            return;
        }
        uint8_t kart_id = data[1];
        ranked_world->setKartPosition(kart_id,position);
        Log::info("ClientLobbyRoomProtocol", "Kart %d has finished #%d", kart_id, position);
        data.removeFront(2);
        position++;
    }
    ranked_world->endSetKartPositions();
    m_state = RACE_FINISHED;
    ranked_world->terminateRace();
}

//-----------------------------------------------------------------------------
