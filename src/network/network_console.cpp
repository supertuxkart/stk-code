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

#include "network/network_config.hpp"
#include "network/network_player_profile.hpp"
#include "network/server_config.hpp"
#include "network/stk_host.hpp"
#include "network/stk_peer.hpp"
#include "network/protocols/server_lobby.hpp"
#include "utils/time.hpp"
#include "utils/vs.hpp"
#include "main_loop.hpp"

#include <iostream>
#include <limits>

namespace NetworkConsole
{
// ----------------------------------------------------------------------------
void showHelp()
{
    std::cout << "Available command:" << std::endl;
    std::cout << "help, Print this." << std::endl;
    std::cout << "quit, Shut down the server." << std::endl;
    std::cout << "kickall, Kick all players out of STKHost." << std::endl;
    std::cout << "kick #, kick # peer of STKHost." << std::endl;
    std::cout << "kickban #, kick and ban # peer of STKHost." << std::endl;
    std::cout << "listpeers, List all peers with host ID and IP." << std::endl;
    std::cout << "listban, List IP ban list of server." << std::endl;
    std::cout << "speedstats, Show upload and download speed." << std::endl;
}   // showHelp

// ----------------------------------------------------------------------------
void mainLoop(STKHost* host)
{
    VS::setThreadName("NetworkConsole");
    showHelp();
    std::string str = "";
    while (!host->requestedShutdown())
    {
        getline(std::cin, str);
        std::stringstream ss(str);
        int number = -1;
        ss >> str >> number;
        if (str == "help")
        {
            showHelp();
        }
        else if (str == "quit")
        {
            host->requestShutdown();
        }
        else if (str == "kickall")
        {
            auto peers = host->getPeers();
            for (unsigned int i = 0; i < peers.size(); i++)
            {
                peers[i]->kick();
            }
        }
        else if (str == "kick" && number != -1 &&
            NetworkConfig::get()->isServer())
        {
            std::shared_ptr<STKPeer> peer = host->findPeerByHostId(number);
            if (peer)
                peer->kick();
            else
                std::cout << "Unknown host id: " << number << std::endl;
        }
        else if (str == "kickban" && number != -1 &&
            NetworkConfig::get()->isServer())
        {
            std::shared_ptr<STKPeer> peer = host->findPeerByHostId(number);
            if (peer)
            {
                peer->kick();
                // ATM use permanently ban
                auto sl = LobbyProtocol::get<ServerLobby>();
                // We don't support banning ipv6 address atm
                if (sl && peer->getIPV6Address().empty())
                    sl->saveIPBanTable(peer->getAddress());
            }
            else
                std::cout << "Unknown host id: " << number << std::endl;
        }
        else if (str == "listpeers")
        {
            auto peers = host->getPeers();
            if (peers.empty())
                std::cout << "No peers exist" << std::endl;
            for (unsigned int i = 0; i < peers.size(); i++)
            {
                std::cout << peers[i]->getHostId() << ": " <<
                    peers[i]->getRealAddress() <<  " " <<
                    peers[i]->getUserVersion() << std::endl;
            }
        }
        else if (str == "listban")
        {
            auto sl = LobbyProtocol::get<ServerLobby>();
            if (sl)
                sl->listBanTable();
        }
        else if (str == "speedstats")
        {
            std::cout << "Upload speed (KBps): " <<
                (float)host->getUploadSpeed() / 1024.0f <<
                "   Download speed (KBps): " <<
                (float)host->getDownloadSpeed() / 1024.0f  << std::endl;
        }
        else
        {
            std::cout << "Unknown command: " << str << std::endl;
        }
    }   // while !stop
    main_loop->requestAbort();
}   // mainLoop

}
