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

/*! \file game_setup.hpp
 */

#ifndef HEADER_NETWORK_PLAYER_PROFILE
#define HEADER_NETWORK_PLAYER_PROFILE

#include "network/remote_kart_info.hpp"
#include "utils/types.hpp"

#include "irrString.h"

#include <string>
namespace Online { class OnlineProfile; }


/*! \class NetworkPlayerProfile
 *  \brief Contains the profile of a player.
 */
class NetworkPlayerProfile
{
private:
    /** The unique id of the player for this race. The number is assigned
     *  by the server (and it might not be the index of this player in the
     *  peer list. */
    uint8_t m_global_player_id;

    /** Host id of this player. */
    uint8_t m_host_id;

    /** The selected kart id. */
    std::string m_kart_name; 

    /** The name of the player. */
    irr::core::stringw m_player_name;

    /** The kart id in the World class (pointer to AbstractKart). */
    uint8_t m_world_kart_id;

    /** Per player difficulty. */
    PerPlayerDifficulty m_per_player_difficulty;
public:
         NetworkPlayerProfile(const irr::core::stringw &name,
                              int global_player_id, int host_id);
        ~NetworkPlayerProfile();
    bool isLocalPlayer() const;

    // ------------------------------------------------------------------------
    /** Sets the global player id of this player. */
    void setGlobalPlayerId(int player_id) { m_global_player_id = player_id; }
    // ------------------------------------------------------------------------
    /** Returns the global ID of this player in this race. */
    int getGlobalPlayerId() const { return m_global_player_id; }
    // ------------------------------------------------------------------------
    /** Returns the host id of this player. */
    uint8_t getHostId() const { return m_host_id; }
    // ------------------------------------------------------------------------
    /** Sets the kart name for this player. */
    void setKartName(const std::string &kart_name) { m_kart_name = kart_name; }
    // ------------------------------------------------------------------------
    /** Returns the name of the kart this player has selected. */
    const std::string &getKartName() const { return m_kart_name; }
    // ------------------------------------------------------------------------
    /** Sets the world kart id for this player. */
    void setWorldKartID(int id) { m_world_kart_id = id; }
    // ------------------------------------------------------------------------
    /** Retuens the world kart id for this player. */
    int getWorldKartID() const { return m_world_kart_id; }
    // ------------------------------------------------------------------------
    /** Returns the per-player difficulty. */
    PerPlayerDifficulty getPerPlayerDifficulty() const
    {
        return m_per_player_difficulty;
    }   // getPerPlayerDifficulty
    // ------------------------------------------------------------------------
    /** Returns the name of this player. */
    const irr::core::stringw& getName() const { return m_player_name; }
    // ------------------------------------------------------------------------

};   // class NetworkPlayerProfile

#endif // HEADER_NETWORK_PLAYER_PROFILE
