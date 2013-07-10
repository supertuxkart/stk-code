#include "network/protocols/server_lobby_room_protocol.hpp"

#include "network/server_network_manager.hpp"
#include "network/protocols/get_public_address.hpp"
#include "network/protocols/show_public_address.hpp"
#include "network/protocols/connect_to_peer.hpp"
#include "network/protocols/start_server.hpp"

#include "online/current_online_user.hpp"
#include "online/http_connector.hpp"
#include "config/user_config.hpp"
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
    Log::info("ServerLobbyRoomProtocol", "Starting the protocol.");
}

//-----------------------------------------------------------------------------

void ServerLobbyRoomProtocol::notifyEvent(Event* event)
{
    assert(m_setup); // assert that the setup exists
    if (event->type == EVENT_TYPE_MESSAGE)
    {
        assert(event->data.size()); // message not empty
        uint8_t message_type;
        message_type = event->data.getAndRemoveUInt8();
        Log::info("ServerLobbyRoomProtocol", "Message received with type %d.", message_type);
        if (message_type == 1) // player requesting connection
        {
            if (event->data.size() != 5 || event->data[0] != 4)
            {
                Log::warn("LobbyRoomProtocol", "A player is sending a badly formated message. Size is %d and first byte %d", event->data.size(), event->data[0]);
                return;
            }
            Log::verbose("LobbyRoomProtocol", "New player.");
            int player_id = 0;
            player_id = event->data.getUInt32(1);
            // can we add the player ?
            if (m_setup->getPlayerCount() < 16) // accept player
            {
                // add the player to the game setup
                while(m_setup->getProfile(m_next_id)!=NULL)
                    m_next_id++;
                NetworkPlayerProfile profile;
                profile.race_id = m_next_id;
                profile.kart_name = "";
                profile.user_profile = new OnlineUser("Unnamed Player");
                m_setup->addPlayer(profile);
                // notify everybody that there is a new player
                NetworkString message;
                // new player (1) -- size of id -- id -- size of local id -- local id;
                message.ai8(1).ai8(4).ai32(player_id).ai8(1).ai8(m_next_id);
                m_listener->sendMessageExcept(this, event->peer, message);
                // send a message to the one that asked to connect
                NetworkString message_ack;
                // 0b10000001 (connection success) ;
                RandomGenerator token_generator;
                // use 4 random numbers because rand_max is probably 2^15-1.
                uint32_t token = (uint32_t)(((token_generator.get(RAND_MAX)<<24) & 0xff) +
                                            ((token_generator.get(RAND_MAX)<<16) & 0xff) +
                                            ((token_generator.get(RAND_MAX)<<8)  & 0xff) +
                                            ((token_generator.get(RAND_MAX)      & 0xff)));
                // connection success (129) -- size of token -- token
                message_ack.ai8(0x81).ai8(1).ai8(m_next_id).ai8(4).ai32(token).ai8(4).ai32(player_id);
                m_listener->sendMessage(this, event->peer, message_ack);
            } // accept player
            else  // refuse the connection with code 0 (too much players)
            {
                NetworkString message;
                message.ai8(0x80);            // 128 means connection refused
                message.ai8(1);               // 1 bytes for the error code
                message.ai8(0);               // 0 = too much players
                // send only to the peer that made the request
                m_listener->sendMessage(this, event->peer, message);
            }
        }
    } // if (event->type == EVENT_TYPE_MESSAGE)
    else if (event->type == EVENT_TYPE_CONNECTED)
    {
    } // if (event->type == EVENT_TYPE_CONNECTED)
    else if (event->type == EVENT_TYPE_DISCONNECTED)
    {

    } // if (event->type == EVENT_TYPE_DISCONNECTED)
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
        // first poll every 5 seconds
        static double last_poll_time = 0;
        if (Time::getRealTime() > last_poll_time+10.0)
        {
            last_poll_time = Time::getRealTime();
            TransportAddress addr = NetworkManager::getInstance()->getPublicAddress();
            HTTPConnector * connector = new HTTPConnector((std::string)UserConfigParams::m_server_multiplayer + "address-management.php");
            connector->setParameter("id",CurrentOnlineUser::get()->getUserID());
            connector->setParameter("token",CurrentOnlineUser::get()->getToken());
            connector->setParameter("address",addr.ip);
            connector->setParameter("port",addr.port);
            connector->setParameter("action","poll-connection-requests");

            const XMLNode * result = connector->getXMLFromPage();
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
        }

        // now
        for (unsigned int i = 0; i < m_incoming_peers_ids.size(); i++)
        {
            m_listener->requestStart(new ConnectToPeer(m_incoming_peers_ids[i]));
        }
        m_incoming_peers_ids.clear();

        break;
    }
    case DONE:
        m_listener->requestTerminate(this);
        break;
    }
}
//-----------------------------------------------------------------------------

