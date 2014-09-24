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

/*! \file network_manager.hpp
 *  \brief Instantiates the generic functionnalities of a network manager.
 */

#ifndef NETWORKMANAGER_HPP
#define NETWORKMANAGER_HPP

#include "network/stk_peer.hpp"
#include "network/stk_host.hpp"

#include "network/protocol_manager.hpp"
#include "network/types.hpp"
#include "network/event.hpp"
#include "network/game_setup.hpp"
#include "utils/singleton.hpp"

#include <vector>

/** \class NetworkManager
 *  \brief Gives the general functions to use network communication.
 *  This class is in charge of storing the peers connected to this host.
 *  It also stores the host, and brings the functions to send messages to peers.
 *  It automatically dispatches the events or packets it receives. This class
 *  also stores the public address when known and the player login.
 *  Here are defined some functions that will be specifically implemented by
 *  the ServerNetworkManager and the ClientNetworkManager.
 */
class NetworkManager : public AbstractSingleton<NetworkManager>
{
    friend class AbstractSingleton<NetworkManager>;
    public:
        /** \brief Function to start the Network Manager (start threads) */
        virtual void run();
        /** \brief Function to reset the Network Manager.
         *  This function resets the peers and the listening host.
         */
        virtual void reset();
        /** \brief Function that aborts the NetworkManager.
         *  This function will stop the listening, delete the host and stop
         *  threads that are related to networking.
         */
        virtual void abort();

        // network management functions
        /** \brief Try to establish a connection to a given transport address.
         *  \param peer : The transport address which you want to connect to.
         *  \return True if we're successfully connected. False elseway.
         */
        virtual bool connect(TransportAddress peer);
        /** \brief Changes the socket working mode.
         *  Sockets can be in two modes : The ENet mode and a mode we will call
         *  the 'Raw' mode. In the ENet mode, the socket will be read as
         *  \param peer : The transport address which you want to connect to.
         *  \return True if we're successfully connected. False elseway.
         */
        virtual void setManualSocketsMode(bool manual);

        // message/packets related functions
        virtual void notifyEvent(Event* event);
        virtual void sendPacket(const NetworkString& data,
                                bool reliable = true) = 0;
        virtual void sendPacket(STKPeer* peer,
                                const NetworkString& data,
                                bool reliable = true);
        virtual void sendPacketExcept(STKPeer* peer,
                                const NetworkString& data,
                                bool reliable = true);

        // Game related functions
        virtual GameSetup* setupNewGame(); //!< Creates a new game setup and returns it
        virtual void disconnected(); //!< Called when you leave a server

        // raw data management
        void setLogin(std::string username, std::string password);
        void setPublicAddress(TransportAddress addr);
        void removePeer(STKPeer* peer);

        // getters
        virtual bool peerExists(TransportAddress peer);
        virtual bool isConnectedTo(TransportAddress peer);

        virtual bool isServer() = 0;
        inline bool isClient()              { return !isServer();         }
        bool isPlayingOnline()              { return m_playing_online;    }
        STKHost* getHost()                  { return m_localhost;         }
        std::vector<STKPeer*> getPeers()    { return m_peers;             }
        unsigned int getPeerCount()         { return (int)m_peers.size(); }
        TransportAddress getPublicAddress() { return m_public_address;    }
        GameSetup* getGameSetup()           { return m_game_setup;        }

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
};

#endif // NETWORKMANAGER_HPP
