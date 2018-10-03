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

#include "network/protocols/server_lobby.hpp"

#include "config/user_config.hpp"
#include "items/item_manager.hpp"
#include "karts/abstract_kart.hpp"
#include "karts/controller/player_controller.hpp"
#include "karts/kart_properties_manager.hpp"
#include "modes/linear_world.hpp"
#include "network/crypto.hpp"
#include "network/event.hpp"
#include "network/game_setup.hpp"
#include "network/network_config.hpp"
#include "network/network_player_profile.hpp"
#include "network/protocol_manager.hpp"
#include "network/protocols/connect_to_peer.hpp"
#include "network/protocols/game_protocol.hpp"
#include "network/protocols/game_events_protocol.hpp"
#include "network/race_event_manager.hpp"
#include "network/server_config.hpp"
#include "network/stk_host.hpp"
#include "network/stk_peer.hpp"
#include "online/online_profile.hpp"
#include "online/request_manager.hpp"
#include "race/race_manager.hpp"
#include "states_screens/online/networking_lobby.hpp"
#include "states_screens/race_result_gui.hpp"
#include "tracks/track.hpp"
#include "tracks/track_manager.hpp"
#include "utils/log.hpp"
#include "utils/random_generator.hpp"
#include "utils/time.hpp"

#include <algorithm>
#include <iterator>
#include <fstream>

/** This is the central game setup protocol running in the server. It is
 *  mostly a finite state machine. Note that all nodes in ellipses and light
 *  grey background are actual states; nodes in boxes and white background 
 *  are functions triggered from a state or triggering potentially a state
 *  change.
 \dot
 digraph interaction {
 node [shape=box]; "Server Constructor"; "playerTrackVote"; "connectionRequested"; 
                   "signalRaceStartToClients"; "startedRaceOnClient"; "loadWorld";
 node [shape=ellipse,style=filled,color=lightgrey];

 "Server Constructor" -> "INIT_WAN" [label="If WAN game"]
 "Server Constructor" -> "WAITING_FOR_START_GAME" [label="If LAN game"]
 "INIT_WAN" -> "GETTING_PUBLIC_ADDRESS" [label="GetPublicAddress protocol callback"]
 "GETTING_PUBLIC_ADDRESS" -> "WAITING_FOR_START_GAME" [label="Register server"]
 "WAITING_FOR_START_GAME" -> "connectionRequested" [label="Client connection request"]
 "connectionRequested" -> "WAITING_FOR_START_GAME"
 "WAITING_FOR_START_GAME" -> "SELECTING" [label="Start race from authorised client"]
 "SELECTING" -> "SELECTING" [label="Client selects kart, #laps, ..."]
 "SELECTING" -> "playerTrackVote" [label="Client selected track"]
 "playerTrackVote" -> "SELECTING" [label="Not all clients have selected"]
 "playerTrackVote" -> "LOAD_WORLD" [label="All clients have selected; signal load_world to clients"]
 "LOAD_WORLD" -> "loadWorld"
 "loadWorld" -> "WAIT_FOR_WORLD_LOADED" 
 "WAIT_FOR_WORLD_LOADED" -> "WAIT_FOR_WORLD_LOADED" [label="Client or server loaded world"]
 "WAIT_FOR_WORLD_LOADED" -> "signalRaceStartToClients" [label="All clients and server ready"]
 "signalRaceStartToClients" -> "WAIT_FOR_RACE_STARTED"
 "WAIT_FOR_RACE_STARTED" ->  "startedRaceOnClient" [label="Client has started race"]
 "startedRaceOnClient" -> "WAIT_FOR_RACE_STARTED" [label="Not all clients have started"]
 "startedRaceOnClient" -> "DELAY_SERVER" [label="All clients have started"]
 "DELAY_SERVER" -> "DELAY_SERVER" [label="Not done waiting"]
 "DELAY_SERVER" -> "RACING" [label="Server starts race now"]
 }
 \enddot


 *  It starts with detecting the public ip address and port of this
 *  host (GetPublicAddress).
 */
ServerLobby::ServerLobby() : LobbyProtocol(NULL)
{
    m_last_success_poll_time.store(StkTime::getRealTimeMs() + 30000);
    m_waiting_players_counts.store(0);
    m_server_owner_id.store(-1);
    m_registered_for_once_only = false;
    m_has_created_server_id_file = false;
    setHandleDisconnections(true);
    m_state = SET_PUBLIC_ADDRESS;
    m_save_server_config = true;
    updateBanList();
    if (ServerConfig::m_ranked)
    {
        Log::info("ServerLobby", "This server will submit ranking scores to "
            "STK addons server, don't bother host one if you don't have the "
            "corresponding permission, they will be rejected if so.");
    }
    m_result_ns = getNetworkString();
    m_result_ns->setSynchronous(true);
    m_waiting_for_reset = false;
    m_server_id_online.store(0);
}   // ServerLobby

//-----------------------------------------------------------------------------
/** Destructor.
 */
ServerLobby::~ServerLobby()
{
    if (NetworkConfig::get()->isNetworking() &&
        NetworkConfig::get()->isWAN())
    {
        unregisterServer(true/*now*/);
    }
    delete m_result_ns;
    if (m_save_server_config)
        ServerConfig::writeServerConfigToDisk();
}   // ~ServerLobby

//-----------------------------------------------------------------------------
void ServerLobby::setup()
{
    LobbyProtocol::setup();
    auto players = m_game_setup->getConnectedPlayers();
    if (m_game_setup->isGrandPrix() && !m_game_setup->isGrandPrixStarted())
    {
        for (auto player : players)
            player->resetGrandPrixData();
    }
    if (!m_game_setup->isGrandPrix() || !m_game_setup->isGrandPrixStarted())
    {
        for (auto player : players)
            player->setKartName("");
    }

    StateManager::get()->resetActivePlayers();
    // We use maximum 16bit unsigned limit
    auto all_k = kart_properties_manager->getAllAvailableKarts();
    auto all_t = track_manager->getAllTrackIdentifiers();
    if (all_k.size() >= 65536)
        all_k.resize(65535);
    if (all_t.size() >= 65536)
        all_t.resize(65535);
    m_available_kts.first = { all_k.begin(), all_k.end() };
    m_available_kts.second = { all_t.begin(), all_t.end() };
    RaceManager::MinorRaceModeType m = ServerConfig::getLocalGameMode().first;
    switch (m)
    {
        case RaceManager::MINOR_MODE_NORMAL_RACE:
        case RaceManager::MINOR_MODE_TIME_TRIAL:
        case RaceManager::MINOR_MODE_FOLLOW_LEADER:
        {
            auto it = m_available_kts.second.begin();
            while (it != m_available_kts.second.end())
            {
                Track* t =  track_manager->getTrack(*it);
                if (t->isArena() || t->isSoccer() || t->isInternal())
                {
                    it = m_available_kts.second.erase(it);
                }
                else
                    it++;
            }
            break;
        }
        case RaceManager::MINOR_MODE_BATTLE:
        {
            auto it = m_available_kts.second.begin();
            while (it != m_available_kts.second.end())
            {
                Track* t =  track_manager->getTrack(*it);
                if (race_manager->getMajorMode() ==
                    RaceManager::MAJOR_MODE_CAPTURE_THE_FLAG)
                {
                    if (!t->isCTF() || t->isInternal())
                    {
                        it = m_available_kts.second.erase(it);
                    }
                    else
                        it++;
                }
                else
                {
                    if (!t->isArena() ||  t->isInternal())
                    {
                        it = m_available_kts.second.erase(it);
                    }
                    else
                        it++;
                }
            }
            break;
        }
        case RaceManager::MINOR_MODE_SOCCER:
        {
            auto it = m_available_kts.second.begin();
            while (it != m_available_kts.second.end())
            {
                Track* t =  track_manager->getTrack(*it);
                if (!t->isSoccer() || t->isInternal())
                {
                    it = m_available_kts.second.erase(it);
                }
                else
                    it++;
            }
            break;
        }
        default:
            assert(false);
            break;
    }

    m_server_has_loaded_world.store(false);

    // Initialise the data structures to detect if all clients and 
    // the server are ready:
    resetPeersReady();
    m_peers_votes.clear();
    m_timeout.store(std::numeric_limits<int64_t>::max());
    m_waiting_for_reset = false;
    m_server_started_at = m_server_delay = 0;
    Log::info("ServerLobby", "Reset server to initial state.");
}   // setup

//-----------------------------------------------------------------------------
bool ServerLobby::notifyEvent(Event* event)
{
    assert(m_game_setup); // assert that the setup exists
    if (event->getType() != EVENT_TYPE_MESSAGE)
        return false;

    NetworkString &data = event->data();
    assert(data.size()); // message not empty
    uint8_t message_type;
    message_type = data.getUInt8();
    Log::info("ServerLobby", "Synchronous message received with type %d.",
              message_type);
    switch (message_type)
    {
    case LE_RACE_FINISHED_ACK: playerFinishedResult(event);   break;
    default: Log::error("ServerLobby", "Unknown message type %d - ignored.",
                        message_type);
             break;
    }   // switch message_type
    return true;
}   // notifyEvent

//-----------------------------------------------------------------------------
void ServerLobby::handleChat(Event* event)
{
    if (!checkDataSize(event, 1)) return;

    core::stringw message;
    event->data().decodeString16(&message);
    if (message.size() > 0)
    {
        NetworkString* chat = getNetworkString();
        chat->setSynchronous(true);
        chat->addUInt8(LE_CHAT).encodeString16(message);
        // After game is started, only send to waiting player
        const bool game_started = m_state.load() != WAITING_FOR_START_GAME;
        STKHost::get()->sendPacketToAllPeersWith([game_started]
            (STKPeer* p)
            {
                if (!p->isValidated())
                    return false;
                if (!game_started)
                    return true;
                if (!p->isWaitingForGame() && game_started)
                    return false;
                return true;
            }, chat);
        delete chat;
    }
}   // handleChat

//-----------------------------------------------------------------------------
void ServerLobby::changeTeam(Event* event)
{
    if (!ServerConfig::m_team_choosing ||
        !race_manager->teamEnabled())
        return;
    if (!checkDataSize(event, 1)) return;
    NetworkString& data = event->data();
    uint8_t local_id = data.getUInt8();
    auto& player = event->getPeer()->getPlayerProfiles().at(local_id);
    if (player->getTeam() == KART_TEAM_BLUE)
        player->setTeam(KART_TEAM_RED);
    else
        player->setTeam(KART_TEAM_BLUE);
    updatePlayerList();
}   // changeTeam

//-----------------------------------------------------------------------------
void ServerLobby::kickHost(Event* event)
{
    if (m_server_owner.lock() != event->getPeerSP())
        return;
    if (!checkDataSize(event, 4)) return;
    NetworkString& data = event->data();
    uint32_t host_id = data.getUInt32();
    std::shared_ptr<STKPeer> peer = STKHost::get()->findPeerByHostId(host_id);
    if (peer)
        peer->kick();
}   // kickHost

