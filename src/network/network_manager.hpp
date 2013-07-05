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

#ifndef NETWORKMANAGER_HPP
#define NETWORKMANAGER_HPP

#include "network/stk_peer.hpp"
#include "network/stk_host.hpp"

#include "network/protocol_manager.hpp"
#include "network/singleton.hpp"
#include "network/types.hpp"
#include "network/event.hpp"
#include "network/game_setup.hpp"

#include <vector>

class NetworkManager : public Singleton<NetworkManager>
{
    friend class Singleton<NetworkManager>;
    public:
        virtual void run(); 
        
        // network management functions
        virtual bool connect(TransportAddress peer);
        virtual void setManualSocketsMode(bool manual);
        
        // message/packets related functions
        virtual void notifyEvent(Event* event);
        virtual void sendPacket(const NetworkString& data) = 0;
        virtual void sendPacket(STKPeer* peer, const NetworkString& data);
        virtual void sendPacketExcept(STKPeer* peer, const NetworkString& data);

        // Game related functions
        virtual GameSetup* setupNewGame(); //!< Creates a new game setup and returns it

        // raw data management
        void setLogin(std::string username, std::string password);
        void setPublicAddress(TransportAddress addr);
        
        // getters
        virtual bool peerExists(TransportAddress peer);
        virtual bool isConnectedTo(TransportAddress peer);
        
        virtual bool isServer() = 0;
        inline bool isClient()              { return !isServer();       }
        bool isPlayingOnline()              { return m_playing_online;  }
        STKHost* getHost()                  { return m_localhost;       }
        std::vector<STKPeer*> getPeers()    { return m_peers;           }
        GameSetup* getGameSetup()           { return m_game_setup;      }
        
    protected:
        NetworkManager();
        virtual ~NetworkManager();
        
        // protected members
        std::vector<STKPeer*> m_peers;
        STKHost* m_localhost;
        bool m_playing_online;
        GameSetup* m_game_setup;
        
        TransportAddress m_public_address;
        PlayerLogin m_player_login;
        
        pthread_t* m_protocol_manager_update_thread;
};

#endif // NETWORKMANAGER_HPP
