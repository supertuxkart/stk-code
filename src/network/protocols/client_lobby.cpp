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

#include "network/protocols/client_lobby.hpp"

#include "config/player_manager.hpp"
#include "modes/world_with_rank.hpp"
#include "network/event.hpp"
#include "network/network_config.hpp"
#include "network/network_player_profile.hpp"
#include "network/protocol_manager.hpp"
#include "network/protocols/latency_protocol.hpp"
#include "network/race_event_manager.hpp"
#include "network/stk_host.hpp"
#include "network/stk_peer.hpp"
#include "online/online_profile.hpp"
#include "states_screens/networking_lobby.hpp"
#include "states_screens/network_kart_selection.hpp"
#include "states_screens/race_result_gui.hpp"
#include "states_screens/state_manager.hpp"
#include "utils/log.hpp"

// ============================================================================
/** The protocol that manages starting a race with the server. It uses a 
 *  finite state machine:
\dot
digraph interaction {
"NONE" -> "LINKED" [label="ENet connection with server established"]
"LINKED" -> "REQUESTING_CONNECTION" [label="Request connection from server"]
"REQUESTING_CONNECTION" -> CONNECTED [label="Connection accepted by server"]
"REQUESTING_CONNECTION" -> "?? TO BE DONE ??" [label="Connection refused by server"]
"CONNECTED" -> "KART_SELECTION" [label="Server tells us to start kart selection"]
"KART_SELECTION" -> "SELECTING_KARTS" [label="Show kart selection screen"]
"SELECTING_KARTS" -> "PLAYING" [label="Server sends start race message"]
}
\enddot
Note that some states are actually managed outside of the client lobby. For
example to select race details after selecting a kart is managed by the GUI
engine.

*/

ClientLobby::ClientLobby() : LobbyProtocol(NULL)
{

    m_server_address.clear();
    m_server = NULL;
    setHandleDisconnections(true);
}   // ClientLobby

//-----------------------------------------------------------------------------

ClientLobby::~ClientLobby()
{
}   // ClientLobby

//-----------------------------------------------------------------------------
/** Sets the address of the server. 
 */
void ClientLobby::setAddress(const TransportAddress &address)
{
    m_server_address.copy(address);
}   // setAddress
//-----------------------------------------------------------------------------

void ClientLobby::setup()
{
    m_game_setup = STKHost::get()->setupNewGame(); // create a new game setup
    m_state = NONE;
}   // setup

//-----------------------------------------------------------------------------
/** Sends the selection of a kart from this client to the server.
 *  \param player_id The global player id of the voting player.
 *  \param kart_name Name of the selected kart.
 */
void ClientLobby::requestKartSelection(uint8_t player_id,
                                       const std::string &kart_name)
{
    NetworkString *request = getNetworkString(3+kart_name.size());
    request->addUInt8(LE_KART_SELECTION).addUInt8(player_id)
            .encodeString(kart_name);
    sendToServer(request, /*reliable*/ true);
    delete request;
}   // requestKartSelection

//-----------------------------------------------------------------------------
/** Sends a vote for a major vote from a client to the server. Note that even
 *  this client will only store the vote when it is received back from the
 *  server.
 *  \param player_id The global player id of the voting player.
 *  \param major Major mode voted for.
 */
void ClientLobby::voteMajor(uint8_t player_id, uint32_t major)
{
    NetworkString *request = getNetworkString(6);
    request->addUInt8(LE_VOTE_MAJOR).addUInt8(player_id)
           .addUInt32(major);
    sendToServer(request, true);
    delete request;
}   // voteMajor

//-----------------------------------------------------------------------------
/** Sends a vote for the number of tracks from a client to the server. Note
 *  that even this client will only store the vote when it is received back
 *  from the server.
 *  \param player_id The global player id of the voting player.
 *  \param count NUmber of tracks to play.
 */
void ClientLobby::voteRaceCount(uint8_t player_id, uint8_t count)
{
    NetworkString *request = getNetworkString(3);
    request->addUInt8(LE_VOTE_RACE_COUNT).addUInt8(player_id).addUInt8(count);
    sendToServer(request, true);
    delete request;
}   // voteRaceCount

//-----------------------------------------------------------------------------
/** Sends a vote for the minor game mode from a client to the server. Note that
 *  even this client will only store the vote when it is received back from the
 *  server.
 *  \param player_id The global player id of the voting player.
 *  \param minor Voted minor mode.
 */