//-----------------------------------------------------------------------------
bool ServerLobby::notifyEventAsynchronous(Event* event)
{
    assert(m_game_setup); // assert that the setup exists
    if (event->getType() == EVENT_TYPE_MESSAGE)
    {
        NetworkString &data = event->data();
        assert(data.size()); // message not empty
        uint8_t message_type;
        message_type = data.getUInt8();
        Log::info("ServerLobby", "Message received with type %d.",
                  message_type);
        switch(message_type)
        {
        case LE_CONNECTION_REQUESTED: connectionRequested(event); break;
        case LE_KART_SELECTION: kartSelectionRequested(event);    break;
        case LE_CLIENT_LOADED_WORLD: finishedLoadingWorldClient(event); break;
        case LE_VOTE: playerVote(event);                          break;
        case LE_KICK_HOST: kickHost(event);                       break;
        case LE_CHANGE_TEAM: changeTeam(event);                   break;
        case LE_REQUEST_BEGIN: startSelection(event);             break;
        case LE_CHAT: handleChat(event);                          break;
        default:                                                  break;
        }   // switch
    } // if (event->getType() == EVENT_TYPE_MESSAGE)
    else if (event->getType() == EVENT_TYPE_DISCONNECTED)
    {
        clientDisconnected(event);
    } // if (event->getType() == EVENT_TYPE_DISCONNECTED)
    return true;
}   // notifyEventAsynchronous

//-----------------------------------------------------------------------------
/** Create the server id file to let the graphics server client connect. */
void ServerLobby::createServerIdFile()
{
    std::string sid = NetworkConfig::get()->getServerIdFile();
    if (!sid.empty() && !m_has_created_server_id_file)
    {
        std::fstream fs;
        sid += StringUtils::toString(m_server_id_online.load()) + "_" +
            StringUtils::toString(STKHost::get()->getPrivatePort());
        fs.open(sid, std::ios::out);
        fs.close();
        m_has_created_server_id_file = true;
    }
}   // createServerIdFile

//-----------------------------------------------------------------------------
/** Find out the public IP server or poll STK server asynchronously. */
void ServerLobby::asynchronousUpdate()
{
    // Check if server owner has left
    updateServerOwner();

    if (ServerConfig::m_ranked && m_state.load() == WAITING_FOR_START_GAME)
        clearDisconnectedRankedPlayer();

    if (allowJoinedPlayersWaiting() || (m_game_setup->isGrandPrix() &&
        m_state.load() == WAITING_FOR_START_GAME))
    {
        updateWaitingPlayers();
        // Only poll the STK server if this is a WAN server.
        if (NetworkConfig::get()->isWAN())
            checkIncomingConnectionRequests();
        handlePendingConnection();
    }

    if (NetworkConfig::get()->isWAN() &&
        allowJoinedPlayersWaiting() && m_server_recovering.expired() &&
        StkTime::getRealTimeMs() > m_last_success_poll_time.load() + 30000)
    {
        Log::warn("ServerLobby", "Trying auto server recovery.");
        registerServer(false/*now*/);
    }

    switch (m_state.load())
    {
    case SET_PUBLIC_ADDRESS:
    {
        // In case of LAN we don't need our public address or register with the
        // STK server, so we can directly go to the accepting clients state.
        if (NetworkConfig::get()->isLAN())
        {
            m_state = WAITING_FOR_START_GAME;
            STKHost::get()->startListening();
            createServerIdFile();
            return;
        }
        STKHost::get()->setPublicAddress();
        if (STKHost::get()->getPublicAddress().isUnset())
        {
            m_state = ERROR_LEAVE;
        }
        else
        {
            m_server_address = STKHost::get()->getPublicAddress();
            STKHost::get()->startListening();
            m_state = REGISTER_SELF_ADDRESS;
        }
        break;
    }
    case REGISTER_SELF_ADDRESS:
    {
        if (m_game_setup->isGrandPrixStarted() || m_registered_for_once_only)
        {
            m_state = WAITING_FOR_START_GAME;
            break;
        }
        // Register this server with the STK server. This will block
        // this thread, because there is no need for the protocol manager
        // to react to any requests before the server is registered.
        if (registerServer(true/*now*/))
        {
            if (allowJoinedPlayersWaiting())
                m_registered_for_once_only = true;
            m_state = WAITING_FOR_START_GAME;
            createServerIdFile();
        }
        else
        {
            m_state = ERROR_LEAVE;
        }
        break;
    }
    case WAITING_FOR_START_GAME:
    {
        if (ServerConfig::m_owner_less)
        {
            int player_size = m_game_setup->getPlayerCount();
            if ((player_size >= ServerConfig::m_min_start_game_players ||
                m_game_setup->isGrandPrixStarted()) &&
                m_timeout.load() == std::numeric_limits<int64_t>::max())
            {
                m_timeout.store((int64_t)StkTime::getRealTimeMs() +
                    (int64_t)
                    (ServerConfig::m_start_game_counter * 1000.0f));
            }
            else if (player_size < ServerConfig::m_min_start_game_players &&
                !m_game_setup->isGrandPrixStarted())
            {
                m_timeout.store(std::numeric_limits<int64_t>::max());
            }
            if (m_timeout.load() < (int64_t)StkTime::getRealTimeMs())
            {
                startSelection();
                return;
            }
        }
        break;
    }
    case ERROR_LEAVE:
    {
        requestTerminate();
        m_state = EXITING;
        STKHost::get()->requestShutdown();
        break;
    }
    case WAIT_FOR_WORLD_LOADED:
    {
        // m_server_has_loaded_world is set by main thread with atomic write
        if (m_server_has_loaded_world.load() == false)
            return;
        if (!checkPeersReady())
            return;
        // Reset for next state usage
        resetPeersReady();
        configPeersStartTime();
        break;
    }
    case SELECTING:
    {
        auto result = handleVote();
        if (m_timeout.load() < (int64_t)StkTime::getRealTimeMs() ||
            (std::get<3>(result) &&
            m_timeout.load() -
            (int64_t)(ServerConfig::m_voting_timeout / 2.0f * 1000.0f) <
            (int64_t)StkTime::getRealTimeMs()))
        {
            m_game_setup->setRace(std::get<0>(result), std::get<1>(result),
                std::get<2>(result));
            // Remove disconnected player (if any) one last time
            m_game_setup->update(true);
            m_game_setup->sortPlayersForGrandPrix();
            m_game_setup->sortPlayersForTeamGame();
            auto players = m_game_setup->getConnectedPlayers();
            for (auto& player : players)
                player->getPeer()->clearAvailableKartIDs();
            NetworkString* load_world = getNetworkString();
            load_world->setSynchronous(true);
            load_world->addUInt8(LE_LOAD_WORLD).encodeString(std::get<0>(result))
                .addUInt8(std::get<1>(result)).addUInt8(std::get<2>(result))
                .addUInt8((uint8_t)players.size());
            for (unsigned i = 0; i < players.size(); i++)
            {
                std::shared_ptr<NetworkPlayerProfile>& player = players[i];
                player->getPeer()->addAvailableKartID(i);
                load_world->encodeString(player->getName())
                    .addUInt32(player->getHostId())
                    .addFloat(player->getDefaultKartColor())
                    .addUInt32(player->getOnlineId())
                    .addUInt8(player->getPerPlayerDifficulty())
                    .addUInt8(player->getLocalPlayerId())
                    .addUInt8(player->getTeam());
                if (player->getKartName().empty())
                {
                    RandomGenerator rg;
                    std::set<std::string>::iterator it =
                        m_available_kts.first.begin();
                    std::advance(it, rg.get((int)m_available_kts.first.size()));
                    player->setKartName(*it);
                }
                load_world->encodeString(player->getKartName());
            }
            uint32_t random_seed = (uint32_t)StkTime::getTimeSinceEpoch();
            ItemManager::updateRandomSeed(random_seed);
            load_world->addUInt32(random_seed);
            if (race_manager->getMinorMode() == RaceManager::MINOR_MODE_BATTLE)
            {
                auto hcl = getHitCaptureLimit((float)players.size());
                load_world->addUInt32(hcl.first).addFloat(hcl.second);
                m_game_setup->setHitCaptureTime(hcl.first, hcl.second);
            }
            configRemoteKart(players);

            // Reset for next state usage
            resetPeersReady();
            m_state = LOAD_WORLD;
            sendMessageToPeers(load_world);
            delete load_world;
        }
        break;
    }
    default:
        break;
    }

}   // asynchronousUpdate

//-----------------------------------------------------------------------------
void ServerLobby::sendBadConnectionMessageToPeer(std::shared_ptr<STKPeer> p)
{
    const unsigned max_ping = ServerConfig::m_max_ping;
    Log::warn("ServerLobby", "Peer %s cannot catch up with max ping %d.",
        p->getAddress().toString().c_str(), max_ping);
    NetworkString* msg = getNetworkString();
    msg->setSynchronous(true);
    msg->addUInt8(LE_BAD_CONNECTION);
    p->sendPacket(msg, /*reliable*/true);
    delete msg;
}   // sendBadConnectionMessageToPeer

//-----------------------------------------------------------------------------
/** Simple finite state machine.  Once this
 *  is known, register the server and its address with the stk server so that
 *  client can find it.
 */
