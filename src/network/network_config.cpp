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

NetworkConfig *NetworkConfig::m_network_config = NULL;
bool           NetworkConfig::m_disable_lan    = false;

/** \class NetworkConfig
 *  This class is the interface between STK and the online code, particularly
 *  STKHost. It stores all online related properties (e.g. if this is a server
 *  or a host, name of the server, maximum number of players, ip address, ...).
 *  They can either be set from the GUI code, or via the command line (for a
 *  stand-alone server).
 *  When STKHost is created, it takes all necessary information from this
 *  instance.
 */
// ============================================================================
/** Constructor.
 */
NetworkConfig::NetworkConfig()
{
    m_network_type          = NETWORK_NONE;
    m_is_server             = false;
    m_is_public_server      = false;
    m_max_players           = 4;
    m_is_registered         = false;
    m_server_name           = "";
    m_password              = "";
    m_server_discovery_port = 2757;
    m_server_port           = 2758;
    m_client_port           = 2759;
    m_my_address.lock();
    m_my_address.getData().clear();
    m_my_address.unlock();
}   // NetworkConfig

//-----------------------------------------------------------------------------
/** Stores the public address of this host.
 */
void NetworkConfig::setMyAddress(const TransportAddress& addr)
{
    m_my_address.lock();
    m_my_address.getData().copy(addr);
    m_my_address.unlock();
}   // setPublicAddress

// --------------------------------------------------------------------
/** Sets if this instance is a server or client. It also assigns the
 *  private port depending if this is a server or client.
 */
void NetworkConfig::setIsServer(bool b)
{
    m_is_server = b;
}   // setIsServer
