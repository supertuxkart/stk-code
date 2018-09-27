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
#include "guiengine/screen_keyboard.hpp"
#include "input/device_manager.hpp"
#include "items/item_manager.hpp"
#include "karts/kart_properties_manager.hpp"
#include "modes/linear_world.hpp"
#include "network/crypto.hpp"
#include "network/event.hpp"
#include "network/game_setup.hpp"
#include "network/network_config.hpp"
#include "network/network_player_profile.hpp"
#include "network/network_timer_synchronizer.hpp"
#include "network/protocols/connect_to_server.hpp"
#include "network/protocols/game_protocol.hpp"
#include "network/protocols/game_events_protocol.hpp"
#include "network/race_event_manager.hpp"
#include "network/server.hpp"
#include "network/server_config.hpp"
#include "network/stk_host.hpp"
#include "network/stk_peer.hpp"
#include "states_screens/online/networking_lobby.hpp"
#include "states_screens/online/network_kart_selection.hpp"
#include "states_screens/race_result_gui.hpp"
#include "states_screens/state_manager.hpp"
#include "states_screens/online/tracks_screen.hpp"
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
"SELECTING_KARTS" -> "RACING" [label="Server sends start race message"]
}
\enddot
Note that some states are actually managed outside of the client lobby. For
example to select race details after selecting a kart is managed by the GUI
engine.

*/

ClientLobby::ClientLobby(const TransportAddress& a, std::shared_ptr<Server> s)
           : LobbyProtocol(NULL)
{
    m_waiting_for_game = false;
    m_server_auto_lap = false;
    m_received_server_result = false;
    m_state.store(NONE);
    m_server_address = a;
    m_server = s;
    setHandleDisconnections(true);
    m_disconnected_msg[PDI_TIMEOUT] = _("Server connection timed out.");
    m_disconnected_msg[PDI_NORMAL] = _("Server has been shut down.");
    m_disconnected_msg[PDI_KICK] = _("You were kicked from the server.");
    m_disconnected_msg[PDI_BAD_CONNECTION] =
        _("Bad network connection is detected.");
}   // ClientLobby

//-----------------------------------------------------------------------------
ClientLobby::~ClientLobby()
{
    clearPlayers();
    if (m_server->supportsEncryption())
    {
        Online::XMLRequest* request =
            new Online::XMLRequest(true/*manager_memory*/);
        NetworkConfig::get()->setServerDetails(request,
            "clear-user-joined-server");
        request->queue();
        ConnectToServer::m_previous_unjoin = request->observeExistence();
    }
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
        case LE_PLAYER_DISCONNECTED :  disconnectedPlayer(event);  break;
        case LE_CONNECTION_REFUSED:    connectionRefused(event);   break;
        case LE_VOTE:                  displayPlayerVote(event);   break;
        case LE_SERVER_OWNERSHIP:      becomingServerOwner();      break;
        case LE_BAD_TEAM:              handleBadTeam();            break;
        case LE_BAD_CONNECTION:        handleBadConnection();      break;
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
            case LE_START_RACE: startGame(event);                        break;
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
        STKHost::get()->setErrorMessage(
            m_disconnected_msg.at(event->getPeerDisconnectInfo()));
        STKHost::get()->requestShutdown();
        return true;
    } // disconnection
    return false;
}   // notifyEventAsynchronous

