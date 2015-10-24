//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2013-2015 SuperTuxKart-Team
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

#include "network/server_console.hpp"

#include "main_loop.hpp"
#include "network/network_manager.hpp"
#include "network/protocol_manager.hpp"
#include "network/stk_host.hpp"
#include "network/protocols/server_lobby_room_protocol.hpp"
#include "network/protocols/stop_server.hpp"
#include "utils/log.hpp"
#include "utils/time.hpp"

#include <iostream>

#ifdef XX
#include "network/protocols/connect_to_server.hpp"
#include "network/protocols/get_peer_address.hpp"
#include "network/protocols/get_public_address.hpp"
#include "network/protocols/hide_public_address.hpp"
#include "network/protocols/show_public_address.hpp"

#include <enet/enet.h>
#include <pthread.h>
#include <string>
#include <stdlib.h>
#endif

ServerConsole::ServerConsole()
{
    m_localhost = NULL;
    m_thread_keyboard = NULL;
}

// ----------------------------------------------------------------------------
ServerConsole::~ServerConsole()
{
    if (m_thread_keyboard)
        pthread_cancel(*m_thread_keyboard);//, SIGKILL);
}

// ----------------------------------------------------------------------------
void ServerConsole::run()
{
    if (enet_initialize() != 0)
    {
        Log::error("ServerConsole", "Could not initialize enet.");
        return;
    }

    Log::info("ServerConsole", "Host initialized.");

    // listen keyboard console input
    m_thread_keyboard = new pthread_t;
    pthread_create(m_thread_keyboard, NULL, mainLoop, this);

    Log::info("ServerConsole", "Ready.");
}   // run

// ----------------------------------------------------------------------------
void* ServerConsole::mainLoop(void* data)
{
    ServerConsole *me = static_cast<ServerConsole*>(data);
    std::string str = "";
    bool stop = false;
    while(!stop)
    {
        getline(std::cin, str);
        if (str == "quit")
        {
            stop = true;
        }
        else if (str == "kickall")
        {
            me->kickAllPlayers();
        }
        else if (str == "start")
        {
            ServerLobbyRoomProtocol* protocol = static_cast<ServerLobbyRoomProtocol*>(ProtocolManager::getInstance()->getProtocol(PROTOCOL_LOBBY_ROOM));
            assert(protocol);
            protocol->startGame();
        }
        else if (str == "selection")
        {
            ServerLobbyRoomProtocol* protocol = static_cast<ServerLobbyRoomProtocol*>(ProtocolManager::getInstance()->getProtocol(PROTOCOL_LOBBY_ROOM));
            assert(protocol);
            protocol->startSelection();
        }
        else if (str == "compute_race")
        {
            GameSetup* setup = NetworkManager::getInstance()->getGameSetup();
            setup->getRaceConfig()->computeRaceMode();
        }
        else if (str == "compute_track")
        {
            GameSetup* setup = NetworkManager::getInstance()->getGameSetup();
            setup->getRaceConfig()->computeNextTrack();
        }
    }

    uint32_t id = ProtocolManager::getInstance()->requestStart(new StopServer());
    while(ProtocolManager::getInstance()->getProtocolState(id) != PROTOCOL_STATE_TERMINATED)
    {
        StkTime::sleep(1);
    }

    main_loop->abort();

    return NULL;
}   // mainLoop

// ----------------------------------------------------------------------------
void ServerConsole::kickAllPlayers()
{
    std::vector<STKPeer*> peers = NetworkManager::getInstance()->getPeers();
    for (unsigned int i = 0; i < peers.size(); i++)
    {
        peers[i]->disconnect();
    }
}   // kickAllPlayers

// ----------------------------------------------------------------------------
void ServerConsole::sendPacket(const NetworkString& data, bool reliable)
{
    m_localhost->broadcastPacket(data, reliable);
}   // sendPacket
