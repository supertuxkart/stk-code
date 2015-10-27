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

#include "network/network_console.hpp"

#include "main_loop.hpp"
#include "network/protocol_manager.hpp"
#include "network/stk_host.hpp"
#include "network/protocols/client_lobby_room_protocol.hpp"
#include "network/protocols/server_lobby_room_protocol.hpp"
#include "network/protocols/stop_server.hpp"
#include "network/stk_peer.hpp"
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
        else if (str == "kickall" && STKHost::isServer())
        {
            me->kickAllPlayers();
        }
        else if (str == "start" && STKHost::isServer())
        {
            ServerLobbyRoomProtocol* protocol =
                static_cast<ServerLobbyRoomProtocol*>
                (ProtocolManager::getInstance()->getProtocol(PROTOCOL_LOBBY_ROOM));
            assert(protocol);
            protocol->startGame();
        }
        else if (str == "selection" && STKHost::isServer())
        {
            ServerLobbyRoomProtocol* protocol =
                static_cast<ServerLobbyRoomProtocol*>
                (ProtocolManager::getInstance()->getProtocol(PROTOCOL_LOBBY_ROOM));
            assert(protocol);
            protocol->startSelection();
        }
        else if (str == "compute_race"&& STKHost::isServer())
        {
            GameSetup* setup = STKHost::get()->getGameSetup();
            setup->getRaceConfig()->computeRaceMode();
        }
        else if (str == "compute_track" && STKHost::isServer())
        {
            GameSetup* setup = STKHost::get()->getGameSetup();
            setup->getRaceConfig()->computeNextTrack();
        }
        else if (str == "select" && STKHost::isclient())
        {
            std::string str2;
            getline(std::cin, str2);
            Protocol* protocol =
                ProtocolManager::getInstance()->getProtocol(PROTOCOL_LOBBY_ROOM);
            ClientLobbyRoomProtocol* clrp = static_cast<ClientLobbyRoomProtocol*>(protocol);
            clrp->requestKartSelection(str2);
        }
        else if (str == "vote" && STKHost::isclient())
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
        // FIXME
        // If STK shuts down, but should receive an input after the network 
        // manager was deleted, the getInstance call will return NULL.
        else if (STKHost::isclient() && STKHost::get()->getPeerCount() > 0)
        {
            NetworkString msg(1 + str.size());
            msg.ai8(0);
            msg += str;
            STKHost::get()->getPeers()[0]->sendPacket(msg);
        }



    }   // while !stop

    uint32_t id = ProtocolManager::getInstance()->requestStart(new StopServer());
    while(ProtocolManager::getInstance()->getProtocolState(id) != PROTOCOL_STATE_TERMINATED)
    {
        StkTime::sleep(1);
    }

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
