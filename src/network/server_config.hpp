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

#ifndef HEADER_SERVER_CONFIG
#define HEADER_SERVER_CONFIG

#include "config/user_config.hpp"
#include "race/race_manager.hpp"

#ifndef SERVER_CFG_PREFIX
#define SERVER_CFG_PREFIX extern
#endif

#ifndef SERVER_CFG_DEFAULT
#define SERVER_CFG_DEFAULT(X)
#endif

#include <string>
#include <vector>

class XMLNode;

namespace ServerConfig
{
    // ========================================================================
    class FloatServerConfigParam : public FloatUserConfigParam
    {
    public:
        FloatServerConfigParam(float default_value, const char* param_name,
                               const char* comment);
        using FloatUserConfigParam::operator=;
    };
    // ========================================================================
    class IntServerConfigParam : public IntUserConfigParam
    {
    public:
        IntServerConfigParam(int default_value, const char* param_name,
                             const char* comment);
        using IntUserConfigParam::operator=;
    };
    // ========================================================================
    class BoolServerConfigParam : public BoolUserConfigParam
    {
    public:
        BoolServerConfigParam(bool default_value, const char* param_name,
                              const char* comment);
        using BoolUserConfigParam::operator=;
    };
    // ========================================================================
    class StringServerConfigParam : public StringUserConfigParam
    {
    public:
        StringServerConfigParam(std::string default_value,
                                const char* param_name, const char* comment);
        using StringUserConfigParam::operator=;
    };
    // ========================================================================
    template<typename T, typename U>
    class MapServerConfigParam : public MapUserConfigParam<T, U>
    {
    private:
        using MapUserConfigParam<T, U>::m_key_names;
        using MapUserConfigParam<T, U>::m_elements;
    public:
        MapServerConfigParam(const char* param_name, const char* comment,
                             std::array<std::string, 3> key_names,
                             std::map<T, U> default_value);
        using MapUserConfigParam<T, U>::operator=;
    };
    // ========================================================================
    typedef MapServerConfigParam<uint32_t, uint32_t> UIntToUIntServerConfigParam;
    typedef MapServerConfigParam<std::string, uint32_t>
        StringToUIntServerConfigParam;
    // ========================================================================
    SERVER_CFG_PREFIX StringServerConfigParam m_server_name
        SERVER_CFG_DEFAULT(StringServerConfigParam("stk server", "server-name",
        "Name of server, encode in XML if you want to use unicode "
        "characters."));

    SERVER_CFG_PREFIX IntServerConfigParam m_server_port
        SERVER_CFG_DEFAULT(IntServerConfigParam(0, "server-port",
        "Port used in server, if you specify 0, it will use the server port "
        "specified in stk_config.xml or if random-server-port is enabled "
        "in user config, than any port. STK will auto change to random "
        "port if the port you specify failed to be bound."));

    SERVER_CFG_PREFIX IntServerConfigParam m_server_mode
        SERVER_CFG_DEFAULT(IntServerConfigParam(3, "server-mode",
        "Game mode in server, 0 is normal race (grand prix), "
        "1 is time trial (grand prix), 3 is normal race, "
        "4 time trial, 6 is soccer, 7 is free-for-all and "
        "8 is capture the flag. Notice: grand prix server doesn't "
        "allow for players to join and wait for ongoing game."));

    SERVER_CFG_PREFIX IntServerConfigParam m_server_difficulty
        SERVER_CFG_DEFAULT(IntServerConfigParam(0, "server-difficulty",
        "Difficulty in server, 0 is beginner, 1 is intermediate, 2 is expert "
        "and 3 is supertux (the most difficult)."));

    SERVER_CFG_PREFIX IntServerConfigParam m_gp_track_count
        SERVER_CFG_DEFAULT(IntServerConfigParam(3, "gp-track-count",
        "Number of grand prix tracks per game (If grand prix enabled)."));

    SERVER_CFG_PREFIX BoolServerConfigParam m_soccer_goal_target
        SERVER_CFG_DEFAULT(BoolServerConfigParam(false, "soccer-goal-target",
        "Use goal target in soccer."));

    SERVER_CFG_PREFIX BoolServerConfigParam m_wan_server
        SERVER_CFG_DEFAULT(BoolServerConfigParam(true, "wan-server",
        "Enable wan server, which requires you to have an stk-addons account "
        "with a saved session. Check init-user command for details."));

