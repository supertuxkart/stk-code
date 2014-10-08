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
#include "network/protocols/synchronization_protocol.hpp"

#include "utils/log.hpp"

#include <stdlib.h>
#include <iostream>
#include <string>

void* waitInput(void* data)
{
    std::string str = "";
    bool stop = false;

    while(!stop)
    {
        getline(std::cin, str);
        if (str == "quit")
        {
            stop = true;
        }
        else if (str == "select")
        {
            std::string str2;
            getline(std::cin, str2);
            Protocol* protocol = ProtocolManager::getInstance()->getProtocol(PROTOCOL_LOBBY_ROOM);
            ClientLobbyRoomProtocol* clrp = static_cast<ClientLobbyRoomProtocol*>(protocol);
            clrp->requestKartSelection(str2);
        }
        else if (str == "vote")
        {
            std::cout << "Vote for ? (track/laps/reversed/major/minor/race#) :";
            std::string str2;
            getline(std::cin, str2);
            Protocol* protocol = ProtocolManager::getInstance()->getProtocol(PROTOCOL_LOBBY_ROOM);
            ClientLobbyRoomProtocol* clrp = static_cast<ClientLobbyRoomProtocol*>(protocol);
            if (str2 == "track")
            {
                std::cin >> str2;
                clrp->voteTrack(str2);
            }
            else if (str2 == "laps")
            {
                int cnt;
                std::cin >> cnt;
                clrp->voteLaps(cnt);
            }
            else if (str2 == "reversed")
            {
                bool cnt;
                std::cin >> cnt;
                clrp->voteReversed(cnt);
            }
            else if (str2 == "major")
            {
                int cnt;
                std::cin >> cnt;
                clrp->voteMajor(cnt);
            }
            else if (str2 == "minor")
            {
                int cnt;
                std::cin >> cnt;
                clrp->voteMinor(cnt);
            }
            else if (str2 == "race#")
            {
                int cnt;
                std::cin >> cnt;
                clrp->voteRaceCount(cnt);
            }
            std::cout << "\n";
        }
        // If STK shuts down, but should receive an input after the network 
        // manager was deleted, the getInstance call will return NULL.
        else if (NetworkManager::getInstance() && 
                 NetworkManager::getInstance()->getPeers().size() > 0)
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
    // On windows in release mode there is no console, and the
    // thread is not created.
    if(m_thread_keyboard)
        pthread_cancel(*m_thread_keyboard);
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

    Log::info("ClientNetworkManager", "Host initialized.");

    m_thread_keyboard = NULL;
    // This code can cause crashes atm (see #1529): if the 
    // client_network_manager receives input after the network manager
    // has been deleted, stk will crash. And this happens on windows
    // in release mode (since there is no console, so no stdin), and
    // apparently on osx at shutdown time - getline returns an empty string
    // (not sure if it has an error condition).
#ifdef RE_ENABLE_LATER
    // On windows in release mode the console is suppressed, so nothing can
    // be read from std::cin. Since getline(std::cin,...) then returns
    // an empty string, the waitInput thread is running all the time, consuming
    // CPU. Therefore don't start the console thread in this case.
#if defined(WIN32) && defined(_MSC_VER) && !defined(DEBUG)
    m_thread_keyboard = NULL;
#else
    // listen keyboard console input
    m_thread_keyboard = (pthread_t*)(malloc(sizeof(pthread_t)));
    pthread_create(m_thread_keyboard, NULL, waitInput, NULL);
#endif
#endif
    NetworkManager::run();

    Log::info("ClientNetworkManager", "Ready !");
}

void ClientNetworkManager::reset()
{
    NetworkManager::reset();

    m_connected = false;
    m_localhost = new STKHost();
    m_localhost->setupClient(1, 2, 0, 0);
    m_localhost->startListening();

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
