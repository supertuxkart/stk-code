#include "network/race_config.hpp"

#include "race/race_manager.hpp"

template<typename S>
S getHighestInHistogram(std::map<S,int>* histogram)
{
    S best_item;
    int highest_count;
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
}

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
void TrackVote::voteLaps(int laps)
{
    track_info.laps = laps;
    has_voted_laps = true;
}

//-----------------------------------------------------------------------------

RaceConfig::RaceConfig()
{
    m_max_players = 0;
}

//-----------------------------------------------------------------------------

void RaceConfig::setPlayerCount(int count)
{
    m_max_players = count;
    m_votes.resize(m_max_players);
}

//-----------------------------------------------------------------------------

void RaceConfig::setPlayerTrackVote(int player_id, std::string track_name)
{
    m_votes[player_id].voteTrack(track_name);
}

//-----------------------------------------------------------------------------

void RaceConfig::setPlayerReverseVote(int player_id, bool reversed)
{
    m_votes[player_id].voteReversed(reversed);
}

//-----------------------------------------------------------------------------

void RaceConfig::setPlayerLapVote(int player_id, int lap_count)
{
    m_votes[player_id].voteLaps(lap_count);
}

//-----------------------------------------------------------------------------

void RaceConfig::computeNextTrack()
{
    // first create histograms of the votes
    std::map<std::string,int> tracks_histogram;
    std::map<bool,int> reversed_histogram;
    std::map<int,int> laps_histogram;
    for (unsigned int i = 0; i < m_max_players; i++)
    {
        // increase the count of votes
        if (m_votes[i].has_voted_track)
        {
            try // maps
            {
                tracks_histogram.at(m_votes[i].track_info.track) ++;
            }
            catch (const std::out_of_range& oor) // doesn't exist in the map : add it
            {
                tracks_histogram[m_votes[i].track_info.track] = 1;
            }
        }
        else if (m_votes[i].has_voted_reversed)
        {
            try // reversed
            {
                reversed_histogram.at(m_votes[i].track_info.reversed) ++;
            }
            catch (const std::out_of_range& oor) // doesn't exist in the map : add it
            {
                reversed_histogram[m_votes[i].track_info.reversed] = 1;
            }
        }
        else if (m_votes[i].has_voted_laps)
        {
            try // laps
            {
                laps_histogram.at(m_votes[i].track_info.laps) ++;
            }
            catch (const std::out_of_range& oor) // doesn't exist in the map : add it
            {
                laps_histogram[m_votes[i].track_info.laps] = 1;
            }
        }
    }
    // now find the highest votes
    m_track.track = getHighestInHistogram<std::string>(&tracks_histogram);
    m_track.reversed = getHighestInHistogram<bool>(&reversed_histogram);
    m_track.laps = getHighestInHistogram<int>(&laps_histogram);
}

//-----------------------------------------------------------------------------

const TrackInfo* RaceConfig::getNextTrackInfo() const
{
    return &m_track;
}

//-----------------------------------------------------------------------------
