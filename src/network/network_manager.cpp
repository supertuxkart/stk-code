//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2008 Joerg Henrichs, Stephen Leak
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

#include "network/network_manager.hpp"

#include "config/stk_config.hpp"
#include "config/user_config.hpp"
#include "karts/kart_properties_manager.hpp"
#include "modes/world.hpp"
#include "network/connect_message.hpp"
#include "network/character_info_message.hpp"
#include "network/character_selected_message.hpp"
#include "network/race_info_message.hpp"
#include "network/race_start_message.hpp"
#include "network/world_loaded_message.hpp"
#include "network/race_state.hpp"
#include "network/kart_control_message.hpp"
#include "network/character_confirm_message.hpp"
#include "network/race_result_message.hpp"
#include "network/race_result_ack_message.hpp"
#include "race/race_manager.hpp"

NetworkManager* network_manager = 0;

NetworkManager::NetworkManager()
{
     m_mode           = NW_NONE;
     m_state          = NS_ACCEPT_CONNECTIONS;
     m_host           = NULL;

     m_num_clients    = 0;
     m_host_id        = 0;

     if (enet_initialize () != 0)
     {
      fprintf (stderr, "An error occurred while initializing ENet.\n");
      exit(-1);
     }
}   // NetworkManager

// -----------------------------------------------------------------------------
bool NetworkManager::initialiseConnections()
{
     switch(m_mode)
     {
     case NW_NONE:   return true;
     case NW_CLIENT: return initClient();
     case NW_SERVER: return initServer();
     }
     return true;
}   // NetworkManager

// -----------------------------------------------------------------------------
NetworkManager::~NetworkManager()
{
     if(m_mode==NW_SERVER || m_mode==NW_CLIENT) enet_host_destroy(m_host);
     enet_deinitialize(); 
}   // ~NetworkManager

// -----------------------------------------------------------------------------
bool NetworkManager::initServer()
{
     ENetAddress address;
     address.host = ENET_HOST_ANY;
     address.port = UserConfigParams::m_server_port;

     m_host = enet_host_create (& address     /* the address to bind the server host to */, 
                                stk_config->m_max_karts /* number of connections */,
                                0             /* channel limit */,
                                0             /* incoming bandwidth */,
                                0             /* outgoing bandwidth */     );
    if (m_host == NULL)
    {
        fprintf (stderr, 
                 "An error occurred while trying to create an ENet server host.\n"
                 "Progressing in non-network mode\n");
        m_mode = NW_NONE;
        return false;
    }

    m_server = NULL;
    m_clients.push_back(NULL); // server has host_id=0, so put a dummy entry at 0 in client array

    m_client_names.push_back("server");
    return true;
}   // initServer

// -----------------------------------------------------------------------------
/** Initialises the client. This function tries to connect to the server. 
 */
bool NetworkManager::initClient()
{
    m_host = enet_host_create (NULL /* create a client host */,
                               1    /* only allow 1 outgoing connection */,
                               0    /* channel limit */,
                               0    /* downstream bandwidth unlimited   */,
                               0    /*  upstream bandwidth unlimited    */ );
    
    if (m_host == NULL)
    {
        fprintf (stderr, 
            "An error occurred while trying to create an ENet client host.\n");
        return false;
    }

    ENetAddress address;
    ENetEvent event;
    ENetPeer *peer;

    enet_address_set_host (& address, UserConfigParams::m_server_address.c_str());
    address.port = UserConfigParams::m_server_port;

    /* Initiate the connection, allocating the two channels 0 and 1. */
    peer = enet_host_connect (m_host, &address, 2, 0);
    
    if (peer == NULL)
    {
        fprintf(stderr, 
                "No available peers for initiating an ENet connection.\n");
        return false;
    }
    
    /* Wait up to 5 seconds for the connection attempt to succeed. */
    if (enet_host_service (m_host, & event, 5000) <= 0 ||
        event.type != ENET_EVENT_TYPE_CONNECT)
    {
        /* Either the 5 seconds are up or a disconnect event was */
        /* received. Reset the peer in the event the 5 seconds   */
        /* had run out without any significant event.            */
        enet_peer_reset (peer);

        fprintf(stderr, "Connection to '%s:%d' failed.\n",
                UserConfigParams::m_server_address.c_str(), (int)UserConfigParams::m_server_port);
        return false;
    }
    m_server = peer;
    return true;
}  // initClient

