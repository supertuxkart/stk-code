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
#include "karts/kart_properties_manager.hpp"
#include "guiengine/modaldialog.hpp"
#include "guiengine/message_queue.hpp"
#include "modes/world_with_rank.hpp"
#include "network/event.hpp"
#include "network/network_config.hpp"
#include "network/network_player_profile.hpp"
#include "network/protocol_manager.hpp"
#include "network/protocols/latency_protocol.hpp"
#include "network/race_event_manager.hpp"
#include "network/stk_host.hpp"
#include "network/stk_peer.hpp"
#include "online/online_player_profile.hpp"
#include "online/online_profile.hpp"
#include "states_screens/networking_lobby.hpp"
#include "states_screens/network_kart_selection.hpp"
#include "states_screens/race_result_gui.hpp"
#include "states_screens/state_manager.hpp"
#include "tracks/track_manager.hpp"
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
        case LE_START_SELECTION:       startSelection(event);      break;
        case LE_KART_SELECTION_UPDATE: kartSelectionUpdate(event); break;
        case LE_LOAD_WORLD:            loadWorld();                break;
        case LE_RACE_FINISHED:         raceFinished(event);        break;
        case LE_EXIT_RESULT:           exitResultScreen(event);    break;
        case LE_UPDATE_PLAYER_LIST:    updatePlayerList(event);    break;
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
            case LE_PLAYER_DISCONNECTED : disconnectedPlayer(event);     break;
            case LE_START_RACE: startGame(event);                        break;
            case LE_CONNECTION_REFUSED: connectionRefused(event);        break;
            case LE_CONNECTION_ACCEPTED: connectionAccepted(event);      break;
            case LE_KART_SELECTION_REFUSED: kartSelectionRefused(event); break;
            case LE_VOTE_MAJOR : playerMajorVote(event);                 break;
            case LE_VOTE_RACE_COUNT: playerRaceCountVote(event);         break;
            case LE_VOTE_MINOR: playerMinorVote(event);                  break;
            case LE_VOTE_TRACK: playerTrackVote(event);                  break;
            case LE_VOTE_REVERSE: playerReversedVote(event);             break;
            case LE_VOTE_LAPS: playerLapsVote(event);                    break;
            case LE_SERVER_OWNERSHIP: becomingServerOwner();             break;
            default:                                                     break;
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
        STKHost::get()->disconnectAllPeers(false/*timeout_waiting*/);
        switch(event->getPeerDisconnectInfo())
        {
            case PDI_TIMEOUT:
                STKHost::get()->setErrorMessage(
                    _("Server connection timed out."));
                break;
            case PDI_NORMAL:
                STKHost::get()->setErrorMessage(
                    _("Server has been shut down."));
                break;
            case PDI_KICK:
                STKHost::get()->setErrorMessage(
                    _("You were kicked from the server."));
                break;
        }   // switch
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
        // atm assume only 1 local player (no split screen yet)
        NetworkString *ns = getNetworkString();
        ns->addUInt8(LE_CONNECTION_REQUESTED)
            .encodeString(NetworkConfig::get()->getPassword());

        uint8_t num_player = 1;
        ns->addUInt8(num_player);
        for (unsigned i = 0; i < num_player; i++)
        {
            core::stringw name;
            PlayerProfile* player = PlayerManager::getCurrentPlayer();
            if (PlayerManager::getCurrentOnlineState() ==
                PlayerProfile::OS_SIGNED_IN)
                name = PlayerManager::getCurrentOnlineUserName();
            else
                name = player->getName();
            std::string name_u8 = StringUtils::wideToUtf8(name);
            ns->encodeString(name_u8).addFloat(player->getDefaultKartColor());
            Online::OnlinePlayerProfile* opp =
                dynamic_cast<Online::OnlinePlayerProfile*>(player);
            ns->addUInt32(opp && opp->getProfile() ?
                opp->getProfile()->getID() : 0);
            // Assume no handicap player now
            ns->addUInt8(PLAYER_DIFFICULTY_NORMAL);
        }
        auto all_k = kart_properties_manager->getAllAvailableKarts();
        auto all_t = track_manager->getAllTrackIdentifiers();
        if (all_k.size() >= 65536)
            all_k.resize(65535);
        if (all_t.size() >= 65536)
            all_t.resize(65535);
        ns->addUInt16((uint16_t)all_k.size()).addUInt16((uint16_t)all_t.size());
        for (const std::string& kart : all_k)
        {
            ns->encodeString(kart);
        }
        for (const std::string& track : all_t)
        {
            ns->encodeString(track);
        }

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
        // In case the user opened a user info dialog
        GUIEngine::ModalDialog::dismiss();
        NetworkKartSelectionScreen* screen =
                                     NetworkKartSelectionScreen::getInstance();
        screen->setAvailableKartsFromServer(m_available_karts);
        screen->push();
        m_state = SELECTING_KARTS;

        std::make_shared<LatencyProtocol>()->requestStart();
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
        requestTerminate();
        break;
    case EXITING:
        break;
    }
}   // update

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
    unsigned disconnected_player_count = data.getUInt8();
    for (unsigned i = 0; i < disconnected_player_count; i++)
    {
        core::stringw player_name;
        data.decodeStringW(&player_name);
        core::stringw msg = _("%s disconnected.", player_name);
        // Use the friend icon to avoid an error-like message
        MessageQueue::add(MessageQueue::MT_FRIEND, msg);
    }

}   // disconnectedPlayer

