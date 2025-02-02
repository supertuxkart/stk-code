//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2021 SuperTuxKart-Team
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

#include "network/tournament/tournament_manager.hpp"
#include "network/protocols/global_log.hpp"
#include "network/peer_vote.hpp"
#include "network/protocols/lobby_protocol.hpp"
#include "network/protocols/server_lobby.hpp"
#include "network/stk_peer.hpp"
#include "utils/string_utils.hpp"
#include <algorithm>
#include <csignal>
#include <cstdio>
#include <ctime>
#include <iostream>
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <utility>
#include "network/server_config.hpp"
#include "utils/log.hpp"
#include "utils/string_utils.hpp"

TournamentManager* TournamentManager::g_tournament_manager = nullptr;
const char* TournamentManager::g_matchplan_blank_word = "?";

void TournamentManager::FilterScorerData(std::vector<SoccerWorld::ScorerData>& scorers)
{
    for (int i = scorers.size() - 1; i >= 0; i--)
    {
        std::string player_name = StringUtils::wideToUtf8(scorers[i].m_player);
        if (StringUtils::hasSuffix(player_name, " (not counted)"))
            scorers.erase(scorers.begin() + i);
    }

    for (auto& scorer : scorers)
        scorer.m_time += m_elapsed_time;
}

void TournamentManager::GetAdditionalTime(int& minutes, int& seconds) const
{
    int total_seconds = (int)GetAdditionalSeconds();
    minutes = total_seconds / 60;
    seconds = total_seconds - 60 * minutes;
}

void TournamentManager::OnGameEnded()
{
    if (GameInitialized())
    {
        std::string log = "Match: " + m_red_team + " vs " + m_blue_team + "\n";
        log += "Game: " + std::to_string(m_current_game_index) + "\n";
        log += "Soccer Field: " + GetPlayedField() + "\n";
        log += "Result: " + std::to_string(m_current_game_result.m_red_goals) + ":" + std::to_string(m_current_game_result.m_blue_goals) + "\n";
        for (auto& scorer : m_current_game_result.m_red_scorers)
            log += "Goal " + m_red_team + " " + StringUtils::wideToUtf8(scorer.m_player) + " " + std::to_string(scorer.m_time) + "\n";
        for (auto& scorer : m_current_game_result.m_blue_scorers)
            log += "Goal " + m_blue_team + " " + StringUtils::wideToUtf8(scorer.m_player) + " " + std::to_string(scorer.m_time) + "\n";

        std::ofstream logfile;
        logfile.open(ServerConfig::m_tourn_log, std::ios_base::app);
        if (logfile.is_open())
        {
            logfile << log;
            logfile.close();
        }
        Log::info("TournamentManager", log.c_str());

#if 0
        std::string cmd = "python3 update_matchplan.py " + m_red_team + " " + m_blue_team + " " + std::to_string(m_current_game_index) + " " + std::to_string(m_current_game_result.m_red_goals) + "-" + std::to_string(m_current_game_result.m_blue_goals) + " " + m_current_game_result.m_played_field + " " + m_referee + " " + m_video;
        system(cmd.c_str());
#endif
        if (ServerConfig::m_update_matchplan)
        {
            LoadMatchPlan(false);
            UpdateMatchPlan(
                    m_red_team, m_blue_team, m_current_game_index,
                    m_current_game_result.m_red_goals, m_current_game_result.m_blue_goals,
                    m_current_game_result.m_played_field, m_referee, m_video);
            SaveMatchPlan();
        }
        m_current_game_result.m_elapsed_time = m_elapsed_time;
        m_game_results[m_current_game_index] = m_current_game_result;
        m_current_game_result.m_blue_goals = 0;
        m_current_game_result.m_red_goals = 0;
        m_current_game_index = -1;
    }
}

TournamentManager::TournamentManager()
{
}

TournamentManager::~TournamentManager()
{
}
void TournamentManager::create()
{
    if (g_tournament_manager)
        return;
    g_tournament_manager = new TournamentManager();
}
void TournamentManager::destroy()
{
    if (!g_tournament_manager) return;
    delete g_tournament_manager;
    g_tournament_manager = nullptr;
}
void TournamentManager::clear()
{
    if (!g_tournament_manager)
        return;
    g_tournament_manager->m_player_teams.clear();
    g_tournament_manager->m_match_plan.clear();
    g_tournament_manager->m_red_team.clear();
    g_tournament_manager->m_blue_team.clear();
    g_tournament_manager->m_red_players.clear();
    g_tournament_manager->m_player_karts.clear();
    g_tournament_manager->m_referee.clear();
    g_tournament_manager->m_video.clear();
    g_tournament_manager->m_game_results.clear();
    g_tournament_manager->m_current_game_index = -1;
    g_tournament_manager->m_current_game_result = {};
    g_tournament_manager->m_target_time = .0f;
    g_tournament_manager->m_elapsed_time = .0f;
    g_tournament_manager->m_stopped_at = .0f;
}