void ClientLobby::voteMinor(uint8_t player_id, uint32_t minor)
{
    NetworkString *request = getNetworkString(6);
    request->addUInt8(LE_VOTE_MINOR).addUInt8(player_id).addUInt32(minor);
    sendToServer(request, true);
    delete request;
}   // voteMinor

//-----------------------------------------------------------------------------
/** Sends the vote about which track to play at which place in the list of
 *  tracks (like a custom GP definition). Note that even this client will only
 *  store the vote when it is received back from the server.
 *  \param player_id The global player id of the voting player.
 *  \param track Name of the track.
 *  \param At which place in the list of tracks this track should be played.
 */
void ClientLobby::voteTrack(uint8_t player_id,
                                        const std::string &track,
                                        uint8_t track_nb)
{
    NetworkString *request = getNetworkString(2+1+track.size());
    request->addUInt8(LE_VOTE_TRACK).addUInt8(player_id).addUInt8(track_nb)
            .encodeString(track);
    sendToServer(request, true);
    delete request;
}   // voteTrack

//-----------------------------------------------------------------------------
/** Sends a vote if a track at a specified place in the list of all tracks
 *  should be played in reverse or not. Note that even this client will only
 *  store the vote when it is received back from the server.
 *  \param player_id Global player id of the voting player.
 *  \param reversed True if the track should be played in reverse.
 *  \param track_nb Index for the track to be voted on in the list of all
 *         tracks.
 */
void ClientLobby::voteReversed(uint8_t player_id, bool reversed, 
                                           uint8_t track_nb)
{
    NetworkString *request = getNetworkString(9);
    request->addUInt8(LE_VOTE_REVERSE).addUInt8(player_id).addUInt8(reversed)
            .addUInt8(track_nb);
    sendToServer(request, true);
    delete request;
}   // voteReversed

//-----------------------------------------------------------------------------
/** Vote for the number of laps of the specified track. Note that even this
 *  client will only store the vote when it is received back from the server.
 *  \param player_id Global player id of the voting player.
 *  \param laps Number of laps for the specified track.
 *  \param track_nb Index of the track in the list of all tracks.
 */
void ClientLobby::voteLaps(uint8_t player_id, uint8_t laps,
                                       uint8_t track_nb)
{
    NetworkString *request = getNetworkString(10);
    request->addUInt8(LE_VOTE_LAPS).addUInt8(player_id).addUInt8(laps)
            .addUInt8(track_nb);
    sendToServer(request, true);
    delete request;
}   // voteLaps

//-----------------------------------------------------------------------------
/** Called when a client selects to exit a server.
 */
void ClientLobby::leave()
{
    m_server->disconnect();
    STKHost::get()->removePeer(m_server);
    m_server_address.clear();
    ServersManager::get()->unsetJoinedServer();
}   // leave

//-----------------------------------------------------------------------------
/** Called from the gui when a client clicked on 'continue' on the race result
 *  screen. It notifies the server that this client has exited the screen and
 *  is back at the lobby.
 */
void ClientLobby::doneWithResults()
{
    NetworkString *done = getNetworkString(1);
    done->addUInt8(LE_RACE_FINISHED_ACK);
    sendToServer(done, /*reliable*/true);
    delete done;
}   // doneWithResults

//-----------------------------------------------------------------------------

bool ClientLobby::notifyEvent(Event* event)
{
    assert(m_game_setup); // assert that the setup exists

    NetworkString &data = event->data();
    assert(data.size()); // assert that data isn't empty
    uint8_t message_type = data.getUInt8();
    Log::info("ClientLobby", "Synchronous message of type %d",
              message_type);
    switch(message_type)
    {
        case LE_KART_SELECTION_UPDATE: kartSelectionUpdate(event); break;
        case LE_LOAD_WORLD:            loadWorld();                break;
        case LE_RACE_FINISHED:         raceFinished(event);        break;
        case LE_EXIT_RESULT:           exitResultScreen(event);    break;
        default:
            return false;
            break;
    }   // switch
    return true;
}   // notifyEvent

//-----------------------------------------------------------------------------