// ----------------------------------------------------------------------------
/** Switches the network manager to client mode. This function sets the state 
 *  to waiting_for_chars (so that the message from the server containing all
 *  available characters can be received).
 */
void NetworkManager::becomeClient()
{
    m_mode  = NW_CLIENT;
    m_state = NS_WAIT_FOR_AVAILABLE_CHARACTERS;
}   // becomeClient

// ----------------------------------------------------------------------------
/** Switches the network manager to server mode. This function sets the state
 *  to accepting connections.
 */
void NetworkManager::becomeServer()
{
    m_mode  = NW_SERVER;
    m_state = NS_ACCEPT_CONNECTIONS;
}   // becomeServer

// ----------------------------------------------------------------------------
/** Called in case of an error, to switch back to non-networking mode.
*/
void NetworkManager::disableNetworking()
{
    m_mode=NW_NONE;
    if (m_host != NULL)
    {
        enet_host_destroy(m_host);
        m_host = NULL;
    }
    // FIXME: what other enet data structures do we have to free/reset???

}   // disableNetworking

// ----------------------------------------------------------------------------
void NetworkManager::handleNewConnection(ENetEvent *event)
{
    // Only accept while waiting for connections
    if(m_state!=NS_ACCEPT_CONNECTIONS) return;

    // The logical connection (from STK point of view) happens when
    // the connection message is received. But for now reserve the
    // space in the data structures (e.g. in case that two connects
    // happen before a connect message is received
    m_client_names.push_back("NOT SET YET");
    m_clients.push_back(event->peer);
    event->peer->data = (void*)int(m_clients.size()-1);  // save hostid in peer data

}   // handleNewConnection

// ----------------------------------------------------------------------------
void NetworkManager::handleDisconnection(ENetEvent *event)
{
    if(m_state!=NS_ACCEPT_CONNECTIONS)
    {
        fprintf(stderr, "Disconnect while in race - close your eyes and hope for the best.\n");
        return;
    }
    fprintf(stderr, "%x:%d disconnected (host id %d).\n", event->peer->address.host,
        event->peer->address.port, (int)(long)event->peer->data );
    m_num_clients--;
}   // handleDisconnection

// ----------------------------------------------------------------------------
void NetworkManager::handleMessageAtServer(ENetEvent *event)
{  // handle message at server (from client)

    switch(m_state)
    {
    case NS_ACCEPT_CONNECTIONS:
        {
            ConnectMessage m(event->packet);
            m_client_names[(int)(long)event->peer->data] = m.getId();
            m_num_clients++;
            return;
        }
    case NS_KART_CONFIRMED:    // Fall through
    case NS_CHARACTER_SELECT:
        {
            CharacterSelectedMessage m(event->packet);
            unsigned int hostid=(unsigned int)(long)event->peer->data;
            assert(hostid>=1 && hostid<=m_num_clients);
            if(m_num_local_players[hostid]==-1)  // first package from that host
            {
                m_num_local_players[hostid] = m.getNumPlayers();
                m_num_all_players          += m.getNumPlayers();
                // count how many hosts have sent (at least) one message
                m_barrier_count ++;
            }
            RemoteKartInfo ki=m.getKartInfo();
            ki.setHostId(hostid);
            m_kart_info.push_back(ki);

            int kart_id = kart_properties_manager->getKartId(ki.getKartName());
            kart_properties_manager->testAndSetKart(kart_id);
            // TODO - character selection screen in networking
            /*
            CharSel *menu = dynamic_cast<CharSel*>(menu_manager->getCurrentMenu());
            if(menu)
                menu->updateAvailableCharacters();
             */
            
            // Broadcast the information about a selected kart to all clients
            CharacterConfirmMessage ccm(ki.getKartName(), hostid);
            broadcastToClients(ccm);
            // See if this was the last message, i.e. we have received at least
            // one message from each client, and the size of the kart_info
            // array is the same as the number of all players (which does not
            // yet include the number of players on the host).
            if(m_barrier_count == (int)m_num_clients &&
                m_num_all_players==(int)m_kart_info.size())
            {
                // we can't send the race info yet, since the server might
                // not yet have selected all characters!
                m_state = NS_ALL_REMOTE_CHARACTERS_DONE;
            }
            break;
        }
    case NS_READY_SET_GO_BARRIER:
        {
            m_barrier_count ++;
            if(m_barrier_count==(int)m_num_clients)
            {
                m_state = NS_RACING;
                RaceStartMessage m;
                broadcastToClients(m);
            }
            break;
        }
    case NS_RACE_RESULT_BARRIER:
        {
            // Other message, esp. kart control, are silently ignored.
            // FIXME: we might want to make sure that no such message actually arrives
            if(Message::peekType(event->packet)!=Message::MT_RACE_RESULT_ACK)
            {
                enet_packet_destroy(event->packet);
                return;
            }
            m_barrier_count++;
            if(m_barrier_count==(int)m_num_clients)
            {
                m_state = NS_MAIN_MENU;
            }
            break;
        }
    default: assert(0);  // should not happen
    }   // switch m_state
}   // handleMessageAtServer

