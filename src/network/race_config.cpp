#include "network/race_config.hpp"

#include "race/race_manager.hpp"
#include "utils/log.hpp"

#include <stdexcept>

/** \brief Gets the element with the highest count in a std::map<S,int>.
 *  \param histogram : A pointer to the histogram.
 *  \return The key of type S that has the highest second value.
 */
template<typename S>
S getHighestInHistogram(std::map<S,int>* histogram)
{
    S best_item;
    uint8_t highest_count = histogram->begin()->second;
    for (typename std::map<S, int>::iterator it = histogram->begin();
        it != histogram->end(); ++it)
    {
        if (it->second > highest_count)
        {
            highest_count = it->second;
            best_item = it->first;
        }
    }
    return best_item;
}

//-----------------------------------------------------------------------------
//---------------------------------  TrackVote --------------------------------
//-----------------------------------------------------------------------------

TrackVote::TrackVote()
{
    has_voted_laps = false;
    has_voted_track = false;
    has_voted_reversed = false;
}
//-----------------------------------------------------------------------------
void TrackVote::voteTrack(std::string track)
{
    track_info.track = track;
    has_voted_track = true;
}
//-----------------------------------------------------------------------------
void TrackVote::voteReversed(bool reversed)
{
    track_info.reversed = reversed;
    has_voted_reversed = true;
}
//-----------------------------------------------------------------------------
void TrackVote::voteLaps(uint8_t laps)
{
    track_info.laps = laps;
    has_voted_laps = true;
}

//-----------------------------------------------------------------------------
//---------------------------------  RaceVote ---------------------------------
//-----------------------------------------------------------------------------

RaceVote::RaceVote()
{
    m_has_voted_major = false;
    m_has_voted_minor = false;
    m_has_voted_races_count = false;
    m_major_mode = 0;
    m_minor_mode = 0;
    m_races_count = 0;
}

//-----------------------------------------------------------------------------

void RaceVote::voteMajor(uint8_t major)
{
    m_has_voted_major = true;
    m_major_mode = major;
}

//-----------------------------------------------------------------------------

void RaceVote::voteRaceCount(uint8_t count)
{
    m_has_voted_races_count = true;
    m_races_count = count;
}

//-----------------------------------------------------------------------------

void RaceVote::voteMinor(uint8_t minor)
{
    m_has_voted_minor = true;
    m_minor_mode = minor;
}

//-----------------------------------------------------------------------------

void RaceVote::voteTrack(std::string track, uint8_t track_number)
{
    m_tracks_vote[track_number].voteTrack(track);
}

//-----------------------------------------------------------------------------

void RaceVote::voteReversed(bool reversed, uint8_t track_number)
{
    m_tracks_vote[track_number].voteReversed(reversed);
}

//-----------------------------------------------------------------------------

void RaceVote::voteLaps(uint8_t laps, uint8_t track_number)
{
    m_tracks_vote[track_number].voteLaps(laps);
}

//-----------------------------------------------------------------------------
bool RaceVote::hasVotedMajor() const
{
    return m_has_voted_major;
}
//-----------------------------------------------------------------------------
bool RaceVote::hasVotedRacesCount() const
{
    return m_has_voted_races_count;
}
//-----------------------------------------------------------------------------
bool RaceVote::hasVotedMinor() const
{
    return m_has_voted_minor;
}
//-----------------------------------------------------------------------------
bool RaceVote::hasVotedTrack(uint8_t track_number) const
{
    return m_tracks_vote[track_number].has_voted_track;
}
//-----------------------------------------------------------------------------
bool RaceVote::hasVotedReversed(uint8_t track_number) const
{
    return m_tracks_vote[track_number].has_voted_reversed;
}
//-----------------------------------------------------------------------------
bool RaceVote::hasVotedLaps(uint8_t track_number) const
{
    return m_tracks_vote[track_number].has_voted_laps;
}

//-----------------------------------------------------------------------------

uint8_t RaceVote::getMajorVote() const
{
    return m_major_mode;
}
//-----------------------------------------------------------------------------
uint8_t RaceVote::getRacesCountVote() const
{
    return m_races_count;
}
//-----------------------------------------------------------------------------
uint8_t RaceVote::getMinorVote() const
{
    return m_minor_mode;
}
//-----------------------------------------------------------------------------
std::string RaceVote::getTrackVote(uint8_t track_number) const
{
    return m_tracks_vote[track_number].track_info.track;
}
//-----------------------------------------------------------------------------
bool RaceVote::getReversedVote(uint8_t track_number) const
{
    return m_tracks_vote[track_number].track_info.reversed;
}
//-----------------------------------------------------------------------------
uint8_t RaceVote::getLapsVote(uint8_t track_number) const
{
    return m_tracks_vote[track_number].track_info.laps;
}

