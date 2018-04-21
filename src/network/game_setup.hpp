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

#include <cassert>
#include <memory>
#include <mutex>
#include <string>
#include <vector>

class NetworkPlayerProfile;
class NetworkString;

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

    std::vector<std::string> m_tracks;

    unsigned m_laps;

    bool m_reverse;

    int m_extra_server_info;

public:
    // ------------------------------------------------------------------------
    GameSetup()
    {
        m_extra_server_info = -1;
        reset();
    }
    // ------------------------------------------------------------------------
    ~GameSetup() {}
    // ------------------------------------------------------------------------
    void addPlayer(std::shared_ptr<NetworkPlayerProfile> profile)
                                              { m_players.push_back(profile); }
    // ------------------------------------------------------------------------
    void update(bool remove_disconnected_players);
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
    void setRace(const std::string& track, unsigned laps, bool reverse)
    {
        m_tracks.push_back(track);
        m_laps = laps;
        m_reverse = reverse;
    }
    // ------------------------------------------------------------------------
    void reset()
    {
        if (!isGrandPrixStarted())
            m_tracks.clear();
        m_laps = 0;
        m_reverse = false;
    }
    // ------------------------------------------------------------------------
    void setGrandPrixTrack(int tracks_no)  { m_extra_server_info = tracks_no; }
    // ------------------------------------------------------------------------
    void addServerInfo(NetworkString* ns);
    // ------------------------------------------------------------------------
    void loadWorld();
    // ------------------------------------------------------------------------
    bool isGrandPrix() const;
    // ------------------------------------------------------------------------
    bool hasExtraSeverInfo() const        { return m_extra_server_info != -1; }
    // ------------------------------------------------------------------------
    uint8_t getExtraServerInfo() const
    {
        assert(hasExtraSeverInfo());
        return (uint8_t)m_extra_server_info;
    }
    // ------------------------------------------------------------------------
    unsigned getTotalGrandPrixTracks() const
    {
        assert(isGrandPrix());
        return m_extra_server_info;
    }
    // ------------------------------------------------------------------------
    void setSoccerGoalTarget(bool val)      { m_extra_server_info = (int)val; }
    // ------------------------------------------------------------------------
    bool isSoccerGoalTarget() const
    {
        assert(hasExtraSeverInfo());
        return m_extra_server_info != 0;
    }
    // ------------------------------------------------------------------------
    bool isGrandPrixStarted() const
    {
        return isGrandPrix() && !m_tracks.empty() &&
            m_tracks.size() != getTotalGrandPrixTracks();
    }
    // ------------------------------------------------------------------------
    void stopGrandPrix()                                  { m_tracks.clear(); }
    // ------------------------------------------------------------------------
    const std::vector<std::string>& getAllTracks() const   { return m_tracks; }
    // ------------------------------------------------------------------------
    void sortPlayersForGrandPrix();
};

#endif // GAME_SETUP_HPP
