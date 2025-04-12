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

#include "network/protocols/ranking.hpp"
#include "io/xml_node.hpp"
#include "utils/log.hpp"
#include "utils/time.hpp"
#include <bitset>
#include <cmath>
#include <vector>

namespace
{
    /* Ranking related variables */
    // If updating the base points, update the base points distribution in DB
    const double BASE_RANKING_POINTS    = 4000.0; // Given to a new player on 1st connection to a ranked server
    const double BASE_RATING_DEVIATION  = 1000.0; // Given to a new player on 1st connection to a ranked server
    const double MIN_RATING_DEVIATION   = 100.0; // A server cron job makes RD go up if a player is inactive
    const double BASE_RD_PER_DISCONNECT = 15.0;
    const double VAR_RD_PER_DISCONNECT  = 3.0;
    const double MAX_SCALING_TIME       = 360.0;
    const double BASE_POINTS_PER_SECOND = 0.25;
    const double HANDICAP_OFFSET        = 2000.0;
}


//-----------------------------------------------------------------------------
RankingEntry::RankingEntry(uint32_t online_id): online_id(online_id)
{
    raw_score = BASE_RANKING_POINTS;
    score     = BASE_RANKING_POINTS - 3*BASE_RATING_DEVIATION + 3*MIN_RATING_DEVIATION;
    max_score = BASE_RANKING_POINTS - 3*BASE_RATING_DEVIATION + 3*MIN_RATING_DEVIATION;
    deviation = BASE_RATING_DEVIATION;
    disconnects = 0;
    races = 0;
}

//-----------------------------------------------------------------------------
/** Compute the new player's rankings used in ranked servers
 */
