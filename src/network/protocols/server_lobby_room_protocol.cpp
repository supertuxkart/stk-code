//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2013 SuperTuxKart-Team
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

#include "network/server_network_manager.hpp"
#include "network/network_world.hpp"
#include "network/protocols/get_public_address.hpp"
#include "network/protocols/show_public_address.hpp"
#include "network/protocols/connect_to_peer.hpp"
#include "network/protocols/start_server.hpp"
#include "network/protocols/start_game_protocol.hpp"

#include "online/current_user.hpp"
#include "online/http_manager.hpp"
#include "config/user_config.hpp"
#include "modes/world.hpp"
#include "utils/log.hpp"
#include "utils/time.hpp"
#include "utils/random_generator.hpp"

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
    m_setup = NetworkManager::getInstance()->setupNewGame(); // create a new setup
    m_next_id = 0;
    m_state = NONE;
    m_public_address.ip = 0;
    m_public_address.port = 0;
    m_selection_enabled = false;
    m_in_race = false;
    Log::info("ServerLobbyRoomProtocol", "Starting the protocol.");
}

//-----------------------------------------------------------------------------

bool ServerLobbyRoomProtocol::notifyEventAsynchronous(Event* event)
{
    assert(m_setup); // assert that the setup exists
    if (event->type == EVENT_TYPE_MESSAGE)
    {
        NetworkString data = event->data();
        assert(data.size()); // message not empty
        uint8_t message_type;
        message_type = data[0];
        event->removeFront(1);
        Log::info("ServerLobbyRoomProtocol", "Message received with type %d.", message_type);
        if (message_type == 0x01) // player requesting connection
            connectionRequested(event);
        if (message_type == 0x02) // player requesting kart selection
            kartSelectionRequested(event);
    } // if (event->type == EVENT_TYPE_MESSAGE)
    else if (event->type == EVENT_TYPE_CONNECTED)
    {
    } // if (event->type == EVENT_TYPE_CONNECTED)
    else if (event->type == EVENT_TYPE_DISCONNECTED)
    {
        kartDisconnected(event);
    } // if (event->type == EVENT_TYPE_DISCONNECTED)
    return true;
}

//-----------------------------------------------------------------------------

void ServerLobbyRoomProtocol::update()
{
    switch (m_state)
    {
    case NONE:
        m_current_protocol_id = m_listener->requestStart(new GetPublicAddress(&m_public_address));
        m_state = GETTING_PUBLIC_ADDRESS;
        break;
    case GETTING_PUBLIC_ADDRESS:
        if (m_listener->getProtocolState(m_current_protocol_id) == PROTOCOL_STATE_TERMINATED)
        {
            NetworkManager::getInstance()->setPublicAddress(m_public_address);
            m_current_protocol_id = m_listener->requestStart(new StartServer());
            m_state = LAUNCHING_SERVER;
            Log::debug("ServerLobbyRoomProtocol", "Public address known.");
        }
        break;
    case LAUNCHING_SERVER:
        if (m_listener->getProtocolState(m_current_protocol_id) == PROTOCOL_STATE_TERMINATED)
        {
            m_state = WORKING;
            Log::info("ServerLobbyRoomProtocol", "Server setup");
        }
        break;
    case WORKING:
    {
        checkIncomingConnectionRequests();
        if (m_in_race && World::getWorld() && NetworkWorld::getInstance<NetworkWorld>()->isRunning())
            checkRaceFinished();

        break;
    }
    case DONE:
        m_state = EXITING;
        m_listener->requestTerminate(this);
        break;
    case EXITING:
        break;
    }
}

//-----------------------------------------------------------------------------

void ServerLobbyRoomProtocol::startGame()
{
    std::vector<STKPeer*> peers = NetworkManager::getInstance()->getPeers();
    for (unsigned int i = 0; i < peers.size(); i++)
    {
        NetworkString ns;
        ns.ai8(0x04).ai8(4).ai32(peers[i]->getClientServerToken()); // start game
        m_listener->sendMessage(this, peers[i], ns, true); // reliably
    }
    m_listener->requestStart(new StartGameProtocol(m_setup));
    m_in_race = true;
}

//-----------------------------------------------------------------------------

void ServerLobbyRoomProtocol::startSelection()
{
    std::vector<STKPeer*> peers = NetworkManager::getInstance()->getPeers();
    for (unsigned int i = 0; i < peers.size(); i++)
    {
        NetworkString ns;
        ns.ai8(0x05).ai8(4).ai32(peers[i]->getClientServerToken()); // start selection
        m_listener->sendMessage(this, peers[i], ns, true); // reliably
    }
    m_selection_enabled = true;
}