// Format of config_string = "player1 A player2 A player3 B player4 B player5 Sub"
void TournamentManager::InitializePlayersAndTeams(std::string red_team, std::string blue_team)
{
#if 0
    std::vector<std::string> player_teams = StringUtils::split(config_string, ' ');
    if (player_teams.size() % 2 != 0)
    {
        Log::error("TournamentManager", "Invalid config string");
        return;
    }
        
    for (size_t i = 0; i < player_teams.size(); i += 2)
        m_player_teams[player_teams[i]] = player_teams[i + 1];

#endif
    try
    {
        std::ifstream teams_file(ServerConfig::m_teams_path, std::ios_base::in);
        teams_file.exceptions(std::ifstream::badbit);
        if (!teams_file.is_open())
        {
            Log::error("TournamentManager", "File %s does not exist.", ServerConfig::m_teams_path.c_str());
            return;
        }
        
        // each line is a team configuration, first word is the team name
        char c_line[2048];
        std::string line;
        for (;!teams_file.eof() && !teams_file.fail() && !teams_file.bad();
                teams_file.getline(c_line, 2048), line = c_line)
        {
            std::vector<std::string> contents = StringUtils::split(line, ' ');

            // team name is contents[0], iterate over the player names
            for (unsigned int i = 1; i < contents.size(); i++)
            {
                m_player_teams[contents[i]] = contents[0];
            }
        }

        // NOTE: performance bottleneck, this can be done in just one go
        UpdateTeams(red_team, blue_team);
    }
    catch (const std::ifstream::failure& exception)
    {
        Log::fatal("TournamentManager", "Unable to read teams from file %s: %s",
                ServerConfig::m_teams_path.c_str(), exception.what());
        return;
    }
}

void TournamentManager::UpdateTeams(std::string red_team, std::string blue_team)
{
    m_red_team = red_team;
    m_blue_team = blue_team;

    m_red_players.clear();
    m_blue_players.clear();
    for (auto& pt : m_player_teams)
    {
        if (pt.second == red_team)
            m_red_players.insert(pt.first);
        if (pt.second == blue_team)
            m_blue_players.insert(pt.first);
    }
}

std::string TournamentManager::GetTeam(std::string player_name)
{
    return (m_player_teams.find(player_name) == m_player_teams.end()) ? "" 
        : m_player_teams[player_name];
}

KartTeam TournamentManager::GetKartTeam(std::string player_name) const
{
    bool blue = m_blue_players.find(player_name) != m_blue_players.end();
    bool red = m_red_players.find(player_name) != m_red_players.end();

    return red ? KART_TEAM_RED : (blue ? KART_TEAM_BLUE : KART_TEAM_NONE);
}

void TournamentManager::SetKartTeam(std::string player_name, KartTeam team)
{
    switch (team)
    {
    case KART_TEAM_NONE:
        m_red_players.erase(player_name);
        m_blue_players.erase(player_name);
        return;
    case KART_TEAM_RED:
        m_red_players.insert(player_name);
        m_blue_players.erase(player_name);
        return;
    case KART_TEAM_BLUE:
        m_blue_players.insert(player_name);
        m_red_players.erase(player_name);
        return;
    }
}

std::string TournamentManager::GetKart(std::string player_name) const
{
    return m_player_karts.count(player_name) ? m_player_karts.at(player_name) : "";
}

void TournamentManager::SetKart(std::string player_name, std::string kart_name)
{
    if (kart_name == "")
        m_player_karts.erase(player_name);
    else
        m_player_karts[player_name] = kart_name;
}

std::set<std::string> TournamentManager::GetKartRestrictedUsers() const
{
    std::set<std::string> player_names;
    for (auto& keyValue : m_player_karts)
        player_names.insert(keyValue.first);
    return player_names;
}

bool TournamentManager::CanPlay(std::string player_name) const
{
    return GameInitialized() && (GetKartTeam(player_name) != KART_TEAM_NONE);
}

bool TournamentManager::CountPlayerVote(std::string player_name) const
{
#if 0
    if (m_current_game_index == 2)
        return GetKartTeam(player_name) == KART_TEAM_RED;
    else if (m_current_game_index == 3)
        return GetKartTeam(player_name) == KART_TEAM_BLUE;
    return true;
#endif
    return IsGameVotable(GetKartTeam(player_name));
}
bool TournamentManager::CountPlayerVote(STKPeer* peer) const
{
    if (!peer->hasPlayerProfiles())
        return false;

    for (auto& profile : peer->getPlayerProfiles())
        if (IsGameVotable(profile->getTeam()))
            return true;
    return false;
}

void TournamentManager::StartNextGame(const bool announce)
{
    if (m_current_game_index < 1)
        m_current_game_index = 1;
    else if (m_current_game_index > m_game_setup.size())
    {
        m_current_game_index = -1;
        return;
    }

    while (GameDone(m_current_game_index))
    {
        if (m_current_game_index > m_game_setup.size())
        {
            m_current_game_index = -1;
            return;
        }
        m_current_game_index++;
    }
    if (m_current_game_index > m_game_setup.size())
    {
        m_current_game_index = -1;
        return;
    }
    StartGame(m_current_game_index, announce);
}

