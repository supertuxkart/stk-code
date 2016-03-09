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
    setHandleDisconnections(true);
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
    NetworkString *request = getNetworkString(2+kart_name.size());
    request->addUInt8(LE_KART_SELECTION).encodeString(kart_name);
    sendToServer(request, /*reliable*/ true);
    delete request;
}   // requestKartSelection

//-----------------------------------------------------------------------------

void ClientLobbyRoomProtocol::voteMajor(uint32_t major)
{
    NetworkString *request = getNetworkString(8);
    request->addUInt8(LE_VOTE_MAJOR).addUInt8(4)
           .addUInt8(4).addUInt32(major);
    sendToServer(request, true);
    delete request;
}   // voteMajor

//-----------------------------------------------------------------------------

void ClientLobbyRoomProtocol::voteRaceCount(uint8_t count)
{
    NetworkString *request = getNetworkString(8);
    request->addUInt8(LE_VOTE_RACE_COUNT).addUInt8(count);
    sendToServer(request, true);
    delete request;
}   // voteRaceCount

//-----------------------------------------------------------------------------

void ClientLobbyRoomProtocol::voteMinor(uint32_t minor)
{
    NetworkString *request = getNetworkString(8);
    request->addUInt8(LE_VOTE_MINOR).addUInt32(minor);
    sendToServer(request, true);
    delete request;
}   // voteMinor

//-----------------------------------------------------------------------------

void ClientLobbyRoomProtocol::voteTrack(const std::string &track,
                                        uint8_t track_nb)
{
    NetworkString *request = getNetworkString(2+1+track.size());
    request->addUInt8(LE_VOTE_TRACK).addUInt8(track_nb).encodeString(track);
    sendToServer(request, true);
    delete request;
}   // voteTrack

//-----------------------------------------------------------------------------

void ClientLobbyRoomProtocol::voteReversed(bool reversed, uint8_t track_nb)
{
    NetworkString *request = getNetworkString(9);
    request->addUInt8(LE_VOTE_REVERSE).addUInt8(reversed).addUInt8(track_nb);
    sendToServer(request, true);
    delete request;
}   // voteReversed

//-----------------------------------------------------------------------------