//-----------------------------------------------------------------------------

void ServerLobbyRoomProtocol::checkIncomingConnectionRequests()
{
    // first poll every 5 seconds
    static double last_poll_time = 0;
    if (Time::getRealTime() > last_poll_time+10.0)
    {
        last_poll_time = Time::getRealTime();
        TransportAddress addr = NetworkManager::getInstance()->getPublicAddress();
        Online::XMLRequest* request = new Online::XMLRequest();
        request->setURL((std::string)UserConfigParams::m_server_multiplayer + "address-management.php");
        request->setParameter("id",Online::CurrentUser::get()->getUserID());
        request->setParameter("token",Online::CurrentUser::get()->getToken());
        request->setParameter("address",addr.ip);
        request->setParameter("port",addr.port);
        request->setParameter("action","poll-connection-requests");

        Online::HTTPManager::get()->synchronousRequest(request);
        assert(request->isDone());

        const XMLNode * result = request->getResult();
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
        m_listener->requestStart(new ConnectToPeer(m_incoming_peers_ids[i]));
    }
    m_incoming_peers_ids.clear();
}

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
        for (int j = 0; j < num_players; j++)
        {
            float lowest_time = race_manager->getKartRaceTime(0);
            int lowest_index = 0;
            for (int i = 0; i < num_players; i++)
            {
                float time = race_manager->getKartRaceTime(i);
                if (time < lowest_time)
                {
                    lowest_time = time;
                    lowest_index = i;
                }
            }
            karts_results.push_back(lowest_index);
        }

        std::vector<STKPeer*> peers = NetworkManager::getInstance()->getPeers();

        NetworkString queue;
        for (unsigned int i = 0; i < karts_results.size(); i++)
        {
            queue.ai8(1).ai8(karts_results[i]); // position is i+1
        }
        for (unsigned int i = 0; i < peers.size(); i++)
        {
            NetworkString ns;
            ns.ai8(0x06).ai8(4).ai32(peers[i]->getClientServerToken());
            NetworkString total = ns + queue;
            m_listener->sendMessage(this, peers[i], total, true);
        }
        Log::info("ServerLobbyRoomProtocol", "End of game message sent");
        m_in_race = false;

        // stop race protocols
        Protocol* protocol = NULL;
        protocol = m_listener->getProtocol(PROTOCOL_CONTROLLER_EVENTS);
        if (protocol)
            m_listener->requestTerminate(protocol);
        else
            Log::error("ClientLobbyRoomProtocol", "No controller events protocol registered.");

        protocol = m_listener->getProtocol(PROTOCOL_KART_UPDATE);
        if (protocol)
            m_listener->requestTerminate(protocol);
        else
            Log::error("ClientLobbyRoomProtocol", "No kart update protocol registered.");

        protocol = m_listener->getProtocol(PROTOCOL_GAME_EVENTS);
        if (protocol)
            m_listener->requestTerminate(protocol);
        else
            Log::error("ClientLobbyRoomProtocol", "No game events protocol registered.");

        // notify the network world that it is stopped
        NetworkWorld::getInstance()->stop();
    }
    else
    {
        //Log::info("ServerLobbyRoomProtocol", "Phase is %d", World::getWorld()->getPhase());
    }
}

//-----------------------------------------------------------------------------

