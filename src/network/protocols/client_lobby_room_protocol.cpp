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

#include "network/protocols/client_lobby_room_protocol.hpp"

#include "config/player_manager.hpp"
#include "modes/world_with_rank.hpp"
#include "network/event.hpp"
#include "network/network_config.hpp"
#include "network/network_player_profile.hpp"
#include "network/protocols/start_game_protocol.hpp"
#include "network/protocol_manager.hpp"
#include "network/race_event_manager.hpp"
#include "network/stk_host.hpp"
#include "network/stk_peer.hpp"
#include "online/online_profile.hpp"
#include "states_screens/networking_lobby.hpp"
#include "states_screens/network_kart_selection.hpp"
#include "states_screens/state_manager.hpp"
#include "utils/log.hpp"

ClientLobbyRoomProtocol::
ClientLobbyRoomProtocol(const TransportAddress& server_address)
                       : LobbyRoomProtocol(NULL)
{
    m_server_address.copy(server_address);
    m_server = NULL;
}   // ClientLobbyRoomProtocol

//-----------------------------------------------------------------------------

ClientLobbyRoomProtocol::~ClientLobbyRoomProtocol()
{
}   // ClientLobbyRoomProtocol

//-----------------------------------------------------------------------------

void ClientLobbyRoomProtocol::setup()
{
    m_setup = STKHost::get()->setupNewGame(); // create a new setup
    m_state = NONE;
}   // setup

//-----------------------------------------------------------------------------

void ClientLobbyRoomProtocol::requestKartSelection(const std::string &kart_name)
{
    NewNetworkString *request = getNetworkString(6+1+kart_name.size());
    // size_token (4), token, size kart name, kart name
    request->setToken(m_server->getClientServerToken());
    request->addUInt8(LE_KART_SELECTION).encodeString(kart_name);

    NewNetworkString *r = getNetworkString(7+kart_name.size());
    r->setToken(m_server->getClientServerToken());
    r->addUInt8(LE_KART_SELECTION).addUInt8(4).encodeString(kart_name);
    sendMessage(*r, true);
    delete r;
}   // requestKartSelection

//-----------------------------------------------------------------------------

void ClientLobbyRoomProtocol::voteMajor(uint32_t major)
{
    NewNetworkString *request = getNetworkString(8);
    request->setToken(m_server->getClientServerToken());
    // size_token (4), token, size major(1),major
    request->addUInt8(LE_VOTE_MAJOR).addUInt8(4)
           .addUInt8(4).addUInt32(major);
    sendMessage(*request, true);
    delete request;
}   // voteMajor

//-----------------------------------------------------------------------------

void ClientLobbyRoomProtocol::voteRaceCount(uint8_t count)
{
    NewNetworkString *request = getNetworkString(8);
    request->setToken(m_server->getClientServerToken());
    // size_token (4), token, size race count(1), count
    request->addUInt8(LE_VOTE_RACE_COUNT).addUInt8(4)
           .addUInt8(1).addUInt8(count);
    sendMessage(*request, true);
    delete request;
}   // voteRaceCount

//-----------------------------------------------------------------------------

void ClientLobbyRoomProtocol::voteMinor(uint32_t minor)
{
    NewNetworkString *request = getNetworkString(8);
    request->setToken(m_server->getClientServerToken());
    // size_token (4), token, size minor(1),minor
    request->addUInt8(LE_VOTE_MINOR).addUInt8(4)
           .addUInt8(4).addUInt32(minor);
    sendMessage(*request, true);
    delete request;
}   // voteMinor

//-----------------------------------------------------------------------------

void ClientLobbyRoomProtocol::voteTrack(const std::string &track,
                                        uint8_t track_nb)
{
    NewNetworkString *request = getNetworkString(8+1+track.size());
    request->setToken(m_server->getClientServerToken());
    // size_token (4), token, size track, track, size #track, #track
    request->addUInt8(LE_VOTE_TRACK).addUInt8(4)
           .encodeString(track).addUInt8(1).addUInt8(track_nb);
    sendMessage(*request, true);
    delete request;
}   // voteTrack

