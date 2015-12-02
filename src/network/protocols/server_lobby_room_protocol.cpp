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

#include "network/protocols/server_lobby_room_protocol.hpp"

#include "config/player_manager.hpp"
#include "config/user_config.hpp"
#include "modes/world.hpp"
#include "network/event.hpp"
#include "network/network_config.hpp"
#include "network/network_world.hpp"
#include "network/network_player_profile.hpp"
#include "network/protocols/get_public_address.hpp"
#include "network/protocols/connect_to_peer.hpp"
#include "network/protocols/start_game_protocol.hpp"
#include "network/protocol_manager.hpp"
#include "network/stk_host.hpp"
#include "network/stk_peer.hpp"
#include "online/online_profile.hpp"
#include "online/request_manager.hpp"
#include "utils/log.hpp"
#include "utils/random_generator.hpp"
#include "utils/time.hpp"


/** This is the central game setup protocol running in the server.
 *  It starts with detecting the public ip address and port of this
 *  host (GetPublicAddress).
 */
ServerLobbyRoomProtocol::ServerLobbyRoomProtocol() : LobbyRoomProtocol(NULL)
{
}   // ServerLobbyRoomProtocol

//-----------------------------------------------------------------------------
/** Destructor.
 */
ServerLobbyRoomProtocol::~ServerLobbyRoomProtocol()
{
}   // ~ServerLobbyRoomProtocol

//-----------------------------------------------------------------------------

void ServerLobbyRoomProtocol::setup()
{
    m_setup             = STKHost::get()->setupNewGame();
    m_next_player_id.setAtomic(0);

    // In case of LAN we don't need our public address or register with the
    // STK server, so we can directly go to the accepting clients state.
    m_state             = NetworkConfig::get()->isLAN() ? ACCEPTING_CLIENTS 
                                                        : NONE;
    m_selection_enabled = false;
    m_in_race           = false;
    m_current_protocol  = NULL;
    Log::info("ServerLobbyRoomProtocol", "Starting the protocol.");
}   // setup

//-----------------------------------------------------------------------------

bool ServerLobbyRoomProtocol::notifyEventAsynchronous(Event* event)
{
    assert(m_setup); // assert that the setup exists
    if (event->getType() == EVENT_TYPE_MESSAGE)
    {
        const NetworkString &data = event->data();
        assert(data.size()); // message not empty
        uint8_t message_type;
        message_type = data[0];
        event->removeFront(1);
        Log::info("ServerLobbyRoomProtocol", "Message received with type %d.",
                  message_type);
        switch(message_type)
        {
        case LE_CONNECTION_REQUESTED: connectionRequested(event); break;
        case LE_REQUEST_BEGIN: startSelection(event);             break;
        case LE_KART_SELECTION: kartSelectionRequested(event);    break;
        case LE_VOTE_MAJOR: playerMajorVote(event);               break;
        case LE_VOTE_RACE_COUNT: playerRaceCountVote(event);      break;
        case LE_VOTE_MINOR: playerMinorVote(event);               break;
        case LE_VOTE_TRACK: playerTrackVote(event);               break;
        case LE_VOTE_REVERSE: playerReversedVote(event);          break;
        case LE_VOTE_LAPS:  playerLapsVote(event);                break;
        }   // switch
           
    } // if (event->getType() == EVENT_TYPE_MESSAGE)
    else if (event->getType() == EVENT_TYPE_CONNECTED)
    {
    } // if (event->getType() == EVENT_TYPE_CONNECTED)
    else if (event->getType() == EVENT_TYPE_DISCONNECTED)
    {
        kartDisconnected(event);
    } // if (event->getType() == EVENT_TYPE_DISCONNECTED)
    return true;
}   // notifyEventAsynchronous

//-----------------------------------------------------------------------------
/** Simple finite state machine. First get the public ip address. Once this
 *  is known, register the server and its address with the stk server so that
 *  client can find it.
 */
