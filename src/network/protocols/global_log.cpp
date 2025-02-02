#include "global_log.hpp"
#include <network/server_config.hpp>
#include <network/network_player_profile.hpp>
#include <fstream>
#include <iostream>
#include <utils/log.hpp>
#include <utils/string_utils.hpp>

std::ofstream GlobalLog::outfile_posLog;  
std::ofstream GlobalLog::outfile_goalLog;
std::map<unsigned int, std::string> GlobalLog::ingame_players;

void GlobalLog::openLog(GlobalLogTypes log_name)
{
    if (log_name == GlobalLogTypes::POS_LOG)
    {
        if (GlobalLog::outfile_posLog.is_open()) return;
        else GlobalLog::outfile_posLog.open(ServerConfig::m_soccer_log_path,std::ios_base::app);
        //Log::info("!!!!","openLog succeded");
    }
    //else if (log_name == GlobalLogTypes::GOAL_LOG)
    //{
    //    //Log::info("!!!!","openLog called");
    //    if (GlobalLog::outfile_goalLog.is_open()) return;
    //    else GlobalLog::outfile_goalLog.open(ServerConfig::m_logfile_name,std::ios_base::app);
    //    //Log::info("!!!!","openLog succeded");
    //}
}

void GlobalLog::writeLog(std::string text, GlobalLogTypes log_name)
{
    openLog(log_name);
    if (log_name == GlobalLogTypes::POS_LOG)
    {
        GlobalLog::outfile_posLog << text;
        GlobalLog::outfile_posLog.flush();
    }
    //else if (log_name == GlobalLogTypes::GOAL_LOG)
    //{
    //    GlobalLog::outfile_goalLog << text;
    //    GlobalLog::outfile_goalLog.flush();
    //    if (!text.empty()) text.pop_back();
    //    Log::info("GoalLog",text.c_str());
    //}
}

void GlobalLog::closeLog(GlobalLogTypes log_name)
{
    //std::string msg = "closeLog called " + (log_name == GlobalLogTypes::POS_LOG) ? "posLog" : "goalLog";
    //Log::info("!!!!", msg.c_str());
    if (log_name == GlobalLogTypes::POS_LOG)
    {
        if (!GlobalLog::outfile_posLog.is_open()) return;
        else GlobalLog::outfile_posLog.close();
    }
    //else if (log_name == GlobalLogTypes::GOAL_LOG)
    //{
    //    if (!GlobalLog::outfile_goalLog.is_open()) return;
    //    else GlobalLog::outfile_goalLog.close();
    //}
}

void GlobalLog::addIngamePlayer(unsigned int world_kart_id, std::string player_name, bool offline_account)
{
    std::string prefix = offline_account ? "?" : "";
    std::string escape_char = StringUtils::wideToUtf8(L"\u03df");
    player_name = prefix + StringUtils::replace(player_name, " ", escape_char);
    GlobalLog::ingame_players[world_kart_id] = player_name;
}

void GlobalLog::removeIngamePlayer(unsigned int world_kart_id)
{
    GlobalLog::ingame_players.erase(world_kart_id);
}

std::string GlobalLog::getPlayerName(unsigned int world_kart_id)
{
    return GlobalLog::ingame_players[world_kart_id];
}

void GlobalLog::resetIngamePlayers()
{
    GlobalLog::ingame_players.clear();
}
