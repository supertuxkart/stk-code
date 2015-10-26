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
#include "network/network_world.hpp"
#include "network/protocols/get_public_address.hpp"
#include "network/protocols/show_public_address.hpp"
#include "network/protocols/connect_to_peer.hpp"
#include "network/protocols/start_server.hpp"
#include "network/protocols/start_game_protocol.hpp"
#include "network/protocol_manager.hpp"
#include "network/server_console.hpp"
#include "network/stk_host.hpp"
#include "network/stk_peer.hpp"
#include "online/online_profile.hpp"
#include "online/request_manager.hpp"
#include "utils/log.hpp"
#include "utils/random_generator.hpp"
#include "utils/time.hpp"


ServerLobbyRoomProtocol::ServerLobbyRoomProtocol() : LobbyRoomProtocol(NULL)
{
}

//-----------------------------------------------------------------------------

ServerLobbyRoomProtocol::~ServerLobbyRoomProtocol()
{
}

//-----------------------------------------------------------------------------

void ServerLobbyRoomProtocol::setup()
{
    m_setup = STKHost::get()->setupNewGame(); // create a new setup
    m_setup->getRaceConfig()->setPlayerCount(16); //FIXME : this has to be moved to when logging into the server
    m_next_id = 0;
    m_state = NONE;
    m_selection_enabled = false;
    m_in_race = false;
    Log::info("ServerLobbyRoomProtocol", "Starting the protocol.");
}

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
        Log::info("ServerLobbyRoomProtocol", "Message received with type %d.", message_type);
        if (message_type == 0x01) // player requesting connection
            connectionRequested(event);
        else if (message_type == 0x02) // player requesting kart selection
            kartSelectionRequested(event);
        else if (message_type == 0xc0) // vote for major mode
            playerMajorVote(event);
        else if (message_type == 0xc1) // vote for race count
            playerRaceCountVote(event);
        else if (message_type == 0xc2) // vote for minor mode
            playerMinorVote(event);
        else if (message_type == 0xc3) // vote for track
            playerTrackVote(event);
        else if (message_type == 0xc4) // vote for reversed mode
            playerReversedVote(event);
        else if (message_type == 0xc5) // vote for laps
            playerLapsVote(event);
    } // if (event->getType() == EVENT_TYPE_MESSAGE)
    else if (event->getType() == EVENT_TYPE_CONNECTED)
    {
    } // if (event->getType() == EVENT_TYPE_CONNECTED)
    else if (event->getType() == EVENT_TYPE_DISCONNECTED)
    {
        kartDisconnected(event);
    } // if (event->getType() == EVENT_TYPE_DISCONNECTED)
    return true;
}

//-----------------------------------------------------------------------------

void ServerLobbyRoomProtocol::update()
{
    switch (m_state)
    {
    case NONE:
        m_current_protocol = new GetPublicAddress();
        m_current_protocol->requestStart();
        m_state = GETTING_PUBLIC_ADDRESS;
        break;
    case GETTING_PUBLIC_ADDRESS:
        if (m_current_protocol->getState() == PROTOCOL_STATE_TERMINATED)
        {
            m_current_protocol = new StartServer();
            m_current_protocol->requestStart();
            m_state = LAUNCHING_SERVER;
            Log::debug("ServerLobbyRoomProtocol", "Public address known.");
        }
        break;
    case LAUNCHING_SERVER:
        if (m_current_protocol->getState() == PROTOCOL_STATE_TERMINATED)
        {
            m_state = WORKING;
            Log::info("ServerLobbyRoomProtocol", "Server setup");
        }
        break;
    case WORKING:
    {
        checkIncomingConnectionRequests();
        if (m_in_race && World::getWorld() &&
            NetworkWorld::getInstance<NetworkWorld>()->isRunning())
        {
            checkRaceFinished();
        }

        break;
    }
    case DONE:
        m_state = EXITING;
        requestTerminate();
        break;
    case EXITING:
        break;
    }
}

//-----------------------------------------------------------------------------

void ServerLobbyRoomProtocol::startGame()
{
    const std::vector<STKPeer*> &peers = STKHost::get()->getPeers();
    for (unsigned int i = 0; i < peers.size(); i++)
    {
        NetworkString ns(6);
        ns.ai8(0x04).ai8(4).ai32(peers[i]->getClientServerToken()); // start game
        sendMessage(peers[i], ns, true); // reliably
    }
    Protocol *p = new StartGameProtocol(m_setup);
    p->requestStart();
    m_in_race = true;
}   // startGame

//-----------------------------------------------------------------------------