void TournamentManager::StartGame(int index, float target_time, bool announce)
{
    assert(index < m_game_setup.size());
    m_current_game_index = index;
    if (m_current_game_result.m_blue_goals != 0 || m_current_game_result.m_red_goals != 0)
    {
        m_game_results[index].m_blue_goals = m_current_game_result.m_blue_goals;
        m_game_results[index].m_red_goals = m_current_game_result.m_red_goals;
    }
    m_current_game_result = m_game_results[index];
    m_target_time = target_time;
    m_elapsed_time = 0;
    m_stopped_at = 0;
    //m_player_karts.clear();

    if (index >= 1 && index <= TournamentManager::get()->GetMaxGames())
    {
        const int minutes = GetAdditionalMinutesRounded();
        Log::info("TournamentManager", "game %d %d", index, minutes);
        if (!announce)
            return;
        auto sl = LobbyProtocol::get<ServerLobby>();
        if (!sl)
            return;

        std::string msg = "Ready to start game " + std::to_string(index) + " for " + std::to_string(minutes) + " minutes!";
        sl->sendStringToAllPeers(msg);
        GlobalLog::writeLog(msg + "\n", GlobalLogTypes::GOAL_LOG);
    }
}

void TournamentManager::StartGame(int index, bool announce)
{
    if (index > m_game_setup.size() || index < 1)
        return;

    StartGame(index, m_game_setup[index - 1].m_game_minutes * 60.0f, announce);
}

void TournamentManager::StopGame(float elapsed_time, bool announce)
{
    m_stopped_at += elapsed_time;
    Log::info("TournamentManager", "stop");
    if (!announce)
        return;
    auto sl = LobbyProtocol::get<ServerLobby>();
    if (!sl)
        return;

    std::string msg = "The game is stopped.";
    sl->sendStringToAllPeers(msg);
}

void TournamentManager::ResumeGame(float elapsed_time, bool announce)
{
    if (m_stopped_at != 0)
    {
        m_target_time += elapsed_time - m_stopped_at;
        m_stopped_at = 0;
    }
    Log::info("TournamentManager", "resume");
    if (!announce)
        return;
    auto sl = LobbyProtocol::get<ServerLobby>();
    if (!sl)
        return;

    std::string msg = "The game is resumed.";
    sl->sendStringToAllPeers(msg);
}

void TournamentManager::HandleGameResult(float elapsed_time, GameResult result)
{
    FilterScorerData(result.m_red_scorers);
    FilterScorerData(result.m_blue_scorers);

    m_current_game_result.m_red_goals += result.m_red_scorers.size();
    m_current_game_result.m_blue_goals += result.m_blue_scorers.size();

    m_current_game_result.m_red_scorers.insert(m_current_game_result.m_red_scorers.end(), result.m_red_scorers.begin(), result.m_red_scorers.end());
    m_current_game_result.m_blue_scorers.insert(m_current_game_result.m_blue_scorers.end(), result.m_blue_scorers.begin(), result.m_blue_scorers.end());

    if (m_stopped_at != 0)
    {
        m_elapsed_time += m_stopped_at;
        m_stopped_at = 0;
    }
    else
    {
        m_elapsed_time += elapsed_time;
    }

    if (!GameOpen())
        OnGameEnded();
}

void TournamentManager::ForceEndGame()
{
    m_target_time = m_elapsed_time;
    OnGameEnded();
}

void TournamentManager::ResetGame(int index)
{
    m_current_game_index = -1;
    m_current_game_result = GameResult();
    m_target_time = 0;
    m_stopped_at = 0;
    m_elapsed_time = 0;
    m_game_results.erase(index);

    std::ofstream logfile;
    logfile.open(ServerConfig::m_tourn_log, std::ios_base::app);
    if (logfile.is_open())
    {
        logfile << "RESET Game " + std::to_string(index) + "\n";
        logfile.close();
    }

    Log::info("TournamentManager", "game %d reset", index);
}

void TournamentManager::GetCurrentResult(int& red_goals, int& blue_goals)
{
    red_goals = m_current_game_result.m_red_goals;
    blue_goals = m_current_game_result.m_blue_goals;
}

void TournamentManager::SetCurrentResult(int red_goals, int blue_goals)
{
    m_current_game_result.m_red_goals = red_goals;
    m_current_game_result.m_blue_goals = blue_goals;

    World* world = World::getWorld();
    if (!world)
        return;

    SoccerWorld* sworld = dynamic_cast<SoccerWorld*>(world);
    if (!sworld)
        return;

    sworld->setInitialCount(red_goals, blue_goals);
    sworld->tellCountIfDiffers();
    Log::info("TournamentManager", "init %d %d", red_goals, blue_goals);
}

float TournamentManager::GetAdditionalSeconds() const
{
    return m_target_time - m_elapsed_time;
}

int TournamentManager::GetAdditionalMinutesRounded() const
{
    return std::max(0, ((int)GetAdditionalSeconds() + 30) / 60);
}

std::string TournamentManager::GetAdditionalTimeMessage() const
{
    int additional_minutes = GetAdditionalMinutesRounded();
    int minutes = 0, seconds = 0;
    GetAdditionalTime(minutes, seconds);
    std::string min_str = additional_minutes == 1 ? " minute" : " minutes";
    char sec_string[3];
    sprintf(sec_string, "%02d", seconds);
    return std::to_string(additional_minutes) + min_str + " (" + std::to_string(minutes) + ":" + sec_string + ") to replay.";
}

