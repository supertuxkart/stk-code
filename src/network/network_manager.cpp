//  $Id: network_manager.hpp 2128 2008-06-13 00:53:52Z cosmosninja $
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

#include "network_manager.hpp"
#include "connect_message.hpp"
#include "character_info_message.hpp"
#include "character_selected_message.hpp"
#include "race_info_message.hpp"
#include "race_start_message.hpp"
#include "world_loaded_message.hpp"
#include "stk_config.hpp"
#include "user_config.hpp"
#include "race_manager.hpp"
#include "kart_properties_manager.hpp"
#include "translation.hpp"
#include "gui/font.hpp"

NetworkManager* network_manager = 0;

NetworkManager::NetworkManager()
{
     m_mode           = NW_NONE;
     m_state          = NS_ACCEPT_CONNECTIONS;

     m_num_clients    = 0;
     m_host_id        = 0;

#ifdef HAVE_ENET
     if (enet_initialize () != 0)
     {
	  fprintf (stderr, "An error occurred while initializing ENet.\n");
	  exit(-1);
     }
#endif
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
#ifdef HAVE_ENET
     if(m_mode==NW_SERVER || m_mode==NW_CLIENT) enet_host_destroy(m_host);
     enet_deinitialize(); 
#endif
}   // ~NetworkManager

// -----------------------------------------------------------------------------
bool NetworkManager::initServer()
{
#ifdef HAVE_ENET
     ENetAddress address;
     address.host = ENET_HOST_ANY;
     address.port = user_config->m_server_port;

     m_host = enet_host_create (& address     /* the address to bind the server host to */, 
                                stk_config->m_max_karts /* number of connections */,
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
#endif
}   // initServer

// -----------------------------------------------------------------------------
bool NetworkManager::initClient()
{
#ifdef HAVE_ENET
    m_host = enet_host_create (NULL /* create a client host */,
                               1    /* only allow 1 outgoing connection */,
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

    enet_address_set_host (& address, user_config->m_server_address.c_str());
    address.port = user_config->m_server_port;

    /* Initiate the connection, allocating the two channels 0 and 1. */
    peer = enet_host_connect (m_host, &address, 2);    
    
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
                user_config->m_server_address.c_str(), user_config->m_server_port);
        return false;
    }
    m_server = peer;
    return true;
#endif
}  // initClient

// ----------------------------------------------------------------------------
/** Called in case of an error, to switch back to non-networking mode.
*/
void NetworkManager::disableNetworking()
{
    m_mode=NW_NONE;
    // FIXME: what enet data structures do we have to free/reset???

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
        event->peer->address.port, (int)event->peer->data );
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
            m_client_names[(int)event->peer->data] = m.getId();
            m_num_clients++;
            return;
        }
    case NS_CHARACTER_SELECT:
        {
            CharacterSelectedMessage m(event->packet);
            int hostid=(int)event->peer->data;
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
            // See if this was the last message, i.e. we have received at least
            // one message from each client, and the size of the kart_info
            // array is the same as the number of all players (which does not
            // yet include the number of players on the host).
            if(m_barrier_count = m_num_clients &&
                m_num_all_players==m_kart_info.size())
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
            if(m_barrier_count==m_num_clients)
            {
                m_state = NS_RACING;
                RaceStartMessage m;
                broadcastToClients(m);
            }
        }
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
    case NS_WAIT_FOR_RACE_DATA:
        {
            RaceInfoMessage m(event->packet);
            // The constructor actually sets the information in the race manager
            m_state = NS_LOADING_WORLD;
            break;
        }
    case NS_READY_SET_GO_BARRIER:
        {
            m_state = NS_RACING;
            break;
        }
    }   // switch m_state
}   // handleMessageAtClient

// ----------------------------------------------------------------------------
void NetworkManager::update(float dt)
{
    if(m_mode==NW_NONE) return;
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
void NetworkManager::sendToServer(Message* m)
{
    enet_peer_send(m_server, 0, m->getPacket());
    enet_host_flush(m_host); 
}   // sendToServer

// ----------------------------------------------------------------------------
void NetworkManager::switchToCharacterSelection()
{
    // This is called the first time the character selection menu is displayed
    assert(m_state == NS_NONE);
    if(m_mode==NW_CLIENT)
    {
        // Change state to wait for list of characters from server
        m_state = NS_WAIT_FOR_AVAILABLE_CHARACTERS;
    }
    else
    {   // server: create message with all valid characters
        // ================================================
        CharacterInfoMessage m;
        broadcastToClients(m);

        // Prepare the data structures to receive and 
        // store information from all clients.
        m_num_local_players.clear();
        // Server (hostid 0) is not included in the num_clients count.  So to
        // be able to use the hostid as index, we have to allocate one 
        // additional element.
        m_num_local_players.resize(m_num_clients+1, -1);
        m_kart_info.clear();
        m_num_all_players = 0;
        // use barrier count to see if we had at least one message from each host
        m_barrier_count      = 0;  
        m_state              = NS_CHARACTER_SELECT;
    }

}   // switchTocharacterSelection

// ----------------------------------------------------------------------------
void NetworkManager::sendCharacterSelected(int player_id)
{
    CharacterSelectedMessage m(player_id);
    sendToServer(&m);
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
        sendToServer(&m);
    }
    else if(m_mode==NW_SERVER)
    {
        assert(m_state=NS_READY_SET_GO_BARRIER);
        RaceStartMessage m;
        broadcastToClients(m);
    }

}   // worldLoaded

// ----------------------------------------------------------------------------
// ----------------------------------------------------------------------------
/** Receive and store the information from sendKartsInformation()
*/
void NetworkManager::setupPlayerKartInfo()
{
    // Get the local kart info
    for(unsigned int i=0; i<race_manager->getNumLocalPlayers(); i++)
        m_kart_info.push_back(race_manager->getLocalKartInfo(i));

    // Now sort by (hostid, playerid)
    std::sort(m_kart_info.begin(), m_kart_info.end());

    // Set the global player ID for each player
    for(unsigned int i=0; i<m_kart_info.size(); i++)
        m_kart_info[i].setGlobalPlayerId(i);

    // Set the player kart information
    race_manager->setNumPlayers(m_kart_info.size());
    for(unsigned int i=0; i<m_kart_info.size(); i++)
    {
        race_manager->setPlayerKart(i, m_kart_info[i]);
    }
}   // setupPlayerKartInfo

// ----------------------------------------------------------------------------
/** Sends the information from the race_manager to all clients.
*/
void NetworkManager::sendRaceInformationToClients()
{
    setupPlayerKartInfo();
    RaceInfoMessage m(m_kart_info);
    broadcastToClients(m);
    m_state         = NS_READY_SET_GO_BARRIER;
    m_barrier_count = 0;
    if(m_num_clients==0) m_state = NS_RACING;
}   // sendRaceInformationToClients

// ----------------------------------------------------------------------------
void NetworkManager::sendConnectMessage()
{
    ConnectMessage msg;
    sendToServer(&msg);
}   // sendConnectMessage
// ----------------------------------------------------------------------------