void ServerLobbyRoomProtocol::startSelection()
{
    const std::vector<STKPeer*> &peers = STKHost::get()->getPeers();
    for (unsigned int i = 0; i < peers.size(); i++)
    {
        NetworkString ns(6);
        ns.ai8(0x05).ai8(4).ai32(peers[i]->getClientServerToken()); // start selection
        sendMessage(peers[i], ns, true); // reliably
    }
    m_selection_enabled = true;
}   // startSelection

//-----------------------------------------------------------------------------

void ServerLobbyRoomProtocol::checkIncomingConnectionRequests()
{
    // first poll every 5 seconds
    static double last_poll_time = 0;
    if (StkTime::getRealTime() > last_poll_time+10.0)
    {
        last_poll_time = StkTime::getRealTime();
        const TransportAddress &addr = STKHost::get()->getPublicAddress();
        Online::XMLRequest* request = new Online::XMLRequest();
        PlayerManager::setUserDetails(request, "poll-connection-requests", Online::API::SERVER_PATH);

        request->addParameter("address", addr.getIP());
        request->addParameter("port", addr.getPort());

        request->executeNow();
        assert(request->isDone());

        const XMLNode * result = request->getXMLData();
        std::string rec_success;

        if(result->get("success", &rec_success))
        {
            if(rec_success == "yes")
            {
                const XMLNode * users_xml = result->getNode("users");
                uint32_t id = 0;
                for (unsigned int i = 0; i < users_xml->getNumNodes(); i++)
                {
                    users_xml->getNode(i)->get("id", &id);
                    Log::debug("ServerLobbyRoomProtocol", "User with id %d wants to connect.", id);
                    m_incoming_peers_ids.push_back(id);
                }
            }
            else
            {
                Log::error("ServerLobbyRoomProtocol", "Error while reading the list.");
            }
        }
        else
        {
            Log::error("ServerLobbyRoomProtocol", "Cannot retrieve the list.");
        }
        delete request;
    }

    // now
    for (unsigned int i = 0; i < m_incoming_peers_ids.size(); i++)
    {
        Protocol *p = new ConnectToPeer(m_incoming_peers_ids[i]);
        p->requestStart();
    }
    m_incoming_peers_ids.clear();
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
            Log::info("ServerLobbyRoomProtocol", "Kart %d finished #%d", karts_results[i], i+1);
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
        Protocol* protocol = NULL;
        protocol = ProtocolManager::getInstance()->getProtocol(PROTOCOL_CONTROLLER_EVENTS);
        if (protocol)
            protocol->requestTerminate();
        else
            Log::error("ClientLobbyRoomProtocol", "No controller events protocol registered.");

        protocol = ProtocolManager::getInstance()->getProtocol(PROTOCOL_KART_UPDATE);
        if (protocol)
            protocol->requestTerminate();
        else
            Log::error("ClientLobbyRoomProtocol", "No kart update protocol registered.");

        protocol = ProtocolManager::getInstance()->getProtocol(PROTOCOL_GAME_EVENTS);
        if (protocol)
            protocol->requestTerminate();
        else
            Log::error("ClientLobbyRoomProtocol", "No game events protocol registered.");

        // notify the network world that it is stopped
        NetworkWorld::getInstance()->stop();
        // exit the race now
        race_manager->exitRace();
        race_manager->setAIKartOverride("");
    }
    else
    {
        //Log::info("ServerLobbyRoomProtocol", "Phase is %d", World::getWorld()->getPhase());
    }
}   // checkRaceFinished

//-----------------------------------------------------------------------------

void ServerLobbyRoomProtocol::kartDisconnected(Event* event)
{
    STKPeer* peer = event->getPeer();
    if (peer->getPlayerProfile() != NULL) // others knew him
    {
        NetworkString msg(3);
        msg.ai8(0x02).ai8(1).ai8(peer->getPlayerProfile()->race_id);
        sendMessage(msg);
        Log::info("ServerLobbyRoomProtocol", "Player disconnected : id %d",
                  peer->getPlayerProfile()->race_id);
        m_setup->removePlayer(peer->getPlayerProfile()->race_id);
        STKHost::get()->removePeer(peer);
    }
    else
        Log::info("ServerLobbyRoomProtocol", "The DC peer wasn't registered.");
}

//-----------------------------------------------------------------------------

/*! \brief Called when a player asks for a connection.
 *  \param event : Event providing the information.
 *
 *  Format of the data :
 *  Byte 0   1                  5
 *       ------------------------
 *  Size | 1 |          4       |
 *  Data | 4 | global player id |
 *       ------------------------
 */
