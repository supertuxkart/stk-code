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

#include <algorithm>
#include <fstream>
#include <random>

//-----------------------------------------------------------------------------
GameSetup::GameSetup()
{
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
    if (ServerConfig::m_owner_less)
    {
        ns->addUInt8(ServerConfig::m_min_start_game_players)
            .addFloat(ServerConfig::m_start_game_counter);
    }
    else
        ns->addUInt8(0).addFloat(0.0f);

    ns->encodeString16(m_message_of_today);
    ns->addUInt8((uint8_t)ServerConfig::m_server_configurable);
    ns->addUInt8(ServerConfig::m_live_players? 1 : 0);
}   // addServerInfo

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
    std::vector<std::shared_ptr<NetworkPlayerProfile> >& players) const
{
    if (!isGrandPrix())
    {
        std::random_device rd;
        std::mt19937 g(rd());
        std::shuffle(players.begin(), players.end(), g);
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