void ServerLobby::update(int ticks)
{
    // Reset server to initial state if no more connected players
    if (m_waiting_for_reset)
    {
        if ((RaceEventManager::getInstance() &&
            !RaceEventManager::getInstance()->protocolStopped()) ||
            !GameProtocol::emptyInstance())
            return;

        RaceResultGUI::getInstance()->backToLobby();
        std::lock_guard<std::mutex> lock(m_connection_mutex);
        m_game_setup->stopGrandPrix();
        resetServer();
    }

    if ((m_state.load() > WAITING_FOR_START_GAME ||
        m_game_setup->isGrandPrixStarted()) &&
        m_game_setup->getPlayerCount() == 0 &&
        NetworkConfig::get()->getServerIdFile().empty())
    {
        if (RaceEventManager::getInstance() &&
            RaceEventManager::getInstance()->isRunning())
        {
            RaceEventManager::getInstance()->stop();
            RaceEventManager::getInstance()->getProtocol()->requestTerminate();
            GameProtocol::lock()->requestTerminate();
        }
        m_waiting_for_reset = true;
        return;
    }

    // Reset for ranked server if in kart / track selection has only 1 player
    if (ServerConfig::m_ranked &&
        m_state.load() == SELECTING &&
        m_game_setup->getPlayerCount() == 1)
    {
        NetworkString* exit_result_screen = getNetworkString(1);
        exit_result_screen->setSynchronous(true);
        exit_result_screen->addUInt8(LE_EXIT_RESULT);
        sendMessageToPeers(exit_result_screen, /*reliable*/true);
        delete exit_result_screen;
        std::lock_guard<std::mutex> lock(m_connection_mutex);
        m_game_setup->stopGrandPrix();
        resetServer();
    }

    if (m_game_setup)
    {
        // Remove disconnected players if in these two states
        m_game_setup->update(m_state.load() == WAITING_FOR_START_GAME ||
            m_state.load() == SELECTING);
    }
    switch (m_state.load())
    {
    case SET_PUBLIC_ADDRESS:
    case REGISTER_SELF_ADDRESS:
    case WAITING_FOR_START_GAME:
    case WAIT_FOR_WORLD_LOADED:
    case WAIT_FOR_RACE_STARTED:
    {
        // Waiting for asynchronousUpdate
        break;
    }
    case SELECTING:
        // The function playerTrackVote will trigger the next state
        // once all track votes have been received.
        break;
    case LOAD_WORLD:
        Log::info("ServerLobbyRoom", "Starting the race loading.");
        // This will create the world instance, i.e. load track and karts
        loadWorld();
        m_state = WAIT_FOR_WORLD_LOADED;
        break;
    case RACING:
        if (World::getWorld() &&
            RaceEventManager::getInstance<RaceEventManager>()->isRunning())
        {
            checkRaceFinished();
        }
        break;
    case WAIT_FOR_RACE_STOPPED:
        if (!RaceEventManager::getInstance()->protocolStopped() ||
            !GameProtocol::emptyInstance())
            return;

        // This will go back to lobby in server (and exit the current race)
        RaceResultGUI::getInstance()->backToLobby();
        // Reset for next state usage
        resetPeersReady();
        // Set the delay before the server forces all clients to exit the race
        // result screen and go back to the lobby
        m_timeout.store((int64_t)StkTime::getRealTimeMs() + 15000);
        m_state = RESULT_DISPLAY;
        sendMessageToPeers(m_result_ns, /*reliable*/ true);
        Log::info("ServerLobby", "End of game message sent");
        break;
    case RESULT_DISPLAY:
        if (checkPeersReady() ||
            (int64_t)StkTime::getRealTimeMs() > m_timeout.load())
        {
            // Send a notification to all clients to exit
            // the race result screen
            NetworkString *exit_result_screen = getNetworkString(1);
            exit_result_screen->setSynchronous(true);
            exit_result_screen->addUInt8(LE_EXIT_RESULT);
            sendMessageToPeers(exit_result_screen, /*reliable*/true);
            delete exit_result_screen;
            std::lock_guard<std::mutex> lock(m_connection_mutex);
            resetServer();
        }
        break;
    case ERROR_LEAVE:
    case EXITING:
        break;
    }
}   // update

//-----------------------------------------------------------------------------
/** Register this server (i.e. its public address) with the STK server
 *  so that clients can find it. It blocks till a response from the
 *  stk server is received (this function is executed from the 
 *  ProtocolManager thread). The information about this client is added
 *  to the table 'server'.
 */
bool ServerLobby::registerServer(bool now)
{
    while (now && !m_server_unregistered.expired())
        StkTime::sleep(1);

    // ========================================================================
    class RegisterServerRequest : public Online::XMLRequest
    {
    private:
        std::weak_ptr<ServerLobby> m_server_lobby;
    protected:
        virtual void afterOperation()
        {
            Online::XMLRequest::afterOperation();
            const XMLNode* result = getXMLData();
            std::string rec_success;
            auto sl = m_server_lobby.lock();
            if (!sl)
                return;

            if (result->get("success", &rec_success) &&
                rec_success == "yes")
            {
                const XMLNode* server = result->getNode("server");
                assert(server);
                const XMLNode* server_info = server->getNode("server-info");
                assert(server_info);
                unsigned server_id_online = 0;
                server_info->get("id", &server_id_online);
                assert(server_id_online != 0);
                Log::info("ServerLobby",
                    "Server %d is now online.", server_id_online);
                sl->m_server_id_online.store(server_id_online);
                sl->m_last_success_poll_time.store(StkTime::getRealTimeMs());
                return;
            }
            Log::error("ServerLobby", "%s",
                StringUtils::wideToUtf8(getInfo()).c_str());
            // For auto server recovery wait 3 seconds for next try
            // This sleep only the request manager thread
            if (manageMemory())
                StkTime::sleep(3000);
        }
    public:
        RegisterServerRequest(bool now, std::shared_ptr<ServerLobby> sl)
        : XMLRequest(!now/*manage memory*/), m_server_lobby(sl) {}
    };   // RegisterServerRequest

    RegisterServerRequest *request = new RegisterServerRequest(now,
        std::dynamic_pointer_cast<ServerLobby>(shared_from_this()));
    NetworkConfig::get()->setServerDetails(request, "create");
    request->addParameter("address",      m_server_address.getIP()        );
    request->addParameter("port",         m_server_address.getPort()      );
    request->addParameter("private_port",
                                    STKHost::get()->getPrivatePort()      );
    // The ServerConfig::m_server_name has xml encoded name so we send this
    // insteaf of from game_setup
    const std::string& server_name = ServerConfig::m_server_name;
    request->addParameter("name", server_name);
    request->addParameter("max_players", ServerConfig::m_server_max_players);
    request->addParameter("difficulty", ServerConfig::m_server_difficulty);
    request->addParameter("game_mode", ServerConfig::m_server_mode);
    const std::string& pw = ServerConfig::m_private_server_password;
    request->addParameter("password", (unsigned)(!pw.empty()));
    request->addParameter("version", (unsigned)ServerConfig::m_server_version);

    Log::info("ServerLobby", "Public server address %s",
        m_server_address.toString().c_str());

    if (now)
    {
        request->executeNow();
        delete request;
        if (m_server_id_online.load() == 0)
            return false;
    }
    else
    {
        request->queue();
        m_server_recovering = request->observeExistence();
    }
    return true;
}   // registerServer

//-----------------------------------------------------------------------------
/** Unregister this server (i.e. its public address) with the STK server,
 *  currently when karts enter kart selection screen it will be done or quit
 *  stk.
 */
void ServerLobby::unregisterServer(bool now)
{
    Online::XMLRequest* request =
        new Online::XMLRequest(!now/*manage memory*/);
    m_server_unregistered = request->observeExistence();
    NetworkConfig::get()->setServerDetails(request, "stop");

    request->addParameter("address", m_server_address.getIP());
    request->addParameter("port", m_server_address.getPort());
    Log::info("ServerLobby", "Unregister server address %s",
        m_server_address.toString().c_str());
    // No need to check for result as server will be auto-cleared anyway
    // when no polling is done
    if (now)
    {
        request->executeNow();
        delete request;
    }
    else
        request->queue();

}   // unregisterServer

//-----------------------------------------------------------------------------
/** Instructs all clients to start the kart selection. If event is NULL,
 *  the command comes from the owner less server.
 */
void ServerLobby::startSelection(const Event *event)
{
    if (event != NULL)
    {
        if (m_state != WAITING_FOR_START_GAME)
        {
            Log::warn("ServerLobby",
                "Received startSelection while being in state %d",
                m_state.load());
            return;
        }
        if (event->getPeerSP() != m_server_owner.lock())
        {
            Log::warn("ServerLobby",
                "Client %d is not authorised to start selection.",
                event->getPeer()->getHostId());
            return;
        }
    }

    if (ServerConfig::m_team_choosing && race_manager->teamEnabled())
    {
        auto red_blue = m_game_setup->getPlayerTeamInfo();
        if ((red_blue.first == 0 || red_blue.second == 0) &&
            red_blue.first + red_blue.second != 1)
        {
            Log::warn("ServerLobby", "Bad team choosing.");
            NetworkString* bt = getNetworkString();
            bt->setSynchronous(true);
            bt->addUInt8(LE_BAD_TEAM);
            sendMessageToPeers(bt, true/*reliable*/);
            delete bt;
            return;
        }
    }

    if (!allowJoinedPlayersWaiting())
    {
        ProtocolManager::lock()->findAndTerminate(PROTOCOL_CONNECTION);
        if (NetworkConfig::get()->isWAN())
        {
            unregisterServer(false/*now*/);
        }
    }

    NetworkString *ns = getNetworkString(1);
    // Start selection - must be synchronous since the receiver pushes
    // a new screen, which must be done from the main thread.
    ns->setSynchronous(true);
    ns->addUInt8(LE_START_SELECTION).addUInt8(
        m_game_setup->isGrandPrixStarted() ? 1 : 0)
        .addUInt8(ServerConfig::m_auto_lap_ratio > 0.0f ? 1 : 0);

    // Remove karts / tracks from server that are not supported on all clients
    std::set<std::string> karts_erase, tracks_erase;
    auto peers = STKHost::get()->getPeers();
    for (auto peer : peers)
    {
        if (!peer->isValidated() || peer->isWaitingForGame())
            continue;
        peer->eraseServerKarts(m_available_kts.first, karts_erase);
        peer->eraseServerTracks(m_available_kts.second, tracks_erase);
    }
    for (const std::string& kart_erase : karts_erase)
    {
        m_available_kts.first.erase(kart_erase);
    }
    for (const std::string& track_erase : tracks_erase)
    {
        m_available_kts.second.erase(track_erase);
    }

    if (race_manager->getMinorMode() == RaceManager::MINOR_MODE_BATTLE &&
        race_manager->getMajorMode() == RaceManager::MAJOR_MODE_FREE_FOR_ALL)
    {
        auto it = m_available_kts.second.begin();
        while (it != m_available_kts.second.end())
        {
            Track* t =  track_manager->getTrack(*it);
            if (t->getMaxArenaPlayers() < m_game_setup->getPlayerCount())
            {
                it = m_available_kts.second.erase(it);
            }
            else
                it++;
        }
    }

    const auto& all_k = m_available_kts.first;
    const auto& all_t = m_available_kts.second;
    ns->addUInt16((uint16_t)all_k.size()).addUInt16((uint16_t)all_t.size());
    for (const std::string& kart : all_k)
    {
        ns->encodeString(kart);
    }
    for (const std::string& track : all_t)
    {
        ns->encodeString(track);
    }

    sendMessageToPeers(ns, /*reliable*/true);
    delete ns;

    m_state = SELECTING;
    if (!allowJoinedPlayersWaiting())
    {
        // Drop all pending players and keys if doesn't allow joinning-waiting
        for (auto& p : m_pending_connection)
        {
            if (auto peer = p.first.lock())
                peer->disconnect();
        }
        m_pending_connection.clear();
        std::unique_lock<std::mutex> ul(m_keys_mutex);
        m_keys.clear();
        ul.unlock();
    }

    // Will be changed after the first vote received
    m_timeout.store(std::numeric_limits<int64_t>::max());
}   // startSelection

