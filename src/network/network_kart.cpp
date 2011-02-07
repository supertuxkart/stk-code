//  $Id$
//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2004-2005 Steve Baker <sjbaker1@airmail.net>
//  Copyright (C) 2006 SuperTuxKart-Team, Joerg Henrichs, Steve Baker
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
#include "network/network_kart.hpp"

/** A network kart. On the server, it receives its control information (steering etc)
    from the network manager.
    */
NetworkKart::NetworkKart(const std::string &kart_name, Track* track, int position, 
                         const btTransform &init_transform, int global_player_id,
                         RaceManager::KartType type)
                         : Kart(kart_name, track, position, init_transform, type)
{
    m_global_player_id = global_player_id;
}   // NetworkKart

// ----------------------------------------------------------------------------
void NetworkKart::setControl(const KartControl& kc)
{
    assert(network_manager->getMode()==NetworkManager::NW_SERVER);
    m_controls = kc;
}   // setControl