// ----------------------------------------------------------------------------
void NetworkManager::handleMessageAtClient(ENetEvent *event)
{  // handle message at client (from server)
    switch(m_state)
    {
    case NS_WAIT_FOR_AVAILABLE_CHARACTERS:
        {
            CharacterInfoMessage m(event->packet);
            // FIXME: handle list of available characters
            m_state = NS_CHARACTER_SELECT;
            break;
        }
    case NS_CHARACTER_SELECT:  
        {
            CharacterConfirmMessage m(event->packet);
            kart_properties_manager->selectKartName(m.getKartName());
            // TODO - karts selection screen in networking
            /*
            CharSel *menu = dynamic_cast<CharSel*>(menu_manager->getCurrentMenu());
            if(menu)
                menu->updateAvailableCharacters();
             */
            break;
        }
    case NS_WAIT_FOR_KART_CONFIRMATION:
        {
            CharacterConfirmMessage m(event->packet);
            kart_properties_manager->selectKartName(m.getKartName());

            // If the current menu is the character selection menu,
            // update the menu so that the newly taken character is removed.
            // TODO - kart selection screen and networking
            /*
            CharSel *menu = dynamic_cast<CharSel*>(menu_manager->getCurrentMenu());
            if(menu)
                menu->updateAvailableCharacters();*/
            // Check if we received a message about the kart we just selected.
            // If so, the menu needs to progress, otherwise a different kart
            // must be selected by the current player.
            if(m.getKartName()==m_kart_to_confirm)
            {
                int host_id = m.getHostId();
                m_state = (host_id == getMyHostId()) ? NS_KART_CONFIRMED
                                                     : NS_CHARACTER_SELECT;
            }   // m.getkartName()==m_kart_to_confirm
            break;
        }   // wait for kart confirmation
    case NS_WAIT_FOR_RACE_DATA:
        {
            // It is possible that character confirm messages arrive at the 
            // client when it has already left the character selection screen.
            // In this case the messages can simply be ignored.
            if(Message::peekType(event->packet)==Message::MT_CHARACTER_CONFIRM)
            {
                // Receiving it will automatically free the memory.
                CharacterConfirmMessage m(event->packet);
                return;
            }
            RaceInfoMessage m(event->packet);
            // The constructor actually sets the information in the race manager
            m_state = NS_LOADING_WORLD;
            break;
        }
    case NS_READY_SET_GO_BARRIER:
        {
            // Not actually needed, but the destructor of RaceStartMessage
            // will free the memory of the event.
            RaceStartMessage m(event->packet);
            m_state = NS_RACING;
            break;
        }
    case NS_RACING:
        {
            assert(false);  // should never be here while racing
            break;
        }
    case NS_RACE_RESULT_BARRIER:
        {
            RaceResultAckMessage message(event->packet);
            // TODO - race results menu in networking
            /*
            RaceResultsGUI *menu = dynamic_cast<RaceResultsGUI*>(menu_manager->getCurrentMenu());
            if(menu)
                menu->setSelectedWidget(message.getSelectedMenu());*/
            m_state = NS_RACE_RESULT_BARRIER_OVER;
            break;
        }
    default: 
        {
            printf("received unknown message: type %d\n",
                Message::peekType(event->packet));
         //   assert(0);   // should not happen
        }
    }   // switch m_state
}   // handleMessageAtClient