void Ranking::computeNewRankings(std::vector<RaceResultData>& data, bool time_trial)
{
    // TODO : go over the variables and look
    //        for things that can be simplified away.
    //        e.g. can new/prev be simplified ?

    std::vector<RankingEntry*> ranking_data;
    for (const auto& result: data)
    {
        auto it = m_entries.find(result.online_id);
        if (it == m_entries.end())
        {
            Log::fatal("Ranking", "Failed to obtain the saved ranking for online id %u", result.online_id);
            return;
        }
        auto it2 = m_old_entries.find(result.online_id);
        if (it2 == m_old_entries.end())
            m_old_entries[result.online_id] = it->second.entry;
        ranking_data.push_back(&(it->second.entry));
    }

    std::vector<double> raw_scores_change;
    std::vector<double> new_raw_scores;
    std::vector<double> prev_raw_scores;
    std::vector<double> prev_scores;
    std::vector<double> new_rating_deviations;
    std::vector<double> prev_rating_deviations;
    std::vector<uint64_t> prev_disconnects; //bitflag
    std::vector<int>    disconnects;

    unsigned player_count = data.size();

    // Initialize data vectors
    for (unsigned i = 0; i < player_count; i++)
    {
        double prev_raw_score = ranking_data[i]->raw_score;
        new_raw_scores.push_back(prev_raw_score);
        prev_raw_scores.push_back(prev_raw_score);

        prev_scores.push_back(ranking_data[i]->score);

        double prev_deviation = ranking_data[i]->deviation;
        new_rating_deviations.push_back(prev_deviation);
        prev_rating_deviations.push_back(prev_deviation);

        prev_disconnects.push_back(ranking_data[i]->disconnects);
    }
 
    // Update some variables
    for (unsigned i = 0; i < player_count; i++)
    {
        //First, update the number of ranked races
        ranking_data[i]->races++;

        // Update the number of disconnects
        // We store the last 64 results as bit flags in a 64-bit int.
        // This way, shifting flushes the oldest result.
        ranking_data[i]->disconnects <<= 1;

        if (data[i].is_eliminated)
            ranking_data[i]->disconnects++;

        // std::popcount is C++20 only
        std::bitset<64> b(ranking_data[i]->disconnects);
        disconnects.push_back(b.count());
    }

    // In this loop, considering the race as a set
    // of head to head minimatches, we compute :
    // I - Point changes for each ordered player pair.
    //     In a (p1, p2) pair, only p1's rating is changed.
    //     However, the loop will also go over (p2, p1).
    //     Point changes can be assymetric.
    // II - Rating deviation changes
    for (unsigned i = 0; i < player_count; i++)
    {
        raw_scores_change.push_back(0.0);

        double player1_raw_scores = new_raw_scores[i];
        if (data[i].handicap)
            player1_raw_scores -= HANDICAP_OFFSET;

        // If the player has quitted before the race end,
        // the time value will be incorrect, but it will not be used
        double player1_time  = data[i].time;
        double player1_rd = prev_rating_deviations[i];

        // On a disconnect, increase RD once,
        // no matter how many opponents
        if (data[i].is_eliminated && disconnects[i] >= 3)
            new_rating_deviations[i] =  prev_rating_deviations[i]
                                      + BASE_RD_PER_DISCONNECT
                                      + VAR_RD_PER_DISCONNECT * (disconnects[i] - 3);

        // Loop over all opponents
        for (unsigned j = 0; j < player_count; j++)
        {
            // Don't compare a player with himself
            if (i == j)
                continue;

            // No change between two quitting players
            if (data[i].is_eliminated && data[j].is_eliminated)
                continue;

            double diff, result, expected_result, ranking_importance, max_time;
            diff = result = expected_result = ranking_importance = max_time = 0.0;

            double player2_raw_scores = new_raw_scores[j];
            if (data[j].handicap)
                player2_raw_scores -= HANDICAP_OFFSET;

            double player2_time = data[j].time;
            double player2_rd = prev_rating_deviations[j];

            // Each result can be viewed as new data helping to refine our previous
            // estimates. But first, we need to assess how reliable this new data is
            // compared to existing estimates.

            bool handicap_used = (data[i].handicap || data[j].handicap);
            double accuracy = computeDataAccuracy(player1_rd, player2_rd, player1_raw_scores, player2_raw_scores, player_count, handicap_used);

            // Now that we've computed the reliability value,
            // we can proceed with computing the points gained or lost

            // Compute the result and race ranking importance

            double mode_factor = getModeFactor(time_trial);

            if (data[i].is_eliminated)
            {
                // Recurring disconnects are punished through
                // increased RD and higher RD floor,
                // not through higher raw score loss
                result = 0.0;
                player1_time = player2_time * 1.2; // for getTimeSpread
            }
            else if (data[j].is_eliminated)
            {
                result = 1.0;
                player2_time = player1_time * 1.2;
            }
            else
            {
                result = computeH2HResult(player1_time, player2_time);
            }

            max_time = std::min(MAX_SCALING_TIME, std::max(player1_time, player2_time));

            ranking_importance = accuracy * mode_factor * scalingValueForTime(max_time);

            // Compute the expected result using an ELO-like function
            diff = player2_raw_scores - player1_raw_scores;

            expected_result = 1.0/ (1.0 + std::pow(10.0,
                diff / (  BASE_RANKING_POINTS / 2.0
                        * getModeSpread(time_trial)
                        * getTimeSpread(std::min(player1_time, player2_time)))));

            // Compute the ranking change
            raw_scores_change[i] += ranking_importance * (result - expected_result);

            // We now update the rating deviation. The change
            // depends on the current RD, on the result's accuracy,
            // on how expected the result was (upsets can increase RD)

            // If there was a disconnect in this race, RD was handled once already
            if (!data[i].is_eliminated) {
                // First the RD reduction based on accuracy and current RD
                double rd_change_factor = accuracy * 0.0016;
                double rd_change = (-1) * prev_rating_deviations[i] * rd_change_factor;

                // If the unexpected result happened, we add a RD increase
                // TODO : more reliable would be accumulating an expected_result/result
                // differential over time, weighted through relative RDs.
                // If that differential goes high, then increase RD while decaying
                // the differential. Some work needed to ensure sensible maths.
                double upset = std::abs(result - expected_result);
                if (upset > 0.5)
                {
                    // Renormalize so expected result 50% is 1.0 and expected result 100% is 0.0
                    upset = 2.0 - 2 * upset;
                    upset = std::max(0.02, upset);

                    // If upsets happen at the rate predicted by expected score,
                    // this won't prevent the rating deviation from going down.
                    // However, if upsets are at least twice more frequent than expected, RD will go up.
                    rd_change += MIN_RATING_DEVIATION * rd_change_factor / upset;
                }

                // Minimum RD will be handled after all iterative RD change have been done,
                // so as to avoid the order in which player pairs are computed changing results.
                new_rating_deviations[i] += rd_change;
            }
        }
    }

    // Don't merge it in the main loop as new_scores value are used there
    for (unsigned i = 0; i < player_count; i++)
    {
        new_raw_scores[i] += raw_scores_change[i];
        ranking_data[i]->raw_score = new_raw_scores[i];

        // Ensure RD doesn't go below the RD floor.
        // The minimum RD is increased in case of repeated disconnects
        double disconnects_floor = 0;
        if (disconnects[i] >= 3)
        {
            int n = disconnects[i] - 3;
            disconnects_floor =   (disconnects[i] - 2) * BASE_RD_PER_DISCONNECT
                                + VAR_RD_PER_DISCONNECT * (n * (n + 1)) / 2;
        }
        new_rating_deviations[i] = std::max(new_rating_deviations[i], MIN_RATING_DEVIATION + disconnects_floor);
        ranking_data[i]->deviation = new_rating_deviations[i];

        // Update the main public rating. At min RD, it is equal to the raw score.
        ranking_data[i]->score = ranking_data[i]->raw_score - 3 * new_rating_deviations[i] + 3 * MIN_RATING_DEVIATION;
        if (ranking_data[i]->score > ranking_data[i]->max_score)
            ranking_data[i]->max_score = ranking_data[i]->score;
    }
}   // computeNewRankings