//-----------------------------------------------------------------------------

void ClientLobbyRoomProtocol::voteReversed(bool reversed, uint8_t track_nb)
{
    NewNetworkString *request = getNetworkString(9);
    request->setToken(m_server->getClientServerToken());
    // size_token (4), token, size reversed(1),reversed, size #track, #track
    request->addUInt8(LE_VOTE_REVERSE).addUInt8(4)
           .addUInt8(1).addUInt8(reversed).addUInt8(1).addUInt8(track_nb);
    sendMessage(*request, true);
    delete request;
}   // voteReversed

//-----------------------------------------------------------------------------

void ClientLobbyRoomProtocol::voteLaps(uint8_t laps, uint8_t track_nb)
{
    NewNetworkString *request = getNetworkString(10);
    request->setToken(m_server->getClientServerToken());
    // size_token (4), token, size laps(1),laps, size #track, #track
    request->addUInt8(LE_VOTE_LAPS)
           .addUInt8(1).addUInt8(laps).addUInt8(1).addUInt8(track_nb);
    sendMessage(*request, true);
}   // voteLaps

//-----------------------------------------------------------------------------

void ClientLobbyRoomProtocol::leave()
{
    m_server->disconnect();
    STKHost::get()->removePeer(m_server);
    m_server_address.clear();
    ServersManager::get()->unsetJoinedServer();
}   // leave

//-----------------------------------------------------------------------------

bool ClientLobbyRoomProtocol::notifyEvent(Event* event)
{
    assert(m_setup); // assert that the setup exists
    if (event->getType() == EVENT_TYPE_MESSAGE)
    {
        const NewNetworkString &data = event->data();
        assert(data.size()); // assert that data isn't empty
        uint8_t message_type = data[0];
        if (message_type != LE_KART_SELECTION_UPDATE &&
            message_type != LE_RACE_FINISHED            )
            return false; // don't treat the event

        event->removeFront(1);
        Log::info("ClientLobbyRoomProtocol", "Synchronous message of type %d",
                  message_type);
        if (message_type == LE_KART_SELECTION_UPDATE) // kart selection update
            kartSelectionUpdate(event);
        else if (message_type == LE_RACE_FINISHED) // end of race
            raceFinished(event);

        return true;
    }
    return false;
}   // notifyEvent

//-----------------------------------------------------------------------------

bool ClientLobbyRoomProtocol::notifyEventAsynchronous(Event* event)
{
    assert(m_setup); // assert that the setup exists
    if (event->getType() == EVENT_TYPE_MESSAGE)
    {
        const NewNetworkString &data = event->data();
        assert(data.size()); // assert that data isn't empty
        uint8_t message_type = data[0];

        event->removeFront(1);
        Log::info("ClientLobbyRoomProtocol", "Asynchronous message of type %d",
                  message_type);
        switch(message_type)
        {
            case LE_NEW_PLAYER_CONNECTED: newPlayer(event);              break;
            case LE_PLAYER_DISCONNECTED : disconnectedPlayer(event);     break;
            case LE_START_RACE: startGame(event);                        break;
            case LE_START_SELECTION: startSelection(event);              break;
            case LE_CONNECTION_REFUSED: connectionRefused(event);        break;
            case LE_CONNECTION_ACCEPTED: connectionAccepted(event);      break;
            case LE_KART_SELECTION_REFUSED: kartSelectionRefused(event); break;
            case LE_VOTE_MAJOR : playerMajorVote(event);                 break;
            case LE_VOTE_RACE_COUNT: playerRaceCountVote(event);         break;
            case LE_VOTE_MINOR: playerMinorVote(event);                  break;
            case LE_VOTE_TRACK: playerTrackVote(event);                  break;
            case LE_VOTE_REVERSE: playerReversedVote(event);             break;
            case LE_VOTE_LAPS: playerLapsVote(event);                    break;
        }   // switch

        return true;
    } // message
    else if (event->getType() == EVENT_TYPE_CONNECTED)
    {
        return true;
    } // connection
    else if (event->getType() == EVENT_TYPE_DISCONNECTED) 
    {
        // This means we left essentially.
        // We can't delete STKHost from this thread, since the main
        // thread might still test if STKHost exists and then call
        // the ProtocolManager, which might already have been deleted.
        // So only signal tha the STKHost should exit, which will be tested
        // from the main thread.
        STKHost::get()->requestShutdown();
        return true;
    } // disconnection
    return false;
}   // notifyEventAsynchronous