// ----------------------------------------------------------------------------
void NetworkManager::update(float dt)
{
    if(m_mode==NW_NONE) return;
    // Messages during racing are handled in the sendUpdates/receiveUpdate
    // calls, so don't do anything in this case.
    if(m_state==NS_RACING) return;

    ENetEvent event;
    int result = enet_host_service (m_host, &event, 0);
    if(result==0) return;
    if(result<0)
    {
        fprintf(stderr, "Error while receiving messages -> ignored.\n");
        return;
    }
    switch (event.type)
    {
    case ENET_EVENT_TYPE_CONNECT:    handleNewConnection(&event); break;
    case ENET_EVENT_TYPE_RECEIVE:
          if(m_mode==NW_SERVER) 
              handleMessageAtServer(&event);    
          else
              handleMessageAtClient(&event);
          break;
    case ENET_EVENT_TYPE_DISCONNECT: handleDisconnection(&event); break;
    case ENET_EVENT_TYPE_NONE:       break;
    }
}   // update

// ----------------------------------------------------------------------------
void NetworkManager::broadcastToClients(Message &m)
{
    enet_host_broadcast(m_host, 0, m.getPacket());
    enet_host_flush(m_host); 
}   // broadcastToClients

// ----------------------------------------------------------------------------
void NetworkManager::sendToServer(Message &m)
{
    enet_peer_send(m_server, 0, m.getPacket());
    enet_host_flush(m_host); 
}   // sendToServer

// ----------------------------------------------------------------------------
/** Cleans up character related data structures. Must be called before any
 *  character related data is set.
 */
void NetworkManager::initCharacterDataStructures()
{
    // This is called the first time the character selection menu is displayed

    if(m_mode==NW_CLIENT)
    {
        // Change state to wait for list of characters from server
        m_state = NS_WAIT_FOR_AVAILABLE_CHARACTERS;
    }
    else   // Server or no network
    {   
        if(m_mode==NW_SERVER)
        {
            // server: create message with all valid characters
            // ================================================
            for(unsigned int i=1; i<=m_num_clients; i++)
            {
                CharacterInfoMessage m(i);
                enet_peer_send(m_clients[i], 0, m.getPacket());
            }
            enet_host_flush(m_host); 
        }
        // For server and no network:
        // ==========================
        // Prepare the data structures to receive and 
        // store information from all clients.
        m_num_local_players.clear();
        // Server (hostid 0) is not included in the num_clients count.  So to
        // be able to use the hostid as index, we have to allocate one 
        // additional element.
        m_num_local_players.resize(m_num_clients+1, -1);
        m_num_local_players[0] = race_manager->getNumLocalPlayers();
        m_kart_info.clear();
        m_num_all_players = 0;
        // use barrier count to see if we had at least one message from each host
        m_barrier_count      = 0;  
        m_state              = NS_CHARACTER_SELECT;
    }

}   // switchTocharacterSelection

// ----------------------------------------------------------------------------
/** Called on the client to send the data about the selected kart to the 
 *  server and wait for confirmation.
 *  \param player_id Local id of the player which selected the kart.
 *  \param kart_id Identifier of the selected kart. this is used to wait till
 *                 a message about this kart is received back from the server.
 */
