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

#include "network/race_config.hpp"

#include "config/user_config.hpp"
#include "network/network_config.hpp"
#include "race/race_manager.hpp"
#include "utils/log.hpp"

#include <stdexcept>

/** \brief Gets the element with the highest count in a std::map<S,int>.
 *  \param histogram : A pointer to the histogram.
 *  \return The key of type S that has the highest second value.
 */
template<typename S>
S getHighestInHistogram(std::map<S,int>* histogram, S default_value)
{
    if(histogram->empty())
        return default_value;

    S best_item = histogram->begin()->first;
    uint8_t highest_count = histogram->begin()->second;
    for (typename std::map<S, int>::iterator it = histogram->begin();
        it != histogram->end(); it++)
    {
        if (it->second > highest_count)
        {
            highest_count = it->second;
            best_item = it->first;
        }
    }
    return best_item;
}   // getHighestInHistogram

//-----------------------------------------------------------------------------
//---------------------------------  TrackVote --------------------------------
//-----------------------------------------------------------------------------
/** Constructor for a track vote.
 */
TrackVote::TrackVote()
{
    has_voted_laps = false;
    has_voted_track = false;
    has_voted_reversed = false;
}   // TrackVote

//-----------------------------------------------------------------------------
/** Sets that this vote is for the specified track.
 */
void TrackVote::voteTrack(const std::string &track)
{
    track_info.track = track;
    has_voted_track = true;
}   // voteTrack

//-----------------------------------------------------------------------------
/** Sets if this vote is for normal or reversed driving.
 *  \param reversed True if this vote is for reversed racing.
 */
void TrackVote::voteReversed(bool reversed)
{
    track_info.reversed = reversed;
    has_voted_reversed = true;
}   // voteReversed

//-----------------------------------------------------------------------------
/** Votes for the number of laps.
 *  \param laps Numger of laps.
 */
void TrackVote::voteLaps(uint8_t laps)
{
    track_info.laps = laps;
    has_voted_laps = true;
}   // voteLaps

//-----------------------------------------------------------------------------
//---------------------------------  RaceVote ---------------------------------
//-----------------------------------------------------------------------------
/** Constructor for a race vote.
 */
RaceVote::RaceVote()
{
    m_has_voted_major = false;
    m_has_voted_minor = false;
    m_has_voted_races_count = false;
    m_major_mode = 0;
    m_minor_mode = 0;
    m_races_count = 0;
    m_tracks_vote.resize(1);
}   // RaceVote

//-----------------------------------------------------------------------------
/** Sets the selected major race vote.
 */
void RaceVote::voteMajor(uint32_t major)
{
    m_has_voted_major = true;
    m_major_mode      = major;
}   // voteMajor

//-----------------------------------------------------------------------------
/** Sets the vote for race count.
 */
void RaceVote::voteRaceCount(uint8_t count)
{
    m_has_voted_races_count = true;
    m_races_count = count;
}   // voteRaceCount

//-----------------------------------------------------------------------------
/** Sets vote for minor race mode.
 */
void RaceVote::voteMinor(uint32_t minor)
{
    m_has_voted_minor = true;
    m_minor_mode = minor;
}   // voteMinor

//-----------------------------------------------------------------------------
/** Sets a track vote.
 *  \param track Name of the track.
 */
void RaceVote::voteTrack(const std::string &track, uint8_t track_number)
{
    m_tracks_vote[track_number].voteTrack(track);
}   // voteTrack

//-----------------------------------------------------------------------------
/** Sets a vote for reveresed racing.
 */
void RaceVote::voteReversed(bool reversed, uint8_t track_number)
{
    m_tracks_vote[track_number].voteReversed(reversed);
}   // voteReversed

//-----------------------------------------------------------------------------
/** Sets a vote for number of laps.
 */
void RaceVote::voteLaps(uint8_t laps, uint8_t track_number)
{
    m_tracks_vote[track_number].voteLaps(laps);
}   // voteLaps

//-----------------------------------------------------------------------------
bool RaceVote::hasVotedMajor() const
{
    return m_has_voted_major;
}   // hasVotedMajor
//-----------------------------------------------------------------------------
bool RaceVote::hasVotedRacesCount() const
{
    return m_has_voted_races_count;
}   // hasVotedRacesCount

//-----------------------------------------------------------------------------
bool RaceVote::hasVotedMinor() const
{
    return m_has_voted_minor;
}   // hasVotedMinor

//-----------------------------------------------------------------------------
bool RaceVote::hasVotedTrack(uint8_t track_number) const
{
    return m_tracks_vote[track_number].has_voted_track;
}   // hasVotedTrack

