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

#include "online/online_player_profile.hpp"

/** Constructor. The player id is a unique number assigned from the server
 *  to this player (though it might not be the index in the peer list).
 */
NetworkPlayerProfile::NetworkPlayerProfile(int race_player_id,
                                           const irr::core::stringw &name)
{
    m_global_player_id      = race_player_id;
    m_kart_name             = "";
    m_world_kart_id         = 0;
    m_per_player_difficulty = PLAYER_DIFFICULTY_NORMAL;
    m_player_name           = name;
}   // BetworkPlayerProfile

// ----------------------------------------------------------------------------
NetworkPlayerProfile::~NetworkPlayerProfile()
{
}   // ~NetworkPlayerProfile
