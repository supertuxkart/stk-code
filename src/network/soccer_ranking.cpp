#include "network/soccer_ranking.hpp"
#include "network/server_config.hpp"
#include "utils/log.hpp"
#include "utils/string_utils.hpp"
#include <fstream>
#include <sstream>

void SoccerRanking::parseLineTo(
        SoccerRanking::RankingEntry& out,
        const std::string& line)
{
    std::stringstream ss;
    ss << line << std::endl;
    ss.exceptions(
            std::stringstream::failbit |
            std::stringstream::badbit |
            std::stringstream::eofbit);
    try
    {
        ss >> out.m_name;
        ss >> out.m_played_games;
        ss >> out.m_avg_team_size;
        ss >> out.m_goals_per_game;
        ss >> out.m_win_rate;
        ss >> out.m_elo;
    }
    catch (const std::exception& e)
    {
        Log::error("SoccerRanking",
                "cannot parse line: \"%s\" (%s)",
                line.c_str(), e.what());
    }
} // parseLineTo
//--------------------------------------------------------------------
SoccerRanking::RankingEntry SoccerRanking::parseLine(
        const std::string& line)
{
    RankingEntry re;
    re.m_rank = 0;
    SoccerRanking::parseLineTo(re, line);
    return re;
} // parseLine

//--------------------------------------------------------------------
void SoccerRanking::readRankings(
        std::vector<RankingEntry>& out,
        const std::size_t max,
        std::size_t offset
        )
{
    const std::string path = ServerConfig::m_soccer_ranking_file;
    if (path.empty())
        return;
    try 
    {
        // REPLACE ME WHEN PROPER DATABASE INTERFACE IS IMPLEMENTED
        // open a file (closes it automatically)
        std::ifstream f(path, std::ios_base::in);
        f.exceptions(
                std::ifstream::failbit |
                std::ifstream::badbit);
        char linebuf[256];
        RankingEntry re;
        re.m_rank = 1;
    
        if (offset)
            for (std::size_t i = 0; !f.eof() && i < offset; ++i, ++re.m_rank)
                f.getline(linebuf, 256);

        if (f.eof() || f.bad() || f.fail())
            return;
        for (std::size_t i = 0; i < max; ++i, ++re.m_rank)
        {
            f.getline(linebuf, 256);

            if (f.eof() || f.bad() || f.fail())
                break;

            parseLineTo(re, linebuf);
            out.push_back(re);
        }
    }
    catch (const std::exception& e)
    {
        Log::verbose("SoccerRanking", "Failed to read ranking data: %s",
                e.what());
        return;
    }
} // readRankings
//--------------------------------------------------------------------
SoccerRanking::RankingEntry SoccerRanking::getRankOf(
        const std::string &playername)
{
    const std::string path = ServerConfig::m_soccer_ranking_file;
    RankingEntry re = {};
    re.m_rank = 1;
    std::string lower_pn = StringUtils::toLowerCase(playername);

    if (path.empty())
        return re;

    try
    {
        std::ifstream f(path, std::ios_base::in);
        f.exceptions(
                std::ifstream::failbit |
                std::ifstream::badbit);
        char linebuf[256];
        for (; !f.eof() && !f.fail() && !f.bad(); ++re.m_rank)
        {
            f.getline(linebuf, 256);
            if (linebuf[0] == 0)
                return re;
            parseLineTo(re, linebuf);

            if (StringUtils::startsWith(
                        StringUtils::toLowerCase(re.m_name),
                        lower_pn))
                return re;
        }
    }
    catch (const std::exception& e)
    {
        Log::verbose("SoccerRanking", "Failed to read ranking data: %s",
                e.what());
    }

    RankingEntry res;
    res.m_rank = 0;
    return res;
} // getRankOf
