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

/*! \file client_network_manager.hpp
 *  \brief Defines a Client Network manager, that will connect to a server.
 */

#ifndef CLIENT_NETWORK_MANAGER_HPP
#define CLIENT_NETWORK_MANAGER_HPP

#include "network/network_manager.hpp"

/*! \class ClientNetworkManager
 *  \ingroup network
 */
class ClientNetworkManager : public NetworkManager
{
    friend class AbstractSingleton<NetworkManager>;
    public:
        /*! \brief Get the instance.
         *  This is a utility function to avoid passing templates parameters
         *  to the getInstance singleton method.
         */
        static ClientNetworkManager* getInstance()
        {
            return AbstractSingleton<NetworkManager>::getInstance<ClientNetworkManager>();
        }

        /*! \brief Initializes network.
         *  This starts the threads and initializes network libraries.
         */
        virtual void run();
        /*! \brief Resets the network socket. */
        virtual void reset();
        /*! \brief Sends a packet to the server.
         *  \param data : The network 8-bit string to send.
         *  \param reliable : If set to true, ENet will ensure that the packet is received.
         */
        virtual void sendPacket(const NetworkString& data, bool reliable = true);

        /*! \brief Get the peer (the server)
         *  \return The peer with whom we're connected (if it exists). NULL elseway.
         */
        STKPeer* getPeer();
        /*! \brief Function to know if we're a server.
         *  \return Returns true if we're on a server. False if we're a client.
         */
        virtual bool isServer()         { return false; }
        /*! \brief Function used to notice the manager that we're connected to a server.
         *  \param value : True if we're connected, false elseway.
         */
        void setConnected(bool value)   { m_connected = value; }
        /*! \brief Function to know if we're a server.
         *  \return Returns true if we're on a server. False if we're a client.
         */
        bool isConnected()              { return m_connected; }

    protected:
        ClientNetworkManager();
        virtual ~ClientNetworkManager();

        bool m_connected; //!< Is the user connected to a server
        pthread_t* m_thread_keyboard; //!< The thread listening for keyboard console input.
};

#endif // CLIENT_NETWORK_MANAGER_HPP