//-----------------------------------------------------------------------------
bool RaceVote::hasVotedReversed(uint8_t track_number) const
{
    return m_tracks_vote[track_number].has_voted_reversed;
}   // hasVotedReversed

//-----------------------------------------------------------------------------
bool RaceVote::hasVotedLaps(uint8_t track_number) const
{
    return m_tracks_vote[track_number].has_voted_laps;
}   // hasVotedLaps

//-----------------------------------------------------------------------------
uint8_t RaceVote::getMajorVote() const
{
    return m_major_mode;
}   // getMajorVote

//-----------------------------------------------------------------------------
uint8_t RaceVote::getRacesCountVote() const
{
    return m_races_count;
}   // getRacesCountVote

//-----------------------------------------------------------------------------
uint8_t RaceVote::getMinorVote() const
{
    return m_minor_mode;
}   // getMinorVote

//-----------------------------------------------------------------------------
const std::string &RaceVote::getTrackVote(uint8_t track_number) const
{
    return m_tracks_vote[track_number].track_info.track;
}   // getTrackVote

//-----------------------------------------------------------------------------
bool RaceVote::getReversedVote(uint8_t track_number) const
{
    return m_tracks_vote[track_number].track_info.reversed;
}   // getReversedVote

//-----------------------------------------------------------------------------
uint8_t RaceVote::getLapsVote(uint8_t track_number) const
{
    return m_tracks_vote[track_number].track_info.laps;
}   // getLapsVote

//-----------------------------------------------------------------------------
//---------------------------------  RaceConfig -------------------------------
//-----------------------------------------------------------------------------

RaceConfig::RaceConfig()
{
    m_max_players = NetworkConfig::get()->getMaxPlayers();
}   // RaceConfig

//-----------------------------------------------------------------------------
void RaceConfig::setPlayerMajorVote(uint8_t player_id, uint32_t major)
{
    Log::info("RaceConfig", "Player %d voted for major %d", player_id, major);
    m_votes[player_id].voteMajor(major);
}   // setPlayerMajorVote

//-----------------------------------------------------------------------------
void RaceConfig::setPlayerRaceCountVote(uint8_t player_id, uint8_t count)
{
    Log::info("RaceConfig", "Player %d voted for %d races in GP",
              player_id, count);
    m_votes[player_id].voteRaceCount(count);
}   // setPlayerRaceCountVote

//-----------------------------------------------------------------------------
void RaceConfig::setPlayerMinorVote(uint8_t player_id, uint32_t minor)
{
    Log::info("RaceConfig", "Player %d voted for minor %d", player_id, minor);
    m_votes[player_id].voteMinor(minor);
}   // setPlayerMinorVote

//-----------------------------------------------------------------------------
void RaceConfig::setPlayerTrackVote(uint8_t player_id, 
                                    const std::string &track, uint8_t track_nb)
{
    Log::info("RaceConfig", "Player %d voted for track %s",
              player_id, track.c_str());
    m_votes[player_id].voteTrack(track, track_nb);
}   // setPlayerTrackVote

//-----------------------------------------------------------------------------
void RaceConfig::setPlayerReversedVote(uint8_t player_id, bool reversed,
                                       uint8_t track_nb)
{
    if (reversed)
        Log::info("RaceConfig", "Player %d voted map %d to be reversed",
                  player_id, track_nb);
    else
        Log::info("RaceConfig", "Player %d voted map %d NOT to be reversed",
                  player_id, track_nb);
    m_votes[player_id].voteReversed(reversed, track_nb);
}   // setPlayerReversedVote

//-----------------------------------------------------------------------------
void RaceConfig::setPlayerLapsVote(uint8_t player_id, uint8_t lap_count,
                                   uint8_t track_nb)
{
    Log::info("RaceConfig", "Player %d voted map %d to have %d laps",
              player_id, track_nb, lap_count);
    m_votes[player_id].voteLaps(lap_count, track_nb);
}   // setPlayerLapsVote

//-----------------------------------------------------------------------------
/** Computes the selected race mode.
 */
