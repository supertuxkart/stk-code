//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2013-2015 Joerg Henrichs
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


#ifndef HEADER_SOCCER_RANKING_HPP
#define HEADER_SOCCER_RANKING_HPP

#include <limits>
#include <string>
#include <vector>

class SoccerRanking
{
public:
    explicit SoccerRanking() = delete;
    explicit SoccerRanking(SoccerRanking& r) = delete;
    explicit SoccerRanking(SoccerRanking&& r) = delete;

    struct RankingEntry
    {
        unsigned int    m_rank;
        float           m_played_games,
                        m_avg_team_size,
                        m_goals_per_game,
                        m_win_rate;
        int             m_elo;

        std::string     m_name;
    };
private:

    static RankingEntry parseLine(
            const std::string& line
            );
    static void parseLineTo(
            RankingEntry& out,
            const std::string& line
            );

public:

    static void readRankings(
            std::vector<RankingEntry>& out,
            std::size_t max = std::numeric_limits<std::size_t>::max(),
            std::size_t offset = 0);
    static RankingEntry getRankOf(
            const std::string& playername);
}; // SoccerRanking

#endif//HEADER_SOCCER_RANKING_HPP

