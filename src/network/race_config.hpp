#ifndef RACE_CONFIG_HPP
#define RACE_CONFIG_HPP

#include <string>
#include <vector>

class TrackInfo
{
    public:
    std::string track;
    bool reversed;
    int laps;
};
class TrackVote
{
    public:
    TrackVote();

    void voteTrack(std::string track);
    void voteReversed(bool reversed);
    void voteLaps(int laps);

    TrackInfo track_info;

    bool has_voted_track;
    bool has_voted_reversed;
    bool has_voted_laps;

    int minor_race_type; // corresponds to the enum in race_manager.hpp
};

class RaceConfig
{
    public:
    RaceConfig();

    void setPlayerCount(int count);
    void setPlayerTrackVote(int player_id, std::string track_name);
    void setPlayerReverseVote(int player_id, bool reversed);
    void setPlayerLapVote(int player_id, int lap_count);

    void computeNextTrack();

    const TrackInfo* getNextTrackInfo() const;
    bool getReverse() const;
    bool getLapCount() const;

    protected:
    TrackInfo m_track;
    bool m_reverse;
    int m_laps;

    std::vector<TrackVote> m_votes;
    int m_max_players;
};


#endif // RACE_CONFIG_HPP