void RaceConfig::computeRaceMode()
{
    // calculate the race type and number of tracks (in GP mode).
    std::map<int,int> major_histogram;
    std::map<int,int> races_count_histogram;
    std::map<int,int> minor_histogram;
    for (unsigned int i = 0; i < m_max_players; i++)
    {
        // increase the count of votes
        if (m_votes[i].hasVotedMajor())
        {
            try
            {
                major_histogram.at(m_votes[i].getMajorVote()) ++;
            }
            catch (const std::out_of_range&) // doesn't exist in the map
            {
                major_histogram[m_votes[i].getMajorVote()] = 1;
            }
        }
        else if (m_votes[i].hasVotedRacesCount())
        {
            try
            {
                races_count_histogram.at(m_votes[i].getRacesCountVote()) ++;
            }
            catch (const std::out_of_range&) // doesn't exist in the map
            {
                races_count_histogram[m_votes[i].getRacesCountVote()] = 1;
            }
        }
        else if (m_votes[i].hasVotedMinor())
        {
            try
            {
                minor_histogram.at(m_votes[i].getMinorVote()) ++;
            }
            catch (const std::out_of_range&) // doesn't exist in the map
            {
                minor_histogram[m_votes[i].getMinorVote()] = 1;
            }
        }
    }
    // now we know :
    m_major_mode  = getHighestInHistogram(&major_histogram,
                                          (int)RaceManager::MAJOR_MODE_SINGLE);
    m_minor_mode  = getHighestInHistogram(&minor_histogram,
                                     (int)RaceManager::MINOR_MODE_NORMAL_RACE);

    if (m_major_mode == RaceManager::MAJOR_MODE_GRAND_PRIX)
    {
        m_races_count = getHighestInHistogram(&races_count_histogram, 1);
        m_tracks.resize(m_races_count);
    }
    else
    {
        m_tracks.resize(1);
        m_races_count = 1;
    }

    Log::info("RaceConfig", "Major mode will be %d with %d races. Minor is %d",
              m_major_mode, m_races_count, m_minor_mode);
}   // computeRaceMode

// ----------------------------------------------------------------------------
void RaceConfig::computeNextTrack()
{
    for (unsigned int j = 0; j < m_races_count; j++)
    {
        // first create histograms of the votes
        std::map<std::string,int> tracks_histogram;
        std::map<bool,int> reversed_histogram;
        std::map<int,int> laps_histogram;
        for (unsigned int i = 0; i < m_max_players; i++)
        {
            // increase the count of votes
            if (m_votes[i].hasVotedTrack())
            {
                try // maps
                {
                    tracks_histogram.at(m_votes[i].getTrackVote()) ++;
                }
                catch (const std::out_of_range&) // doesn't exist in the map
                {
                    tracks_histogram[m_votes[i].getTrackVote()] = 1;
                }
            }
            if (m_votes[i].hasVotedReversed())
            {
                try // reversed
                {
                    reversed_histogram.at(m_votes[i].getReversedVote()) ++;
                }
                catch (const std::out_of_range&) // doesn't exist in the map
                {
                    reversed_histogram[m_votes[i].getReversedVote()] = 1;
                }
            }
            if (m_votes[i].hasVotedLaps())
            {
                try // laps
                {
                    laps_histogram.at(m_votes[i].getLapsVote()) ++;
                }
                catch (const std::out_of_range&) // doesn't exist in the mapt
                {
                    laps_histogram[m_votes[i].getLapsVote()] = 1;
                }
            }
        }
        // now find the highest votes
        m_tracks[j].track = getHighestInHistogram<std::string>(&tracks_histogram,
                                                 UserConfigParams::m_last_track);
        m_tracks[j].reversed = getHighestInHistogram(&reversed_histogram, false);
        m_tracks[j].laps = getHighestInHistogram<int>(&laps_histogram,
                                                   UserConfigParams::m_num_laps); 
        if (m_tracks[j].reversed)
            Log::info("RaceConfig",
                      "Race %d will be on %s with %d laps and reversed",
                       j, m_tracks[j].track.c_str(), m_tracks[j].laps);
        else
            Log::info("RaceConfig", "Race %d will be on %s with %d laps", 
                      j, m_tracks[j].track.c_str(), m_tracks[j].laps);
    }
}   // computeNextTrack

//-----------------------------------------------------------------------------
/** Computes the selected setting (based on the users' vote) and sets them
 *  in the race manager. Then it loads the world.
 */
void RaceConfig::loadWorld()
{
    computeRaceMode();
    computeNextTrack();
    race_manager->startSingleRace(m_tracks[0].track, m_tracks[0].laps,
                                  m_tracks[0].reversed);
}   // loadWorld

//-----------------------------------------------------------------------------
const TrackInfo* RaceConfig::getNextTrackInfo() const
{
    return &m_tracks[0];
}   // getNextTrackInfo

//-----------------------------------------------------------------------------
int RaceConfig::getNumTrackVotes() const
{
    int count = 0;

    for (auto entry : m_votes)
    {
        if (entry.second.hasVotedTrack())
            count++;
    }

    return count;
}   // getNumTrackVotes