void ClientLobbyRoomProtocol::voteLaps(uint8_t laps, uint8_t track_nb)
{
    NetworkString *request = getNetworkString(10);
    request->addUInt8(LE_VOTE_LAPS).addUInt8(laps).addUInt8(track_nb);
    sendToServer(request, true);
    delete request;
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
        NetworkString &data = event->data();
        assert(data.size()); // assert that data isn't empty
        uint8_t message_type = data[0];
        if (message_type != LE_KART_SELECTION_UPDATE &&
            message_type != LE_RACE_FINISHED            )
            return false; // don't treat the event

        data.removeFront(1);
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
        NetworkString &data = event->data();
        assert(data.size()); // assert that data isn't empty
        uint8_t message_type = data[0];

        data.removeFront(1);
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
    else if (event->getType() == EVENT_TYPE_DISCONNECTED) 
    {
        // This means we left essentially.
        // We can't delete STKHost from this thread, since the main
        // thread might still test if STKHost exists and then call
        // the ProtocolManager, which might already have been deleted.
        // So only signal that STKHost should exit, which will be tested
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
        NetworkString *ns = getNetworkString(6+1+name_u8.size()
                                                 +1+password.size());
        // 4 (size of id), global id
        ns->addUInt8(LE_CONNECTION_REQUESTED).encodeString(name)
          .encodeString(NetworkConfig::get()->getPassword());
        sendToServer(ns);
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
 *  Byte 0            1         2          
 *       -------------------------------------
 *  Size |     1      |    1   |             |
 *  Data | player_id  | hostid | player name |
 *       -------------------------------------
 */
void ClientLobbyRoomProtocol::newPlayer(Event* event)
{
    if (!checkDataSize(event, 2)) return;
    const NetworkString &data = event->data();

    uint8_t player_id = data[0];
    uint8_t hostid    = data[1];
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
 *  Byte 0 
 *       -------------
 *  Size |    1      |
 *  Data | player id |
 *       -------------
 */
void ClientLobbyRoomProtocol::disconnectedPlayer(Event* event)
{
    if (!checkDataSize(event, 1)) return;

    const NetworkString &data = event->data();
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
 *  Byte 0                   1        2            3       
 *       ---------------------------------------------------------
 *  Size |    1     |   1    | 1          |             |
 *  Data | player_id| hostid | authorised |playernames* |
 *       ---------------------------------------------------------
 */
void ClientLobbyRoomProtocol::connectionAccepted(Event* event)
{
    // At least 3 bytes should remain now
    if(!checkDataSize(event, 3)) return;

    NetworkString &data = event->data();
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
    uint8_t my_player_id = data[0];
    uint8_t my_host_id   = data[1];
    uint8_t authorised   = data[2];
    data.removeFront(3);
    // Store this client's authorisation status in the peer information
    // for the server.
    event->getPeer()->setAuthorised(authorised!=0);
    STKHost::get()->setMyHostId(my_host_id);

    NetworkPlayerProfile* profile = new NetworkPlayerProfile(my_player_id, name);
    profile->setHostId(my_host_id);
    STKHost::get()->getGameSetup()->setLocalMaster(my_player_id);
    m_setup->setNumLocalPlayers(1);
    // connection token
    uint32_t token = data.getToken();
    peer->setClientServerToken(token);

    // Add all players
    // ===============
    while (data.size() > 0)
    {
        uint8_t player_id = data[0];
        uint8_t host_id   = data[1];
        irr::core::stringw name;
        int bytes_read = data.decodeStringW(2, &name);
        
        NetworkPlayerProfile* profile2 =
            new NetworkPlayerProfile(player_id, name);
        profile2->setHostId(host_id);
        m_setup->addPlayer(profile2);
        // Inform the network lobby of all players so that the GUI can
        // show all currently connected players.
        NetworkingLobby::getInstance()->addPlayer(profile2);
        data.removeFront(bytes_read+2);
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
 *  Byte 0 
 *       ----------------
 *  Size |      1       |
 *  Data | refusal code |
 *       ----------------
 */
void ClientLobbyRoomProtocol::connectionRefused(Event* event)
{
    if (!checkDataSize(event, 1)) return;
    const NetworkString &data = event->data();
    
    switch (data[0]) // the second byte
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
 *  Byte 0
 *       ----------------
 *  Size |      1       |
 *  Data | refusal code |
 *       ----------------
 */
void ClientLobbyRoomProtocol::kartSelectionRefused(Event* event)
{
    if(!checkDataSize(event, 1)) return;

    const NetworkString &data = event->data();

    switch (data[0]) // the error code
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
 *  Byte 0           1           2                    3           N+3
 *       --------------------------------------------------
 *  Size |    1      |       1            |     N     |
 *  Data | player id | N (kart name size) | kart name |
 *       --------------------------------------------------
 */
void ClientLobbyRoomProtocol::kartSelectionUpdate(Event* event)
{
    if(!checkDataSize(event, 3)) return;
    const NetworkString &data = event->data();
    uint8_t player_id = data[0];
    std::string kart_name;
    data.decodeString(1, &kart_name);
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
 *  \param event : Event providing the information (no additional information
 *                 in this case).
 */
void ClientLobbyRoomProtocol::startGame(Event* event)
{
    const NetworkString &data = event->data();
    m_state = PLAYING;
    ProtocolManager::getInstance()
        ->requestStart(new StartGameProtocol(m_setup));
    Log::info("ClientLobbyRoomProtocol", "Starting new game");
}   // startGame

//-----------------------------------------------------------------------------

/*! \brief Called when the kart selection starts.
 *  \param event : Event providing the information (no additional information
 *                 in this case).
 */
void ClientLobbyRoomProtocol::startSelection(Event* event)
{
    m_state = KART_SELECTION;
    Log::info("ClientLobbyRoomProtocol", "Kart selection starts now");
}   // startSelection

//-----------------------------------------------------------------------------

/*! \brief Called when all karts have finished the race.
 *  \param event : Event providing the information.
 *
 *  Format of the data :
 *  Byte 0           1
 *       -------------------------------
 *  Size |     1     |     1     |     |
 *  Data | Kart 1 ID | kart id 2 | ... |
 *       -------------------------------
 */
void ClientLobbyRoomProtocol::raceFinished(Event* event)
{
    if(!checkDataSize(event, 1)) return;

    NetworkString &data = event->data();
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
        uint8_t kart_id = data[0];
        ranked_world->setKartPosition(kart_id,position);
        Log::info("ClientLobbyRoomProtocol", "Kart %d has finished #%d",
                  kart_id, position);
        data.removeFront(1);
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
 *  Byte 0          1                 2
 *       ------------------------------
 *  Size |    1     |        1        |
 *  Data |player id | major mode vote |
 *       ------------------------------
 */
void ClientLobbyRoomProtocol::playerMajorVote(Event* event)
{
    const NetworkString &data = event->data();
    if (!checkDataSize(event, 2))
        return;
    m_setup->getRaceConfig()->setPlayerMajorVote(data[0], data[1]);
}   // playerMajorVote

//-----------------------------------------------------------------------------
/*! \brief Called when a player votes for the number of races in a GP.
 *  \param event : Event providing the information.
 *
 *  Format of the data :
 *  Byte 0           1
 *       ---------------------------
 *  Size |     1     |      1      |
 *  Data | player id | races count |
 *       ---------------------------
 */
void ClientLobbyRoomProtocol::playerRaceCountVote(Event* event)
{
    if (!checkDataSize(event, 2)) return;
    const NetworkString &data = event->data();
    m_setup->getRaceConfig()->setPlayerRaceCountVote(data[0], data[1]);
}   // playerRaceCountVote

//-----------------------------------------------------------------------------
/*! \brief Called when a player votes for a minor race mode.
 *  \param event : Event providing the information.
 *
 *  Format of the data :
 *  Byte 0           1
 *       -------------------------------
 *  Size |      1    |        4        |
 *  Data | player id | minor mode vote |
 *       -------------------------------
 */
void ClientLobbyRoomProtocol::playerMinorVote(Event* event)
{
    if (!checkDataSize(event, 2)) return;
    const NetworkString &data = event->data();
    m_setup->getRaceConfig()->setPlayerMinorVote(data[0], data[1]);
}   // playerMinorVote

//-----------------------------------------------------------------------------

/*! \brief Called when a player votes for a track.
 *  \param event : Event providing the information.
 *
 *  Format of the data :
 *  Byte 0           1                   2   3
 *       --------------------------------------------------
 *  Size |      1    |       1           | 1 |     N      |
 *  Data | player id | track number (gp) | N | track name |
 *       --------------------------------------------------
 */
void ClientLobbyRoomProtocol::playerTrackVote(Event* event)
{
    if (!checkDataSize(event, 3)) return;
    const NetworkString &data = event->data();
    std::string track_name;
    int N = data.decodeString(2, &track_name);
    m_setup->getRaceConfig()->setPlayerTrackVote(data[0], track_name,
                                                 data[1]);
}   // playerTrackVote

//-----------------------------------------------------------------------------

/*! \brief Called when a player votes for the reverse mode of a race
 *  \param event : Event providing the information.
 *
 *  Format of the data :
 *  Byte 0           1         2
 *       -------------------------------------------
 *  Size |     1     |    1    |       1           |
 *  Data | player id |reversed | track number (gp) |
 *       -------------------------------------------
 */
void ClientLobbyRoomProtocol::playerReversedVote(Event* event)
{
    if (!checkDataSize(event, 3)) return;
    const NetworkString &data = event->data();
    m_setup->getRaceConfig()->setPlayerReversedVote(data[0], data[1]!=0,
                                                    data[2]);
}   // playerReversedVote

//-----------------------------------------------------------------------------

/*! \brief Called when a player votes for a major race mode.
 *  \param event : Event providing the information.
 *
 *  Format of the data :
 *  Byte 0           1      2
 *       ----------------------------------------
 *  Size |     1     |   1  |       1           |
 *  Data | player id | laps | track number (gp) |
 *       ----------------------------------------
 */
void ClientLobbyRoomProtocol::playerLapsVote(Event* event)
{
    if (!checkDataSize(event, 3)) return;
    const NetworkString &data = event->data();
    m_setup->getRaceConfig()->setPlayerLapsVote(data[0], data[1], data[2]);
}   // playerLapsVote

//-----------------------------------------------------------------------------
