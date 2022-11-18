//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2018 SuperTuxKart-Team
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

#include "utils/log.hpp"
#include "utils/file_utils.hpp"
#include "utils/string_utils.hpp"
#include "utils/translation.hpp"

#include <vector>

// The order here is important. If all_params is declared later (e.g. after
// the #includes), all elements will be added to all_params, and then
// g_server_params will be initialised, i.e. cleared!
// ============================================================================
class UserConfigParam;
static std::vector<UserConfigParam*> g_server_params;
// ============================================================================

// X-macros
#define SERVER_CFG_PREFIX
#define SERVER_CFG_DEFAULT(X) = X

#include "network/server_config.hpp"
#include "config/stk_config.hpp"
#include "io/file_manager.hpp"
#include "network/game_setup.hpp"
#include "network/network_config.hpp"
#include "network/protocols/lobby_protocol.hpp"
#include "network/stk_host.hpp"
#include "race/race_manager.hpp"
#include "utils/string_utils.hpp"

#include <fstream>

#include <IFileSystem.h>

namespace ServerConfig
{
// ============================================================================
std::string g_server_config_path;
std::string m_server_uid;
// ============================================================================
FloatServerConfigParam::FloatServerConfigParam(float default_value,
                                               const char* param_name,
                                               const char* comment)
                      : FloatUserConfigParam(param_name, comment)
{
    m_can_be_deleted = false;
    m_value = default_value;
    m_default_value = default_value;
    g_server_params.push_back(this);
}   // FloatServerConfigParam

// ============================================================================
IntServerConfigParam::IntServerConfigParam(int default_value,
                                           const char* param_name,
                                           const char* comment)
                    : IntUserConfigParam(param_name, comment)
{
    m_can_be_deleted = false;
    m_value = default_value;
    m_default_value = default_value;
    g_server_params.push_back(this);
}   // IntServerConfigParam

// ============================================================================
BoolServerConfigParam::BoolServerConfigParam(bool default_value,
                                             const char* param_name,
                                             const char* comment)
                     : BoolUserConfigParam(param_name, comment)
{
    m_can_be_deleted = false;
    m_value = default_value;
    m_default_value = default_value;
    g_server_params.push_back(this);
}   // BoolServerConfigParam

// ============================================================================
StringServerConfigParam::StringServerConfigParam(std::string default_value,
                                                 const char* param_name,
                                                 const char* comment)
                       : StringUserConfigParam(param_name, comment)
{
    m_can_be_deleted = false;
    m_value = default_value;
    m_default_value = default_value;
    g_server_params.push_back(this);
}   // StringServerConfigParam

// ============================================================================
template<typename T, typename U>
MapServerConfigParam<T, U>::MapServerConfigParam(const char* param_name,
    const char* comment, std::array<std::string, 3> key_names,
    std::map<T, U> default_value)
                          : MapUserConfigParam<T, U>(param_name, comment)
{
    m_key_names = key_names;
    m_elements = default_value;
    g_server_params.push_back(this);
}   // MapServerConfigParam

// ============================================================================
void loadServerConfig(const std::string& path)
{
    bool default_config = false;
    if (path.empty())
    {
        default_config = true;
        g_server_config_path =
            file_manager->getUserConfigFile("server_config.xml");
    }
    else
    {
        g_server_config_path = file_manager->getFileSystem()
            ->getAbsolutePath(path.c_str()).c_str();
    }
    m_server_uid = StringUtils::removeExtension(
        StringUtils::getBasename(g_server_config_path));
    const XMLNode* root = file_manager->createXMLTree(g_server_config_path);
    loadServerConfigXML(root, default_config);
}   // loadServerConfig

// ----------------------------------------------------------------------------
void loadServerConfigXML(const XMLNode* root, bool default_config)
{
    if (!root || root->getName() != "server-config")
    {
        Log::info("ServerConfig",
            "Could not read server config file '%s'. "
            "A new file will be created.", g_server_config_path.c_str());
        if (root)
            delete root;
        writeServerConfigToDisk();
        return;
    }

    int config_file_version = -1;
    if (root->get("version", &config_file_version) < 1 ||
        config_file_version < stk_config->m_min_server_version ||
        config_file_version > stk_config->m_max_server_version)
    {
        Log::info("ServerConfig", "Your config file was not compatible, "
            "so it was deleted and a new one will be created.");
        delete root;
        writeServerConfigToDisk();
        return;
    }

    for (unsigned i = 0; i < g_server_params.size(); i++)
        g_server_params[i]->findYourDataInAChildOf(root);

    delete root;
}   // loadServerConfigXML

// ----------------------------------------------------------------------------
std::string getServerConfigXML()
{
    std::stringstream ss;

    ss << "<?xml version=\"1.0\"?>\n";
    ss << "<server-config version=\"" << m_server_version << "\" >\n\n";

    for (unsigned i = 0; i < g_server_params.size(); i++)
        g_server_params[i]->write(ss);

    ss << "</server-config>\n";
    return ss.str();
}   // getServerConfigXML

// ----------------------------------------------------------------------------
void writeServerConfigToDisk()
{
    const std::string& config_xml = getServerConfigXML();
    try
    {
        // Save to a new file and rename later to avoid disk space problem, see #4709
        std::ofstream configfile(FileUtils::getPortableWritingPath(
            g_server_config_path + "new"), std::ofstream::out);
        configfile << config_xml;
        configfile.close();
        file_manager->removeFile(g_server_config_path);
        FileUtils::renameU8Path(g_server_config_path + "new", g_server_config_path);
    }
    catch (std::runtime_error& e)
    {
        Log::error("ServerConfig", "Failed to write server config to %s, "
            "because %s", g_server_config_path.c_str(), e.what());
    }
}   // writeServerConfigToDisk

// ----------------------------------------------------------------------------
/** Returns the minor and major game mode from server database id. */
std::pair<RaceManager::MinorRaceModeType, RaceManager::MajorRaceModeType>
    getLocalGameMode(int mode)
{
    switch (mode)
    {
        case 0:
            return { RaceManager::MINOR_MODE_NORMAL_RACE,
                RaceManager::MAJOR_MODE_GRAND_PRIX };
        case 1:
            return { RaceManager::MINOR_MODE_TIME_TRIAL,
                RaceManager::MAJOR_MODE_GRAND_PRIX };
        case 2:
            return { RaceManager::MINOR_MODE_FOLLOW_LEADER,
                RaceManager::MAJOR_MODE_GRAND_PRIX };
        case 3:
            return { RaceManager::MINOR_MODE_NORMAL_RACE,
                RaceManager::MAJOR_MODE_SINGLE };
        case 4:
            return { RaceManager::MINOR_MODE_TIME_TRIAL,
                RaceManager::MAJOR_MODE_SINGLE };
        case 5:
            return { RaceManager::MINOR_MODE_FOLLOW_LEADER,
                RaceManager::MAJOR_MODE_SINGLE };
        case 6:
            return { RaceManager::MINOR_MODE_SOCCER,
                RaceManager::MAJOR_MODE_SINGLE };
        case 7:
            return { RaceManager::MINOR_MODE_FREE_FOR_ALL,
                RaceManager::MAJOR_MODE_SINGLE };
        case 8:
            return { RaceManager::MINOR_MODE_CAPTURE_THE_FLAG,
                RaceManager::MAJOR_MODE_SINGLE };
        default:
            break;
    }
    return { RaceManager::MINOR_MODE_NORMAL_RACE,
        RaceManager::MAJOR_MODE_SINGLE };

}   // getLocalGameMode

// ----------------------------------------------------------------------------
std::pair<RaceManager::MinorRaceModeType, RaceManager::MajorRaceModeType>
    getLocalGameModeFromConfig()
{
    return getLocalGameMode(m_server_mode);
}   // getLocalGameModeFromConfig

// ----------------------------------------------------------------------------
core::stringw getModeName(unsigned id)
{
    switch(id)
    {
        case 0:
            return _("Normal Race (Grand Prix)");
        case 1:
            return _("Time Trial (Grand Prix)");
        case 3:
            return _("Normal Race");
        case 4:
            return _("Time Trial");
        case 6:
            return _("Soccer");
        case 7:
            // I18n: Free for all means a deathmatch game with battle mode in
            // networking
            return _("Free-For-All");
        case 8:
            return _("Capture The Flag");
        default:
            return L"";
    }
}   // getModeName

// ----------------------------------------------------------------------------
void loadServerLobbyFromConfig()
{
    if (unsupportedGameMode())
        Log::fatal("ServerConfig", "Unsupported game mode");

    if (stk_config->time2Ticks(m_flag_return_timeout) > 65535 ||
        m_flag_return_timeout <= 0.0f)
    {
        float timeout = m_flag_return_timeout;
        // in CTFFlag it uses 16bit unsigned integer for timeout
        Log::warn("ServerConfig", "Invalid %f m_flag_return_timemout which "
            "is invalid, use default value.", timeout);
        m_flag_return_timeout.revertToDefaults();
    }

    if (stk_config->time2Ticks(m_flag_deactivated_time) > 2047 ||
        m_flag_deactivated_time < 0.0f)
    {
        float timeout = m_flag_deactivated_time;
        // in CTFFlag it uses 11bit unsigned integer for timeout
        Log::warn("ServerConfig", "Invalid %f m_flag_return_timemout which "
            "is invalid, use default value.", timeout);
        m_flag_deactivated_time.revertToDefaults();
    }

    int frequency_in_config = m_state_frequency;
    if (frequency_in_config <= 0 ||
        frequency_in_config > stk_config->getPhysicsFPS())
    {
        Log::warn("ServerConfig", "Invalid %d state frequency which is larger "
            "than physics FPS %d, use default value.",
            frequency_in_config, stk_config->getPhysicsFPS());
        m_state_frequency.revertToDefaults();
    }
    NetworkConfig::get()->setStateFrequency(m_state_frequency);

    if (m_player_reports_expired_days < 0.0f)
        m_player_reports_expired_days.revertToDefaults();
    if (m_server_difficulty > RaceManager::DIFFICULTY_LAST)
        m_server_difficulty = RaceManager::DIFFICULTY_LAST;
    if (m_server_mode > 8)
        m_server_mode = 3;

    if (m_official_karts_threshold > 1.0f)
        m_official_karts_threshold = 1.0f;
    if (m_official_tracks_threshold > 1.0f)
        m_official_tracks_threshold = 1.0f;

    if (m_live_players)
        m_official_karts_threshold = 1.0f;
    if (m_high_ping_workaround)
        m_kick_high_ping_players = false;
    auto modes = getLocalGameModeFromConfig();
    RaceManager::get()->setMinorMode(modes.first);
    RaceManager::get()->setMajorMode(modes.second);
    unsigned difficulty = m_server_difficulty;
    RaceManager::get()->setDifficulty(RaceManager::Difficulty(difficulty));

    if (RaceManager::get()->getMinorMode() == RaceManager::MINOR_MODE_FREE_FOR_ALL &&
        m_server_max_players > 10)
        m_server_max_players = 10;

    m_max_players_in_game = (m_max_players_in_game <= 0) ? m_server_max_players :
        std::min(m_max_players_in_game, m_server_max_players);

    if (m_ipv6_connection)
    {
#ifndef ENABLE_IPV6
        Log::warn("ServerConfig", "IPv6 support not compiled.");
#endif
    }

    if (m_ranked)
    {
        m_validating_player = true;
        m_auto_end = true;
        m_owner_less = true;
        m_strict_players = true;
    }
    if (m_owner_less)
    {
        if (m_min_start_game_players > m_max_players_in_game)
            m_min_start_game_players = 1;
        if (!m_live_players)
            m_team_choosing = false;
        m_server_configurable = false;
    }
    if (modes.second == RaceManager::MAJOR_MODE_GRAND_PRIX)
        m_server_configurable = false;

    const bool is_soccer =
        RaceManager::get()->getMinorMode() == RaceManager::MINOR_MODE_SOCCER;
    const bool is_gp =
        RaceManager::get()->getMajorMode() == RaceManager::MAJOR_MODE_GRAND_PRIX;
    const bool is_battle = RaceManager::get()->isBattleMode();

    std::shared_ptr<LobbyProtocol> server_lobby;
    server_lobby = STKHost::create();

    if (is_soccer)
    {
        server_lobby->getGameSetup()
            ->setSoccerGoalTarget(m_soccer_goal_target);
    }
    else if (is_gp)
    {
        server_lobby->getGameSetup()->setGrandPrixTrack(m_gp_track_count);
    }
    else if (is_battle)
    {
        if (m_hit_limit <= 0 && m_time_limit_ffa <= 0)
        {
            Log::warn("main", "Reset invalid hit and time limit settings");
            m_hit_limit.revertToDefaults();
            m_time_limit_ffa.revertToDefaults();
        }
        if (m_capture_limit <= 0 && m_time_limit_ctf <= 0)
        {
            Log::warn("main", "Reset invalid Capture and time limit settings");
            m_capture_limit.revertToDefaults();
            m_time_limit_ctf.revertToDefaults();
        }
    }

    // The extra server info has to be set before the server lobby is started
    if (server_lobby)
        server_lobby->requestStart();
}   // loadServerLobbyFromConfig

// ----------------------------------------------------------------------------
std::string getConfigDirectory()
{
    return StringUtils::getPath(g_server_config_path);
}   // getConfigDirectory

}

