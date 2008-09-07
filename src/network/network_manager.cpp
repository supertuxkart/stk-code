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
#include "stk_config.hpp"
#include "user_config.hpp"
#include "race_manager.hpp"
#include "kart_properties_manager.hpp"

NetworkManager* network_manager = 0;

NetworkManager::NetworkManager()
{
     m_mode           = NW_NONE;
     m_state          = NS_ACCEPT_CONNECTIONS;
     m_port           = 12345;
     m_server_address = "172.31.41.53";
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
     fprintf(stderr, "Initialising server, listening on %d\n", m_port);

     ENetAddress address;
     address.host = ENET_HOST_ANY;
     address.port = m_port;

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

    fprintf(stderr, "Server initialised, waiting for connections ...\n");
    m_client_names.push_back("server");
    return true;
#endif
}   // initServer

// -----------------------------------------------------------------------------
bool NetworkManager::initClient()
{
    fprintf(stderr, "Initialising client\n");
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

    enet_address_set_host (& address, m_server_address.c_str());
    address.port = m_port;

    /* Initiate the connection, allocating the two channels 0 and 1. */
    peer = enet_host_connect (m_host, &address, 2);    
    
    if (peer == NULL)
    {
        fprintf (stderr, 
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
                m_server_address.c_str(), m_port);
        return false;
    }
    fprintf(stderr, "Connection to %s:%d succeeded.\n", 
             m_server_address.c_str(), m_port);
    enet_host_service(m_host, &event, 1000);
    // FIXME Receive host id from server here!!
    m_host_id = 1;

    return true;
#endif
}  // initClient

// ----------------------------------------------------------------------------
void NetworkManager::handleNewConnection(ENetEvent *event)
{
    if(m_state!=NS_ACCEPT_CONNECTIONS)
    {
        // We don't accept connections atm
        return;
    }
    m_num_clients++;
    fprintf (stderr, "A new client connected from %x:%u. Connected: %d.\n", 
             event->peer -> address.host,
             event->peer -> address.port, m_num_clients);

    // FIXME: send m_num_clients as hostid back to new client.
    // FIXME: client should send an id as well to be displayed
    m_client_names.push_back("client");
}   // handleNewConnection

// ----------------------------------------------------------------------------
void NetworkManager::handleDisconnection(ENetEvent *event)
{
    if(m_state!=NS_ACCEPT_CONNECTIONS)
    {
        fprintf(stderr, "Disconnect while in race - close your eyes and hope for the best.\n");
        return;
    }
    fprintf(stderr, "%x:%d disconected.\n", event->peer->address.host,
            event->peer->address.port );
    m_num_clients--;
}   // handleDisconnection

// ----------------------------------------------------------------------------
void NetworkManager::handleServerMessage(ENetEvent *event)
{
    switch(m_state)
    {
    case NS_ACCEPT_CONNECTIONS:
        fprintf(stderr, "Received a receive event while waiting for client - ignored.\n");
        return;
    case NS_CHARACTER_SELECT:
        {   
            // only accept testAndSet and 'character selected' messages here.
            // Get character from message, check if it's still available
            int kartid=0, playerid=0, hostid=0;
            std::string name="tuxkart", user="guest";
            if(kart_properties_manager->testAndSetKart(kartid))
            {
                // send 'ok' message to client and all other clients
                m_kart_info.push_back(RemoteKartInfo(playerid, name, user, hostid));
            }
            else
            {
                // send 'not avail' to sender
            }
            break;
        }
    case NS_READY_SET_GO_BARRIER:
        m_barrier_count++;
        if(m_barrier_count==m_num_clients)
        {
            // broadcast start message
            m_state = NS_RACING;
        }
        break;
    case NS_KART_INFO_BARRIER:
        m_barrier_count++;
        if(m_barrier_count==m_num_clients)
        {
            // broadcast start message
            m_state = NS_RACING;
        }
        break;

    }   // switch m_state
}   // handleServerMessage

// ----------------------------------------------------------------------------
void NetworkManager::switchToReadySetGoBarrier()
{
    assert(m_state == NS_CHARACTER_SELECT);
    m_state         = NS_READY_SET_GO_BARRIER;
    m_barrier_count = 0;
}   // switchToReadySetGoBarrier