    SERVER_CFG_PREFIX BoolServerConfigParam m_enable_console
        SERVER_CFG_DEFAULT(BoolServerConfigParam(false, "enable-console",
        "Enable network console, which can do for example kickban."));

    SERVER_CFG_PREFIX IntServerConfigParam m_server_max_players
        SERVER_CFG_DEFAULT(IntServerConfigParam(8, "server-max-players",
        "Maximum number of players on the server, setting it more than "
        "8 will have performance degradation."));

    SERVER_CFG_PREFIX StringServerConfigParam m_private_server_password
        SERVER_CFG_DEFAULT(StringServerConfigParam("",
        "private-server-password", "Password for private server, "
        "empty for a public server."));

    SERVER_CFG_PREFIX StringServerConfigParam m_motd
        SERVER_CFG_DEFAULT(StringServerConfigParam("",
        "motd", "Message of today shown in lobby, you can enter encoded XML "
        "words here or a file.txt and let STK load it."));

    SERVER_CFG_PREFIX FloatServerConfigParam m_voting_timeout
        SERVER_CFG_DEFAULT(FloatServerConfigParam(20.0f, "voting-timeout",
        "Timeout in seconds for voting tracks in server."));

    SERVER_CFG_PREFIX FloatServerConfigParam m_validation_timeout
        SERVER_CFG_DEFAULT(FloatServerConfigParam(20.0f, "validation-timeout",
        "Timeout in seconds for validation of clients in wan, currently "
        "stk will use the stk-addons server to share AES key between client "
        "and server."));

    SERVER_CFG_PREFIX BoolServerConfigParam m_validating_player
        SERVER_CFG_DEFAULT(BoolServerConfigParam(true, "validating-player",
        "By default WAN server will always validate player and LAN will not, "
        "disable it to allow non-validated player in WAN."));

    SERVER_CFG_PREFIX BoolServerConfigParam m_firewalled_server
        SERVER_CFG_DEFAULT(BoolServerConfigParam(true, "firewalled-server",
        "Disable it to turn off all stun related code in server, "
        "it allows saving server resource if your server is not "
        "behind a firewall."));

    SERVER_CFG_PREFIX BoolServerConfigParam m_owner_less
        SERVER_CFG_DEFAULT(BoolServerConfigParam(false, "owner-less",
        "No server owner in lobby which can control the starting of game or "
        "kick any players."));

    SERVER_CFG_PREFIX FloatServerConfigParam m_start_game_counter
        SERVER_CFG_DEFAULT(FloatServerConfigParam(30.0f, "start-game-counter",
        "Time to wait before entering kart selection screen "
        "if satisfied min-start-game-players below for owner less or ranked "
        "server."));

    SERVER_CFG_PREFIX IntServerConfigParam m_min_start_game_players
        SERVER_CFG_DEFAULT(IntServerConfigParam(2, "min-start-game-players",
        "Only auto start kart selection when number of "
        "connected player is larger than or equals this value, for "
        "owner less or ranked server, after start-game-counter reaches 0."));

    SERVER_CFG_PREFIX BoolServerConfigParam m_auto_end
        SERVER_CFG_DEFAULT(BoolServerConfigParam(false, "auto-end",
        "Automatically end linear race game after 1st player finished "
        "for some time (currently his finished time * 0.25 + 15.0)."));

    SERVER_CFG_PREFIX BoolServerConfigParam m_team_choosing
        SERVER_CFG_DEFAULT(BoolServerConfigParam(true, "team-choosing",
        "Enable team choosing in lobby in team game (soccer and CTF). "
        "If owner-less is enabled, than this option is always disabled."));

    SERVER_CFG_PREFIX BoolServerConfigParam m_ranked
        SERVER_CFG_DEFAULT(BoolServerConfigParam(false, "ranked",
        "Server will submit ranking to stk addons server "
        "for linear race games, you require permission for that. "
        "validating-player, auto-end and owner-less will be turned on."));

    SERVER_CFG_PREFIX FloatServerConfigParam m_flag_return_timemout
        SERVER_CFG_DEFAULT(FloatServerConfigParam(20.0f, "flag-return-timemout",
        "Time in seconds when a flag is dropped a by player in CTF "
        "returning to its own base."));

    SERVER_CFG_PREFIX FloatServerConfigParam m_hit_limit_threshold
        SERVER_CFG_DEFAULT(FloatServerConfigParam(5.0f, "hit-limit-threshold",
        "Value used to calculate hit limit in free for all, which "
        "is min(number of players * hit-limit-threshold, 40), "
        "negative value to disable hit limit."));