void ServerLobbyRoomProtocol::connectionRequested(Event* event)
{
    STKPeer* peer = event->getPeer();
    const NetworkString &data = event->data();
    if (data.size() != 5 || data[0] != 4)
    {
        Log::warn("ServerLobbyRoomProtocol", "Receiving badly formated message. Size is %d and first byte %d", data.size(), data[0]);
        return;
    }
    uint32_t player_id = 0;
    player_id = data.getUInt32(1);
    // can we add the player ?
    if (m_setup->getPlayerCount() < STKHost::getMaxPlayers()) //accept
    {
        // add the player to the game setup
        m_next_id = m_setup->getPlayerCount();
        // notify everybody that there is a new player
        NetworkString message(8);
        // new player (1) -- size of id -- id -- size of local id -- local id;
        message.ai8(1).ai8(4).ai32(player_id).ai8(1).ai8(m_next_id);
        ProtocolManager::getInstance()->sendMessageExcept(this, peer, message);

        /// now answer to the peer that just connected
        RandomGenerator token_generator;
        // use 4 random numbers because rand_max is probably 2^15-1.
        uint32_t token = (uint32_t)(((token_generator.get(RAND_MAX)<<24) & 0xff) +
                                    ((token_generator.get(RAND_MAX)<<16) & 0xff) +
                                    ((token_generator.get(RAND_MAX)<<8)  & 0xff) +
                                    ((token_generator.get(RAND_MAX)      & 0xff)));

        std::vector<NetworkPlayerProfile*> players = m_setup->getPlayers();
        // send a message to the one that asked to connect
        // Size is overestimated, probably one player's data will not be sent
        NetworkString message_ack(13+players.size()*7);
        // connection success (129) -- size of token -- token
        message_ack.ai8(0x81).ai8(1).ai8(m_next_id).ai8(4).ai32(token).ai8(4).ai32(player_id);
        // add all players so that this user knows
        for (unsigned int i = 0; i < players.size(); i++)
        {
            // do not duplicate the player into the message
            if (players[i]->race_id != m_next_id && players[i]->user_profile->getID() != player_id)
                message_ack.ai8(1).ai8(players[i]->race_id).ai8(4).ai32(players[i]->user_profile->getID());
        }
        sendMessage(peer, message_ack);

        peer->setClientServerToken(token);

        NetworkPlayerProfile* profile = new NetworkPlayerProfile();
        profile->race_id = m_next_id;
        profile->kart_name = "";
        profile->user_profile = new Online::OnlineProfile(player_id, "");
        m_setup->addPlayer(profile);
        peer->setPlayerProfile(profile);
        Log::verbose("ServerLobbyRoomProtocol", "New player.");
    } // accept player
    else  // refuse the connection with code 0 (too much players)
    {
        NetworkString message(3);
        message.ai8(0x80);            // 128 means connection refused
        message.ai8(1);               // 1 bytes for the error code
        message.ai8(0);               // 0 = too much players
        // send only to the peer that made the request
        sendMessage(peer, message);
        Log::verbose("ServerLobbyRoomProtocol", "Player refused");
    }
}

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
    const NetworkString &data = event->data();
    STKPeer* peer = event->getPeer();
    if (!checkDataSizeAndToken(event, 6))
        return;

    uint8_t kart_name_size = data.gui8(5);
    std::string kart_name = data.getString(6, kart_name_size);
    if (kart_name.size() != kart_name_size)
    {
        Log::error("ServerLobbyRoomProtocol", "Kart names sizes differ: told:"
                   "%d, real: %d.", kart_name_size, kart_name.size());
        return;
    }
    // check if selection is possible
    if (!m_selection_enabled)
    {
        NetworkString answer(3);
        answer.ai8(0x82).ai8(1).ai8(2); // selection still not started
        sendMessage(peer, answer);
        return;
    }
    // check if somebody picked that kart
    if (!m_setup->isKartAvailable(kart_name))
    {
        NetworkString answer(3);
        answer.ai8(0x82).ai8(1).ai8(0); // kart is already taken
        sendMessage(peer, answer);
        return;
    }
    // check if this kart is authorized
    if (!m_setup->isKartAllowed(kart_name))
    {
        NetworkString answer(3);
        answer.ai8(0x82).ai8(1).ai8(1); // kart is not authorized
        sendMessage(peer, answer);
        return;
    }
    // send a kart update to everyone
    NetworkString answer(3+1+kart_name.size());
    // kart update (3), 1, race id
    answer.ai8(0x03).ai8(1).ai8(peer->getPlayerProfile()->race_id);
    //  kart name size, kart name
    answer.add(kart_name);
    sendMessage(answer);
    m_setup->setPlayerKart(peer->getPlayerProfile()->race_id, kart_name);
}

//-----------------------------------------------------------------------------

