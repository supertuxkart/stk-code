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

#include "network/kart_data.hpp"
#include "utils/types.hpp"

#include "irrString.h"
#include <atomic>
#include <limits>
#include <memory>
#include <string>

class STKPeer;
enum KartTeam : int8_t;
enum HandicapLevel : uint8_t;

// Moderation toolkit
enum PlayerRestriction: uint32_t
{
    PRF_OK = 0, //!< Default, no restrictions are applied to the player
    PRF_NOSPEC = 1, //!< Player is unable to spectate
    PRF_NOGAME = 2, //!< Player is unable to play the game
    PRF_NOCHAT = 4, //!< Player is unable to send chat messages
    PRF_NOPCHAT = 8, //!< Player is unable to send private chat messages
    PRF_NOTEAM = 16, //!< Player profiles of the peer cannot change teams
    PRF_HANDICAP = 32, //!< Player is unable to toggle the handicap
    //PRF_KART = 64, //!< Player is unable to select the kart by themselved
    PRF_TRACK = 128, //!< Player is unable to vote for the track
    PRF_ITEMS = 256, //!< Player is unable to pick up items in game
};  // PlayerRestriction


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

    /** Handicap level of this player. */
    std::atomic<HandicapLevel> m_handicap;

    /** The selected kart id. */
    std::string m_kart_name; 

    /** The forced kart id if any. */
    std::string m_forced_kart_name;

    /** Cached value for permissions. Usually updated by ServerLobby. */
    uint32_t m_permission_level;

    /** Veto level: 0 is for no veto, 80 is for lobby commands veto,
     * 100 is for track selection veto. Cannot be set higher than
     * the permission level defined by the server lobby. */
    uint32_t m_veto;

    // Player restrictions are defined by the flag enum PeerRestriction
    uint32_t m_restrictions;

    /** The local player id relative to each peer. */
    uint8_t m_local_player_id;

    /** Score if grand prix. */
    int m_score;

    /** Overall time if grand prix. */
    float m_overall_time;

    std::atomic<KartTeam> m_team;

    /** 2-letter country code of player. */
    std::string m_country_code;

    KartData m_kart_data;
public:
    // ------------------------------------------------------------------------
    static std::shared_ptr<NetworkPlayerProfile>
        getReservedProfile(KartTeam team)
    {
        return std::make_shared<NetworkPlayerProfile>(team);
    }
    // ------------------------------------------------------------------------
    /* Placeholder profile for reserved player in live join, which its host id
     * is uint32_t max. */
    NetworkPlayerProfile(KartTeam team)
    {
        m_kart_name             = "tux";
        m_host_id               = std::numeric_limits<uint32_t>::max();
        m_default_kart_color    = 0.0f;
        m_online_id             = 0;
        m_handicap.store((HandicapLevel)0);
        m_local_player_id       = 0;
        m_permission_level      = 0;
        m_veto                  = 0;
        m_team.store(team);
        resetGrandPrixData();
    }
    // ------------------------------------------------------------------------
    NetworkPlayerProfile(std::shared_ptr<STKPeer> peer,
                         const irr::core::stringw &name, uint32_t host_id,
                         float default_kart_color, uint32_t online_id,
                         HandicapLevel handicap,
                         uint8_t local_player_id, KartTeam team,
                         const std::string& country_code, int permlvl = 0,
                         uint32_t restrictions = 0)
    {
        m_peer                  = peer;
        m_player_name           = name;
        m_host_id               = host_id;
        m_default_kart_color    = default_kart_color;
        m_online_id             = online_id;
        m_handicap.store(handicap);
        m_local_player_id       = local_player_id;
        m_team.store(team);
        m_country_code          = country_code;
        m_permission_level      = permlvl;
        m_veto                  = 0;
        m_restrictions          = restrictions;
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
    void setKartName(const std::string &kart_name)
    {
        m_kart_name = kart_name;
        m_kart_data = KartData();
    }
    // ------------------------------------------------------------------------
    /** Returns the name of the kart this player has selected. */
    const std::string &getKartName() const              { return m_kart_name; }
    // ------------------------------------------------------------------------
    /** Retuens the local player id for this player. */
    uint8_t getLocalPlayerId() const              { return m_local_player_id; }
    // ------------------------------------------------------------------------
    /** Returns the player's handicap. */
    HandicapLevel getHandicap() const { return m_handicap.load(); }
    // ------------------------------------------------------------------------
    void setHandicap(HandicapLevel h) { m_handicap.store(h); }
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
    // ------------------------------------------------------------------------
    void setTeam(KartTeam team)                         { m_team.store(team); }
    // ------------------------------------------------------------------------
    KartTeam getTeam() const                          { return m_team.load(); }
    // ------------------------------------------------------------------------
    const std::string& getCountryCode() const        { return m_country_code; }
    // ------------------------------------------------------------------------
    void setKartData(const KartData& data)              { m_kart_data = data; }
    // ------------------------------------------------------------------------
    const KartData& getKartData() const                 { return m_kart_data; }
    // ------------------------------------------------------------------------
    // Moderation toolkit
    // ------------------------------------------------------------------------
    const int32_t getPermissionLevel() const     { return m_permission_level; }
    // ------------------------------------------------------------------------
    void setPermissionLevel(const int32_t lvl)    { m_permission_level = lvl; }
    // ------------------------------------------------------------------------
    uint32_t getVeto() const                                 { return m_veto; }
    // ------------------------------------------------------------------------
    void setVeto(const uint32_t v)                              { m_veto = v; }
    // ------------------------------------------------------------------------
    uint32_t getRestrictions() const                 { return m_restrictions; }
    // ------------------------------------------------------------------------
    void setRestrictions(const uint32_t r)              { m_restrictions = r; }
    // ------------------------------------------------------------------------
    void clearRestrictions()                       { m_restrictions = PRF_OK; }
    // ------------------------------------------------------------------------
    void addRestriction(const PlayerRestriction restriction)
                                             { m_restrictions |= restriction; }
    // ------------------------------------------------------------------------
    void removeRestriction(const PlayerRestriction restriction)
                                            { m_restrictions &= ~restriction; }
    // ------------------------------------------------------------------------
    bool notRestrictedBy(const PlayerRestriction restriction)
                                      { return ~m_restrictions & restriction; }
    // ------------------------------------------------------------------------
    bool hasRestriction(const PlayerRestriction restriction)
                                       { return m_restrictions & restriction; }
    // ------------------------------------------------------------------------
    const std::string& getForcedKart() const     { return m_forced_kart_name; }
    // ------------------------------------------------------------------------
    void forceKart(const std::string& name)      { m_forced_kart_name = name; }
    // ------------------------------------------------------------------------
    void unforceKart()                          { m_forced_kart_name.clear(); }
    // ------------------------------------------------------------------------

};   // class NetworkPlayerProfile

#endif // HEADER_NETWORK_PLAYER_PROFILE
