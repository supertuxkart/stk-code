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

#include "server_network_manager.hpp"

#include "protocols/get_public_address.hpp"
#include "protocols/hide_public_address.hpp"
#include "protocols/show_public_address.hpp"
#include "protocols/get_peer_address.hpp"
#include "protocols/connect_to_server.hpp"

#include <enet/enet.h>
#include <stdio.h>
#include <string.h>
#include <pthread.h>

ServerNetworkManager::ServerNetworkManager()
{
    m_localhost = NULL;
}

ServerNetworkManager::~ServerNetworkManager()
{
}

void ServerNetworkManager::run()
{
    if (enet_initialize() != 0) 
    {
        printf("Could not initialize enet.\n");
        return;
    }
    NetworkManager::run();
}

void ServerNetworkManager::start()
{
    m_localhost = new STKHost();
    m_localhost->setupServer(STKHost::HOST_ANY, 7321, 32, 2, 0, 0);
    m_localhost->startListening();
    printf("Server now setup, listening on port 7321.\n");
    
    printf("_NetworkInterface>Starting the global protocol\n");
    // step 1 : retreive public address
    Protocol* protocol = new GetPublicAddress(&m_public_address);
    ProtocolManager::getInstance()->requestStart(protocol);
    while (ProtocolManager::getInstance()->getProtocolState(protocol) != PROTOCOL_STATE_TERMINATED )
    {
    }
    printf("_NetworkInterface> The public address is known.\n"); 
    
    // step 2 : show the public address for others (here, the server)
    ShowPublicAddress* spa = new ShowPublicAddress(NULL);
    spa->setPassword(m_player_login.password);
    spa->setUsername(m_player_login.username);
    spa->setPublicAddress(m_public_address.ip, m_public_address.port);
    ProtocolManager::getInstance()->requestStart(spa);
    while (ProtocolManager::getInstance()->getProtocolState(spa) != PROTOCOL_STATE_TERMINATED )
    {
    }
    printf("_NetworkInterface> The public address is being shown online.\n"); 
}

bool ServerNetworkManager::connectToPeer(std::string peer_username)
{
    printf("_NetworkInterface>Starting the connection to host protocol\n");
    
    // step 3 : get the peer's addres.
    TransportAddress addr;
    GetPeerAddress* gpa = new GetPeerAddress(&addr);
    gpa->setPeerName(peer_username);
    ProtocolManager::getInstance()->requestStart(gpa);
    while (ProtocolManager::getInstance()->getProtocolState(gpa) != PROTOCOL_STATE_TERMINATED )
    {
    }
    printf("_NetworkInterface> The public address of the peer is known.\n"); 
    
    // step 2 : connect to the peer
    ConnectToServer* cts = new ConnectToServer(NULL);
    cts->setServerAddress(addr.ip, addr.port);
    ProtocolManager::getInstance()->requestStart(cts);
    while (ProtocolManager::getInstance()->getProtocolState(cts) != PROTOCOL_STATE_TERMINATED )
    {
    } 
    bool success = false;
    if (isConnectedTo(addr))
    {
        success = true;
        printf("_NetworkInterface> CONNECTION SUCCES : YOU ARE NOW CONNECTED TO A PEER.\n");
    }
    else 
    {
        printf("_NetworkInterface> We are NOT connected to the server.\n");
    }
    
    return success;
}

void ServerNetworkManager::packetReceived(char* data)
{
    printf("ServerNetworkManager::packetReceived()\n");
    puts(data);
    sendPacket(data);
}
void ServerNetworkManager::sendPacket(char* data)
{
    m_localhost->broadcastPacket(data);
}