//-----------------------------------------------------------------------------

void ClientLobbyRoomProtocol::update()
{
    switch (m_state)
    {
    case NONE:
        if (STKHost::get()->isConnectedTo(m_server_address))
        {
            m_state = LINKED;
        }
        break;
    case LINKED:
    {
        core::stringw name;
        if(PlayerManager::getCurrentOnlineState()==PlayerProfile::OS_SIGNED_IN)
            name = PlayerManager::getCurrentOnlineUserName();
        else
            name = PlayerManager::getCurrentPlayer()->getName();

        std::string name_u8 = StringUtils::wideToUtf8(name);
        const std::string &password = NetworkConfig::get()->getPassword();
        NewNetworkString *ns = getNetworkString(6+1+name_u8.size()
                                                 +1+password.size());
        // 4 (size of id), global id
        ns->addUInt8(LE_CONNECTION_REQUESTED).encodeString(name)
          .encodeString(NetworkConfig::get()->getPassword());
        sendMessage(*ns);
        delete ns;
        m_state = REQUESTING_CONNECTION;
    }
    break;
    case REQUESTING_CONNECTION:
        break;
    case CONNECTED:
        break;
    case KART_SELECTION:
    {
        NetworkKartSelectionScreen* screen =
                                     NetworkKartSelectionScreen::getInstance();
        screen->push();
        m_state = SELECTING_KARTS;
    }
    break;
    case SELECTING_KARTS:
        break;
    case PLAYING:
    {
        // race is now over, kill race protocols and return to connected state
        if (RaceEventManager::getInstance<RaceEventManager>()->isRaceOver())
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
        ProtocolManager::getInstance()->requestTerminate(this);
        break;
    case EXITING:
        break;
    }
}   // update

//-----------------------------------------------------------------------------

/*! \brief Called when a new player is connected to the server
 *  \param event : Event providing the information.
 *
 *  Format of the data :
 *  Byte 0    1                   2s
 *       ---------------------------------------
 *  Size | 1 |          1        |             |
 *  Data | 1 | 0 <= race id < 16 |  player name|
 *       ---------------------------------------
 */
void ClientLobbyRoomProtocol::newPlayer(Event* event)
{
    const NewNetworkString &data = event->data();
    if (data[0] != 1)
    {
        Log::error("ClientLobbyRoomProtocol",
                   "A message notifying a new player wasn't formated "
                   "as expected.");
        return;
    }

    uint8_t player_id = data.getUInt8(1);

    core::stringw name;
    data.decodeStringW(2, &name);
    // FIXME need adjusting when splitscreen is used/
    if(STKHost::get()->getGameSetup()->isLocalMaster(player_id))
    {
        Log::error("ClientLobbyRoomProtocol",
                   "The server notified me that I'm a new player in the "
                   "room (not normal).");
    }
    else if (m_setup->getProfile(player_id) == NULL)
    {
        Log::verbose("ClientLobbyRoomProtocol", "New player connected.");
        NetworkPlayerProfile* profile = new NetworkPlayerProfile(player_id, name);
        m_setup->addPlayer(profile);
        NetworkingLobby::getInstance()->addPlayer(profile);
    }
    else
    {
        Log::error("ClientLobbyRoomProtocol",
                   "One of the player notified in the list is myself.");
    }
}   // newPlayer

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
    const NewNetworkString &data = event->data();
    if (data.size() != 2 || data[0] != 1)
    {
        Log::error("ClientLobbyRoomProtocol",
                   "A message notifying a new player wasn't formated "
                   "as expected.");
        return;
    }
    if (m_setup->removePlayer(event->getPeer()->getPlayerProfile()))
    {
        Log::info("ClientLobbyRoomProtocol", "Peer removed successfully.");
    }
    else
    {
        Log::error("ClientLobbyRoomProtocol",
                   "The disconnected peer wasn't known.");
    }
    STKHost::get()->removePeer(event->getPeer());
}   // disconnectedPlayer