bool ClientLobby::notifyEventAsynchronous(Event* event)
{
    assert(m_game_setup); // assert that the setup exists
    if (event->getType() == EVENT_TYPE_MESSAGE)
    {
        NetworkString &data = event->data();
        assert(data.size()); // assert that data isn't empty
        uint8_t message_type = data.getUInt8();

        Log::info("ClientLobby", "Asynchronous message of type %d",
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

void ClientLobby::update(float dt)
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

        Protocol *p = new LatencyProtocol();
        p->requestStart();
        Log::info("LobbyProtocol", "LatencyProtocol started.");
    }
    break;
    case SELECTING_KARTS:
        break;
    case PLAYING:
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
void ClientLobby::newPlayer(Event* event)
{
    if (!checkDataSize(event, 2)) return;
    const NetworkString &data = event->data();

    uint8_t player_id = data.getUInt8();
    uint8_t host_id   = data.getUInt8();
    core::stringw name;
    data.decodeStringW(&name);
    // FIXME need adjusting when splitscreen is used/
    if(STKHost::get()->getGameSetup()->isLocalMaster(player_id))
    {
        Log::error("ClientLobby",
                   "The server notified me that I'm a new player in the "
                   "room (not normal).");
    }
    else if (m_game_setup->getProfile(player_id) == NULL)
    {
        Log::verbose("ClientLobby", "New player connected.");
        NetworkPlayerProfile* profile = 
                      new NetworkPlayerProfile(name, player_id, host_id);
        m_game_setup->addPlayer(profile);
        NetworkingLobby::getInstance()->addPlayer(profile);
    }
    else
    {
        Log::error("ClientLobby",
                   "One of the player notified in the list is myself.");
    }
}   // newPlayer

//-----------------------------------------------------------------------------

/*! \brief Called when a new player is disconnected
 *  \param event : Event providing the information.
 *
 *  Format of the data :
 *  Byte 0 
 *       --------------
 *  Size |    1       |
 *  Data | player id *|
 *       --------------
 */
void ClientLobby::disconnectedPlayer(Event* event)
{
    if (!checkDataSize(event, 1)) return;

    NetworkString &data = event->data();
    while(data.size()>0)
    {
        const NetworkPlayerProfile *profile = 
                        m_game_setup->getProfile(data.getUInt8());
        if (m_game_setup->removePlayer(profile))
        {
            Log::info("ClientLobby",
                      "Player %d removed successfully.",
                      profile->getGlobalPlayerId());
        }
        else
        {
            Log::error("ClientLobby",
                       "The disconnected peer wasn't known.");
        }
    }   // while

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
void ClientLobby::connectionAccepted(Event* event)
{
    // At least 3 bytes should remain now
    if(!checkDataSize(event, 3)) return;

    NetworkString &data = event->data();
    STKPeer* peer = event->getPeer();

    // Accepted
    // ========
    Log::info("ClientLobby",
              "The server accepted the connection.");

    // self profile
    irr::core::stringw name;
    if (PlayerManager::getCurrentOnlineState() == PlayerProfile::OS_SIGNED_IN)
        name = PlayerManager::getCurrentOnlineUserName();
    else
        name = PlayerManager::getCurrentPlayer()->getName();
    uint8_t my_player_id = data.getUInt8();
    uint8_t my_host_id   = data.getUInt8();
    uint8_t authorised   = data.getUInt8();
    // Store this client's authorisation status in the peer information
    // for the server.
    event->getPeer()->setAuthorised(authorised!=0);
    STKHost::get()->setMyHostId(my_host_id);

    NetworkPlayerProfile* profile = 
        new NetworkPlayerProfile(name, my_player_id, my_host_id);
    STKHost::get()->getGameSetup()->setLocalMaster(my_player_id);
    m_game_setup->setNumLocalPlayers(1);
    // connection token
    uint32_t token = data.getToken();
    peer->setClientServerToken(token);

    // Add all players
    // ===============
    while (data.size() > 0)
    {
        uint8_t player_id = data.getUInt8();
        uint8_t host_id   = data.getUInt8();
        irr::core::stringw name;
        int bytes_read = data.decodeStringW(&name);
        
        NetworkPlayerProfile* profile2 =
            new NetworkPlayerProfile(name, player_id, host_id);
        m_game_setup->addPlayer(profile2);
        // Inform the network lobby of all players so that the GUI can
        // show all currently connected players.
        NetworkingLobby::getInstance()->addPlayer(profile2);
    }

    // Add self after other players so that player order is identical
    // on server and all clients.
    m_game_setup->addPlayer(profile);
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
void ClientLobby::connectionRefused(Event* event)
{
    if (!checkDataSize(event, 1)) return;
    const NetworkString &data = event->data();
    
    switch (data.getUInt8()) // the second byte
    {
    case 0:
        Log::info("ClientLobby",
                  "Connection refused : too many players.");
        break;
    case 1:
        Log::info("ClientLobby", "Connection refused : banned.");
        break;
    case 2:
        Log::info("ClientLobby", "Client busy.");
        break;
    default:
        Log::info("ClientLobby", "Connection refused.");
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
void ClientLobby::kartSelectionRefused(Event* event)
{
    if(!checkDataSize(event, 1)) return;

    const NetworkString &data = event->data();

    switch (data.getUInt8()) // the error code
    {
    case 0:
        Log::info("ClientLobby",
                  "Kart selection refused : already taken.");
        break;
    case 1:
        Log::info("ClientLobby",
                  "Kart selection refused : not available.");
        break;
    default:
        Log::info("ClientLobby", "Kart selection refused.");
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
void ClientLobby::kartSelectionUpdate(Event* event)
{
    if(!checkDataSize(event, 3)) return;
    const NetworkString &data = event->data();
    uint8_t player_id = data.getUInt8();
    std::string kart_name;
    data.decodeString(&kart_name);
    if (!m_game_setup->isKartAvailable(kart_name))
    {
        Log::error("ClientLobby",
                   "The updated kart is taken already.");
    }
    m_game_setup->setPlayerKart(player_id, kart_name);
    NetworkKartSelectionScreen::getInstance()->playerSelected(player_id,
                                                              kart_name);
}   // kartSelectionUpdate

//-----------------------------------------------------------------------------

/*! \brief Called when the server broadcasts to start the race.
race needs to be started.
 *  \param event : Event providing the information (no additional information
 *                 in this case).
 */
void ClientLobby::startGame(Event* event)
{
    m_state = PLAYING;
    // Triggers the world finite state machine to go from WAIT_FOR_SERVER_PHASE
    // to READY_PHASE.
    World::getWorld()->setReadyToRace();
    Log::info("ClientLobby", "Starting new game");
}   // startGame

//-----------------------------------------------------------------------------
/** Called from WorldStatus when reaching the READY phase, i.e. when the race
 *  was started. It is going to inform the server of the race start. This
 *  allows the server to wait for all clients to start, so the server will
 *  be running behind the client with the biggest latency, which should
 *  make it likely that at local time T on the server all messages from
 *  all clients at their local time T have arrived.
 */
void ClientLobby::startingRaceNow()
{
    NetworkString *ns = getNetworkString(2);
    ns->addUInt8(LE_STARTED_RACE);
    sendToServer(ns, /*reliable*/true);
    terminateLatencyProtocol();
}   // startingRaceNow

//-----------------------------------------------------------------------------
/*! \brief Called when the kart selection starts.
 *  \param event : Event providing the information (no additional information
 *                 in this case).
 */
void ClientLobby::startSelection(Event* event)
{
    m_state = KART_SELECTION;
    Log::info("ClientLobby", "Kart selection starts now");
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
void ClientLobby::raceFinished(Event* event)
{
    if(!checkDataSize(event, 1)) return;

    NetworkString &data = event->data();
    Log::error("ClientLobby",
               "Server notified that the race is finished.");

    // stop race protocols
    Protocol* protocol = ProtocolManager::getInstance()
                       ->getProtocol(PROTOCOL_CONTROLLER_EVENTS);
    if (protocol)
        ProtocolManager::getInstance()->requestTerminate(protocol);
    else
        Log::error("ClientLobby",
                   "No controller events protocol registered.");

    protocol = ProtocolManager::getInstance() 
             ->getProtocol(PROTOCOL_KART_UPDATE);
    if (protocol)
        ProtocolManager::getInstance()->requestTerminate(protocol);
    else
        Log::error("ClientLobby",
                   "No kart update protocol registered.");

    protocol = ProtocolManager::getInstance()
             ->getProtocol(PROTOCOL_GAME_EVENTS);
    if (protocol)
        ProtocolManager::getInstance()->requestTerminate(protocol);
    else
        Log::error("ClientLobby",
                   "No game events protocol registered.");

    // finish the race
    WorldWithRank* ranked_world = (WorldWithRank*)(World::getWorld());
    ranked_world->beginSetKartPositions();
    ranked_world->setPhase(WorldStatus::RESULT_DISPLAY_PHASE);
    int position = 1;
    while(data.size()>0)
    {
        uint8_t kart_id = data.getUInt8();
        ranked_world->setKartPosition(kart_id,position);
        Log::info("ClientLobby", "Kart %d has finished #%d",
                  kart_id, position);
        position++;
    }
    ranked_world->endSetKartPositions();
    m_state = RACE_FINISHED;
    ranked_world->terminateRace();
}   // raceFinished

//-----------------------------------------------------------------------------
/** Called when the server informs the clients to exit the race result screen.
 *  It exits the race, and goes back to the lobby.
 */
void ClientLobby::exitResultScreen(Event *event)
{
    RaceResultGUI::getInstance()->backToLobby();
}   // exitResultScreen

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
void ClientLobby::playerMajorVote(Event* event)
{
    const NetworkString &data = event->data();
    if (!checkDataSize(event, 2))
        return;
    uint8_t player_id = data.getUInt8();
    uint8_t mode      = data.getUInt8();
    m_game_setup->getRaceConfig()->setPlayerMajorVote(player_id, mode);
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
void ClientLobby::playerRaceCountVote(Event* event)
{
    if (!checkDataSize(event, 2)) return;
    const NetworkString &data = event->data();
    uint8_t player_id = data.getUInt8();
    uint8_t count     = data.getUInt8();
    m_game_setup->getRaceConfig()->setPlayerRaceCountVote(player_id, count);
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
void ClientLobby::playerMinorVote(Event* event)
{
    if (!checkDataSize(event, 2)) return;
    const NetworkString &data = event->data();
    uint8_t player_id = data.getUInt8();
    uint8_t minor     = data.getUInt8();
    m_game_setup->getRaceConfig()->setPlayerMinorVote(player_id, minor);
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
void ClientLobby::playerTrackVote(Event* event)
{
    if (!checkDataSize(event, 3)) return;
    const NetworkString &data = event->data();
    std::string track_name;
    uint8_t player_id = data.getUInt8();
    uint8_t number    = data.getUInt8();
    int N = data.decodeString(&track_name);
    m_game_setup->getRaceConfig()->setPlayerTrackVote(player_id, track_name,
                                                 number);
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
void ClientLobby::playerReversedVote(Event* event)
{
    if (!checkDataSize(event, 3)) return;
    const NetworkString &data = event->data();
    uint8_t player_id = data.getUInt8();
    uint8_t reversed  = data.getUInt8();
    uint8_t number    = data.getUInt8();
    m_game_setup->getRaceConfig()->setPlayerReversedVote(player_id, reversed!=0,
                                                    number);
}   // playerReversedVote

//-----------------------------------------------------------------------------

/*! \brief Called when a player votes for a major race mode.
 *  \param event : Event providing the information.
 *
 *  Format of the data :
 *  Byte 0           1      2
 *       ----------------------------------------
 *  Size |     1     |   1  |       1           |
 *  Data | player id | laps | track number (gp) |
 *       ----------------------------------------
 */
void ClientLobby::playerLapsVote(Event* event)
{
    if (!checkDataSize(event, 3)) return;
    const NetworkString &data = event->data();
    uint8_t player_id = data.getUInt8();
    uint8_t laps      = data.getUInt8();
    uint8_t number    = data.getUInt8();
    m_game_setup->getRaceConfig()->setPlayerLapsVote(player_id, laps, number);
}   // playerLapsVote

//-----------------------------------------------------------------------------
/** Callback when the world is loaded. The client will inform the server
 *  that the players on this host are ready to start the race. It is called by
 *  the RaceManager after the world is loaded.
 */
void ClientLobby::finishedLoadingWorld()
{
    assert(STKHost::get()->getPeerCount() == 1);
    std::vector<NetworkPlayerProfile*> players =
                                         STKHost::get()->getMyPlayerProfiles();
    NetworkString *ns = getNetworkString(2);
    ns->addUInt8(LE_CLIENT_LOADED_WORLD);
    ns->addUInt8( uint8_t(players.size()) ) ;
    for (unsigned int i = 0; i < players.size(); i++)
    {
        ns->addUInt8(players[i]->getGlobalPlayerId());
        Log::info("ClientLobby", 
                  "Player %d ready, notifying server.",
                  players[i]->getGlobalPlayerId());
    }   // for i < players.size()
    sendToServer(ns, /*reliable*/true);
    delete ns;
}   // finishedLoadingWorld
