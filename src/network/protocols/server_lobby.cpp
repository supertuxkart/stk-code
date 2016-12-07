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

#include "config/player_manager.hpp"
#include "config/user_config.hpp"
#include "modes/world.hpp"
#include "network/event.hpp"
#include "network/network_config.hpp"
#include "network/network_player_profile.hpp"
#include "network/protocols/get_public_address.hpp"
#include "network/protocols/connect_to_peer.hpp"
#include "network/protocols/latency_protocol.hpp"
#include "network/protocol_manager.hpp"
#include "network/race_event_manager.hpp"
#include "network/stk_host.hpp"
#include "network/stk_peer.hpp"
#include "online/online_profile.hpp"
#include "online/request_manager.hpp"
#include "states_screens/networking_lobby.hpp"
#include "states_screens/race_result_gui.hpp"
#include "states_screens/waiting_for_others.hpp"
#include "utils/log.hpp"
#include "utils/random_generator.hpp"
#include "utils/time.hpp"


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
    setHandleDisconnections(true);
}   // ServerLobby

//-----------------------------------------------------------------------------
/** Destructor.
 */
ServerLobby::~ServerLobby()
{
}   // ~ServerLobby

//-----------------------------------------------------------------------------

void ServerLobby::setup()
{
    m_game_setup = STKHost::get()->setupNewGame();
    m_game_setup->setNumLocalPlayers(0);    // no local players on a server
    m_next_player_id.setAtomic(0);

    // In case of LAN we don't need our public address or register with the
    // STK server, so we can directly go to the accepting clients state.
    m_state             = NetworkConfig::get()->isLAN() ? ACCEPTING_CLIENTS 
                                                        : INIT_WAN;
    m_selection_enabled = false;
    m_current_protocol  = NULL;
    Log::info("ServerLobby", "Starting the protocol.");

    // Initialise the data structures to detect if all clients and 
    // the server are ready:
    m_player_states.clear();
    m_client_ready_count.setAtomic(0);
    m_server_has_loaded_world = false;
    const std::vector<NetworkPlayerProfile*> &players =
                                                    m_game_setup->getPlayers();
    for (unsigned int i = 0; i < players.size(); i++)
    {
        m_player_states[players[i]->getGlobalPlayerId()] = false;
    }

}   // setup

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
        case LE_REQUEST_BEGIN: startSelection(event);             break;
        case LE_KART_SELECTION: kartSelectionRequested(event);    break;
        case LE_CLIENT_LOADED_WORLD: finishedLoadingWorldClient(event); break;
        case LE_STARTED_RACE:  startedRaceOnClient(event);        break;
        case LE_VOTE_MAJOR: playerMajorVote(event);               break;
        case LE_VOTE_RACE_COUNT: playerRaceCountVote(event);      break;
        case LE_VOTE_MINOR: playerMinorVote(event);               break;
        case LE_VOTE_TRACK: playerTrackVote(event);               break;
        case LE_VOTE_REVERSE: playerReversedVote(event);          break;
        case LE_VOTE_LAPS:  playerLapsVote(event);                break;
        case LE_RACE_FINISHED_ACK: playerFinishedResult(event);   break;
        }   // switch
           
    } // if (event->getType() == EVENT_TYPE_MESSAGE)
    else if (event->getType() == EVENT_TYPE_DISCONNECTED)
    {
        clientDisconnected(event);
    } // if (event->getType() == EVENT_TYPE_DISCONNECTED)
    return true;
}   // notifyEventAsynchronous

//-----------------------------------------------------------------------------
/** Simple finite state machine. First get the public ip address. Once this
 *  is known, register the server and its address with the stk server so that
 *  client can find it.
 */
