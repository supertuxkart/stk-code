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

/*! \file network_manager.hpp
 *  \brief Instantiates the generic functionnalities of a network manager.
 */

#ifndef NETWORKMANAGER_HPP
#define NETWORKMANAGER_HPP

#include "network/stk_peer.hpp"
#include "network/stk_host.hpp"

#include "network/protocol_manager.hpp"
#include "network/types.hpp"
#include "utils/singleton.hpp"
#include "utils/synchronised.hpp"

#include <vector>

class Event;
class GameSetup;

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
protected:
             NetworkManager();
    virtual ~NetworkManager() {};


private:

    PlayerLogin m_player_login;


    friend class AbstractSingleton<NetworkManager>;
public:


};   // class NetworkManager

#endif // NETWORKMANAGER_HPP