void TournamentManager::AddAdditionalSeconds(float seconds, const bool announce)
{
    m_target_time += seconds;

    auto sl = LobbyProtocol::get<ServerLobby>();
    if (!sl)
        return;

    std::string msg = std::to_string((int)(seconds / 60)) + " additional minutes"
        " will be played for already completed game "
        + std::to_string(m_current_game_index) + ".";
    sl->sendStringToAllPeers(msg);
}

void TournamentManager::AddAdditionalSeconds(int game, float seconds, const bool announce)
{
    if (GameDone(game))
    {
        m_current_game_index = game;
        if (m_current_game_result.m_blue_goals != 0 || m_current_game_result.m_red_goals != 0)
        {
            m_game_results[game].m_blue_goals = m_current_game_result.m_blue_goals;
            m_game_results[game].m_red_goals = m_current_game_result.m_red_goals;
        }
        m_current_game_result = m_game_results[game];
        m_target_time = m_current_game_result.m_elapsed_time + seconds;
        m_elapsed_time = m_current_game_result.m_elapsed_time;
        m_stopped_at = 0;
    }

    auto sl = LobbyProtocol::get<ServerLobby>();
    if (!sl)
        return;

    std::string msg = std::to_string((int)(seconds / 60)) + " additional minutes"
        " will be played for already completed game "
        + std::to_string(m_current_game_index) + ".";
    sl->sendStringToAllPeers(msg);
}

bool TournamentManager::GameInitialized() const
{
    return m_current_game_index > 0;
}

bool TournamentManager::GameOpen() const
{
    return GetAdditionalMinutesRounded() > 0;
}

bool TournamentManager::GameDone(int index) const
{
    return m_game_results.find(index) != m_game_results.end();
}

std::string TournamentManager::GetPlayedField() const
{
    return m_current_game_result.m_played_field;
}

bool TournamentManager::IsPlayedFieldForced() const
{
    return m_current_game_result.m_forced;
}
bool TournamentManager::IsGameVotable() const
{
    if (m_current_game_index > m_game_setup.size())
        return true;
    if (!m_current_game_result.m_played_field.empty())
        return false;
    const TournamentManager::TournGameSetup& cur = m_game_setup[m_current_game_index - 1];
    return cur.m_votable_addons || cur.m_votable_fields.empty() || cur.m_votable_fields.size() > 1;
}
bool TournamentManager::IsGameVotable(KartTeam team) const
{
    if (m_current_game_index > m_game_setup.size())
        return true;
    const TournamentManager::TournGameSetup& cur = m_game_setup[m_current_game_index - 1];
    return IsGameVotable() && (cur.m_team_choosing == KART_TEAM_NONE || cur.m_team_choosing == team);
}
PeerVote TournamentManager::GetForcedVote() const
{
    PeerVote res;
    res.m_reverse = false;
    res.m_player_name = L"";
    if (m_current_game_index > m_game_setup.size())
        return res;
    const struct TournGameSetup& gs = m_game_setup[m_current_game_index - 1];
    res.m_reverse = gs.m_random_items;

    const bool game_open = GameOpen();
    if (game_open && !m_current_game_result.m_played_field.empty())
    {
        res.m_num_laps = GetAdditionalMinutesRounded();
        res.m_track_name = GetPlayedField();
        Log::verbose("TournamentManager", "Forced vote: Additional minutes will be used, voted early: %d", res.m_num_laps);
        return res;
    }
    else if (game_open)
    {
        res.m_num_laps = GetAdditionalMinutesRounded();
        res.m_track_name = *gs.m_votable_fields.begin();
        Log::verbose("TournamentManager", "Forced vote: Additional minutes will be used, but was always fixed: %d", res.m_num_laps);
        return res;
    }
    else if (!gs.m_votable_addons && gs.m_votable_fields.size() == 1)
    {
        res.m_num_laps = gs.m_game_minutes;
        res.m_track_name = *gs.m_votable_fields.begin();
        Log::verbose("TournamentManager", "Forced vote: Fixed game field will be used.");
        return res;
    }
    return res;
}
bool TournamentManager::IsRandomItems() const
{
    if (m_current_game_index > m_game_setup.size())
        return false;
    return m_game_setup[m_current_game_index - 1].m_random_items;
}
void TournamentManager::SetPlayedField(std::string field, const bool force)
{
    m_current_game_result.m_played_field = field;
    m_current_game_result.m_forced = force;
    Log::verbose("TournamentManager", "Field set: %s", field.c_str());
}

bool TournamentManager::HasRequiredAddons(const std::set<std::string>& player_tracks) const
{

    for (const std::string& track : m_required_fields)
    {
        if (player_tracks.find(track) == player_tracks.end())
            return false;
    }

    int semi_required_count = 0;
    for (const std::string& track : m_semi_required_fields)
    {
        if (player_tracks.find(track) != player_tracks.end())
            semi_required_count++;
    }

    return semi_required_count >= m_semi_required_fields.size() -
        ServerConfig::m_tourn_semi_required_fields_minus;
}

