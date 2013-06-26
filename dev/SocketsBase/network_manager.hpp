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

#include "stk_peer.hpp"
#include "stk_host.hpp"
#include <vector>

#include "protocol_manager.hpp"
#include "singleton.hpp"
#include "types.hpp"
#include "event.hpp"

class NetworkManager : public Singleton<NetworkManager>
{
    friend class Singleton<NetworkManager>;
    public:
        virtual void run(); 
        
        // network management functions
        virtual bool connect(TransportAddress peer);
        virtual void setManualSocketsMode(bool manual);
        virtual void notifyEvent(Event* event);
        virtual void packetReceived(char* data) = 0;

        // raw data management
        void setLogin(std::string username, std::string password);
        void setPublicAddress(TransportAddress addr);
        
        // getters
        virtual bool peerExists(TransportAddress peer);
        virtual bool isConnectedTo(TransportAddress peer);
        STKHost* getHost();
        std::vector<STKPeer*> getPeers();
    protected:
        NetworkManager();
        virtual ~NetworkManager();
        
        // protected members
        std::vector<STKPeer*> m_peers;
        STKHost* m_localhost;
        
        TransportAddress m_public_address;
        PlayerLogin m_player_login;
        
        pthread_t* m_protocol_manager_update_thread;
};

#endif // NETWORKMANAGER_HPP
