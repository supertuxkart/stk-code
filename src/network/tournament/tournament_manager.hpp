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

#ifndef TOURNAMENT_MANAGER_HPP
#define TOURNAMENT_MANAGER_HPP

#include "modes/soccer_world.hpp"
#include "network/remote_kart_info.hpp"
#include <map>
#include <set>
#include <string>

class PeerVote;

struct GameResult
{
    int m_red_goals;
    int m_blue_goals;
    std::vector<SoccerWorld::ScorerData> m_red_scorers;
    std::vector<SoccerWorld::ScorerData> m_blue_scorers;
    std::string m_played_field;
    // whether or not the field is forced or chosen
    bool m_forced;
    float m_elapsed_time;

    GameResult() 
    {
        m_red_goals = 0;
        m_blue_goals = 0;
        m_played_field = "";
        m_elapsed_time = 0;
        m_forced = false;
    };

    GameResult(std::vector<SoccerWorld::ScorerData> red_scorers, std::vector<SoccerWorld::ScorerData> blue_scorers)
    {
        m_red_goals = 0;
        m_blue_goals = 0;
        m_red_scorers = red_scorers;
        m_blue_scorers = blue_scorers;
        m_played_field = "";
        m_elapsed_time = 0;
        m_forced = false;
    }
};

class TournamentManager
{
public:

    // how does one tournament game differ from other games
    // for example, one game is played on Icy Soccer Field,
    // next game is an addon game chosen by the red team
    // next game is an addon game chosen by the blue team
    // next game is an addon game chosen by all teams
    // and finally, last game is played on Wood Warbler Soccer.
    struct TournGameSetup
    {
        // how long does the game go
        int m_game_minutes = 7;
        // scatters random items around the map, no by default.
        bool m_random_items = false;
        // all teams are allowed to vote by default
        KartTeam m_team_choosing = KART_TEAM_NONE;
        // which fields can be voted in,
        // if empty, all fields can be voted, unless m_votable_addons is true.
        // if there's only one, it is forced.
        std::set<std::string> m_votable_fields;
        // ... or, specify this as true, makes votable only addon fields.
        bool m_votable_addons = false;
        unsigned int m_votable_offset = 0;
    };
    struct MatchplanGameResult
    {
        bool     m_set = false;
        unsigned m_goal_red = 0,
                 m_goal_blue = 0;
    };
    struct MatchplanEntry
    {
        std::string m_weekday_name;
        std::string m_team_red;
        std::string m_team_blue;
        std::string m_winner_team;
        std::string m_referee, m_footage_url;
        std::vector<struct MatchplanGameResult>
                    m_game_results;
        struct MatchplanGameResult
                    m_final_score;
        std::vector<std::string> m_fields;
        unsigned    m_year, m_month, m_day,
                    m_hour, m_minute;
        bool        m_is_draw;
    };

private:
    std::map<std::string, std::string> m_player_teams; // m_player_teams[player1] = "A"
    std::vector<struct MatchplanEntry> m_match_plan;
    std::map<std::string, struct MatchplanEntry*> m_matchplan_map;
    std::string m_red_team; // "A"
    std::string m_blue_team; // "B"
    std::set<std::string> m_red_players;
    std::set<std::string> m_blue_players;
    std::map<std::string, std::string> m_player_karts;
    std::set<std::string> m_required_fields{ "icy_soccer_field", "soccer_field", "lasdunassoccer", "addon_wood-warbler-soccer" };
    std::set<std::string> m_semi_required_fields{ "addon_xr-4r3n4_1", "addon_ice-rink_1", "addon_hole-drop", "addon_skyline--soccer-","addon_babyfball"};
    
    std::string m_referee = "vinder-af-karting-spil";
    std::string m_video = "https://tierchester.eu/stk-supertournament/";

