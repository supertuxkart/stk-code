//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2013-2015 SuperTuxKart-Team
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

#include "network/game_setup.hpp"

#include "config/player_manager.hpp"
#include "config/user_config.hpp"
#include "network/tournament/tournament_manager.hpp"
#include <limits>
#ifdef DEBUG
#include "network/network_config.hpp"
#endif
#include "network/network_player_profile.hpp"
#include "network/peer_vote.hpp"
#include "network/protocols/server_lobby.hpp"
#include "network/server_config.hpp"
#include "network/stk_host.hpp"
#include "race/race_manager.hpp"
#include "utils/file_utils.hpp"
#include "utils/log.hpp"
#include "utils/stk_process.hpp"
#include "utils/string_utils.hpp"
#include "tracks/track_manager.hpp"

#include <algorithm>
#include <cerrno>
#include <fstream>
#include <random>

//-----------------------------------------------------------------------------
GameSetup::GameSetup()
{
    // Read txt file for motd, if present
    const std::string& motd = ServerConfig::m_motd;
    if (motd.find(".txt") != std::string::npos)
    {
        const std::string& path = ServerConfig::getConfigDirectory() + "/" +
            motd;
        std::ifstream message(FileUtils::getPortableReadingPath(path));
        if (message.is_open())
        {
            for (std::string line; std::getline(message, line); )
            {
                m_message_of_today += StringUtils::utf8ToWide(line).trim() +
                    L"\n";
            }
            // Remove last newline
            m_message_of_today.erase(m_message_of_today.size() - 1);
        }
    }
    else if (!motd.empty())
        m_message_of_today = StringUtils::xmlDecode(motd);

    const std::string& server_name = ServerConfig::m_server_name;
    m_server_name_utf8 = StringUtils::wideToUtf8
        (StringUtils::xmlDecode(server_name));
    m_extra_server_info = -1;
    m_is_grand_prix.store(false);

#if 0
    // Read txt file for random installaddon lines, if present
    m_addons_of_the_day.clear();
    //m_addons_of_the_day.shrink_to_fit();
    
    const std::string& ril = ServerConfig::m_random_installaddon_lines;
    if (!ril.empty())
    {
        const std::string& path = ServerConfig::getConfigDirectory() + "/" +
            ril;
        std::ifstream message(FileUtils::getPortableReadingPath(path));
        if (message.is_open())
        {
            for (std::string line; std::getline(message, line); )
            {
	        m_addons_of_the_day.push_back(
		    StringUtils::utf8ToWide(line).trim()
		);
            }
        }
	else
	{
	    Log::error("GameSetup", "Could not read random-installaddon-lines file: %s", strerror(errno));
	}
    }
#endif

    reset();
}   // GameSetup

//-----------------------------------------------------------------------------
void GameSetup::loadWorld()
{
    // Notice: for arena (battle / soccer) lap and reverse will be mapped to
    // goals / time limit and random item location
    assert(!m_tracks.empty());
    // Disable accidentally unlocking of a challenge
    if (STKProcess::getType() == PT_MAIN && PlayerManager::getCurrentPlayer())
        PlayerManager::getCurrentPlayer()->setCurrentChallenge("");
    RaceManager::get()->setTimeTarget(0.0f);
    if (RaceManager::get()->isSoccerMode() ||
        RaceManager::get()->isBattleMode())
    {
        const bool is_ctf = RaceManager::get()->getMinorMode() ==
            RaceManager::MINOR_MODE_CAPTURE_THE_FLAG;
        bool prev_val = UserConfigParams::m_random_arena_item;
        if (is_ctf)
            UserConfigParams::m_random_arena_item = false;
        else
            UserConfigParams::m_random_arena_item = m_reverse;

        RaceManager::get()->setReverseTrack(false);

        if (RaceManager::get()->isSoccerMode())
        {
#if 0
            if (ServerConfig::m_supertournament && TournamentManager::get()->GameInitialized())
            {
                RaceManager::get()->setTimeTarget(TournamentManager::get()->GetAdditionalSeconds());
            }
            else 
#endif
            if (isSoccerGoalTarget())
                RaceManager::get()->setMaxGoal(m_laps);
            else
                RaceManager::get()->setTimeTarget((float)m_laps * 60.0f);
        }
        else
        {
            RaceManager::get()->setHitCaptureTime(m_hit_capture_limit,
                m_battle_time_limit);
        }
        RaceManager::get()->startSingleRace(m_tracks.back(), -1,
            false/*from_overworld*/);
        UserConfigParams::m_random_arena_item = prev_val;
    }
    else
    {
        RaceManager::get()->setReverseTrack(m_reverse);
        RaceManager::get()->startSingleRace(m_tracks.back(), m_laps,
                                      false/*from_overworld*/);
    }
}   // loadWorld