/*! \brief Called when a player votes for a major race mode.
 *  \param event : Event providing the information.
 *
 *  Format of the data :
 *  Byte 0   1            5   6                 7
 *       ----------------------------------------
 *  Size | 1 |      4     | 1 |        1        |
 *  Data | 4 | priv token | 1 | major mode vote |
 *       ----------------------------------------
 */
void ServerLobbyRoomProtocol::playerMajorVote(Event* event)
{
    NetworkString &data = event->data();
    STKPeer* peer = event->getPeer();
    if (!checkDataSizeAndToken(event, 7))
        return;
    if (!isByteCorrect(event, 5, 1))
        return;
    uint8_t player_id = peer->getPlayerProfile()->race_id;
    m_setup->getRaceConfig()->setPlayerMajorVote(player_id, data[6]);
    // Send the vote to everybody (including the sender)
    data.removeFront(5); // remove the token
    NetworkString other(2+data.size());
    other.ai8(1).ai8(player_id); // add the player id
    other += data; // add the data
    NetworkString prefix(1);
    prefix.ai8(0xc0); // prefix the token with the ype
    sendMessageToPeersChangingToken(prefix, other);
}
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
    uint8_t player_id = peer->getPlayerProfile()->race_id;
    m_setup->getRaceConfig()->setPlayerRaceCountVote(player_id, data[6]);
    // Send the vote to everybody (including the sender)
    data.removeFront(5); // remove the token
    NetworkString other(2+data.size());
    other.ai8(1).ai8(player_id); // add the player id
    other += data; // add the data
    NetworkString prefix(1);
    prefix.ai8(0xc1); // prefix the token with the type
    sendMessageToPeersChangingToken(prefix, other);
}
//-----------------------------------------------------------------------------

/*! \brief Called when a player votes for a minor race mode.
 *  \param event : Event providing the information.
 *
 *  Format of the data :
 *  Byte 0   1            5   6                 7
 *       ----------------------------------------
 *  Size | 1 |      4     | 1 |        1        |
 *  Data | 4 | priv token | 1 | minor mode vote |
 *       ----------------------------------------
 */
void ServerLobbyRoomProtocol::playerMinorVote(Event* event)
{
    NetworkString &data = event->data();
    STKPeer* peer = event->getPeer();
    if (!checkDataSizeAndToken(event, 7))
        return;
    if (!isByteCorrect(event, 5, 1))
        return;
    uint8_t player_id = peer->getPlayerProfile()->race_id;
    m_setup->getRaceConfig()->setPlayerMinorVote(player_id, data[6]);
    // Send the vote to everybody (including the sender)
    data.removeFront(5); // remove the token
    NetworkString other(2+data.size());
    other.ai8(1).ai8(player_id); // add the player id
    other += data; // add the data
    NetworkString prefix(1);
    prefix.ai8(0xc2); // prefix the token with the ype
    sendMessageToPeersChangingToken(prefix, other);
}
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
    int N = data[5];
    std::string track_name = data.getString(5, N);
    if (!isByteCorrect(event, N+6, 1))
        return;
    uint8_t player_id = peer->getPlayerProfile()->race_id;
    m_setup->getRaceConfig()->setPlayerTrackVote(player_id, track_name, data[N+7]);
    // Send the vote to everybody (including the sender)
    data.removeFront(5); // remove the token
    NetworkString other(2+data.size());
    other.ai8(1).ai8(player_id); // add the player id
    other += data; // add the data
    NetworkString prefix(1);
    prefix.ai8(0xc3); // prefix the token with the ype
    sendMessageToPeersChangingToken(prefix, other);
}
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
    uint8_t player_id = peer->getPlayerProfile()->race_id;
    m_setup->getRaceConfig()->setPlayerReversedVote(player_id, data[6]!=0, data[8]);
    // Send the vote to everybody (including the sender)
    data.removeFront(5); // remove the token
    NetworkString other(2+data.size());
    other.ai8(1).ai8(player_id); // add the player id
    other += data; // add the data
    NetworkString prefix(1);
    prefix.ai8(0xc4); // prefix the token with the ype
    sendMessageToPeersChangingToken(prefix, other);
}
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
    uint8_t player_id = peer->getPlayerProfile()->race_id;
    m_setup->getRaceConfig()->setPlayerLapsVote(player_id, data[6], data[8]);
    // Send the vote to everybody (including the sender)
    data.removeFront(5); // remove the token
    NetworkString other(2+data.size());
    other.ai8(1).ai8(player_id); // add the player id
    other += data; // add the data
    NetworkString prefix(1);
    prefix.ai8(0xc5); // prefix the token with the ype
    sendMessageToPeersChangingToken(prefix, other);
}
//-----------------------------------------------------------------------------
