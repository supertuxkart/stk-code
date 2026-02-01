//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2024 SuperTuxKart-Team
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

#ifndef RANKING_HPP
#define RANKING_HPP

#include <map>
#include <memory>
#include <vector>
#include <stdint.h>

class XMLNode;
class NetworkPlayerProfile;

struct RankingEntry
{
    uint32_t online_id;
    double raw_score;
    double score;
    double max_score;
    double deviation;
    uint64_t disconnects;
    unsigned races;

    RankingEntry(uint32_t online_id = -1);
};

struct RaceResultData {
    uint32_t online_id;
    bool is_eliminated;
    double time;
    bool handicap;
};

struct RankingEntryAndProfile
{
    RankingEntry entry;
    std::weak_ptr<NetworkPlayerProfile> profile;
};

class Ranking
{
private:
    // A map that stores the actual scores.
    std::map<uint32_t, RankingEntryAndProfile> m_entries;

    // A map that stores the score before the last series of computeNewRankings calls has started.
    std::map<uint32_t, RankingEntry> m_old_entries;

    static double getModeFactor(bool time_trial);
    static double getModeSpread(bool time_trial);
    static double getTimeSpread(double time);
    static double scalingValueForTime(double time);
    static double computeH2HResult(double player1_time, double player2_time);
    static double computeDataAccuracy(double player1_rd, double player2_rd,
                               double player1_scores, double player2_scores,
                               int player_count, bool handicap_used);
public:
    void computeNewRankings(std::vector<RaceResultData>& data, bool time_trial);
    void cleanup();
    void fill(uint32_t online_id, const XMLNode* result, std::shared_ptr<NetworkPlayerProfile> npp);
    bool has(uint32_t online_id);
    double getDelta(uint32_t online_id);
    const RankingEntry getScores(uint32_t online_id) const;
    const RankingEntry getTemporaryPenalizedScores(uint32_t online_id) const;
};

#endif