//-----------------------------------------------------------------------------
/** Returns the mode race importance factor,
 *  used to make ranking move slower in more random modes.
 */
double Ranking::getModeFactor(bool time_trial)
{
    if (time_trial)
        return 1.0;
    return 0.75;
}   // getModeFactor

//-----------------------------------------------------------------------------
/** Returns the mode spread factor, used so that a similar difference in
 *  skill will result in a similar ranking difference in more random modes.
 */
double Ranking::getModeSpread(bool time_trial)
{
    if (time_trial)
        return 1.0;

    //TODO: For 2.0, it would be more sensible to simply use
    // a different ranking for time-trial and normal races, as
    // some players perform better in one mode than in the other,
    // so an unified list is always going to be flawed. Trying to
    // patch these flaws is never going to work well.
    return 1.25;
}   // getModeSpread

//-----------------------------------------------------------------------------
/** Returns the time spread factor.
 *  Short races are more random, so the expected result changes depending
 *  on race duration.
 * TODO: collect real-world data on the relationship between race duration and
 * the likelihood of winning of players, and fit the formula based on it.
 */
double Ranking::getTimeSpread(double time)
{
    return sqrt(120.0 / time);
}   // getTimeSpread

//-----------------------------------------------------------------------------
/** Compute the scaling value of a given time
 *  This is linear to race duration, getTimeSpread takes care
 *  of expecting a more random result in shorter races.
 */
double Ranking::scalingValueForTime(double time)
{
    return time * BASE_POINTS_PER_SECOND;
}   // scalingValueForTime

//-----------------------------------------------------------------------------
/** Computes the score of a head-to-head minimatch.
 *  If time difference > 2,5% ; the result is 1 (complete win of player 1)
 *  or 0 (complete loss of player 1)
 *  Otherwise, it is averaged between 0 and 1.
 */
double Ranking::computeH2HResult(double player1_time, double player2_time)
{
    double max_time = std::max(player1_time, player2_time);
    double min_time = std::min(player1_time, player2_time);

    double result = (max_time - min_time) / (min_time / 20.0);
    result = std::min(1.0, 0.5 + result);

    if (player2_time <= player1_time)
        result = 1.0 - result;

    return result;
}   // computeH2HResult

//-----------------------------------------------------------------------------
/** Computes a relative factor indicating how much informative value
 *  the new race result gives us.
 *
 *  For a player with a high own rating deviation, the current rating is unreliable
 *  so any new data holds more importance. This is crucial to allow reasonably
 *  fast rating convergence of new players, provided they play accurately rated opponents.
 *
 *  When the opponent has a high rating deviation, the expected scores are likely off.
 *  Therefore, the information from such a result is much less valuable.
 *
 *  We also reduce rating changes when the player ratings are very different, even
 *  after considering the uncertainties from rating deviation.
 *  This is multi-purpose :
 *   - With a very high rating difference, random race events (very poor luck, disconnects)
 *     are very likely to be the cause of any upset, so the rate of legitimate upsets is
 *     unreliable. No rating method is safe.
 *   - Attempting to "farm" much lower rated players against which a practical 100% winrate
 *     may be reached (outside of random events) becomes very ineffective. Instead,
 *     to gain rating points, the player has incentive to play well-rated opponents.
 *   - The primary goal is to ensure that two players of equal rating would be about
 *     evenly matched in head-to-head. If two strong players each beat a much weaker third
 *     player, very little information is gained on how a direct head-to-head between the
 *     strong players would go.
 *  For the purposes of this rating computation, we assume that the informational value
 *  of a race is roughly proportional to the likelihood of the weaker player winning.
 * 
 *  A downside of this method is that a legitimate loss to a much lower rated player won't
 *  change the rating much, but these are rare enough that it's better to miss them than
 *  to reward "farming" and to incorrectly reward wins due to external factors.
 *
 *  In a race with many players, a single event can have a significant impact on the
 *  results of all the H2H. To avoid races with high players count having too strong
 *  rating swings, we apply a modifier scaling down accuracy.
 *
 *  Finally, while handicap is allowed in ranked races and a rating offset is applied
 *  to keep expected results realistic (without incentivizing playing handicap-only),
 *  the results of such races are much less reliable.
 */