//-----------------------------------------------------------------------------
/** Query the STK server for connection requests. For each connection request
 *  start a ConnectToPeer protocol.
 */
void ServerLobby::checkIncomingConnectionRequests()
{
    // First poll every 5 seconds. Return if no polling needs to be done.
    const uint64_t POLL_INTERVAL = 5000;
    static uint64_t last_poll_time = 0;
    if (StkTime::getRealTimeMs() < last_poll_time + POLL_INTERVAL ||
        StkTime::getRealTimeMs() > m_last_success_poll_time.load() + 30000 ||
        m_server_id_online.load() == 0)
        return;

    // Keep the port open, it can be sent to anywhere as we will send to the
    // correct peer later in ConnectToPeer.
    if (ServerConfig::m_firewalled_server)
    {
        BareNetworkString data;
        data.addUInt8(0);
        STKHost::get()->sendRawPacket(data, STKHost::get()->getStunAddress());
    }

    // Now poll the stk server
    last_poll_time = StkTime::getRealTimeMs();

    // ========================================================================
    class PollServerRequest : public Online::XMLRequest
    {
    private:
        std::weak_ptr<ServerLobby> m_server_lobby;
    protected:
        virtual void afterOperation()
        {
            Online::XMLRequest::afterOperation();
            const XMLNode* result = getXMLData();
            std::string success;

            if (!result->get("success", &success) || success != "yes")
            {
                Log::error("ServerLobby", "Poll server request failed: %s",
                    StringUtils::wideToUtf8(getInfo()).c_str());
                return;
            }

            // Now start a ConnectToPeer protocol for each connection request
            const XMLNode * users_xml = result->getNode("users");
            std::map<uint32_t, KeyData> keys;
            auto sl = m_server_lobby.lock();
            if (!sl)
                return;
            sl->m_last_success_poll_time.store(StkTime::getRealTimeMs());
            if (sl->m_state.load() != WAITING_FOR_START_GAME &&
                !sl->allowJoinedPlayersWaiting())
            {
                sl->replaceKeys(keys);
                return;
            }

            sl->removeExpiredPeerConnection();
            for (unsigned int i = 0; i < users_xml->getNumNodes(); i++)
            {
                uint32_t addr, id;
                uint16_t port;
                users_xml->getNode(i)->get("ip", &addr);
                users_xml->getNode(i)->get("port", &port);
                users_xml->getNode(i)->get("id", &id);
                users_xml->getNode(i)->get("aes-key", &keys[id].m_aes_key);
                users_xml->getNode(i)->get("aes-iv", &keys[id].m_aes_iv);
                users_xml->getNode(i)->get("username", &keys[id].m_name);
                keys[id].m_tried = false;
                if (ServerConfig::m_firewalled_server)
                {
                    TransportAddress peer_addr(addr, port);
                    std::string peer_addr_str = peer_addr.toString();
                    if (sl->m_pending_peer_connection.find(peer_addr_str) !=
                        sl->m_pending_peer_connection.end())
                    {
                        continue;
                    }
                    std::make_shared<ConnectToPeer>(peer_addr)->requestStart();
                    sl->addPeerConnection(peer_addr_str);
                }
            }
            sl->replaceKeys(keys);
        }
    public:
        PollServerRequest(std::shared_ptr<ServerLobby> sl)
        : XMLRequest(true), m_server_lobby(sl)
        {
            m_disable_sending_log = true;
        }
    };   // PollServerRequest
    // ========================================================================

    PollServerRequest* request = new PollServerRequest(
        std::dynamic_pointer_cast<ServerLobby>(shared_from_this()));
    NetworkConfig::get()->setServerDetails(request,
        "poll-connection-requests");
    const TransportAddress &addr = STKHost::get()->getPublicAddress();
    request->addParameter("address", addr.getIP()  );
    request->addParameter("port",    addr.getPort());
    request->addParameter("current-players",
        m_game_setup->getPlayerCount() + m_waiting_players_counts.load());
    request->addParameter("game-started",
        m_state.load() == WAITING_FOR_START_GAME ? 0 : 1);
    request->queue();

}   // checkIncomingConnectionRequests

//-----------------------------------------------------------------------------
/** Checks if the race is finished, and if so informs the clients and switches
 *  to state RESULT_DISPLAY, during which the race result gui is shown and all
 *  clients can click on 'continue'.
 */
void ServerLobby::checkRaceFinished()
{
    assert(RaceEventManager::getInstance()->isRunning());
    assert(World::getWorld());
    if (!RaceEventManager::getInstance()->isRaceOver()) return;

    Log::info("ServerLobby", "The game is considered finish.");
    // notify the network world that it is stopped
    RaceEventManager::getInstance()->stop();

    // stop race protocols before going back to lobby (end race)
    RaceEventManager::getInstance()->getProtocol()->requestTerminate();
    GameProtocol::lock()->requestTerminate();

    // Save race result before delete the world
    m_result_ns->clear();
    m_result_ns->addUInt8(LE_RACE_FINISHED);
    if (m_game_setup->isGrandPrix())
    {
        // fastest lap
        int fastest_lap =
            static_cast<LinearWorld*>(World::getWorld())->getFastestLapTicks();
        m_result_ns->addUInt32(fastest_lap);

        // all gp tracks
        m_result_ns->addUInt8((uint8_t)m_game_setup->getTotalGrandPrixTracks())
            .addUInt8((uint8_t)m_game_setup->getAllTracks().size());
        for (const std::string& gp_track : m_game_setup->getAllTracks())
            m_result_ns->encodeString(gp_track);

        // each kart score and total time
        auto& players = m_game_setup->getPlayers();
        m_result_ns->addUInt8((uint8_t)players.size());
        for (unsigned i = 0; i < players.size(); i++)
        {
            int last_score = race_manager->getKartScore(i);
            int cur_score = last_score;
            float overall_time = race_manager->getOverallTime(i);
            if (auto player = players[i].lock())
            {
                last_score = player->getScore();
                cur_score += last_score;
                overall_time = overall_time + player->getOverallTime();
                player->setScore(cur_score);
                player->setOverallTime(overall_time);
            }
            m_result_ns->addUInt32(last_score).addUInt32(cur_score)
                .addFloat(overall_time);            
        }
    }
    else if (race_manager->modeHasLaps())
    {
        int fastest_lap =
            static_cast<LinearWorld*>(World::getWorld())->getFastestLapTicks();
        m_result_ns->addUInt32(fastest_lap);
    }
    if (ServerConfig::m_ranked)
    {
        computeNewRankings();
        submitRankingsToAddons();
    }
    m_state.store(WAIT_FOR_RACE_STOPPED);
}   // checkRaceFinished

//-----------------------------------------------------------------------------
/** Compute the new player's rankings used in ranked servers
 */
void ServerLobby::computeNewRankings()
{
    // No ranking for battle mode
    if (!race_manager->modeHasLaps())
        return;

    // Using a vector of vector, it would be possible to fill
    // all j < i v[i][j] with -v[j][i]
    // Would this be worth it ?
    std::vector<double> scores_change;
    std::vector<double> new_scores;

    auto players = m_game_setup->getConnectedPlayers(true/*same_offset*/);
    for (unsigned i = 0; i < players.size(); i++)
    {
        const uint32_t id = race_manager->getKartInfo(i).getOnlineId();
        new_scores.push_back(m_scores.at(id));
        new_scores[i] += distributeBasePoints(id);
    }

    for (unsigned i = 0; i < players.size(); i++)
    {
        scores_change.push_back(0.0);

        double player1_scores = new_scores[i];
        // If the player has quitted before the race end,
        // the value will be incorrect, but it will not be used
        double player1_time  = race_manager->getKartRaceTime(i);
        double player1_factor =
            computeRankingFactor(race_manager->getKartInfo(i).getOnlineId());

        for (unsigned j = 0; j < players.size(); j++)
        {
            // Don't compare a player with himself
            if (i == j)
                continue;

            double result = 0.0;
            double expected_result = 0.0;
            double ranking_importance = 0.0;

            // No change between two quitting players
            if (!players[i] && !players[j])
                continue;

            double player2_scores = new_scores[j];
            double player2_time = race_manager->getKartRaceTime(j);

            // Compute the expected result using an ELO-like function
            double diff = player2_scores - player1_scores;
            expected_result = 1.0/ (1.0 + std::pow(10.0,
                diff / (BASE_RANKING_POINTS * getModeSpread() / 2.0)));

            // Compute the result and race ranking importance
            double player_factors = std::max(player1_factor,
                computeRankingFactor(
                race_manager->getKartInfo(j).getOnlineId()));

            double mode_factor = getModeFactor();

            if (!players[i])
            {
                result = 0.0;
                ranking_importance = mode_factor *
                    scalingValueForTime(MAX_SCALING_TIME) * player_factors;
            }
            else if (!players[j])
            {
                result = 1.0;
                ranking_importance = mode_factor *
                    scalingValueForTime(MAX_SCALING_TIME) * player_factors;
            }
            else
            {
                // If time difference > 2,5% ; the result is 1 or 0
                // Otherwise, it is averaged between 0 and 1.
                if (player1_time <= player2_time)
                {
                    result =
                        (player2_time - player1_time) / (player1_time / 20.0);
                    result = std::min(1.0, 0.5 + result);
                }
                else
                {
                    result =
                        (player1_time - player2_time) / (player2_time / 20.0);
                    result = std::max(0.0, 0.5 - result);
                }

                double max_time = std::min(std::max(player1_time, player2_time),
                    MAX_SCALING_TIME);
                ranking_importance = mode_factor *
                    scalingValueForTime(max_time) * player_factors;
            }
            // Compute the ranking change
            scores_change[i] +=
                ranking_importance * (result - expected_result);
        }
    }

    // Don't merge it in the main loop as new_scores value are used there
    for (unsigned i = 0; i < players.size(); i++)
    {
        new_scores[i] += scores_change[i];
        const uint32_t id = race_manager->getKartInfo(i).getOnlineId();
        m_scores.at(id) =  new_scores[i];
        if (m_scores.at(id) > m_max_scores.at(id))
            m_max_scores.at(id) = m_scores.at(id);
        m_num_ranked_races.at(id)++;
    }
}   // computeNewRankings

//-----------------------------------------------------------------------------
/** Compute the ranking factor, used to make top rankings more stable
 *  and to allow new players to faster get to an appropriate ranking
 */