//-----------------------------------------------------------------------------

/*! \brief Called when the server accepts the connection.
 *  \param event : Event providing the information.
 *
 *  Format of the data :
 *  Byte 0   1                   2   3            7        8             9
 *       -----------------------------------------------------------------------------
 *  Size | 1 |         1         | 1 |      4     |   1    | 1          |             |
 *  Data | 1 | 0 <= race id < 16 | 4 | priv token | hostid | authorised |playernames* |
 *       ------------------------------------------------------------------------------
 */
void ClientLobbyRoomProtocol::connectionAccepted(Event* event)
{
    const NewNetworkString &data = event->data();
    // At least 12 bytes should remain now
    if (data.size() < 9|| data[0] != 1 || data[2] != 4)
    {
        Log::error("ClientLobbyRoomProtocol",
                   "A message notifying an accepted connection wasn't "
                   "formated as expected.");
        return;
    }
    STKPeer* peer = event->getPeer();

    // Accepted
    // ========
    Log::info("ClientLobbyRoomProtocol",
              "The server accepted the connection.");

    // self profile
    irr::core::stringw name;
    if (PlayerManager::getCurrentOnlineState() == PlayerProfile::OS_SIGNED_IN)
        name = PlayerManager::getCurrentOnlineUserName();
    else
        name = PlayerManager::getCurrentPlayer()->getName();
    uint8_t my_player_id = data.getUInt8(1);
    uint8_t my_host_id   = data.getUInt8(7);
    uint8_t authorised   = data.getUInt8(8);
    // Store this client's authorisation status in the peer information
    // for the server.
    event->getPeer()->setAuthorised(authorised!=0);
    STKHost::get()->setMyHostId(my_host_id);

    NetworkPlayerProfile* profile = new NetworkPlayerProfile(my_player_id, name);
    profile->setHostId(my_host_id);
    STKHost::get()->getGameSetup()->setLocalMaster(my_player_id);
    m_setup->setNumLocalPlayers(1);
    // connection token
    uint32_t token = data.getUInt32(3);
    peer->setClientServerToken(token);

    // Add all players
    // ===============
    int n = 9;
    while (n < data.size())
    {
        if (data[n] != 1 )
            Log::error("ClientLobbyRoomProtocol",
                       "Bad format in players list.");

        uint8_t player_id = data[n + 1];
        irr::core::stringw name;
        int bytes_read = data.decodeStringW(n + 2, &name);
        uint8_t host_id = data.getUInt8(n+2+bytes_read);

        NetworkPlayerProfile* profile2 =
            new NetworkPlayerProfile(player_id, name);
        profile2->setHostId(host_id);
        m_setup->addPlayer(profile2);
        n += bytes_read+3;
        // Inform the network lobby of all players so that the GUI can
        // show all currently connected players.
        NetworkingLobby::getInstance()->addPlayer(profile2);
    }

    // Add self after other players so that player order is identical
    // on server and all clients.
    m_setup->addPlayer(profile);
    NetworkingLobby::getInstance()->addPlayer(profile);
    m_server = event->getPeer();
    m_state = CONNECTED;
}   // connectionAccepted

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
    const NewNetworkString &data = event->data();
    if (data.size() != 2 || data[0] != 1) // 2 bytes remains now
    {
        Log::error("ClientLobbyRoomProtocol",
                   "A message notifying a refused connection wasn't formated "
                   "as expected.");
        return;
    }

    switch (data[1]) // the second byte
    {
    case 0:
        Log::info("ClientLobbyRoomProtocol",
                  "Connection refused : too many players.");
        break;
    case 1:
        Log::info("ClientLobbyRoomProtocol", "Connection refused : banned.");
        break;
    case 2:
        Log::info("ClientLobbyRoomProtocol", "Client busy.");
        break;
    default:
        Log::info("ClientLobbyRoomProtocol", "Connection refused.");
        break;
    }
}   // connectionRefused

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
    const NewNetworkString &data = event->data();
    if (data.size() != 2 || data[0] != 1)
    {
        Log::error("ClientLobbyRoomProtocol",
                   "A message notifying a refused kart selection wasn't "
                   "formated as expected.");
        return;
    }

    switch (data[1]) // the error code
    {
    case 0:
        Log::info("ClientLobbyRoomProtocol",
                  "Kart selection refused : already taken.");
        break;
    case 1:
        Log::info("ClientLobbyRoomProtocol",
                  "Kart selection refused : not available.");
        break;
    default:
        Log::info("ClientLobbyRoomProtocol", "Kart selection refused.");
        break;
    }
}   // kartSelectionRefused