    // Size of this vector defines the game amount per each match,
    // and the sum of all TournGameSetup that has m_votable_addons set to true,
    // or m_votable_fields of size not equal to 1, defines the 
    // amount of field ids that are set in each MatchplanEntry.
    //
    std::vector<struct TournGameSetup> m_game_setup;
    std::map<int, GameResult> m_game_results;
    unsigned int m_votable_amount = 0;
    int m_current_votable_game = -1;
    int m_current_game_index = -1; // 1-based count index of the games
    GameResult m_current_game_result;
    float m_target_time = 0;
    float m_elapsed_time = 0;
    float m_stopped_at = 0;

    void FilterScorerData(std::vector<SoccerWorld::ScorerData>& scorers);
    void GetAdditionalTime(int& minutes, int& seconds) const;
    void OnGameEnded();
    bool IsGameSetupVotable(TournGameSetup setup) const;
    static MatchplanGameResult ParseGameEntryFrom(const std::string& str);
    static const char*         g_matchplan_blank_word;

    static TournamentManager* g_tournament_manager;
    void LoadSTDSetFromFile(std::set<std::string>& target, const std::string&& filename);

public:
    TournamentManager();
    virtual ~TournamentManager();

    static TournamentManager* get()
    {
        assert(g_tournament_manager);
        return g_tournament_manager;
    }
    static void create();
    static void destroy();
    static void clear();

    bool IsGameOOB()
    {
        return m_current_game_index > m_game_setup.size();
    }
    void InitializePlayersAndTeams(std::string red_team, std::string blue_team);
    void UpdateTeams(std::string red_team, std::string blue_team);
    std::string GetTeam(std::string player_name);
    KartTeam GetKartTeam(std::string player_name) const;
    void SetKartTeam(std::string player_name, KartTeam team);
    std::string GetKart(std::string player_name) const;
    void SetKart(std::string player_name, std::string kart_name);
    std::set<std::string> GetKartRestrictedUsers() const;
    bool CanPlay(std::string player_name) const;
    bool CountPlayerVote(std::string player_name) const;
    bool CountPlayerVote(STKPeer* peer) const;

    void StartNextGame(bool announce = true);
    void StartGame(int index, float target_time, bool announce = true);
    void StartGame(int index, bool announce = true);
    void StopGame(float elapsed_time, bool announce = true);
    void ResumeGame(float elapsed_time, bool announce = true);
    unsigned GetMaxGames() const { return m_game_setup.size(); }
    void HandleGameResult(float elapsed_time, GameResult result);
    void ForceEndGame();
    void ResetGame(int index);
    void GetCurrentResult(int& red_goals, int& blue_goals);
    void SetCurrentResult(int red_goals, int blue_goals);
    float GetAdditionalSeconds() const;
    int GetAdditionalMinutesRounded() const;
    std::string GetAdditionalTimeMessage() const;
    void AddAdditionalSeconds(float seconds, bool announce = true);
    void AddAdditionalSeconds(int game, float seconds, bool announce = true);
    bool GameInitialized() const;
    bool GameOpen() const;
    bool GameDone(int index) const;
    std::string GetPlayedField() const;
    bool IsPlayedFieldForced() const;
    bool IsGameVotable() const;
    bool IsGameVotable(KartTeam team) const;
    bool IsRandomItems() const;
    PeerVote GetForcedVote() const;
    void SetPlayedField(std::string field, bool force = false);
    bool HasRequiredAddons(const std::set<std::string>& player_tracks) const;
    std::set<std::string> GetExcludedAddons(const std::set<std::string>& available_tracks);
    std::pair<size_t, std::string> FormatMissingAddons(STKPeer* peer, bool semi_required = false);
    size_t GetRequiredAddonAmount(bool semi_required = false);
    void SetReferee(std::string name);
    void SetVideo(std::string name);

    bool LoadMatchPlan(bool load_game_results = true);
    void SaveMatchPlan();
    void UpdateMatchPlan(const std::string& team_red, const std::string& team_blue, unsigned game_index,
            unsigned goal_red, unsigned goal_blue, const std::string& field_id,
            const std::string& referee, const std::string& footage_url);
    void LoadGamePlan();
    void LoadRequiredFields();
    void LoadSemiRequiredFields();
    void LogMatchResults(const MatchplanEntry& ent) const;
};

#endif
