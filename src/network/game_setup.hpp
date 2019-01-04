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

#include <irrString.h>

#include <atomic>
#include <cassert>
#include <memory>
#include <string>
#include <vector>

class NetworkPlayerProfile;
class NetworkString;
class PeerVote;

// ============================================================================
/*! \class GameSetup
 *  \brief Used to store the needed data about the players that join a game.
 *  This class stores all the possible information about players in a lobby.
 */
class GameSetup
{
private:
    std::vector<std::string> m_tracks;

    unsigned m_laps;

    int m_extra_server_info;

    int m_hit_capture_limit;

    float m_battle_time_limit;

    bool m_reverse;

    std::atomic_bool m_is_grand_prix;

    irr::core::stringw m_message_of_today;

    /** Utf8 server name (with xml decoded) */
    std::string m_server_name_utf8;

public:
    // ------------------------------------------------------------------------
    GameSetup();
    // ------------------------------------------------------------------------
    ~GameSetup() {}
    // ------------------------------------------------------------------------
    void setRace(const PeerVote &vote);
    // ------------------------------------------------------------------------
    void reset()
    {
        if (!isGrandPrixStarted())
            m_tracks.clear();
        m_laps = 0;
        m_reverse = false;
        m_hit_capture_limit = 0;
        m_battle_time_limit = 0.0f;
    }
    // ------------------------------------------------------------------------
    void resetExtraServerInfo()
    {
        m_is_grand_prix.store(false);
        m_extra_server_info = -1;
    }
    // ------------------------------------------------------------------------
    void setGrandPrixTrack(int tracks_no)
    {
        m_is_grand_prix.store(true);
        m_extra_server_info = tracks_no;
    }
    // ------------------------------------------------------------------------
    void addServerInfo(NetworkString* ns);
    // ------------------------------------------------------------------------
    void loadWorld();
    // ------------------------------------------------------------------------
    bool isGrandPrix() const                 { return m_is_grand_prix.load(); }
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
    const std::string& getCurrentTrack() const      { return m_tracks.back(); }
    // ------------------------------------------------------------------------
    void sortPlayersForGrandPrix(
        std::vector<std::shared_ptr<NetworkPlayerProfile> >& players) const;
    // ------------------------------------------------------------------------
    void sortPlayersForGame(
        std::vector<std::shared_ptr<NetworkPlayerProfile> >& players) const;
    // ------------------------------------------------------------------------
    void setHitCaptureTime(int hc, float time)
    {
        m_hit_capture_limit = hc;
        m_battle_time_limit = time;
    }
    // ------------------------------------------------------------------------
    const std::string& getServerNameUtf8() const { return m_server_name_utf8; }
};

#endif // GAME_SETUP_HPP