void ServerLobbyRoomProtocol::update()
{
    switch (m_state)
    {
    case NONE:
        // Start the protocol to find the public ip address.
        m_current_protocol = new GetPublicAddress(this);
        m_current_protocol->requestStart();
        m_state = GETTING_PUBLIC_ADDRESS;
        // The callback from GetPublicAddress will wake this protocol up
        ProtocolManager::getInstance()->pauseProtocol(this);
        break;
    case GETTING_PUBLIC_ADDRESS:
        {
            Log::debug("ServerLobbyRoomProtocol", "Public address known.");
            // Free GetPublicAddress protocol
            delete m_current_protocol;

            // Register this server with the STK server. This will block
            // this thread, but there is no need for the protocol manager
            // to react to any requests before the server is registered.
            registerServer();
            Log::info("ServerLobbyRoomProtocol", "Server registered.");
            m_state = ACCEPTING_CLIENTS;
        }
        break;
    case ACCEPTING_CLIENTS:
    {
        // Only poll the STK server if this is a WAN server.
        if(NetworkConfig::get()->isWAN())
            checkIncomingConnectionRequests();
        if (m_in_race && World::getWorld() &&
            NetworkWorld::getInstance<NetworkWorld>()->isRunning())
        {
            checkRaceFinished();
        }

        break;
    }
    case SELECTING_KARTS:
        break;   // Nothing to do, this is event based
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
 * protocol, which triggers the next state of the finite state machine.
 */
void ServerLobbyRoomProtocol::callback(Protocol *protocol)
{
    ProtocolManager::getInstance()->unpauseProtocol(this);
}   // callback

//-----------------------------------------------------------------------------
/** Register this server (i.e. its public address) with the STK server
 *  so that clients can find it. It blocks till a response from the
 *  stk server is received (this function is executed from the 
 *  ProtocolManager thread). The information about this client is added
 *  to the table 'server'.
 */
void ServerLobbyRoomProtocol::registerServer()
{
    Online::XMLRequest *request = new Online::XMLRequest();
    const TransportAddress& addr = NetworkConfig::get()->getMyAddress();
#ifdef NEW_PROTOCOL
    PlayerManager::setUserDetails(request, "register", Online::API::SERVER_PATH);
#else
    PlayerManager::setUserDetails(request, "start", Online::API::SERVER_PATH);
#endif
    request->addParameter("address",      addr.getIP()                    );
    request->addParameter("port",         addr.getPort()                  );
    request->addParameter("private_port",
                                    NetworkConfig::get()->getPrivatePort());
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
        STKHost::get()->setErrorMessage(_("Failed to register server"));
    }

}   // registerServer

//-----------------------------------------------------------------------------

void ServerLobbyRoomProtocol::startGame()
{
    const std::vector<STKPeer*> &peers = STKHost::get()->getPeers();
    for (unsigned int i = 0; i < peers.size(); i++)
    {
        NetworkString ns(6);
        ns.ai8(LE_START_RACE).ai8(4).ai32(peers[i]->getClientServerToken());
        sendMessage(peers[i], ns, true); // reliably
    }
    Protocol *p = new StartGameProtocol(m_setup);
    p->requestStart();
    m_in_race = true;
}   // startGame

//-----------------------------------------------------------------------------
/** Instructs all clients to start the kart selection. If event is not NULL,
 *  the command comes from a client (which needs to be authorised).
 */
void ServerLobbyRoomProtocol::startSelection(const Event *event)
{
    if(event && !STKHost::get()->isAuthorisedToControl(event->getPeer()))
    {
        Log::warn("ServerLobby", 
                  "Client %lx is not authorised to start selection.",
                  event->getPeer());
        return;
    }
    const std::vector<STKPeer*> &peers = STKHost::get()->getPeers();
    for (unsigned int i = 0; i < peers.size(); i++)
    {
        NetworkString ns(6);
        // start selection
        ns.ai8(LE_START_SELECTION).ai8(4).ai32(peers[i]->getClientServerToken());
        sendMessage(peers[i], ns, true); // reliably
    }
    m_selection_enabled = true;

    m_state = SELECTING_KARTS;
}   // startSelection

//-----------------------------------------------------------------------------
/** Query the STK server for connection requests. For each connection request
 *  start a ConnectToPeer protocol.
 */
void ServerLobbyRoomProtocol::checkIncomingConnectionRequests()
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
        Log::error("ServerLobbyRoomProtocol", "Cannot retrieve the list.");
        return;
    }

    // Now start a ConnectToPeer protocol for each connection request
    const XMLNode * users_xml = result->getNode("users");
    uint32_t id = 0;
    for (unsigned int i = 0; i < users_xml->getNumNodes(); i++)
    {
        users_xml->getNode(i)->get("id", &id);
        Log::debug("ServerLobbyRoomProtocol",
                   "User with id %d wants to connect.", id);
        Protocol *p = new ConnectToPeer(id);
        p->requestStart();
    }        
    delete request;
}   // checkIncomingConnectionRequests

