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

#ifndef GAME_SETUP_HPP
#define GAME_SETUP_HPP

#include "network/race_config.hpp"
#include "network/remote_kart_info.hpp"

#include <vector>
#include <string>

namespace Online { class OnlineProfile; }
class NetworkPlayerProfile;


// ============================================================================
/*! \class GameSetup
 *  \brief Used to store the needed data about the players that join a game.
 *  This class stores all the possible information about players in a lobby.
 */
class GameSetup
{
private:
    /** Information about all connected players. */
    std::vector<NetworkPlayerProfile*> m_players;

    /** The race configuration. */
    RaceConfig* m_race_config;

    /** Stores the number of local players. */
    int m_num_local_players;

    /** The player id of the local game master, used in 
     *  kart selection screen. */
    uint8_t m_local_master;
public:
             GameSetup();
    virtual ~GameSetup();

    void addPlayer(NetworkPlayerProfile* profile); //!< Add a player.
    bool removePlayer(const NetworkPlayerProfile *profile);
    void setPlayerKart(uint8_t player_id, const std::string &kart_name);
    void bindKartsToProfiles(); //!< Sets the right world_kart_id in profiles
    void setLocalMaster(uint8_t player_id);

    bool isLocalMaster(uint8_t player_id);
    const NetworkPlayerProfile* getProfile(uint8_t id);
    const NetworkPlayerProfile* getProfile(const std::string &kart_name);
    std::vector<NetworkPlayerProfile*> getAllPlayersOnHost(uint8_t host_id);

    /*! \brief Used to know if a kart is available.
     *  \param kart_name : Name of the kart to check.
     *  \return True if the kart hasn't been selected yet, false elseway.
     */
    bool isKartAvailable(std::string kart_name);
    // ------------------------------------------------------------------------
    /** Sets the number of local players. */
    void setNumLocalPlayers(int n) { m_num_local_players = n; } 
    // ------------------------------------------------------------------------
    /** Returns the nunber of local players. */
    int getNumLocalPlayers() const { return m_num_local_players; }
    // ------------------------------------------------------------------------
    /*! \brief Used to know if a kart is playable.
     *  \param kart_name : Name of the kart to check.
     *  \return True if the kart is playable (standard kart).
     *  Currently this is always true as the kart selection screen shows
     *  only the standard karts.
     */
    bool isKartAllowed(std::string kart_name) { return true; }
    // ------------------------------------------------------------------------
    /** Returns the configuration for this race. */
    RaceConfig* getRaceConfig() { return m_race_config; }
    // ------------------------------------------------------------------------
    /** \brief Get the players that are in the game
    *  \return A vector containing pointers on the players profiles. */
    const std::vector<NetworkPlayerProfile*>& getPlayers() const
    {
        return m_players;
    }   // getPlayers
    // ------------------------------------------------------------------------
    /** Returns the number of connected players. */
    int getPlayerCount() { return (int)m_players.size(); }
    // ------------------------------------------------------------------------
    /** Returns the id of the local master. */
    int getLocalMasterID() const { return m_local_master; }
};

#endif // GAME_SETUP_HPP
