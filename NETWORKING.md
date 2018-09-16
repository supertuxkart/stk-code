# Online networking games for STK

## Hosting server
First of all, you can compile STK with `-DSERVER_ONLY=ON` which will produce a GUI-less STK binary optimized for size and memory usage, useful for situation like in VPS.

### Hosting WAN (public internet) server
You are required to have an stk online account first, go [here](https://addons.supertuxkart.net/register.php) for registration.

It is recommended you have a saved user in your computer to allow hosting multiple servers simultaneously with the same account, if you have a fresh STK installation, first run:

`supertuxkart --init-user --login=your_registered_name --password=your_password`

After that you should see `Done saving user, leaving` in terminal if it successfully logged in.

Than you can just run:

`supertuxkart --server-config=your_config.xml --network-console`

It will create that xml configuration file if not found in current directory, you can type `quit` in terminal, than you can edit that file for further configuration as required.

The current server configuration xml looks like this:
```xml
<?xml version="1.0"?>
<server-config version="1" >

    <!-- Name of server, encode in XML if you want to use unicode characters. -->
    <server-name value="stk server" />

    <!-- Port used in server, if you specify 0, it will use the server port specified in stk_config.xml or if random-server-port is enabled in user config, than any port. STK will auto change to random port if the port you specify failed to be bound. -->
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

    <!-- Maximum number of players on the server, setting it more than 8 will have performance degradation. -->
    <server-max-players value="8" />

    <!-- Password for private server, empty for a public server. -->
    <private-server-password value="" />

    <!-- Message of today shown in lobby, you can enter encoded XML words here or a file.txt and let STK load it. -->
    <motd value="" />

    <!-- Timeout in seconds for voting tracks in server. -->
    <voting-timeout value="20" />

    <!-- Timeout in seconds for validation of clients in wan, currently stk will use the stk-addons server to share AES key between client and server. -->
    <validation-timeout value="20" />

    <!-- By default WAN server will always validate player and LAN will not, disable it to allow non-validated player in WAN. -->
    <validating-player value="true" />

    <!-- Disable it to turn off all stun related code in server, it allows saving server resource if your server is not behind a firewall. -->
    <firewalled-server value="true" />

    <!-- No server owner in lobby which can control the starting of game or kick any players. -->
    <owner-less value="false" />

    <!-- Time to wait before entering kart selection screen if satisfied min-start-game-players below for owner less or ranked server. -->
    <start-game-counter value="30" />

    <!-- Only auto start kart selection when number of connected player is larger than or equals this value, for owner less or ranked server, after start-game-counter reaches 0. -->
    <min-start-game-players value="2" />

    <!-- Automatically end linear race game after 1st player finished for some time (currently his finished time * 0.25 + 15.0). -->
    <auto-end value="false" />

    <!-- Enable team choosing in lobby in team game (soccer and CTF). If owner-less is enabled, than this option is always disabled. -->
    <team-choosing value="true" />

    <!-- Server will submit ranking to stk addons server for linear race games, you require permission for that. validating-player, auto-end and owner-less will be turned on. -->
    <ranked value="false" />

    <!-- Time in seconds when a flag is dropped a by player in CTF returning to its own base. -->
    <flag-return-timemout value="20" />

    <!-- Value used to calculate hit limit in free for all, which is min(number of players * hit-limit-threshold, 40), negative value to disable hit limit. -->
    <hit-limit-threshold value="5" />

    <!-- Value used to calculate time limit in free for all, which is max(number of players * time-limit-threshold-ffa, 3.0) * 60, negative value to disable time limit. -->
    <time-limit-threshold-ffa value="0.7" />

    <!-- Value used to calculate capture limit in CTF, which is max(3.0, number of players * capture-limit-threshold), negative value to disable capture limit. -->
    <capture-limit-threshold value="0.7" />

    <!-- Value used to calculate time limit in CTF, which is max(3.0, number of players * (time-limit-threshold-ctf + flag-return-timemout / 60.0)) * 60.0, negative value to disable time limit. -->
    <time-limit-threshold-ctf value="0.9" />

    <!-- Value used by server to automatically calculate lap of each race in network game, if more than 0.0f, the number of lap of each track vote in linear race will be determined by max(1.0f, auto-lap-ratio * default lap of that track). -->
    <auto-lap-ratio value="-1" />

    <!-- Maximum ping allowed for a player (in ms). -->
    <max-ping value="300" />

    <!-- Tolerance of jitter in network allowed (in ms). -->
    <jitter-tolerance value="100" />

    <!-- Kick players whose ping is above max-ping. -->
    <kick-high-ping-players value="false" />

    <!-- ip: IP in X.X.X.X/Y (CIDR) format for banning, use Y of 32 for a specific ip, expired-time: unix timestamp to expire, if -1 (uint32_t max) than a permanent ban. -->
    <server-ip-ban-list>
        <ban ip="0.0.0.0/0" expired-time="0"/>
    </server-ip-ban-list>

    <!-- online-id: online id for banning, expired-time: unix timestamp to expire, if -1 (uint32_t max) than a permanent ban. -->
    <server-online-id-ban-list>
        <ban online-id="0" expired-time="0"/>
    </server-online-id-ban-list>

</server-config>


```

At the moment STK has a list of STUN servers for NAT penetration which allows players or servers behind a firewall or router to be able to connect to each other, but in case it doesn't work, you have to manually disable the firewall or port forward the port(s) used by the STK.
By default STK servers use port `2759`. For example, in Ubuntu based distributions, run the following command to disable the firewall on that port:

`sudo ufw allow 2759`

You may also need to handle the server discovery port `2757` for connecting your WAN server in LAN / localhost.

Notice: You don't need to make any firewall or router configuration changes if you connect to our official servers.

### Hosting LAN (local internet) server
Everything is basically the same as WAN one, except you don't need an stk online account, just do:

`supertuxkart --server-config=your_config.xml --lan-server=your_server_name --network-console`

In LAN network it is required that the server and server discovery port is connectable by clients directly, no NAT penetration will be done in LAN.

------
After the first time configuration, you can just start the server with the command:

`supertuxkart --server-config=your_config.xml`, regardless of whether LAN or WAN server is chosen (of course you need to have a saved user for the WAN one), by default your server logging will be saved to the STK configuration directory with a name of `your_config.log`, given that the server configuration filename is `your_config.xml`.

You can find out that directory location [here (See Where is the configuration stored?)](https://supertuxkart.net/FAQ)

## Testing server
There is a network AI tester in STK which can use AI on player controller for server hosting linear races game mode, which helps automating the testing for servers, to enable it use:

`supertuxkart --connect-now=x.x.x.x:y --server-id=id --network-ai=n --auto-connect --no-graphics`

x.x.x.x:y is your server ip address with its port, id is the id field of server-info in STK server xml list, omit it if you are testing LAN server, n is the number of AI you want to create.

You can see STK server xml list [here](https://addons.supertuxkart.net/api/v2/server/get-all).

The server you want to test must be able to be connected without NAT penetration. You can remove `--auto-connect` if you have another client which can control the starting of games in server, or you can consider enable owner-less mode on server so the games on server can keep going. Remove `--no-graphics` if you want to see the AI racing. You can also run network AI tester in server-only build of STK.

With the network AI tester, it's easier to for example simulate high-loaded servers or bad (high ping with packet loss) network.

Tested on a Raspberry Pi 3 Model B+, if you have 8 players connected to a server hosted on it, the usage of a single CPU core is ~60% and there are ~60MB of memory usage for game with heavy tracks like Cocoa Temple or Candela City on the server, you can use the above figures to consider number of STK servers hosting on a same computer.

For bad network simulation, we recommend `network traffic control` by linux kernel, see [here](https://wiki.linuxfoundation.org/networking/netem) for details.

You have the best gaming experience when choosing server less than 100ms ping with no packet loss.