//-----------------------------------------------------------------------------

void ServerLobbyRoomProtocol::checkRaceFinished()
{
    assert(NetworkWorld::getInstance()->isRunning());
    assert(World::getWorld());
    // if race is over, give the final score to everybody
    if (NetworkWorld::getInstance()->isRaceOver())
    {
        // calculate karts ranks :
        int num_players = race_manager->getNumberOfKarts();
        std::vector<int> karts_results;
        std::vector<float> karts_times;
        for (int j = 0; j < num_players; j++)
        {
            float kart_time = race_manager->getKartRaceTime(j);
            for (unsigned int i = 0; i < karts_times.size(); i++)
            {
                if (kart_time < karts_times[i])
                {
                    karts_times.insert(karts_times.begin()+i, kart_time);
                    karts_results.insert(karts_results.begin()+i, j);
                    break;
                }
            }
        }

        const std::vector<STKPeer*> &peers = STKHost::get()->getPeers();

        NetworkString queue(karts_results.size()*2);
        for (unsigned int i = 0; i < karts_results.size(); i++)
        {
            queue.ai8(1).ai8(karts_results[i]); // kart pos = i+1
            Log::info("ServerLobbyRoomProtocol", "Kart %d finished #%d",
                      karts_results[i], i + 1);
        }
        for (unsigned int i = 0; i < peers.size(); i++)
        {
            NetworkString ns(6);
            ns.ai8(0x06).ai8(4).ai32(peers[i]->getClientServerToken());
            NetworkString total = ns + queue;
            sendMessage(peers[i], total, true);
        }
        Log::info("ServerLobbyRoomProtocol", "End of game message sent");
        m_in_race = false;

        // stop race protocols
        Protocol* protocol = ProtocolManager::getInstance()
                           ->getProtocol(PROTOCOL_CONTROLLER_EVENTS);
        if (protocol)
            protocol->requestTerminate();
        else
            Log::error("ClientLobbyRoomProtocol",
                       "No controller events protocol registered.");

        protocol = ProtocolManager::getInstance()
                 ->getProtocol(PROTOCOL_KART_UPDATE);
        if (protocol)
            protocol->requestTerminate();
        else
            Log::error("ClientLobbyRoomProtocol",
                       "No kart update protocol registered.");

        protocol = ProtocolManager::getInstance()
                 ->getProtocol(PROTOCOL_GAME_EVENTS);
        if (protocol)
            protocol->requestTerminate();
        else
            Log::error("ClientLobbyRoomProtocol",
                       "No game events protocol registered.");

        // notify the network world that it is stopped
        NetworkWorld::getInstance()->stop();
        // exit the race now
        race_manager->exitRace();
        race_manager->setAIKartOverride("");
    }
}   // checkRaceFinished

//-----------------------------------------------------------------------------

