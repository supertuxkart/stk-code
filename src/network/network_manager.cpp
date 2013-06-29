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

#include "network_manager.hpp"

#include "protocols/hide_public_address.hpp"
#include "protocols/show_public_address.hpp"
#include "protocols/get_public_address.hpp"

#include "protocol_manager.hpp"
#include "client_network_manager.hpp"
#include "server_network_manager.hpp"

#include <stdio.h>

void* protocolManagerUpdate(void* data)
{
    ProtocolManager* manager = static_cast<ProtocolManager*>(data);
    while(1)
    {
        manager->update();
    }
    return NULL;
}


NetworkManager::NetworkManager()
{
    m_public_address.ip = 0;
    m_public_address.port = 0;
    m_protocol_manager_update_thread = NULL;
}

NetworkManager::~NetworkManager() 
{
}

void NetworkManager::run()
{
    ProtocolManager::getInstance<ProtocolManager>();
    m_protocol_manager_update_thread = (pthread_t*)(malloc(sizeof(pthread_t)));
    pthread_create(m_protocol_manager_update_thread, NULL, protocolManagerUpdate, ProtocolManager::getInstance());
}

bool NetworkManager::connect(TransportAddress peer)
{
    if (peerExists(peer))
        return isConnectedTo(peer);
    
    return STKPeer::connectToHost(m_localhost, peer, 2, 0);
}

void NetworkManager::setManualSocketsMode(bool manual)
{
    if (manual)
        m_localhost->stopListening();
    else
        m_localhost->startListening();
}

void NetworkManager::notifyEvent(Event* event)
{
    printf("EVENT received\n");
    switch (event->type) 
    {
        case EVENT_TYPE_MESSAGE:
            printf("Message, Sender : %u, message = \"%s\"\n", event->peer->getAddress(), event->data.c_str());
            break;
        case EVENT_TYPE_DISCONNECTED:
            printf("Somebody is now disconnected. There are now %lu peers.\n", m_peers.size());
            printf("Disconnected host: %i.%i.%i.%i:%i\n", event->peer->getAddress()>>24&0xff, event->peer->getAddress()>>16&0xff, event->peer->getAddress()>>8&0xff, event->peer->getAddress()&0xff,event->peer->getPort());
            // remove the peer:
            for (unsigned int i = 0; i < m_peers.size(); i++)
            {
                if (m_peers[i] == event->peer)
                {
                    delete m_peers[i];
                    m_peers.erase(m_peers.begin()+i, m_peers.begin()+i+1);
                    break;
                }
            }
            printf("ERROR : the peer that has been disconnected was not registered by the Network Manager.\n");
            break;
        case EVENT_TYPE_CONNECTED:
            printf("A client has just connected. There are now %lu peers.\n", m_peers.size() + 1);
            // create the new peer:
            m_peers.push_back(event->peer);
            break;
    }
    ProtocolManager::getInstance()->notifyEvent(event);
}

void NetworkManager::setLogin(std::string username, std::string password)
{
    m_player_login.username = username;
    m_player_login.password = password;
}

void NetworkManager::setPublicAddress(TransportAddress addr)
{
    m_public_address = addr;
}

bool NetworkManager::peerExists(TransportAddress peer)
{
    return m_localhost->peerExists(peer);
}

bool NetworkManager::isConnectedTo(TransportAddress peer)
{
    return m_localhost->isConnectedTo(peer);
}
