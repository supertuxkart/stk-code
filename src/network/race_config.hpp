//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2014-2015 SuperTuxKart-Team
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

#ifndef RACE_CONFIG_HPP
#define RACE_CONFIG_HPP

#include <string>
#include <vector>
#include <map>
#include "utils/types.hpp"

/** Stores the name of a track, number of laps, and reverse driving.
 */
class TrackInfo
{
public:
    std::string track;
    bool reversed;
    uint8_t laps;
    TrackInfo() { laps = 0; reversed = false; }
};   // TrackInfo

// ============================================================================
/** Stores a vote about the name of a track, number of laps, and reverse
 *  driving.
 */
class TrackVote
{
public:
    TrackVote();

    void voteTrack(const std::string &track);
    void voteReversed(bool reversed);
    void voteLaps(uint8_t laps);

    TrackInfo track_info;

    bool has_voted_track;
    bool has_voted_reversed;
    bool has_voted_laps;
};   // class TrackVote

// ============================================================================
class RaceVote
{
    public:
    RaceVote();

    void voteMajor(uint32_t major);
    void voteRaceCount(uint8_t count);
    void voteMinor(uint32_t minor);
    void voteTrack(const std::string &track, uint8_t track_number = 0);
    void voteReversed(bool reversed, uint8_t track_number = 0);
    void voteLaps(uint8_t laps, uint8_t track_number = 0);

    bool hasVotedMajor() const;
    bool hasVotedRacesCount() const;
    bool hasVotedMinor() const;
    bool hasVotedTrack(uint8_t track_number = 0) const;
    bool hasVotedReversed(uint8_t track_number = 0) const;
    bool hasVotedLaps(uint8_t track_number = 0) const;

    uint8_t getMajorVote() const;
    uint8_t getRacesCountVote() const;
    uint8_t getMinorVote() const;
    const std::string &getTrackVote(uint8_t track_number = 0) const;
    bool getReversedVote(uint8_t track_number = 0) const;
    uint8_t getLapsVote(uint8_t track_number = 0) const;

    private:
    uint32_t m_major_mode;
    uint32_t m_minor_mode;
    uint8_t m_races_count; //!< Stores the number of races that will be in a GP
    bool m_has_voted_major;
    bool m_has_voted_minor;
    bool m_has_voted_races_count;
    std::vector<TrackVote> m_tracks_vote;
};   // RaceVote

// ============================================================================
class RaceConfig
{
private:
    void computeRaceMode();
    void computeNextTrack();
public:
    RaceConfig();

    void setPlayerMajorVote(uint8_t player_id, uint32_t major);
    void setPlayerRaceCountVote(uint8_t player_id, uint8_t count);
    void setPlayerMinorVote(uint8_t player_id, uint32_t minor);
    void setPlayerTrackVote(uint8_t player_id, const std::string &track,
                            uint8_t track_nb = 0);
    void setPlayerReversedVote(uint8_t player_id, bool reversed,
                               uint8_t track_nb = 0);
    void setPlayerLapsVote(uint8_t player_id, uint8_t lap_count,
                           uint8_t track_nb = 0);

    void loadWorld();

    const TrackInfo* getNextTrackInfo() const;
    bool getReverse() const;
    bool getLapCount() const;
    int getNumTrackVotes() const;

    const RaceVote& getRaceVote(int global_player_id) { return m_votes[global_player_id]; }
    int getMaxPlayers() const { return m_max_players; }

protected:
    std::vector<TrackInfo> m_tracks;
    int m_minor_mode;
    int m_major_mode;
    unsigned int m_races_count;

    /** Key: globalPlayerID */
    std::map<int, RaceVote> m_votes;

    uint8_t m_max_players;
};   // class RaceConfig


#endif // RACE_CONFIG_HPP