std::set<std::string> TournamentManager::GetExcludedAddons(
        const std::set<std::string>& available_tracks)
{
    // TODO: change the behavior to TournGameSetup
    std::set<std::string> excluded_addons;
#if 0

    if (m_current_game_index == 2)
    {
        excluded_addons.insert("icy_soccer_field");
    }
    else if (m_current_game_index == 3)
    {
        excluded_addons.insert("icy_soccer_field");

        if (m_game_results.find(2) != m_game_results.end())
        {
            std::string field_game_2 = m_game_results[2].m_played_field;
            if (field_game_2 != "addon_wood-warbler-soccer")
                excluded_addons.insert(field_game_2);
        }
    }

    return excluded_addons;
#endif
    if (m_current_game_index > m_game_setup.size())
        return excluded_addons;
    const TournGameSetup& cur = m_game_setup[m_current_game_index - 1];

    
    if (cur.m_votable_addons)
    {
        Log::verbose("TournamentManager", "->This game is only for addons.");
        for (const std::string& track_name : available_tracks)
            if (!StringUtils::startsWith(track_name, "addon_"))
            {
                excluded_addons.insert(track_name);
            }
    }
    else if (cur.m_votable_fields.empty())
    {
        Log::verbose("TournamentManager", "->No whitelist for the votable fields.");
        return excluded_addons;
    }
    else
    {
        excluded_addons.insert(available_tracks.begin(), available_tracks.end());
        for (const std::string& field : cur.m_votable_fields)
        {
            excluded_addons.erase(field);
        }
    }
    // Log::verbose("TournamentManager", "->The other game results will be ruled out.");
    // for (auto& game_result : m_game_results)
    // {
    //     if (game_result.first >= m_current_game_index)
    //         return excluded_addons;
    //     // do not repeat already played addons
    //     excluded_addons.insert(game_result.second.m_played_field);
    // }
    return excluded_addons;
}
std::pair<size_t, std::string> TournamentManager::FormatMissingAddons(STKPeer* const peer,
        bool semi_required)
{
    std::set<std::string> missing_addons;
    std::set<std::string> installed_addons;
    if (semi_required && !m_semi_required_fields.empty())
        installed_addons.insert(m_semi_required_fields.cbegin(), m_semi_required_fields.cend());
    else if (!semi_required && !m_required_fields.empty())
        installed_addons.insert(m_required_fields.cbegin(), m_required_fields.cend());
    else
        return std::make_pair(0, "");
    peer->eraseServerTracks(installed_addons, missing_addons);
    
    size_t max_l = missing_addons.size();
    size_t cur_l = 0;
    std::string result;
    for (const std::string& addon_id : missing_addons)
    {
        cur_l++;
        result += addon_id;
        if (cur_l < max_l)
            result += ", ";
    }
    return std::make_pair(cur_l, result);
}
size_t TournamentManager::GetRequiredAddonAmount(bool semi_required)
{
    if (semi_required) return m_semi_required_fields.size();
    else return m_required_fields.size();
}

void TournamentManager::SetReferee(std::string name)
{
    m_referee = name;
}

void TournamentManager::SetVideo(std::string link)
{
    m_video = link;
}