void ServerLobbyRoomProtocol::kartDisconnected(Event* event)
{
    STKPeer* peer = event->getPeer();
    if (peer->getPlayerProfile() != NULL) // others knew him
    {
        NetworkString msg(3);
        msg.ai8(LE_PLAYER_DISCONNECTED).ai8(1)
           .ai8(peer->getPlayerProfile()->getPlayerID());
        sendMessage(msg);
        Log::info("ServerLobbyRoomProtocol", "Player disconnected : id %d",
                  peer->getPlayerProfile()->getPlayerID());
        m_setup->removePlayer(peer->getPlayerProfile());
        // Remove the profile from the peer (to avoid double free)
        peer->setPlayerProfile(NULL);
        STKHost::get()->removePeer(peer);
    }
    else
        Log::info("ServerLobbyRoomProtocol", "The DC peer wasn't registered.");
}   // kartDisconnected

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
void ServerLobbyRoomProtocol::connectionRequested(Event* event)
{
    STKPeer* peer = event->getPeer();
    const NetworkString &data = event->data();

    // can we add the player ?
    if (m_setup->getPlayerCount() >= NetworkConfig::get()->getMaxPlayers() ||
        m_state!=ACCEPTING_CLIENTS                                           )
    {
        NetworkString message(3);
        // Len, error code: 2 = busy, 0 = too many players
        message.ai8(LE_CONNECTION_REFUSED).ai8(1)
                .ai8(m_state!=ACCEPTING_CLIENTS ? 2 : 0);

        // send only to the peer that made the request
        sendMessage(peer, message);
        Log::verbose("ServerLobbyRoomProtocol", "Player refused");
        return;
    }

    // Connection accepted.
    // ====================
    std::string name_u8;
    int name_len = data.decodeString(0, &name_u8);
    core::stringw name = StringUtils::utf8ToWide(name_u8);

    // add the player to the game setup
    m_next_player_id.lock();
    m_next_player_id.getData()++;
    int new_player_id = m_next_player_id.getData();
    m_next_player_id.unlock();

    NetworkPlayerProfile* profile = new NetworkPlayerProfile(new_player_id, name);
    m_setup->addPlayer(profile);
    peer->setPlayerProfile(profile);

    // notify everybody that there is a new player
    NetworkString message(8);
    // size of id -- id -- size of local id -- local id;
    message.ai8(LE_NEW_PLAYER_CONNECTED).ai8(1).ai8(new_player_id)
           .encodeString(name_u8);
    ProtocolManager::getInstance()->sendMessageExcept(this, peer, message);

    // Now answer to the peer that just connected
    RandomGenerator token_generator;
    // use 4 random numbers because rand_max is probably 2^15-1.
    uint32_t token = (uint32_t)((token_generator.get(RAND_MAX) & 0xff) << 24 |
                                (token_generator.get(RAND_MAX) & 0xff) << 16 |
                                (token_generator.get(RAND_MAX) & 0xff) <<  8 |
                                (token_generator.get(RAND_MAX) & 0xff));

    std::vector<NetworkPlayerProfile*> players = m_setup->getPlayers();
    // send a message to the one that asked to connect
    // Size is overestimated, probably one player's data will not be sent
    NetworkString message_ack(13 + players.size() * 7);
    // connection success -- size of token -- token
    message_ack.ai8(LE_CONNECTION_ACCEPTED).ai8(1).ai8(new_player_id).ai8(4)
               .ai32(token);
    // add all players so that this user knows
    for (unsigned int i = 0; i < players.size(); i++)
    {
        // do not duplicate the player into the message
        if (players[i]->getPlayerID() != new_player_id )
        {
            message_ack.ai8(1).ai8(players[i]->getPlayerID())
                       .encodeString(players[i]->getName());
        }
    }
    sendMessage(peer, message_ack);

    peer->setClientServerToken(token);

    Log::verbose("ServerLobbyRoomProtocol", "New player.");

}   // connectionRequested

//-----------------------------------------------------------------------------

/*! \brief Called when a player asks to select a kart.
 *  \param event : Event providing the information.
 *
 *  Format of the data :
 *  Byte 0   1            5                    6           N+6
 *       ---------------------------------------------------
 *  Size | 1 |      4     |          1         |     N     |
 *  Data | 4 | priv token | N (kart name size) | kart name |
 *       ---------------------------------------------------
 */
void ServerLobbyRoomProtocol::kartSelectionRequested(Event* event)
{
    if(m_state!=SELECTING_KARTS)
    {
        Log::warn("Server", "Received kart selection while in state %d.",
                  m_state);
        return;
    }

    const NetworkString &data = event->data();
    STKPeer* peer = event->getPeer();
    if (!checkDataSizeAndToken(event, 6))
        return;

    std::string kart_name;
    data.decodeString(5, &kart_name);
    // check if selection is possible
    if (!m_selection_enabled)
    {
        NetworkString answer(3);
        // selection still not started
        answer.ai8(LE_KART_SELECTION_REFUSED).ai8(1).ai8(2);
        sendMessage(peer, answer);
        return;
    }
    // check if somebody picked that kart
    if (!m_setup->isKartAvailable(kart_name))
    {
        NetworkString answer(3);
        // kart is already taken
        answer.ai8(LE_KART_SELECTION_REFUSED).ai8(1).ai8(0);
        sendMessage(peer, answer);
        return;
    }
    // check if this kart is authorized
    if (!m_setup->isKartAllowed(kart_name))
    {
        NetworkString answer(3);
        // kart is not authorized
        answer.ai8(LE_KART_SELECTION_REFUSED).ai8(1).ai8(1);
        sendMessage(peer, answer);
        return;
    }
    // send a kart update to everyone
    NetworkString answer(3+1+kart_name.size());
    // kart update (3), 1, race id
    uint8_t player_id = peer->getPlayerProfile()->getPlayerID();
    answer.ai8(LE_KART_SELECTION_UPDATE).ai8(1).ai8(player_id)
          .encodeString(kart_name);
    sendMessage(answer);
    m_setup->setPlayerKart(player_id, kart_name);
}   // kartSelectionRequested