//-----------------------------------------------------------------------------
void ClientLobby::addAllPlayers(Event* event)
{
    // In case the user opened a user info dialog
    GUIEngine::ModalDialog::dismiss();
    GUIEngine::ScreenKeyboard::dismiss();

    if (!checkDataSize(event, 1))
    {
        // If recieved invalid message for players leave now
        STKHost::get()->disconnectAllPeers(false/*timeout_waiting*/);
        STKHost::get()->requestShutdown();
        return;
    }
    // Timeout is too slow to synchronize, force it to stop and set current
    // time
    if (!STKHost::get()->getNetworkTimerSynchronizer()->isSynchronised())
    {
        if (ServerConfig::m_voting_timeout >= 10.0f)
        {
            core::stringw msg = _("Bad network connection is detected.");
            MessageQueue::add(MessageQueue::MT_ERROR, msg);
            Log::warn("ClientLobby", "Failed to synchronize timer before game "
                "start, maybe you enter the game too quick? (at least 5 "
                "seconds are required for synchronization.");
        }
        STKHost::get()->getNetworkTimerSynchronizer()->enableForceSetTimer();
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
        KartTeam team = (KartTeam)data.getUInt8();
        auto player = std::make_shared<NetworkPlayerProfile>(peer, player_name,
            host_id, kart_color, online_id, ppd, local_id, team);
        std::string kart_name;
        data.decodeString(&kart_name);
        player->setKartName(kart_name);
        peer->addPlayer(player);
        m_game_setup->addPlayer(player);
        players.push_back(player);
    }
    uint32_t random_seed = data.getUInt32();
    ItemManager::updateRandomSeed(random_seed);
    if (race_manager->getMinorMode() == RaceManager::MINOR_MODE_BATTLE)
    {
        int hit_capture_limit = data.getUInt32();
        float time_limit = data.getFloat();
        m_game_setup->setHitCaptureTime(hit_capture_limit, time_limit);
    }
    configRemoteKart(players);
    loadWorld();
    // Switch to assign mode in case a player hasn't chosen any karts
    input_manager->getDeviceManager()->setAssignMode(ASSIGN);
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
        NetworkString* ns = getNetworkString();
        ns->addUInt8(LE_CONNECTION_REQUESTED)
            .addUInt32(ServerConfig::m_server_version)
            .encodeString(StringUtils::getUserAgentString());

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
        assert(!NetworkConfig::get()->isAddingNetworkPlayers());
        const uint8_t player_count =
            (uint8_t)NetworkConfig::get()->getNetworkPlayers().size();
        ns->addUInt8(player_count);

        bool encryption = false;
        uint32_t id = PlayerManager::getCurrentOnlineId();

        BareNetworkString* rest = new BareNetworkString();
        if (m_server->supportsEncryption() && id != 0)
        {
            ns->addUInt32(id);
            encryption = true;
        }
        else
        {
            ns->addUInt32(id).addUInt32(0);
            if (id != 0)
                ns->encodeString(PlayerManager::getCurrentOnlineUserName());
        }

        rest->encodeString(ServerConfig::m_private_server_password)
            .addUInt8(player_count);
        for (auto& p : NetworkConfig::get()->getNetworkPlayers())
        {
            core::stringw name;
            PlayerProfile* player = std::get<1>(p);
            rest->encodeString(player->getName()).
                addFloat(player->getDefaultKartColor());
            // Per-player handicap
            rest->addUInt8(std::get<2>(p));
        }

        finalizeConnectionRequest(ns, rest, encryption);
        m_state.store(REQUESTING_CONNECTION);
    }
    break;
    case RACE_FINISHED:
        if (!RaceEventManager::getInstance()->protocolStopped() ||
            !GameProtocol::emptyInstance())
            return;
        if (!m_received_server_result)
        {
            m_received_server_result = true;
            // In case someone opened paused race dialog or menu in network game
            GUIEngine::ModalDialog::dismiss();
            GUIEngine::ScreenKeyboard::dismiss();
            if (StateManager::get()->getGameState() == GUIEngine::INGAME_MENU)
                StateManager::get()->enterGameState();
            World::getWorld()->enterRaceOverState();
        }
        break;
    case DONE:
        m_state.store(EXITING);
        requestTerminate();
        break;
    case REQUESTING_CONNECTION:
    case CONNECTED:
        if (STKHost::get()->isAuthorisedToControl() &&
            NetworkConfig::get()->isAutoConnect())
        {
            // Send a message to the server to start
            NetworkString start(PROTOCOL_LOBBY_ROOM);
            start.addUInt8(LobbyProtocol::LE_REQUEST_BEGIN);
            STKHost::get()->sendToServer(&start, true);
        }
    case SELECTING_ASSETS:
    case RACING:
    case EXITING:
        break;
    }
}   // update

