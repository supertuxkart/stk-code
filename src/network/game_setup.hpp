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

#include "network/remote_kart_info.hpp"

#include <memory>
#include <mutex>
#include <string>
#include <vector>

class NetworkPlayerProfile;

// ============================================================================
/*! \class GameSetup
 *  \brief Used to store the needed data about the players that join a game.
 *  This class stores all the possible information about players in a lobby.
 */
class GameSetup
{
private:
    mutable std::mutex m_players_mutex;

    /** Information about all connected players. */
    std::vector<std::weak_ptr<NetworkPlayerProfile> > m_players;

    /** Stores the number of local players. */
    int m_num_local_players;

    /** The player id of the local game master, used in 
     *  kart selection screen. */
    uint8_t m_local_master;

    std::string m_track;

    unsigned m_laps;

    bool m_reverse;

public:
    // ------------------------------------------------------------------------
    GameSetup();
    // ------------------------------------------------------------------------
    ~GameSetup() {}
    // ------------------------------------------------------------------------
    void addPlayer(std::shared_ptr<NetworkPlayerProfile> profile)
                                              { m_players.push_back(profile); }
    // ------------------------------------------------------------------------
    void update(bool remove_disconnected_players);
    // ------------------------------------------------------------------------
    /** Sets the number of local players. */
    void setNumLocalPlayers(int n) { m_num_local_players = n; } 
    // ------------------------------------------------------------------------
    /** Returns the nunber of local players. */
    int getNumLocalPlayers() const { return m_num_local_players; }
    // ------------------------------------------------------------------------
    /** \brief Get the players that are / were in the game
    *  \return A vector containing pointers on the players profiles. */
    const std::vector<std::weak_ptr<NetworkPlayerProfile> >& getPlayers() const
    {
        std::lock_guard<std::mutex> lock(m_players_mutex);
        return m_players;
    }   // getPlayers
    // ------------------------------------------------------------------------
    /** \brief Get the players that are in the game
    *  \return A vector containing pointers on the players profiles. */
    std::vector<std::shared_ptr<NetworkPlayerProfile> >
        getConnectedPlayers() const
    {
        std::lock_guard<std::mutex> lock(m_players_mutex);
        std::vector<std::shared_ptr<NetworkPlayerProfile> > players;
        for (auto player_weak : m_players)
        {
            if (auto player_connected = player_weak.lock())
                players.push_back(player_connected);
        }
        return players;
    }   // getConnectedPlayers
    // ------------------------------------------------------------------------
    /** Returns the number of connected players. */
    unsigned getPlayerCount()
    {
        std::lock_guard<std::mutex> lock(m_players_mutex);
        return (unsigned)m_players.size();
    }
    // ------------------------------------------------------------------------
    /** Returns the id of the local master. */
    int getLocalMasterID() const { return m_local_master; }
    // ------------------------------------------------------------------------
    void setRace(const std::string& track, unsigned laps, bool reverse)
    {
        m_track = track;
        m_laps = laps;
        m_reverse = reverse;
    }
    // ------------------------------------------------------------------------
    void loadWorld();
};

#endif // GAME_SETUP_HPP