//-----------------------------------------------------------------------------

/*! \brief Called when a player votes for a major race mode.
 *  \param event : Event providing the information.
 *
 *  Format of the data :
 *  Byte 0   1            5   6                 10
 *       ----------------------------------------
 *  Size | 1 |      4     | 1 |        4        |
 *  Data | 4 | priv token | 4 | major mode vote |
 *       ----------------------------------------
 */
void ServerLobbyRoomProtocol::playerMajorVote(Event* event)
{
    NetworkString &data = event->data();
    STKPeer* peer = event->getPeer();
    if (!checkDataSizeAndToken(event, 7))
        return;
    if (!isByteCorrect(event, 5, 4))
        return;
    uint8_t player_id = peer->getPlayerProfile()->getPlayerID();
    uint32_t major = data.getUInt32(6);
    m_setup->getRaceConfig()->setPlayerMajorVote(player_id, major);
    // Send the vote to everybody (including the sender)
    data.removeFront(5); // remove the token
    NetworkString other(5+data.size());
    other.ai8(1).ai8(player_id); // add the player id
    other += data; // add the data
    NetworkString prefix(1);
    prefix.ai8(LE_VOTE_MAJOR); // prefix the token with the type
    sendMessageToPeersChangingToken(prefix, other);
}   // playerMajorVote

//-----------------------------------------------------------------------------

/*! \brief Called when a player votes for the number of races in a GP.
 *  \param event : Event providing the information.
 *
 *  Format of the data :
 *  Byte 0   1            5   6             7
 *       ------------------------------------
 *  Size | 1 |      4     | 1 |      1      |
 *  Data | 4 | priv token | 1 | races count |
 *       ------------------------------------
 */
void ServerLobbyRoomProtocol::playerRaceCountVote(Event* event)
{
    NetworkString &data = event->data();
    STKPeer* peer = event->getPeer();
    if (!checkDataSizeAndToken(event, 7))
        return;
    if (!isByteCorrect(event, 5, 1))
        return;
    uint8_t player_id = peer->getPlayerProfile()->getPlayerID();
    m_setup->getRaceConfig()->setPlayerRaceCountVote(player_id, data[6]);
    // Send the vote to everybody (including the sender)
    data.removeFront(5); // remove the token
    NetworkString other(2+data.size());
    other.ai8(1).ai8(player_id); // add the player id
    other += data; // add the data
    NetworkString prefix(1);
    prefix.ai8(LE_VOTE_RACE_COUNT); // prefix the token with the type
    sendMessageToPeersChangingToken(prefix, other);
}   // playerRaceCountVote

//-----------------------------------------------------------------------------

/*! \brief Called when a player votes for a minor race mode.
 *  \param event : Event providing the information.
 *
 *  Format of the data :
 *  Byte 0   1            5   6                 10
 *       ----------------------------------------
 *  Size | 1 |      4     | 1 |        4        |
 *  Data | 4 | priv token | 4 | minor mode vote |
 *       ----------------------------------------
 */
void ServerLobbyRoomProtocol::playerMinorVote(Event* event)
{
    NetworkString &data = event->data();
    STKPeer* peer = event->getPeer();
    if (!checkDataSizeAndToken(event, 7))
        return;
    if (!isByteCorrect(event, 5, 4))
        return;
    uint8_t player_id = peer->getPlayerProfile()->getPlayerID();
    uint32_t minor = data.getUInt32(6);
    m_setup->getRaceConfig()->setPlayerMinorVote(player_id, minor);
    // Send the vote to everybody (including the sender)
    data.removeFront(5); // remove the token
    NetworkString other(2+data.size());
    other.ai8(1).ai8(player_id); // add the player id
    other += data; // add the data
    NetworkString prefix(1);
    prefix.ai8(LE_VOTE_MINOR); // prefix the token with the ype
    sendMessageToPeersChangingToken(prefix, other);
}   // playerMinorVote