double ServerLobby::computeRankingFactor(uint32_t online_id)
{
    double max_points = m_max_scores.at(online_id);
    unsigned num_races = m_num_ranked_races.at(online_id);

    if (max_points >= (BASE_RANKING_POINTS * 2.0))
        return 0.4;
    else if (max_points >= (BASE_RANKING_POINTS * 1.75) || num_races > 500)
        return 0.5;
    else if (max_points >= (BASE_RANKING_POINTS * 1.5) || num_races > 250)
        return 0.6;
    else if (max_points >= (BASE_RANKING_POINTS * 1.25) || num_races > 100)
        return 0.7;
    // The base ranking points are not distributed all at once
    // So it's not guaranteed a player reach them
    else if (max_points >= (BASE_RANKING_POINTS) || num_races > 50)
        return 0.8;
    else
        return 1.0;

}   // computeRankingFactor

//-----------------------------------------------------------------------------
/** Returns the mode race importance factor,
 *  used to make ranking move slower in more random modes.
 */
double ServerLobby::getModeFactor()
{
    if (race_manager->isTimeTrialMode())
        return 1.0;
    return 0.4;
}   // getModeFactor

//-----------------------------------------------------------------------------
/** Returns the mode spread factor, used so that a similar difference in
 *  skill will result in a similar ranking difference in more random modes.
 */
double ServerLobby::getModeSpread()
{
    if (race_manager->isTimeTrialMode())
        return 1.0;

    //TODO: the value used here for normal races is a wild guess.
    // When hard data to the spread tendencies of time-trial
    // and normal mode becomes available, update this to make
    // the spreads more comparable
    return 1.4;
}   // getModeSpread

//-----------------------------------------------------------------------------
/** Compute the scaling value of a given time
 *  Short races are more random, so we don't use strict proportionality
 */
double ServerLobby::scalingValueForTime(double time)
{
    return time * sqrt(time / 120.0) * MAX_POINTS_PER_SECOND;
}   // scalingValueForTime

//-----------------------------------------------------------------------------
/** Manages the distribution of the base points.
 *  Gives half of the points progressively
 *  by smaller and smaller chuncks from race 1 to 45.
 *  The first half is distributed when the player enters
 *  for the first time in the ranked server.
 */
double ServerLobby::distributeBasePoints(uint32_t online_id)
{
    unsigned num_races  = m_num_ranked_races.at(online_id);
    if (num_races < 45)
    {
        return BASE_RANKING_POINTS / 2000.0 * std::max((45u - num_races), 4u);
    }
    else
        return 0.0;
}   // distributeBasePoints

//-----------------------------------------------------------------------------
/** Called when a client disconnects.
 *  \param event The disconnect event.
 */
void ServerLobby::clientDisconnected(Event* event)
{
    auto players_on_peer = event->getPeer()->getPlayerProfiles();
    if (players_on_peer.empty())
        return;

    NetworkString* msg = getNetworkString(2);
    const bool waiting_peer_disconnected =
        event->getPeer()->isWaitingForGame();
    msg->setSynchronous(true);
    msg->addUInt8(LE_PLAYER_DISCONNECTED);
    msg->addUInt8((uint8_t)players_on_peer.size());
    for (auto p : players_on_peer)
    {
        std::string name = StringUtils::wideToUtf8(p->getName());
        msg->encodeString(name);
        Log::info("ServerLobby", "%s disconnected", name.c_str());
    }

    // Don't show waiting peer disconnect message to in game player
    STKHost::get()->sendPacketToAllPeersWith([waiting_peer_disconnected]
        (STKPeer* p)
        {
            if (!p->isValidated())
                return false;
            if (!p->isWaitingForGame() && waiting_peer_disconnected)
                return false;
            return true;
        }, msg);
    updatePlayerList();
    delete msg;
}   // clientDisconnected

//-----------------------------------------------------------------------------
void ServerLobby::clearDisconnectedRankedPlayer()
{
    for (auto it = m_ranked_players.begin(); it != m_ranked_players.end();)
    {
        if (it->second.expired())
        {
            const uint32_t id = it->first;
            m_scores.erase(id);
            m_max_scores.erase(id);
            m_num_ranked_races.erase(id);
            it = m_ranked_players.erase(it);
        }
        else
        {
            it++;
        }
    }
}   // clearDisconnectedRankedPlayer

//-----------------------------------------------------------------------------
void ServerLobby::connectionRequested(Event* event)
{
    std::lock_guard<std::mutex> lock(m_connection_mutex);
    std::shared_ptr<STKPeer> peer = event->getPeerSP();
    NetworkString& data = event->data();
    if (!checkDataSize(event, 14)) return;

    peer->cleanPlayerProfiles();

    // can we add the player ?
    if (!allowJoinedPlayersWaiting() &&
        (m_state.load() != WAITING_FOR_START_GAME ||
        m_game_setup->isGrandPrixStarted()))
    {
        NetworkString *message = getNetworkString(2);
        message->setSynchronous(true);
        message->addUInt8(LE_CONNECTION_REFUSED).addUInt8(RR_BUSY);
        // send only to the peer that made the request and disconect it now
        peer->sendPacket(message, true/*reliable*/, false/*encrypted*/);
        peer->reset();
        delete message;
        Log::verbose("ServerLobby", "Player refused: selection started");
        return;
    }

    // Check server version
    int version = data.getUInt32();
    if (version < stk_config->m_min_server_version ||
        version > stk_config->m_max_server_version)
    {
        NetworkString *message = getNetworkString(2);
        message->setSynchronous(true);
        message->addUInt8(LE_CONNECTION_REFUSED)
                .addUInt8(RR_INCOMPATIBLE_DATA);
        peer->sendPacket(message, true/*reliable*/, false/*encrypted*/);
        peer->reset();
        delete message;
        Log::verbose("ServerLobby", "Player refused: wrong server version");
        return;
    }
    std::string user_version;
    data.decodeString(&user_version);
    event->getPeer()->setUserVersion(user_version);

    std::set<std::string> client_karts, client_tracks;
    const unsigned kart_num = data.getUInt16();
    const unsigned track_num = data.getUInt16();
    for (unsigned i = 0; i < kart_num; i++)
    {
        std::string kart;
        data.decodeString(&kart);
        client_karts.insert(kart);
    }
    for (unsigned i = 0; i < track_num; i++)
    {
        std::string track;
        data.decodeString(&track);
        client_tracks.insert(track);
    }

    // Drop this player if he doesn't have at least 1 kart / track the same
    // as server
    std::set<std::string> karts_erase, tracks_erase;
    for (const std::string& server_kart : m_available_kts.first)
    {
        if (client_karts.find(server_kart) == client_karts.end())
        {
            karts_erase.insert(server_kart);
        }
    }
    for (const std::string& server_track : m_available_kts.second)
    {
        if (client_tracks.find(server_track) == client_tracks.end())
        {
            tracks_erase.insert(server_track);
        }
    }

    if (karts_erase.size() == m_available_kts.first.size() ||
        tracks_erase.size() == m_available_kts.second.size())
    {
        NetworkString *message = getNetworkString(2);
        message->setSynchronous(true);
        message->addUInt8(LE_CONNECTION_REFUSED)
            .addUInt8(RR_INCOMPATIBLE_DATA);
        peer->cleanPlayerProfiles();
        peer->sendPacket(message, true/*reliable*/, false/*encrypted*/);
        peer->reset();
        delete message;
        Log::verbose("ServerLobby", "Player has incompatible karts / tracks");
        return;
    }

    // Save available karts and tracks from clients in STKPeer so if this peer
    // disconnects later in lobby it won't affect current players
    peer->setAvailableKartsTracks(client_karts, client_tracks);

    unsigned player_count = data.getUInt8();
    uint32_t online_id = 0;
    uint32_t encrypted_size = 0;
    online_id = data.getUInt32();
    encrypted_size = data.getUInt32();

    bool is_banned = isBannedForIP(peer->getAddress());
    if (online_id != 0 && !is_banned)
    {
        if (m_online_id_ban_list.find(online_id) !=
            m_online_id_ban_list.end() &&
            (uint32_t)StkTime::getTimeSinceEpoch() <
            m_online_id_ban_list.at(online_id))
        {
            is_banned = true;
        }
    }

    if (is_banned)
    {
        NetworkString *message = getNetworkString(2);
        message->setSynchronous(true);
        message->addUInt8(LE_CONNECTION_REFUSED).addUInt8(RR_BANNED);
        peer->cleanPlayerProfiles();
        peer->sendPacket(message, true/*reliable*/, false/*encrypted*/);
        peer->reset();
        delete message;
        Log::verbose("ServerLobby", "Player refused: banned");
        return;
    }

    if (m_game_setup->getPlayerCount() + player_count +
        m_waiting_players_counts.load() >
        (unsigned)ServerConfig::m_server_max_players)
    {
        NetworkString *message = getNetworkString(2);
        message->setSynchronous(true);
        message->addUInt8(LE_CONNECTION_REFUSED).addUInt8(RR_TOO_MANY_PLAYERS);
        peer->sendPacket(message, true/*reliable*/, false/*encrypted*/);
        peer->reset();
        delete message;
        Log::verbose("ServerLobby", "Player refused: too many players");
        return;
    }

    // Reject non-valiated player joinning if WAN server and not disabled
    // encforement of validation, unless it's player from localhost or lan
    // And no duplicated online id or split screen players in ranked server
    bool duplicated_ranked_player =
        m_ranked_players.find(online_id) != m_ranked_players.end() &&
        !m_ranked_players.at(online_id).expired();

    if (((encrypted_size == 0 || online_id == 0) &&
        !(peer->getAddress().isPublicAddressLocalhost() ||
        peer->getAddress().isLAN()) &&
        NetworkConfig::get()->isWAN() &&
        ServerConfig::m_validating_player) ||
        (ServerConfig::m_ranked &&
        (player_count != 1 || online_id == 0 || duplicated_ranked_player)))
    {
        NetworkString* message = getNetworkString(2);
        message->setSynchronous(true);
        message->addUInt8(LE_CONNECTION_REFUSED).addUInt8(RR_INVALID_PLAYER);
        peer->sendPacket(message, true/*reliable*/, false/*encrypted*/);
        peer->reset();
        delete message;
        Log::verbose("ServerLobby", "Player refused: invalid player");
        return;
    }

    if (encrypted_size != 0)
    {
        m_pending_connection[peer] = std::make_pair(online_id,
            BareNetworkString(data.getCurrentData(), encrypted_size));
    }
    else
    {
        core::stringw online_name;
        if (online_id > 0)
            data.decodeStringW(&online_name);
        handleUnencryptedConnection(peer, data, online_id, online_name);
    }
}   // connectionRequested

