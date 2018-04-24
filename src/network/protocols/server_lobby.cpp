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
#include "karts/kart_properties_manager.hpp"
#include "modes/linear_world.hpp"
#include "network/event.hpp"
#include "network/game_setup.hpp"
#include "network/network_config.hpp"
#include "network/network_player_profile.hpp"
#include "network/protocols/connect_to_peer.hpp"
#include "network/protocols/game_protocol.hpp"
#include "network/protocols/game_events_protocol.hpp"
#include "network/race_event_manager.hpp"
#include "network/stk_host.hpp"
#include "network/stk_peer.hpp"
#include "online/online_profile.hpp"
#include "online/request_manager.hpp"
#include "race/race_manager.hpp"
#include "states_screens/networking_lobby.hpp"
#include "states_screens/race_result_gui.hpp"
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
 "Server Constructor" -> "ACCEPTING_CLIENTS" [label="If LAN game"]
 "INIT_WAN" -> "GETTING_PUBLIC_ADDRESS" [label="GetPublicAddress protocol callback"]
 "GETTING_PUBLIC_ADDRESS" -> "ACCEPTING_CLIENTS" [label="Register server"]
 "ACCEPTING_CLIENTS" -> "connectionRequested" [label="Client connection request"]
 "connectionRequested" -> "ACCEPTING_CLIENTS" 
 "ACCEPTING_CLIENTS" -> "SELECTING" [label="Start race from authorised client"]
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
    m_has_created_server_id_file = false;
    setHandleDisconnections(true);
    m_state = SET_PUBLIC_ADDRESS;
    updateBanList();
}   // ServerLobby

//-----------------------------------------------------------------------------
/** Destructor.
 */
ServerLobby::~ServerLobby()
{
    if (m_server_registered)
    {
        unregisterServer();
    }
}   // ~ServerLobby