//-----------------------------------------------------------------------------
/*! \brief Called when the server accepts the connection.
 *  \param event : Event providing the information.
 */
void ClientLobby::connectionAccepted(Event* event)
{
    // At least 4 byte should remain now
    if (!checkDataSize(event, 4)) return;

    NetworkString &data = event->data();
    STKPeer* peer = event->getPeer();

    // Accepted
    // ========
    Log::info("ClientLobby", "The server accepted the connection.");

    STKHost::get()->setMyHostId(data.getUInt32());
    m_game_setup->setNumLocalPlayers(1);
    // connection token
    uint32_t token = data.getToken();
    peer->setClientServerToken(token);
    m_state = CONNECTED;
}   // connectionAccepted

//-----------------------------------------------------------------------------
void ClientLobby::updatePlayerList(Event* event)
{
    if (!checkDataSize(event, 1)) return;
    NetworkString& data = event->data();
    unsigned player_count = data.getUInt8();
    std::vector<std::tuple<uint32_t, uint32_t, core::stringw, int> > players;
    for (unsigned i = 0; i < player_count; i++)
    {
        std::tuple<uint32_t, uint32_t, core::stringw, int> pl;
        std::get<0>(pl) = data.getUInt32();
        std::get<1>(pl) = data.getUInt32();
        data.decodeStringW(&std::get<2>(pl));
        // icon to be used, see NetworkingLobby::loadedFromFile
        std::get<3>(pl) = data.getUInt8() == 1 /*if server owner*/ ? 0 :
            std::get<1>(pl) != 0 /*if online account*/ ? 1 : 2;
        players.push_back(pl);
    }
    NetworkingLobby::getInstance()->updatePlayers(players);
}   // updatePlayerList

//-----------------------------------------------------------------------------
void ClientLobby::becomingServerOwner()
{
    MessageQueue::add(MessageQueue::MT_GENERIC,
        _("You are now the owner of server."));
    STKHost::get()->setAuthorisedToControl(true);
    if (m_state == CONNECTED && NetworkConfig::get()->isAutoConnect())
    {
        // Send a message to the server to start
        NetworkString start(PROTOCOL_LOBBY_ROOM);
        start.setSynchronous(true);
        start.addUInt8(LobbyProtocol::LE_REQUEST_BEGIN);
        STKHost::get()->sendToServer(&start, true);
    }
}   // becomingServerOwner

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
    switch ((RejectReason)data.getUInt8()) // the second byte
    {
    case RR_BUSY:
        STKHost::get()->setErrorMessage(
            _("Connection refused: Server is busy."));
        break;
    case RR_BANNED:
        STKHost::get()->setErrorMessage(
            _("Connection refused: You are banned from the server."));
        break;
    case RR_INCORRECT_PASSWORD:
        STKHost::get()->setErrorMessage(
            _("Connection refused: Server password is incorrect."));
        break;
    case RR_INCOMPATIBLE_DATA:
        STKHost::get()->setErrorMessage(
            _("Connection refused: Game data is incompatible."));
        break;
    case RR_TOO_MANY_PLAYERS:
        STKHost::get()->setErrorMessage(
            _("Connection refused: Server is full."));
        break;
    }
    STKHost::get()->disconnectAllPeers(false/*timeout_waiting*/);
    STKHost::get()->requestShutdown();
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

/*! \brief Called when the server broadcasts to start the race to all clients.
 *  \param event : Event providing the information (no additional information
 *                 in this case).
 */
void ClientLobby::startGame(Event* event)
{
    m_state = PLAYING;
    // Triggers the world finite state machine to go from WAIT_FOR_SERVER_PHASE
    // to READY_PHASE.
    World::getWorld()->setReadyToRace();
    Log::info("ClientLobby", "Starting new game at %lf",
              StkTime::getRealTime());
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
    Log::verbose("ClientLobby", "StartingRaceNow at %lf",
                 StkTime::getRealTime());
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
    const NetworkString& data = event->data();
    const unsigned kart_num = data.getUInt16();
    const unsigned track_num = data.getUInt16();
    m_available_karts.clear();
    m_available_tracks.clear();
    for (unsigned i = 0; i < kart_num; i++)
    {
        std::string kart;
        data.decodeString(&kart);
        m_available_karts.insert(kart);
    }
    for (unsigned i = 0; i < track_num; i++)
    {
        std::string track;
        data.decodeString(&track);
        m_available_tracks.insert(track);
    }
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
    auto pm = ProtocolManager::lock();
    assert(pm);
    pm->findAndTerminate(PROTOCOL_CONTROLLER_EVENTS);
    pm->findAndTerminate(PROTOCOL_KART_UPDATE);
    pm->findAndTerminate(PROTOCOL_GAME_EVENTS);

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
    // Will be reset to linked if connected to server, see update(float dt)
    m_game_setup = STKHost::get()->setupNewGame();
    STKHost::get()->getServerPeerForClient()->unsetClientServerToken();
    // stop race protocols
    auto pm = ProtocolManager::lock();
    assert(pm);
    pm->findAndTerminate(PROTOCOL_CONTROLLER_EVENTS);
    pm->findAndTerminate(PROTOCOL_KART_UPDATE);
    pm->findAndTerminate(PROTOCOL_GAME_EVENTS);
    m_state = NONE;
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
    data.decodeString(&track_name);
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