//-----------------------------------------------------------------------------
void GameSetup::addServerInfo(NetworkString* ns)
{
#ifdef DEBUG
    assert(NetworkConfig::get()->isServer());
#endif
    ns->encodeString(m_server_name_utf8);
    auto sl = LobbyProtocol::get<ServerLobby>();
    assert(sl);
    ns->addUInt8((uint8_t)sl->getDifficulty())
        .addUInt8((uint8_t)ServerConfig::m_server_max_players)
        // Reserve for extra spectators
        .addUInt8(0)
        .addUInt8((uint8_t)sl->getGameMode());
    if (hasExtraSeverInfo())
    {
        if (isGrandPrix())
        {
            uint8_t cur_track = (uint8_t)m_tracks.size();
            if (!isGrandPrixStarted())
                cur_track = 0;
            ns->addUInt8((uint8_t)2).addUInt8(cur_track)
                .addUInt8(getExtraServerInfo());
        }
        else
        {
            // Soccer mode
            ns->addUInt8((uint8_t)1).addUInt8(getExtraServerInfo());
        }
    }
    else
    {
        // No extra server info
        ns->addUInt8((uint8_t)0);
    }
    if (ServerConfig::m_supertournament)
    {
        ns->addUInt8(ServerConfig::m_min_start_game_players)
            .addFloat(std::numeric_limits<float>::max());
    }
    else if (ServerConfig::m_owner_less)
    {
        ns->addUInt8(ServerConfig::m_min_start_game_players)
            .addFloat(ServerConfig::m_start_game_counter);
    }
    else
        ns->addUInt8(0).addFloat(0.0f);
#if 0
    if (!m_addons_of_the_day.empty()) {
        // In case config has random-installaddon-lines
	irr::core::stringw total_motd(m_message_of_today);
	
	// Delimiter for the prefix
	total_motd.append(L"\n", 1);
	total_motd.append(StringUtils::utf8ToWide(ServerConfig::m_ril_prefix));
	total_motd.append(L" ", 1);

	// Pick random line
        std::random_device rd;
        std::mt19937_64 g(rd());

	// add the latter
        total_motd.append( m_addons_of_the_day[g() % m_addons_of_the_day.size()] );
        ns->encodeString16(total_motd);

    }
    else
#endif
    ns->encodeString16(m_message_of_today);
    ns->addUInt8((uint8_t)ServerConfig::m_server_configurable);
    ns->addUInt8(ServerConfig::m_live_players? 1 : 0);
}   // addServerInfo
//-----------------------------------------------------------------------------
void GameSetup::addModifiedServerInfo(
        NetworkString* ns,
        int difficulty,
        int server_max_players,
        uint8_t extra_spectators,
        int server_game_mode,
        int extra_server_info,
        int min_start_game_players,
        float start_game_counter,
        const irr::core::stringw& motd,
        bool motd_override,
        bool configurable,
        bool configurable_override,
        bool live_players,
        bool live_players_override)
{
#ifdef DEBUG
    assert(NetworkConfig::get()->isServer());
#endif
    ns->encodeString(m_server_name_utf8);
    auto sl = LobbyProtocol::get<ServerLobby>();
    assert(sl);

    if (difficulty == -1)
        difficulty = sl->getDifficulty();
    if (server_max_players == -1)
        server_max_players = ServerConfig::m_server_max_players;
    if (server_game_mode == -1)
        server_game_mode = sl->getGameMode();
    if (min_start_game_players == -1)
        min_start_game_players = ServerConfig::m_min_start_game_players;
    if (start_game_counter == -1.0f)
        start_game_counter = ServerConfig::m_start_game_counter;
    if (!configurable_override)
        configurable = ServerConfig::m_server_configurable;
    if (!live_players_override)
        live_players = ServerConfig::m_live_players;

    ns->addUInt8((uint8_t)difficulty)
        .addUInt8((uint8_t)server_max_players)
        // Reserve for extra spectators
        .addUInt8(extra_spectators)
        .addUInt8(server_game_mode);
    if (hasExtraSeverInfo() && extra_server_info == -1)
    {
        if (isGrandPrix())
        {
            uint8_t cur_track = (uint8_t)m_tracks.size();
            if (!isGrandPrixStarted())
                cur_track = 0;
            ns->addUInt8((uint8_t)2).addUInt8(cur_track)
                .addUInt8(getExtraServerInfo());
        }
        else
        {
            // Soccer mode
            ns->addUInt8((uint8_t)1).addUInt8(getExtraServerInfo());
        }
    }
    else
    {
        // No extra server info
        ns->addUInt8((uint8_t)extra_server_info);
    }
    if (ServerConfig::m_owner_less)
    {
        ns->addUInt8(min_start_game_players)
            .addFloat(start_game_counter);
    }
    else
        ns->addUInt8(0).addFloat(0.0f);
    ns->encodeString16(motd_override ? motd : m_message_of_today);
    ns->addUInt8((uint8_t)configurable);
    ns->addUInt8(live_players? 1 : 0);
}   // addModifiedServerInfo