//-----------------------------------------------------------------------------
void ServerLobby::setup()
{
    LobbyProtocol::setup();
    if (m_game_setup->isGrandPrix() && !m_game_setup->isGrandPrixStarted())
    {
        auto players = m_game_setup->getConnectedPlayers();
        for (auto player : players)
            player->resetGrandPrixData();
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

    m_server_registered = false;
    m_server_has_loaded_world.store(false);

    // Initialise the data structures to detect if all clients and 
    // the server are ready:
    resetPeersReady();
    m_peers_votes.clear();
    m_server_delay = 0.0;
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
    case LE_REQUEST_BEGIN: startSelection(event); break;
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
    if (!event->getPeer()->hasPlayerProfiles())
    {
        Log::warn("ServerLobby", "Unauthorized peer wants to chat.");
        return;
    }
    core::stringw message;
    event->data().decodeStringW(&message);
    if (message.size() > 0)
    {
        NetworkString* chat = getNetworkString();
        chat->setSynchronous(true);
        chat->addUInt8(LE_CHAT).encodeString(message);
        sendMessageToPeersChangingToken(chat, /*reliable*/true);
        delete chat;
    }
}   // handleChat

//-----------------------------------------------------------------------------
void ServerLobby::kickHost(Event* event)
{
    std::unique_lock<std::mutex> lock(m_connection_mutex);
    if (m_server_owner.lock() != event->getPeerSP())
        return;
    lock.unlock();
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
        case LE_STARTED_RACE:  startedRaceOnClient(event);        break;
        case LE_VOTE: playerVote(event);                          break;
        case LE_KICK_HOST: kickHost(event);                       break;
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
    const std::string& sid = NetworkConfig::get()->getServerIdFile();
    if (!sid.empty() && !m_has_created_server_id_file)
    {
        std::fstream fs;
        fs.open(sid, std::ios::out);
        fs.close();
        m_has_created_server_id_file = true;
    }
}   // createServerIdFile

//-----------------------------------------------------------------------------
/** Find out the public IP server or poll STK server asynchronously. */
void ServerLobby::asynchronousUpdate()
{
    switch (m_state.load())
    {
    case SET_PUBLIC_ADDRESS:
    {
        // In case of LAN we don't need our public address or register with the
        // STK server, so we can directly go to the accepting clients state.
        if (NetworkConfig::get()->isLAN())
        {
            m_state = ACCEPTING_CLIENTS;
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
        if (m_game_setup->isGrandPrixStarted())
        {
            m_state = ACCEPTING_CLIENTS;
            break;
        }
        // Register this server with the STK server. This will block
        // this thread, but there is no need for the protocol manager
        // to react to any requests before the server is registered.
        registerServer();
        if (m_server_registered)
        {
            m_state = ACCEPTING_CLIENTS;
            createServerIdFile();
        }
        break;
    }
    case ACCEPTING_CLIENTS:
    {
        // Only poll the STK server if this is a WAN server.
        if (NetworkConfig::get()->isWAN())
            checkIncomingConnectionRequests();
        break;
    }
    case ERROR_LEAVE:
    {
        requestTerminate();
        m_state = EXITING;
        STKHost::get()->setErrorMessage(_("Failed to setup server."));
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
        m_state = WAIT_FOR_RACE_STARTED;
        // Reset for next state usage
        resetPeersReady();
        signalRaceStartToClients();
        break;
    }
    case WAIT_FOR_RACE_STARTED:
        // The function startedRaceOnClient() will trigger the
        // next state.
        break;
    case DELAY_SERVER:
        if (m_server_delay < StkTime::getRealTime())
        {
            Log::verbose("ServerLobby", "End delay at %lf",
                         StkTime::getRealTime());
            m_state = RACING;
            World::getWorld()->setReadyToRace();
        }
        break;
    case SELECTING:
        if (m_timeout.load() < (float)StkTime::getRealTime())
        {
            std::lock_guard<std::mutex> lock(m_connection_mutex);
            auto result = handleVote();
            // Remove disconnected player (if any) one last time
            m_game_setup->update(true);
            m_game_setup->sortPlayersForGrandPrix();
            auto players = m_game_setup->getConnectedPlayers();
            NetworkString* load_world = getNetworkString();
            load_world->setSynchronous(true);
            load_world->addUInt8(LE_LOAD_WORLD).encodeString(std::get<0>(result))
                .addUInt8(std::get<1>(result)).addUInt8(std::get<2>(result))
                .addUInt8((uint8_t)players.size());
            for (auto player : players)
            {
                load_world->encodeString(player->getName())
                    .addUInt32(player->getHostId())
                    .addFloat(player->getDefaultKartColor())
                    .addUInt32(player->getOnlineId())
                    .addUInt8(player->getPerPlayerDifficulty())
                    .addUInt8(player->getLocalPlayerId());
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
            configRemoteKart(players);

            // Reset for next state usage
            resetPeersReady();
            m_state = LOAD_WORLD;
            sendMessageToPeersChangingToken(load_world);
            delete load_world;
        }
        break;
    default:
        break;
    }

}   // asynchronousUpdate

//-----------------------------------------------------------------------------
/** Simple finite state machine.  Once this
 *  is known, register the server and its address with the stk server so that
 *  client can find it.
 */
void ServerLobby::update(int ticks)
{
    // Reset server to initial state if no more connected players
    if ((m_state.load() > ACCEPTING_CLIENTS ||
        m_game_setup->isGrandPrixStarted()) &&
        STKHost::get()->getPeerCount() == 0 &&
        NetworkConfig::get()->getServerIdFile().empty())
    {
        if (RaceEventManager::getInstance() &&
            RaceEventManager::getInstance()->isRunning())
        {
            stopCurrentRace();
        }
        std::lock_guard<std::mutex> lock(m_connection_mutex);
        m_game_setup->stopGrandPrix();
        m_state = NetworkConfig::get()->isLAN() ?
            ACCEPTING_CLIENTS : REGISTER_SELF_ADDRESS;
        setup();
    }

    // Check if server owner has left
    updateServerOwner();
    if (m_game_setup)
    {
        // Remove disconnected players if in these two states
        m_game_setup->update(m_state.load() == ACCEPTING_CLIENTS ||
            m_state.load() == SELECTING);
    }
    switch (m_state.load())
    {
    case SET_PUBLIC_ADDRESS:
    case REGISTER_SELF_ADDRESS:
    case ACCEPTING_CLIENTS:
    case WAIT_FOR_WORLD_LOADED:
    case WAIT_FOR_RACE_STARTED:
    case DELAY_SERVER:
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
    case RESULT_DISPLAY:
        if (checkPeersReady() ||
            StkTime::getRealTime() > m_timeout.load())
        {
            // Send a notification to all clients to exit
            // the race result screen
            NetworkString *exit_result_screen = getNetworkString(1);
            exit_result_screen->setSynchronous(true);
            exit_result_screen->addUInt8(LE_EXIT_RESULT);
            sendMessageToPeersChangingToken(exit_result_screen,
                                            /*reliable*/true);
            delete exit_result_screen;
            std::lock_guard<std::mutex> lock(m_connection_mutex);
            m_state = NetworkConfig::get()->isLAN() ?
                ACCEPTING_CLIENTS : REGISTER_SELF_ADDRESS;
            updatePlayerList();
            NetworkString* server_info = getNetworkString();
            server_info->setSynchronous(true);
            server_info->addUInt8(LE_SERVER_INFO);
            m_game_setup->addServerInfo(server_info);
            sendMessageToPeersChangingToken(server_info);
            delete server_info;
            setup();
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
void ServerLobby::registerServer()
{
    Online::XMLRequest *request = new Online::XMLRequest();
    NetworkConfig::get()->setUserDetails(request, "create");
    request->addParameter("address",      m_server_address.getIP()        );
    request->addParameter("port",         m_server_address.getPort()      );
    request->addParameter("private_port",
                                    STKHost::get()->getPrivatePort()      );
    request->addParameter("name",   NetworkConfig::get()->getServerName() );
    request->addParameter("max_players",
        NetworkConfig::get()->getMaxPlayers());
    request->addParameter("difficulty", race_manager->getDifficulty());
    request->addParameter("game_mode", NetworkConfig::get()->getServerMode());
    request->addParameter("password",
        (unsigned)(!NetworkConfig::get()->getPassword().empty()));
    request->addParameter("version",
        (unsigned)NetworkConfig::m_server_version);

    Log::info("ServerLobby", "Public server address %s",
        m_server_address.toString().c_str());

    request->executeNow();

    const XMLNode * result = request->getXMLData();
    std::string rec_success;

    if (result->get("success", &rec_success) && rec_success == "yes")
    {
        Log::info("ServerLobby", "Server is now online.");
        m_server_registered = true;
    }
    else
    {
        irr::core::stringc error(request->getInfo().c_str());
        Log::error("ServerLobby", "%s", error.c_str());
        m_server_registered = false;
        m_state = ERROR_LEAVE;
    }
    delete request;
}   // registerServer

//-----------------------------------------------------------------------------
/** Unregister this server (i.e. its public address) with the STK server,
 *  currently when karts enter kart selection screen it will be done.
 */
void ServerLobby::unregisterServer()
{
    Online::XMLRequest* request = new Online::XMLRequest();
    NetworkConfig::get()->setUserDetails(request, "stop");

    request->addParameter("address", m_server_address.getIP());
    request->addParameter("port", m_server_address.getPort());
    Log::info("ServerLobby", "Unregister server address %s",
        m_server_address.toString().c_str());
    request->executeNow();

    const XMLNode * result = request->getXMLData();
    std::string rec_success;

    if (result->get("success", &rec_success))
    {
        if (rec_success == "yes")
        {
            Log::info("ServerLobby", "Server is now unregister.");
        }
        else
        {
            Log::error("ServerLobby", "Fail to unregister server.");
        }
    }
    else
    {
        Log::error("ServerLobby", "Fail to stop server.");
    }
    delete request;

}   // unregisterServer

//-----------------------------------------------------------------------------
/** This function is called when all clients have loaded the world and
 *  are therefore ready to start the race. It signals to all clients
 *  to start the race and then switches state to DELAY_SERVER.
 */
void ServerLobby::signalRaceStartToClients()
{
    Log::verbose("Server", "Signaling race start to clients at %lf",
                 StkTime::getRealTime());
    NetworkString *ns = getNetworkString(1);
    ns->addUInt8(LE_START_RACE);
    sendMessageToPeersChangingToken(ns, /*reliable*/true);
    delete ns;
}   // startGame

//-----------------------------------------------------------------------------
/** Instructs all clients to start the kart selection. If event is not NULL,
 *  the command comes from a client (which needs to be authorised).
 */
void ServerLobby::startSelection(const Event *event)
{
    std::lock_guard<std::mutex> lock(m_connection_mutex);

    if (m_state != ACCEPTING_CLIENTS)
    {
        Log::warn("ServerLobby",
                  "Received startSelection while being in state %d",
                  m_state.load());
        return;
    }
    if (event->getPeerSP() != m_server_owner.lock())
    {
        Log::warn("ServerLobby", 
                  "Client %lx is not authorised to start selection.",
                  event->getPeer());
        return;
    }

    if (m_server_registered)
    {
        unregisterServer();
        m_server_registered = false;
    }

    NetworkString *ns = getNetworkString(1);
    // Start selection - must be synchronous since the receiver pushes
    // a new screen, which must be done from the main thread.
    ns->setSynchronous(true);
    ns->addUInt8(LE_START_SELECTION).addUInt8(
        m_game_setup->isGrandPrixStarted() ? 1 : 0);

    // Remove karts / tracks from server that are not supported on all clients
    std::set<std::string> karts_erase, tracks_erase;
    auto peers = STKHost::get()->getPeers();
    for (auto peer : peers)
    {
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

    sendMessageToPeersChangingToken(ns, /*reliable*/true);
    delete ns;

    m_state = SELECTING;

    // Will be changed after the first vote received
    m_timeout.store(std::numeric_limits<float>::max());
}   // startSelection

//-----------------------------------------------------------------------------
/** Query the STK server for connection requests. For each connection request
 *  start a ConnectToPeer protocol.
 */
void ServerLobby::checkIncomingConnectionRequests()
{
    // First poll every 5 seconds. Return if no polling needs to be done.
    const float POLL_INTERVAL = 5.0f;
    static double last_poll_time = 0;
    if (StkTime::getRealTime() < last_poll_time + POLL_INTERVAL)
        return;

    // Keep the port open, it can be sent to anywhere as we will send to the
    // correct peer later in ConnectToPeer.
    BareNetworkString data;
    data.addUInt8(0);
    STKHost::get()->sendRawPacket(data, STKHost::get()->getStunAddress());

    // Now poll the stk server
    last_poll_time = StkTime::getRealTime();
    Online::XMLRequest* request = new Online::XMLRequest();
    NetworkConfig::get()->setUserDetails(request, "poll-connection-requests");

    const TransportAddress &addr = STKHost::get()->getPublicAddress();
    request->addParameter("address", addr.getIP()  );
    request->addParameter("port",    addr.getPort());
    request->addParameter("current_players", m_game_setup->getPlayerCount());

    request->executeNow();
    assert(request->isDone());

    const XMLNode *result = request->getXMLData();
    std::string success;

    if (!result->get("success", &success) || success != "yes")
    {
        Log::error("ServerLobby", "Cannot retrieve the list.");
        return;
    }

    // Now start a ConnectToPeer protocol for each connection request
    const XMLNode * users_xml = result->getNode("users");
    uint32_t id = 0;
    for (unsigned int i = 0; i < users_xml->getNumNodes(); i++)
    {
        users_xml->getNode(i)->get("id", &id);
        Log::debug("ServerLobby",
                   "User with id %d wants to connect.", id);
        std::make_shared<ConnectToPeer>(id)->requestStart();
    }
    delete request;
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

    // Reset for next state usage
    resetPeersReady();
    NetworkString* total = getNetworkString();
    total->setSynchronous(true);
    total->addUInt8(LE_RACE_FINISHED);
    if (m_game_setup->isGrandPrix())
    {
        // fastest lap
        int fastest_lap =
            static_cast<LinearWorld*>(World::getWorld())->getFastestLapTicks();
        total->addUInt32(fastest_lap);

        // all gp tracks
        total->addUInt8((uint8_t)m_game_setup->getTotalGrandPrixTracks())
            .addUInt8((uint8_t)m_game_setup->getAllTracks().size());
        for (const std::string& gp_track : m_game_setup->getAllTracks())
            total->encodeString(gp_track);

        // each kart score and total time
        auto& players = m_game_setup->getPlayers();
        total->addUInt8((uint8_t)players.size());
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
            total->addUInt32(last_score).addUInt32(cur_score)
                .addFloat(overall_time);            
        }
    }
    else if (race_manager->modeHasLaps())
    {
        int fastest_lap =
            static_cast<LinearWorld*>(World::getWorld())->getFastestLapTicks();
        total->addUInt32(fastest_lap);
    }

    stopCurrentRace();
    // Set the delay before the server forces all clients to exit the race
    // result screen and go back to the lobby
    m_timeout.store((float)StkTime::getRealTime() + 15.0f);
    m_state = RESULT_DISPLAY;
    sendMessageToPeersChangingToken(total, /*reliable*/ true);
    delete total;
    Log::info("ServerLobby", "End of game message sent");

}   // checkRaceFinished

//-----------------------------------------------------------------------------
/** Stop any race currently in server, should only be called in main thread.
 */
void ServerLobby::stopCurrentRace()
{
    // notify the network world that it is stopped
    RaceEventManager::getInstance()->stop();

    // stop race protocols before going back to lobby (end race)
    RaceEventManager::getInstance()->getProtocol()->requestTerminate();
    GameProtocol::lock()->requestTerminate();

    while (!RaceEventManager::getInstance()->protocolStopped())
        StkTime::sleep(1);
    while (!GameProtocol::emptyInstance())
        StkTime::sleep(1);

    // This will go back to lobby in server (and exit the current race)
    RaceResultGUI::getInstance()->backToLobby();
}   // stopCurrentRace

//-----------------------------------------------------------------------------
/** Called when a client disconnects.
 *  \param event The disconnect event.
 */
void ServerLobby::clientDisconnected(Event* event)
{
    std::lock_guard<std::mutex> lock(m_connection_mutex);
    auto players_on_peer = event->getPeer()->getPlayerProfiles();
    if (players_on_peer.empty())
        return;

    NetworkString* msg = getNetworkString(2);
    msg->addUInt8(LE_PLAYER_DISCONNECTED);
    msg->addUInt8((uint8_t)players_on_peer.size());
    for (auto p : players_on_peer)
    {
        std::string name = StringUtils::wideToUtf8(p->getName());
        msg->encodeString(name);
        Log::info("ServerLobby", "%s disconnected", name.c_str());
    }
    sendMessageToPeersChangingToken(msg, /*reliable*/true);
    updatePlayerList();
    delete msg;
}   // clientDisconnected

//-----------------------------------------------------------------------------

/*! \brief Called when a player asks for a connection.
 *  \param event : Event providing the information.
 *
 *  Format of the data :
 *  Byte 0   1                
 *       ---------------------
 *  Size | 1 |1|             |
 *  Data | 4 |n| player name |
 *       ---------------------
 */
void ServerLobby::connectionRequested(Event* event)
{
    std::lock_guard<std::mutex> lock(m_connection_mutex);
    std::shared_ptr<STKPeer> peer = event->getPeerSP();
    peer->cleanPlayerProfiles();

    const NetworkString &data = event->data();

    // can we add the player ?
    if (m_state != ACCEPTING_CLIENTS ||
        m_game_setup->isGrandPrixStarted())
    {
        NetworkString *message = getNetworkString(2);
        message->addUInt8(LE_CONNECTION_REFUSED).addUInt8(RR_BUSY);
        // send only to the peer that made the request and disconect it now
        peer->sendPacket(message);
        peer->reset();
        delete message;
        Log::verbose("ServerLobby", "Player refused: selection started");
        return;
    }

    // Check server version
    int version = data.getUInt8();
    if (version < stk_config->m_max_server_version ||
        version > stk_config->m_max_server_version)
    {
        NetworkString *message = getNetworkString(2);
        message->addUInt8(LE_CONNECTION_REFUSED)
                .addUInt8(RR_INCOMPATIBLE_DATA);
        peer->sendPacket(message);
        peer->reset();
        delete message;
        Log::verbose("ServerLobby", "Player refused: wrong server version");
        return;
    }

    // Check for password
    std::string password;
    data.decodeString(&password);
    if (password != NetworkConfig::get()->getPassword())
    {
        NetworkString *message = getNetworkString(2);
        message->addUInt8(LE_CONNECTION_REFUSED)
                .addUInt8(RR_INCORRECT_PASSWORD);
        peer->sendPacket(message);
        peer->reset();
        delete message;
        Log::verbose("ServerLobby", "Player refused: incorrect password");
        return;
    }

    unsigned player_count = data.getUInt8();
    if (m_game_setup->getPlayerCount() + player_count >
        NetworkConfig::get()->getMaxPlayers())
    {
        NetworkString *message = getNetworkString(2);
        message->addUInt8(LE_CONNECTION_REFUSED).addUInt8(RR_TOO_MANY_PLAYERS);
        peer->sendPacket(message);
        peer->reset();
        delete message;
        Log::verbose("ServerLobby", "Player refused: too many players");
        return;
    }

    for (unsigned i = 0; i < player_count; i++)
    {
        std::string name_u8;
        data.decodeString(&name_u8);
        core::stringw name = StringUtils::utf8ToWide(name_u8);
        float default_kart_color = data.getFloat();
        uint32_t online_id = data.getUInt32();
        PerPlayerDifficulty per_player_difficulty =
            (PerPlayerDifficulty)data.getUInt8();
        peer->addPlayer(std::make_shared<NetworkPlayerProfile>
            (peer, name, peer->getHostId(), default_kart_color, online_id,
            per_player_difficulty, (uint8_t)i));
    }

    bool is_banned = false;
    auto ret = m_ban_list.find(peer->getAddress().getIP());
    if (ret != m_ban_list.end())
    {
        // Ban all players
        if (ret->second == 0)
        {
            is_banned = true;
        }
        else
        {
            for (auto& p : peer->getPlayerProfiles())
            {
                if (ret->second == p->getOnlineId())
                {
                    is_banned = true;
                    break;
                }
            }
        }
    }

    if (is_banned)
    {
        NetworkString *message = getNetworkString(2);
        message->addUInt8(LE_CONNECTION_REFUSED).addUInt8(RR_BANNED);
        peer->cleanPlayerProfiles();
        peer->sendPacket(message);
        peer->reset();
        delete message;
        Log::verbose("ServerLobby", "Player refused: banned");
        return;
    }

    // Connection accepted.
    // ====================
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
        message->addUInt8(LE_CONNECTION_REFUSED)
            .addUInt8(RR_INCOMPATIBLE_DATA);
        peer->cleanPlayerProfiles();
        peer->sendPacket(message);
        peer->reset();
        delete message;
        Log::verbose("ServerLobby", "Player has incompatible karts / tracks");
        return;
    }

    // Save available karts and tracks from clients in STKPeer so if this peer
    // disconnects later in lobby it won't affect current players
    peer->setAvailableKartsTracks(client_karts, client_tracks);

    if (!peer->isClientServerTokenSet())
    {
        // Now answer to the peer that just connected
        // ------------------------------------------
        RandomGenerator token_generator;
        // use 4 random numbers because rand_max is probably 2^15-1.
        uint32_t token = (uint32_t)((token_generator.get(RAND_MAX) & 0xff) << 24 |
                                    (token_generator.get(RAND_MAX) & 0xff) << 16 |
                                    (token_generator.get(RAND_MAX) & 0xff) <<  8 |
                                    (token_generator.get(RAND_MAX) & 0xff));

        peer->setClientServerToken(token);
    }

    // send a message to the one that asked to connect
    NetworkString* message_ack = getNetworkString(4);
    message_ack->setSynchronous(true);
    // connection success -- return the host id of peer
    message_ack->addUInt8(LE_CONNECTION_ACCEPTED).addUInt32(peer->getHostId());
    peer->sendPacket(message_ack);
    delete message_ack;

    NetworkString* server_info = getNetworkString();
    server_info->setSynchronous(true);
    server_info->addUInt8(LE_SERVER_INFO);
    m_game_setup->addServerInfo(server_info);
    peer->sendPacket(server_info);
    delete server_info;

    m_peers_ready[peer] = false;
    for (std::shared_ptr<NetworkPlayerProfile> npp : peer->getPlayerProfiles())
    {
        m_game_setup->addPlayer(npp);
        Log::info("ServerLobby", "New player %s with online id %u from %s.",
            StringUtils::wideToUtf8(npp->getName()).c_str(),
            npp->getOnlineId(), peer->getAddress().toString().c_str());
    }
    updatePlayerList();
}   // connectionRequested

//-----------------------------------------------------------------------------
void ServerLobby::updatePlayerList()
{
    if (m_state.load() > ACCEPTING_CLIENTS)
        return;
    auto all_profiles = STKHost::get()->getAllPlayerProfiles();
    NetworkString* pl = getNetworkString();
    pl->setSynchronous(true);
    pl->addUInt8(LE_UPDATE_PLAYER_LIST).addUInt8((uint8_t)all_profiles.size());
    for (auto profile : all_profiles)
    {
        pl->addUInt32(profile->getHostId()).addUInt32(profile->getOnlineId())
            .encodeString(profile->getName());
        uint8_t server_owner = 0;
        if (m_server_owner.lock() == profile->getPeer())
            server_owner = 1;
        pl->addUInt8(server_owner);
    }
    sendMessageToPeersChangingToken(pl);
    delete pl;
}   // updatePlayerList

//-----------------------------------------------------------------------------
void ServerLobby::updateServerOwner()
{
    if (m_state.load() < ACCEPTING_CLIENTS ||
        m_state.load() > RESULT_DISPLAY)
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
    std::lock_guard<std::mutex> lock(m_connection_mutex);
    // Make sure no one access the weak pointer or adding player to peers
    for (auto peer: peers)
    {
        // Only 127.0.0.1 can be server owner in case of graphics-client-server
        if (peer->hasPlayerProfiles() &&
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
        ns->addUInt8(LE_SERVER_OWNERSHIP);
        owner->sendPacket(ns);
        delete ns;
        m_server_owner = owner;
        updatePlayerList();
    }
}   // updateServerOwner

//-----------------------------------------------------------------------------
/*! \brief Called when a player asks to select karts.
 *  \param event : Event providing the information.
 */
void ServerLobby::kartSelectionRequested(Event* event)
{
    std::lock_guard<std::mutex> lock(m_connection_mutex);
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
            // Will be reset to a random one later
            Log::debug("ServerLobby", "Player %d from peer %d chose unknown "
                "kart %s, use a random one", i, peer->getHostId(),
                kart.c_str());
            continue;
        }
        else
        {
            peer->getPlayerProfiles()[i]->setKartName(kart);
        }
        Log::debug("ServerLobby", "Player %d from peer %d chose %s", i,
            peer->getHostId(), kart.c_str());
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
        event->getPeer()->getPlayerProfiles().empty())
        return;

    // Check if first vote, if so start counter
    if (m_timeout.load() == std::numeric_limits<float>::max())
    {
        m_timeout.store((float)StkTime::getRealTime() +
            UserConfigParams::m_voting_timeout);
    }
    float remaining_time = m_timeout.load() - (float)StkTime::getRealTime();
    if (remaining_time < 0.0f)
    {
        return;
    }

    NetworkString& data = event->data();
    NetworkString other = NetworkString(PROTOCOL_LOBBY_ROOM);
    std::string name = StringUtils::wideToUtf8(event->getPeer()
        ->getPlayerProfiles()[0]->getName());
    other.addUInt8(LE_VOTE).addFloat(UserConfigParams::m_voting_timeout)
        .encodeString(name).addUInt32(event->getPeer()->getHostId());
    other += data;

    std::string track_name;
    data.decodeString(&track_name);
    uint8_t lap = data.getUInt8();
    uint8_t reverse = data.getUInt8();
    m_peers_votes[event->getPeerSP()] =
        std::make_tuple(track_name, lap, reverse == 1);
    sendMessageToPeersChangingToken(&other);

}   // playerVote

// ----------------------------------------------------------------------------
std::tuple<std::string, uint8_t, bool> ServerLobby::handleVote()
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
        final_track = track_vote->first;

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
        final_laps = lap_vote->first;

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
        final_reverse = reverse_vote->first;

    m_game_setup->setRace(final_track, final_laps, final_reverse);
    return std::make_tuple(final_track, final_laps, final_reverse);
}   // handleVote

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
/** Called when a notification from a client is received that the client has
 *  started the race. Once all clients have informed the server that they 
 *  have started the race, the server can start. This makes sure that the
 *  server's local time is behind all clients by (at least) their latency,
 *  which in turn means that when the server simulates local time T all
 *  messages from clients at their local time T should have arrived at
 *  the server, which creates smoother play experience.
 */
void ServerLobby::startedRaceOnClient(Event *event)
{
    std::shared_ptr<STKPeer> peer = event->getPeerSP();
    m_peers_ready.at(peer) = true;
    Log::info("ServerLobby", "Peer %d has started race at %lf",
        peer->getHostId(), StkTime::getRealTime());

    if (checkPeersReady())
    {
        m_state = DELAY_SERVER;
        m_server_delay = StkTime::getRealTime() + 0.1;
        Log::verbose("ServerLobby", "Started delay at %lf set delay to %lf",
            StkTime::getRealTime(), m_server_delay);
    }
}   // startedRaceOnClient

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
    std::lock_guard<std::mutex> lock(m_connection_mutex);
    m_ban_list.clear();
    for (auto& ban : UserConfigParams::m_server_ban_list)
    {
        if (ban.first == "0.0.0.0")
            continue;
        m_ban_list[TransportAddress(ban.first).getIP()] = ban.second;
    }
}   // updateBanList

//-----------------------------------------------------------------------------
bool ServerLobby::waitingForPlayers() const
{
    return m_state.load() == ACCEPTING_CLIENTS &&
        !m_game_setup->isGrandPrixStarted();
}   // waitingForPlayers
