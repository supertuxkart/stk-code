# Online networking games for STK

## Hosting server
First of all, you can compile STK with `-DSERVER_ONLY=ON` which will produce a GUI-less STK binary optimized for size and memory usage, useful for situation like in VPS.
The dependencies for RHEL/CentOS 7 are installed with:
```bash
yum install wget; cd /tmp; wget https://dl.fedoraproject.org/pub/epel/7/x86_64/Packages/e/epel-release-7-12.noarch.rpm; rpm -Uvh epel-release*rpm
yum install gcc-c++ cmake openssl-devel libcurl-devel zlib-devel enet
```
### Hosting WAN (public internet) server
You are required to have an stk online account first, go [here](https://online.supertuxkart.net/register.php) for registration.

It is recommended you have a saved user in your computer to allow hosting multiple servers simultaneously with the same account, if you have a fresh STK installation, first run:

If you intend to keep your server always on (24x7) you are required to implement port forward / direct connection with NAT penetration in your network, we will regularly remove any servers not following this rule.

`supertuxkart --init-user --login=your_registered_name --password=your_password`

After that you should see `Done saving user, leaving` in terminal if it successfully logged in.

Than you can just run:

`supertuxkart --server-config=your_config.xml --network-console`

It will create that xml configuration file if not found in current directory, you can type `quit` in terminal, than you can edit that file for further configuration as required.
` --network-console` should not be used if you run supertuxkart server later with systemd service, see issue [#4299](https://github.com/supertuxkart/stk-code/issues/4299).

The current server configuration xml looks like this:
```xml
<?xml version="1.0"?>
<server-config version="6" >

    <!-- Name of server, encode in XML if you want to use unicode characters. -->
    <server-name value="STK Server" />

    <!-- Port used in server, if you specify 0, it will use the server port specified in stk_config.xml. If you wish to use a random port, set random-server-port to '1' in user config. STK will automatically switch to a random port if the port you specify fails to be bound. -->
    <server-port value="0" />

    <!-- Game mode in server, 0 is normal race (grand prix), 1 is time trial (grand prix), 3 is normal race, 4 time trial, 6 is soccer, 7 is free-for-all and 8 is capture the flag. Notice: grand prix server doesn't allow for players to join and wait for ongoing game. -->
    <server-mode value="3" />

    <!-- Difficulty in server, 0 is beginner, 1 is intermediate, 2 is expert and 3 is supertux (the most difficult). -->
    <server-difficulty value="0" />

    <!-- Number of grand prix tracks per game (If grand prix enabled). -->
    <gp-track-count value="3" />

    <!-- Use goal target in soccer. -->
    <soccer-goal-target value="false" />

    <!-- Enable wan server, which requires you to have an stk-addons account with a saved session. Check init-user command for details. -->
    <wan-server value="true" />

    <!-- Enable network console, which can do for example kickban. -->
    <enable-console value="false" />

    <!-- Maximum number of players on the server, setting this to a value greater than 8 can cause performance degradation. -->
    <server-max-players value="8" />

    <!-- Password for private server, leave empty for a public server. -->
    <private-server-password value="" />

    <!-- Message of today shown in lobby, you can enter encoded XML words here or a file.txt and let STK load it. -->
    <motd value="" />

    <!-- If this value is set to false, the server will ignore chat messages from all players. -->
    <chat value="true" />

    <!-- If client sends more than chat-consecutive-interval / 2 chats within this value (read in seconds), it will be ignore, negative value to disable. -->
    <chat-consecutive-interval value="8" />

    <!-- Allow players to vote for which track to play. If this value is set to false, the server will randomly pick the next track to play. -->
    <track-voting value="true" />

    <!-- Timeout in seconds for selecting karts and (or) voting tracks in server, you may want to use a lower value if you have track-voting off. -->
    <voting-timeout value="30" />

    <!-- Timeout in seconds for validation of clients in wan, currently STK will use the stk-addons server to share AES key between the client and server. -->
    <validation-timeout value="20" />

    <!-- By default WAN server will always validate player and LAN will not, disable it to allow non-validated player in WAN. -->
    <validating-player value="true" />

    <!-- Disable it to turn off all stun related code in server, it allows for saving of server resources if your server is not behind a firewall. -->
    <firewalled-server value="true" />

    <!-- Enable to allow IPv6 connection if you have a public IPv6 address. STK currently uses dual-stack mode which requires server to have both IPv4 and IPv6 and listen to same port. If STK detects your server has no public IPv6 address or port differs between IPv4 and IPv6 then it will use IPv4 only socket. For system which doesn't support dual-stack socket (like OpenBSD) you may fail to be connected by IPv4 clients. You can override the detection in config.xml at supertuxkart config-0.10 folder, with default-ip-type option. -->
    <ipv6-connection value="true" />

    <!-- No server owner in lobby which can control the starting of game or kick any players. -->
    <owner-less value="false" />

    <!-- Time to wait before entering kart selection screen if satisfied min-start-game-players below for owner less or ranked server. -->
    <start-game-counter value="60" />

    <!-- Clients below this value will be rejected from joining this server. It's determined by number of official karts in client / number of official karts in server -->
    <official-karts-threshold value="1" />

    <!-- Clients below this value will be rejected from joining this server. It's determined by number of official tracks in client / number of official tracks in server, setting this value too high will prevent android players from joining this server, because STK android apk has some official tracks removed. -->
    <official-tracks-threshold value="0.7" />

    <!-- Only auto start kart selection when number of connected player is larger than or equals this value, for owner less or ranked server, after start-game-counter reaches 0. -->
    <min-start-game-players value="2" />

    <!-- Automatically end linear race game after 1st player finished for some time (currently his finished time * 0.25 + 15.0). -->
    <auto-end value="false" />

    <!-- Enable team choosing in lobby in team game (soccer and CTF). If owner-less is enabled and live-spectate is not enabled, than this option is always disabled. -->
    <team-choosing value="true" />

    <!-- If strict-players is on, no duplicated online id or split screen players are allowed, which can prevent someone using more than 1 network AI with this server. -->
    <strict-players value="false" />

    <!-- Server will submit ranking to stk-addons server for linear race games, you require permission for that. validating-player, auto-end, strict-player and owner-less will be turned on. -->
    <ranked value="false" />

    <!-- If true, the server owner can config the difficulty and game mode in the GUI of lobby. This option cannot be used with owner-less or grand prix server, and will be automatically turned on if the server was created using the in-game GUI. The changed difficulty and game mode will not be saved in this config file. -->
    <server-configurable value="false" />

    <!-- If true, players can live join or spectate the in-progress game. Currently live joining is only available if the current game mode used in server is FFA, CTF or soccer, also official-karts-threshold will be made 1.0. If false addon karts will use their original hitbox other than tux, all players having it restriction applies. -->
    <live-spectate value="true" />

    <!-- Time in seconds when a flag is dropped a by player in CTF returning to its own base. -->
    <flag-return-timeout value="20" />

    <!-- Time in seconds to deactivate a flag when it's captured or returned to own base by players. -->
    <flag-deactivated-time value="3" />

    <!-- Hit limit of free for all, zero to disable hit limit. -->
    <hit-limit value="20" />

    <!-- Time limit of free for all in seconds, zero to disable time limit. -->
    <time-limit-ffa value="360" />

    <!-- Capture limit of CTF, zero to disable capture limit. -->
    <capture-limit value="5" />

    <!-- Time limit of CTF in seconds, zero to disable time limit. -->
    <time-limit-ctf value="600" />

    <!-- Value used by server to automatically estimate each game time. For races, it decides the lap of each race in network game, if more than 0.0f, the number of lap of each track vote in linear race will be determined by max(1.0f, auto-game-time-ratio * default lap of that track). For soccer if more than 0.0f, for time limit game it will be auto-game-time-ratio * soccer-time-limit in UserConfig, for goal limit game it will be auto-game-time-ratio * numgoals in UserConfig, -1 to disable for all. -->
    <auto-game-time-ratio value="-1" />

    <!-- Maximum ping allowed for a player (in ms), it's recommended to use default value if live-spectate is on. -->
    <max-ping value="300" />

    <!-- Tolerance of jitter in network allowed (in ms), it's recommended to use default value if live-spectate is on. -->
    <jitter-tolerance value="100" />

    <!-- Kick players whose ping is above max-ping. -->
    <kick-high-ping-players value="false" />

    <!-- Allow players exceeding max-ping to have a playable game, if enabled kick-high-ping-players will be disabled, please also use a default value for max-ping and jitter-tolerance with it. -->
    <high-ping-workaround value="true" />

    <!-- Kick idle player which has no network activity to server for more than some seconds during game, unless he has finished the race. Negative value to disable, and this option will always be disabled for LAN server. -->
    <kick-idle-player-seconds value="60" />

    <!-- Set how many states the server will send per second, the higher this value, the more bandwidth requires, also each client will trigger more rewind, which clients with slow device may have problem playing this server, use the default value is recommended. -->
    <state-frequency value="10" />

    <!-- Use sql database for handling server stats and maintenance, STK needs to be compiled with sqlite3 supported. -->
    <sql-management value="false" />

    <!-- Database filename for sqlite to use, it can be shared for all servers created in this machine, and STK will create specific table for each server. You need to create the database yourself first, see NETWORKING.md for details -->
    <database-file value="stkservers.db" />

    <!-- Specified in millisecond for maximum time waiting in sqlite3_busy_handler. You may need a higher value if your database is shared by many servers or having a slow hard disk. -->
    <database-timeout value="1000" />

    <!-- IPv4 ban list table name, you need to create the table first, see NETWORKING.md for details, empty to disable. This table can be shared for all servers if you use the same name. STK can auto kick active peer from ban list (update per minute) whichallows live kicking peer by inserting record to database. -->
    <ip-ban-table value="ip_ban" />

    <!-- IPv6 ban list table name, you need to create the table first, see NETWORKING.md for details, empty to disable. This table can be shared for all servers if you use the same name. STK can auto kick active peer from ban list (update per minute) which allows live kicking peer by inserting record to database. -->
    <ipv6-ban-table value="ipv6_ban" />

    <!-- Online ID ban list table name, you need to create the table first, see NETWORKING.md for details, empty to disable. This table can be shared for all servers if you use the same name. STK can auto kick active peer from ban list (update per minute) which allows live kicking peer by inserting record to database. -->
    <online-id-ban-table value="online_id_ban" />

    <!-- Player reports table name, which will be written when a player reports player in the network user dialog, you need to create the table first, see NETWORKING.md for details, empty to disable. This table can be shared for all servers if you use the same name. -->
    <player-reports-table value="player_reports" />

    <!-- Days to keep player reports, older than that will be auto cleared, 0 to keep them forever. -->
    <player-reports-expired-days value="3" />

    <!-- IP geolocation table, you only need this table if you want to geolocate IP from non-stk-addons connection, as all validated players connecting from stk-addons will provide the location info, you need to create the table first, see NETWORKING.md for details, empty to disable. This table can be shared for all servers if you use the same name. -->
    <ip-geolocation-table value="ip_mapping" />

    <!-- IPv6 geolocation table, you only need this table if you want to geolocate IP from non-stk-addons connection, as all validated players connecting from stk-addons will provide the location info, you need to create the table first, see NETWORKING.md for details, empty to disable. This table can be shared for all servers if you use the same name. -->
    <ipv6-geolocation-table value="ipv6_mapping" />

    <!-- If true this server will auto add / remove AI connected with network-ai=x, which will kick N - 1 bot(s) where N is the number of human players. Only use this for non-GP racing server. -->
    <ai-handling value="false" />

    <!-- If true this server will allow AI instance to be connected from anywhere. (other than LAN network only) -->
    <ai-anywhere value="false" />

</server-config>

```

At the moment STK has a list of STUN servers for NAT penetration which allows players or servers behind a firewall or router to be able to connect to each other, but in case it doesn't work, you have to manually disable the firewall or port forward the port(s) used by STK.
By default STK servers use port `2759`. For example, in Ubuntu based distributions, run the following command to disable the firewall on that port:

`sudo ufw allow 2759`

You may also need to handle the server discovery port `2757` for connecting your WAN server in LAN / localhost.

Notice: You don't need to make any firewall or router configuration changes if you connect to the recommended servers (marked with ☆★STK★☆).

### Hosting LAN (local internet) server
Everything is basically the same as WAN one, except you don't need an stk online account, just do:

`supertuxkart --server-config=your_config.xml --lan-server=your_server_name --network-console`

For LAN server it is required that the server and server discovery port is connectable by clients directly, no NAT penetration will be done in LAN.

LAN server can be connected too by typing your server public address (with port) in ```Enter server address``` dialog without relying on stk-addons.

------
After the first time configuration, you can just start the server with the command:

`supertuxkart --server-config=your_config.xml`, regardless of whether LAN or WAN server is chosen (of course you need to have a saved user for the WAN one), by default your server logging will be saved to the STK configuration directory with a name of `your_config.log`, given that the server configuration filename is `your_config.xml`.

You can find out that directory location [here (See Where is the configuration stored?)](https://supertuxkart.net/FAQ)

## Testing server
There is a network AI tester in STK which can use AI on player controller for server hosting linear races game mode, which helps automating the testing for servers, to enable it use it on lan server:

`supertuxkart --connect-now=x.x.x.x:y --network-ai=n --no-graphics`

Remove `--no-graphics` if you want to see the AI racing. You can also run network AI tester in server-only build of STK.

With the network AI tester, it's easier to for example simulate high-loaded servers or bad networks (ones with high ping and/or packet loss).

Tested on a Raspberry Pi 3 Model B+, if you have 8 players connected to a server hosted on it, the usage of a single CPU core is ~60% and there are ~60MB of memory usage for game with heavy tracks like Cocoa Temple or Candela City on the server, you can use the above figures to estimate how many STK servers can be hosted on the same computer.

For bad network simulation, we recommend `network traffic control` by Linux kernel, see [here](https://wiki.linuxfoundation.org/networking/netem) for details.

You will have the best gaming experience by choosing a server where all players have less than 100ms ping with no packet loss.

## Server management (Since 1.1)

Currently STK uses sqlite (if building with sqlite3 on) for server management with the following functions at the moment:
1. Server statistics
2. IP / online ID ban list
3. Player reports
4. IPv4 and IPv6 geolocation

You need to create a database in sqlite first, run `sqlite3 stkservers.db` in the folder where (all) your server_config.xml(s) located.

A table named `v(server database version)_(your_server_config_filename_without_.xml_extension)_stats` will also be created in your database if one does not exist.:
```sql
CREATE TABLE IF NOT EXISTS (table name above)
(
    host_id INTEGER UNSIGNED NOT NULL PRIMARY KEY, -- Unique host id in STKHost of each connection session for a STKPeer
    ip INTEGER UNSIGNED NOT NULL, -- IP decimal of host
    ipv6 TEXT NOT NULL DEFAULT '', -- IPv6 (if exists) in string of host (only created if IPv6 server)
    port INTEGER UNSIGNED NOT NULL, -- Port of host
    online_id INTEGER UNSIGNED NOT NULL, -- Online if of the host (0 for offline account)
    username TEXT NOT NULL, -- First player name in the host (if the host has splitscreen player)
    player_num INTEGER UNSIGNED NOT NULL, -- Number of player(s) from the host, more than 1 if it has splitscreen player
    country_code TEXT NULL DEFAULT NULL, -- 2-letter country code of the host
    version TEXT NOT NULL, -- SuperTuxKart version of the host
    os TEXT NOT NULL, -- Operating system of the host
    connected_time TIMESTAMP NOT NULL DEFAULT CURRENT_TIMESTAMP, -- Time when connected
    disconnected_time TIMESTAMP NOT NULL DEFAULT CURRENT_TIMESTAMP, -- Time when disconnected (saved when disconnected)
    ping INTEGER UNSIGNED NOT NULL DEFAULT 0 -- Ping of the host
) WITHOUT ROWID;
```

STK will also create the following default views from the stats table:

`*_full_stats`
Full stats with ip in human readable format and time played of each players in minutes.

`*_current_players`
Current players in server with ip in human readable format and time played of each players in minutes.

`*_player_stats`
All players with online id and username with their time played stats in this server since creation of this database.
If sqlite supports window functions (since 3.25), it will include last session player info (ip, country, ping...).

A empty table named `v(server database version)_countries` will also be created in your database if not exists:
```sql
CREATE TABLE IF NOT EXISTS (table name above)
(
    country_code TEXT NOT NULL PRIMARY KEY UNIQUE, -- Unique 2-letter country code
    country_flag TEXT NOT NULL, -- Unicode country flag representation of 2-letter country code
    country_name TEXT NOT NULL -- Readable name of this country
) WITHOUT ROWID;
```

If you want to see flags and readable names of countries in the above views, you need to initialize `v(server database version)_countries` table, check [this script](tools/generate-countries-table.py).

For IPv4 and online ID ban list, player reports or IP mapping, you need to create one yourself:
```sql
CREATE TABLE ip_ban
(
    ip_start INTEGER UNSIGNED NOT NULL UNIQUE, -- Starting of ip decimal for banning (inclusive)
    ip_end INTEGER UNSIGNED NOT NULL UNIQUE, -- Ending of ip decimal for banning (inclusive)
    starting_time TIMESTAMP NOT NULL DEFAULT CURRENT_TIMESTAMP, -- Starting time of this banning entry to be effective
    expired_days REAL NULL DEFAULT NULL, -- Days for this banning to be expired, use NULL for a permanent ban
    reason TEXT NOT NULL DEFAULT '', -- Banned reason shown in user stk menu, can be empty
    description TEXT NOT NULL DEFAULT '', -- Private description for server admin
    trigger_count INTEGER UNSIGNED NOT NULL DEFAULT 0, -- Number of banning triggered by this ban entry
    last_trigger TIMESTAMP NULL DEFAULT NULL -- Latest time this banning entry was triggered
);

CREATE TABLE ipv6_ban
(
    ipv6_cidr TEXT NOT NULL UNIQUE, -- IPv6 CIDR range for banning (for example 2001::/64), use /128 for a specific ip
    starting_time TIMESTAMP NOT NULL DEFAULT CURRENT_TIMESTAMP, -- Starting time of this banning entry to be effective
    expired_days REAL NULL DEFAULT NULL, -- Days for this banning to be expired, use NULL for a permanent ban
    reason TEXT NOT NULL DEFAULT '', -- Banned reason shown in user stk menu, can be empty
    description TEXT NOT NULL DEFAULT '', -- Private description for server admin
    trigger_count INTEGER UNSIGNED NOT NULL DEFAULT 0, -- Number of banning triggered by this ban entry
    last_trigger TIMESTAMP NULL DEFAULT NULL -- Latest time this banning entry was triggered
);

CREATE TABLE online_id_ban
(
    online_id INTEGER UNSIGNED NOT NULL UNIQUE, -- Online id from STK addons database for banning
    starting_time TIMESTAMP NOT NULL DEFAULT CURRENT_TIMESTAMP, -- Starting time of this banning entry to be effective
    expired_days REAL NULL DEFAULT NULL, -- Days for this banning to be expired, use NULL for a permanent ban
    reason TEXT NOT NULL DEFAULT '', -- Banned reason shown in user stk menu, can be empty
    description TEXT NOT NULL DEFAULT '', -- Private description for server admin
    trigger_count INTEGER UNSIGNED NOT NULL DEFAULT 0, -- Number of banning triggered by this ban entry
    last_trigger TIMESTAMP NULL DEFAULT NULL -- Latest time this banning entry was triggered
);

CREATE TABLE player_reports
(
    server_uid TEXT NOT NULL, -- Report from which server unique id (config filename)
    reporter_ip INTEGER UNSIGNED NOT NULL, -- IP decimal of player who reports
    reporter_ipv6 TEXT NOT NULL DEFAULT '', -- IPv6 (if exists) in string of player who reports (only needed for IPv6 server)
    reporter_online_id INTEGER UNSIGNED NOT NULL, -- Online id of player who reports, 0 for offline player
    reporter_username TEXT NOT NULL, -- Player name who reports
    reported_time TIMESTAMP NOT NULL DEFAULT CURRENT_TIMESTAMP, -- Time of reporting
    info TEXT NOT NULL, -- Report info by reporter
    reporting_ip INTEGER UNSIGNED NOT NULL, -- IP decimal of player being reported
    reporting_ipv6 TEXT NOT NULL DEFAULT '', -- IPv6 (if exists) in string of player who reports (only needed for IPv6 server)
    reporting_online_id INTEGER UNSIGNED NOT NULL, -- Online id of player being reported, 0 for offline player
    reporting_username TEXT NOT NULL -- Player name being reported
);

CREATE TABLE ip_mapping
(
    ip_start INTEGER UNSIGNED NOT NULL PRIMARY KEY UNIQUE, -- IP decimal start
    ip_end INTEGER UNSIGNED NOT NULL UNIQUE, -- IP decimal end
    latitude REAL NOT NULL, -- Latitude of this IP range
    longitude REAL NOT NULL, -- Longitude of this IP range
    country_code TEXT NOT NULL -- 2-letter country code
) WITHOUT ROWID;

CREATE TABLE ipv6_mapping
(
    ip_start INTEGER UNSIGNED NOT NULL PRIMARY KEY UNIQUE, -- IP decimal (upper 64bit) start
    ip_end INTEGER UNSIGNED NOT NULL UNIQUE, -- IP decimal (upper 64bit) end
    latitude REAL NOT NULL, -- Latitude of this IP range
    longitude REAL NOT NULL, -- Longitude of this IP range
    country_code TEXT NOT NULL -- 2-letter country code
)
```

For initialization of `ip_mapping` table, check [this script](tools/generate-ip-mappings.py).
