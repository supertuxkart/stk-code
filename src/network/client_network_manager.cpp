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

#include "network/client_network_manager.hpp"

#include "network/protocols/get_public_address.hpp"
#include "network/protocols/hide_public_address.hpp"
#include "network/protocols/show_public_address.hpp"
#include "network/protocols/get_peer_address.hpp"
#include "network/protocols/connect_to_server.hpp"

#include "utils/log.hpp"

ClientNetworkManager::ClientNetworkManager()
{
}

ClientNetworkManager::~ClientNetworkManager()
{
}

void ClientNetworkManager::run() 
{
    if (enet_initialize() != 0) 
    {
        Log::error("ClientNetworkManager", "Could not initialize enet.\n");
        return;
    }
    m_localhost = new STKHost();
    m_localhost->setupClient(1, 2, 0, 0);
    m_localhost->startListening();
    
    NetworkManager::run();
}

bool ClientNetworkManager::connectToHost(std::string serverNickname)
{
    Log::info("ClientNetworkManager", "Starting the connection to host protocol\n");
    // step 1 : retreive public address
    Protocol* protocol = new GetPublicAddress(&m_public_address);
    ProtocolManager::getInstance()->requestStart(protocol);
    while (ProtocolManager::getInstance()->getProtocolState(protocol) != PROTOCOL_STATE_TERMINATED )
    {
    }
    Log::info("ClientNetworkManager", "The public address is known.\n"); 
    
    // step 2 : show the public address for others (here, the server)
    ShowPublicAddress* spa = new ShowPublicAddress(NULL);
    spa->setPassword(m_player_login.password);
    spa->setUsername(m_player_login.username);
    spa->setPublicAddress(m_public_address.ip, m_public_address.port);
    ProtocolManager::getInstance()->requestStart(spa);
    while (ProtocolManager::getInstance()->getProtocolState(spa) != PROTOCOL_STATE_TERMINATED )
    {
    }
    Log::info("ClientNetworkManager", "The public address is being shown online.\n"); 
    
    // step 3 : get the server's addres.
    TransportAddress addr;
    GetPeerAddress* gpa = new GetPeerAddress(&addr);
    gpa->setPeerName(serverNickname);
    ProtocolManager::getInstance()->requestStart(gpa);
    while (ProtocolManager::getInstance()->getProtocolState(gpa) != PROTOCOL_STATE_TERMINATED )
    {
    }
    Log::info("ClientNetworkManager", "The public address of the server is known.\n"); 
    
    // step 4 : connect to the server
    ConnectToServer* cts = new ConnectToServer(NULL);
    cts->setServerAddress(addr.ip, addr.port);
    ProtocolManager::getInstance()->requestStart(cts);
    while (ProtocolManager::getInstance()->getProtocolState(cts) != PROTOCOL_STATE_TERMINATED )
    {
    } 
    bool success = false;
    if (m_localhost->isConnectedTo(TransportAddress(addr.ip, addr.port)))
    {
        success = true;
        Log::info("ClientNetworkManager", "Connection success. You are now connected to a server.\n");
    }
    else 
    {
        Log::error("ClientNetworkManager", "We are NOT connected to the server.\n");
    }
    // step 5 : hide our public address
    HidePublicAddress* hpa = new HidePublicAddress(NULL);
    hpa->setPassword(m_player_login.password);
    hpa->setUsername(m_player_login.username);
    ProtocolManager::getInstance()->requestStart(hpa);
    while (ProtocolManager::getInstance()->getProtocolState(hpa) != PROTOCOL_STATE_TERMINATED )
    {
    }
    Log::info("ClientNetworkManager", "The public address is now hidden online.\n"); 
    
    return success;
}

void ClientNetworkManager::sendPacket(const NetworkString& data)
{
    if (m_peers.size() > 1)
        Log::warn("ClientNetworkManager", "Ambiguous send of data.\n");
    m_peers[0]->sendPacket(data);
}

STKPeer* ClientNetworkManager::getPeer()
{
    return m_peers[0];
}
