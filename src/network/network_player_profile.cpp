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

#include "network/network_player_profile.hpp"
#include "network/network_config.hpp"
#include "network/stk_host.hpp"

// ----------------------------------------------------------------------------
/** Returns true if this player is local, i.e. running on this computer. This
 *  is done by comparing the host id of this player with the host id of this
 *  computer.
 */
bool NetworkPlayerProfile::isLocalPlayer() const
{
    // Server never has local player atm
    return NetworkConfig::get()->isClient() &&
        m_host_id == STKHost::get()->getMyHostId();
}   // isLocalPlayer