    SERVER_CFG_PREFIX FloatServerConfigParam m_time_limit_threshold_ffa
        SERVER_CFG_DEFAULT(FloatServerConfigParam(0.7f,
        "time-limit-threshold-ffa",
        "Value used to calculate time limit in free for all, which "
        "is max(number of players * time-limit-threshold-ffa, 3.0) * 60, "
        "negative value to disable time limit."));

    SERVER_CFG_PREFIX FloatServerConfigParam m_capture_limit_threshold
        SERVER_CFG_DEFAULT(FloatServerConfigParam(0.7f,
        "capture-limit-threshold",
        "Value used to calculate capture limit in CTF, which "
        "is max(3.0, number of players * capture-limit-threshold), "
        "negative value to disable capture limit."));

    SERVER_CFG_PREFIX FloatServerConfigParam m_time_limit_threshold_ctf
        SERVER_CFG_DEFAULT(FloatServerConfigParam(0.9f,
        "time-limit-threshold-ctf",
        "Value used to calculate time limit in CTF, which "
        "is max(3.0, number of players * "
        "(time-limit-threshold-ctf + flag-return-timemout / 60.0)) * 60.0,"
        " negative value to disable time limit."));

    SERVER_CFG_PREFIX FloatServerConfigParam m_auto_lap_ratio
        SERVER_CFG_DEFAULT(FloatServerConfigParam(-1.0f, "auto-lap-ratio",
        "Value used by server to automatically calculate "
        "lap of each race in network game, if more than 0.0f, the number of "
        "lap of each track vote in linear race will be determined by "
        "max(1.0f, auto-lap-ratio * default lap of that track)."));

    SERVER_CFG_PREFIX IntServerConfigParam m_max_ping
        SERVER_CFG_DEFAULT(IntServerConfigParam(300, "max-ping",
        "Maximum ping allowed for a player (in ms)."));

    SERVER_CFG_PREFIX IntServerConfigParam m_jitter_tolerance
        SERVER_CFG_DEFAULT(IntServerConfigParam(100, "jitter-tolerance",
        "Tolerance of jitter in network allowed (in ms)."));

    SERVER_CFG_PREFIX BoolServerConfigParam m_kick_high_ping_players
        SERVER_CFG_DEFAULT(BoolServerConfigParam(false,
        "kick-high-ping-players",
        "Kick players whose ping is above max-ping."));

    SERVER_CFG_PREFIX StringToUIntServerConfigParam m_server_ip_ban_list
        SERVER_CFG_DEFAULT(StringToUIntServerConfigParam("server-ip-ban-list",
        "ip: IP in X.X.X.X/Y (CIDR) format for banning, use Y of 32 for a "
        "specific ip, expired-time: unix timestamp to expire, "
        "if -1 (uint32_t max) than a permanent ban.",
        {{ "ban", "ip", "expired-time" }},
        { { "0.0.0.0/0", 0u } }));

    SERVER_CFG_PREFIX UIntToUIntServerConfigParam m_server_online_id_ban_list
        SERVER_CFG_DEFAULT(UIntToUIntServerConfigParam(
        "server-online-id-ban-list",
        "online-id: online id for banning, expired-time: unix timestamp to "
        "expire, if -1 (uint32_t max) than a permanent ban.",
        {{ "ban", "online-id", "expired-time" }},
        { { 0u, 0u } }));

    // ========================================================================
    /** Server version, will be advanced if there are protocol changes. */
    static const uint32_t m_server_version = 1;
    // ========================================================================
    void loadServerConfig(const std::string& path = "");
    // ------------------------------------------------------------------------
    void loadServerConfigXML(const XMLNode* root);
    // ------------------------------------------------------------------------
    std::string getServerConfigXML();
    // ------------------------------------------------------------------------
    void writeServerConfigToDisk();
    // ------------------------------------------------------------------------
    std::pair<RaceManager::MinorRaceModeType, RaceManager::MajorRaceModeType>
        getLocalGameMode();
    // ------------------------------------------------------------------------
    core::stringw getModeName(unsigned id);
    // ------------------------------------------------------------------------
    inline bool unsupportedGameMode()
                           { return m_server_mode == 2 || m_server_mode == 5; }
    // ------------------------------------------------------------------------
    void loadServerLobbyFromConfig();
    // ------------------------------------------------------------------------
    std::string getConfigDirectory();

};   // namespace ServerConfig

#endif // HEADER_SERVER_CONFIG
