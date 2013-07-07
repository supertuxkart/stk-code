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

#include "network/server_network_manager.hpp"

#include "network/protocols/get_public_address.hpp"
#include "network/protocols/hide_public_address.hpp"
#include "network/protocols/show_public_address.hpp"
#include "network/protocols/get_peer_address.hpp"
#include "network/protocols/connect_to_server.hpp"

#include "utils/log.hpp"

#include <enet/enet.h>
#include <pthread.h>

ServerNetworkManager::ServerNetworkManager()
{
    m_localhost = NULL;
}

ServerNetworkManager::~ServerNetworkManager()
{
}

void ServerNetworkManager::run()
{
    if (enet_initialize() != 0) 
    {
        Log::error("ServerNetworkManager", "Could not initialize enet.\n");
        return;
    }
    NetworkManager::run();
}

void ServerNetworkManager::start()
{

}

void ServerNetworkManager::sendPacket(const NetworkString& data)
{
    m_localhost->broadcastPacket(data);
}