//-----------------------------------------------------------------------------
void ClientLobby::finalizeConnectionRequest(NetworkString* header,
                                            BareNetworkString* rest,
                                            bool encrypt)
{
    if (encrypt)
    {
        auto crypto = Crypto::getClientCrypto();
        Crypto::resetClientAES();
        BareNetworkString* result = new BareNetworkString();
        if (!crypto->encryptConnectionRequest(*rest))
        {
            // Failed
            result->addUInt32(0);
            *result += BareNetworkString(rest->getData(), rest->getTotalSize());
            encrypt = false;
        }
        else
        {
            Log::info("ClientLobby", "Server will validate this online player.");
            result->addUInt32(rest->getTotalSize());
            *result += BareNetworkString(rest->getData(), rest->getTotalSize());
        }
        delete rest;
        *header += *result;
        delete result;
        sendToServer(header);
        delete header;
        if (encrypt)
        {
            STKHost::get()->getServerPeerForClient()
                ->setCrypto(std::move(crypto));
        }
    }
    else
    {
        *header += *rest;
        delete rest;
        sendToServer(header);
        delete header;
    }
}   // finalizeConnectionRequest

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
    core::stringw vote_msg;
    if (race_manager->getMinorMode() == RaceManager::MINOR_MODE_BATTLE &&
        race_manager->getMajorMode() == RaceManager::MAJOR_MODE_FREE_FOR_ALL)
    {
        //I18N: Vote message in network game from a player
        vote_msg = _("Track: %s,\nrandom item location: %s",
            track_readable, rev == 1 ? yes : no);
    }
    else if (race_manager->getMinorMode() == RaceManager::MINOR_MODE_BATTLE &&
        race_manager->getMajorMode() ==
        RaceManager::MAJOR_MODE_CAPTURE_THE_FLAG)
    {
        //I18N: Vote message in network game from a player
        vote_msg = _("Track: %s", track_readable);
    }
    else if (race_manager->getMinorMode() == RaceManager::MINOR_MODE_SOCCER)
    {
        if (m_game_setup->isSoccerGoalTarget())
        {
            //I18N: Vote message in network game from a player
            vote_msg = _("Track: %s,\n"
                "number of goals to win: %d,\nrandom item location: %s",
                track_readable, lap, rev == 1 ? yes : no);
        }
        else
        {
            //I18N: Vote message in network game from a player
            vote_msg = _("Track: %s,\n"
                "maximum time: %d,\nrandom item location: %s",
                track_readable, lap, rev == 1 ? yes : no);
        }
    }
    else
    {
        //I18N: Vote message in network game from a player
        vote_msg = _("Track: %s,\nlaps: %d, reversed: %s",
            track_readable, lap, rev == 1 ? yes : no);
    }
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
    // At least 8 bytes should remain now
    if (!checkDataSize(event, 8)) return;

    NetworkString &data = event->data();
    // Accepted
    // ========
    Log::info("ClientLobby", "The server accepted the connection.");

    // I18N: Message shown in network lobby to tell user that
    // player name is clickable
    core::stringw msg = _("Press player name in the list for player management"
        " and ranking information.");
    MessageQueue::add(MessageQueue::MT_GENERIC, msg);

    STKHost::get()->setMyHostId(data.getUInt32());
    assert(!NetworkConfig::get()->isAddingNetworkPlayers());
    uint32_t server_version = data.getUInt32();
    NetworkConfig::get()->setJoinedServerVersion(server_version);
    assert(server_version != 0);
    m_state.store(CONNECTED);
    float auto_start_timer = data.getFloat();
    if (auto_start_timer != std::numeric_limits<float>::max())
        NetworkingLobby::getInstance()->setStartingTimerTo(auto_start_timer);
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

    unsigned max_player = data.getUInt8();
    //I18N: In the networking lobby
    each_line = _("Max players: %d", (int)max_player);
    NetworkingLobby::getInstance()->addMoreServerInfo(each_line);

    u_data = data.getUInt8();
    ServerConfig::m_server_mode = u_data;
    auto game_mode = ServerConfig::getLocalGameMode();
    race_manager->setMinorMode(game_mode.first);
    if (game_mode.first == RaceManager::MINOR_MODE_BATTLE)
        race_manager->setMajorMode(game_mode.second);
    else
    {
        // We use single mode in network even it's grand prix
        race_manager->setMajorMode(RaceManager::MAJOR_MODE_SINGLE);
    }

    //I18N: In the networking lobby
    core::stringw mode_name = ServerConfig::getModeName(u_data);
    each_line = _("Game mode: %s", mode_name);
    NetworkingLobby::getInstance()->addMoreServerInfo(each_line);

    uint8_t extra_server_info = data.getUInt8();
    bool grand_prix_started = false;
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
            grand_prix_started = cur_gp_track != 0;
            unsigned total_gp_track = data.getUInt8();
            m_game_setup->setGrandPrixTrack(total_gp_track);
            each_line = _("Grand prix progress: %d / %d", cur_gp_track,
                total_gp_track);
            NetworkingLobby::getInstance()->addMoreServerInfo(each_line);
            break;
        }
    }
    // Auto start info
    unsigned min_players = data.getUInt8();
    float start_timeout = data.getFloat();
    NetworkingLobby::getInstance()->initAutoStartTimer(grand_prix_started,
        min_players, start_timeout, max_player);

    // MOTD
    core::stringw motd;
    data.decodeString16(&motd);
    const std::vector<core::stringw>& motd_line = StringUtils::split(motd,
        '\n');
    if (!motd_line.empty())
    {
        for (const core::stringw& motd : motd_line)
            NetworkingLobby::getInstance()->addMoreServerInfo(motd);
    }
}   // handleServerInfo