//-----------------------------------------------------------------------------

/*! \brief Called when the server tells to update a player's kart.
 *  \param event : Event providing the information.
 *
 *  Format of the data :
 *  Byte 0   1           2                    3           N+3
 *       --------------------------------------------------
 *  Size | 1 |    1      |       1            |     N     |
 *  Data | 1 | player id | N (kart name size) | kart name |
 *       --------------------------------------------------
 */
void ClientLobbyRoomProtocol::kartSelectionUpdate(Event* event)
{
    const NewNetworkString &data = event->data();
    if (data.size() < 3 || data[0] != 1)
    {
        Log::error("ClientLobbyRoomProtocol", 
                   "A message notifying a kart selection update wasn't "
                   "formated as expected.");
        return;
    }
    uint8_t player_id = data[1];
    std::string kart_name;
    data.decodeString(2, &kart_name);
    if (!m_setup->isKartAvailable(kart_name))
    {
        Log::error("ClientLobbyRoomProtocol",
                   "The updated kart is taken already.");
    }
    m_setup->setPlayerKart(player_id, kart_name);
    NetworkKartSelectionScreen::getInstance()->playerSelected(player_id,
                                                              kart_name);
}   // kartSelectionUpdate

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
    const NewNetworkString &data = event->data();
    if (data.size() < 5 || data[0] != 4)
    {
        Log::error("ClientLobbyRoomProtocol", "A message notifying a kart "
                   "selection update wasn't formated as expected.");
        return;
    }
    uint32_t token = data.getUInt32(1);
    if (token == STKHost::get()->getPeers()[0]->getClientServerToken())
    {
        m_state = PLAYING;
        ProtocolManager::getInstance()
                       ->requestStart(new StartGameProtocol(m_setup));
        Log::error("ClientLobbyRoomProtocol", "Starting new game");
    }
    else
        Log::error("ClientLobbyRoomProtocol", "Bad token when starting game");

}   // startGame

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
    const NewNetworkString &data = event->data();
    if (data.size() < 5 || data[0] != 4)
    {
        Log::error("ClientLobbyRoomProtocol", "A message notifying a kart "
                   "selection update wasn't formated as expected.");
        return;
    }
    uint32_t token = data.getUInt32(1);
    if (token == STKHost::get()->getPeers()[0]->getClientServerToken())
    {
        m_state = KART_SELECTION;
        Log::info("ClientLobbyRoomProtocol", "Kart selection starts now");
    }
    else
        Log::error("ClientLobbyRoomProtocol", "Bad token");

}   // startSelection

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
    NewNetworkString &data = event->data();
    if (data.size() < 5)
    {
        Log::error("ClientLobbyRoomProtocol", "Not enough data provided.");
        return;
    }
    if (event->getPeer()->getClientServerToken() != data.getUInt32(1))
    {
        Log::error("ClientLobbyRoomProtocol", "Bad token");
        return;
    }
    data.removeFront(5);
    Log::error("ClientLobbyRoomProtocol",
               "Server notified that the race is finished.");

    // stop race protocols
    Protocol* protocol = ProtocolManager::getInstance()
                       ->getProtocol(PROTOCOL_CONTROLLER_EVENTS);
    if (protocol)
        ProtocolManager::getInstance()->requestTerminate(protocol);
    else
        Log::error("ClientLobbyRoomProtocol",
                   "No controller events protocol registered.");

    protocol = ProtocolManager::getInstance() 
             ->getProtocol(PROTOCOL_KART_UPDATE);
    if (protocol)
        ProtocolManager::getInstance()->requestTerminate(protocol);
    else
        Log::error("ClientLobbyRoomProtocol",
                   "No kart update protocol registered.");

    protocol = ProtocolManager::getInstance()
             ->getProtocol(PROTOCOL_GAME_EVENTS);
    if (protocol)
        ProtocolManager::getInstance()->requestTerminate(protocol);
    else
        Log::error("ClientLobbyRoomProtocol",
                   "No game events protocol registered.");

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
        Log::info("ClientLobbyRoomProtocol", "Kart %d has finished #%d",
                  kart_id, position);
        data.removeFront(2);
        position++;
    }
    ranked_world->endSetKartPositions();
    m_state = RACE_FINISHED;
    ranked_world->terminateRace();
}   // raceFinished