//-----------------------------------------------------------------------------

/*! \brief Called when a player votes for a track.
 *  \param event : Event providing the information.
 *
 *  Format of the data :
 *  Byte 0   1            5   6            N+6 N+7                 N+8
 *       -----------------------------------------------------------
 *  Size | 1 |      4     | 1 |      N     | 1 |       1           |
 *  Data | 4 | priv token | N | track name | 1 | track number (gp) |
 *       -----------------------------------------------------------
 */
void ServerLobbyRoomProtocol::playerTrackVote(Event* event)
{
    NetworkString &data = event->data();
    STKPeer* peer = event->getPeer();
    if (!checkDataSizeAndToken(event, 8))
        return;
    std::string track_name;
    int N = data.decodeString(5, &track_name);
    if (!isByteCorrect(event, N+5, 1))
        return;
    uint8_t player_id = peer->getPlayerProfile()->getPlayerID();
    m_setup->getRaceConfig()->setPlayerTrackVote(player_id, track_name, data[N+6]);
    // Send the vote to everybody (including the sender)
    data.removeFront(5); // remove the token
    NetworkString other(2+data.size());
    other.ai8(1).ai8(player_id); // add the player id
    other += data; // add the data
    NetworkString prefix(1);
    prefix.ai8(LE_VOTE_TRACK); // prefix the token with the ype
    sendMessageToPeersChangingToken(prefix, other);
}   // playerTrackVote

//-----------------------------------------------------------------------------

/*! \brief Called when a player votes for the reverse mode of a race
 *  \param event : Event providing the information.
 *
 *  Format of the data :
 *  Byte 0   1            5   6          7   8                   9
 *       ---------------------------------------------------------
 *  Size | 1 |      4     | 1 |     1    | 1 |       1           |
 *  Data | 4 | priv token | 1 | reversed | 1 | track number (gp) |
 *       ---------------------------------------------------------
 */
void ServerLobbyRoomProtocol::playerReversedVote(Event* event)
{
    NetworkString &data = event->data();
    STKPeer* peer = event->getPeer();
    if (!checkDataSizeAndToken(event, 9))
        return;
    if (!isByteCorrect(event, 5, 1))
        return;
    if (!isByteCorrect(event, 7, 1))
        return;
    uint8_t player_id = peer->getPlayerProfile()->getPlayerID();
    m_setup->getRaceConfig()->setPlayerReversedVote(player_id,
                                                    data[6]!=0, data[8]);
    // Send the vote to everybody (including the sender)
    data.removeFront(5); // remove the token
    NetworkString other(2+data.size());
    other.ai8(1).ai8(player_id); // add the player id
    other += data; // add the data
    NetworkString prefix(1);
    prefix.ai8(LE_VOTE_REVERSE); // prefix the token with the ype
    sendMessageToPeersChangingToken(prefix, other);
}   // playerReversedVote

//-----------------------------------------------------------------------------

/*! \brief Called when a player votes for a major race mode.
 *  \param event : Event providing the information.
 *
 *  Format of the data :
 *  Byte 0   1            5   6      7   8                   9
 *       -----------------------------------------------------
 *  Size | 1 |      4     | 1 |   1  | 1 |       1           |
 *  Data | 4 | priv token | 1 | laps | 1 | track number (gp) |
 *       -----------------------------------------------------
 */
void ServerLobbyRoomProtocol::playerLapsVote(Event* event)
{
    NetworkString &data = event->data();
    STKPeer* peer = event->getPeer();
    if (!checkDataSizeAndToken(event, 9))
        return;
    if (!isByteCorrect(event, 5, 1))
        return;
    if (!isByteCorrect(event, 7, 1))
        return;
    uint8_t player_id = peer->getPlayerProfile()->getPlayerID();
    m_setup->getRaceConfig()->setPlayerLapsVote(player_id, data[6], data[8]);
    // Send the vote to everybody (including the sender)
    data.removeFront(5); // remove the token
    NetworkString other(2+data.size());
    other.ai8(1).ai8(player_id); // add the player id
    other += data; // add the data
    NetworkString prefix(1);
    prefix.ai8(LE_VOTE_LAPS); // prefix the token with the ype
    sendMessageToPeersChangingToken(prefix, other);
}   // playerLapsVote

//-----------------------------------------------------------------------------
