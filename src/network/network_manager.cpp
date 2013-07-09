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

#include "network/network_manager.hpp"

#include "network/protocols/hide_public_address.hpp"
#include "network/protocols/show_public_address.hpp"
#include "network/protocols/get_public_address.hpp"

#include "network/protocol_manager.hpp"
#include "network/client_network_manager.hpp"
#include "network/server_network_manager.hpp"

#include "utils/log.hpp"

#include <pthread.h>
#include <signal.h>

//-----------------------------------------------------------------------------

NetworkManager::NetworkManager()
{
    m_public_address.ip = 0;
    m_public_address.port = 0;
    m_localhost = NULL;
    m_game_setup = NULL;
}

//-----------------------------------------------------------------------------

NetworkManager::~NetworkManager() 
{
    ProtocolManager::kill();
       
    if (m_localhost)
        delete m_localhost; 
    while(!m_peers.empty())
    {
        delete m_peers.back();
        m_peers.pop_back();
    }
}

//-----------------------------------------------------------------------------

void NetworkManager::run()
{ 
    // create the protocol manager
    ProtocolManager::getInstance<ProtocolManager>();
}

//-----------------------------------------------------------------------------

bool NetworkManager::connect(TransportAddress peer)
{
    if (peerExists(peer))
        return isConnectedTo(peer);
    
    return STKPeer::connectToHost(m_localhost, peer, 2, 0);
}

//-----------------------------------------------------------------------------

void NetworkManager::setManualSocketsMode(bool manual)
{
    if (manual)
        m_localhost->stopListening();
    else
        m_localhost->startListening();
}

//-----------------------------------------------------------------------------

void NetworkManager::notifyEvent(Event* event)
{
    Log::info("NetworkManager", "EVENT received");
    switch (event->type) 
    {
        case EVENT_TYPE_MESSAGE:
            Log::info("NetworkManager", "Message, Sender : %i.%i.%i.%i, message = \"%s\"", event->peer->getAddress()>>24&0xff, event->peer->getAddress()>>16&0xff, event->peer->getAddress()>>8&0xff, event->peer->getAddress()&0xff, event->data.c_str());
            break;
        case EVENT_TYPE_DISCONNECTED:
        {
            Log::info("NetworkManager", "Somebody is now disconnected. There are now %lu peers.", m_peers.size());
            Log::info("NetworkManager", "Disconnected host: %i.%i.%i.%i:%i", event->peer->getAddress()>>24&0xff, event->peer->getAddress()>>16&0xff, event->peer->getAddress()>>8&0xff, event->peer->getAddress()&0xff,event->peer->getPort());
            // remove the peer:
            bool removed = false; 
            for (unsigned int i = 0; i < m_peers.size(); i++)
            {
                if (m_peers[i] == event->peer && !removed) // remove only one
                {
                    delete m_peers[i];
                    m_peers.erase(m_peers.begin()+i, m_peers.begin()+i+1);
                    Log::info("NetworkManager", "The peer has been removed from the Network Manager.");
                    removed = true;
                }
                else if (m_peers[i] == event->peer)
                {
                    Log::fatal("NetworkManager", "Multiple peers match the disconnected one.");
                }
            }
            if (!removed)
                Log::fatal("NetworkManager", "The peer that has been disconnected was not registered by the Network Manager.");
            break;
        }
        case EVENT_TYPE_CONNECTED:
            Log::info("NetworkManager", "A client has just connected. There are now %lu peers.", m_peers.size() + 1);
            // create the new peer:
            m_peers.push_back(event->peer);
            break;
    }
    ProtocolManager::getInstance()->notifyEvent(event);
}

//-----------------------------------------------------------------------------

void NetworkManager::sendPacket(STKPeer* peer, const NetworkString& data)
{
    if (peer)
        peer->sendPacket(data);
}

//-----------------------------------------------------------------------------

void NetworkManager::sendPacketExcept(STKPeer* peer, const NetworkString& data)
{
    for (unsigned int i = 0; i < m_peers.size(); i++)
    {
        if (m_peers[i] != peer)
            m_peers[i]->sendPacket(data);
    }
}

//-----------------------------------------------------------------------------

GameSetup* NetworkManager::setupNewGame()
{
    if (m_game_setup)
        delete m_game_setup;
    m_game_setup = new GameSetup();
    return m_game_setup;
}

//-----------------------------------------------------------------------------


void NetworkManager::setLogin(std::string username, std::string password)
{
    m_player_login.username = username;
    m_player_login.password = password;
}

//-----------------------------------------------------------------------------

void NetworkManager::setPublicAddress(TransportAddress addr)
{
    m_public_address = addr;
}

//-----------------------------------------------------------------------------

bool NetworkManager::peerExists(TransportAddress peer)
{
    return m_localhost->peerExists(peer);
}

//-----------------------------------------------------------------------------

bool NetworkManager::isConnectedTo(TransportAddress peer)
{
    return m_localhost->isConnectedTo(peer);
}
