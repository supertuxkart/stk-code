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
#include <memory>
#include <string>
#include <tuple>

class STKPeer;

/*! \class NetworkPlayerProfile
 *  \brief Contains the profile of a player.
 */
class NetworkPlayerProfile
{
private:
    std::weak_ptr<STKPeer> m_peer;

    /** The name of the player. */
    irr::core::stringw m_player_name;

    /** Host id of this player. */
    uint32_t m_host_id;

    float m_default_kart_color;

    uint32_t m_online_id;

    /** Per player difficulty. */
    PerPlayerDifficulty m_per_player_difficulty;

    /** The selected kart id. */
    std::string m_kart_name; 

    /** The local player id relative to each peer. */
    int m_local_player_id;

    /** Score if grand prix. */
    int m_score;

    /** Overall time if grand prix. */
    float m_overall_time;

public:
    NetworkPlayerProfile(std::shared_ptr<STKPeer> peer,
                         const irr::core::stringw &name, uint32_t host_id,
                         float default_kart_color, uint32_t online_id,
                         PerPlayerDifficulty per_player_difficulty,
                         uint8_t local_player_id)
    {
        m_peer                  = peer;
        m_player_name           = name;
        m_host_id               = host_id;
        m_default_kart_color    = default_kart_color;
        m_online_id             = online_id;
        m_per_player_difficulty = per_player_difficulty;
        m_local_player_id       = local_player_id;
        resetGrandPrixData();
    }
    // ------------------------------------------------------------------------
    ~NetworkPlayerProfile() {}
    // ------------------------------------------------------------------------
    bool isLocalPlayer() const;
    // ------------------------------------------------------------------------
    /** Returns the host id of this player. */
    uint32_t getHostId() const                            { return m_host_id; }
    // ------------------------------------------------------------------------
    /** Sets the kart name for this player. */
    void setKartName(const std::string &kart_name) { m_kart_name = kart_name; }
    // ------------------------------------------------------------------------
    /** Returns the name of the kart this player has selected. */
    const std::string &getKartName() const              { return m_kart_name; }
    // ------------------------------------------------------------------------
    /** Retuens the local player id for this player. */
    int getLocalPlayerId() const                  { return m_local_player_id; }
    // ------------------------------------------------------------------------
    /** Returns the per-player difficulty. */
    PerPlayerDifficulty getPerPlayerDifficulty() const
                                            { return m_per_player_difficulty; }
    // ------------------------------------------------------------------------
    /** Returns the name of this player. */
    const irr::core::stringw& getName() const         { return m_player_name; }
    // ------------------------------------------------------------------------
    float getDefaultKartColor() const          { return m_default_kart_color; }
    // ------------------------------------------------------------------------
    uint32_t getOnlineId() const                        { return m_online_id; }
    // ------------------------------------------------------------------------
    bool isOfflineAccount() const                  { return m_online_id == 0; }
    // ------------------------------------------------------------------------
    std::shared_ptr<STKPeer> getPeer() const          { return m_peer.lock(); }
    // ------------------------------------------------------------------------
    int getScore() const                                    { return m_score; }
    // ------------------------------------------------------------------------
    float getOverallTime() const                     { return m_overall_time; }
    // ------------------------------------------------------------------------
    void setScore(int score)                               { m_score = score; }
    // ------------------------------------------------------------------------
    void setOverallTime(float time)                  { m_overall_time = time; }
    // ------------------------------------------------------------------------
    void resetGrandPrixData()
    {
        m_score = 0;
        m_overall_time = 0.0f;
    }

};   // class NetworkPlayerProfile

#endif // HEADER_NETWORK_PLAYER_PROFILE