//-----------------------------------------------------------------------------

/*! \brief Called when a player votes for a major race mode.
 *  \param event : Event providing the information.
 *
 *  Format of the data :
 *  Byte 0   1            5   6           7   8                 9
 *       --------------------------------------------------------
 *  Size | 1 |      4     | 1 |     1     | 1 |        1        |
 *  Data | 4 | priv token | 1 | player id | 1 | major mode vote |
 *       --------------------------------------------------------
 */
void ClientLobbyRoomProtocol::playerMajorVote(Event* event)
{
    const NewNetworkString &data = event->data();
    if (!checkDataSizeAndToken(event, 9))
        return;
    if (!isByteCorrect(event, 5, 1))
        return;
    if (!isByteCorrect(event, 7, 4))
        return;
    m_setup->getRaceConfig()->setPlayerMajorVote(data[6], data[8]);
}   // playerMajorVote

//-----------------------------------------------------------------------------
/*! \brief Called when a player votes for the number of races in a GP.
 *  \param event : Event providing the information.
 *
 *  Format of the data :
 *  Byte 0   1            5   6           7   8             9
 *       ----------------------------------------------------
 *  Size | 1 |      4     | 1 |     1     | 1 |      1      |
 *  Data | 4 | priv token | 1 | player id | 1 | races count |
 *       ----------------------------------------------------
 */
void ClientLobbyRoomProtocol::playerRaceCountVote(Event* event)
{
    const NewNetworkString &data = event->data();
    if (!checkDataSizeAndToken(event, 9))
        return;
    if (!isByteCorrect(event, 5, 1))
        return;
    if (!isByteCorrect(event, 7, 1))
        return;
    m_setup->getRaceConfig()->setPlayerRaceCountVote(data[6], data[8]);
}   // playerRaceCountVote

