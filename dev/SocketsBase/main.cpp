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

#include <iostream>
#include <enet/enet.h>

#include "client_network_manager.hpp"
#include "server_network_manager.hpp"
#include "protocol_manager.hpp"
#include "protocols/get_public_address.hpp"
#include "http_functions.hpp"
#include "protocols/connect_to_server.hpp"
#include "protocols/hide_public_address.hpp"
#include "network_interface.hpp"

#include <stdio.h>
#include <string.h>

using namespace std;

int main()
{

    HTTP::init();
    
    std::string answer;
    cout << "host or client:";
    answer = "client";
    cin >> answer;
    if (answer == "client")
    {
        ClientNetworkManager::getInstance()->run();
        
        std::string nickname;
        std::string password;
        std::string hostNickname;
        std::cout << "Username=";
        std::cin >> nickname;
        std::cout << "Password=";
        std::cin >> password;
        
        //NetworkManager::getInstance()->run();
        
        NetworkManager::getInstance()->setLogin(nickname, password);
        bool connected = false;
        //clt.connect(0x0100007f, 7000); // addr in little endian, real address is 7f 00 00 01 (127.0.0.1)
        std::string buffer;
        while (1)
        {
            getline(std::cin, buffer);
            if (buffer == "cmd=connect")
            {
                cout << "Host Nickname=";
                std::cin >> hostNickname;
                connected = ClientNetworkManager::getInstance()->connectToHost(hostNickname);
                continue;
            }
            if (buffer == "cmd=login")
            {
                std::cout << "Username=";
                std::cin >> nickname;
                std::cout << "Password=";
                std::cin >> password;
                NetworkManager::getInstance()->setLogin(nickname, password);
            }
            if (buffer.size() == 0) { continue; }
            char buffer2[256];
            strcpy(buffer2, buffer.c_str());
            if (connected)
                ClientNetworkManager::getInstance()->sendPacket(buffer2);
        }
        
        
        enet_deinitialize();
    }
    else if (answer == "host")
    {
        std::string nickname;
        std::string password;
        std::cout << "Username=";
        std::cin >> nickname;
        std::cout << "Password=";
        std::cin >> password;
        //NetworkInterface::getInstance()->initNetwork(true);
        ServerNetworkManager::getInstance()->setLogin(nickname, password);
        ServerNetworkManager::getInstance()->run();
        ServerNetworkManager::getInstance()->start();
        //GetPublicAddress
        
        std::string buffer;
        while (1)
        {
            getline(std::cin, buffer);
            if (buffer == "cmd=connect")
            {
                std::string peer;
                std::cout << "Peer nickname=";
                std::cin >> peer;
                ServerNetworkManager::getInstance()->connectToPeer(peer);
                continue;
            }
            if (buffer == "cmd=talkto")
            {
                uint32_t peer;
                std::cout << "Peer ip=";
                std::cin >> peer;
                uint32_t port;
                std::cout << "Peer port=";
                std::cin >> port;
                ServerNetworkManager::getInstance()->setManualSocketsMode(true);
                char msg[] = "test";
                ServerNetworkManager::getInstance()->getHost()->sendRawPacket((uint8_t*)(msg), sizeof(msg), TransportAddress(peer, port));
                ServerNetworkManager::getInstance()->setManualSocketsMode(false);
                continue;
            }
            if (buffer.size() == 0) { continue; }
            char buffer2[256];
            strcpy(buffer2, buffer.c_str());
            
        }
    }
    return 0;
}
