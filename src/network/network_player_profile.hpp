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

#include "utils/types.hpp"

#include "karts/player_difficulty.hpp"

#include <string>
namespace Online { class OnlineProfile; }


/*! \class PlayerProfile
 *  \brief Contains the profile of a player.
 */
class NetworkPlayerProfile
{
private:
    /** The id of the player for the race. */
    uint8_t m_race_player_id;

    /** The selected kart. */
    std::string m_kart_name; 

    /** Pointer to the online profile of this player. */
    Online::OnlineProfile* m_online_profile;

    /** The kart id in the World class (pointer to AbstractKart). */
    uint8_t m_world_kart_id;

    /** Per player difficulty. */
    PerPlayerDifficulty m_per_player_difficulty;
public:
    NetworkPlayerProfile(int race_player_id);
    ~NetworkPlayerProfile();
    int getGlobalID() const;

    // ------------------------------------------------------------------------
    /** Sets the player id of this player. */
    void setPlayerID(int player_id) { m_race_player_id = player_id; }
    // ------------------------------------------------------------------------
    /** Returns the loca ID of this player in this race. */
    int getPlayerID() const { return m_race_player_id; }
    // ------------------------------------------------------------------------
    /** Sets the online profile for this player. */
    void setOnlineProfile(Online::OnlineProfile *profile)
    {
        m_online_profile = profile;
    }   // setOnlineProfile
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
    /** Returns the pointer to the online profile of this player, or NULL if
     *  the player is not online. */
    Online::OnlineProfile *getOnlineProfile() { return m_online_profile; }
    // ------------------------------------------------------------------------
    /** Returns the per-player difficulty. */
    PerPlayerDifficulty getPerPlayerDifficulty() const { return m_per_player_difficulty; }
    // ------------------------------------------------------------------------
    // ------------------------------------------------------------------------

};   // class NetworkPlayerProfile

#endif // HEADER_NETWORK_PLAYER_PROFILE