//-----------------------------------------------------------------------------
void ServerLobby::handleUnencryptedConnection(std::shared_ptr<STKPeer> peer,
    BareNetworkString& data, uint32_t online_id,
    const core::stringw& online_name)
{
    if (data.size() < 2) return;

    // Check for password
    std::string password;
    data.decodeString(&password);
    const std::string& server_pw = ServerConfig::m_private_server_password;
    if (password != server_pw)
    {
        NetworkString *message = getNetworkString(2);
        message->setSynchronous(true);
        message->addUInt8(LE_CONNECTION_REFUSED)
                .addUInt8(RR_INCORRECT_PASSWORD);
        peer->sendPacket(message, true/*reliable*/, false/*encrypted*/);
        peer->reset();
        delete message;
        Log::verbose("ServerLobby", "Player refused: incorrect password");
        return;
    }

    // Check again for duplicated player in ranked server
    bool duplicated_ranked_player =
        m_ranked_players.find(online_id) != m_ranked_players.end() &&
        !m_ranked_players.at(online_id).expired();
    if (ServerConfig::m_ranked && duplicated_ranked_player)
    {
        NetworkString* message = getNetworkString(2);
        message->setSynchronous(true);
        message->addUInt8(LE_CONNECTION_REFUSED).addUInt8(RR_INVALID_PLAYER);
        peer->sendPacket(message, true/*reliable*/, false/*encrypted*/);
        peer->reset();
        delete message;
        Log::verbose("ServerLobby", "Player refused: invalid player");
        return;
    }

    unsigned player_count = data.getUInt8();
    auto red_blue = m_game_setup->getPlayerTeamInfo();
    for (unsigned i = 0; i < player_count; i++)
    {
        core::stringw name;
        data.decodeStringW(&name);
        float default_kart_color = data.getFloat();
        PerPlayerDifficulty per_player_difficulty =
            (PerPlayerDifficulty)data.getUInt8();
        auto player = std::make_shared<NetworkPlayerProfile>
            (peer, i == 0 && !online_name.empty() ? online_name : name,
            peer->getHostId(), default_kart_color, i == 0 ? online_id : 0,
            per_player_difficulty, (uint8_t)i, KART_TEAM_NONE);
        if (ServerConfig::m_team_choosing &&
            race_manager->teamEnabled())
        {
            KartTeam cur_team = KART_TEAM_NONE;
            if (red_blue.first > red_blue.second)
            {
                cur_team = KART_TEAM_BLUE;
                red_blue.second++;
            }
            else
            {
                cur_team = KART_TEAM_RED;
                red_blue.first++;
            }
            player->setTeam(cur_team);
        }
        peer->addPlayer(player);
    }

    peer->setValidated();

    // send a message to the one that asked to connect
    NetworkString* server_info = getNetworkString();
    server_info->setSynchronous(true);
    server_info->addUInt8(LE_SERVER_INFO);
    m_game_setup->addServerInfo(server_info);
    peer->sendPacket(server_info);
    delete server_info;

    const bool game_started = m_state.load() != WAITING_FOR_START_GAME;
    NetworkString* message_ack = getNetworkString(4);
    message_ack->setSynchronous(true);
    // connection success -- return the host id of peer
    float auto_start_timer = 0.0f;
    if (m_timeout.load() == std::numeric_limits<int64_t>::max())
        auto_start_timer = std::numeric_limits<float>::max();
    else
    {
        auto_start_timer =
            (m_timeout.load() - (int64_t)StkTime::getRealTimeMs()) / 1000.0f;
    }
    message_ack->addUInt8(LE_CONNECTION_ACCEPTED).addUInt32(peer->getHostId())
        .addUInt32(ServerConfig::m_server_version).addFloat(auto_start_timer);

    if (game_started)
    {
        peer->setWaitingForGame(true);
        for (auto& p : peer->getPlayerProfiles())
            m_waiting_players.push_back(p);
        updatePlayerList();
        peer->sendPacket(message_ack);
        delete message_ack;
    }
    else
    {
        peer->setWaitingForGame(false);
        m_peers_ready[peer] = false;
        for (std::shared_ptr<NetworkPlayerProfile>& npp :
            peer->getPlayerProfiles())
        {
            m_game_setup->addPlayer(npp);
            Log::info("ServerLobby",
                "New player %s with online id %u from %s with %s.",
                StringUtils::wideToUtf8(npp->getName()).c_str(),
                npp->getOnlineId(), peer->getAddress().toString().c_str(),
                peer->getUserVersion().c_str());
        }
        updatePlayerList();
        peer->sendPacket(message_ack);
        delete message_ack;

        if (ServerConfig::m_ranked)
        {
            getRankingForPlayer(peer->getPlayerProfiles()[0]);
        }
    }
}   // handleUnencryptedConnection

//-----------------------------------------------------------------------------
/** Called when any players change their setting (team for example), or
 *  connection / disconnection, it will use the game_started parameter to
 *  determine if this should be send to all peers in server or just in game.
 *  \param update_when_reset_server If true, this message will be sent to
 *  all peers.
 */
void ServerLobby::updatePlayerList(bool update_when_reset_server)
{
    const bool game_started = m_state.load() != WAITING_FOR_START_GAME &&
        !update_when_reset_server;

    // No need to update player list (for started grand prix currently)
    if (!allowJoinedPlayersWaiting() &&
        m_state.load() > WAITING_FOR_START_GAME && !update_when_reset_server)
        return;

    auto all_profiles = STKHost::get()->getAllPlayerProfiles();
    NetworkString* pl = getNetworkString();
    pl->setSynchronous(true);
    pl->addUInt8(LE_UPDATE_PLAYER_LIST)
        .addUInt8((uint8_t)(game_started ? 1 : 0))
        .addUInt8((uint8_t)all_profiles.size());
    for (auto profile : all_profiles)
    {
        pl->addUInt32(profile->getHostId()).addUInt32(profile->getOnlineId())
            .addUInt8(profile->getLocalPlayerId())
            .encodeString(profile->getName());
        pl->addUInt8((uint8_t)
            (profile->getPeer()->isWaitingForGame() ? 1 : 0));
        uint8_t server_owner = 0;
        if (m_server_owner_id.load() == profile->getPeer()->getHostId())
            server_owner = 1;
        pl->addUInt8(server_owner);
        pl->addUInt8(profile->getPerPlayerDifficulty());
        if (ServerConfig::m_team_choosing &&
            race_manager->teamEnabled())
            pl->addUInt8(profile->getTeam());
        else
            pl->addUInt8(KART_TEAM_NONE);
    }

    // Don't send this message to in-game players
    STKHost::get()->sendPacketToAllPeersWith([game_started]
        (STKPeer* p)
        {
            if (!p->isValidated())
                return false;
            if (!p->isWaitingForGame() && game_started)
                return false;
            return true;
        }, pl);
    delete pl;
}   // updatePlayerList

//-----------------------------------------------------------------------------
void ServerLobby::updateServerOwner()
{
    if (m_state.load() < WAITING_FOR_START_GAME ||
        m_state.load() > RESULT_DISPLAY ||
        ServerConfig::m_owner_less)
        return;
    if (!m_server_owner.expired())
        return;
    auto peers = STKHost::get()->getPeers();
    if (peers.empty())
        return;
    std::sort(peers.begin(), peers.end(), [](const std::shared_ptr<STKPeer> a,
        const std::shared_ptr<STKPeer> b)->bool
        {
            return a->getHostId() < b->getHostId();
        });

    std::shared_ptr<STKPeer> owner;
    for (auto peer: peers)
    {
        // Only 127.0.0.1 can be server owner in case of graphics-client-server
        if (peer->isValidated() &&
            (NetworkConfig::get()->getServerIdFile().empty() ||
            peer->getAddress().getIP() == 0x7f000001))
        {
            owner = peer;
            break;
        }
    }
    if (owner)
    {
        NetworkString* ns = getNetworkString();
        ns->setSynchronous(true);
        ns->addUInt8(LE_SERVER_OWNERSHIP);
        owner->sendPacket(ns);
        delete ns;
        m_server_owner = owner;
        m_server_owner_id.store(owner->getHostId());
        updatePlayerList();
    }
}   // updateServerOwner

//-----------------------------------------------------------------------------
/*! \brief Called when a player asks to select karts.
 *  \param event : Event providing the information.
 */
void ServerLobby::kartSelectionRequested(Event* event)
{
    if (m_state != SELECTING || m_game_setup->isGrandPrixStarted())
    {
        Log::warn("ServerLobby", "Received kart selection while in state %d.",
                  m_state.load());
        return;
    }

    if (!checkDataSize(event, 1) ||
        event->getPeer()->getPlayerProfiles().empty())
        return;

    const NetworkString& data = event->data();
    STKPeer* peer = event->getPeer();
    unsigned player_count = data.getUInt8();
    for (unsigned i = 0; i < player_count; i++)
    {
        std::string kart;
        data.decodeString(&kart);
        if (m_available_kts.first.find(kart) == m_available_kts.first.end())
        {
            continue;
        }
        else
        {
            peer->getPlayerProfiles()[i]->setKartName(kart);
        }
    }
}   // kartSelectionRequested

//-----------------------------------------------------------------------------
/*! \brief Called when a player votes for track(s).
 *  \param event : Event providing the information.
 */
void ServerLobby::playerVote(Event* event)
{
    if (m_state != SELECTING)
    {
        Log::warn("ServerLobby", "Received track vote while in state %d.",
                  m_state.load());
        return;
    }

    if (!checkDataSize(event, 4) ||
        event->getPeer()->getPlayerProfiles().empty() ||
        event->getPeer()->isWaitingForGame())
        return;

    // Check if first vote, if so start counter
    if (m_timeout.load() == std::numeric_limits<int64_t>::max())
    {
        m_timeout.store((int64_t)StkTime::getRealTimeMs() +
            (int64_t)(ServerConfig::m_voting_timeout * 1000.0f));
    }
    int64_t remaining_time =
        m_timeout.load() - (int64_t)StkTime::getRealTimeMs();
    if (remaining_time < 0)
    {
        return;
    }

    NetworkString& data = event->data();
    std::string track_name;
    data.decodeString(&track_name);
    uint8_t lap = data.getUInt8();
    uint8_t reverse = data.getUInt8();

    if (race_manager->modeHasLaps())
    {
        if (ServerConfig::m_auto_lap_ratio > 0.0f)
        {
            Track* t = track_manager->getTrack(track_name);
            if (t)
            {
                lap = (uint8_t)(fmaxf(1.0f,
                    (float)t->getDefaultNumberOfLaps() *
                    ServerConfig::m_auto_lap_ratio));
            }
            else
            {
                // Prevent someone send invalid vote
                track_name = *m_available_kts.second.begin();
                lap = (uint8_t)3;
            }
        }
        else if (lap == 0)
            lap = (uint8_t)3;
    }

    NetworkString other = NetworkString(PROTOCOL_LOBBY_ROOM);
    std::string name = StringUtils::wideToUtf8(event->getPeer()
        ->getPlayerProfiles()[0]->getName());
    other.setSynchronous(true);
    other.addUInt8(LE_VOTE).addFloat(ServerConfig::m_voting_timeout)
        .encodeString(name).addUInt32(event->getPeer()->getHostId())
        .encodeString(track_name).addUInt8(lap).addUInt8(reverse);

    m_peers_votes[event->getPeerSP()] =
        std::make_tuple(track_name, lap, reverse == 1);
    sendMessageToPeers(&other);

}   // playerVote