//-----------------------------------------------------------------------------
void GameSetup::sortPlayersForGrandPrix(
    std::vector<std::shared_ptr<NetworkPlayerProfile> >& players) const
{
    if (!isGrandPrix())
        return;

    if (m_tracks.size() == 1)
    {
        std::random_device rd;
        std::mt19937 g(rd());
        std::shuffle(players.begin(), players.end(), g);
        return;
    }

    std::sort(players.begin(), players.end(),
        [](const std::shared_ptr<NetworkPlayerProfile>& a,
        const std::shared_ptr<NetworkPlayerProfile>& b)
        {
            return (a->getScore() < b->getScore()) ||
                (a->getScore() == b->getScore() &&
                a->getOverallTime() > b->getOverallTime());
        });
    if (UserConfigParams::m_gp_most_points_first)
    {
        std::reverse(players.begin(), players.end());
    }
}   // sortPlayersForGrandPrix

//-----------------------------------------------------------------------------
void GameSetup::sortPlayersForGame(
    std::vector<std::shared_ptr<NetworkPlayerProfile> >& players,
            unsigned ignoreLeading, const bool shuffle) const
{
    ignoreLeading = std::min(ignoreLeading, (unsigned)players.size());
    if (!isGrandPrix() && shuffle)
    {
        std::random_device rd;
        std::mt19937 g(rd());
        std::shuffle(players.begin() + 
                ignoreLeading,
            players.end(), g);
        Log::verbose("GameSetup", "Player positions have been shuffled, "
                "ignoreLeading = %u", ignoreLeading);
    }
    if (!RaceManager::get()->teamEnabled() ||
        ServerConfig::m_team_choosing)
        return;
    for (unsigned i = 0; i < players.size(); i++)
    {
        players[i]->setTeam((KartTeam)(i % 2));
    }
}   // sortPlayersForGame

// ----------------------------------------------------------------------------
void GameSetup::setRace(const PeerVote &vote)
{
    m_tracks.push_back(vote.m_track_name);
    m_laps = vote.m_num_laps;
    m_reverse = vote.m_reverse;
}   // setRace