void ServerLobby::update(float dt)
{
    switch (m_state)
    {
    case INIT_WAN:
        // Start the protocol to find the public ip address.
        m_current_protocol = new GetPublicAddress(this);
        m_current_protocol->requestStart();
        m_state = GETTING_PUBLIC_ADDRESS;
        // The callback from GetPublicAddress will wake this protocol up
        requestPause();
        break;
    case GETTING_PUBLIC_ADDRESS:
    {
        Log::debug("ServerLobby", "Public address known.");
        // Free GetPublicAddress protocol
        delete m_current_protocol;

        // Register this server with the STK server. This will block
        // this thread, but there is no need for the protocol manager
        // to react to any requests before the server is registered.
        registerServer();
        Log::info("ServerLobby", "Server registered.");
        m_state = ACCEPTING_CLIENTS;
    }
    break;
    case ACCEPTING_CLIENTS:
    {
        // Only poll the STK server if this is a WAN server.
        if(NetworkConfig::get()->isWAN())
            checkIncomingConnectionRequests();
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
    case WAIT_FOR_WORLD_LOADED:
        // Note that m_server_has_loaded_world is called by the main thread
        // (same a the thread updating this protocol)
        m_client_ready_count.lock();
        if (m_server_has_loaded_world &&
            m_client_ready_count.getData() == m_game_setup->getPlayerCount())
        {
            signalRaceStartToClients();
            m_server_delay = 0.02f;
            m_client_ready_count.getData() = 0;
        }
        m_client_ready_count.unlock();
        // Initialise counter again, to wait for all clients to indicate that
        // they have started the race/
        break;
    case WAIT_FOR_RACE_STARTED: 
        // The function finishedLoadingWorldClient() will trigger the
        // next state.
        break;
    case DELAY_SERVER:
        m_server_delay -= dt;
        if (m_server_delay < 0)
        {
            m_state = RACING;
            World::getWorld()->setReadyToRace();
        }
        break;
    case RACING:
        if (World::getWorld() &&
            RaceEventManager::getInstance<RaceEventManager>()->isRunning())
        {
            checkRaceFinished();
        }
        break;
    case RESULT_DISPLAY:
        if(StkTime::getRealTime() > m_timeout)
        {
            // Send a notification to all clients to exit 
            // the race result screen
            NetworkString *exit_result_screen = getNetworkString(1);
            exit_result_screen->setSynchronous(true);
            exit_result_screen->addUInt8(LE_EXIT_RESULT);
            sendMessageToPeersChangingToken(exit_result_screen,
                                            /*reliable*/true);
            delete exit_result_screen;
            m_state = ACCEPTING_CLIENTS;
            RaceResultGUI::getInstance()->backToLobby();
            // notify the network world that it is stopped
            RaceEventManager::getInstance()->stop();
            // stop race protocols
            findAndTerminateProtocol(PROTOCOL_CONTROLLER_EVENTS);
            findAndTerminateProtocol(PROTOCOL_KART_UPDATE);
            findAndTerminateProtocol(PROTOCOL_GAME_EVENTS);
        }
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
/** Callback when the GetPublicAddress terminates. It will unpause this
 *  protocol, which triggers the next state of the finite state machine.
 */
void ServerLobby::callback(Protocol *protocol)
{
    requestUnpause();
}   // callback

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
    const TransportAddress& addr = NetworkConfig::get()->getMyAddress();
    PlayerManager::setUserDetails(request, "create", Online::API::SERVER_PATH);
    request->addParameter("address",      addr.getIP()                    );
    request->addParameter("port",         addr.getPort()                  );
    request->addParameter("private_port",
                                    NetworkConfig::get()->getServerPort() );
    request->addParameter("name",   NetworkConfig::get()->getServerName() );
    request->addParameter("max_players", 
                          UserConfigParams::m_server_max_players          );
    Log::info("RegisterServer", "Showing addr %s", addr.toString().c_str());
    
    request->executeNow();

    const XMLNode * result = request->getXMLData();
    std::string rec_success;

    if (result->get("success", &rec_success) && rec_success == "yes")
    {
        Log::info("RegisterServer", "Server is now online.");
        STKHost::get()->setRegistered(true);
    }
    else
    {
        irr::core::stringc error(request->getInfo().c_str());
        Log::error("RegisterServer", "%s", error.c_str());
        STKHost::get()->setErrorMessage(_("Failed to register server: %s", error.c_str()));
    }

}   // registerServer

//-----------------------------------------------------------------------------
/** This function is called when all clients have loaded the world and
 *  are therefore ready to start the race. It signals to all clients
 *  to start the race and then switches state to DELAY_SERVER.
 */
void ServerLobby::signalRaceStartToClients()
{
    const std::vector<STKPeer*> &peers = STKHost::get()->getPeers();
    NetworkString *ns = getNetworkString(1);
    ns->addUInt8(LE_START_RACE);
    sendMessageToPeersChangingToken(ns, /*reliable*/true);
    delete ns;
    m_state = WAIT_FOR_RACE_STARTED;
}   // startGame

//-----------------------------------------------------------------------------
/** Instructs all clients to start the kart selection. If event is not NULL,
 *  the command comes from a client (which needs to be authorised).
 */
void ServerLobby::startSelection(const Event *event)
{

    if (m_state != ACCEPTING_CLIENTS)
    {
        Log::warn("ServerLobby",
                  "Received startSelection while being in state %d", m_state);
        return;
    }
    if(event && !event->getPeer()->isAuthorised())
    {
        Log::warn("ServerLobby", 
                  "Client %lx is not authorised to start selection.",
                  event->getPeer());
        return;
    }
    const std::vector<STKPeer*> &peers = STKHost::get()->getPeers();
    NetworkString *ns = getNetworkString(1);
    // start selection
    ns->addUInt8(LE_START_SELECTION);
    sendMessageToPeersChangingToken(ns, /*reliable*/true);
    delete ns;

    m_selection_enabled = true;

    m_state = SELECTING;
    WaitingForOthersScreen::getInstance()->push();

    Protocol *p = new LatencyProtocol();
    p->requestStart();
    Log::info("LobbyProtocol", "LatencyProtocol started.");

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

    // Now poll the stk server
    last_poll_time = StkTime::getRealTime();
    Online::XMLRequest* request = new Online::XMLRequest();
    PlayerManager::setUserDetails(request, "poll-connection-requests",
                                  Online::API::SERVER_PATH);

    const TransportAddress &addr = NetworkConfig::get()->getMyAddress();
    request->addParameter("address", addr.getIP()  );
    request->addParameter("port",    addr.getPort());

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
        Protocol *p = new ConnectToPeer(id);
        p->requestStart();
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
    if(!RaceEventManager::getInstance()->isRaceOver()) return;

    m_player_ready_counter = 0;
    // Set the delay before the server forces all clients to exit the race
    // result screen and go back to the lobby
    m_timeout = (float)(StkTime::getRealTime()+15.0f);
    m_state = RESULT_DISPLAY;

    // calculate karts ranks :
    int num_karts = race_manager->getNumberOfKarts();
    std::vector<int> karts_results;
    std::vector<float> karts_times;
    for (int j = 0; j < num_karts; j++)
    {
        float kart_time = race_manager->getKartRaceTime(j);
        for (unsigned int i = 0; i < karts_times.size(); i++)
        {
            if (kart_time < karts_times[i])
            {
                karts_times.insert(karts_times.begin() + i, kart_time);
                karts_results.insert(karts_results.begin() + i, j);
                break;
            }
        }
    }

    const std::vector<STKPeer*> &peers = STKHost::get()->getPeers();

    NetworkString *total = getNetworkString(1 + karts_results.size());
    total->setSynchronous(true);
    total->addUInt8(LE_RACE_FINISHED);
    for (unsigned int i = 0; i < karts_results.size(); i++)
    {
        total->addUInt8(karts_results[i]); // kart pos = i+1
        Log::info("ServerLobby", "Kart %d finished #%d",
            karts_results[i], i + 1);
    }
    sendMessageToPeersChangingToken(total, /*reliable*/ true);
    delete total;
    Log::info("ServerLobby", "End of game message sent");
        
}   // checkRaceFinished

//-----------------------------------------------------------------------------
/** Called when a client disconnects.
 *  \param event The disconnect event.
 */
void ServerLobby::clientDisconnected(Event* event)
{
    std::vector<NetworkPlayerProfile*> players_on_host = 
                                      event->getPeer()->getAllPlayerProfiles();

    NetworkString *msg = getNetworkString(2);
    msg->addUInt8(LE_PLAYER_DISCONNECTED);

    for(unsigned int i=0; i<players_on_host.size(); i++)
    {
        msg->addUInt8(players_on_host[i]->getGlobalPlayerId());
        Log::info("ServerLobby", "Player disconnected : id %d",
                  players_on_host[i]->getGlobalPlayerId());
        m_game_setup->removePlayer(players_on_host[i]);
    }

    sendMessageToPeersChangingToken(msg, /*reliable*/true);
    // Remove the profile from the peer (to avoid double free)
    STKHost::get()->removePeer(event->getPeer());
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
    STKPeer* peer = event->getPeer();
    const NetworkString &data = event->data();

    // can we add the player ?
    if (m_game_setup->getPlayerCount() >= NetworkConfig::get()->getMaxPlayers() ||
        m_state!=ACCEPTING_CLIENTS                                           )
    {
        NetworkString *message = getNetworkString(2);
        // Len, error code: 2 = busy, 0 = too many players
        message->addUInt8(LE_CONNECTION_REFUSED)
                .addUInt8(m_state!=ACCEPTING_CLIENTS ? 2 : 0);

        // send only to the peer that made the request
        peer->sendPacket(message);
        delete message;
        Log::verbose("ServerLobby", "Player refused");
        return;
    }

    // Connection accepted.
    // ====================
    std::string name_u8;
    int len = data.decodeString(&name_u8);
    core::stringw name = StringUtils::utf8ToWide(name_u8);
    std::string password;
    data.decodeString(&password);
    bool is_authorised = (password==NetworkConfig::get()->getPassword());

    // Get the unique global ID for this player.
    m_next_player_id.lock();
    m_next_player_id.getData()++;
    int new_player_id = m_next_player_id.getData();
    m_next_player_id.unlock();
    if(m_game_setup->getLocalMasterID()==0)
        m_game_setup->setLocalMaster(new_player_id);

    // The host id has already been incremented when the peer
    // was added, so it is the right id now.
    int new_host_id = STKHost::get()->getNextHostId();

    // Notify everybody that there is a new player
    // -------------------------------------------
    NetworkString *message = getNetworkString(3+1+name_u8.size());
    // size of id -- id -- size of local id -- local id;
    message->addUInt8(LE_NEW_PLAYER_CONNECTED).addUInt8(new_player_id)
            .addUInt8(new_host_id).encodeString(name_u8);
    STKHost::get()->sendPacketExcept(peer, message);
    delete message;

    // Now answer to the peer that just connected
    // ------------------------------------------
    RandomGenerator token_generator;
    // use 4 random numbers because rand_max is probably 2^15-1.
    uint32_t token = (uint32_t)((token_generator.get(RAND_MAX) & 0xff) << 24 |
                                (token_generator.get(RAND_MAX) & 0xff) << 16 |
                                (token_generator.get(RAND_MAX) & 0xff) <<  8 |
                                (token_generator.get(RAND_MAX) & 0xff));

    peer->setClientServerToken(token);
    peer->setAuthorised(is_authorised);
    peer->setHostId(new_host_id);

    const std::vector<NetworkPlayerProfile*> &players = m_game_setup->getPlayers();
    // send a message to the one that asked to connect
    // Estimate 10 as average name length
    NetworkString *message_ack = getNetworkString(4 + players.size() * (2+10));
    // connection success -- size of token -- token
    message_ack->addUInt8(LE_CONNECTION_ACCEPTED).addUInt8(new_player_id)
                .addUInt8(new_host_id).addUInt8(is_authorised);
    // Add all players so that this user knows (this new player is only added
    // to the list of players later, so the new player's info is not included)
    for (unsigned int i = 0; i < players.size(); i++)
    {
        message_ack->addUInt8(players[i]->getGlobalPlayerId())
                    .addUInt8(players[i]->getHostId())
                    .encodeString(players[i]->getName());
    }
    peer->sendPacket(message_ack);
    delete message_ack;

    NetworkPlayerProfile* profile = 
        new NetworkPlayerProfile(name, new_player_id, new_host_id);
    m_game_setup->addPlayer(profile);
    NetworkingLobby::getInstance()->addPlayer(profile);

    Log::verbose("ServerLobby", "New player.");

}   // connectionRequested

//-----------------------------------------------------------------------------

/*! \brief Called when a player asks to select a kart.
 *  \param event : Event providing the information.
 *
 *  Format of the data :
 *  Byte 0          1                      2            
 *       ----------------------------------------------
 *  Size |    1     |           1         |     N     |
 *  Data |player id |  N (kart name size) | kart name |
 *       ----------------------------------------------
 */
void ServerLobby::kartSelectionRequested(Event* event)
{
    if(m_state!=SELECTING)
    {
        Log::warn("Server", "Received kart selection while in state %d.",
                  m_state);
        return;
    }

    if (!checkDataSize(event, 1)) return;

    const NetworkString &data = event->data();
    STKPeer* peer = event->getPeer();

    uint8_t player_id = data.getUInt8();
    std::string kart_name;
    data.decodeString(&kart_name);
    // check if selection is possible
    if (!m_selection_enabled)
    {
        NetworkString *answer = getNetworkString(2);
        // selection still not started
        answer->addUInt8(LE_KART_SELECTION_REFUSED).addUInt8(2);
        peer->sendPacket(answer);
        delete answer;
        return;
    }
    // check if somebody picked that kart
    if (!m_game_setup->isKartAvailable(kart_name))
    {
        NetworkString *answer = getNetworkString(2);
        // kart is already taken
        answer->addUInt8(LE_KART_SELECTION_REFUSED).addUInt8(0);
        peer->sendPacket(answer);
        delete answer;
        return;
    }
    // check if this kart is authorized
    if (!m_game_setup->isKartAllowed(kart_name))
    {
        NetworkString *answer = getNetworkString(2);
        // kart is not authorized
        answer->addUInt8(LE_KART_SELECTION_REFUSED).addUInt8(1);
        peer->sendPacket(answer);
        delete answer;
        return;
    }

    // send a kart update to everyone
    NetworkString *answer = getNetworkString(3+kart_name.size());
    // This message must be handled synchronously on the client.
    answer->setSynchronous(true);
    // kart update (3), 1, race id
    answer->addUInt8(LE_KART_SELECTION_UPDATE).addUInt8(player_id)
          .encodeString(kart_name);
    sendMessageToPeersChangingToken(answer);
    delete answer;
    m_game_setup->setPlayerKart(player_id, kart_name);
}   // kartSelectionRequested

//-----------------------------------------------------------------------------

/*! \brief Called when a player votes for a major race mode.
 *  \param event : Event providing the information.
 *
 *  Format of the data :
 *  Byte 0           1
 *       -------------------------------
 *  Size |      1    |       4         |
 *  Data | player-id | major mode vote |
 *       -------------------------------
 */
void ServerLobby::playerMajorVote(Event* event)
{
    if (!checkDataSize(event, 5)) return;

    NetworkString &data = event->data();
    uint8_t player_id   = data.getUInt8();
    uint32_t major      = data.getUInt32();
    m_game_setup->getRaceConfig()->setPlayerMajorVote(player_id, major);
    // Send the vote to everybody (including the sender)
    NetworkString *other = getNetworkString(6);
    other->addUInt8(LE_VOTE_MAJOR).addUInt8(player_id).addUInt32(major);
    sendMessageToPeersChangingToken(other);
    delete other;
}   // playerMajorVote

//-----------------------------------------------------------------------------
/** \brief Called when a player votes for the number of races in a GP.
 *  \param event : Event providing the information.
 *
 *  Format of the data :
 *  Byte 0           1
 *       ---------------------------
 *  Size |      1    |       4     |
 *  Data | player-id | races count |
 *       ---------------------------
 */
void ServerLobby::playerRaceCountVote(Event* event)
{
    if (!checkDataSize(event, 1)) return;
    NetworkString &data = event->data();
    uint8_t player_id   = data.getUInt8();
    uint8_t race_count  = data.getUInt8();
    m_game_setup->getRaceConfig()->setPlayerRaceCountVote(player_id, race_count);
    // Send the vote to everybody (including the sender)
    NetworkString *other = getNetworkString(3);
    other->addUInt8(LE_VOTE_RACE_COUNT).addUInt8(player_id)
          .addUInt8(race_count);
    sendMessageToPeersChangingToken(other);
    delete other;
}   // playerRaceCountVote

//-----------------------------------------------------------------------------

/*! \brief Called when a player votes for a minor race mode.
 *  \param event : Event providing the information.
 *
 *  Format of the data :
 *  Byte 0           1
 *       -------------------------------
 *  Size |      1    |         4       |
 *  Data | player-id | minor mode vote |
 *       -------------------------------
 */
void ServerLobby::playerMinorVote(Event* event)
{
    if (!checkDataSize(event, 1)) return;
    NetworkString &data = event->data();
    uint8_t player_id   = data.getUInt8();
    uint32_t minor      = data.getUInt32();
    m_game_setup->getRaceConfig()->setPlayerMinorVote(player_id, minor);

    // Send the vote to everybody (including the sender)
    NetworkString *other = getNetworkString(3);
    other->addUInt8(LE_VOTE_MINOR).addUInt8(player_id).addUInt8(minor); 
    sendMessageToPeersChangingToken(other);
    delete other;
}   // playerMinorVote

//-----------------------------------------------------------------------------

/*! \brief Called when a player votes for a track.
 *  \param event : Event providing the information.
 *
 *  Format of the data :
 *  Byte 0           1                    2  3
 *       --------------------------------------------------
 *  Size |     1     |        1          | 1 |      N     |
 *  Data | player id | track number (gp) | N | track name |
 *       --------------------------------------------------
 */
void ServerLobby::playerTrackVote(Event* event)
{
    if (!checkDataSize(event, 3)) return;
    NetworkString &data  = event->data();
    uint8_t player_id    = data.getUInt8();
    // As which track this track should be used, e.g. 1st track: Sandtrack
    // 2nd track Mathclass, ...
    uint8_t track_number = data.getUInt8();
    std::string track_name;
    int N = data.decodeString(&track_name);
    m_game_setup->getRaceConfig()->setPlayerTrackVote(player_id, track_name,
                                                 track_number);
    // Send the vote to everybody (including the sender)
    NetworkString *other = getNetworkString(3+1+data.size());
    other->addUInt8(LE_VOTE_TRACK).addUInt8(player_id).addUInt8(track_number)
          .encodeString(track_name);
    sendMessageToPeersChangingToken(other);
    delete other;

    // Check if we received all information
    if (m_game_setup->getRaceConfig()->getNumTrackVotes() ==
                                                m_game_setup->getPlayerCount())
    {
        // Inform clients to start loading the world
        NetworkString *ns = getNetworkString(1);
        ns->setSynchronous(true);
        ns->addUInt8(LE_LOAD_WORLD);
        sendMessageToPeersChangingToken(ns, /*reliable*/true);
        delete ns;
        m_state = LOAD_WORLD;   // Server can now load world
    }
}   // playerTrackVote

//-----------------------------------------------------------------------------
/*! \brief Called when a player votes for the reverse mode of a race
 *  \param event : Event providing the information.
 *
 *  Format of the data :
 *  Byte 0           1          2
 *       --------------------------------------------
 *  Size |     1     |     1    |       1           |
 *  Data | player id | reversed | track number (gp) |
 *       --------------------------------------------
 */
void ServerLobby::playerReversedVote(Event* event)
{
    if (!checkDataSize(event, 3)) return;

    NetworkString &data = event->data();
    uint8_t player_id   = data.getUInt8();
    uint8_t reverse     = data.getUInt8();
    uint8_t nb_track    = data.getUInt8();
    m_game_setup->getRaceConfig()->setPlayerReversedVote(player_id,
                                                         reverse!=0, nb_track);
    // Send the vote to everybody (including the sender)
    NetworkString *other = getNetworkString(4);
    other->addUInt8(LE_VOTE_REVERSE).addUInt8(player_id).addUInt8(reverse)
          .addUInt8(nb_track);
    sendMessageToPeersChangingToken(other);
    delete other;
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
void ServerLobby::playerLapsVote(Event* event)
{
    if (!checkDataSize(event, 2)) return;
    NetworkString &data = event->data();
    uint8_t player_id   = data.getUInt8();
    uint8_t lap_count   = data.getUInt8();
    uint8_t track_nb    = data.getUInt8();
    m_game_setup->getRaceConfig()->setPlayerLapsVote(player_id, lap_count,
                                                     track_nb);
    NetworkString *other = getNetworkString(4);
    other->addUInt8(LE_VOTE_LAPS).addUInt8(player_id).addUInt8(lap_count)
          .addUInt8(track_nb);
    sendMessageToPeersChangingToken(other);
    delete other;
}   // playerLapsVote

// ----------------------------------------------------------------------------
/** Called from the RaceManager of the server when the world is loaded. Marks
 *  the server to be ready to start the race.
 */
void ServerLobby::finishedLoadingWorld()
{
    m_server_has_loaded_world = true;
}   // finishedLoadingWorld;

//-----------------------------------------------------------------------------
/** Called when a client notifies the server that it has loaded the world.
 *  When all clients and the server are ready, the race can be started.
 */
void ServerLobby::finishedLoadingWorldClient(Event *event)
{
    if (!checkDataSize(event, 1)) return;

    const NetworkString &data = event->data();
    uint8_t player_count = data.getUInt8();
    m_client_ready_count.lock();
    for (unsigned int i = 0; i < player_count; i++)
    {
        uint8_t player_id = data.getUInt8();
        if (m_player_states[player_id])
        {
            Log::error("ServerLobbyProtocol",
                       "Player %d send more than one ready message.",
                       player_id);
            m_client_ready_count.unlock();
            return;
        }
        m_player_states[player_id] = true;
        m_client_ready_count.getData()++;
        Log::info("ServerLobbyeProtocol", "Player %d is ready (%d/%d).",
            player_id, m_client_ready_count.getData(),
            m_game_setup->getPlayerCount());
    }
    m_client_ready_count.unlock();

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
    m_client_ready_count.lock();
    Log::verbose("ServerLobby", "Host %d has started race.",
                 event->getPeer()->getHostId());
    m_client_ready_count.getData()++;
    if (m_client_ready_count.getData() == m_game_setup->getPlayerCount())
    {
        m_state = DELAY_SERVER;
        terminateLatencyProtocol();
    }
    m_client_ready_count.unlock();
}   // startedRaceOnClient

//-----------------------------------------------------------------------------
/** Called when a client clicks on 'ok' on the race result screen.
 *  If all players have clicked on 'ok', go back to the lobby.
 */
void ServerLobby::playerFinishedResult(Event *event)
{
    m_player_ready_counter++;
    if(m_player_ready_counter == STKHost::get()->getPeerCount())
    {
        // We can't trigger the world/race exit here, since this is called
        // from the protocol manager thread. So instead we force the timeout
        // to get triggered (which is done from the main thread):
        m_timeout = 0;
    }
}   // playerFinishedResult

//-----------------------------------------------------------------------------