//-----------------------------------------------------------------------------
void ClientLobby::updatePlayerList(Event* event)
{
    if (!checkDataSize(event, 1)) return;
    NetworkString& data = event->data();
    bool waiting = data.getUInt8() == 1;
    if (m_waiting_for_game && !waiting)
    {
        // The waiting game finished
        NetworkingLobby::getInstance()
            ->addMoreServerInfo(L"--------------------");
        SFXManager::get()->quickSound("wee");
    }

    m_waiting_for_game = waiting;
    unsigned player_count = data.getUInt8();
    std::vector<std::tuple<uint32_t, uint32_t, uint32_t, core::stringw,
        int, KartTeam> > players;
    for (unsigned i = 0; i < player_count; i++)
    {
        std::tuple<uint32_t, uint32_t, uint32_t, core::stringw, int,
            KartTeam> pl;
        std::get<0>(pl) = data.getUInt32();
        std::get<1>(pl) = data.getUInt32();
        std::get<2>(pl) = data.getUInt8();
        data.decodeStringW(&std::get<3>(pl));
        bool is_peer_waiting_for_game = data.getUInt8() == 1;
        bool is_peer_server_owner = data.getUInt8() == 1;
        // icon to be used, see NetworkingLobby::loadedFromFile
        std::get<4>(pl) = is_peer_server_owner ? 0 :
            std::get<1>(pl) != 0 /*if online account*/ ? 1 : 2;
        if (waiting && !is_peer_waiting_for_game)
            std::get<4>(pl) = 3;
        PerPlayerDifficulty d = (PerPlayerDifficulty)data.getUInt8();
        if (d == PLAYER_DIFFICULTY_HANDICAP)
            std::get<3>(pl) = _("%s (handicapped)", std::get<3>(pl));
        std::get<5>(pl) = (KartTeam)data.getUInt8();
        players.push_back(pl);
    }
    NetworkingLobby::getInstance()->updatePlayers(players);
}   // updatePlayerList

//-----------------------------------------------------------------------------
void ClientLobby::handleBadTeam()
{
    SFXManager::get()->quickSound("anvil");
    //I18N: Display when all players are in red or blue team, which the race
    //will not be allowed to start
    core::stringw msg = _("All players joined red or blue team.");
    MessageQueue::add(MessageQueue::MT_ERROR, msg);
}   // handleBadTeam

