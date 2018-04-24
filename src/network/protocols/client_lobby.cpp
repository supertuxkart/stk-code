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

#include "audio/sfx_manager.hpp"
#include "config/user_config.hpp"
#include "config/player_manager.hpp"
#include "guiengine/modaldialog.hpp"
#include "guiengine/message_queue.hpp"
#include "input/device_manager.hpp"
#include "karts/kart_properties_manager.hpp"
#include "modes/linear_world.hpp"
#include "network/event.hpp"
#include "network/game_setup.hpp"
#include "network/network_config.hpp"
#include "network/network_player_profile.hpp"
#include "network/protocols/game_protocol.hpp"
#include "network/protocols/game_events_protocol.hpp"
#include "network/race_event_manager.hpp"
#include "network/stk_host.hpp"
#include "network/stk_peer.hpp"
#include "online/online_player_profile.hpp"
#include "online/online_profile.hpp"
#include "states_screens/networking_lobby.hpp"
#include "states_screens/network_kart_selection.hpp"
#include "states_screens/race_result_gui.hpp"
#include "states_screens/state_manager.hpp"
#include "states_screens/tracks_screen.hpp"
#include "tracks/track.hpp"
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
    clearPlayers();
}   // ClientLobby

//-----------------------------------------------------------------------------
void ClientLobby::clearPlayers()
{
    StateManager::get()->resetActivePlayers();
    if (input_manager)
    {
        input_manager->getDeviceManager()->setAssignMode(NO_ASSIGN);
        input_manager->getDeviceManager()->setSinglePlayer(NULL);
        input_manager->setMasterPlayerOnly(false);
        input_manager->getDeviceManager()->clearLatestUsedDevice();
    }
}   // clearPlayers

//-----------------------------------------------------------------------------
/** Sets the address of the server. 
 */
void ClientLobby::setAddress(const TransportAddress &address)
{
    m_server_address = address;
}   // setAddress

//-----------------------------------------------------------------------------
void ClientLobby::setup()
{
    clearPlayers();
    m_received_server_result = false;
    TracksScreen::getInstance()->resetVote();
    LobbyProtocol::setup();
    m_state.store(NONE);
}   // setup

//-----------------------------------------------------------------------------
/** Called from the gui when a client clicked on 'continue' on the race result
 *  screen. It notifies the server that this client has exited the screen and
 *  is back at the lobby.
 */