// ----------------------------------------------------------------------------
std::tuple<std::string, uint8_t, bool, bool> ServerLobby::handleVote()
{
    // Default settings if no votes at all
    RandomGenerator rg;
    std::set<std::string>::iterator it = m_available_kts.second.begin();
    std::advance(it, rg.get((int)m_available_kts.second.size()));
    std::string final_track = *it;
    unsigned final_laps = UserConfigParams::m_num_laps;
    bool final_reverse = final_track.size() % 2 == 0;

    std::map<std::string, unsigned> tracks;
    std::map<unsigned, unsigned> laps;
    std::map<bool, unsigned> reverses;

    float cur_players = 0.0f;
    auto peers = STKHost::get()->getPeers();
    for (auto peer : peers)
    {
        if (peer->hasPlayerProfiles() && !peer->isWaitingForGame())
            cur_players += 1.0f;
    }
    if (cur_players == 0.0f)
        return std::make_tuple(final_track, final_laps, final_reverse, false);
    float tracks_rate = 0.0f;
    float laps_rate = 0.0f;
    float reverses_rate = 0.0f;

    for (auto p : m_peers_votes)
    {
        if (p.first.expired())
            continue;
        auto track_vote = tracks.find(std::get<0>(p.second));
        if (track_vote == tracks.end())
            tracks[std::get<0>(p.second)] = 1;
        else
            track_vote->second++;
        auto lap_vote = laps.find(std::get<1>(p.second));
        if (lap_vote == laps.end())
            laps[std::get<1>(p.second)] = 1;
        else
            lap_vote->second++;
        auto reverse_vote = reverses.find(std::get<2>(p.second));
        if (reverse_vote == reverses.end())
            reverses[std::get<2>(p.second)] = 1;
        else
            reverse_vote->second++;
    }

    unsigned vote = 0;
    auto track_vote = tracks.begin();
    for (auto c_vote = tracks.begin(); c_vote != tracks.end(); c_vote++)
    {
        if (c_vote->second > vote)
        {
            vote = c_vote->second;
            track_vote = c_vote;
        }
    }
    if (track_vote != tracks.end())
    {
        final_track = track_vote->first;
        tracks_rate = float(track_vote->second) / cur_players;
    }

    vote = 0;
    auto lap_vote = laps.begin();
    for (auto c_vote = laps.begin(); c_vote != laps.end(); c_vote++)
    {
        if (c_vote->second > vote)
        {
            vote = c_vote->second;
            lap_vote = c_vote;
        }
    }
    if (lap_vote != laps.end())
    {
        final_laps = lap_vote->first;
        laps_rate = float(lap_vote->second) / cur_players;
    }

    vote = 0;
    auto reverse_vote = reverses.begin();
    for (auto c_vote = reverses.begin(); c_vote != reverses.end(); c_vote++)
    {
        if (c_vote->second > vote)
        {
            vote = c_vote->second;
            reverse_vote = c_vote;
        }
    }
    if (reverse_vote != reverses.end())
    {
        final_reverse = reverse_vote->first;
        reverses_rate = float(reverse_vote->second) / cur_players;
    }

    return std::make_tuple(final_track, final_laps, final_reverse,
        tracks_rate > 0.5f && laps_rate > 0.5f && reverses_rate > 0.5f ?
        true : false);
}   // handleVote

// ----------------------------------------------------------------------------
std::pair<int, float> ServerLobby::getHitCaptureLimit(float num_karts)
{
    // Read user_config.hpp for formula
    int hit_capture_limit = std::numeric_limits<int>::max();
    float time_limit = 0.0f;
    if (race_manager->getMajorMode() ==
        RaceManager::MAJOR_MODE_CAPTURE_THE_FLAG)
    {
        if (ServerConfig::m_capture_limit_threshold > 0.0f)
        {
            float val = fmaxf(3.0f, num_karts *
                ServerConfig::m_capture_limit_threshold);
            hit_capture_limit = (int)val;
        }
        if (ServerConfig::m_time_limit_threshold_ctf > 0.0f)
        {
            time_limit = fmaxf(2.0f, num_karts *
                (ServerConfig::m_time_limit_threshold_ctf +
                ServerConfig::m_flag_return_timemout / 60.f) * 60.0f);
        }
    }
    else
    {
        if (ServerConfig::m_hit_limit_threshold > 0.0f)
        {
            float val = fminf(num_karts *
                ServerConfig::m_hit_limit_threshold, 40.0f);
            hit_capture_limit = (int)val;
            if (hit_capture_limit == 0)
                hit_capture_limit = 1;
        }
        if (ServerConfig::m_time_limit_threshold_ffa > 0.0f)
        {
            time_limit = fmaxf(num_karts *
                ServerConfig::m_time_limit_threshold_ffa, 3.0f) * 60.0f;
        }
    }
    return std::make_pair(hit_capture_limit, time_limit);
}   // getHitCaptureLimit

// ----------------------------------------------------------------------------
/** Called from the RaceManager of the server when the world is loaded. Marks
 *  the server to be ready to start the race.
 */
void ServerLobby::finishedLoadingWorld()
{
    m_server_has_loaded_world.store(true);
}   // finishedLoadingWorld;

//-----------------------------------------------------------------------------
/** Called when a client notifies the server that it has loaded the world.
 *  When all clients and the server are ready, the race can be started.
 */
void ServerLobby::finishedLoadingWorldClient(Event *event)
{
    std::shared_ptr<STKPeer> peer = event->getPeerSP();
    m_peers_ready.at(peer) = true;
    Log::info("ServerLobby", "Peer %d has finished loading world at %lf",
        peer->getHostId(), StkTime::getRealTime());
}   // finishedLoadingWorldClient

//-----------------------------------------------------------------------------
/** Called when a client clicks on 'ok' on the race result screen.
 *  If all players have clicked on 'ok', go back to the lobby.
 */
void ServerLobby::playerFinishedResult(Event *event)
{
    if (m_state.load() != RESULT_DISPLAY)
        return;
    std::shared_ptr<STKPeer> peer = event->getPeerSP();
    m_peers_ready.at(peer) = true;
}   // playerFinishedResult

//-----------------------------------------------------------------------------
void ServerLobby::updateBanList()
{
    m_ip_ban_list.clear();
    m_online_id_ban_list.clear();

    for (auto& ban : ServerConfig::m_server_ip_ban_list)
    {
        if (ban.first == "0.0.0.0/0" ||
            (uint32_t)StkTime::getTimeSinceEpoch() > ban.second)
            continue;
        uint32_t netbits = 0;
        std::vector<std::string> ip_and_netbits =
            StringUtils::split(ban.first, '/');
        if (ip_and_netbits.size() != 2 ||
            !StringUtils::fromString(ip_and_netbits[1], netbits) ||
            netbits > 32)
        {
            Log::error("STKHost", "Wrong CIDR: %s", ban.first.c_str());
            continue;
        }
        TransportAddress addr(ip_and_netbits[0]);
        if (addr.getIP() == 0)
        {
            Log::error("STKHost", "Wrong CIDR: %s", ban.first.c_str());
            continue;
        }
        uint32_t mask = ~((1 << (32 - netbits)) - 1);
        uint32_t ip_start = addr.getIP() & mask;
        uint32_t ip_end = (addr.getIP() & mask) | ~mask;
        m_ip_ban_list[ip_start] =
            std::make_tuple(ip_end, ban.first, ban.second);
    }

    std::map<std::string, uint32_t> final_ip_ban_list;
    for (auto it = m_ip_ban_list.begin();
        it != m_ip_ban_list.end();)
    {
        auto next_itr = std::next(it);
        if (next_itr != m_ip_ban_list.end() &&
            next_itr->first <= std::get<0>(it->second))
        {
            Log::warn("ServerLobby", "%s overlaps %s, removing the first one.",
                std::get<1>(next_itr->second).c_str(),
                std::get<1>(it->second).c_str());
            m_ip_ban_list.erase(next_itr);
            continue;
        }
        final_ip_ban_list[std::get<1>(it->second)] =
            ServerConfig::m_server_ip_ban_list.at(std::get<1>(it->second));
        it++;
    }
    ServerConfig::m_server_ip_ban_list = final_ip_ban_list;
    // Default guided entry
    ServerConfig::m_server_ip_ban_list["0.0.0.0/0"] = 0;

    std::map<uint32_t, uint32_t> final_online_id_ban_list;
    for (auto& ban : ServerConfig::m_server_online_id_ban_list)
    {
        if (ban.first == 0 ||
            (uint32_t)StkTime::getTimeSinceEpoch() > ban.second)
            continue;
        m_online_id_ban_list[ban.first] = ban.second;
        final_online_id_ban_list[ban.first] =
            ServerConfig::m_server_online_id_ban_list.at(ban.first);
    }
    ServerConfig::m_server_online_id_ban_list = final_online_id_ban_list;
    ServerConfig::m_server_online_id_ban_list[0] = 0;
}   // updateBanList

//-----------------------------------------------------------------------------
bool ServerLobby::waitingForPlayers() const
{
    if (m_game_setup->isGrandPrix() && m_game_setup->isGrandPrixStarted())
        return false;
    return m_state.load() >= WAITING_FOR_START_GAME;
}   // waitingForPlayers

//-----------------------------------------------------------------------------
void ServerLobby::handlePendingConnection()
{
    std::lock_guard<std::mutex> lock(m_keys_mutex);

    for (auto it = m_pending_connection.begin();
         it != m_pending_connection.end();)
    {
        auto peer = it->first.lock();
        if (!peer)
        {
            it = m_pending_connection.erase(it);
        }
        else
        {
            const uint32_t online_id = it->second.first;
            auto key = m_keys.find(online_id);
            if (key != m_keys.end() && key->second.m_tried == false)
            {
                try
                {
                    if (decryptConnectionRequest(peer, it->second.second,
                        key->second.m_aes_key, key->second.m_aes_iv, online_id,
                        key->second.m_name))
                    {
                        it = m_pending_connection.erase(it);
                        m_keys.erase(online_id);
                        continue;
                    }
                    else
                        key->second.m_tried = true;
                }
                catch (std::exception& e)
                {
                    Log::error("ServerLobby",
                        "handlePendingConnection error: %s", e.what());
                    key->second.m_tried = true;
                }
            }
            it++;
        }
    }
}   // handlePendingConnection