//-----------------------------------------------------------------------------
void ClientLobby::handleBadConnection()
{
    SFXManager::get()->quickSound("anvil");
    core::stringw msg = _("Bad network connection is detected.");
    MessageQueue::add(MessageQueue::MT_ERROR, msg);
}   // handleBadConnection

//-----------------------------------------------------------------------------
void ClientLobby::becomingServerOwner()
{
    STKHost::get()->setAuthorisedToControl(true);
    if (STKHost::get()->isClientServer())
        return;

    SFXManager::get()->quickSound("wee");
    //I18N: Display when a player is allow to control the server
    core::stringw msg = _("You are now the owner of server.");
    MessageQueue::add(MessageQueue::MT_GENERIC, msg);
}   // becomingServerOwner

//-----------------------------------------------------------------------------
void ClientLobby::handleChat(Event* event)
{
    if (!UserConfigParams::m_lobby_chat)
        return;
    SFXManager::get()->quickSound("plopp");
    core::stringw message;
    event->data().decodeString16(&message);
    Log::info("ClientLobby", "%s", StringUtils::wideToUtf8(message).c_str());
    if (message.size() > 0)
    {
        NetworkingLobby::getInstance()->addMoreServerInfo(message);
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
    case RR_INVALID_PLAYER:
        STKHost::get()->setErrorMessage(
            _("Connection refused: Invalid player connecting."));
        break;
    }
    STKHost::get()->disconnectAllPeers(false/*timeout_waiting*/);
    STKHost::get()->requestShutdown();
}   // connectionRefused

//-----------------------------------------------------------------------------

/*! \brief Called when the server broadcasts to start the race to all clients.
 *  \param event : Event providing the time the client should start game.
 */
void ClientLobby::startGame(Event* event)
{
    uint64_t start_time = event->data().getUInt64();
    joinStartGameThread();
    m_start_game_thread = std::thread([start_time, this]()
        {
            const uint64_t cur_time = STKHost::get()->getNetworkTimer();
            if (!(start_time > cur_time))
            {
                Log::warn("ClientLobby", "Network timer is too slow to catch "
                    "up, you must have a poor network.");
                STKHost::get()->setErrorMessage(
                    m_disconnected_msg.at(PDI_BAD_CONNECTION));
                STKHost::get()->requestShutdown();
                return;
            }
            int sleep_time = (int)(start_time - cur_time);
            Log::info("ClientLobby", "Start game after %dms", sleep_time);
            std::this_thread::sleep_for(std::chrono::milliseconds(sleep_time));
            Log::info("ClientLobby", "Started at %lf", StkTime::getRealTime());
            m_state.store(RACING);
        });
}   // startGame

//-----------------------------------------------------------------------------
/*! \brief Called when the kart selection starts.
 *  \param event : Event providing the information (no additional information
 *                 in this case).
 */
void ClientLobby::startSelection(Event* event)
{
    SFXManager::get()->quickSound("wee");
    const NetworkString& data = event->data();
    bool skip_kart_screen = data.getUInt8() == 1;
    m_server_auto_lap = data.getUInt8() == 1;
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
    GUIEngine::ScreenKeyboard::dismiss();
    NetworkKartSelectionScreen* screen =
        NetworkKartSelectionScreen::getInstance();
    screen->setAvailableKartsFromServer(m_available_karts);
    // In case of auto-connect or continue a grand prix, use random karts
    // (or previous kart) from server and go to track selection
    if (NetworkConfig::get()->isAutoConnect() || skip_kart_screen)
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
    m_state.store(RACE_FINISHED);
}   // raceFinished

//-----------------------------------------------------------------------------
/** Called when the server informs the clients to exit the race result screen.
 *  It exits the race, and goes back to the lobby.
 */
void ClientLobby::exitResultScreen(Event *event)
{
    // In case the user opened a user info dialog
    GUIEngine::ModalDialog::dismiss();
    GUIEngine::ScreenKeyboard::dismiss();
    
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