void NetworkManager::sendCharacterSelected(int player_id, 
                                           const std::string &kart_id)
{
    if(m_mode==NW_SERVER)
    {
        CharacterConfirmMessage ccm(kart_id, getMyHostId());
        broadcastToClients(ccm);
    } 
    else if(m_mode==NW_CLIENT)
    {
        CharacterSelectedMessage m(player_id);
        sendToServer(m);
        // Wait till we receive confirmation about the selected character.
        m_state           = NS_WAIT_FOR_KART_CONFIRMATION;
        m_kart_to_confirm = kart_id;
    }
}   // sendCharacterSelected

// ----------------------------------------------------------------------------
void NetworkManager::waitForRaceInformation()
{
    m_state = NS_WAIT_FOR_RACE_DATA;
}   // waitForRaceInformation

// ----------------------------------------------------------------------------
void NetworkManager::worldLoaded()
{
    if(m_mode==NW_CLIENT)
    {
        WorldLoadedMessage m;
        sendToServer(m);
        m_state = NS_READY_SET_GO_BARRIER;
    }
}   // worldLoaded

// ----------------------------------------------------------------------------
/** Receive and store the information from sendKartsInformation()
*/
void NetworkManager::setupPlayerKartInfo()
{
    // Not sure if this should be here, but without it extra uncontrolled
    // human players accumulate after each race.
    m_kart_info.clear();

    // Get the local kart info
    for(unsigned int i=0; i<race_manager->getNumLocalPlayers(); i++)
        m_kart_info.push_back(race_manager->getLocalKartInfo(i));

    printf("[setupPlayerKartInfo] m_num_karts = %i\n", race_manager->getNumberOfKarts());

    // Now sort by (hostid, playerid)
    std::sort(m_kart_info.begin(), m_kart_info.end());

    // Set the player kart information
    race_manager->setNumPlayers(m_kart_info.size());

    printf("[setupPlayerKartInfo2] m_num_karts = %i\n", race_manager->getNumberOfKarts());
    
    // Set the global player ID for each player
    for(unsigned int i=0; i<m_kart_info.size(); i++)
    {
        m_kart_info[i].setGlobalPlayerId(i);
        race_manager->setPlayerKart(i, m_kart_info[i]);
    }
    
    printf("[setupPlayerKartInfo3] m_num_karts = %i\n", race_manager->getNumberOfKarts());
    
    // Compute the id of the first kart from each host
    m_kart_id_offset.resize(m_num_clients+1);
    m_kart_id_offset[0]=0;
    for(unsigned int i=1; i<=m_num_clients; i++)
        m_kart_id_offset[i]=m_kart_id_offset[i-1]+m_num_local_players[i-1];

    race_manager->computeRandomKartList();
    
    printf("[setupPlayerKartInfo4] m_num_karts = %i\n", race_manager->getNumberOfKarts());
}   // setupPlayerKartInfo

// ----------------------------------------------------------------------------
/** Sends the information from the race_manager to all clients.
*/
void NetworkManager::sendRaceInformationToClients()
{
    if(m_mode==NW_SERVER)
    {
        setupPlayerKartInfo();
        RaceInfoMessage m(m_kart_info);
        broadcastToClients(m);
    }
    beginReadySetGoBarrier();
}   // sendRaceInformationToClients

// ----------------------------------------------------------------------------
void NetworkManager::beginReadySetGoBarrier()
{
    m_state         = NS_READY_SET_GO_BARRIER;
    m_barrier_count = 0;
    if(m_num_clients==0) m_state = NS_RACING;
}   // beginReadySetGoBarrier
// ----------------------------------------------------------------------------
void NetworkManager::sendConnectMessage()
{
    ConnectMessage msg;
    sendToServer(msg);
}   // sendConnectMessage
// ----------------------------------------------------------------------------
/*** Send all kart controls and kart positions to all clients
*/
void NetworkManager::sendUpdates()
{
    if(m_mode==NW_SERVER)
    {
        race_state->serialise();
        broadcastToClients(*race_state);
    }
    else if(m_mode==NW_CLIENT)
    {
        KartControlMessage m;
        sendToServer(m);
    }
}   // sendUpdates