//-----------------------------------------------------------------------------
//---------------------------------  RaceConfig -------------------------------
//-----------------------------------------------------------------------------

RaceConfig::RaceConfig()
{
    m_max_players = 0;
}

//-----------------------------------------------------------------------------

void RaceConfig::setPlayerCount(uint8_t count)
{
    m_max_players = count;
    m_votes.resize(m_max_players);
}

//-----------------------------------------------------------------------------

void RaceConfig::setPlayerMajorVote(uint8_t player_id, uint8_t major)
{
    Log::info("RaceConfig", "Player %d voted for major %d", player_id, major);
    m_votes[player_id].voteMajor(major);
}

//-----------------------------------------------------------------------------

void RaceConfig::setPlayerRaceCountVote(uint8_t player_id, uint8_t count)
{
    Log::info("RaceConfig", "Player %d voted for %d races in GP", player_id, count);
    m_votes[player_id].voteRaceCount(count);
}

//-----------------------------------------------------------------------------

void RaceConfig::setPlayerMinorVote(uint8_t player_id, uint8_t minor)
{
    Log::info("RaceConfig", "Player %d voted for minor %d", player_id, minor);
    m_votes[player_id].voteMinor(minor);
}

//-----------------------------------------------------------------------------

void RaceConfig::setPlayerTrackVote(uint8_t player_id, std::string track, uint8_t track_nb)
{
    Log::info("RaceConfig", "Player %d voted for track %s", player_id, track.c_str());
    m_votes[player_id].voteTrack(track, track_nb);
}

//-----------------------------------------------------------------------------

void RaceConfig::setPlayerReversedVote(uint8_t player_id, bool reversed, uint8_t track_nb)
{
    if (reversed)
        Log::info("RaceConfig", "Player %d voted map %d to be reversed", player_id, track_nb);
    else
        Log::info("RaceConfig", "Player %d voted map %d NOT to be reversed", player_id, track_nb);
    m_votes[player_id].voteReversed(reversed, track_nb);
}

//-----------------------------------------------------------------------------

void RaceConfig::setPlayerLapsVote(uint8_t player_id, uint8_t lap_count, uint8_t track_nb)
{
    Log::info("RaceConfig", "Player %d voted map %d to have %d laps", player_id, track_nb, lap_count);
    m_votes[player_id].voteLaps(lap_count, track_nb);
}

//-----------------------------------------------------------------------------

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
    m_major_mode = (!major_histogram.empty() ? getHighestInHistogram<int>(&major_histogram) : 1);
    m_races_count = (!minor_histogram.empty() ? getHighestInHistogram<int>(&races_count_histogram) : 1);
    m_minor_mode = (!minor_histogram.empty() ? getHighestInHistogram<int>(&minor_histogram) : 0);

    if (m_major_mode == RaceManager::MAJOR_MODE_GRAND_PRIX)
        m_tracks.resize(m_races_count);
    else
    {
        m_tracks.resize(1);
        m_races_count = 1;
    }

    Log::info("RaceConfig", "Major mode will be %d with %d races. Minor is %d", m_major_mode, m_races_count, m_minor_mode);
}
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
            else if (m_votes[i].hasVotedReversed())
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
            else if (m_votes[i].hasVotedLaps())
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
        m_tracks[j].track = getHighestInHistogram<std::string>(&tracks_histogram);
        m_tracks[j].reversed = getHighestInHistogram<bool>(&reversed_histogram);
        m_tracks[j].laps = getHighestInHistogram<int>(&laps_histogram);
        if (m_tracks[j].reversed)
            Log::info("RaceConfig", "Race %d will be on %s with %d laps and reversed", j, m_tracks[j].track.c_str(), m_tracks[j].laps);
        else
            Log::info("RaceConfig", "Race %d will be on %s with %d laps", j, m_tracks[j].track.c_str(), m_tracks[j].laps);
    }
}

//-----------------------------------------------------------------------------

const TrackInfo* RaceConfig::getNextTrackInfo() const
{
    return &m_tracks[0];
}

//-----------------------------------------------------------------------------