TournamentManager::MatchplanGameResult 
TournamentManager::ParseGameEntryFrom(const std::string& str)
{
    TournamentManager::MatchplanGameResult res;
    if (str == g_matchplan_blank_word)
    {
        res.m_set = false;
        res.m_goal_red = 0;
        res.m_goal_blue = 0;
    }
    else
    {
        const std::string::size_type
            delimiter = str.find('-');
        if (delimiter == str.npos)
        {
            throw new std::runtime_error(
                StringUtils::insertValues("Game result format is invalid: %s", str));
        }
        res.m_goal_red = std::stoi(str.substr(0, delimiter));
        res.m_goal_blue = std::stoi(str.substr(delimiter + 1));
        res.m_set = true;
    }
    return res;
}
bool TournamentManager::LoadMatchPlan(bool load_game_results)
{
    bool success = false;
    try
    {
        std::ifstream matchplan_file(ServerConfig::m_matchplan_path, std::ios_base::in);
        matchplan_file.exceptions(std::ifstream::badbit);
        
        char c_line[2048];
        std::string line;
        unsigned line_count = 0;

        // Clear previous matchplan
        m_match_plan.clear();
        m_matchplan_map.clear();

        for(;!matchplan_file.eof() && !matchplan_file.bad() && !matchplan_file.fail();
                matchplan_file.getline(c_line, 2048), line = c_line)
        {
            MatchplanEntry entry = {};
            if (line.empty() || line[0] == '#')
                continue;
            ++line_count;

            auto votable_game_setup = m_game_setup.begin();
            int votable_game = 1;
            std::vector<std::string> contents = StringUtils::split(line, ' ');

            if (contents.size() < 11 - 2 + m_game_setup.size() + m_votable_amount)
            {
                Log::error("TournamentManager", "Invalid matchplan entry on line %d, expected at least %d columns, got %d",
                        line_count, 11 - 2 + m_game_setup.size() + m_votable_amount, contents.size());
            }

            // Order of the elements in the matchplan
            entry.m_team_red = contents[0];
            entry.m_team_blue = contents[1];

            // If this value is true, current entry is the currently configured match,
            // this will update the current game data
            const bool is_current_match =
                entry.m_team_red  == (std::string)ServerConfig::m_red_team &&
                entry.m_team_blue == (std::string)ServerConfig::m_blue_team;

            entry.m_weekday_name = contents[2];
            // the date is always formatted as YYYY-MM-DD
            std::string date_str = contents[3];
            if (date_str.size() != 10)
            {
                Log::error("TournamentManager", "Invalid date specified in %s, line %d: %s",
                        ServerConfig::m_matchplan_path.c_str(), line_count, date_str.c_str());
                continue;
            }
            // the time is always formatted like HH:MM
            std::string time_str = contents[4];
            if (time_str.size() != 5)
            {
                Log::error("TournamentManager", "Invalid time specified in %s, line %d: %s",
                        ServerConfig::m_matchplan_path.c_str(), line_count, time_str.c_str());
                continue;
            }

            entry.m_year    = std::stoi(date_str.substr(0, 4));
            entry.m_month   = std::stoi(date_str.substr(5, 2));
            entry.m_day     = std::stoi(date_str.substr(8, 2));
            entry.m_hour    = std::stoi(time_str.substr(0, 2));
            entry.m_minute  = std::stoi(time_str.substr(3, 2));

            entry.m_referee = contents[5] != g_matchplan_blank_word ? contents[5] : "";
            if (load_game_results && is_current_match && !entry.m_referee.empty())
                SetReferee(entry.m_referee);
            unsigned offset;
            entry.m_game_results.resize(m_game_setup.size());
            for (offset = 0; offset < m_game_setup.size(); ++offset)
            {
                MatchplanGameResult mpgr = ParseGameEntryFrom(contents[6 + offset]);
                entry.m_game_results[offset] = mpgr;
                if (load_game_results && is_current_match && mpgr.m_set)
                {
                    m_game_results[offset + 1].m_red_goals = mpgr.m_goal_red;
                    m_game_results[offset + 1].m_blue_goals = mpgr.m_goal_blue;
                }
            }
            entry.m_final_score = ParseGameEntryFrom(contents[6 + offset]);
            entry.m_winner_team = contents[7 + offset] != g_matchplan_blank_word ? contents[7 + offset] : "";

            entry.m_fields.resize(m_votable_amount);
            for (; offset < m_game_setup.size() + m_votable_amount; ++offset)
            {
                const std::string& field_id = contents[8 + offset];
                entry.m_fields[offset - m_game_setup.size()] = field_id != g_matchplan_blank_word ? contents[8 + offset] : "";

                // when ran out of votable game setup, don't update any played fields
                if (!is_current_match || !load_game_results || field_id == g_matchplan_blank_word || votable_game_setup >= m_game_setup.end())
                    continue;

                // find the next votable game and advance forward
                TournGameSetup gr = *votable_game_setup;
                while (votable_game_setup < m_game_setup.end() && (!gr.m_votable_addons && gr.m_votable_fields.size() == 1))
                {
                    votable_game_setup++;
                    votable_game++;
                    gr = *votable_game_setup;
                }
                if (votable_game_setup >= m_game_setup.end())
                    continue;

                // set the played field
                m_game_results[votable_game].m_played_field = field_id;
                // load next game
                votable_game_setup++;
                votable_game++;
            }
            entry.m_footage_url = contents[8 + offset] != g_matchplan_blank_word ? contents[8 + offset] : "";
            if (load_game_results && is_current_match && !entry.m_footage_url.empty())
                SetVideo(entry.m_footage_url);

            m_match_plan.push_back(entry);
            std::string key = entry.m_team_red + entry.m_team_blue;
            // also create an easy access to the MatchplanEntry
            MatchplanEntry* const ptr = &(*(m_match_plan.end() - 1));
            if (ptr->m_team_red != entry.m_team_red || ptr->m_team_blue != entry.m_team_blue)
            {
                Log::fatal("TournamentManager", "Literally bug in the code when assigning matchplan entry to the mapping. Tell DernisNW he sucks at coding.");
                std::raise(SIGABRT);
            }
            m_matchplan_map[key] = ptr;
            success = true;

        }
        Log::info("TournamentManager", "File %s has been loaded with %d rows.", ServerConfig::m_matchplan_path.c_str(),
                m_match_plan.size());
        return success;
    }
    catch (const std::exception& exception)
    {
        Log::fatal("TournamentManager", "Failed to load %s: %s",
                ServerConfig::m_matchplan_path.c_str(), exception.what());
        return false;
    }
}
void TournamentManager::SaveMatchPlan()
{
    if (m_match_plan.empty())
    {
        Log::error("TournamentManager", "Cannot save %s, nothing to save.", ServerConfig::m_matchplan_path.c_str());
        return;
    }
    try
    {
        std::ofstream matchplan_file(ServerConfig::m_matchplan_path, std::ios::out | std::ios::trunc);
        if (!matchplan_file.is_open())
        {
            Log::fatal("TournamentManager", "Failed to open %s for saving.",
                    ServerConfig::m_matchplan_path.c_str());
            return;
        }

        // Add header
        matchplan_file << "# RedTeam BlueTeam WeekDayName YYYY-MM-DD HH:MM RefereeName Game1Red-Game1Blue Game2Red-Game2Blue..."
            " ScoreRed-ScoreBlue WinnerTeam "
            "FieldChoice1 FieldChoice2... FootageURL" << std::endl;
        for (auto& entry : m_match_plan)
        {
            if (entry.m_game_results.size() != m_game_setup.size())
            {
                Log::error("TournamentManager", "Game results amount doesn't match with the game setup, size expected: %d, got: %d",
                        m_game_setup.size(), entry.m_game_results.size());
            }
            if (entry.m_fields.size() != m_votable_amount)
            {
                Log::error("TournamentManager", "Votable addon amount doesn't match with the game setup, size expected: %d, got: %d",
                        m_votable_amount, entry.m_fields.size());
                return;
            }
            char date_res[11];
            char time_res[6];
            std::snprintf(date_res, 11, "%.4u-%.2u-%.2u", entry.m_year, entry.m_month, entry.m_day);
            std::snprintf(time_res, 6, "%.2u:%.2u", entry.m_hour, entry.m_minute);
            matchplan_file << entry.m_team_red << " " << entry.m_team_blue << " " << 
                (entry.m_weekday_name.empty() ? g_matchplan_blank_word : entry.m_weekday_name) << " ";
            matchplan_file << date_res << " " << time_res << " ";
            matchplan_file << (entry.m_referee.empty() ? g_matchplan_blank_word : entry.m_referee) << " ";
            for (auto& game_entry : entry.m_game_results)
            {
                if (game_entry.m_set)
                    matchplan_file << game_entry.m_goal_red << "-" << game_entry.m_goal_blue << " ";
                else
                    matchplan_file << "? ";
            }
            if (entry.m_final_score.m_set)
                matchplan_file << entry.m_final_score.m_goal_red << "-"
                    << entry.m_final_score.m_goal_blue << " ";
            else
                matchplan_file << "? ";
            matchplan_file << (entry.m_winner_team.empty() ? g_matchplan_blank_word : entry.m_winner_team) << " ";

            for (std::string& field_id : entry.m_fields)
            {
                if (field_id.empty())
                    matchplan_file << "? ";
                else
                    matchplan_file << field_id << " ";
            }
            matchplan_file << (entry.m_footage_url.empty() ? g_matchplan_blank_word : entry.m_footage_url) << std::endl;
            matchplan_file.flush();
        }
        Log::info("TournamentManager", "File %s has been updated with %d rows.",
                ServerConfig::m_matchplan_path.c_str(), m_match_plan.size());
    }
    catch (const std::exception& exception)
    {
        Log::fatal("TournamentManager", "Failed to save %s: %s",
                ServerConfig::m_matchplan_path.c_str(), exception.what());
    }
}
void TournamentManager::UpdateMatchPlan(const std::string& team_red, const std::string& team_blue, const unsigned game_index,
        const unsigned goal_red, const unsigned goal_blue, const std::string& field_id,
        const std::string& referee, const std::string& footage_url)
{
    std::string key = team_red + team_blue;
    MatchplanEntry* ent = m_matchplan_map[key];
    if (!ent)
    {
        Log::error("TournamentManager",
                "Unable to update the matchplan for teams %s and %s, entry is not set in the mapping (%s).",
                team_red.c_str(), team_blue.c_str(), key.c_str());
        return;
    }
    if (game_index > ent->m_game_results.size())
    {
        Log::error("TournamentManager", "Unable to update the matchplan, game index (%d) exceeds set game size (%d).",
                game_index, ent->m_game_results.size());
    }
    MatchplanGameResult& gres = ent->m_game_results[game_index - 1];
    gres.m_set = true;
    gres.m_goal_blue = goal_blue;
    gres.m_goal_red = goal_red;
    if (!field_id.empty() && IsGameSetupVotable(m_game_setup[game_index - 1]))
        ent->m_fields[m_game_setup[game_index - 1].m_votable_offset] = field_id;
    if (!referee.empty())
        ent->m_referee = referee;
    if (!footage_url.empty())
        ent->m_footage_url = footage_url;
    
    // try to determine the final score and the winner
    unsigned int final_red = 0, final_blue = 0;
    for (const auto& gres2 : ent->m_game_results)
    {
        if (!gres2.m_set)
            return;

        if (gres2.m_goal_blue < gres2.m_goal_red)
            final_red++;
        else if (gres2.m_goal_red < gres2.m_goal_blue)
            final_blue++;
        else
        {
            final_red++;
            final_blue++;
        }
    }
    ent->m_final_score.m_set = true;
    ent->m_final_score.m_goal_red = final_red;
    ent->m_final_score.m_goal_blue = final_blue;
    if (final_blue < final_red)
        ent->m_winner_team = ent->m_team_red;
    else if (final_red < final_blue)
        ent->m_winner_team = ent->m_team_blue;
    else
        ent->m_winner_team = "Draw";

    LogMatchResults(*ent);
}
void TournamentManager::LogMatchResults(const MatchplanEntry& ent) const
{
    std::stringstream ss;
    ss << "match_finished " << ent.m_team_red << " " << ent.m_team_blue << " " << ent.m_referee << " ";
    for (const auto& gs : ent.m_game_results)
        ss << gs.m_goal_red << "-" << gs.m_goal_blue << " ";
    ss << ent.m_final_score.m_goal_red << "-" << ent.m_final_score.m_goal_blue << " ";
    ss << ent.m_winner_team << " ";
    for (const auto& field_id : ent.m_fields)
        ss << field_id << " ";
    ss << ent.m_footage_url;

    Log::info("TournamentManager", ss.str().c_str());
}

