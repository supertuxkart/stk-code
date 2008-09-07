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

NetworkManager* network_manager = 0;

NetworkManager::NetworkManager()
{
     m_mode           = NW_NONE;
     m_state          = NS_SYNCHRONISING;
     m_port           = 12345;
     m_server_address = "172.31.41.53";
     m_num_clients    = 0;
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
    if (enet_host_service (m_host, & event, 5000) > 0 &&
        event.type == ENET_EVENT_TYPE_CONNECT)
    {
        fprintf(stderr, "Connection to %s:%d succeeded.\n", 
                m_server_address.c_str(), m_port);
        enet_host_service(m_host, &event, 1000);
    }
    else
    {
        /* Either the 5 seconds are up or a disconnect event was */
        /* received. Reset the peer in the event the 5 seconds   */
        /* had run out without any significant event.            */
        enet_peer_reset (peer);

        fprintf(stderr, "Connection to '%s:%d' failed.\n",
		m_server_address.c_str(), m_port);
    }

    return true;
#endif
}  // initServer

// ----------------------------------------------------------------------------
void NetworkManager::handleNewConnection(ENetEvent *event)
{
    if(m_state!=NS_SYNCHRONISING)
    {
        // We don't accept connections atm
        return;
    }
    m_num_clients++;
    fprintf (stderr, "A new client connected from %x:%u. Connected: %d.\n", 
        event->peer -> address.host,
        event->peer -> address.port, m_num_clients);

}   // handleNewConnection
// ----------------------------------------------------------------------------
void NetworkManager::handleDisconnection(ENetEvent *event)
{
    if(m_state!=NS_SYNCHRONISING)
    {
        fprintf(stderr, "Disconnect while in race - close your eyes and hope for the best.\n");
        return;
    }
    fprintf(stderr, "%x:%d disconected.\n", event->peer->address.host,
            event->peer->address.port );
    m_num_clients--;
}   // handleDisconnection

// ----------------------------------------------------------------------------
void NetworkManager::handleNewMessage(ENetEvent *event)
{
    if(m_state==NS_SYNCHRONISING)
    {
        fprintf(stderr, "Received a receive event while waiting for client - ignored.\n");
        return;
    }
}   // handleNewMessage

// ----------------------------------------------------------------------------
void NetworkManager::update(float dt)
{
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
    case ENET_EVENT_TYPE_CONNECT: handleNewConnection(&event);    break;
    case ENET_EVENT_TYPE_RECEIVE:    handleNewMessage(&event);    break;
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
    fprintf(stderr, "Client sending kart information to server\n");
}   // sendKartsInformationToServer
// ----------------------------------------------------------------------------
/** Receive and store the information from sendKartsInformation()
*/
void NetworkManager::waitForKartsInformation()
{
    fprintf(stderr, "Server receiving all kart information\n");
}   // waitForKartsInformation

// ----------------------------------------------------------------------------
/** Sends the information from the race_manager to all clients.
*/
void NetworkManager::sendRaceInformationToClients()
{
    fprintf(stderr, "server sending race_manager information to all clients\n");
}   // sendRaceInformationToClients

// ----------------------------------------------------------------------------
/** Receives and sets the race_manager information.
*/
void NetworkManager::waitForRaceInformation()
{
    fprintf(stderr, "Client waiting for race information\n");
}   // waitForRaceInformation
// ----------------------------------------------------------------------------
