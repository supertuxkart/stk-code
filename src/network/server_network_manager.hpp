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

/*! \file server_network_manager.hpp
 */

#ifndef SERVER_NETWORK_MANAGER_HPP
#define SERVER_NETWORK_MANAGER_HPP

#include "network/network_manager.hpp"


class ServerNetworkManager : public NetworkManager
{
    friend class AbstractSingleton<NetworkManager>;
    public:
        static ServerNetworkManager* getInstance()
        {
            return AbstractSingleton<NetworkManager>::getInstance<ServerNetworkManager>();
        }

        virtual void run();

        void setMaxPlayers(uint8_t count) { m_max_players = count; }
        uint8_t getMaxPlayers() {return m_max_players;}

        void kickAllPlayers();

        virtual void sendPacket(const NetworkString& data, bool reliable = true);

        virtual bool isServer()         { return true; }

    protected:
        ServerNetworkManager();
        virtual ~ServerNetworkManager();

        pthread_t* m_thread_keyboard;
        uint8_t m_max_players;

};

#endif // SERVER_NETWORK_MANAGER_HPP