void ClientLobby::doneWithResults()
{
    NetworkString* done = getNetworkString(1);
    done->setSynchronous(true);
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
        case LE_LOAD_WORLD:            addAllPlayers(event);       break;
        case LE_RACE_FINISHED:         raceFinished(event);        break;
        case LE_EXIT_RESULT:           exitResultScreen(event);    break;
        case LE_UPDATE_PLAYER_LIST:    updatePlayerList(event);    break;
        case LE_CHAT:                  handleChat(event);          break;
        case LE_CONNECTION_ACCEPTED:   connectionAccepted(event);  break;
        case LE_SERVER_INFO:           handleServerInfo(event);    break;
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
            case LE_VOTE: displayPlayerVote(event);                      break;
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
void ClientLobby::addAllPlayers(Event* event)
{
    if (!checkDataSize(event, 1))
    {
        // If recieved invalid message for players leave now
        STKHost::get()->disconnectAllPeers(false/*timeout_waiting*/);
        STKHost::get()->requestShutdown();
        return;
    }
    NetworkString& data = event->data();
    std::string track_name;
    data.decodeString(&track_name);
    uint8_t lap = data.getUInt8();
    uint8_t reverse = data.getUInt8();
    m_game_setup->setRace(track_name, lap, reverse == 1);

    std::shared_ptr<STKPeer> peer = event->getPeerSP();
    peer->cleanPlayerProfiles();
    m_game_setup->update(true/*remove_disconnected_players*/);
    std::vector<std::shared_ptr<NetworkPlayerProfile> > players;
    unsigned player_count = data.getUInt8();

    for (unsigned i = 0; i < player_count; i++)
    {
        core::stringw player_name;
        data.decodeStringW(&player_name);
        uint32_t host_id = data.getUInt32();
        float kart_color = data.getFloat();
        uint32_t online_id = data.getUInt32();
        PerPlayerDifficulty ppd = (PerPlayerDifficulty)data.getUInt8();
        uint8_t local_id = data.getUInt8();
        auto player = std::make_shared<NetworkPlayerProfile>(peer, player_name,
            host_id, kart_color, online_id, ppd, local_id);
        std::string kart_name;
        data.decodeString(&kart_name);
        player->setKartName(kart_name);
        peer->addPlayer(player);
        m_game_setup->addPlayer(player);
        players.push_back(player);
    }
    configRemoteKart(players);
    loadWorld();
}   // addAllPlayers

//-----------------------------------------------------------------------------
void ClientLobby::update(int ticks)
{
    switch (m_state.load())
    {
    case NONE:
        if (STKHost::get()->isConnectedTo(m_server_address))
        {
            m_state.store(LINKED);
        }
        break;
    case LINKED:
    {
        NetworkString *ns = getNetworkString();
        ns->addUInt8(LE_CONNECTION_REQUESTED)
            .addUInt8(NetworkConfig::m_server_version)
            .encodeString(NetworkConfig::get()->getPassword());

        assert(!NetworkConfig::get()->isAddingNetworkPlayers());
        ns->addUInt8(
            (uint8_t)NetworkConfig::get()->getNetworkPlayers().size());
        // Only first player has online name and profile
        bool first_player = true;
        for (auto& p : NetworkConfig::get()->getNetworkPlayers())
        {
            core::stringw name;
            PlayerProfile* player = std::get<1>(p);
            if (PlayerManager::getCurrentOnlineState() ==
                PlayerProfile::OS_SIGNED_IN && first_player)
            {
                name = PlayerManager::getCurrentOnlineUserName();
            }
            else
            {
                name = player->getName();
            }
            std::string name_u8 = StringUtils::wideToUtf8(name);
            ns->encodeString(name_u8).addFloat(player->getDefaultKartColor());
            Online::OnlinePlayerProfile* opp =
                dynamic_cast<Online::OnlinePlayerProfile*>(player);
            ns->addUInt32(first_player && opp && opp->getProfile() ?
                opp->getProfile()->getID() : 0);
            // Per-player handicap
            ns->addUInt8(std::get<2>(p));
            first_player = false;
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
        m_state.store(REQUESTING_CONNECTION);
    }
    break;
    case REQUESTING_CONNECTION:
        break;
    case CONNECTED:
        break;
    case SELECTING_ASSETS:
        break;
    case PLAYING:
        break;
    case RACE_FINISHED:
        break;
    case DONE:
        m_state.store(EXITING);
        requestTerminate();
        break;
    case EXITING:
        break;
    }
}   // update

//-----------------------------------------------------------------------------
void ClientLobby::displayPlayerVote(Event* event)
{
    if (!checkDataSize(event, 4)) return;
    // Get the player name who voted
    NetworkString& data = event->data();
    float timeout = data.getFloat();
    TracksScreen::getInstance()->setVoteTimeout(timeout);
    std::string player_name;
    data.decodeString(&player_name);
    uint32_t host_id = data.getUInt32();
    player_name += ": ";
    std::string track_name;
    data.decodeString(&track_name);
    Track* track = track_manager->getTrack(track_name);
    if (!track)
        Log::fatal("ClientLobby", "Missing track %s", track_name.c_str());
    core::stringw track_readable = track->getName();
    int lap = data.getUInt8();
    int rev = data.getUInt8();
    core::stringw yes = _("Yes");
    core::stringw no = _("No");
    //I18N: Vote message in network game from a player
    core::stringw vote_msg = _("Track: %s,\nlaps: %d, reversed: %s",
        track_readable, lap, rev == 1 ? yes : no);
    vote_msg = StringUtils::utf8ToWide(player_name) + vote_msg;
    TracksScreen::getInstance()->addVoteMessage(player_name +
        StringUtils::toString(host_id), vote_msg);
}   // displayPlayerVote

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
    SFXManager::get()->quickSound("appear");
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
    // At least 4 bytes should remain now
    if (!checkDataSize(event, 4)) return;

    NetworkString &data = event->data();
    STKPeer* peer = event->getPeer();

    // Accepted
    // ========
    Log::info("ClientLobby", "The server accepted the connection.");

    STKHost::get()->setMyHostId(data.getUInt32());
    assert(!NetworkConfig::get()->isAddingNetworkPlayers());
    // connection token
    uint32_t token = data.getToken();
    peer->setClientServerToken(token);
    m_state.store(CONNECTED);

}   // connectionAccepted

//-----------------------------------------------------------------------------
void ClientLobby::handleServerInfo(Event* event)
{
    // At least 6 bytes should remain now
    if (!checkDataSize(event, 6)) return;

    NetworkString &data = event->data();
    // Add server info
    core::stringw str, each_line;
    uint8_t u_data;
    data.decodeStringW(&str);

    //I18N: In the networking lobby
    each_line = _("Server name: %s", str);
    NetworkingLobby::getInstance()->addMoreServerInfo(each_line);

    u_data = data.getUInt8();
    const core::stringw& difficulty_name =
        race_manager->getDifficultyName((RaceManager::Difficulty)u_data);
    race_manager->setDifficulty((RaceManager::Difficulty)u_data);
    //I18N: In the networking lobby
    each_line = _("Difficulty: %s", difficulty_name);
    NetworkingLobby::getInstance()->addMoreServerInfo(each_line);

    u_data = data.getUInt8();
    //I18N: In the networking lobby
    each_line = _("Max players: %d", (int)u_data);
    NetworkingLobby::getInstance()->addMoreServerInfo(each_line);

    u_data = data.getUInt8();
    NetworkConfig::get()->setServerMode(u_data);
    auto game_mode = NetworkConfig::get()->getLocalGameMode();
    race_manager->setMinorMode(game_mode.first);
    // We use single mode in network even it's grand prix
    race_manager->setMajorMode(RaceManager::MAJOR_MODE_SINGLE);

    //I18N: In the networking lobby
    core::stringw mode_name = NetworkConfig::get()->getModeName(u_data);
    each_line = _("Game mode: %s", mode_name);
    NetworkingLobby::getInstance()->addMoreServerInfo(each_line);

    uint8_t extra_server_info = data.getUInt8();
    switch (extra_server_info)
    {
        case 0:
            break;
        case 1:
        {
            u_data = data.getUInt8();
            core::stringw tl = _("Time limit");
            core::stringw gl = _("Goals limit");
            core::stringw sgt = u_data == 0 ? tl : gl;
            m_game_setup->setSoccerGoalTarget(u_data != 0);
            //I18N: In the networking lobby
            each_line = _("Soccer game type: %s", sgt);
            NetworkingLobby::getInstance()->addMoreServerInfo(each_line);
            break;
        }
        case 2:
        {
            unsigned cur_gp_track = data.getUInt8();
            unsigned total_gp_track = data.getUInt8();
            m_game_setup->setGrandPrixTrack(total_gp_track);
            each_line = _("Grand prix progress: %d / %d", cur_gp_track,
                total_gp_track);
            NetworkingLobby::getInstance()->addMoreServerInfo(each_line);
            break;
        }
    }

    // MOTD
    data.decodeStringW(&str);
    if (!str.empty())
        NetworkingLobby::getInstance()->addMoreServerInfo(str);

}   // handleServerInfo

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
    SFXManager::get()->quickSound("wee");
    core::stringw msg = _("You are now the owner of server.");
    MessageQueue::add(MessageQueue::MT_GENERIC, msg);
    STKHost::get()->setAuthorisedToControl(true);
    if (m_state.load() == CONNECTED && NetworkConfig::get()->isAutoConnect())
    {
        // Send a message to the server to start
        NetworkString start(PROTOCOL_LOBBY_ROOM);
        start.setSynchronous(true);
        start.addUInt8(LobbyProtocol::LE_REQUEST_BEGIN);
        STKHost::get()->sendToServer(&start, true);
    }
}   // becomingServerOwner

