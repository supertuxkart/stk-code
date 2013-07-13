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
#include "network/protocols/client_lobby_room_protocol.hpp"

#include "utils/log.hpp"

#include <stdlib.h>
#include <iostream>
#include <string>

void* waitInput(void* data)
{
    std::string str = "";
    bool stop = false;
    int n = 0;
    bool success = false;
    uint32_t ping = 0;

    while(!stop)
    {
        getline(std::cin, str);
        if (str == "quit")
        {
            stop = true;
        }
        else if (str == "disconnect")
        {
            NetworkManager::getInstance()->getPeers()[0]->disconnect();
        }
        else if (str == "connect")
        {
            ProtocolManager::getInstance()->requestStart(new ConnectToServer());
        }
        else if (sscanf(str.c_str(), "connect=%d", &n))
        {
            ProtocolManager::getInstance()->requestStart(new ConnectToServer(n));
        }
        else if (str == "select")
        {
            std::string str2;
            getline(std::cin, str2);
            Protocol* protocol = ProtocolManager::getInstance()->getProtocol(PROTOCOL_LOBBY_ROOM);
            ClientLobbyRoomProtocol* clrp = static_cast<ClientLobbyRoomProtocol*>(protocol);
            clrp->requestKartSelection(str2);
        }
        else if (str == "synchronize")
        {
            ProtocolManager::getInstance()->requestStart(new SynchronizationProtocol(&ping, &success));
        }
        else if (NetworkManager::getInstance()->getPeers().size() > 0)
        {
            NetworkString msg;
            msg.ai8(0);
            msg += str;
            NetworkManager::getInstance()->getPeers()[0]->sendPacket(msg);
        }
    }

    exit(0);

    return NULL;
}

ClientNetworkManager::ClientNetworkManager()
{
    m_thread_keyboard = NULL;
    m_connected = false;
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

    // listen keyboard console input
    m_thread_keyboard = (pthread_t*)(malloc(sizeof(pthread_t)));
    pthread_create(m_thread_keyboard, NULL, waitInput, NULL);

    NetworkManager::run();
}

void ClientNetworkManager::sendPacket(const NetworkString& data, bool reliable)
{
    if (m_peers.size() > 1)
        Log::warn("ClientNetworkManager", "Ambiguous send of data.\n");
    m_peers[0]->sendPacket(data, reliable);
}

STKPeer* ClientNetworkManager::getPeer()
{
    return m_peers[0];
}
