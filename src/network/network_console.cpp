//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2015 Joerg Henrichs
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

#include "network/network_console.hpp"

#include "main_loop.hpp"
#include "network/network_config.hpp"
#include "network/protocol_manager.hpp"
#include "network/stk_host.hpp"
#include "network/protocols/client_lobby_room_protocol.hpp"
#include "network/protocols/server_lobby_room_protocol.hpp"
#include "network/protocols/stop_server.hpp"
#include "network/stk_peer.hpp"
#include "utils/log.hpp"
#include "utils/time.hpp"

#include <iostream>


NetworkConsole::NetworkConsole()
{
    m_localhost = NULL;
    m_thread_keyboard = NULL;
}

// ----------------------------------------------------------------------------
NetworkConsole::~NetworkConsole()
{
    if (m_thread_keyboard)
        pthread_cancel(*m_thread_keyboard);//, SIGKILL);
}

// ----------------------------------------------------------------------------
void NetworkConsole::run()
{
    // listen keyboard console input
    m_thread_keyboard = new pthread_t;
    pthread_create(m_thread_keyboard, NULL, mainLoop, this);

    Log::info("NetworkConsole", "Ready.");
}   // run

// ----------------------------------------------------------------------------
void* NetworkConsole::mainLoop(void* data)
{
    NetworkConsole *me = static_cast<NetworkConsole*>(data);
    std::string str = "";
    bool stop = false;
    while (!stop)
    {
        getline(std::cin, str);
        if (str == "quit")
        {
            stop = true;
        }
        else if (str == "kickall" && NetworkConfig::get()->isServer())
        {
            me->kickAllPlayers();
        }
        else if (str == "start" && NetworkConfig::get()->isServer())
        {
            ServerLobbyRoomProtocol* protocol =
                static_cast<ServerLobbyRoomProtocol*>
                (ProtocolManager::getInstance()->getProtocol(PROTOCOL_LOBBY_ROOM));
            assert(protocol);
            protocol->startGame();
        }
        else if (str == "selection" && NetworkConfig::get()->isServer())
        {
            ServerLobbyRoomProtocol* protocol =
                static_cast<ServerLobbyRoomProtocol*>
                (ProtocolManager::getInstance()->getProtocol(PROTOCOL_LOBBY_ROOM));
            assert(protocol);
            protocol->startSelection();
        }
        else if (str == "select" && NetworkConfig::get()->isclient())
        {
            std::string str2;
            getline(std::cin, str2);
            Protocol* protocol =
                ProtocolManager::getInstance()->getProtocol(PROTOCOL_LOBBY_ROOM);
            ClientLobbyRoomProtocol* clrp = static_cast<ClientLobbyRoomProtocol*>(protocol);
            clrp->requestKartSelection(str2);
        }
        else if (str == "vote" && NetworkConfig::get()->isclient())
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
        else
        {
            Log::info("Console", "Unknown command '%s'.", str.c_str());
        }
    }   // while !stop

    Protocol *p = new StopServer();
    while(p->getState() != PROTOCOL_STATE_TERMINATED)
    {
        StkTime::sleep(1);
    }

    delete p;

    main_loop->abort();

    return NULL;
}   // mainLoop

// ----------------------------------------------------------------------------
void NetworkConsole::kickAllPlayers()
{
    const std::vector<STKPeer*> &peers = STKHost::get()->getPeers();
    for (unsigned int i = 0; i < peers.size(); i++)
    {
        peers[i]->disconnect();
    }
}   // kickAllPlayers

// ----------------------------------------------------------------------------
void NetworkConsole::sendPacket(const NetworkString& data, bool reliable)
{
    m_localhost->broadcastPacket(data, reliable);
}   // sendPacket
