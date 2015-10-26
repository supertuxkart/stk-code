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

#include "network/network_manager.hpp"

#include "network/event.hpp"
#include "network/game_setup.hpp"
#include "network/protocol_manager.hpp"
#include "utils/log.hpp"

#include <pthread.h>
#include <signal.h>

//-----------------------------------------------------------------------------

NetworkManager::NetworkManager()
{
}   // NetworkManager

//----------------------------------------------------------------------------
/** \brief Function to start the Network Manager (start threads).
 */
void NetworkManager::run()
{
}   // run

//-----------------------------------------------------------------------------
/** \brief Try to establish a connection to a given transport address.
 *  \param peer : The transport address which you want to connect to.
 *  \return True if we're successfully connected. False elseway.
 */
bool NetworkManager::connect(const TransportAddress& address)
{
    if (STKHost::get()->peerExists(address))
        return isConnectedTo(address);

    return STKPeer::connectToHost(STKHost::get(), address, 2, 0);
}   // connect