// ----------------------------------------------------------------------------
void NetworkManager::switchToCharacterSelection()
{
    // This must be called from the network info menu, 
    // so make sure the state is correct
    assert(m_state == NS_ACCEPT_CONNECTIONS);
    m_state         = NS_CHARACTER_SELECT;
}   // switchTocharacterSelection

// ----------------------------------------------------------------------------
void NetworkManager::switchToRaceDataSynchronisation()
{
    assert(m_state == NS_CHARACTER_SELECT);
    m_state         = NS_KART_INFO_BARRIER;
    m_barrier_count = 0;
}   // switchToRaceDataSynchronisation

// ----------------------------------------------------------------------------
void NetworkManager::handleClientMessage(ENetEvent *event)
{
    switch(m_state)
    {
    case NS_ACCEPT_CONNECTIONS:
        fprintf(stderr, "Received a receive event while waiting for client - ignored.\n");
        return;

    }   // switch m_state
}   // handleClientMessage

// ----------------------------------------------------------------------------
void NetworkManager::update(float dt)
{
    if(m_mode==NW_NONE) return;
    ENetEvent event;
    int result = enet_host_service (m_host, &event, 1);
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
              handleServerMessage(&event);    
          else
              handleClientMessage(&event);
          break;
    case ENET_EVENT_TYPE_DISCONNECT: handleDisconnection(&event); break;
    case ENET_EVENT_TYPE_NONE:       break;
    }
}   // update


// ----------------------------------------------------------------------------
/** Send race_manager->getNumPlayers(), the kart and the name of each
    player to the server. 
*/
void NetworkManager::sendKartsInformationToServer()
{
    for(int i=0; i<(int)race_manager->getNumLocalPlayers(); i++)
    {
        fprintf(stderr, "Sending name '%s', ",user_config->m_player[i].getName().c_str());
        fprintf(stderr, "kart name '%s'\n", race_manager->getLocalKartInfo(i).getKartName().c_str());
    }   // for i<getNumLocalPlayers
    fprintf(stderr, "Client sending kart information to server\n");
}   // sendKartsInformationToServer

// ----------------------------------------------------------------------------
/** Receive and store the information from sendKartsInformation()
*/
void NetworkManager::waitForKartsInformation()
{
    m_kart_info.clear();

    fprintf(stderr, "Server receiving all kart information\n");
    // FIXME: debugging
    m_kart_info.push_back(RemoteKartInfo(0, "tuxkart","xx", 1));
    m_kart_info.push_back(RemoteKartInfo(1, "yetikart",   "yy", 1));

    // Get the local kart info
    for(unsigned int i=0; i<race_manager->getNumLocalPlayers(); i++)
        m_kart_info.push_back(race_manager->getLocalKartInfo(i));

    // Now sort by (hostid, playerid)
    std::sort(m_kart_info.begin(), m_kart_info.end());

    // Set the global player ID for each player
    for(unsigned int i=0; i<m_kart_info.size(); i++)
        m_kart_info[i].setGlobalPlayerId(i);

    // FIXME: distribute m_kart_info to all clients

    // Set the player kart information
    race_manager->setNumPlayers(m_kart_info.size());
    for(unsigned int i=0; i<m_kart_info.size(); i++)
    {
        race_manager->setPlayerKart(i, m_kart_info[i]);
    }
}   // waitForKartsInformation

// ----------------------------------------------------------------------------
/** Sends the information from the race_manager to all clients.
*/
void NetworkManager::sendRaceInformationToClients()
{
    fprintf(stderr, "server sending race_manager information to all clients\n");
    for(unsigned i=0; i<race_manager->getNumLocalPlayers(); i++)
    {
        const RemoteKartInfo& ki=race_manager->getLocalKartInfo(i);
        fprintf(stderr, "Sending kart '%s' playerid %d host %d\n",
                ki.getKartName().c_str(), ki.getLocalPlayerId(), ki.getHostId());
    }   // for i
}   // sendRaceInformationToClients

// ----------------------------------------------------------------------------
/** Receives and sets the race_manager information.
*/
void NetworkManager::waitForRaceInformation()
{
    fprintf(stderr, "Client waiting for race information\n");
}   // waitForRaceInformation
// ----------------------------------------------------------------------------