void TournamentManager::LoadGamePlan()
{
    try
    {
        std::ifstream gameplan_file(ServerConfig::m_gameplan_path, std::ios::in);
        gameplan_file.exceptions(std::ifstream::badbit);
        if (!gameplan_file.is_open())
        {
            Log::fatal("TournamentManager", "Cannot load file %s, file does not exist.",
                    ServerConfig::m_gameplan_path.c_str());
            return;
        }

        m_game_setup.clear();
        m_votable_amount = 0;

        char c_line[2048];
        std::string line;
        unsigned line_count = 0;
        struct TournGameSetup game_setup;
        for (; !gameplan_file.eof() && !gameplan_file.bad() && !gameplan_file.fail();
                gameplan_file.getline(c_line, 2048), line = c_line)
        {
            if (line.empty() || line[0] == '#')
                continue;
            ++line_count;


            std::vector<std::string> contents = StringUtils::split(line, ' ');
            if (contents.size() < 3)
            {
                Log::error("TournamentManager", "Invalid game setup line in %s, line %d, expected %d columns, got %d: %s",
                        ServerConfig::m_gameplan_path.c_str(), line_count, 3, contents.size(), line.c_str());
                continue;
            }
            
            game_setup.m_game_minutes = std::stoi(contents[0]);
            game_setup.m_random_items = contents[1][0] == 'y';

            if (contents[2][0] == 'r')
                game_setup.m_team_choosing = KART_TEAM_RED;
            else if (contents[2][0] == 'b')
                game_setup.m_team_choosing = KART_TEAM_BLUE;
            else if (contents[2][0] == 'n' || contents[2][0] == 'a')
                game_setup.m_team_choosing = KART_TEAM_NONE;

            game_setup.m_votable_offset = m_votable_amount;
            if (contents.size() > 3)
            {
                if (contents[3] == "=ADDONS")
                {
                    game_setup.m_votable_addons = true;
                    m_votable_amount++;
                }
                else if (contents[3] != "=ALL")
                {
                    // use the contents vector to set all the fields
                    contents.erase(contents.cbegin(), contents.cbegin() + 3);
                    game_setup.m_votable_fields.insert(contents.cbegin(), contents.cend());
                    if (contents.size() > 1)
                    {
                        m_votable_amount++;
                    }
                }
                else
                {
                    m_votable_amount++;
                }
            }
            else
                m_votable_amount++;
            m_game_setup.push_back(game_setup);
        }
        Log::info("TournamentManager", "Amount of games per match: %d, with %d votables.",
                m_game_setup.size(), m_votable_amount);
    }
    catch (const std::exception& exception)
    {
        Log::fatal("TournamentManager", "Cannot load file %s: %s",
                ServerConfig::m_gameplan_path.c_str(), exception.what());
    }
}
void TournamentManager::LoadSTDSetFromFile(std::set<std::string>& target, const std::string&& filename)
{
    try
    {
        std::ifstream ssv_file(filename, std::ios::in);
        ssv_file.exceptions(std::ifstream::badbit);
        if (!ssv_file.is_open())
        {
            Log::fatal("TournamentManager", "Cannot load file %s, file does not exist.",
                    filename.c_str());
            return;
        }

        //target.clear();
        
        for (; !ssv_file.eof() && !ssv_file.bad() && !ssv_file.fail();)
        {
            std::string word;
            ssv_file >> word;
            if (!word.empty())
                target.insert(word);
        }
    }
    catch (const std::exception& exception)
    {
        Log::fatal("TournamentManager", "Cannot load file %s: %s",
                ServerConfig::m_gameplan_path.c_str(), exception.what());
    }
}
void TournamentManager::LoadRequiredFields()
{
    m_required_fields.clear();
    LoadSTDSetFromFile(m_required_fields, ServerConfig::m_tourn_required_fields_path);
}
void TournamentManager::LoadSemiRequiredFields()
{
    m_semi_required_fields.clear();
    LoadSTDSetFromFile(m_semi_required_fields, ServerConfig::m_tourn_semi_required_fields_path);
}
bool TournamentManager::IsGameSetupVotable(const TournamentManager::TournGameSetup setup) const
{
    return setup.m_votable_addons || setup.m_votable_fields.size() != 1;
}
