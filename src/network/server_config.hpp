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
        SERVER_CFG_DEFAULT(StringServerConfigParam("STK Server", "server-name",
        "Name of server, encode in XML if you want to use unicode "
        "characters."));

        // ----Server Config Variables
    SERVER_CFG_PREFIX BoolServerConfigParam m_supertournament
        SERVER_CFG_DEFAULT(BoolServerConfigParam(false, "supertournament",
                    "supertournament"));
    SERVER_CFG_PREFIX StringServerConfigParam m_gameplan_path
        SERVER_CFG_DEFAULT(StringServerConfigParam(
                    "supertournament_gameplan.txt",
                    "gameplan-file", "Path to the gameplan file. "
                    "Defines amount of games per match, game duration, "
                    "selectable fields or forces a certain field."));
    SERVER_CFG_PREFIX StringServerConfigParam m_tourn_required_fields_path
        SERVER_CFG_DEFAULT(StringServerConfigParam(
                    "supertournament_required_fields.txt",
                    "tourn-required-fields-path",
                    "Path to the file that contains required fields. "
                    "Players without said fields won't be"
                    " able to play the game."));
    SERVER_CFG_PREFIX StringServerConfigParam m_tourn_semi_required_fields_path
        SERVER_CFG_DEFAULT(StringServerConfigParam(
                    "supertournament_semi_required_fields.txt",
                    "tourn-semi-required-fields-path",
                    "Path to the file that contains required fields. "
                    "Players that have less than their amount minus the "
                    "'tourn-semi-required-fields-minus'"
                    " won't be"
                    " able to play the game."));
    SERVER_CFG_PREFIX IntServerConfigParam m_tourn_semi_required_fields_minus
        SERVER_CFG_DEFAULT(IntServerConfigParam(1,
                    "tourn-semi-required-fields-minus",
                    "Amount of fields that could be forgiven "
                    "for not being installed in semi-required fields."));
    SERVER_CFG_PREFIX StringServerConfigParam m_matchplan_path
        SERVER_CFG_DEFAULT(StringServerConfigParam(
                    "supertournament_matchplan.txt",
                    "matchplan-file", "Path to the matchplan file. "
                    "Defines scheduled matches and their results."));
    SERVER_CFG_PREFIX BoolServerConfigParam m_update_matchplan
        SERVER_CFG_DEFAULT(BoolServerConfigParam(false, "update-matchplan",
                    "If set to true, matchplan will be updated"
                    " once the game ends."));
    SERVER_CFG_PREFIX StringServerConfigParam m_teams_path
        SERVER_CFG_DEFAULT(StringServerConfigParam("supertournament_teams.txt",
                    "teams-file", "Path to the teams file."));
    SERVER_CFG_PREFIX StringServerConfigParam m_tourn_log
        SERVER_CFG_DEFAULT(StringServerConfigParam("supertournament_log.txt", 
                    "tourn-log-file",
                    "File where to log the supertournament game results."));
    SERVER_CFG_PREFIX StringServerConfigParam m_red_team
        SERVER_CFG_DEFAULT(StringServerConfigParam(
                    "", "red-team", "Tournament team of red players."));
    SERVER_CFG_PREFIX StringServerConfigParam m_blue_team
        SERVER_CFG_DEFAULT(StringServerConfigParam(
                    "", "blue-team", "Tournament team of blue players."));
    SERVER_CFG_PREFIX IntServerConfigParam m_fixed_lap_count
        SERVER_CFG_DEFAULT(IntServerConfigParam(
                    -1, "fixed-lap-count","fixed-lap-count"));


    SERVER_CFG_PREFIX IntServerConfigParam m_server_port
        SERVER_CFG_DEFAULT(IntServerConfigParam(0, "server-port",
        "Port used in server, if you specify 0, it will use the server port "
        "specified in stk_config.xml. If you wish to use a random port, "
        "set random-server-port to '1' in user config. STK will automatically "
        "switch to a random port if the port you specify fails to be bound."));

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
    
    SERVER_CFG_PREFIX BoolServerConfigParam m_allow_powerupper
        SERVER_CFG_DEFAULT(BoolServerConfigParam(false, "allow-powerupper",
        "Allow powerupper."));

    SERVER_CFG_PREFIX BoolServerConfigParam m_tiers_roulette
        SERVER_CFG_DEFAULT(BoolServerConfigParam(false, "tiers-roulette",
        "Enable roulette between features. This will rotate after every finished game."));

    SERVER_CFG_PREFIX StringServerConfigParam m_tiers_roulette_sequence
        SERVER_CFG_DEFAULT(StringServerConfigParam(
        "kartheavy+itemchaos,kartmedium+itemchaos,kartlight+itemchaos,bowlparty,cakeparty",
        "tiers-roulette-sequence",
        "When TierS roulette is enabled, this is the sequence of the features being rotated. "
        "Separate modifiers with commas: kartheavy,kartmedium,none,... "
        "Combine modifiers with a plus sign: kartheavy+itemcakeparty,... "
        "Specify none for the iteration to be all default (without any modifiers): none,none,... "
        "THIS OVERRIDES THE FALSE VALUE OF THE allow-*party, which only controls the command "
        "invocation."));

    SERVER_CFG_PREFIX BoolServerConfigParam m_show_elo
        SERVER_CFG_DEFAULT(BoolServerConfigParam(false, "show-elo",
        "Show the elo after the username in the lobby."));

    SERVER_CFG_PREFIX BoolServerConfigParam m_show_rank
        SERVER_CFG_DEFAULT(BoolServerConfigParam(false, "show-rank",
        "Show the rank before the username in the lobby."));

    SERVER_CFG_PREFIX BoolServerConfigParam m_allow_heavyparty
        SERVER_CFG_DEFAULT(BoolServerConfigParam(false, "allow-heavyparty",
        "Allow heavy party."));

    SERVER_CFG_PREFIX BoolServerConfigParam m_allow_mediumparty
        SERVER_CFG_DEFAULT(BoolServerConfigParam(false, "allow-mediumparty",
        "Allow medium party."));

    SERVER_CFG_PREFIX BoolServerConfigParam m_allow_lightparty
        SERVER_CFG_DEFAULT(BoolServerConfigParam(false, "allow-lightparty",
        "Allow light party."));

    SERVER_CFG_PREFIX BoolServerConfigParam m_allow_itemchaos
        SERVER_CFG_DEFAULT(BoolServerConfigParam(false, "allow-itemchaos",
        "Allow item chaos. This will set random powerup (except for basketball, anvil, parachute and plunger) "
        "for all the karts in the world during game every one minute as well as filling up the nitro to 100.0"));

    SERVER_CFG_PREFIX BoolServerConfigParam m_allow_bowlparty
        SERVER_CFG_DEFAULT(BoolServerConfigParam(false, "allow-bowlparty",
        "Allow bowling party command."));

    SERVER_CFG_PREFIX BoolServerConfigParam m_allow_cakeparty
        SERVER_CFG_DEFAULT(BoolServerConfigParam(false, "allow-cakeparty",
        "Allow cake party command."));

    SERVER_CFG_PREFIX BoolServerConfigParam m_allow_plungerparty
        SERVER_CFG_DEFAULT(BoolServerConfigParam(false, "allow-plungerparty",
        "Allow plunger party command."));

    SERVER_CFG_PREFIX BoolServerConfigParam m_allow_zipperparty
        SERVER_CFG_DEFAULT(BoolServerConfigParam(false, "allow-zipperparty",
        "Allow zipper party command."));

    SERVER_CFG_PREFIX BoolServerConfigParam m_allow_pole
        SERVER_CFG_DEFAULT(BoolServerConfigParam(false, "allow-pole",
        "Allow pole. Players can team vote which teammate gets the leading start position."));

    SERVER_CFG_PREFIX BoolServerConfigParam m_enable_ril
        SERVER_CFG_DEFAULT(BoolServerConfigParam(false, "enable-ril",
        "When a player enters server lobby, show random /installaddon command line according to the current server mode (default is false)."));

    SERVER_CFG_PREFIX StringServerConfigParam m_ril_prefix
        SERVER_CFG_DEFAULT(StringServerConfigParam("/installaddon", "ril-prefix",
        "If random-installaddon-lines is configured, specifies the prefix before the addon name."));

    SERVER_CFG_PREFIX StringServerConfigParam m_game_start_message
        SERVER_CFG_DEFAULT(StringServerConfigParam("Have Fun", "game-start-message",
        "This message is shown when the game is started. Defaults to \"Have Fun\", if empty, this function is disabled."));

    SERVER_CFG_PREFIX BoolServerConfigParam  m_soccer_log
        SERVER_CFG_DEFAULT(BoolServerConfigParam(false, "soccer-log","Soccer Log (true or false.)"));

    SERVER_CFG_PREFIX StringServerConfigParam m_soccer_log_path
        SERVER_CFG_DEFAULT(StringServerConfigParam("soccer_log.txt", "soccer-log-path", "Directory where the soccer log should be written to with / at the end."));
    
    SERVER_CFG_PREFIX StringServerConfigParam m_live_soccer_log_path
        SERVER_CFG_DEFAULT(StringServerConfigParam("soccer_match.log", "live-soccer-log-path", "File path to the live soccer log."));

    SERVER_CFG_PREFIX BoolServerConfigParam m_wan_server
        SERVER_CFG_DEFAULT(BoolServerConfigParam(true, "wan-server",
        "Enable wan server, which requires you to have an stk-addons account "
        "with a saved session. Check init-user command for details."));

    SERVER_CFG_PREFIX BoolServerConfigParam m_enable_console
        SERVER_CFG_DEFAULT(BoolServerConfigParam(false, "enable-console",
        "Enable network console, which can do for example kickban."));

    SERVER_CFG_PREFIX IntServerConfigParam m_server_max_players
        SERVER_CFG_DEFAULT(IntServerConfigParam(8, "server-max-players",
        "Maximum number of players on the server, setting this to a value "
        "greater than 8 can cause performance degradation."));

    SERVER_CFG_PREFIX IntServerConfigParam m_max_players_in_game
        SERVER_CFG_DEFAULT(IntServerConfigParam(0, "max-players-in-game",
        "Maximum number of players in the game, all other players on "
        "the server are spectators. Specify 0 to allow all players on "
        "the server to play."));

    SERVER_CFG_PREFIX IntServerConfigParam m_slots_min
        SERVER_CFG_DEFAULT(IntServerConfigParam(2, "slots-min",
        "Bottom limit of the /slots command argument that can be voted for."));

    SERVER_CFG_PREFIX IntServerConfigParam m_slots_max
        SERVER_CFG_DEFAULT(IntServerConfigParam(12, "slots-max",
        "Top limit of the /slots command argument that can be voted for."));
    
    SERVER_CFG_PREFIX BoolServerConfigParam m_command_voting
        SERVER_CFG_DEFAULT(BoolServerConfigParam(true, "command-voting",
        "Set true to enable command voting on the server."));

    SERVER_CFG_PREFIX StringServerConfigParam m_private_server_password
        SERVER_CFG_DEFAULT(StringServerConfigParam("",
        "private-server-password", "Password for private server, "
        "leave empty for a public server."));

    SERVER_CFG_PREFIX StringServerConfigParam m_motd
        SERVER_CFG_DEFAULT(StringServerConfigParam("",
        "motd", "Message of today shown in lobby, you can enter encoded XML "
        "words here or a file.txt and let STK load it."));

    SERVER_CFG_PREFIX StringServerConfigParam m_feature_filepath
        SERVER_CFG_DEFAULT(StringServerConfigParam("features.txt",
        "feature-filepath", "File to log the /inform (message) messages into from players."));

    SERVER_CFG_PREFIX StringServerConfigParam m_reports_filepath
        SERVER_CFG_DEFAULT(StringServerConfigParam("report.txt",
        "reports-filepath", "File to log the /report (message) messages into from players."));

    SERVER_CFG_PREFIX BoolServerConfigParam m_chat
        SERVER_CFG_DEFAULT(BoolServerConfigParam(true, "chat",
        "If this value is set to false, the server will ignore chat messages "
        "from all players."));

    SERVER_CFG_PREFIX BoolServerConfigParam m_global_chat
        SERVER_CFG_DEFAULT(BoolServerConfigParam(false, "global-chat",
        "If this is set to true, chat messages won't be separated from players in game and"
        " lobby."));

    SERVER_CFG_PREFIX IntServerConfigParam m_chat_consecutive_interval
        SERVER_CFG_DEFAULT(IntServerConfigParam(8, "chat-consecutive-interval",
        "If client sends more than chat-consecutive-interval / 2 chats within "
        "this value (read in seconds), it will be ignore, negative value to "
        "disable."));

    SERVER_CFG_PREFIX BoolServerConfigParam m_track_voting
        SERVER_CFG_DEFAULT(BoolServerConfigParam(true, "track-voting",
        "Allow players to vote for which track to play. If this value is set "
        "to false, the server will randomly pick the next track to play."));

    SERVER_CFG_PREFIX BoolServerConfigParam m_command_track_mode
        SERVER_CFG_DEFAULT(BoolServerConfigParam(false, "command-track-mode",
        "In order to play the next game, the player(s) need to use /settrack "
        "command, in which case the track voting screen is skipped "
        "(it will override track-voting parameter above). This will also "
        "lower the permission level for settrack command to PLAYER, plus "
        "the player being able to play the game (not being queued)."));

    SERVER_CFG_PREFIX BoolServerConfigParam m_command_kart_mode
        SERVER_CFG_DEFAULT(BoolServerConfigParam(false, "command-kart-mode",
        "In order to play the next game, the player(s) need to use /setkart "
        "command. "
        "This will also "
        "lower the permission level for setkart command to PLAYER, plus "
        "the player being able to play the game (not being queued)."));

    SERVER_CFG_PREFIX FloatServerConfigParam m_voting_timeout
        SERVER_CFG_DEFAULT(FloatServerConfigParam(30.0f, "voting-timeout",
        "Timeout in seconds for selecting karts and (or) voting tracks in "
        "server, you may want to use a lower value if you have track-voting "
        "off."));

    SERVER_CFG_PREFIX FloatServerConfigParam m_validation_timeout
        SERVER_CFG_DEFAULT(FloatServerConfigParam(20.0f, "validation-timeout",
        "Timeout in seconds for validation of clients in wan, currently "
        "STK will use the stk-addons server to share AES key between the client "
        "and server."));

    SERVER_CFG_PREFIX FloatServerConfigParam m_check_servers_cooldown
        SERVER_CFG_DEFAULT(FloatServerConfigParam(10.0f, "check-servers-cooldown",
        "Global cooldown for the /check-servers command. Set to 0 to disable the command."));

    SERVER_CFG_PREFIX BoolServerConfigParam m_validating_player
        SERVER_CFG_DEFAULT(BoolServerConfigParam(true, "validating-player",
        "By default WAN server will always validate player and LAN will not, "
        "disable it to allow non-validated player in WAN."));

    SERVER_CFG_PREFIX BoolServerConfigParam m_firewalled_server
        SERVER_CFG_DEFAULT(BoolServerConfigParam(true, "firewalled-server",
        "Disable it to turn off all stun related code in server, "
        "it allows for saving of server resources if your server is not "
        "behind a firewall."));

    SERVER_CFG_PREFIX BoolServerConfigParam m_ipv6_connection
        SERVER_CFG_DEFAULT(BoolServerConfigParam(true, "ipv6-connection",
        "Enable to allow IPv6 connection if you have a public IPv6 address. "
        "STK currently uses dual-stack mode which requires server to have both "
        "IPv4 and IPv6 and listen to same port. If STK detects your server "
        "has no public IPv6 address or port differs between IPv4 and IPv6 "
        "then it will use IPv4 only socket. For system which doesn't support "
        "dual-stack socket (like OpenBSD) you may fail to be connected by "
        "IPv4 clients. You can override the detection in config.xml at "
        "supertuxkart config-0.10 folder, with default-ip-type option."));

    SERVER_CFG_PREFIX BoolServerConfigParam m_owner_less
        SERVER_CFG_DEFAULT(BoolServerConfigParam(false, "owner-less",
        "No server owner in lobby which can control the starting of game or "
        "kick any players."));

    SERVER_CFG_PREFIX FloatServerConfigParam m_start_game_counter
        SERVER_CFG_DEFAULT(FloatServerConfigParam(60.0f, "start-game-counter",
        "Time to wait before entering kart selection screen "
        "if satisfied min-start-game-players below for owner less or ranked "
        "server."));

    SERVER_CFG_PREFIX FloatServerConfigParam m_official_karts_threshold
        SERVER_CFG_DEFAULT(FloatServerConfigParam(1.0f,
        "official-karts-threshold",
        "Clients below this value will be rejected from joining this server. "
        "It's determined by number of official karts in client / number of "
        "official karts in server"));

    SERVER_CFG_PREFIX FloatServerConfigParam m_official_tracks_threshold
        SERVER_CFG_DEFAULT(FloatServerConfigParam(0.7f,
        "official-tracks-threshold",
        "Clients below this value will be rejected from joining this server. "
        "It's determined by number of official tracks in client / number of "
        "official tracks in server, setting this value too high will prevent "
        "android players from joining this server, because STK android apk "
        "has some official tracks removed."));

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
        "If owner-less is enabled and live-spectate is not enabled, than this "
        "option is always disabled."));

    SERVER_CFG_PREFIX BoolServerConfigParam m_strict_players
        SERVER_CFG_DEFAULT(BoolServerConfigParam(false, "strict-players",
        "If strict-players is on, no duplicated online id or split screen "
        "players are allowed, which can prevent someone using more than 1 "
        "network AI with this server."));

    SERVER_CFG_PREFIX BoolServerConfigParam m_ranked
        SERVER_CFG_DEFAULT(BoolServerConfigParam(false, "ranked",
        "Server will submit ranking to stk-addons server "
        "for linear race games, you require permission for that. "
        "validating-player, auto-end, strict-player and owner-less will be "
        "turned on."));

    SERVER_CFG_PREFIX BoolServerConfigParam m_cheats
        SERVER_CFG_DEFAULT(BoolServerConfigParam(false, "cheats",
        "Allow cheat commands for non-admins like /item and /nitro."));

    SERVER_CFG_PREFIX BoolServerConfigParam m_infinite_game
        SERVER_CFG_DEFAULT(BoolServerConfigParam(false, "infinite-game",
        "Make the games last forever, until /end is done or when all players"
        " leave the game. For soccer it also acts the same as"
        " soccer-goal-target set to false, with infinite time."));

    SERVER_CFG_PREFIX StringServerConfigParam m_cheat_items
        SERVER_CFG_DEFAULT(StringServerConfigParam(
        "bowl:10,cake:10,zipper:10,plunger:10,switch:10,swatter:10,gum:10",
        "cheat-items", "Whitelist of the items:amount that can be get with "
        "/item command."));

    SERVER_CFG_PREFIX FloatServerConfigParam m_cheat_nitro
        SERVER_CFG_DEFAULT(FloatServerConfigParam(20.0, "cheat-nitro",
        "Amount of nitro that is received with /nitro command. Full nitro bar"
        " is 20.0 units."));

    SERVER_CFG_PREFIX StringServerConfigParam m_must_have_tracks
        SERVER_CFG_DEFAULT(StringServerConfigParam("",
        "must-have-tracks",
        "The tracks you must have."));

    SERVER_CFG_PREFIX StringServerConfigParam m_only_played_tracks
        SERVER_CFG_DEFAULT(StringServerConfigParam("",
        "only-played-tracks",
        "The only tracks that are played."));


    SERVER_CFG_PREFIX BoolServerConfigParam m_server_configurable
        SERVER_CFG_DEFAULT(BoolServerConfigParam(false, "server-configurable",
        "If true, the server owner can config the difficulty and game mode in "
        "the GUI of lobby. This option cannot be used with owner-less or "
        "grand prix server, and will be automatically turned on if the server "
        "was created using the in-game GUI. The changed difficulty and game "
        "mode will not be saved in this config file."));

    SERVER_CFG_PREFIX BoolServerConfigParam m_live_players
        SERVER_CFG_DEFAULT(BoolServerConfigParam(true, "live-spectate",
        "If true, players can live join or spectate the in-progress game. "
        "Currently live joining is only available if the current game mode "
        "used in server is FFA, CTF or soccer, also official-karts-threshold "
        "will be made 1.0. If false addon karts will use their original "
        "hitbox other than tux, all players having it restriction applies."));

    SERVER_CFG_PREFIX BoolServerConfigParam m_real_addon_karts
        SERVER_CFG_DEFAULT(BoolServerConfigParam(true, "real-addon-karts",
        "If true, server will send its addon karts real physics (kart size, "
        "length, type, etc) to client. If false or client chooses an addon "
        "kart which server is missing, tux's kart physics and kart type of "
        "the original addon is sent."));

    SERVER_CFG_PREFIX FloatServerConfigParam m_flag_return_timeout
        SERVER_CFG_DEFAULT(FloatServerConfigParam(20.0f, "flag-return-timeout",
        "Time in seconds when a flag is dropped a by player in CTF "
        "returning to its own base."));

    SERVER_CFG_PREFIX FloatServerConfigParam m_flag_deactivated_time
        SERVER_CFG_DEFAULT(FloatServerConfigParam(3.0f, "flag-deactivated-time",
        "Time in seconds to deactivate a flag when it's captured or returned "
        "to own base by players."));

    SERVER_CFG_PREFIX IntServerConfigParam m_hit_limit
        SERVER_CFG_DEFAULT(IntServerConfigParam(20, "hit-limit",
        "Hit limit of free for all, zero to disable hit limit."));

    SERVER_CFG_PREFIX IntServerConfigParam m_time_limit_ffa
        SERVER_CFG_DEFAULT(IntServerConfigParam(360,
        "time-limit-ffa", "Time limit of free for all in seconds, zero to "
        "disable time limit."));

    SERVER_CFG_PREFIX IntServerConfigParam m_capture_limit
        SERVER_CFG_DEFAULT(IntServerConfigParam(5, "capture-limit",
        "Capture limit of CTF, zero to disable capture limit."));

    SERVER_CFG_PREFIX IntServerConfigParam m_time_limit_ctf
        SERVER_CFG_DEFAULT(IntServerConfigParam(600, "time-limit-ctf",
        "Time limit of CTF in seconds, zero to disable time limit."));

    SERVER_CFG_PREFIX FloatServerConfigParam m_auto_game_time_ratio
        SERVER_CFG_DEFAULT(FloatServerConfigParam(-1.0f, "auto-game-time-ratio",
        "Value used by server to automatically estimate each game time. "
        "For races, it decides the lap of each race in network game, "
        "if more than 0.0f, the number of lap of each track vote in "
        "linear race will be determined by "
        "max(1.0f, auto-game-time-ratio * default lap of that track). "
        "For soccer if more than 0.0f, for time limit game it will be "
        "auto-game-time-ratio * soccer-time-limit in UserConfig, for goal "
        "limit game it will be auto-game-time-ratio * numgoals "
        "in UserConfig, -1 to disable for all."));

    SERVER_CFG_PREFIX IntServerConfigParam m_max_ping
        SERVER_CFG_DEFAULT(IntServerConfigParam(300, "max-ping",
        "Maximum ping allowed for a player (in ms), it's recommended to use "
        "default value if live-spectate is on."));

    SERVER_CFG_PREFIX IntServerConfigParam m_jitter_tolerance
        SERVER_CFG_DEFAULT(IntServerConfigParam(100, "jitter-tolerance",
        "Tolerance of jitter in network allowed (in ms), it's recommended to "
        "use default value if live-spectate is on."));

    SERVER_CFG_PREFIX BoolServerConfigParam m_kick_high_ping_players
        SERVER_CFG_DEFAULT(BoolServerConfigParam(false,
        "kick-high-ping-players",
        "Kick players whose ping is above max-ping."));

    SERVER_CFG_PREFIX BoolServerConfigParam m_high_ping_workaround
        SERVER_CFG_DEFAULT(BoolServerConfigParam(true,
        "high-ping-workaround",
        "Allow players exceeding max-ping to have a playable game, if enabled "
        "kick-high-ping-players will be disabled, please also use a default "
        "value for max-ping and jitter-tolerance with it."));

    SERVER_CFG_PREFIX IntServerConfigParam m_kick_idle_player_seconds
        SERVER_CFG_DEFAULT(IntServerConfigParam(60,
        "kick-idle-player-seconds",
        "Kick idle player which has no network activity to server for more "
        "than some seconds during game, unless he has finished the race. "
        "Negative value to disable, and this option will always be disabled "
        "for LAN server."));

    SERVER_CFG_PREFIX IntServerConfigParam m_state_frequency
        SERVER_CFG_DEFAULT(IntServerConfigParam(10,
        "state-frequency",
        "Set how many states the server will send per second, the higher this "
        "value, the more bandwidth requires, also each client will trigger "
        "more rewind, which clients with slow device may have problem playing "
        "this server, use the default value is recommended."));

    SERVER_CFG_PREFIX BoolServerConfigParam m_sql_management
        SERVER_CFG_DEFAULT(BoolServerConfigParam(false,
        "sql-management",
        "Use sql database for handling server stats and maintenance, STK "
        "needs to be compiled with sqlite3 supported."));

    SERVER_CFG_PREFIX StringServerConfigParam m_database_file
        SERVER_CFG_DEFAULT(StringServerConfigParam("stkservers.db",
        "database-file",
        "Database filename for sqlite to use, it can be shared for all "
        "servers created in this machine, and STK will create specific table "
        "for each server. You need to create the database yourself first, see "
        "NETWORKING.md for details"));

    SERVER_CFG_PREFIX IntServerConfigParam m_database_timeout
        SERVER_CFG_DEFAULT(IntServerConfigParam(1000,
        "database-timeout",
        "Specified in millisecond for maximum time waiting in "
        "sqlite3_busy_handler. You may need a higher value if your database "
        "is shared by many servers or having a slow hard disk."));

    SERVER_CFG_PREFIX StringServerConfigParam m_ip_ban_table
        SERVER_CFG_DEFAULT(StringServerConfigParam("ip_ban",
        "ip-ban-table",
        "IPv4 ban list table name, you need to create the table first, see "
        "NETWORKING.md for details, empty to disable. "
        "This table can be shared for all servers if you use the same name. "
        "STK can auto kick active peer from ban list (update per minute) which"
        "allows live kicking peer by inserting record to database."));

    SERVER_CFG_PREFIX StringServerConfigParam m_ipv6_ban_table
        SERVER_CFG_DEFAULT(StringServerConfigParam("ipv6_ban",
        "ipv6-ban-table",
        "IPv6 ban list table name, you need to create the table first, see "
        "NETWORKING.md for details, empty to disable. "
        "This table can be shared for all servers if you use the same name. "
        "STK can auto kick active peer from ban list (update per minute) "
        "which allows live kicking peer by inserting record to database."));

    SERVER_CFG_PREFIX StringServerConfigParam m_online_id_ban_table
        SERVER_CFG_DEFAULT(StringServerConfigParam("online_id_ban",
        "online-id-ban-table",
        "Online ID ban list table name, you need to create the table first, "
        "see NETWORKING.md for details, empty to disable. "
        "This table can be shared for all servers if you use the same name. "
        "STK can auto kick active peer from ban list (update per minute) "
        "which allows live kicking peer by inserting record to database."));

    SERVER_CFG_PREFIX StringServerConfigParam m_player_reports_table
        SERVER_CFG_DEFAULT(StringServerConfigParam("player_reports",
        "player-reports-table",
        "Player reports table name, which will be written when a player "
        "reports player in the network user dialog, you need to create the "
        "table first, see NETWORKING.md for details, empty to disable. "
        "This table can be shared for all servers if you use the same name."));

    SERVER_CFG_PREFIX FloatServerConfigParam m_player_reports_expired_days
        SERVER_CFG_DEFAULT(FloatServerConfigParam(3.0f,
        "player-reports-expired-days", "Days to keep player reports, "
        "older than that will be auto cleared, 0 to keep them forever."));

    SERVER_CFG_PREFIX StringServerConfigParam m_ip_geolocation_table
        SERVER_CFG_DEFAULT(StringServerConfigParam("ip_mapping",
        "ip-geolocation-table",
        "IP geolocation table, you only need this table if you want to "
        "geolocate IP from non-stk-addons connection, as all validated "
        "players connecting from stk-addons will provide the location info, "
        "you need to create the table first, see NETWORKING.md for details, "
        "empty to disable. "
        "This table can be shared for all servers if you use the same name."));

    SERVER_CFG_PREFIX StringServerConfigParam m_ipv6_geolocation_table
        SERVER_CFG_DEFAULT(StringServerConfigParam("ipv6_mapping",
        "ipv6-geolocation-table",
        "IPv6 geolocation table, you only need this table if you want to "
        "geolocate IP from non-stk-addons connection, as all validated "
        "players connecting from stk-addons will provide the location info, "
        "you need to create the table first, see NETWORKING.md for details, "
        "empty to disable. "
        "This table can be shared for all servers if you use the same name."));

    SERVER_CFG_PREFIX IntServerConfigParam m_server_owner
        SERVER_CFG_DEFAULT(IntServerConfigParam(-1, "server-owner",
        "Online ID that owns the server and has all permissions"
        ". (Works only when the player is validated.)"));

    SERVER_CFG_PREFIX StringServerConfigParam m_permissions_table
        SERVER_CFG_DEFAULT(StringServerConfigParam("permissions",
        "permissions-table",
        "Table used for defining staff players. "
        "Contains 2 fields: int online_id and int permission_level. "
        "Usually 0 is for regular player, 80 is for moderator, 100 is for "
        "administrator. Note that the server owner basically has infinite "
        "permission level."));

    SERVER_CFG_PREFIX StringServerConfigParam m_restrictions_table
        SERVER_CFG_DEFAULT(StringServerConfigParam("restrictions",
        "restrictions-table",
        "Table used for issuing restrictions to players. "
        "Contains 2 fields: int online_id and unsigned int restrictions. "
        "Second field has flags: NOSPEC, NOGAME, NOCHAT, NOPCHAT, NOTEAM, "
        "HANDICAP, KART, TRACK. In specified order, 1, 2, 4, 8, 16 etc. "));

    SERVER_CFG_PREFIX StringServerConfigParam m_permission_message
        SERVER_CFG_DEFAULT(StringServerConfigParam(
        "You are not allowed to run this command.",
        "permission-message",
        "Message to show to a player with insufficient permissions "
        "tries to use a restricted command."));

    SERVER_CFG_PREFIX BoolServerConfigParam m_shadow_nochat
        SERVER_CFG_DEFAULT(BoolServerConfigParam(false, "shadow-nochat",
        "Players restricted from chatting/DM won't be aware of it."));

    SERVER_CFG_PREFIX StringServerConfigParam m_soccer_ranking_file
        SERVER_CFG_DEFAULT(StringServerConfigParam(
        "soccer_ranking.txt",
        "soccer-ranking-file",
        "Path of the file to read from by /rank, /rank10 and /rank (player) "
        "commands, must be of a valid structure."));

    SERVER_CFG_PREFIX BoolServerConfigParam m_ai_handling
        SERVER_CFG_DEFAULT(BoolServerConfigParam(false, "ai-handling",
        "If true this server will auto add / remove AI connected with "
        "network-ai=x, which will kick N - 1 bot(s) where N is the number "
        "of human players. Only use this for non-GP racing server."));

    SERVER_CFG_PREFIX BoolServerConfigParam m_ai_anywhere
        SERVER_CFG_DEFAULT(BoolServerConfigParam(false, "ai-anywhere",
        "If true this server will allow AI instance to be connected from "
        "anywhere. (other than LAN network only)"));

    // ========================================================================
    /** Server version, will be advanced if there are protocol changes. */
    static const uint32_t m_server_version = 6;
    // ========================================================================
    /** Server database version, will be advanced if there are protocol
     *  changes. */
    static const uint32_t m_server_db_version = 1;
    // ========================================================================
    /** Server uid, extracted from server_config.xml file with .xml removed. */
    extern std::string m_server_uid;
    // ========================================================================
    void loadServerConfig(const std::string& path = "");
    // ------------------------------------------------------------------------
    void loadServerConfigXML(const XMLNode* root, bool default_config = false);
    // ------------------------------------------------------------------------
    std::string getServerConfigXML();
    // ------------------------------------------------------------------------
    void writeServerConfigToDisk();
    // ------------------------------------------------------------------------
    std::pair<RaceManager::MinorRaceModeType, RaceManager::MajorRaceModeType>
        getLocalGameModeFromConfig();
    // ------------------------------------------------------------------------
    std::pair<RaceManager::MinorRaceModeType, RaceManager::MajorRaceModeType>
        getLocalGameMode(int mode);
    // ------------------------------------------------------------------------
    bool getLocalGameModeFromName(const std::string& name,
            int* out,
            bool allow_gp = true,
            bool allow_singleplayer = true);
    // ------------------------------------------------------------------------
    core::stringw getModeName(unsigned id);
    // ------------------------------------------------------------------------
    inline bool unsupportedGameMode()
                           { return m_server_mode == 2 || m_server_mode == 5; }
    // ------------------------------------------------------------------------
    void loadServerLobbyFromConfig();
    // ------------------------------------------------------------------------
    std::string getConfigDirectory();
    // ------------------------------------------------------------------------
    // TierS additions: /item and /nitro commands
    // return an amount for an item got in m_cheat_items, otherwise return 0
    int getCheatQuantity(PowerupManager::PowerupType type);

};   // namespace ServerConfig

#endif // HEADER_SERVER_CONFIG