double Ranking::computeDataAccuracy(double player1_rd, double player2_rd, double player1_scores, double player2_scores, int player_count, bool handicap_used)
{
    double accuracy = player1_rd / (sqrt(player2_rd) * sqrt(MIN_RATING_DEVIATION));

    double strong_lowerbound = (player1_scores > player2_scores) ? player1_scores - 350 - (player1_rd/2)
                                                                 : player2_scores - 350 - (player2_rd/2);
    double weak_upperbound   = (player1_scores > player2_scores) ? player2_scores + 350 + (player2_rd/2)
                                                                 : player1_scores + 350 + (player1_rd/2);

    if (weak_upperbound < strong_lowerbound)
    {
        double diff = strong_lowerbound - weak_upperbound;
        diff = diff / (BASE_RANKING_POINTS / 2.0);

        // The expected result is that of the weaker player and is between 0 and 0.5
        double expected_result = 1.0/ (1.0 + std::pow(10.0, diff));
        expected_result = std::max(0.05, 2*expected_result);

        accuracy *= expected_result;
    }

    // Reduce the importance of single h2h in a race with many players.
    // The overall impact of a race with more players is still always bigger.
    double player_count_modifier = 2.0 / sqrt((double) player_count);
    accuracy *= player_count_modifier;

    // Races with handicap are unreliable for ranking
    if (handicap_used)
        accuracy *= 0.25;

    return accuracy;
}

//-----------------------------------------------------------------------------
void Ranking::cleanup()
{
    std::map<uint32_t, RankingEntryAndProfile> new_entries;
    for (const auto& pair: m_entries)
    {
        auto profile = pair.second.profile;
        if (!profile.expired())
            new_entries.insert(pair);
    }
    std::swap(new_entries, m_entries);
}

//-----------------------------------------------------------------------------
void Ranking::fill(uint32_t online_id, const XMLNode* result, std::shared_ptr<NetworkPlayerProfile> npp)
{
    RankingEntry entry(online_id);
    result->get("scores", &entry.score);
    result->get("max-scores", &entry.max_score);
    result->get("num-races-done", &entry.races);
    result->get("raw-scores", &entry.raw_score);
    result->get("rating-deviation", &entry.deviation);
    result->get("disconnects", &entry.disconnects);
    m_entries[online_id] = {entry, npp};
}

//-----------------------------------------------------------------------------
bool Ranking::has(uint32_t online_id)
{
    auto it = m_entries.find(online_id);
    if (it == m_entries.end())
        return false;
    if (it->second.profile.expired())
        return false;
    return true;
}

//-----------------------------------------------------------------------------
double Ranking::getDelta(uint32_t online_id)
{
    auto it = m_old_entries.find(online_id);
    if (it == m_old_entries.end())
        return 0.;
    auto it2 = m_entries.find(online_id);
    if (it2 == m_entries.end())
        return 0.;
    double result = it2->second.entry.score - it->second.score;
    m_old_entries.erase(it);
    return result;
}

//-----------------------------------------------------------------------------
const RankingEntry Ranking::getScores(uint32_t online_id) const
{
    return m_entries.at(online_id).entry;
}

//-----------------------------------------------------------------------------
const RankingEntry Ranking::getTemporaryPenalizedScores(uint32_t online_id) const
{
    RankingEntry entry = m_entries.at(online_id).entry;
    entry.score -= 200.0;
    entry.raw_score -= 200.0;
    entry.races += 1;
    entry.disconnects <<= 1;
    entry.disconnects += 1;
    return entry;
}