void ServerLobbyRoomProtocol::kartDisconnected(Event* event)
{
    STKPeer* peer = *(event->peer);
    if (peer->getPlayerProfile() != NULL) // others knew him
    {
        NetworkString msg;
        msg.ai8(0x02).ai8(1).ai8(peer->getPlayerProfile()->race_id);
        m_listener->sendMessage(this, msg);
        Log::info("ServerLobbyRoomProtocol", "Player disconnected : id %d",
                  peer->getPlayerProfile()->race_id);
        m_setup->removePlayer(peer->getPlayerProfile()->race_id);
        NetworkManager::getInstance()->removePeer(peer);
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
    STKPeer* peer = *(event->peer);
    NetworkString data = event->data();
    if (data.size() != 5 || data[0] != 4)
    {
        Log::warn("ServerLobbyRoomProtocol", "Receiving badly formated message. Size is %d and first byte %d", data.size(), data[0]);
        return;
    }
    uint32_t player_id = 0;
    player_id = data.getUInt32(1);
    // can we add the player ?
    if (m_setup->getPlayerCount() < 16) // accept player
    {
        // add the player to the game setup
        m_next_id = m_setup->getPlayerCount();
        // notify everybody that there is a new player
        NetworkString message;
        // new player (1) -- size of id -- id -- size of local id -- local id;
        message.ai8(1).ai8(4).ai32(player_id).ai8(1).ai8(m_next_id);
        m_listener->sendMessageExcept(this, peer, message);

        /// now answer to the peer that just connected
        RandomGenerator token_generator;
        // use 4 random numbers because rand_max is probably 2^15-1.
        uint32_t token = (uint32_t)(((token_generator.get(RAND_MAX)<<24) & 0xff) +
                                    ((token_generator.get(RAND_MAX)<<16) & 0xff) +
                                    ((token_generator.get(RAND_MAX)<<8)  & 0xff) +
                                    ((token_generator.get(RAND_MAX)      & 0xff)));

        // send a message to the one that asked to connect
        NetworkString message_ack;
        // connection success (129) -- size of token -- token
        message_ack.ai8(0x81).ai8(1).ai8(m_next_id).ai8(4).ai32(token).ai8(4).ai32(player_id);
        // add all players so that this user knows
        std::vector<NetworkPlayerProfile*> players = m_setup->getPlayers();
        for (unsigned int i = 0; i < players.size(); i++)
        {
            // do not duplicate the player into the message
            if (players[i]->race_id != m_next_id && players[i]->user_profile->getUserID() != player_id)
                message_ack.ai8(1).ai8(players[i]->race_id).ai8(4).ai32(players[i]->user_profile->getUserID());
        }
        m_listener->sendMessage(this, peer, message_ack);

        peer->setClientServerToken(token);

        NetworkPlayerProfile* profile = new NetworkPlayerProfile();
        profile->race_id = m_next_id;
        profile->kart_name = "";
        profile->user_profile = new Online::User(player_id);
        m_setup->addPlayer(profile);
        peer->setPlayerProfile(profile);
        Log::verbose("ServerLobbyRoomProtocol", "New player.");
    } // accept player
    else  // refuse the connection with code 0 (too much players)
    {
        NetworkString message;
        message.ai8(0x80);            // 128 means connection refused
        message.ai8(1);               // 1 bytes for the error code
        message.ai8(0);               // 0 = too much players
        // send only to the peer that made the request
        m_listener->sendMessage(this, peer, message);
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
    NetworkString data = event->data();
    if (data.size() < 6 || data[0] != 4)
    {
        Log::warn("ServerLobbyRoomProtocol", "Receiving a badly "
                  "formated message. Size is %d and first byte %d",
                  data.size(), data[0]);
        return;
    }
    STKPeer* peer = *(event->peer);
    uint32_t token = data.gui32(1);
    if (token != peer->getClientServerToken())
    {
        Log::warn("ServerLobbyRoomProtocol", "Peer sending bad token. Request "
                  "aborted.");
        return;
    }
    uint8_t kart_name_size = data.gui8(5);
    std::string kart_name = data.gs(6, kart_name_size);
    if (kart_name.size() != kart_name_size)
    {
        Log::error("ServerLobbyRoomProtocol", "Kart names sizes differ: told:"
                   "%d, real: %d.", kart_name_size, kart_name.size());
        return;
    }
    // check if selection is possible
    if (!m_selection_enabled)
    {
        NetworkString answer;
        answer.ai8(0x82).ai8(1).ai8(2); // selection still not started
        m_listener->sendMessage(this, peer, answer);
        return;
    }
    // check if somebody picked that kart
    if (!m_setup->isKartAvailable(kart_name))
    {
        NetworkString answer;
        answer.ai8(0x82).ai8(1).ai8(0); // kart is already taken
        m_listener->sendMessage(this, peer, answer);
        return;
    }
    // check if this kart is authorized
    if (!m_setup->isKartAllowed(kart_name))
    {
        NetworkString answer;
        answer.ai8(0x82).ai8(1).ai8(1); // kart is not authorized
        m_listener->sendMessage(this, peer, answer);
        return;
    }
    // send a kart update to everyone
    NetworkString answer;
    // kart update (3), 1, race id
    answer.ai8(0x03).ai8(1).ai8(peer->getPlayerProfile()->race_id);
    //  kart name size, kart name
    answer.ai8(kart_name.size()).as(kart_name);
    m_listener->sendMessage(this, answer);
    m_setup->setPlayerKart(peer->getPlayerProfile()->race_id, kart_name);
}

//-----------------------------------------------------------------------------