//-----------------------------------------------------------------------------
void ClientLobby::handleChat(Event* event)
{
    if (!UserConfigParams::m_lobby_chat)
        return;
    SFXManager::get()->quickSound("plopp");
    std::string message;
    event->data().decodeString(&message);
    Log::info("ClientLobby", "%s", message.c_str());
    if (message.size() > 0)
    {
        NetworkingLobby::getInstance()->addMoreServerInfo(
            StringUtils::utf8ToWide(message));
    }
}   // handleChat

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

/*! \brief Called when the server broadcasts to start the race to all clients.
 *  \param event : Event providing the information (no additional informati
 *                 in this case).
 */
void ClientLobby::startGame(Event* event)
{
    m_state.store(PLAYING);
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
    NetworkString* ns = getNetworkString(2);
    ns->addUInt8(LE_STARTED_RACE);
    sendToServer(ns, /*reliable*/true);
    delete ns;
    Log::verbose("ClientLobby", "StartingRaceNow at %lf",
                 StkTime::getRealTime());
}   // startingRaceNow

//-----------------------------------------------------------------------------
/*! \brief Called when the kart selection starts.
 *  \param event : Event providing the information (no additional information
 *                 in this case).
 */
void ClientLobby::startSelection(Event* event)
{
    const NetworkString& data = event->data();
    uint8_t skip_kart_screen = data.getUInt8();
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

    // In case the user opened a user info dialog
    GUIEngine::ModalDialog::dismiss();
    NetworkKartSelectionScreen* screen =
        NetworkKartSelectionScreen::getInstance();
    screen->setAvailableKartsFromServer(m_available_karts);
    // In case of auto-connect or continue a grand prix, use random karts
    // (or previous kart) from server and go to track selection
    if (NetworkConfig::get()->isAutoConnect() || skip_kart_screen == 1)
    {
        input_manager->setMasterPlayerOnly(true);
        for (auto& p : NetworkConfig::get()->getNetworkPlayers())
        {
            StateManager::get()
                ->createActivePlayer(std::get<1>(p), std::get<0>(p));
        }
        input_manager->getDeviceManager()->setAssignMode(ASSIGN);
        TracksScreen::getInstance()->setNetworkTracks();
        TracksScreen::getInstance()->push();
    }
    else
    {
        screen->push();
    }

    m_state.store(SELECTING_ASSETS);
    Log::info("ClientLobby", "Selection starts now");
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
    NetworkString &data = event->data();
    Log::info("ClientLobby", "Server notified that the race is finished.");
    if (m_game_setup->isGrandPrix())
    {
        int t = data.getUInt32();
        static_cast<LinearWorld*>(World::getWorld())->setFastestLapTicks(t);
        race_manager->configGrandPrixResultFromNetwork(data);
    }
    else if (race_manager->modeHasLaps())
    {
        int t = data.getUInt32();
        static_cast<LinearWorld*>(World::getWorld())->setFastestLapTicks(t);
    }

    // stop race protocols
    RaceEventManager::getInstance()->stop();

    RaceEventManager::getInstance()->getProtocol()->requestTerminate();
    GameProtocol::lock()->requestTerminate();

    while (!RaceEventManager::getInstance()->protocolStopped())
        StkTime::sleep(1);
    while (!GameProtocol::emptyInstance())
        StkTime::sleep(1);

    m_received_server_result = true;
}   // raceFinished

//-----------------------------------------------------------------------------
/** Called when the server informs the clients to exit the race result screen.
 *  It exits the race, and goes back to the lobby.
 */
void ClientLobby::exitResultScreen(Event *event)
{
    setup();
    m_state.store(CONNECTED);
    RaceResultGUI::getInstance()->backToLobby();
}   // exitResultScreen

//-----------------------------------------------------------------------------
/** Callback when the world is loaded. The client will inform the server
 *  that the players on this host are ready to start the race. It is called by
 *  the RaceManager after the world is loaded.
 */
void ClientLobby::finishedLoadingWorld()
{
    NetworkString* ns = getNetworkString(1);
    ns->addUInt8(LE_CLIENT_LOADED_WORLD);
    sendToServer(ns, true);
    delete ns;
}   // finishedLoadingWorld