//-----------------------------------------------------------------------------
bool ServerLobby::decryptConnectionRequest(std::shared_ptr<STKPeer> peer,
    BareNetworkString& data, const std::string& key, const std::string& iv,
    uint32_t online_id, const core::stringw& online_name)
{
    auto crypto = std::unique_ptr<Crypto>(new Crypto(
        Crypto::decode64(key), Crypto::decode64(iv)));
    if (crypto->decryptConnectionRequest(data))
    {
        peer->setCrypto(std::move(crypto));
        std::lock_guard<std::mutex> lock(m_connection_mutex);
        Log::info("ServerLobby", "%s validated",
            StringUtils::wideToUtf8(online_name).c_str());
        handleUnencryptedConnection(peer, data, online_id,
            online_name);
        return true;
    }
    return false;
}   // decryptConnectionRequest

//-----------------------------------------------------------------------------
void ServerLobby::getRankingForPlayer(std::shared_ptr<NetworkPlayerProfile> p)
{
    Online::XMLRequest* request = new Online::XMLRequest();
    NetworkConfig::get()->setUserDetails(request, "get-ranking");

    const uint32_t id = p->getOnlineId();
    request->addParameter("id", id);
    request->executeNow();

    const XMLNode* result = request->getXMLData();
    std::string rec_success;

    // Default result
    double score = 2000.0;
    double max_score = 2000.0;
    unsigned num_races = 0;
    if (result->get("success", &rec_success))
    {
        if (rec_success == "yes")
        {
            result->get("scores", &score);
            result->get("max-scores", &max_score);
            result->get("num-races-done", &num_races);
        }
        else
        {
            Log::error("ServerLobby", "No ranking info found.");
        }
    }
    else
    {
        Log::error("ServerLobby", "No ranking info found.");
    }
    m_ranked_players[id] = p;
    m_scores[id] = score;
    m_max_scores[id] = max_score;
    m_num_ranked_races[id] = num_races;
    delete request;
}   // getRankingForPlayer

//-----------------------------------------------------------------------------
void ServerLobby::submitRankingsToAddons()
{
    // No ranking for battle mode
    if (!race_manager->modeHasLaps())
        return;

    // ========================================================================
    class SumbitRankingRequest : public Online::XMLRequest
    {
    public:
        SumbitRankingRequest(uint32_t online_id, double scores,
                             double max_scores, unsigned num_races)
            : XMLRequest(true)
        {
            addParameter("id", online_id);
            addParameter("scores", scores);
            addParameter("max-scores", max_scores);
            addParameter("num-races-done", num_races);
        }
        virtual void afterOperation()
        {
            Online::XMLRequest::afterOperation();
            const XMLNode* result = getXMLData();
            std::string rec_success;
            if (!(result->get("success", &rec_success) &&
                rec_success == "yes"))
            {
                Log::error("ServerLobby", "Failed to submit scores.");
            }
        }
    };   // UpdatePlayerRankingRequest
    // ========================================================================

    for (unsigned i = 0; i < race_manager->getNumPlayers(); i++)
    {
        const uint32_t id = race_manager->getKartInfo(i).getOnlineId();
        SumbitRankingRequest* request = new SumbitRankingRequest
            (id, m_scores.at(id), m_max_scores.at(id),
            m_num_ranked_races.at(id));
        NetworkConfig::get()->setUserDetails(request, "submit-ranking");
        Log::info("ServerLobby", "Submiting ranking for %s (%d) : %lf, %lf %d",
            StringUtils::wideToUtf8(
            race_manager->getKartInfo(i).getPlayerName()).c_str(), id,
            m_scores.at(id), m_max_scores.at(id), m_num_ranked_races.at(id));
        request->queue();
    }
}   // submitRankingsToAddons

//-----------------------------------------------------------------------------
/** This function is called when all clients have loaded the world and
 *  are therefore ready to start the race. It determine the start time in
 *  network timer for client and server based on pings and then switches state
 *  to WAIT_FOR_RACE_STARTED.
 */
void ServerLobby::configPeersStartTime()
{
    uint32_t max_ping = 0;
    const unsigned max_ping_from_peers = ServerConfig::m_max_ping;
    for (auto p : m_peers_ready)
    {
        auto peer = p.first.lock();
        if (!peer)
            continue;
        if (peer->getAveragePing() > max_ping_from_peers)
        {
            sendBadConnectionMessageToPeer(peer);
            continue;
        }
        max_ping = std::max(peer->getAveragePing(), max_ping);
    }
    // Start up time will be after 2000ms, so even if this packet is sent late
    // (due to packet loss), the start time will still ahead of current time
    uint64_t start_time = STKHost::get()->getNetworkTimer() + (uint64_t)2000;
    NetworkString* ns = getNetworkString(10);
    ns->addUInt8(LE_START_RACE).addUInt64(start_time);
    sendMessageToPeers(ns, /*reliable*/true);

    const unsigned jitter_tolerance = ServerConfig::m_jitter_tolerance;
    Log::info("ServerLobby", "Max ping from peers: %d, jitter tolerance: %d",
        max_ping, jitter_tolerance);
    // Delay server for max ping / 2 from peers and jitter tolerance.
    m_server_delay = (uint64_t)(max_ping / 2) + (uint64_t)jitter_tolerance;
    start_time += m_server_delay;
    m_server_started_at = start_time;
    delete ns;
    m_state = WAIT_FOR_RACE_STARTED;

    joinStartGameThread();
    m_start_game_thread = std::thread([start_time, this]()
        {
            const uint64_t cur_time = STKHost::get()->getNetworkTimer();
            assert(start_time > cur_time);
            int sleep_time = (int)(start_time - cur_time);
            Log::info("ServerLobby", "Start game after %dms", sleep_time);
            std::this_thread::sleep_for(std::chrono::milliseconds(sleep_time));
            Log::info("ServerLobby", "Started at %lf", StkTime::getRealTime());
            m_state.store(RACING);
        });
}   // configPeersStartTime

//-----------------------------------------------------------------------------
bool ServerLobby::allowJoinedPlayersWaiting() const
{
    return !m_game_setup->isGrandPrix();
}   // allowJoinedPlayersWaiting

//-----------------------------------------------------------------------------
void ServerLobby::updateWaitingPlayers()
{
    // addWaitingPlayersToGame below will be called by main thread in case
    // resetServer
    std::lock_guard<std::mutex> lock(m_connection_mutex);

    m_waiting_players.erase(std::remove_if(
        m_waiting_players.begin(), m_waiting_players.end(), []
        (const std::weak_ptr<NetworkPlayerProfile>& npp)->bool
        {
            return npp.expired();
        }), m_waiting_players.end());
    m_waiting_players_counts.store((uint32_t)m_waiting_players.size());
}   // updateWaitingPlayers

//-----------------------------------------------------------------------------
void ServerLobby::addWaitingPlayersToGame()
{
    if (m_waiting_players.empty())
        return;
    for (auto& p : m_waiting_players)
    {
        auto npp = p.lock();
        if (!npp)
            continue;
        auto peer = npp->getPeer();
        assert(peer);
        uint32_t online_id = npp->getOnlineId();
        if (ServerConfig::m_ranked)
        {
            bool duplicated_ranked_player =
                m_ranked_players.find(online_id) != m_ranked_players.end() &&
                !m_ranked_players.at(online_id).expired();
            if (duplicated_ranked_player)
            {
                NetworkString* message = getNetworkString(2);
                message->setSynchronous(true);
                message->addUInt8(LE_CONNECTION_REFUSED)
                    .addUInt8(RR_INVALID_PLAYER);
                peer->sendPacket(message, true/*reliable*/);
                peer->reset();
                delete message;
                Log::verbose("ServerLobby", "Player refused: invalid player");
                continue;
            }
        }

        peer->setWaitingForGame(false);
        m_peers_ready[peer] = false;
        m_game_setup->addPlayer(npp);
        Log::info("ServerLobby", "New player %s with online id %u from %s.",
            StringUtils::wideToUtf8(npp->getName()).c_str(),
            npp->getOnlineId(), peer->getAddress().toString().c_str());

        if (ServerConfig::m_ranked)
        {
            getRankingForPlayer(peer->getPlayerProfiles()[0]);
        }
    }
    m_waiting_players.clear();
    m_waiting_players_counts.store(0);
}   // addWaitingPlayersToGame

//-----------------------------------------------------------------------------
void ServerLobby::resetServer()
{
    addWaitingPlayersToGame();
    m_state = NetworkConfig::get()->isLAN() ?
        WAITING_FOR_START_GAME : REGISTER_SELF_ADDRESS;
    updatePlayerList(true/*update_when_reset_server*/);
    NetworkString* server_info = getNetworkString();
    server_info->setSynchronous(true);
    server_info->addUInt8(LE_SERVER_INFO);
    m_game_setup->addServerInfo(server_info);
    sendMessageToPeersInServer(server_info);
    delete server_info;
    setup();
}   // resetServer

//-----------------------------------------------------------------------------
bool ServerLobby::isBannedForIP(const TransportAddress& addr) const
{
    uint32_t ip_decimal = addr.getIP();
    auto lb = m_ip_ban_list.lower_bound(addr.getIP());
    bool is_banned = false;
    if (lb != m_ip_ban_list.end() && ip_decimal >= lb->first/*ip_start*/)
    {
        if (ip_decimal <= std::get<0>(lb->second)/*ip_end*/ &&
            (uint32_t)StkTime::getTimeSinceEpoch() < std::get<2>(lb->second))
            is_banned = true;
    }
    else if (lb != m_ip_ban_list.begin())
    {
        lb--;
        if (ip_decimal>= lb->first/*ip_start*/ &&
            ip_decimal <= std::get<0>(lb->second)/*ip_end*/ &&
            (uint32_t)StkTime::getTimeSinceEpoch() < std::get<2>(lb->second))
            is_banned = true;
    }
    if (is_banned)
    {
        Log::info("ServerLobby", "%s is banned by CIDR %s",
            addr.toString(false/*show_port*/).c_str(),
            std::get<1>(lb->second).c_str());
    }
    return is_banned;
}   // isBannedForIP

//-----------------------------------------------------------------------------
float ServerLobby::getStartupBoostOrPenaltyForKart(uint32_t ping,
                                                   unsigned kart_id)
{
    AbstractKart* k = World::getWorld()->getKart(kart_id);
    if (k->getStartupBoost() != 0.0f)
        return k->getStartupBoost();
    uint64_t now = STKHost::get()->getNetworkTimer();
    uint64_t client_time = now - ping / 2;
    uint64_t server_time = client_time + m_server_delay;
    int ticks = stk_config->time2Ticks(
        (float)(server_time - m_server_started_at) / 1000.0f);
    if (ticks < stk_config->time2Ticks(1.0f))
    {
        PlayerController* pc =
            dynamic_cast<PlayerController*>(k->getController());
        pc->displayPenaltyWarning();
        return -1.0f;
    }
    float f = k->getStartupBoostFromStartTicks(ticks);
    k->setStartupBoost(f);
    return f;
}   // getStartupBoostOrPenaltyForKart
