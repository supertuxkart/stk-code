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
#include "network/network_player_profile.hpp"
#include "network/stk_host.hpp"
#include "network/protocols/client_lobby.hpp"
#include "network/protocols/server_lobby.hpp"
#include "network/protocols/stop_server.hpp"
#include "network/stk_peer.hpp"
#include "utils/log.hpp"
#include "utils/time.hpp"
#include "utils/vs.hpp"

#include <iostream>


NetworkConsole::NetworkConsole()
{
    m_localhost = NULL;
    m_thread_keyboard = NULL;
}

// ----------------------------------------------------------------------------
NetworkConsole::~NetworkConsole()
{
#ifndef ANDROID
    if (m_thread_keyboard)
        pthread_cancel(*m_thread_keyboard);//, SIGKILL);
#endif
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
    VS::setThreadName("NetworkConsole");
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
            ServerLobby* protocol = 
                dynamic_cast<ServerLobby*>(LobbyProtocol::get());
            protocol->signalRaceStartToClients();
        }
        else if (str == "selection" && NetworkConfig::get()->isServer())
        {
            ServerLobby* protocol =
                    dynamic_cast<ServerLobby*>(LobbyProtocol::get());
            protocol->startSelection();
        }
        else if (str == "select" && NetworkConfig::get()->isClient())
        {
            std::string str2;
            getline(std::cin, str2);
            ServerLobby* protocol =
                dynamic_cast<ServerLobby*>(LobbyProtocol::get());
            ClientLobby* clrp = dynamic_cast<ClientLobby*>(protocol);
            std::vector<NetworkPlayerProfile*> players =
                                         STKHost::get()->getMyPlayerProfiles();
            // For now send a vote for each local player
            for(unsigned int i=0; i<players.size(); i++)
            {
                clrp->requestKartSelection(players[i]->getGlobalPlayerId(),
                                           str2);
            }   // for i in players
        }
        else if (str == "vote" && NetworkConfig::get()->isClient())
        {
            std::cout << "Vote for ? (track/laps/reversed/major/minor/race#) :";
            std::string str2;
            getline(std::cin, str2);
            LobbyProtocol* protocol = LobbyProtocol::get();
            ClientLobby* clrp =
                               dynamic_cast<ClientLobby*>(protocol);
            std::vector<NetworkPlayerProfile*> players =
                                     STKHost::get()->getMyPlayerProfiles();
            if (str2 == "track")
            {
                std::cin >> str2;
                // For now send a vote for each local player
                for(unsigned int i=0; i<players.size(); i++)
                    clrp->voteTrack(i, str2);
            }
            else if (str2 == "laps")
            {
                int cnt;
                std::cin >> cnt;
                for(unsigned int i=0; i<players.size(); i++)
                    clrp->voteLaps(i, cnt);
            }
            else if (str2 == "reversed")
            {
                bool cnt;
                std::cin >> cnt;
                for(unsigned int i=0; i<players.size(); i++)
                    clrp->voteReversed(i, cnt);
            }
            else if (str2 == "major")
            {
                int cnt;
                std::cin >> cnt;
                for(unsigned int i=0; i<players.size(); i++)
                    clrp->voteMajor(i, cnt);
            }
            else if (str2 == "minor")
            {
                int cnt;
                std::cin >> cnt;
                for(unsigned int i=0; i<players.size(); i++)
                    clrp->voteMinor(i, cnt);
            }
            else if (str2 == "race#")
            {
                int cnt;
                std::cin >> cnt;
                for(unsigned int i=0; i<players.size(); i++)
                    clrp->voteRaceCount(i, cnt);
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