// ----------------------------------------------------------------------------
void NetworkManager::receiveUpdates()
{
    if(m_mode==NW_NONE) return;   // do nothing if not networking
    // The server receives m_num_clients messages, each client one message
    int num_messages = m_mode==NW_SERVER ? m_num_clients : 1;
    ENetEvent event;
    bool correct=true;

    for(int i=0; i<num_messages; i++)
    {
        int result = enet_host_service (m_host, &event, 0);
        if(result<0)
        {
            fprintf(stderr, m_mode==NW_SERVER 
                            ? "Error while waiting for client control - chaos will reign.\n"
                            : "Error while waiting for server update - chaos will reign.\n");
            correct=false;
            continue;
        }
        // if no message, busy wait
        if(result==0 || event.type==ENET_EVENT_TYPE_NONE)
        {
            i--;
            continue;
        }
        if(event.type!=ENET_EVENT_TYPE_RECEIVE)
        {
            fprintf(stderr, "unexpected message, ignored.\n");
            i--;
            continue;
        }
        if(m_mode==NW_SERVER)
        {
            int host_id = getHostId(event.peer);
            KartControlMessage(event.packet, host_id, m_num_local_players[host_id]);
        }
        else
        {
            // Test if it is a game over message:
            if(Message::peekType(event.packet)==Message::MT_RACE_RESULT)
            {
                RaceResultMessage m(event.packet);
                m_state = NS_WAIT_FOR_RACE_RESULT;
		World::getWorld()->enterRaceOverState();
                return;
            }
            race_state->receive(event.packet);
        }
    }   // for i<num_messages
    if(!correct)
        fprintf(stderr, "Missing messages need to be handled!\n");

}   // receiveUpdates

// ----------------------------------------------------------------------------
void NetworkManager::waitForClientData()
{
    ENetEvent event;
    bool correct=true;
    for(unsigned int i=1; i<=m_num_clients; i++)
    {
        int result = enet_host_service (m_host, &event, 100);
        if(result<=0)
        {
            fprintf(stderr, "Error while waiting for client control - chaos will reign.\n");
            correct=false;
            continue;
        }
        if(event.type!=ENET_EVENT_TYPE_RECEIVE)
        {
            fprintf(stderr, "received no message - chaos will reign.\n");
            correct=false;
            continue;
        }
        int host_id = getHostId(event.peer);
        KartControlMessage(event.packet, host_id, m_num_local_players[host_id]);
    }
    if(!correct)
        fprintf(stderr, "Missing messages need to be handled!\n");

}   // waitForClientData
// ----------------------------------------------------------------------------
/** Sends the race result (kart positions and finishing times) from the server
 *  to all clients. Clients keep on racing till they receive this message, and
 *  will then copy the server's race results to the race manager.
 */
void NetworkManager::sendRaceResults()
{
    RaceResultMessage m;
    broadcastToClients(m);
}   // sendRaceResults

// ----------------------------------------------------------------------------
/** Changes the mode to wait in a barrier for all clients and the server to
 *  acknowledge the result screen. The server waits for a message from all 
 *  clients, upon which it sends a message to all clients. The clients wait
 *  for this message before continuing.
 */
void NetworkManager::beginRaceResultBarrier()
{
    m_state = NS_RACE_RESULT_BARRIER;

    if(m_mode==NW_SERVER)
    {
        m_barrier_count = 0;
        // In case of no networking set the next state
        if(m_num_clients == 0) m_state = NS_MAIN_MENU;
    }
}   // beginRaceResultBarrier

// ----------------------------------------------------------------------------
/** Sends a race 'result acknowledge' message from the clients to the server,
 *  or from the server to the clients (in this case it contains the selected
 *  menu choice from the server).
 */
void NetworkManager::sendRaceResultAck(char menu_selection)
{
    // Menu selection is actually not important for a client
    RaceResultAckMessage m(menu_selection);
    if (m_mode==NW_CLIENT)
    {
        sendToServer(m);
    }
    else
    {
        broadcastToClients(m);
    }
}   // sendRaceResultAck