//-----------------------------------------------------------------------------
/*! \brief Called when a player votes for a minor race mode.
 *  \param event : Event providing the information.
 *
 *  Format of the data :
 *  Byte 0   1            5   6           7   8                 9
 *       --------------------------------------------------------
 *  Size | 1 |      4     | 1 |      1    | 1 |        4        |
 *  Data | 4 | priv token | 1 | player id | 4 | minor mode vote |
 *       --------------------------------------------------------
 */
void ClientLobbyRoomProtocol::playerMinorVote(Event* event)
{
    const NewNetworkString &data = event->data();
    if (!checkDataSizeAndToken(event, 9))
        return;
    if (!isByteCorrect(event, 5, 1))
        return;
    if (!isByteCorrect(event, 7, 4))
        return;
    m_setup->getRaceConfig()->setPlayerMinorVote(data[6], data[8]);
}   // playerMinorVote

//-----------------------------------------------------------------------------

/*! \brief Called when a player votes for a track.
 *  \param event : Event providing the information.
 *
 *  Format of the data :
 *  Byte 0   1            5   6           7   8            N+8 N+9              N+10
 *       ---------------------------------------------------------------------------
 *  Size | 1 |      4     | 1 |      1    | 1 |      N     | 1 |       1           |
 *  Data | 4 | priv token | 1 | player id | N | track name | 1 | track number (gp) |
 *       ---------------------------------------------------------------------------
 */
void ClientLobbyRoomProtocol::playerTrackVote(Event* event)
{
    const NewNetworkString &data = event->data();
    if (!checkDataSizeAndToken(event, 10))
        return;
    if (!isByteCorrect(event, 5, 1))
        return;
    std::string track_name;
    int N = data.decodeString(7, &track_name);
    if (!isByteCorrect(event, N+7, 1))
        return;
    m_setup->getRaceConfig()->setPlayerTrackVote(data[6], track_name,
                                                 data[N+8]);
}   // playerTrackVote

//-----------------------------------------------------------------------------

/*! \brief Called when a player votes for the reverse mode of a race
 *  \param event : Event providing the information.
 *
 *  Format of the data :
 *  Byte 0   1            5   6           7   8          9   10                  11
 *       -------------------------------------------------------------------------
 *  Size | 1 |      4     | 1 |     1     | 1 |     1    | 1 |       1           |
 *  Data | 4 | priv token | 1 | player id | 1 | reversed | 1 | track number (gp) |
 *       -------------------------------------------------------------------------
 */
void ClientLobbyRoomProtocol::playerReversedVote(Event* event)
{
    const NewNetworkString &data = event->data();
    if (!checkDataSizeAndToken(event, 11))
        return;
    if (!isByteCorrect(event, 5, 1))
        return;
    if (!isByteCorrect(event, 7, 1))
        return;
    if (!isByteCorrect(event, 9, 1))
        return;
    m_setup->getRaceConfig()->setPlayerReversedVote(data[6], data[8]!=0, data[10]);
}   // playerReversedVote

//-----------------------------------------------------------------------------

/*! \brief Called when a player votes for a major race mode.
 *  \param event : Event providing the information.
 *
 *  Format of the data :
 *  Byte 0   1            5   6           7   8      9   10                  11
 *       ---------------------------------------------------------------------
 *  Size | 1 |      4     | 1 |     1     | 1 |   1  | 1 |       1           |
 *  Data | 4 | priv token | 1 | player id | 1 | laps | 1 | track number (gp) |
 *       ---------------------------------------------------------------------
 */
void ClientLobbyRoomProtocol::playerLapsVote(Event* event)
{
    const NewNetworkString &data = event->data();
    if (!checkDataSizeAndToken(event, 9))
        return;
    if (!isByteCorrect(event, 5, 1))
        return;
    if (!isByteCorrect(event, 7, 1))
        return;
    m_setup->getRaceConfig()->setPlayerLapsVote(data[6], data[8], data[10]);
}   // playerLapsVote

//-----------------------------------------------------------------------------
