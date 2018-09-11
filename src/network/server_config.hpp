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

#ifndef SERVER_CFG_PREFIX
#define SERVER_CFG_PREFIX extern
#endif

#ifndef SERVER_CFG_DEFAULT
#define SERVER_CFG_DEFAULT(X)
#endif

#include <string>
#include <vector>

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
    template<typename T, typename U>
    class MapServerConfigParam : public MapUserConfigParam<T, U>
    {
    private:
        using MapUserConfigParam<T, U>::m_elements;
    public:
        MapServerConfigParam(const char* param_name, const char* comment,
                             std::map<T, U> default_value);
        using MapUserConfigParam<T, U>::operator=;
    };
    // ========================================================================
    typedef MapServerConfigParam<uint32_t, uint32_t> UIntToUIntServerConfigParam;
    typedef MapServerConfigParam<std::string, uint32_t>
        StringToUIntServerConfigParam;
    // ========================================================================
    void loadServerConfig(const std::string& path = "");
    void writeServerConfig();

    SERVER_CFG_PREFIX FloatServerConfigParam m_voting_timeout
        SERVER_CFG_DEFAULT(FloatServerConfigParam(20.0f, "voting-timeout",
        "Timeout in seconds for voting tracks in server."));

    SERVER_CFG_PREFIX FloatServerConfigParam m_validation_timeout
        SERVER_CFG_DEFAULT(FloatServerConfigParam(20.0f, "validation-timeout",
        "Timeout in seconds for validation of clients."));

    SERVER_CFG_PREFIX IntServerConfigParam m_server_max_players
        SERVER_CFG_DEFAULT(IntServerConfigParam(8, "server-max-players",
        "Maximum number of players on the server, setting it more than "
        "8 will have performance degradation."));

    SERVER_CFG_PREFIX BoolServerConfigParam m_firewalled_server
        SERVER_CFG_DEFAULT(BoolServerConfigParam(true, "firewalled-server",
        "Disable it to turn off all stun related code in server, "
        "it allows saving server resource if your server is not "
        "behind a firewall."));

    SERVER_CFG_PREFIX FloatServerConfigParam m_start_game_counter
        SERVER_CFG_DEFAULT(FloatServerConfigParam(30.0f, "start-game-counter",
        "Time to wait before entering kart selection screen "
        "if satisfied start-game-threshold below for owner less or ranked "
        "server."));

    SERVER_CFG_PREFIX FloatServerConfigParam m_start_game_threshold
        SERVER_CFG_DEFAULT(FloatServerConfigParam(0.5f, "start-game-threshold",
        "Only auto start kart selection when number of "
        "connected player is larger than max player * this value, for "
        "owner less or ranked server, after start-game-counter."));

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
        "LHS: IP in X.X.X.X/Y (CIDR) format, use Y of 32 for a specific ip, "
        "RHS: time epoch to expire, "
        "if -1 (uint32_t max) than a permanent ban.",
        { { "0.0.0.0/0", 0u } }));

    SERVER_CFG_PREFIX UIntToUIntServerConfigParam m_server_online_id_ban_list
        SERVER_CFG_DEFAULT(UIntToUIntServerConfigParam(
        "server-online-id-ban-list",
        "LHS: online id, RHS: time epoch to expire, "
        "if -1 (uint32_t max) than a permanent ban.",
        { { 0u, 0u } }));

};   // namespace ServerConfig

#endif // HEADER_SERVER_CONFIG
